/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPortStat.c
*
* @brief CPSS implementation for core port statistics and
* egress counters facility.
*
* @version   58
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prcCpssDxChPortStat.h>

#ifdef PRESTERA_DEBUG
#define PORT_DEBUG
#endif /* PRESTERA_DEBUG */

#ifdef PORT_DEBUG
#define DBG_INFO(x) cpssOsPrintf x
#else
#define DBG_INFO(x)
#endif

/* does this port uses the GE port mechanism or the XG mechanism */
/* 1 - port uses GE port mechanism */
/* 0 - port uses XG port mechanism */
#define IS_GE_PORT_COUNTERS_MAC(_devNum,_portNum)                    \
    ((PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(_devNum,_portNum) == GT_FALSE &&    \
      PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum,_portNum) <= PRV_CPSS_PORT_GE_E) ? \
      ((_portNum < 24) ? 1 : 0) : 0)

/* does this port need to emulate old MIB counters behavior */
#define IS_MIB_API_SW_EMULATED_MAC(_devNum,_portNum)                                            \
    (                                                                                           \
        ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E) &&                 \
        (PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum,_portNum) == PRV_CPSS_PORT_XLG_E))                 \
                                    ||                                                          \
        (                                                                                       \
            ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||            \
            (PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||                                               \
            (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))               \
                                    &&                                                          \
            ((_portNum != CPSS_CPU_PORT_NUM_CNS) ||                                             \
            (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE))  \
         )                                                                                      \
    )
/* macro to check if the MAC is 'MTI'  */
#define IS_MTI_MAC_MAC(_devNum, _portMacNum)                                             \
    (((PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum, _portMacNum) == PRV_CPSS_PORT_MTI_64_E ) ||   \
      (PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum, _portMacNum) == PRV_CPSS_PORT_MTI_400_E)) ? 1 : 0)


/******* info about Ethernet CPU port ******************************************
    support only next counters:

    1. CPSS_GOOD_PKTS_SENT_E:      32 bits counter
    2. CPSS_MAC_TRANSMIT_ERR_E:    16 bits counter
    3. CPSS_GOOD_OCTETS_SENT_E:    32 bits counter
    4. CPSS_DROP_EVENTS_E:         16 bits counter
    5. CPSS_GOOD_PKTS_RCV_E:       16 bits counter
    6. CPSS_BAD_PKTS_RCV_E:        16 bits counter
    7. CPSS_GOOD_OCTETS_RCV_E:     32 bits counter
    8. CPSS_BAD_OCTETS_RCV_E:      16 bits counter

*******************************************************************************/

/* array of MAC counters offsets */
/* gtMacCounterOffset[0] - offsets for not XGMII interface */
static GT_U8 internal_gtMacCounterOffset[CPSS_LAST_MAC_COUNTER_NUM_E];
static GT_BOOL isMacCounterOffsetInit = GT_FALSE;
/* for device that all counters are 64 bits : BC3 */
static GT_U8 internal_gtMacCounterOffset_all64Bits[CPSS_LAST_MAC_COUNTER_NUM_E];

/* Forward declaration */
static GT_STATUS prvCpssDxChPortMacCountersCGMibsConvert
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry
);

static GT_STATUS prvCpssDxChPortMacCountersMtiMibsConvert
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
);

/**
* @internal internal_cpssDxChPortStatInit function
* @endinternal
*
* @brief   Init port statistics counter set CPSS facility.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortStatInit
(
    IN  GT_U8       devNum
)
{
    GT_U8  i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(isMacCounterOffsetInit == GT_FALSE)
    {
        /******************************************/
        /************* init first DB **************/
        /******************************************/
        internal_gtMacCounterOffset[CPSS_GOOD_OCTETS_RCV_E]  = 0x0;
        internal_gtMacCounterOffset[CPSS_BAD_OCTETS_RCV_E]   = 0x8;
        internal_gtMacCounterOffset[CPSS_GOOD_OCTETS_SENT_E] = 0x38;
        internal_gtMacCounterOffset[CPSS_GOOD_UC_PKTS_RCV_E] = 0x10;
        internal_gtMacCounterOffset[CPSS_GOOD_UC_PKTS_SENT_E] = 0x40;
        internal_gtMacCounterOffset[CPSS_MULTIPLE_PKTS_SENT_E] = 0x50;
        internal_gtMacCounterOffset[CPSS_DEFERRED_PKTS_SENT_E] = 0x14;

        for (i = CPSS_MAC_TRANSMIT_ERR_E; i <= CPSS_GOOD_OCTETS_SENT_E; i++)
            internal_gtMacCounterOffset[i] = (GT_U8)(0x4 + (i * 4));

        for (i = CPSS_GOOD_OCTETS_SENT_E + 1; i < CPSS_GOOD_UC_PKTS_RCV_E; i++)
            internal_gtMacCounterOffset[i] = (GT_U8)(0x8 + (i * 4));

        /******************************************/
        /************* init second DB *************/
        /******************************************/
        for (i = CPSS_GOOD_OCTETS_RCV_E; i < CPSS_GOOD_UC_PKTS_RCV_E; i++)
        {
            internal_gtMacCounterOffset_all64Bits[i]  = i*8;
        }

        /* duplications / rename */
        internal_gtMacCounterOffset_all64Bits[CPSS_GOOD_UC_PKTS_RCV_E]  = internal_gtMacCounterOffset_all64Bits[CPSS_GOOD_PKTS_RCV_E];
        internal_gtMacCounterOffset_all64Bits[CPSS_GOOD_UC_PKTS_SENT_E] = internal_gtMacCounterOffset_all64Bits[CPSS_GOOD_PKTS_SENT_E];
        internal_gtMacCounterOffset_all64Bits[CPSS_MULTIPLE_PKTS_SENT_E]= internal_gtMacCounterOffset_all64Bits[CPSS_UNRECOG_MAC_CNTR_RCV_E];
        internal_gtMacCounterOffset_all64Bits[CPSS_DEFERRED_PKTS_SENT_E]= internal_gtMacCounterOffset_all64Bits[CPSS_BAD_PKTS_RCV_E];
    }

    /* bind the device with proper DB */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_FALSE)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset = internal_gtMacCounterOffset;
    }
    else
    {
        /* BC3 */
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset = internal_gtMacCounterOffset_all64Bits;
    }

    isMacCounterOffsetInit = GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChPortStatInit function
* @endinternal
*
* @brief   Init port statistics counter set CPSS facility.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortStatInit
(
    IN  GT_U8       devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortStatInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPortStatInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal dxChMacCountersCpuPortGet function
* @endinternal
*
* @brief   Gets Ethernet CPU port MAC counter .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - not supported counter on the port
*/
static GT_STATUS dxChMacCountersCpuPortGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_U32  regAddr = /*0x00000060*/
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.macCounters;

    switch(cntrName)
    {
        case CPSS_GOOD_PKTS_SENT_E:
            regAddr += 0x0; /* 32 bits */
            break;
        case CPSS_MAC_TRANSMIT_ERR_E:
            regAddr += 0x4; /* 16 bits */
            break;
        case CPSS_GOOD_OCTETS_SENT_E:
            regAddr += 0x8; /* 32 bits */
            break;
        case CPSS_DROP_EVENTS_E:
            regAddr += 0xC; /* 16 bits */
            break;
        case CPSS_GOOD_PKTS_RCV_E:
            regAddr += 0x10;/* 16 bits */
            break;
        case CPSS_BAD_PKTS_RCV_E:
            regAddr += 0x14;/* 16 bits */
            break;
        case CPSS_GOOD_OCTETS_RCV_E:
            regAddr += 0x18;/* 32 bits */
            break;
        case CPSS_BAD_OCTETS_RCV_E:
            regAddr += 0x1c;/* 16 bits */
            break;
        default:
            /* not supported counter */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDrvHwPpPortGroupReadRegister(devNum, PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum),
            regAddr, &(cntrValuePtr->l[0]));
}
/*******************************************************************************
* prvCpssDxChPortSumMibEntry
*
* DESCRIPTION:
*       Sums the counters (destEntry+addEntryPtr), puts the result to destEntry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       destEntryArr
*       addEntryArr
*
* OUTPUTS:
*       destEntryArr
*
* RETURNS:
*       none
*
* COMMENTS:
*       none
*
*******************************************************************************/
/*extern*/ GT_VOID prvCpssDxChPortSumMibEntry
(
    IN  GT_U8  devNum,
    INOUT GT_U32 destEntryArr[/*PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN    GT_U32 addEntryArr[/*PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/]
)
{
    GT_U32 i;
    GT_U64 temp1, temp2;
    GT_U32  numRegisters;
    GT_BOOL portMibCounters64Bits;

    portMibCounters64Bits = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits;

    numRegisters = PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum);

    for (i = 0 ; i < numRegisters ; i++)
    {
        if ((portMibCounters64Bits == GT_TRUE) ||/*all counters are 64 bits*/
            (i == 0 || i == 14))                 /*or specific counters are 64 bits*/
        {
            /* If we in 64bit mode calculation and we now iterating the last element in the arrays,
               it means we allready calculated this element in previuos iteration so we done here.
               We do not want to access element at [<lastIdx>+1] which will cause memory overrun*/
            if ( i == (numRegisters-1) )
            {
                break;
            }
            temp1.l[0] = destEntryArr[i];
            temp1.l[1] = destEntryArr[i+1];
            temp2.l[0] = addEntryArr[i];
            temp2.l[1] = addEntryArr[i+1];
            temp1 = prvCpssMathAdd64(temp1,temp2);
            destEntryArr[i] = temp1.l[0];
            destEntryArr[i+1] = temp1.l[1];
            i++;
        }
        else
        {
            destEntryArr[i] += addEntryArr[i];
        }
    }
}


/**
* @internal prvCpssDxChMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter / MAC Captured counter for a
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number,
*                                      CPU port if getFromCapture is GT_FALSE
* @param[in] cntrName                 - specific counter name
* @param[in] getFromCapture           -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The 10G MAC MIB counters are 64-bit wide.
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
static GT_STATUS prvCpssDxChMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    IN  GT_BOOL                     getFromCapture,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 baseRegAddr;     /* base register address */
    GT_U32 regVal;        /* register value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS rc;      /* return code */
    GT_U32 *entryPtr;
    GT_U32 xlgMibEntry[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL isCgMac = GT_FALSE;
    GT_BOOL isMtiMac = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(getFromCapture == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    isCgMac = (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E);
    isMtiMac = IS_MTI_MAC_MAC(devNum, portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    if((GT_U32)cntrName >= (GT_U32)CPSS_LAST_MAC_COUNTER_NUM_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    cntrValuePtr->l[0] = 0;
    cntrValuePtr->l[1] = 0;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE &&
        portMacNum == CPSS_CPU_PORT_NUM_CNS)
    {
        /* the CPU port support only 8 counters */
        return dxChMacCountersCpuPortGet(devNum,cntrName,cntrValuePtr);
    }

    /* these counters do NOT exist */
    if ( (cntrName == CPSS_BAD_PKTS_RCV_E) ||
         (cntrName == CPSS_UNRECOG_MAC_CNTR_RCV_E) ||
         (cntrName == CPSS_BadFC_RCV_E) ||
         (cntrName == CPSS_GOOD_PKTS_RCV_E) ||
         (cntrName == CPSS_GOOD_PKTS_SENT_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* CPSS_COLLISIONS_E is the PFC counter.
           CPSS_LATE_COLLISIONS_E is the Unknown FC counter.
           The following counters do NOT exist for 10/20/40 Giga ports in eArch devices. */
        if( (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) &&
            ( (cntrName == CPSS_EXCESSIVE_COLLISIONS_E) ||
              (cntrName == CPSS_MULTIPLE_PKTS_SENT_E)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* these counters do NOT exist for 10/20/40 Giga ports */
        if( (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) &&
            ( (cntrName == CPSS_EXCESSIVE_COLLISIONS_E) ||
              (cntrName == CPSS_COLLISIONS_E) ||
              (cntrName == CPSS_LATE_COLLISIONS_E) ||
              (cntrName == CPSS_MULTIPLE_PKTS_SENT_E) ||
              (cntrName == CPSS_DEFERRED_PKTS_SENT_E)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssDxChPortStatInit(devNum);

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        xlgMibShadowPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];
        if(getFromCapture == GT_TRUE)
        {
            entryPtr = xlgMibShadowPtr->captureMibShadow;
        }
        else
        {
            if (isCgMac)
            {
                /* CG MAC MIB counters need to be converted to the regular structure */
                rc = prvCpssDxChPortMacCountersCGMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
            }
            else if (isMtiMac)
            {
                /* CG MAC MIB counters need to be converted to the regular structure */
                rc = prvCpssDxChPortMacCountersMtiMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
            }
            else
            {
                /* read the whole entry */
                rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,
                      PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters,
                      PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum),xlgMibEntry);
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            /* add to shadow */
            prvCpssDxChPortSumMibEntry(devNum,xlgMibShadowPtr->mibShadow,xlgMibEntry);
            entryPtr = xlgMibShadowPtr->mibShadow;
        }

        cntrValuePtr->l[0] = entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4];

        if(getFromCapture == GT_FALSE && xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
        {
            /* Clearing the specific entry in the shadow */
            entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] = 0;
        }

        /* In case the MIB counters are configured as 64bit or the current MIB entry
           is on of the two mib entries with 64 bit length we will read the second 32bits */
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
            cntrName == CPSS_GOOD_OCTETS_RCV_E || cntrName == CPSS_GOOD_OCTETS_SENT_E)
        {
            cntrValuePtr->l[1] = entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 +1];
            if(getFromCapture == GT_FALSE && xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
            {
                /* Clearing the specific entry in the shadow */
                entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 +1] = 0;
            }
        }

        return GT_OK;
    }

    /* If we got until here, the mib emulation not needed and we will read counters from HW,
       either from the HW capture registers or the actual counters*/

    if(getFromCapture == GT_TRUE)
    {
        baseRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                  perPortRegs[portMacNum].macCaptureCounters;
    }
    else
    {
        baseRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                  perPortRegs[portMacNum].macCounters;
    }

    regAddr = baseRegAddr + PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName];

    if ( prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
            regAddr, &(cntrValuePtr->l[0])) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    switch (cntrName)
    {
        case CPSS_GOOD_OCTETS_RCV_E:
        case CPSS_GOOD_OCTETS_SENT_E:
            /* this counter has 64 bits */
            regAddr = regAddr + 4;
            if ( prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cntrValuePtr->l[1])) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            break;

        case CPSS_OVERSIZE_PKTS_E:
            /* Wrong read value for oversize and jabber counters in XG MIBs
               (FEr#85)*/
            if((PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XG_MIB_READ_OVERSIZE_AND_JABBER_COUNTERS_WA_E) ==
                GT_TRUE) && (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E))
            {
                /* Dummy read of GT_JABBER_PKTS */
                regAddr = baseRegAddr + PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName + 1];

                if (prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                        regAddr, &regVal) != GT_OK)
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_JABBER_PKTS_E:
            if((PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_XG_MIB_READ_OVERSIZE_AND_JABBER_COUNTERS_WA_E) ==
                GT_TRUE) && (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E))
            {
                /* Last read was GT_OVERSIZE_PKTS. Read again. */
                if (prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                        regAddr, &(cntrValuePtr->l[0])) != GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       CPSS_GOOD_PKTS_SENT_E, CPSS_MAC_TRANSMIT_ERR_E, CPSS_DROP_EVENTS_E,
*       CPSS_GOOD_OCTETS_SENT_E, CPSS_GOOD_PKTS_RCV_E, CPSS_BAD_PKTS_RCV_E,
*       CPSS_GOOD_OCTETS_RCV_E, CPSS_BAD_OCTETS_RCV_E.
*
*/
static GT_STATUS internal_cpssDxChMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    PRV_CPSS_DXCH_ERRATA_STACK_MAC_COUNT_SHADOW_STC *macCountShadowPtr; /* Pointer To Shadow of all
                                                    MAC Counters of ports 24-27 */
    GT_BOOL                                         clearOnReadWa;      /* clear On Read WA   */
    GT_U64                                          cntrValue;          /* counter value      */
    GT_U64                                          *cntrShadowPtr;     /* Pointer To Shadow of
                                                    MAC Counter specified by function parameters */
    GT_STATUS                                       rc;                 /* return code        */
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);
    if((GT_U32)cntrName >= (GT_U32)CPSS_LAST_MAC_COUNTER_NUM_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCounterGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCounterGet(devNum,portNum,
                                  cntrName,cntrValuePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* default */
    clearOnReadWa     = GT_FALSE;
    macCountShadowPtr = NULL;

    if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(devNum, portMacNum))
    {
        macCountShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_E.
            stackMacCntPtr;
        clearOnReadWa =
            macCountShadowPtr->stackPortArr
            [portMacNum - PRV_CPSS_DXCH_ERRATA_STACK_PORT_BASE_CNS].clearOnRead;
    }

    rc = prvCpssDxChMacCounterGet(
        devNum, portNum, cntrName, GT_FALSE, &cntrValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (clearOnReadWa == GT_FALSE)
    {
        *cntrValuePtr = cntrValue;
        return GT_OK;
    }

    /* clearOnRead Work Around implementation */

    cntrShadowPtr =
        &(macCountShadowPtr->stackPortArr[portMacNum - 24].
          macCntrShadow[cntrName]);

    /* subtract the shadowed value instead of clearing the counter */
    *cntrValuePtr = prvCpssMathSub64(cntrValue, *cntrShadowPtr);

    /* update the counter shadow */
    *cntrShadowPtr = cntrValue;

    return GT_OK;
}

/**
* @internal cpssDxChMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Potrs:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       CPSS_GOOD_PKTS_SENT_E, CPSS_MAC_TRANSMIT_ERR_E, CPSS_DROP_EVENTS_E,
*       CPSS_GOOD_OCTETS_SENT_E, CPSS_GOOD_PKTS_RCV_E, CPSS_BAD_PKTS_RCV_E,
*       CPSS_GOOD_OCTETS_RCV_E, CPSS_BAD_OCTETS_RCV_E.
*
*/
GT_STATUS cpssDxChMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cntrName, cntrValuePtr));

    rc = internal_cpssDxChMacCounterGet(devNum, portNum, cntrName, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cntrName, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMacCountersIsSupportedCheck function
* @endinternal
*
* @brief   Checks if the counter is supported by current device and port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrName                 - counter name
* @param[in] portMacNum               - port MAC number
* @param[in] counterMode              - CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E -
*                                      MIB counter index 4 is sent deferred. MIB counter index 12 is Frames1024toMaxOctets
*                                      CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E -
*                                      MIB counter index 4 is Frames1024to1518Octets. MIB counter index 12 is Frames1519toMaxOctets
*
* @retval GT_TRUE                  - if counter is supported by current device and port
* @retval GT_FALSE                 - if counter is not supported by current device and port
*/
GT_BOOL prvCpssDxChPortMacCountersIsSupportedCheck
(
    IN  GT_U8                                                  devNum,
    IN  GT_U32                                                 portMacNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT                             cntrName,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
{
    GT_BOOL result = GT_TRUE;

        if( ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) && (portMacNum != CPSS_CPU_PORT_NUM_CNS))  ||
            PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
        if(((cntrName == CPSS_PKTS_1024TO1518_OCTETS_E) || (cntrName == CPSS_PKTS_1519TOMAX_OCTETS_E)) &&
            (CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E == counterMode))
        {
            result = GT_FALSE;
        }
        if((cntrName == CPSS_PKTS_1024TOMAX_OCTETS_E) && (CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E != counterMode))
        {
            result = GT_FALSE;
        }
    }

    /* the CPU port support only 8 counters */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE &&
        portMacNum == CPSS_CPU_PORT_NUM_CNS)
    {
        if ( (cntrName != CPSS_GOOD_PKTS_SENT_E) &&
             (cntrName != CPSS_MAC_TRANSMIT_ERR_E) &&
             (cntrName != CPSS_GOOD_OCTETS_SENT_E) &&
             (cntrName != CPSS_DROP_EVENTS_E) &&
             (cntrName != CPSS_GOOD_PKTS_RCV_E) &&
             (cntrName != CPSS_BAD_PKTS_RCV_E) &&
             (cntrName != CPSS_GOOD_OCTETS_RCV_E) &&
             (cntrName != CPSS_BAD_OCTETS_RCV_E) )
        {
            result = GT_FALSE;;
        }
    }
    else
    {
        /* these counters do NOT exist */
        if ( (cntrName == CPSS_BAD_PKTS_RCV_E) ||
             (cntrName == CPSS_UNRECOG_MAC_CNTR_RCV_E) ||
             (cntrName == CPSS_BadFC_RCV_E) ||
             (cntrName == CPSS_GOOD_PKTS_RCV_E) ||
             (cntrName == CPSS_GOOD_PKTS_SENT_E))
        {
            result = GT_FALSE;;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* CPSS_COLLISIONS_E is the PFC counter.
           CPSS_LATE_COLLISIONS_E is the Unknown FC counter.
           The following counters do NOT exist for 10/20/40 Giga ports in eArch devices. */
        if( (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) &&
            ( (cntrName == CPSS_EXCESSIVE_COLLISIONS_E) ||
              (cntrName == CPSS_MULTIPLE_PKTS_SENT_E)))
        {
            result = GT_FALSE;;
        }
    }
    else
    {
        /* these counters do NOT exist for 10/20/40 Giga ports */
        if( (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) &&
            ( (cntrName == CPSS_EXCESSIVE_COLLISIONS_E) ||
              (cntrName == CPSS_COLLISIONS_E) ||
              (cntrName == CPSS_LATE_COLLISIONS_E) ||
              (cntrName == CPSS_MULTIPLE_PKTS_SENT_E) ||
              (cntrName == CPSS_DEFERRED_PKTS_SENT_E)))
        {
            result = GT_FALSE;;
        }
    }

    return result;
}

/**
* @internal prvCpssDxChPortMacCountersInterlakenChannelGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counters per Interlaken port channel.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortMacCountersInterlakenChannelGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS                   rc; /* return code */
    CPSS_PORT_MAC_COUNTERS_ENT  cntrName;                               /* read the counter name  */
    GT_U32                      ilknMibEntry[PRV_CPSS_ILKN_RX_MIB_COUNTERS_ENTRY_SIZE_CNS]; /* HW data */
    GT_U32                      portMacNum;  /* Port MAC number */
    GT_U32                      regAddr;     /* register address */
    GT_U32                      regValue;    /* register value */
    GT_U32                      portGroupId; /*the port group Id - support multi-port-groups device */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr; /* pointer to mapping shadow DB */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* Get detailed info about ILKN port */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regValue = portMapShadowPtr->portMap.ilknChannel;
    /* Copy channel data to shadow, by configuring <channel select> register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRMIB.MIBChannelSelect;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 6, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRMIB.MIBCountersBase;

    /* read the whole entry */
    rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,
          regAddr, PRV_CPSS_ILKN_RX_MIB_COUNTERS_ENTRY_SIZE_CNS,ilknMibEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (cntrName = 0 ; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E ; cntrName++)
    {
        switch (cntrName)
        {
            case CPSS_GOOD_OCTETS_RCV_E:
                portMacCounterSetArrayPtr->goodOctetsRcv.l[0] = ilknMibEntry[0];
                portMacCounterSetArrayPtr->goodOctetsRcv.l[1] = ilknMibEntry[1];
                break;
            case CPSS_GOOD_UC_PKTS_RCV_E:
                portMacCounterSetArrayPtr->ucPktsRcv.l[0] = ilknMibEntry[2];
                break;
            case CPSS_BRDC_PKTS_RCV_E:
                portMacCounterSetArrayPtr->brdcPktsRcv.l[0] = ilknMibEntry[3];
                break;
            case CPSS_MC_PKTS_RCV_E:
                portMacCounterSetArrayPtr->mcPktsRcv.l[0] = ilknMibEntry[4];
                break;
            case CPSS_PKTS_64_OCTETS_E:
                portMacCounterSetArrayPtr->pkts64Octets.l[0] = ilknMibEntry[5];
                break;
            case CPSS_PKTS_65TO127_OCTETS_E:
                portMacCounterSetArrayPtr->pkts65to127Octets.l[0] = ilknMibEntry[6];
                break;
            case CPSS_PKTS_128TO255_OCTETS_E:
                portMacCounterSetArrayPtr->pkts128to255Octets.l[0] = ilknMibEntry[7];
                break;
            case CPSS_PKTS_256TO511_OCTETS_E:
                portMacCounterSetArrayPtr->pkts256to511Octets.l[0] = ilknMibEntry[8];
                break;
            case CPSS_PKTS_512TO1023_OCTETS_E:
                portMacCounterSetArrayPtr->pkts512to1023Octets.l[0] = ilknMibEntry[9];
                break;
            case CPSS_PKTS_1024TO1518_OCTETS_E:
                portMacCounterSetArrayPtr->pkts1024to1518Octets.l[0] = ilknMibEntry[10];
                break;
            case CPSS_PKTS_1519TOMAX_OCTETS_E:
                portMacCounterSetArrayPtr->pkts1519toMaxOctets.l[0] = ilknMibEntry[11];
                break;
            case CPSS_DROP_EVENTS_E:
                portMacCounterSetArrayPtr->dropEvents.l[0] = ilknMibEntry[12];
                break;
            case CPSS_UNDERSIZE_PKTS_E:
                portMacCounterSetArrayPtr->undersizePkts.l[0] = ilknMibEntry[13];
                break;
            case CPSS_OVERSIZE_PKTS_E:
                portMacCounterSetArrayPtr->oversizePkts.l[0] = ilknMibEntry[14];
                break;
            case CPSS_MAC_RCV_ERROR_E:
                portMacCounterSetArrayPtr->macRcvError.l[0] = ilknMibEntry[15];
                break;

            case CPSS_BAD_OCTETS_RCV_E:
            case CPSS_MAC_TRANSMIT_ERR_E:
            case CPSS_GOOD_PKTS_RCV_E:
            case CPSS_PKTS_1024TOMAX_OCTETS_E:
            case CPSS_GOOD_OCTETS_SENT_E:
            case CPSS_GOOD_PKTS_SENT_E:
            case CPSS_EXCESSIVE_COLLISIONS_E:
            case CPSS_MC_PKTS_SENT_E:
            case CPSS_BRDC_PKTS_SENT_E:
            case CPSS_FC_SENT_E:
            case CPSS_GOOD_FC_RCV_E:
            case CPSS_FRAGMENTS_PKTS_E:
            case CPSS_JABBER_PKTS_E:
            case CPSS_BAD_CRC_E:
            case CPSS_COLLISIONS_E:
            case CPSS_LATE_COLLISIONS_E:
            case CPSS_GOOD_UC_PKTS_SENT_E:
            case CPSS_MULTIPLE_PKTS_SENT_E:
            case CPSS_DEFERRED_PKTS_SENT_E:
            case CPSS_BAD_PKTS_RCV_E:
            default:
                continue;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacCountersCGPortReadMem function
* @endinternal
*
* @brief   Gets all CG mac mib counters from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
* @param[in] newCapture               - whether or not to capture a new counters snapshot
*
* @param[out] cgMacMibsCaptureArr      - (pointer to) array of 64bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note cgMacMibsCaptureArr should be of size CPSS_CG_LAST_E.
*
*/
static GT_STATUS prvCpssDxChPortMacCountersCGPortReadMem
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U64  *cgMacMibsCaptureArr /* should be sized CPSS_CG_LAST_E */
)
{
    GT_U32 *cgMacMibsRegsAddrDBArrPtr;    /* The CG MIB counters addresses */
    GT_U32  cgMacMibsRegsAddrDBLength;   /* The CG MIB counters length */
    GT_U32  i;   /* Iterator */
    GT_U32  regAddr;    /* CG MIB counter register address */
    GT_U32  portMacNum;  /* Mac number assosiated with the portNum */
    GT_U32  portGroupId; /* Group id assosiated with the portNum */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* Mac type assosiated with the portNum */
    CPSS_PORT_INTERFACE_MODE_ENT portInterfaceMode; /* Interface mode assisiated with the portNum */
    GT_STATUS rc = GT_OK;
    GT_BOOL canReadCg;
    /* Arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsCaptureArr);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get port group according to port mac num */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* CG port check */
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    if ( portMacType != PRV_CPSS_PORT_CG_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Non CG mac type");
    }

    /* Interface mode check. CG interface modes are KR4 and SRLR4 */
    portInterfaceMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);

    if (((portInterfaceMode != CPSS_PORT_INTERFACE_MODE_KR4_E && portInterfaceMode != CPSS_PORT_INTERFACE_MODE_CR4_E && portInterfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR4_E)
          ||(portInterfaceMode == CPSS_PORT_INTERFACE_MODE_CR4_E && PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum) == CPSS_PORT_SPEED_40000_E))
          && portMacType == PRV_CPSS_PORT_CG_E)
    {
        /* when trying to read mac counters of CG port while it is down, we want to
           simulate as if we read counters of zeros. */
        for (i=0 ; i<CPSS_CG_LAST_E ; i++)
        {
            cgMacMibsCaptureArr[i].l[0] = 0;
            cgMacMibsCaptureArr[i].l[1] = 0;
        }
        return GT_OK;
    }
    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
    canReadCg = mvHwsCgMac28nmAccessGet(devNum, portGroupId, portMacNum);
    if (canReadCg == GT_FALSE) {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        for (i=0 ; i<CPSS_CG_LAST_E ; i++)
        {
            cgMacMibsCaptureArr[i].l[0] = 0;
            cgMacMibsCaptureArr[i].l[1] = 0;
        }
        return GT_OK;
    }

    /* Capturing a new snapshot of the counters */
    if (newCapture)
    {
        /* CG unit requires a capture command to be performed prior to capturing counters for a port */
        PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portMacNum,&regAddr);
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Failed to perform capture command on CG port");
        }
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0/*offset*/,32/*length*/,0x32/*captureRX, captureTX, clearOnRead*/);
        if (rc!=GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            return rc;
        }
    }

    /* Getting length of MIB regs array size */
    cgMacMibsRegsAddrDBLength = CPSS_CG_LAST_E;
    /* get reference to the CG MIB register addresses array */
    cgMacMibsRegsAddrDBArrPtr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(devNum,portMacNum);
    if (cgMacMibsRegsAddrDBArrPtr == NULL) {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }

    /* Null pointer check */
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsRegsAddrDBArrPtr);

    /* Reading content of all CG MIBS registers */
    for (i=0 ; i<cgMacMibsRegsAddrDBLength ; i++)
    {
        /* Getting mib counter address */
        regAddr = cgMacMibsRegsAddrDBArrPtr[i];

        /* Reading first 32bits */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[0]));
        if ( rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read first CG mib counter entry from address 0x%08x", regAddr);
        }
        /* Reading second 32bits */
        regAddr += 4;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[1]));
        if ( rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read second CG mib counter entry from address 0x%08x", regAddr);
        }
    }
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  -  port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
static GT_STATUS internal_cpssDxChPortMacCountersOnCgPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC  *cgMibStcPtr
)
{
    GT_STATUS rc;
    GT_U64    cgMacMibsCaptureArr[CPSS_CG_LAST_E];   /* Array to load to all CG counters from HW */

    /* Arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cgMibStcPtr);

    /* Read all cg mib counters from HW */
    rc = prvCpssDxChPortMacCountersCGPortReadMem(devNum, portNum, GT_TRUE, &cgMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }

    /* Assign each value from the CG array to the proper field in the CG struct */
    cgMibStcPtr->ifOutOctets = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_OCTETS_E];
    cgMibStcPtr->ifOutMulticastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_MC_PKTS_E];
    cgMibStcPtr->ifOutErrors = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_ERRORS_E];
    cgMibStcPtr->ifOutUcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_UC_PKTS_E];
    cgMibStcPtr->ifOutBroadcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_BC_PKTS_E];
    cgMibStcPtr->ifInOctets = cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E];
    cgMibStcPtr->ifInErrors = cgMacMibsCaptureArr[CPSS_CG_IF_IN_ERRORS_E];
    cgMibStcPtr->ifInBroadcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_BC_PKTS_E];
    cgMibStcPtr->ifInMulticastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_MC_PKTS_E];
    cgMibStcPtr->ifInUcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_UC_PKTS_E];
    cgMibStcPtr->aFramesTransmittedOK = cgMacMibsCaptureArr[CPSS_CG_A_FRAMES_TRANSMITTED_OK_E];
    cgMibStcPtr->aFramesReceivedOK = cgMacMibsCaptureArr[CPSS_CG_A_FRAMES_RECIEVED_OK_E];
    cgMibStcPtr->aFrameCheckSequenceErrors = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
    cgMibStcPtr->aMACControlFramesTransmitted = cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E];
    cgMibStcPtr->aMACControlFramesReceived = cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E];
    cgMibStcPtr->aPAUSEMACCtrlFramesTransmitted = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
    cgMibStcPtr->aPAUSEMACCtrlFramesReceived = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_0 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_1 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_2 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_3 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_4 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_5 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_6 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_7 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_0 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_1 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_2 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_3 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_4 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_5 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_6 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_7 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E];
    cgMibStcPtr->etherStatsOctets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OCTETS_E];
    cgMibStcPtr->etherStatsUndersizePkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E];
    cgMibStcPtr->etherStatsOversizePkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E];
    cgMibStcPtr->etherStatsJabbers = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_JABBERS_E];
    cgMibStcPtr->etherStatsDropEvents = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_DROP_EVENTS_E];
    cgMibStcPtr->etherStatsFragments = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_FRAGMENTS_E];
    cgMibStcPtr->etherStatsPkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PKTS_E];
    cgMibStcPtr->etherStatsPkts64Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E];
    cgMibStcPtr->etherStatsPkts65to127Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E];
    cgMibStcPtr->etherStatsPkts128to255Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E];
    cgMibStcPtr->etherStatsPkts256to511Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E];
    cgMibStcPtr->etherStatsPkts512to1023Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
    cgMibStcPtr->etherStatsPkts1024to1518Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
    cgMibStcPtr->etherStatsPkts1519toMaxOctets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
    cgMibStcPtr->aAlignmentErrors = cgMacMibsCaptureArr[CPSS_CG_A_ALIGNMENT_ERRORS_E];
    cgMibStcPtr->aFrameTooLongErrors = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_TOO_LONG_ERRORS_E];
    cgMibStcPtr->aInRangeLengthErrors = cgMacMibsCaptureArr[CPSS_CG_A_IN_RANGE_LENGTH_ERRORS_E];
    cgMibStcPtr->VLANTransmittedOK = cgMacMibsCaptureArr[CPSS_CG_VLAN_TRANSMITED_OK_E ];
    cgMibStcPtr->VLANRecievedOK = cgMacMibsCaptureArr[CPSS_CG_VLAN_RECIEVED_OK_E];

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  -  port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssDxChPortMacCountersOnCgPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC   *cgMibStcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersOnCgPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cgMibStcPtr));

    rc = internal_cpssDxChPortMacCountersOnCgPortGet(devNum, portNum, cgMibStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cgMibStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMacCountersMTIPortReadMem function
* @endinternal
*
* @brief   Gets all RX/TX MTI MAC MIB counters from HW.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
* @param[in] newCapture               - whether or not to capture a new counters snapshot
* @param[in] rxTxMode                 - RX/TX counter mode
*
* @param[out] mtiMacMibsCaptureArr      - (pointer to) array of 64bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note mtiMacMibsCaptureArr should be of size CPSS_MTI_RX_LAST_E/CPSS_MTI_TX_LAST_E.
*
*/
static GT_STATUS prvCpssDxChPortMacCountersMTIPortReadMem
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 newCapture,
    IN  GT_U32                  rxTxMode,
    OUT GT_U64                  *mtiMacMibsCaptureArr /* should be sized CPSS_MTI_RX_LAST_E/CPSS_MTI_TX_LAST_E */
)
{
    GT_U32 *mtiMacMibsRegsAddrDBArrPtr;      /* The MTI MIB counters addresses */
    GT_U32  mtiMacMibsRegsAddrDBLength;      /* The MTI MIB counters length */
    GT_U32  i;   /* Iterator */
    GT_U32  regAddr, regAddrDataHi;    /* MTI MIB counter register address */
    GT_U32  portMacNum;  /* Mac number assosiated with the portNum */
    GT_U32  portGroupId; /* Group id assosiated with the portNum */
    GT_STATUS rc = GT_OK;
    GT_U32 regValue;
    GT_U32 regMask;

    /* Arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(mtiMacMibsCaptureArr);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get port group according to port mac num */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* MTI port check */
    if (!IS_MTI_MAC_MAC(devNum, portMacNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Non MTI MAC type");
    }

    mtiMacMibsRegsAddrDBLength = (rxTxMode == 0) ? CPSS_MTI_RX_LAST_E : CPSS_MTI_TX_LAST_E;
    for (i=0 ; i<mtiMacMibsRegsAddrDBLength ; i++)
    {
        mtiMacMibsCaptureArr[i].l[0] = 0;
        mtiMacMibsCaptureArr[i].l[1] = 0;
    }

    /* Capturing a new snapshot of the counters */
    if (newCapture)
    {
#if 0
        /* MTI unit requires a capture command to be performed prior to capturing counters for a port */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI_STATISTICS[portMacNum/8].config;
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Failed to perform capture command on MTI port");
        }
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,1/*offset*/,1/*length*/,0x1/*clearOnRead*/);
        if (rc!=GT_OK)
        {
            return rc;
        }
#endif
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI_STATISTICS[portMacNum/8].control;
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Failed to perform capture command on MTI port");
        }
        regValue = 1 << (portMacNum % 8);
        regMask = (GT_U32)(0xFF | (3 << 27) | (3 << 30));    /* Port 0..7 mask set, command TX/RX capture, clear on read */
        if(rxTxMode == 0)
        {
            regValue |=  (1 << 27) | (1 << 30);    /* command RX capture, RX clear on read enable */
        }
        else
        {
            regValue |=  (1 << 28) | (1 << 31);    /* command TX capture, TX clear on read enable */
        }
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId,regAddr,regMask,regValue);
        if (rc!=GT_OK)
        {
            return rc;
        }

    }

    /* get reference to the CG MIB register addresses array */
    mtiMacMibsRegsAddrDBArrPtr = &(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI_STATISTICS[portMacNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[0]);
    regAddrDataHi = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI_STATISTICS[portMacNum/8].dataHiCdc;

    /* Null pointer check */
    CPSS_NULL_PTR_CHECK_MAC(mtiMacMibsRegsAddrDBArrPtr);

    /* Reading content of all MTI MIBS registers */
    for (i=0 ; i<mtiMacMibsRegsAddrDBLength ; i++)
    {
        /* Getting mib counter address */
        regAddr = mtiMacMibsRegsAddrDBArrPtr[i];

        /* Reading LOW 32bits */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(mtiMacMibsCaptureArr[i].l[0]));
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read first MTI mib counter entry from address 0x%08x", regAddr);
        }

        /* Reading HIGH 32bits */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddrDataHi, &(mtiMacMibsCaptureArr[i].l[1]));
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read second MTI mib counter entry from address 0x%08x", regAddrDataHi);
        }
    }

    return GT_OK;
}


static GT_STATUS internal_cpssDxChPortMacCountersOnMtiPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_MTI_COUNTER_SET_STC  *mtiMibStcPtr
)
{
    GT_STATUS rc;
    GT_U64    rxMtiMacMibsCaptureArr[CPSS_MTI_RX_LAST_E];   /* Array to load to all RX MTI counters from HW */
    GT_U64    txMtiMacMibsCaptureArr[CPSS_MTI_TX_LAST_E];   /* Array to load to all TX MTI counters from HW */

    /* Arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(mtiMibStcPtr);

    /* Read all cg mib counters from HW */
    rc = prvCpssDxChPortMacCountersMTIPortReadMem(devNum, portNum, GT_TRUE, 0/*RX MIB*/, &rxMtiMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read RX MTI mibs");
    }

    mtiMibStcPtr->rxCounterStc.etherStatsOctets               = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.ifInOctets                     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_OCTETS_RECEIVED_OK_E];
    mtiMibStcPtr->rxCounterStc.aAlignmentErrors               = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_ALIGNMENT_ERRORS_E];
    mtiMibStcPtr->rxCounterStc.aMACControlFramesReceived      = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E];
    mtiMibStcPtr->rxCounterStc.aFrameTooLongErrors            = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_FRAME_TOO_LONG_ERRORS_E];
    mtiMibStcPtr->rxCounterStc.aInRangeLengthErrors           = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_IN_RANGE_LENGTH_ERRORS_E];
    mtiMibStcPtr->rxCounterStc.aFramesReceivedOK              = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_FRAMES_RECIEVED_OK_E];
    mtiMibStcPtr->rxCounterStc.aFrameCheckSequenceErrors      = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
    mtiMibStcPtr->rxCounterStc.VLANRecievedOK                 = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_VLAN_RECIEVED_OK_E];
    mtiMibStcPtr->rxCounterStc.ifInErrors                     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_ERRORS_E];
    mtiMibStcPtr->rxCounterStc.ifInUcastPkts                  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_UC_PKTS_E];
    mtiMibStcPtr->rxCounterStc.ifInMulticastPkts              = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_MC_PKTS_E];
    mtiMibStcPtr->rxCounterStc.ifInBroadcastPkts              = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_BC_PKTS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsDropEvents           = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_DROP_EVENTS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts                 = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PKTS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsUndersizePkts        = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts64Octets         = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts65to127Octets    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts128to255Octets   = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts256to511Octets   = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts512to1023Octets  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts1024to1518Octets = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsPkts1519toMaxOctets  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsOversizePkts         = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsJabbers              = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_JABBERS_E];
    mtiMibStcPtr->rxCounterStc.etherStatsFragments            = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_FRAGMENTS_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_0     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_1     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_2     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_3     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_4     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_5     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_6     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_7     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_8     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_8_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_9     = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_9_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_10    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_10_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_11    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_11_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_12    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_12_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_13    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_13_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_14    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_14_E];
    mtiMibStcPtr->rxCounterStc.aCBFCPAUSEFramesReceived_15    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_15_E];
    mtiMibStcPtr->rxCounterStc.aPAUSEMACCtrlFramesReceived    = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];

    /* Read all cg mib counters from HW */
    rc = prvCpssDxChPortMacCountersMTIPortReadMem(devNum, portNum, GT_TRUE, 1/*TX MIB*/, &txMtiMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read TX MTI mibs");
    }

    mtiMibStcPtr->txCounterStc.ifOutOctets                    = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_OCTETS_E];
    mtiMibStcPtr->txCounterStc.octetsTransmittedOk            = txMtiMacMibsCaptureArr[CPSS_MTI_TX_OCTETS_TRANSMITTED_OK_E];
    mtiMibStcPtr->txCounterStc.aPAUSEMACCtrlFramesTransmitted = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
    mtiMibStcPtr->txCounterStc.aFramesTransmittedOK           = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_FRAMES_TRANSMITTED_OK_E];
    mtiMibStcPtr->txCounterStc.VLANTransmittedOK              = txMtiMacMibsCaptureArr[CPSS_MTI_TX_VLAN_RECIEVED_OK_E];
    mtiMibStcPtr->txCounterStc.ifOutErrors                    = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_ERRORS_E];
    mtiMibStcPtr->txCounterStc.ifOutUcastPkts                 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_UC_PKTS_E];
    mtiMibStcPtr->txCounterStc.ifOutMulticastPkts             = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_MC_PKTS_E];
    mtiMibStcPtr->txCounterStc.ifOutBroadcastPkts             = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_BC_PKTS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts64Octets         = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_64OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts65to127Octets    = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_65TO127_OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts128to255Octets   = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_128TO255_OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts256to511Octets   = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_256TO511_OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts512to1023Octets  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts1024to1518Octets = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts1519toMaxOctets  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_0  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_1  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_2  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_3  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_4  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_5  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_6  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_7  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_8  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_8_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_9  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_9_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_10 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_10_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_11 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_11_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_12 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_12_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_13 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_13_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_14 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_14_E];
    mtiMibStcPtr->txCounterStc.aCBFCPAUSEFramesTransmitted_15 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_15_E];
    mtiMibStcPtr->txCounterStc.aMACControlFramesTransmitted   = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E];
    mtiMibStcPtr->txCounterStc.etherStatsPkts                 = txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PKTS_E];

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacCountersMtiMibsConvert function
* @endinternal
*
* @brief   Gets MTI ethernet MAC counters and convert to legacy mibs structure.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
* @param[in] newCapture               - whether or not to capture a new snapshot of the counters
*
* @param[out] xlgMibEntry              - (pointer to) array of 32bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChPortMacCountersMtiMibsConvert
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
)
{
    GT_U64  rxMtiMacMibsCaptureArr[CPSS_MTI_RX_LAST_E];   /* Array to load to all MTI counters from HW */
    GT_U64  txMtiMacMibsCaptureArr[CPSS_MTI_TX_LAST_E];   /* Array to load to all MTI counters from HW */
    GT_U64  convertedMtiMacMibCounterEntry;               /* Single counter entry */
    GT_U64  temp64;
    GT_U32  i;                                            /* iterator */
    GT_U32  portMacNum;                                   /* Mac number assosiated with the portNum */
    GT_STATUS rc = GT_OK;                                 /* return code */
    CPSS_PORT_MAC_COUNTERS_ENT cntrName;                  /* Counter names iterator */

    /* Check portNum and get macType */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Reading MTI RX mibs from HW */
    rc = prvCpssDxChPortMacCountersMTIPortReadMem(devNum, portNum, newCapture, 0/*RX MIB*/, &rxMtiMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }

    /* Reading MTI TX mibs from HW */
    rc = prvCpssDxChPortMacCountersMTIPortReadMem(devNum, portNum, newCapture, 1, &txMtiMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }

    /* Converting CG MIBS to regular MIBS xlgMibEntry struct. Not all cg mib counters
       are needed for the legacy mib convention. */
    cpssOsMemSet(xlgMibEntry, 0, sizeof(GT_U32) * PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS);
    /* For all MIB counter */
    for(cntrName = 0; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E; cntrName++)
    {
        convertedMtiMacMibCounterEntry.l[0]=0;
        convertedMtiMacMibCounterEntry.l[1]=0;

        /* Filtering unsupported counters */
        if(!prvCpssDxChPortMacCountersIsSupportedCheck(devNum, portMacNum, cntrName,
                                    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E/*argument relevant for AC3*/))
        {
            continue;
        }

        /* Converting a counter */
        switch (cntrName)
        {
            case CPSS_GOOD_OCTETS_RCV_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E]);
                    temp64 = prvCpssMathAdd64(temp64,rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E]);
                }
                convertedMtiMacMibCounterEntry  = prvCpssMathSub64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_OCTETS_RECEIVED_OK_E], temp64);
                break;
            case CPSS_BAD_OCTETS_RCV_E:
                convertedMtiMacMibCounterEntry = prvCpssMathSub64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_OCTETS_E],
                                                                rxMtiMacMibsCaptureArr[CPSS_MTI_RX_OCTETS_RECEIVED_OK_E]);
                break;
            case CPSS_MAC_TRANSMIT_ERR_E:
                convertedMtiMacMibCounterEntry  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_ERRORS_E];
                break;
            case CPSS_GOOD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BAD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BRDC_PKTS_RCV_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_BC_PKTS_E];
                break;
            case CPSS_MC_PKTS_RCV_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_MC_PKTS_E];
                break;
            case CPSS_PKTS_64_OCTETS_E:
                convertedMtiMacMibCounterEntry  = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E],
                                                                txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_64OCTETS_E]);
                break;
            case CPSS_PKTS_65TO127_OCTETS_E:
                convertedMtiMacMibCounterEntry  = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E],
                                                                txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_65TO127_OCTETS_E]);
                break;
            case CPSS_PKTS_128TO255_OCTETS_E:
                convertedMtiMacMibCounterEntry  = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E],
                                                                txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_128TO255_OCTETS_E]);
                break;
            case CPSS_PKTS_256TO511_OCTETS_E:
                convertedMtiMacMibCounterEntry  = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E],
                                                                txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_256TO511_OCTETS_E]);
                break;
            case CPSS_PKTS_512TO1023_OCTETS_E:
                convertedMtiMacMibCounterEntry  = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E],
                                                                txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_512TO1023_OCTETS_E]);
                break;
            case CPSS_PKTS_1024TOMAX_OCTETS_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                temp64 = prvCpssMathAdd64(rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E],
                                          rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E]);
                temp64 = prvCpssMathAdd64(temp64,
                                          txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E]);
                convertedMtiMacMibCounterEntry = prvCpssMathAdd64(temp64,
                                          txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E]);
                break;
            case CPSS_GOOD_OCTETS_SENT_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E]);
                    temp64 = prvCpssMathAdd64(temp64,txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E]);
                }
                convertedMtiMacMibCounterEntry  = prvCpssMathSub64(txMtiMacMibsCaptureArr[CPSS_MTI_TX_ETHER_STATS_OCTETS_E], temp64);
                break;
            case CPSS_GOOD_PKTS_SENT_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_EXCESSIVE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_MC_PKTS_SENT_E:
                convertedMtiMacMibCounterEntry  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_MC_PKTS_E];
                break;
            case CPSS_BRDC_PKTS_SENT_E:
                convertedMtiMacMibCounterEntry  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_BC_PKTS_E];
                break;
            case CPSS_UNRECOG_MAC_CNTR_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_FC_SENT_E:
                convertedMtiMacMibCounterEntry  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E];
                break;
            case CPSS_GOOD_FC_RCV_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E];
                break;
            case CPSS_DROP_EVENTS_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_DROP_EVENTS_E];
                break;
            case CPSS_UNDERSIZE_PKTS_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E];
                break;
            case CPSS_FRAGMENTS_PKTS_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_FRAGMENTS_E];
                break;
            case CPSS_OVERSIZE_PKTS_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E];
                break;
            case CPSS_JABBER_PKTS_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_ETHER_STATS_JABBERS_E];
                break;
            case CPSS_MAC_RCV_ERROR_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_ERRORS_E];
                break;
            case CPSS_BAD_CRC_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
                break;
            case CPSS_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_LATE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_BadFC_RCV_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_GOOD_UC_PKTS_RCV_E:
                convertedMtiMacMibCounterEntry  = rxMtiMacMibsCaptureArr[CPSS_MTI_RX_IF_IN_UC_PKTS_E];
                break;
            case CPSS_GOOD_UC_PKTS_SENT_E:
                convertedMtiMacMibCounterEntry  = txMtiMacMibsCaptureArr[CPSS_MTI_TX_IF_OUT_UC_PKTS_E];
                break;
            case CPSS_MULTIPLE_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_DEFERRED_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_PKTS_1024TO1518_OCTETS_E:
                /* INTERLAKEN */
                /*convertedMtiMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
                break;*/
                continue;
            case CPSS_PKTS_1519TOMAX_OCTETS_E:
                /* INTERLAKEN */
                /*convertedMtiMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
                break;*/
                continue;
            default:
                continue;
        }

        /* Assigning the converted MTI MIB counter to the regular struct array */
        xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/ 4] = convertedMtiMacMibCounterEntry .l[0];

        if ( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
                (cntrName == CPSS_GOOD_OCTETS_RCV_E) ||
                      (cntrName == CPSS_GOOD_OCTETS_SENT_E) )
        {
            xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 + 1] = convertedMtiMacMibCounterEntry.l[1];
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersOnMtiPortGet function
* @endinternal
*
* @brief   Gets the MTI mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  -  port number
*
* @param[out] mtiMibStcPtr            - (pointer to) MTI mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssDxChPortMacCountersOnMtiPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_PORT_MAC_MTI_COUNTER_SET_STC   *mtiMibStcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersOnMtiPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mtiMibStcPtr));

    rc = internal_cpssDxChPortMacCountersOnMtiPortGet(devNum, portNum, mtiMibStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mtiMibStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortMacCountersCGMibsConvert function
* @endinternal
*
* @brief   Gets CG ethernet MAC counters and convert to legacy mibs structure.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
* @param[in] newCapture               - whether or not to capture a new snapshot of the counters
*
* @param[out] xlgMibEntry              - (pointer to) array of 32bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChPortMacCountersCGMibsConvert
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
)
{
    GT_U64  cgMacMibsCaptureArr[CPSS_CG_LAST_E];   /* Array to load to all CG counters from HW */
    GT_U64  covertedCgMacMibCounterEntry ;   /* Single counter entry */
    GT_U64  temp64;
    GT_U32  i; /* iterator */
    GT_U32  portMacNum; /* Mac number assosiated with the portNum */
    GT_STATUS rc = GT_OK;
    CPSS_PORT_MAC_COUNTERS_ENT cntrName;    /* Counter names iterator */

    /* Check portNum and get macType */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Reading CG mibs from HW */
    rc = prvCpssDxChPortMacCountersCGPortReadMem(devNum, portNum, newCapture, &cgMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }

    /* Converting CG MIBS to regular MIBS xlgMibEntry struct. Not all cg mib counters
       are needed for the legacy mib convention. */
    cpssOsMemSet(xlgMibEntry, 0, sizeof(GT_U32) * PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS);
    /* For all MIB counter */
    for(cntrName = 0; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E; cntrName++)
    {
        covertedCgMacMibCounterEntry.l[0]=0;
        covertedCgMacMibCounterEntry.l[1]=0;

        /* Filtering unsupported counters */
        if(!prvCpssDxChPortMacCountersIsSupportedCheck(devNum, portMacNum, cntrName,
                                    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E/*argument relevant for AC3*/))
        {
            continue;
        }

        /* Converting a counter */
        switch (cntrName)
        {
            case CPSS_GOOD_OCTETS_RCV_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E], temp64);
                break;
            case CPSS_BAD_OCTETS_RCV_E:
                covertedCgMacMibCounterEntry = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OCTETS_E],
                                                                cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E]);
                break;
            case CPSS_MAC_TRANSMIT_ERR_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_ERRORS_E];
                break;
            case CPSS_GOOD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BAD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BRDC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_BC_PKTS_E];
                break;
            case CPSS_MC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_MC_PKTS_E];
                break;
            case CPSS_PKTS_64_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E];
                break;
            case CPSS_PKTS_65TO127_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E];
                break;
            case CPSS_PKTS_128TO255_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E];
                break;
            case CPSS_PKTS_256TO511_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E];
                break;
            case CPSS_PKTS_512TO1023_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
                break;
            case CPSS_PKTS_1024TOMAX_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = prvCpssMathAdd64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E],
                            cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E]);
                break;
            case CPSS_GOOD_OCTETS_SENT_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_OUT_OCTETS_E], temp64);
                break;
            case CPSS_GOOD_PKTS_SENT_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_EXCESSIVE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_MC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_MC_PKTS_E];
                break;
            case CPSS_BRDC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_BC_PKTS_E];
                break;
            case CPSS_UNRECOG_MAC_CNTR_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_FC_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
                break;
            case CPSS_GOOD_FC_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];
                break;
            case CPSS_DROP_EVENTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_DROP_EVENTS_E];
                break;
            case CPSS_UNDERSIZE_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E];
                break;
            case CPSS_FRAGMENTS_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_FRAGMENTS_E];
                break;
            case CPSS_OVERSIZE_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E];
                break;
            case CPSS_JABBER_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_JABBERS_E];
                break;
            case CPSS_MAC_RCV_ERROR_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_ERRORS_E];
                break;
            case CPSS_BAD_CRC_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
                break;
            case CPSS_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_LATE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_BadFC_RCV_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_GOOD_UC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_UC_PKTS_E];
                break;
            case CPSS_GOOD_UC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_UC_PKTS_E];
                break;
            case CPSS_MULTIPLE_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_DEFERRED_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_PKTS_1024TO1518_OCTETS_E:
                /* INTERLAKEN */
                /*covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
                break;*/
                continue;
            case CPSS_PKTS_1519TOMAX_OCTETS_E:
                /* INTERLAKEN */
                /*covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
                break;*/
                continue;
            default:
                continue;
        }

        /* Assigning the converted CG MIB counter to the regular struct array */
        xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/ 4] = covertedCgMacMibCounterEntry .l[0];

        if ( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
                (cntrName == CPSS_GOOD_OCTETS_RCV_E) ||
                      (cntrName == CPSS_GOOD_OCTETS_SENT_E) )
        {
            xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 + 1] = covertedCgMacMibCounterEntry .l[1];
        }
    }
    return GT_OK;
}
/**
* @internal prvCpssDxChPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number,
*                                      CPU port if getFromCapture is GT_FALSE
* @param[in] getFromCapture           -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
* @param[in] readMode                 - read counters procedure mode
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvCpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         getFromCapture,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN  PRV_DXCH_PORT_MAC_CNTR_READ_MODE_ENT readMode
)
{
    GT_U64                     cntrValue[CPSS_LAST_MAC_COUNTER_NUM_E];
    CPSS_PORT_MAC_COUNTERS_ENT cntrName;  /* read the counter name  */
    GT_STATUS                  rc;
    GT_U32  xlgMibEntry[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32  portMacNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr; /* pointer to mapping shadow DB */
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue = 0;    /* HW value */
    GT_U32  regValue1 = 0;    /* HW value */
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode = 0;
    GT_BOOL isCgMac = GT_FALSE;
    GT_BOOL isMtiMac = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(getFromCapture == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    }

    if (readMode == PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E)
    {
        CPSS_NULL_PTR_CHECK_MAC(portMacCounterSetArrayPtr);
        cpssOsMemSet(portMacCounterSetArrayPtr, 0, sizeof(CPSS_PORT_MAC_COUNTER_SET_STC));
    }


    /* reset counter value */
    cpssOsMemSet(cntrValue, 0, sizeof(GT_U64)*CPSS_LAST_MAC_COUNTER_NUM_E);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    xlgMibShadowPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];

    isCgMac = (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E);
    isMtiMac = IS_MTI_MAC_MAC(devNum, portMacNum);

    /********************************/
    /* INTERLAKEN Port MIB Counters */
    /********************************/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Get detailed info about ILKN port */
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if((portMapShadowPtr->valid == GT_TRUE) && (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E))
        {
            if (readMode != PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E)
            {
                /* ILNK channels do not need Ethernet MAC related WA. */
                return GT_OK;
            }

            rc = prvCpssDxChPortMacCountersInterlakenChannelGet(devNum, portNum, portMacCounterSetArrayPtr);
            return rc;
        }
    }

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        /* If we want to read values from the capture snapchat taken in a certain time */
        if(getFromCapture == GT_TRUE)
        {
            cpssOsMemCpy(xlgMibEntry,xlgMibShadowPtr->captureMibShadow,sizeof(xlgMibEntry));
        }
        /* If we want to get the current values */
        else
        {
            if(isCgMac)
            {
                /* CG MAC MIB counters need to be converted to the regular structure */
                rc = prvCpssDxChPortMacCountersCGMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
            }
            else if(isMtiMac)
            {
                /* MTI MAC MIB counters need to be converted to the regular structure */
                rc = prvCpssDxChPortMacCountersMtiMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
            }
            else
            {
                /* read the whole entry */
                rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,
                      PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters,
                      PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum),xlgMibEntry);
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            /* If the read mode is to reset the hardware counters */
            if (readMode == PRV_DXCH_PORT_MAC_CNTR_READ_MODE_RESET_HW_E)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    GT_BOOL                 enable;
                    CPSS_DXCH_PORT_COMBO_PARAMS_STC comboParams;

                    rc = cpssDxChPortComboModeEnableGet(devNum, portNum, &enable, &comboParams);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    if(enable)
                    {
                        GT_U32  localPort;
                        GT_U32  cntrsAddr;
                        GT_U32  i;

                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
                        for(i = 0; i < CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS; i++)
                        {
                            if(CPSS_DXCH_PORT_COMBO_NA_MAC_CNS == comboParams.macArray[i].macNum)
                                break;

                            if((localPort == comboParams.macArray[i].macNum) &&
                                (portGroupId == comboParams.macArray[i].macPortGroupNum))
                            {
                                continue;
                            }

                            cntrsAddr = (PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters
                                            & 0xF000000) + (comboParams.macArray[i].macNum * 0x20000);
                            rc = prvCpssDrvHwPpPortGroupReadRam(devNum,
                                        comboParams.macArray[i].macPortGroupNum,
                                        cntrsAddr,
                                        PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum),
                                        xlgMibEntry);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }

                        }/* for(i = 0; */

                    }/* if combo port */

                }/* if lion2 */

                return GT_OK;
            } /* if readMode == PRV_DXCH_PORT_MAC_CNTR_READ_MODE_RESET_HW_E */

            /* We got the current hw entry values. We now need to update the shadow with
               those values. Capture remains as is */
            prvCpssDxChPortSumMibEntry(devNum,xlgMibEntry,xlgMibShadowPtr->mibShadow);
        } /* else -> if(getFromCapture == GT_TRUE) */
    } /* if(getFromCapture == GT_TRUE) */

    /* By here, we read the whole mib counters entry of a port, (either from the HW or
       from the software shadow), We will now decompose the entry (xlgMibEntry)
       for proper 64bit entryies and update output array to whats needed*/
    if (readMode == PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E)
    {
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_FC_SENT_COUNTERS_PROBLEM_WHEN_FCU_BYPASS_DISABLE_WA_E))
        {
            PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                     &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                     &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

                regAddr = fcaStcPtr->sentPktsCntrLSb;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).sentPktsCntrLSb;
            }
            rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                     &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                     &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

                regAddr = fcaStcPtr->sentPktsCntrMSb;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).sentPktsCntrMSb;
            }
            rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &regValue1);
            if (rc != GT_OK)
            {
                return rc;
            }

            regValue |= (regValue1 << 16);
        }

        if(isCgMac || isMtiMac)
        {
            /* no support for those modes */
            /* actual counting is ALWAYS '1024-1518,1519-max' */
            counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
        }
#ifdef GM_USED
        else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* GM of Falcon use XLG MAC Type but it support only one mode - NORMAL */
            counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
        }
#endif
        else
        /* get counterMode from register. The configuration does not exist for CPU MAC */
        if( ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) && (portMacNum != CPSS_CPU_PORT_NUM_CNS))  ||
            PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            rc = cpssDxChPortMacOversizedPacketsCounterModeGet(/*IN*/devNum,  /*IN*/portNum, /*OUT*/&counterMode);
            if (rc != GT_OK )
            {
                return rc;
            }
        }

        /* get all mac counters */
        for(cntrName = 0; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E; cntrName++)
        {
            if(!prvCpssDxChPortMacCountersIsSupportedCheck(/*IN*/devNum, /*IN*/portMacNum, /*IN*/cntrName, /*IN*/counterMode))
            {
                continue;
            }

            if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
            {
                /*The oversized packets counter mode determines the behavior of the CPSS_PKTS_1024TOMAX_OCTETS_E & CPSS_DEFERRED_PKTS_SENT_E counters
*               and influence on CPSS_PKTS_1024TO1518_OCTETS_E and CPSS_PKTS_1519TOMAX_OCTETS_E counters.*/
                if (((counterMode == CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E) || (counterMode == CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E))
                      && cntrName ==CPSS_PKTS_1024TO1518_OCTETS_E )
                {
                    cntrValue[cntrName].l[0] = xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[CPSS_DEFERRED_PKTS_SENT_E] / 4];
                }
                else if (((counterMode == CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E) || (counterMode == CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E))
                          && cntrName ==CPSS_PKTS_1519TOMAX_OCTETS_E )
                {
                    cntrValue[cntrName].l[0] = xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[CPSS_PKTS_1024TOMAX_OCTETS_E] / 4];
                }
                else
                {
                    cntrValue[cntrName].l[0] = xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName] / 4];
                }

                if ( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
                      (cntrName == CPSS_GOOD_OCTETS_RCV_E) ||
                        (cntrName == CPSS_GOOD_OCTETS_SENT_E))
                {
                    cntrValue[cntrName].l[1] = xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 + 1];
                }
            }
            else
            {
                /* get value for counter i */
                if(getFromCapture == GT_TRUE)
                {
                    rc = cpssDxChPortMacCounterCaptureGet(devNum, portNum, cntrName,
                                                          &cntrValue[cntrName]);
                }
                else
                {
                    rc = cpssDxChMacCounterGet(devNum, portNum, cntrName,
                                                &cntrValue[cntrName]);
                }
                if (rc != GT_OK)
                    return rc;
            }
        }

        for (cntrName = 0 ; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E ; cntrName++)
        {
            if(!prvCpssDxChPortMacCountersIsSupportedCheck(/*IN*/devNum, /*IN*/portMacNum, /*IN*/cntrName, /*IN*/counterMode))
            {
                continue;
            }
            switch (cntrName)
            {
            case CPSS_GOOD_OCTETS_RCV_E:
                    portMacCounterSetArrayPtr->goodOctetsRcv = cntrValue[cntrName];
                    break;
                case CPSS_BAD_OCTETS_RCV_E:
                    portMacCounterSetArrayPtr->badOctetsRcv = cntrValue[cntrName];
                    break;
                case CPSS_MAC_TRANSMIT_ERR_E:
                    portMacCounterSetArrayPtr->macTransmitErr = cntrValue[cntrName];
                    break;
                /* This counter is for CPU port only */
                case CPSS_GOOD_PKTS_RCV_E:
                    portMacCounterSetArrayPtr->goodPktsRcv = cntrValue[cntrName];
                    break;
                case CPSS_BRDC_PKTS_RCV_E:
                    portMacCounterSetArrayPtr->brdcPktsRcv = cntrValue[cntrName];
                    break;
                case CPSS_MC_PKTS_RCV_E:
                    portMacCounterSetArrayPtr->mcPktsRcv = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_64_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts64Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_65TO127_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts65to127Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_128TO255_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts128to255Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_256TO511_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts256to511Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_512TO1023_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts512to1023Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_1024TOMAX_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts1024tomaxOoctets = cntrValue[cntrName];
                    break;
                case CPSS_GOOD_OCTETS_SENT_E:
                    portMacCounterSetArrayPtr->goodOctetsSent = cntrValue[cntrName];
                    break;
                /* This counter is for CPU port only */
                case CPSS_GOOD_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->goodPktsSent = cntrValue[cntrName];
                    break;
                case CPSS_EXCESSIVE_COLLISIONS_E:
                    portMacCounterSetArrayPtr->excessiveCollisions = cntrValue[cntrName];
                    break;
                case CPSS_MC_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->mcPktsSent = cntrValue[cntrName];
                    break;
                case CPSS_BRDC_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->brdcPktsSent = cntrValue[cntrName];
                    break;
                case CPSS_FC_SENT_E:
                    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
                        (PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum) == PRV_CPSS_GE_PORT_GE_ONLY_E))
                    {
                        portMacCounterSetArrayPtr->fcSent = cntrValue[cntrName];
                    }
                    else
                    {
                        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_FC_SENT_COUNTERS_PROBLEM_WHEN_FCU_BYPASS_DISABLE_WA_E))
                        {
                            cntrValue[cntrName].l[0] += regValue;
                        }

                        portMacCounterSetArrayPtr->fcSent = cntrValue[cntrName];
                    }

                    break;
                case CPSS_GOOD_FC_RCV_E:
                    portMacCounterSetArrayPtr->goodFcRcv = cntrValue[cntrName];
                    break;
                case CPSS_DROP_EVENTS_E:
                    /* for Twist-D-XG, GT_DROP_EVENTS */
                    /*offset represent BadFCReceived */
                    portMacCounterSetArrayPtr->dropEvents = cntrValue[cntrName];
                    break;
                case CPSS_UNDERSIZE_PKTS_E:
                    portMacCounterSetArrayPtr->undersizePkts = cntrValue[cntrName];
                    break;
                case CPSS_FRAGMENTS_PKTS_E:
                    portMacCounterSetArrayPtr->fragmentsPkts = cntrValue[cntrName];
                    break;
                case CPSS_OVERSIZE_PKTS_E:
                    portMacCounterSetArrayPtr->oversizePkts = cntrValue[cntrName];
                    break;
                case CPSS_JABBER_PKTS_E:
                    portMacCounterSetArrayPtr->jabberPkts = cntrValue[cntrName];
                    break;
                case CPSS_MAC_RCV_ERROR_E:
                    portMacCounterSetArrayPtr->macRcvError = cntrValue[cntrName];
                    break;
                case CPSS_BAD_CRC_E:
                    portMacCounterSetArrayPtr->badCrc = cntrValue[cntrName];
                    break;
                case CPSS_COLLISIONS_E:
                    portMacCounterSetArrayPtr->collisions = cntrValue[cntrName];
                    break;
                case CPSS_LATE_COLLISIONS_E:
                    portMacCounterSetArrayPtr->lateCollisions = cntrValue[cntrName];
                    break;
                case CPSS_GOOD_UC_PKTS_RCV_E:
                    portMacCounterSetArrayPtr->ucPktsRcv = cntrValue[cntrName];
                    break;
                case CPSS_GOOD_UC_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->ucPktsSent = cntrValue[cntrName];
                    break;
                case CPSS_MULTIPLE_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->multiplePktsSent = cntrValue[cntrName];
                    break;
                case CPSS_DEFERRED_PKTS_SENT_E:
                    portMacCounterSetArrayPtr->deferredPktsSent = cntrValue[cntrName];
                    break;
                /* This counter is for CPU port only */
                case CPSS_BAD_PKTS_RCV_E:
                    portMacCounterSetArrayPtr->badPktsRcv = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_1024TO1518_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts1024to1518Octets = cntrValue[cntrName];
                    break;
                case CPSS_PKTS_1519TOMAX_OCTETS_E:
                    portMacCounterSetArrayPtr->pkts1519toMaxOctets = cntrValue[cntrName];
                    break;
                default:
                    break;

            }
        }
    }

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        if ((xlgMibShadowPtr->clearOnReadEnable == GT_TRUE) &&
            (readMode != PRV_DXCH_PORT_MAC_CNTR_READ_MODE_UPDATE_SHADOW_E))
        {
            cpssOsMemSet(xlgMibShadowPtr->mibShadow, 0, sizeof(xlgMibShadowPtr->mibShadow));
        }
        else
        {
            cpssOsMemCpy(xlgMibShadowPtr->mibShadow,xlgMibEntry,sizeof(xlgMibShadowPtr->mibShadow));
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacCountersSpecialShadowReset function
* @endinternal
*
* @brief   Reset Special Mib conters packet in Shadow to work arround hardware error.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacCountersSpecialShadowReset
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32  portMacNum;
    CPSS_PORT_MAC_COUNTERS_ENT cntrName;  /* read the counter name  */
    GT_BOOL tobeReset = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    xlgMibShadowPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];
    for (cntrName = 0 ; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E ; cntrName++)
    {
            switch (cntrName)
            {
               case CPSS_PKTS_64_OCTETS_E:
                    if(xlgMibShadowPtr->mibShadow[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] != (GT_U32)1)
                    {
                        tobeReset = GT_FALSE;
                    }
                    break;
                case CPSS_GOOD_OCTETS_SENT_E:
                    if(xlgMibShadowPtr->mibShadow[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] != (GT_U32)64)
                    {
                        tobeReset = GT_FALSE;
                    }
                    break;
                case CPSS_MC_PKTS_SENT_E:
                    if(xlgMibShadowPtr->mibShadow[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] != (GT_U32)1)
                    {
                        tobeReset = GT_FALSE;
                    }
                    break;
                case CPSS_GOOD_OCTETS_RCV_E:
                case CPSS_BAD_OCTETS_RCV_E:
                case CPSS_MAC_TRANSMIT_ERR_E:
                case CPSS_BRDC_PKTS_RCV_E:
                case CPSS_MC_PKTS_RCV_E:
                case CPSS_PKTS_65TO127_OCTETS_E:
                case CPSS_PKTS_128TO255_OCTETS_E:
                case CPSS_PKTS_256TO511_OCTETS_E:
                case CPSS_PKTS_512TO1023_OCTETS_E:
                case CPSS_PKTS_1024TOMAX_OCTETS_E:
                case CPSS_EXCESSIVE_COLLISIONS_E:
                case CPSS_BRDC_PKTS_SENT_E:
                case CPSS_FC_SENT_E:
                case CPSS_GOOD_FC_RCV_E:
                case CPSS_DROP_EVENTS_E:
                case CPSS_UNDERSIZE_PKTS_E:
                case CPSS_FRAGMENTS_PKTS_E:
                case CPSS_OVERSIZE_PKTS_E:
                case CPSS_JABBER_PKTS_E:
                case CPSS_MAC_RCV_ERROR_E:
                case CPSS_BAD_CRC_E:
                case CPSS_COLLISIONS_E:
                case CPSS_LATE_COLLISIONS_E:
                case CPSS_BadFC_RCV_E:
                case CPSS_GOOD_UC_PKTS_RCV_E:
                case CPSS_GOOD_UC_PKTS_SENT_E:
                case CPSS_MULTIPLE_PKTS_SENT_E:
                case CPSS_DEFERRED_PKTS_SENT_E:
                    if(xlgMibShadowPtr->mibShadow[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] != (GT_U32)0)
                    {
                        tobeReset = GT_FALSE;
                    }
                    break;
                default:
                    break;
            }
            if(GT_FALSE == tobeReset)
            {
                break;
            }
    }
    if(GT_TRUE == tobeReset)
    {
        for (cntrName = 0 ; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E ; cntrName++)
        {
            xlgMibShadowPtr->mibShadow[cntrName] = (GT_U32)0;
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       goodPktsSent, macTransmitErr, goodOctetsSent, dropEvents,
*       goodPktsRcv, badPktsRcv, goodOctetsRcv, badOctetsRcv.
*
*/
static GT_STATUS internal_cpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS   rc;
    GT_U32  portMacNum;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacCounterSetArrayPtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersOnPortGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersOnPortGet(devNum,portNum,
                                  portMacCounterSetArrayPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    return prvCpssDxChPortMacCountersOnPortGet(devNum,portNum, GT_FALSE,
                                               portMacCounterSetArrayPtr,
                                               PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E);
}

/**
* @internal cpssDxChPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       goodPktsSent, macTransmitErr, goodOctetsSent, dropEvents,
*       goodPktsRcv, badPktsRcv, goodOctetsRcv, badOctetsRcv.
*
*/
GT_STATUS   cpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersOnPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacCounterSetArrayPtr));

    rc = internal_cpssDxChPortMacCountersOnPortGet(devNum, portNum, portMacCounterSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacCounterSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Potrs:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*
*/
static GT_STATUS internal_cpssDxChPortMacCounterCaptureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS   rc;
    GT_U32  portMacNum;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    if((GT_U32)cntrName >= (GT_U32)CPSS_LAST_MAC_COUNTER_NUM_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCounterCaptureGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCounterCaptureGet(devNum,portNum,
                                  cntrName,cntrValuePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    return prvCpssDxChMacCounterGet(devNum, portNum, cntrName,
                                    GT_TRUE, cntrValuePtr);
}

/**
* @internal cpssDxChPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Potrs:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*
*/
GT_STATUS cpssDxChPortMacCounterCaptureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCounterCaptureGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cntrName, cntrValuePtr));

    rc = internal_cpssDxChPortMacCounterCaptureGet(devNum, portNum, cntrName, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cntrName, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
static GT_STATUS internal_cpssDxChPortMacCountersCaptureOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS   rc;
    GT_U32  portMacNum;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacCounterSetArrayPtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureOnPortGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureOnPortGet(devNum,portNum,
                                  portMacCounterSetArrayPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    return prvCpssDxChPortMacCountersOnPortGet(devNum,portNum, GT_TRUE,
                                               portMacCounterSetArrayPtr,
                                               PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E);
}

/**
* @internal cpssDxChPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS   cpssDxChPortMacCountersCaptureOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersCaptureOnPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacCounterSetArrayPtr));

    rc = internal_cpssDxChPortMacCountersCaptureOnPortGet(devNum, portNum, portMacCounterSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacCounterSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
static GT_STATUS internal_cpssDxChPortMacCountersCaptureTriggerSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_U32      regAddr; /* register address to set */
    GT_U32      value;   /* register field value */
    GT_U32      group;   /* Port's group (GOP) */
    GT_U32      offset;  /* bit's offset */
    GT_STATUS   rc;      /* return status */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 useAsGePort;/* does this port uses the GE port mechanism or the XG mechanism */
    GT_U32 xlgMibEntry[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32          portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_BOOL isMtiMac;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureTriggerSet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureTriggerSet(devNum,portNum,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(devNum, portNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);


    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        isMtiMac = IS_MTI_MAC_MAC(devNum, portMacNum);
        if(isMtiMac)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "function not supported/not implemented by the MTI mac ");
        }

        xlgMibShadowPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];
        /* read the whole entry */
        rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,
              PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters,
              PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum),xlgMibEntry);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvCpssDxChPortSumMibEntry(devNum,xlgMibShadowPtr->mibShadow,xlgMibEntry);
        cpssOsMemCpy(xlgMibShadowPtr->captureMibShadow,xlgMibShadowPtr->mibShadow,sizeof(xlgMibShadowPtr->captureMibShadow));

        return GT_OK;
    }
    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);

    /* flex ports uses counters like the XG ports */
    if (useAsGePort)
    {
        group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;


        offset = 0;


    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            switch(portNum){
                case 24:
                    offset = 3;
                    break;
                case 25:
                    offset = 4;
                    break;
                case 26:
                    offset = 5;
                    break;
                case 27:
                    offset = 20;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

           regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr;

        }
        /* Cheetah3 and above */
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            offset = 0;
        }
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,
            regAddr,offset,1,&value);

    if(rc != GT_OK)
    {
        return rc;
    }
    /* Previous capture operation isn't finished.*/
    if(value != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }


    if (useAsGePort)
    {
        /* Set capture port. */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                regAddr,1,3,(portNum % 6));

        if(rc != GT_OK)
        {
            return rc;
        }

    }

    /* Set cature trigger. */
    return prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
            regAddr,offset,1,1);

}

/**
* @internal cpssDxChPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS cpssDxChPortMacCountersCaptureTriggerSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersCaptureTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPortMacCountersCaptureTriggerSet(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      - GT_TRUE  - capture is done.
*                                      - GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
static GT_STATUS internal_cpssDxChPortMacCountersCaptureTriggerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL  *captureIsDonePtr
)
{
    GT_U32      regAddr; /* register address to set */
    GT_U32      group;   /* Port's group (GOP) */
    GT_U32      offset;  /* bit's offset */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    GT_U32 useAsGePort;/* does this port uses the GE port mechanism or the XG mechanism */
    GT_U32          portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(captureIsDonePtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureTriggerGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersCaptureTriggerGet(devNum,portNum,
                                  captureIsDonePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        *captureIsDonePtr = GT_TRUE;
        return GT_OK;
    }

    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);

    if(useAsGePort)
    {
        group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;


        offset = 0;


    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            switch(portNum){
                case 24:
                    offset = 3;
                    break;
                case 25:
                    offset = 4;
                    break;
                case 26:
                    offset = 5;
                    break;
                case 27:
                    offset = 20;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

           regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr;

        }
        /* Cheetah3 and above */
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            offset = 0;
        }

    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr,offset,1,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *captureIsDonePtr = (value == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      - GT_TRUE  - capture is done.
*                                      - GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS cpssDxChPortMacCountersCaptureTriggerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL  *captureIsDonePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersCaptureTriggerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, captureIsDonePtr));

    rc = internal_cpssDxChPortMacCountersCaptureTriggerGet(devNum, portNum, captureIsDonePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, captureIsDonePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChSip5PortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue (APPLICABLE RANGES: 0..7),
*                                      if corresponding bit in setModeBmp is 1.
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
static GT_STATUS prvCpssDxChSip5PortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PORT_NUM                     portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 regAddr1;        /* register address */
    GT_U32 value;           /* register value*/
    GT_U32 value1;          /* register value*/
    GT_U32 dxDpLevel;       /* DP level value */
    GT_U32 tcMask;          /* TC mask - different for SIP5 and SIP6 */
    GT_U32 dpOffset;        /* DP Offset - different for SIP5 and SIP6 */


    if (cntrSetNum > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* configuration for egress MIB counters in PREQ unit */
        regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
            egrMIBCntrs.egrMIBCntrsSetConfig[cntrSetNum];
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
            egrMIBCntrs.egrMIBCntrsPortSetConfig[cntrSetNum];
        tcMask   = 0x7; /* preq unit supports 4 bits but the EFT only 3 bits */
        dpOffset = 30;
        /* check VLAN and TC to match the fields of both    */
        /* PREQ and EGF_EFT registers (bounds are different)*/
        if (setModeBmp & CPSS_EGRESS_CNT_VLAN_E)
        {
            if (vlanId > 0x1FFF)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
        {
            /* EGF unit supports only 3 bits (not 4) */
            PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
        }
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
            egrMIBCntrs.txQMIBCntrsSetConfig[cntrSetNum];
        regAddr1 = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
            egrMIBCntrs.txQMIBCntrsPortSetConfig[cntrSetNum];
        tcMask   = 0x7;
        dpOffset = 29;
        if (setModeBmp & CPSS_EGRESS_CNT_VLAN_E)
        {
            PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, vlanId);
        }
        if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
        {
            PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
        }
    }

    dxDpLevel = 0;
    if (setModeBmp & CPSS_EGRESS_CNT_DP_E)
    {
        PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(dpLevel, dxDpLevel);
    }
    if (setModeBmp & CPSS_EGRESS_CNT_PORT_E)
    {
        /* This is ePort and not physical port */
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);
    }

    value = setModeBmp & 0xE;  /* vlan, tc, dp */
    value1 = setModeBmp & 0x1; /* port */

    if (setModeBmp & CPSS_EGRESS_CNT_VLAN_E)
    {
        /* write the port monitored by set */
        value |= (vlanId & 0xFFFF) << 10;
    }
    if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
    {
        /* write the port monitored by set */
        value |= (tc & tcMask) << 26;
    }
    if (setModeBmp & CPSS_EGRESS_CNT_DP_E)
    {
        /* write the port monitored by set */
        value |= dxDpLevel << dpOffset;
    }

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (setModeBmp & CPSS_EGRESS_CNT_PORT_E)
    {
        value1 |= portNum << 4;
    }
    rc = prvCpssHwPpWriteRegister(devNum, regAddr1, value1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* save the info for 'get' configuration */
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
        portEgressCntrModeInfo[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        return GT_OK;
    }

    /* additional configuration for egress MIB counters in EGF_EFT unit */
    /* values already checked upper                                     */

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.
        egrMIBCntrs.egrMIBCntrsSetConfig[cntrSetNum];
    regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.
        egrMIBCntrs.egrMIBCntrsPortSetConfig[cntrSetNum];
    value = (setModeBmp >> 1) & 0x7;  /* vlan, tc, dp */
    value1 = setModeBmp & 0x1; /* port */

    if (setModeBmp & CPSS_EGRESS_CNT_VLAN_E)
    {
        /* write the port monitored by set */
        value |= (vlanId & 0x1FFF) << 3;
    }
    if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
    {
        /* write the port monitored by set */
        value |= (tc & 0x7) << 16;
    }
    if (setModeBmp & CPSS_EGRESS_CNT_DP_E)
    {
        PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
            devNum, dpLevel, dxDpLevel);

        /* write the port monitored by set */
        value |= dxDpLevel << 19;
    }
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (setModeBmp & CPSS_EGRESS_CNT_PORT_E)
    {
        value1 |= portNum << 1;
    }
    rc = prvCpssHwPpWriteRegister(devNum, regAddr1, value1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue (0..7),
*                                      if corresponding bit in setModeBmp is 1.
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
static GT_STATUS internal_cpssDxChPortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 dxDpLevel;/* DP level value */
    GT_U32 value;/* register value*/
    GT_U32  portPortGroupId; /*the port group Id for 'port mode' */
    GT_U32  portValue;/* tmp register value in 'port mode' */
    GT_U32  i;                /* iterator */
    GT_U32  portHwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* only 2 sets supported for now */
    if(cntrSetNum > 1)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(setModeBmp &~ (
                CPSS_EGRESS_CNT_PORT_E |
                CPSS_EGRESS_CNT_VLAN_E |
                CPSS_EGRESS_CNT_TC_E   |
                CPSS_EGRESS_CNT_DP_E
                ))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChSip5PortEgressCntrModeSet(devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel);
    }
    else
    {

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].txQConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        txqVer1.queue.peripheralAccess.egressMibCounterSet.config[cntrSetNum];
        }

        value = setModeBmp & 0xF;

        if (setModeBmp & CPSS_EGRESS_CNT_VLAN_E)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(vlanId);

            /* write the port monitored by set */
            value |= (vlanId & 0xFFF) << 10;
        }
        if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
        {
            PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

            /* write the port monitored by set */
            value |= (tc & 0x7) << 22;
        }
        if (setModeBmp & CPSS_EGRESS_CNT_DP_E)
        {
            PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
                devNum, dpLevel, dxDpLevel);

            /* write the port monitored by set */
            value |= dxDpLevel << 25;
        }

        if (setModeBmp & CPSS_EGRESS_CNT_PORT_E)
        {
            PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portPortGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
            {
                /* write the port monitored by set */
                portValue =  (portNum & 0x3F) << 4;

                rc = prvCpssHwPpPortGroupWriteRegister(devNum, portPortGroupId,regAddr,
                        (value | portValue));

                if(rc != GT_OK)
                {
                    return rc;
                }

            }
            else
            {
                if(portNum == CPSS_CPU_PORT_NUM_CNS)
                {
                    /* CPU port is configured according to SDMA port group
                       Port 15 - port group 0 SDMA  Port 31 - port group 1 SDMA
                       Port 47 - port group 2 SDMA  Port 63 - port group 3 SDMA */
                    portPortGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);
                    portValue = (((portPortGroupId) & 0x3)<< 4) | 0xF;
                }
                else
                {
                    portValue =  PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(devNum,portNum);
                }

                /* loop on all TXQ units */
                /* on the TXQ that 'own' the port , set the needed configuration */
                /* on other TXQ put 'NULL port' */
                for (i =0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
                {
                    if (i == portPortGroupId/PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS)
                    {
                        portHwValue = portValue;
                    }
                    else
                    {
                        portHwValue = PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
                    }

                    /* write the port monitored by set */
                    portHwValue = portHwValue << 4;

                    rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                           i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                           regAddr, (value | portHwValue));
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
                rc = GT_OK;
            }
        }
        else
        {
            portPortGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

            rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
        }

        if(rc == GT_OK)
        {
            /* save the info for 'get' configuration */
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                    portEgressCntrModeInfo[cntrSetNum].portGroupId = portPortGroupId;
        }

        return rc;
    }
}

/**
* @internal cpssDxChPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue (0..7),
*                                      if corresponding bit in setModeBmp is 1.
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS cpssDxChPortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEgressCntrModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel));

    rc = internal_cpssDxChPortEgressCntrModeSet(devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChSip5PortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] vlanIdPtr                - Pointer to the vlan id of the packets counted by the
*                                      set of counters.
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
static GT_STATUS prvCpssDxChSip5PortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PORT_NUM                     *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 regAddr1;        /* register address */
    GT_U32 value;           /* register value*/
    GT_U32 value1;          /* register value*/
    GT_U32 dxDpLevel;       /* DP HW value */
    GT_U32 tcNumBits;       /* TC numOfBits - different for SIP5 and SIP6 */

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
            egrMIBCntrs.egrMIBCntrsSetConfig[cntrSetNum];
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
            egrMIBCntrs.egrMIBCntrsPortSetConfig[cntrSetNum];
        tcNumBits = 4;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[cntrSetNum];
        regAddr1 = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[cntrSetNum];
        tcNumBits = 3;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr1, &value1);
    if (rc != GT_OK)
    {
        return rc;
    }

    *setModeBmpPtr = value1 & 0x1;
    *setModeBmpPtr |= value & 0xE;

    *portNumPtr = U32_GET_FIELD_MAC(value1, 4           , 17);
    *vlanIdPtr  = U32_GET_FIELD_MAC(value, 10           , 16);
    *tcPtr      = U32_GET_FIELD_MAC(value, 26           , tcNumBits);
    dxDpLevel   = U32_GET_FIELD_MAC(value, 26+tcNumBits , 2);

    PRV_CPSS_DXCH_COS_DP_TO_SW_CHECK_AND_CONVERT_MAC(
        devNum, dxDpLevel, (*dpLevelPtr));

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] vlanIdPtr                - Pointer to the vlan id of the packets counted by the
*                                      set of counters.
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
static GT_STATUS internal_cpssDxChPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
    GT_STATUS rc;              /* return code                   */
    GT_U32    regAddr;         /* register address              */
    GT_U32    dxDpLevel;       /* DP HW value                   */
    GT_U32    dxDpLevelMask;   /* DP HW value mask              */
    GT_U32    value;           /* HW value of register contents */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(setModeBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(vlanIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpLevelPtr);

    /* only 2 sets supported for now */
    if(cntrSetNum > 1)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChSip5PortEgressCntrModeGet(devNum, cntrSetNum, setModeBmpPtr, portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr);
    }
    else
    {

        /* get the info from DB */
        portGroupId = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                    portEgressCntrModeInfo[cntrSetNum].portGroupId;

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].txQConfig;
            dxDpLevelMask = 1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        txqVer1.queue.peripheralAccess.egressMibCounterSet.config[cntrSetNum];
            dxDpLevelMask = 3;
        }

        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &value);

        *setModeBmpPtr = value & 0xF;

        *portNumPtr = (GT_U8)((value >> 4) & 0x3F);
        *vlanIdPtr = (GT_U16)((value >> 10) & 0xFFF);
        *tcPtr = (GT_U8)((value >> 22) & 0x7);


        dxDpLevel = (value >> 25) & dxDpLevelMask;

        PRV_CPSS_DXCH_COS_DP_TO_SW_CHECK_AND_CONVERT_MAC(
            devNum, dxDpLevel, (*dpLevelPtr));

        if(1 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* support multi TXQs device, convert local port to global port */
                *portNumPtr = (GT_U8)( PRV_CPSS_DXCH_HEM_LOCAL_TO_GLOBAL_PORT(devNum,portGroupId,(*portNumPtr)));
        }

        return rc;
    }

}

/**
* @internal cpssDxChPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] vlanIdPtr                - Pointer to the vlan id of the packets counted by the
*                                      set of counters.
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS cpssDxChPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEgressCntrModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, setModeBmpPtr, portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr));

    rc = internal_cpssDxChPortEgressCntrModeGet(devNum, cntrSetNum, setModeBmpPtr, portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, setModeBmpPtr, portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEgressCntrsGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
{
    GT_U32 regAddr;         /* register address */
    CPSS_PORT_EGRESS_CNTR_STC   *egressCntrShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(egrCntrPtr);

    if (cntrSetNum > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_LION_BOOKMARK /* need to support PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.multicastFifoDroppedPacketsCounter[cntrSetNum]

                    like in EXMXPM : CPSS_EXMXPM_BRG_EGRESS_CNTR_STC
                    */

    /* read Outgoing Unicast Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
                egrMIBCntrs.egrMIBOutgoingUcPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setOutgoingUcPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    egrTxQConf.txQCountSet[cntrSetNum].outUcPkts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.outgoingUnicastPacketCounter[cntrSetNum];
    }

    if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->outUcFrames,NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* read Outgoing Multicast Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
                egrMIBCntrs.egrMIBOutgoingMcPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setOutgoingMcPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].outMcPkts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.outgoingMulticastPacketCounter[cntrSetNum];
    }


    if(prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->outMcFrames,NULL)
       != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* read Outgoing Byte Count Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
                egrMIBCntrs.egrMIBOutgoingBcPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setOutgoingBcPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].outBytePkts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.outgoingBroadcastPacketCounter[cntrSetNum];
    }

    if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->outBcFrames,NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* read Bridge Egress Filtered Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.
                egrMIBCntrs.egrMIBBridgeEgrFilteredPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setBridgeEgrFilteredPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].brgEgrFiltered;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.bridgeEgressFilteredPacketCounter[cntrSetNum];
    }

    if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->brgEgrFilterDisc,NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.
            egrMIBCntrs.egrMIBMcFilteredPktCntr[cntrSetNum];

        if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->mcFilterDropPkts,NULL) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        egrCntrPtr->mcFilterDropPkts = 0;
    }

    /* read Transmit Queue Filtered Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
                egrMIBCntrs.egrMIBTailDroppedPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setTailDroppedPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].txQFiltered;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.tailDroppedPacketCounter[cntrSetNum];
    }

    if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->txqFilterDisc,NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* read  to cpu, from cpu and to analyzer Packets Count Register*/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.
                egrMIBCntrs.egrMIBCtrlPktCntr[cntrSetNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.
                egrMIBCntrs.setCtrlPktCntr[cntrSetNum];
        }
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].outCtrlPkts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.peripheralAccess.egressMibCounterSet.controlPacketCounter[cntrSetNum];
    }

    if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->outCtrlFrames,NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E) && (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) != GT_TRUE))
    {
        /* read  egress forwarding restriction dropped Packets Count Register
           Supported from ch2 devices.*/
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            egrTxQConf.txQCountSet[cntrSetNum].egrFrwDropPkts;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        txqVer1.queue.peripheralAccess.egressMibCounterSet.
                            egressForwardingRestrictionDroppedPacketsCounter[cntrSetNum];
        }

        if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->egrFrwDropFrames,NULL) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        egrCntrPtr->egrFrwDropFrames = 0;
    }

    /* read Transmit Queue Filtered Packet Count Register */
    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        && (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0))
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setMcFIFO3_0DroppedPktsCntr[cntrSetNum];

        if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->mcFifo3_0DropPkts,NULL) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setMcFIFO7_4DroppedPktsCntr[cntrSetNum];

        if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->mcFifo7_4DropPkts,NULL) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        egrCntrPtr->mcFifo3_0DropPkts = 0;
        egrCntrPtr->mcFifo7_4DropPkts = 0;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_eft.
            egrMIBCntrs.egrMIBMcFIFODroppedPktsCntr[cntrSetNum];
        if (prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, &egrCntrPtr->mcFifoDropPkts,NULL) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        egrCntrPtr->mcFifoDropPkts = 0;
    }


    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
                        devNum,
                        PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E))
     {
        egressCntrShadowPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E.
                      egressCntrShadow[cntrSetNum]);

        egrCntrPtr->outUcFrames      -= egressCntrShadowPtr->outUcFrames;
        egrCntrPtr->outMcFrames      -= egressCntrShadowPtr->outMcFrames;
        egrCntrPtr->outBcFrames      -= egressCntrShadowPtr->outBcFrames;
        egrCntrPtr->brgEgrFilterDisc -= egressCntrShadowPtr->brgEgrFilterDisc;
        egrCntrPtr->txqFilterDisc    -= egressCntrShadowPtr->txqFilterDisc;
        egrCntrPtr->outCtrlFrames    -= egressCntrShadowPtr->outCtrlFrames;
        egrCntrPtr->egrFrwDropFrames -= egressCntrShadowPtr->egrFrwDropFrames;
        egrCntrPtr->mcFifo3_0DropPkts -= egressCntrShadowPtr->mcFifo3_0DropPkts;
        egrCntrPtr->mcFifo7_4DropPkts -= egressCntrShadowPtr->mcFifo7_4DropPkts;

        egressCntrShadowPtr->outUcFrames      += egrCntrPtr->outUcFrames;
        egressCntrShadowPtr->outMcFrames      += egrCntrPtr->outMcFrames;
        egressCntrShadowPtr->outBcFrames      += egrCntrPtr->outBcFrames;
        egressCntrShadowPtr->brgEgrFilterDisc += egrCntrPtr->brgEgrFilterDisc;
        egressCntrShadowPtr->txqFilterDisc    += egrCntrPtr->txqFilterDisc;
        egressCntrShadowPtr->outCtrlFrames    += egrCntrPtr->outCtrlFrames;
        egressCntrShadowPtr->egrFrwDropFrames += egrCntrPtr->egrFrwDropFrames;
        egressCntrShadowPtr->mcFifo3_0DropPkts += egrCntrPtr->mcFifo3_0DropPkts;
        egressCntrShadowPtr->mcFifo7_4DropPkts += egrCntrPtr->mcFifo7_4DropPkts;

     }

    return GT_OK;
}

/**
* @internal cpssDxChPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEgressCntrsGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEgressCntrsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, egrCntrPtr));

    rc = internal_cpssDxChPortEgressCntrsGet(devNum, cntrSetNum, egrCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, egrCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersEnable function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -   update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacCountersEnable
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS   rc;
    GT_U32                            regAddr; /* register address to set */
    GT_U32                            value;   /* value read from register */
    GT_U32                            offset;  /* field offset */
    GT_U32                            portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E]; /* registers info */
    CPSS_PP_FAMILY_TYPE_ENT           family; /* device family */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersEnable)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        GT_BOOL valueToSet = enable;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersEnable(devNum,portNum,
                                  enable,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&valueToSet);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }


    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* To enable MIB counters:
     * Tri-Speed port - set "MIBCntEn" - bit 15 - to 1.
     * HyperG.Stack/XG/XLG - set "MIBCntDis" - bit 15 (CH, CH2, CH3, XCAT, XCAT2) or bit 14 -to 0 */

    /* Tri-Speed interfaces */
    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 15;
    }

    /* HyperG.Stack/XG/XLG interfaces */

    value = (enable == GT_TRUE)? 0 : 1; /* revert enable value */
    family = PRV_CPSS_PP_MAC(devNum)->devFamily;
    if (family <= CPSS_PP_FAMILY_CHEETAH3_E  ||
        family == CPSS_PP_FAMILY_DXCH_XCAT_E ||
        family == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        offset = 15;
    }
    else
    {
        offset = 14;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortMacCountersEnable function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -   update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersEnable(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortMacCountersEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacCountersClearOnReadSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;/* register address to set */
    GT_U32      value;  /* value to set to bits */
    GT_U32      group;  /* Port's group (GOP) */
    GT_U32      offset; /* bit's offset */
    GT_STATUS   rc;     /* return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_ERRATA_STACK_MAC_COUNT_SHADOW_STC *macCountShadowPtr; /* Pointer To MAC Counters Shadow */
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersClearOnReadSet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        GT_BOOL valueToSet = enable;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersClearOnReadSet(devNum,portNum,
                                  enable,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&valueToSet);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

   /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum]->clearOnReadEnable = enable;
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(devNum, portMacNum))
    {
        macCountShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_E.
            stackMacCntPtr;
        macCountShadowPtr->stackPortArr[portMacNum - 24].clearOnRead = enable;
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE &&
        portMacNum == CPSS_CPU_PORT_NUM_CNS)
    {
        value = ((enable == GT_TRUE) ? 1 : 0);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.cpuPortCtrlReg;
        offset = 3;
        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                  regAddr, offset, 1, value);
    }
    else
    {
        value = ((enable == GT_TRUE) ? 0 : 1);
        offset = 4;
        group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);
        if(group < 4)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group];
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        regAddr, offset, 1, value);
                if (rc!=GT_OK)
                {
                    return rc;
                }
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            if(portMacNum < 24)
                return GT_OK;

            switch(portMacNum){
            case 24:
                offset = 0;
                break;
            case 25:
                offset = 1;
                break;
            case 26:
                offset = 2;
                break;
            case 27:
                offset = 19;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr;
        }
        /* Cheetah3 and above */
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            offset = 1;
        }

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                      regAddr, offset, 1, value);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersClearOnReadSet(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersClearOnReadSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortMacCountersClearOnReadSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get "Clear on read" status of MAC Counters per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - (pointer to) "Clear on read" status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacCountersClearOnReadGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address to get */
    GT_U32      regData; /* register data */
    GT_U32      group;   /* Port's group (GOP) */
    GT_U32      offset;  /* bit's offset */
    GT_U32 useAsGePort = 0;/* does this port uses the GE port mechanism or the XG mechanism */
    PRV_CPSS_DXCH_ERRATA_STACK_MAC_COUNT_SHADOW_STC *macCountShadowPtr; /* Pointer To MAC Counters Shadow */
    GT_U32 portMacNum = portNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersClearOnReadGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersClearOnReadGet(devNum,portNum,
                                  enablePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if(IS_MIB_API_SW_EMULATED_MAC(devNum,portMacNum))
    {
        *enablePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum]->clearOnReadEnable;
        return GT_OK;
    }

    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);

    if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(devNum, portMacNum))
    {
        macCountShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_E.
            stackMacCntPtr;
        *enablePtr = macCountShadowPtr->stackPortArr[portMacNum - 24].clearOnRead;
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE &&
        portMacNum == CPSS_CPU_PORT_NUM_CNS)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.cpuPortCtrlReg;
        offset = 3;
    }
    else
    {
        if(useAsGePort)
        {
            group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;
            offset = 4;
        }
        else
        {
            if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
            {
                switch(portMacNum){
                    case 24:
                        offset = 0;
                        break;
                    case 25:
                        offset = 1;
                        break;
                    case 26:
                        offset = 2;
                        break;
                    case 27:
                        offset = 19;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr;
            }

            /* Cheetah3 and above */
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
                offset = 1;
            }

        }

    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, offset, 1, &regData);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE &&
        portMacNum == CPSS_CPU_PORT_NUM_CNS)
    {
        *enablePtr = BIT2BOOL_MAC(regData);
    }
    else
    {
        *enablePtr = (regData == 0 ) ? GT_TRUE : GT_FALSE ;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get "Clear on read" status of MAC Counters per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - (pointer to) "Clear on read" status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersClearOnReadGet(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersClearOnReadGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortMacCountersClearOnReadGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersRxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for received packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacCountersRxHistogramEnable
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;/* register address to set */
    GT_U32      value;/* value to set to bits */
    GT_U32      group;  /* Port's group (GOP) */
    GT_U32      offset; /* bit's offset */
    GT_STATUS rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32          portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersRxHistogramEnable)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        GT_BOOL valueToSet = enable;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersRxHistogramEnable(devNum,portNum,
                                  enable,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&valueToSet);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    value = ((enable == GT_TRUE) ? 1 : 0);

    if((PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum) == GT_FALSE) &&  (portMacNum < 24) )
    {
        group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);

        if(PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT3_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mibCountersCtrl;
            offset = 2;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        regAddr, offset, 1, value);

                if (rc!=GT_OK)
                {
                        return rc;
                }
            }
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;
        offset = 5;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                    regAddr, offset, 1, value);

            if (rc!=GT_OK)
            {
                    return rc;
            }
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        if (portNum < 24)
            return GT_OK;

        switch(portNum){
        case 24:
            offset = 6;
            break;
        case 25:
            offset = 7;
            break;
        case 26:
            offset = 8;
            break;
        case 27:
            offset = 21;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr ;
     }
    /* Cheetah3 and above */
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;

        offset = 2;
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                                                      regAddr, offset, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mibCountersCtrl;

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                regAddr, offset, 1, value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xlgMibCountersCtrl;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        return prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                                                  regAddr, offset, 1, value);
    }

    return GT_OK;

}

/**
* @internal cpssDxChPortMacCountersRxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for received packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersRxHistogramEnable(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersRxHistogramEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortMacCountersRxHistogramEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersTxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for transmitted packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacCountersTxHistogramEnable
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;/* register address to set */
    GT_U32      value;/* value to set to bits */
    GT_U32      group;  /* Port's group (GOP) */
    GT_U32      offset; /* bit's offset */
    GT_STATUS rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32          portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersTxHistogramEnable)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        GT_BOOL valueToSet = enable;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersTxHistogramEnable(devNum,portNum,
                                  enable,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&valueToSet);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    value = ((enable == GT_TRUE) ? 1 : 0);

    if((PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum) == GT_FALSE) &&  (portMacNum < 24) )
    {
        group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);

        if(PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT3_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mibCountersCtrl;
            offset = 3;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        regAddr, offset, 1, value);

                if (rc!=GT_OK)
                {
                        return rc;
                }
            }
        }

       regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;
       offset = 6;
       if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                    regAddr, offset, 1, value);

            if (rc!=GT_OK)
            {
                    return rc;
            }
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        if(portNum < 24)
            return GT_OK;

        switch(portNum){
        case 24:
            offset = 9;
            break;
        case 25:
            offset = 10;
            break;
        case 26:
            offset = 11;
            break;
        case 27:
            offset = 22;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr ;
    }
    /* Cheetah3 and above */
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
        offset = 3;
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                                                  regAddr, offset, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mibCountersCtrl;

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                regAddr, offset, 1, value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xlgMibCountersCtrl;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        return prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                                                  regAddr, offset, 1, value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersTxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for transmitted packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersTxHistogramEnable
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersTxHistogramEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortMacCountersTxHistogramEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behaviour for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2, xCat3; Bobcat2;
*         Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
static GT_STATUS internal_cpssDxChPortMacOversizedPacketsCounterModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
{
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;      /* value to set to bits */
    GT_U32 group;      /* Port's group (GOP) */
    GT_U32 mask;       /* bit's offset */
    GT_U32 portOffset; /* port's offset in register */
    GT_U32 useAsGePort;/* does this port uses the GE port mechanism or the XG mechanism */
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E| CPSS_LION2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacOversizedPacketsCounterModeSet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT valueToSet = counterMode;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacOversizedPacketsCounterModeSet(devNum,portNum,
                                  counterMode,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&valueToSet);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(devNum, portMacNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);
    if(PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT2_E)
    {
        switch(counterMode)
        {
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E:
                value = 0x0; break;
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E:
                value = 0x1; break;
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E:
                value = useAsGePort ? 0x41 : 0x3; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(useAsGePort)
        {
            group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);
            if(group >= 4)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            portOffset = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) % 6);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;

            mask = 0x410000 << portOffset;
            value <<=  16 + portOffset;

            rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,
              PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
              regAddr,mask,value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
              macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
              PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
              regAddr,6,2,value);
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT3_E || PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        switch(counterMode)
        {
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E:
                value = 0x0; break;
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E:
                value = 0x1; break;
            case CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E:
                value = 0x3; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

#ifndef GM_USED
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].mibCountersCtrl;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 2, value);

        if (rc != GT_OK) {
            return rc;
        }

        if (!useAsGePort)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 2, value);
        }
#else /* don't set counter for GM */
        rc = GT_OK;
#endif /* GM_USED */
    }
    else
    {
        rc = GT_BAD_PARAM;
    }
    return rc;
}

/**
* @internal cpssDxChPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behaviour for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2, xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; Lion2 Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacOversizedPacketsCounterModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterMode));

    rc = internal_cpssDxChPortMacOversizedPacketsCounterModeSet(devNum, portNum, counterMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2; xCat3; Bobcat2 Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; Lion2 Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
static GT_STATUS internal_cpssDxChPortMacOversizedPacketsCounterModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
{
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;      /* value to set to bits */
    GT_U32 group;      /* Port's group (GOP) */
    GT_U32 mask;       /* bit's offset */
    GT_U32 portOffset; /* port's offset in register */
    GT_U32 useAsGePort;/* does this port uses the GE port mechanism or the XG mechanism */
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E| CPSS_LION2_E| CPSS_FALCON_E );
    CPSS_NULL_PTR_CHECK_MAC(counterModePtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacOversizedPacketsCounterModeGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacOversizedPacketsCounterModeGet(devNum,portNum,
                                  counterModePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);
    if(PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT2_E)
    {
        if(useAsGePort)
        {
            group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);
            if(group >= 4)
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

            portOffset = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) % 6);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;

            mask = 0x410000 << portOffset;

            rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum,
              PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
              regAddr,mask,&value);
            if (rc != GT_OK)
            {
                return rc;
            }
            value >>= 16 + portOffset;
            switch (value & 0x41)
            {
                case 0x0:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
                    break;
                case 0x1:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
                    break;
                case 0x40:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
                    break;
                case 0x41:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
                    break;
            }
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
              macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
              PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
              regAddr,6,2,&value);
            if (rc != GT_OK)
            {
                return rc;
            }
            switch (value)
            {
                case 0:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
                    break;
                case 1:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
                    break;
                case 2:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
                    break;
                case 3:
                    *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
                    break;
            }
        }
    }
    else if ( PRV_CPSS_PP_MAC(devNum)->appDevFamily == CPSS_XCAT3_E || PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
#ifndef GM_USED
        if(useAsGePort)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].mibCountersCtrl;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 2, &value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 11, 2, &value);
        }
        switch (value)
        {
            case 0:
                *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
                break;
            case 1:
                *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
                break;
            case 3:
                *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
#else /* for GM use default counter mode */
        *counterModePtr = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
        rc = GT_OK;
#endif /* GM_USED */
    }
    else
    {
        rc = GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal cpssDxChPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2; xCat3; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacOversizedPacketsCounterModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterModePtr));

    rc = internal_cpssDxChPortMacOversizedPacketsCounterModeGet(devNum, portNum, counterModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortStatTxDebugCountersGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */
    GT_U32 regValue;    /* register value */
    GT_U32 i;           /* loop iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(dropCntrStcPtr);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.dropCntrs.egrMirroredDroppedCntr;

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, &regValue, NULL);
    if(GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrMirrorDropCntr = regValue;

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.dropCntrs.STCDroppedCntr;

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, &regValue, NULL);
    if(GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrStcDropCntr = regValue;

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNDropCntr;

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, &regValue, NULL);
    if(GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrQcnDropCntr = regValue;

    dropCntrStcPtr->dropPfcEventsCntr = 0;

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[0];

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, &regValue, NULL);
    if(GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->dropPfcEventsCntr = regValue;


    dropCntrStcPtr->clearPacketsDroppedCounter = 0;
    for(i = 0; i < 4; i++)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).global.dropCntrs.clearPktsDroppedCntrPipe[i];

        rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, &regValue, NULL);
        if(GT_OK != rc)
        {
            return rc;
        }
        dropCntrStcPtr->clearPacketsDroppedCounter += regValue;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortStatTxDebugCountersGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortStatTxDebugCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCntrStcPtr));

    rc = internal_cpssDxChPortStatTxDebugCountersGet(devNum, dropCntrStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCntrStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMacCountersHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for received/transmitted packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] direction                - CPSS_PORT_DIRECTION_RX_E - get state of received packets
*                                      CPSS_PORT_DIRECTION_TX_E - get state of transmitted packets
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE  - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, physical port, direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS prvCpssDxChPortMacCountersHistogramEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      group;       /* Port's group (GOP) */
    GT_U32      offset;      /* bit's offset */
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32      portMacNum;
    GT_U32      useAsGePort;
    GT_U32      value = 0;          /* value read from register */
    GT_U32      isRx;
    CPSS_PP_FAMILY_TYPE_ENT family;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersHistogramEnableGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersHistogramEnableGet(devNum,portNum,
                                  direction,
                                  enablePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (direction != CPSS_PORT_DIRECTION_RX_E
        && direction != CPSS_PORT_DIRECTION_TX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    isRx = (direction == CPSS_PORT_DIRECTION_RX_E);

    family = PRV_CPSS_PP_MAC(devNum)->devFamily;
    useAsGePort = IS_GE_PORT_COUNTERS_MAC(devNum,portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(useAsGePort)
    {
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) || family == CPSS_PP_FAMILY_DXCH_LION2_E || family == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            offset = isRx? 2 : 3;
        }
        else if (family == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mibCountersCtrl;
            offset = isRx? 2 : 3;
        }
        else /* legacy devices */
        {
            group = (PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum) / 6);
            if(group >= 4)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.macCountersCtrl[group] ;
            offset = isRx? 5 : 6;
        }

    } else /* not tri-speed ports */
    {
        if (family <= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.hgXsmiiCountersCtr ;
            /* ports 0-23 are covered by the "if" clause */
            switch(portNum){
                case 24:
                    offset = isRx? 6 : 9;
                    break;
                case 25:
                    offset = isRx? 7 : 10;
                    break;
                case 26:
                    offset = isRx? 8 : 11;
                    break;
                case 27:
                    offset = isRx? 21 : 22;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) >= PRV_CPSS_PORT_XLG_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    macRegs.perPortRegs[portMacNum].xlgMibCountersCtrl;
            }
            if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            }
            offset = isRx? 2 : 3;
        }
    }
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                            regAddr, offset, 1, &value);

    if (rc!=GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for received packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssDxChPortMacCountersRxHistogramEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    return prvCpssDxChPortMacCountersHistogramEnableGet(devNum, portNum,
                                                        CPSS_PORT_DIRECTION_RX_E,
                                                        enablePtr);
}

/**
* @internal cpssDxChPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for received packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersRxHistogramEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersRxHistogramEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortMacCountersRxHistogramEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for transmitted packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssDxChPortMacCountersTxHistogramEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    return prvCpssDxChPortMacCountersHistogramEnableGet(devNum, portNum,
                                                        CPSS_PORT_DIRECTION_TX_E,
                                                        enablePtr);
}

/**
* @internal cpssDxChPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for transmitted packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersTxHistogramEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersTxHistogramEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortMacCountersTxHistogramEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssDxChPortMacCountersEnableGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32                  portGroupId;
    GT_U32                  portMacNum;
    GT_U32                  regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    GT_U32                  offset;
    GT_U32                  value;
    CPSS_PP_FAMILY_TYPE_ENT family;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersEnableGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersEnableGet(devNum,portNum,
                                  enablePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,  portMacNum);
    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,  portMacNum, portType, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }


    /* To enable MIB counters:
     * Tri-Speed port      - set "MIBCntEn" - bit 15 - to 1.
     * HyperG.Stack/XG/XLG - set "MIBCntDis" - bit 15 (CH, CH2, CH3, XCAT, XCAT2) or bit 14 - to 0. */

    family = PRV_CPSS_PP_MAC(devNum)->devFamily;

    if (portType <= PRV_CPSS_PORT_GE_E         ||
        family   <= CPSS_PP_FAMILY_CHEETAH3_E  ||
        family   == CPSS_PP_FAMILY_DXCH_XCAT_E ||
        family   == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        offset = 15;
    }
    else
    {
        offset = 14;
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                           regAddr, offset, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (portType <= PRV_CPSS_PORT_GE_E)
    {
        *enablePtr = BOOL2BIT_MAC(value);
    }
    else
    {
        /* revert value */
        *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersEnableGet);

    CPSS_API_LOCK_MAC(0,0);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortMacCountersEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(0,0);

    return rc;
}

/**
* @internal internal_cpssDxChPortRemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         If the packet has the CPU code as configured during remote port Bind,
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*
*/
static GT_STATUS internal_cpssDxChPortRemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
)
{
    GT_STATUS       rc = GT_OK;

    /* Check packet is of type TO_CPU */
    if (rxParamsPtr->dsaParam.dsaType != CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        return GT_OK;
    }
    rc = prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse(
                                                devNum,
                                                numOfBuff,
                                                packetBuffsArrPtr,
                                                buffLenArr,
                                                rxParamsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssDxChPortRemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         If the packet has the CPU code as configured during remote port Bind,
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*
*/
GT_STATUS cpssDxChPortRemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRemotePortsMacCountersByPacketParse);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfBuff, packetBuffsArrPtr, buffLenArr, rxParamsPtr));

    rc = internal_cpssDxChPortRemotePortsMacCountersByPacketParse(devNum, numOfBuff, packetBuffsArrPtr, buffLenArr, rxParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfBuff, packetBuffsArrPtr, buffLenArr, rxParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacCounterOnPhySideGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter on PHY side of connecting link to remote ports.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChMacCounterOnPhySideGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCounterOnPhySideGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCounterOnPhySideGet(devNum,portNum,
                                  cntrName,
                                  cntrValuePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChMacCounterOnPhySideGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter on PHY side of connecting link to remote ports.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChMacCounterOnPhySideGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacCounterOnPhySideGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cntrName, cntrValuePtr));

    rc = internal_cpssDxChMacCounterOnPhySideGet(devNum, portNum, cntrName, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cntrName, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacCountersOnPhySidePortGet function
* @endinternal
*
* @brief   Gets all port Ethernet MAC counters on PHY side of connecting link to remote Port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChPortMacCountersOnPhySidePortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macCounters.macDrvMacPortMacCountersOnPhySidePortGet)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macCounters.macDrvMacPortMacCountersOnPhySidePortGet(devNum,portNum,
                                  portMacCounterSetArrayPtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortMacCountersOnPhySidePortGet function
* @endinternal
*
* @brief   Gets all port Ethernet MAC counters on PHY side of connecting link to remote Port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChPortMacCountersOnPhySidePortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacCountersOnPhySidePortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacCounterSetArrayPtr));

    rc = internal_cpssDxChPortMacCountersOnPhySidePortGet(devNum, portNum, portMacCounterSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacCounterSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



