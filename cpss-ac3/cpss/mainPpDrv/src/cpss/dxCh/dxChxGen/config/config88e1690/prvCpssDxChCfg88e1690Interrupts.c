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
* @file prvCpssDxChCfg88e1690Interrupts.c
*
* @brief file to manage the interrupts of 88e1690 PHY-MAC.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>

#include <msApi.h>
#include <msApiTypes.h>
#include <gtDrvSwRegs.h>
#include <msApiInternal.h>

#define PHY_PAGE(x)     x
#define QD_PHY_INT_ENABLE_REG            18
#define QD_PHY_INT_STATUS_REG            19
#define QD_PHY_INT_PORT_SUMMARY_REG        20

/*
* typedef: enum GT_PHY_INT
*
* Description: Enumeration of PHY interrupt
*/
#define GT_AUTO_NEG_COMPLETED     BIT_11 /*0x800*/
#define GT_LINK_STATUS_CHANGED    BIT_10 /*0x400*/

/*
 * typedef: enum GT_EVENT_TYPE
 *
 * Description: Enumeration of the available hardware driven events.
 *
 * Enumerations:
 *   GT_AVB_INT    - AVB Interrupt Enable
 *   GT_DEVICE_INT - Device Interrupt (GT_DEVICE_INT_TYPE) Enable
 *   GT_STATS_DONE - Statistics Operation Done interrrupt Enable
 *   GT_CPSS_VTU_PROB - VLAN Problem/Violation Interrupt Enable
 *   GT_CPSS_VTU_DONE - VALN Table Operation Done Interrupt Enable
 *   GT_CPSS_ATU_PROB - ATU Problem/Violation Interrupt Enable, for Gigabit Switch
 *   GT_CPSS_ATU_DONE - ATU Done interrupt enable.
 *   GT_CPSS_TCAM_INT  - TCAM interrupt enable
 *   GT_EE_INT   - EEPROM Done interrupt enable.
 */
#define GT_AVB_INT          0x100
#define GT_DEVICE_INT       0x80
#define GT_STATS_DONE       0x40
#define GT_CPSS_VTU_PROB         0x20
#define GT_CPSS_VTU_DONE         0x10
#define GT_CPSS_ATU_PROB         0x8
#define GT_CPSS_ATU_DONE         0x4
#define GT_CPSS_TCAM_INT         0x2
#define GT_EE_INTERRUPT     0x1

#define GT_INT_MASK                        \
                (GT_AVB_INT | GT_DEVICE_INT | GT_STATS_DONE | \
                 GT_CPSS_VTU_PROB | GT_CPSS_VTU_DONE | GT_CPSS_ATU_PROB |         \
                 GT_CPSS_ATU_DONE | GT_CPSS_TCAM_INT | GT_EE_INTERRUPT)

/*
 * typedef: enum GT2_EVENT_TYPE
 *
 * Description: Enumeration of the available hardware driven events
 *                  - at Global Status 2 register.
 *
 * Enumerations:
 *   GT2_WATCHDOG_INT    - WatchDog interrupt
 *   GT2_JAM_LIMIT_INT - Jam Limit interrupt
 *   GT2_DUPLEX_MISMATCH_INT - Dumplex Mismatch interrupt
 *   GT2_WAKE_EVENT_INT - Wake Event interrupt
 *   GT2_PHYS_INT - PHYs 1-10 summary interrupts
 */
#define GT2_WATCHDOG_INT                   BIT_15
#define GT2_JAM_LIMIT_INT           BIT_14
#define GT2_DUPLEX_MISMATCH_INT     BIT_13
#define GT2_WAKE_EVENT_INT          BIT_12
#define GT2_PHYS_INT                (BIT_10 | BIT_9 | BIT_8 |   \
                                        BIT_7 | BIT_6 | BIT_5 | \
                                        BIT_4 | BIT_3 | BIT_2 | BIT_1)

#define GT2_INT_MASK                        \
                (GT2_WATCHDOG_INT | GT2_JAM_LIMIT_INT | GT2_DUPLEX_MISMATCH_INT | \
                 GT2_WAKE_EVENT_INT | GT2_PHYS_INT)

/* must be step of 32 (and not 16) due to function prvCpssDrvIntEvReqDrvnScan(...) */
#define INT_EVENT_STEPS(index)     (32 * (index))




/* cookie info for prvCpssDrvDxExMxGppIsrConnect(...) */
/*
 * Typedef: struct PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC
 *
 * Description: A Structure to hold information for ISR CB for MPP of remote device.
 *              used as cookie info for prvCpssDrvDxExMxGppIsrConnect(...)
 *
 * Fields:
 *      remotePhyMacInfoPtr - pointer to 88e1960 on this MPP-GPIO
 *      nextDevicePtr       - pointer to info about next 88e1960 on this MPP-GPIO
 */
typedef struct PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STCT{
    /* pointer to current 88e1690 that is bound to the MPP-GPIO */
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    /* pointer to next (another) 88e1690 that may be bound to the MPP-GPIO */
    /* this to support 'Many to 1' */
    struct PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STCT *nextDevicePtr;
}PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STC;


/**
* @internal internal88e1690EventMaskSet function
* @endinternal
*
* @brief   for 88e1690 device : This routine mask/unmasks an unified event on specific remote device.
*
* @param[in] devNum                   - device number (of the DX device)
* @param[in] uniEvent                 - The unified event.
* @param[in] operation                - the  : mask / unmask
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*                                      NOTE: when NULL , meaning that the event is not for specific
*                                      88e1690 device ... but for all 88e1690 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
static GT_STATUS internal88e1690EventMaskSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN CPSS_EVENT_MASK_SET_ENT  operation,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  event;          /* The event index              */
    GT_U32  intNodesPoolSize;
    GT_U32  dxPortNum;
    PRV_CPSS_DRV_EV_REQ_NODE_STC  *nodePoolArr;   /* The event node array   */

    if(remotePhyMacInfoPtr) /* specific port */
    {
        intNodesPoolSize = remotePhyMacInfoPtr->interrupts.numOfIntBits;
        nodePoolArr = remotePhyMacInfoPtr->interrupts.intNodesPool;

        for (event = 0; event < intNodesPoolSize; event++)
        {
            if (nodePoolArr[event].uniEvCause != (GT_U32)uniEvent)
            {
                continue;
            }
            rc = nodePoolArr[event].intMaskSetFptr(&nodePoolArr[event], operation);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
            for(dxPortNum = 0 ; dxPortNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; dxPortNum++)
            {
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[dxPortNum] == NULL)
                {
                    continue;
                }

                remotePhyMacInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[dxPortNum];

                intNodesPoolSize = remotePhyMacInfoPtr->interrupts.numOfIntBits;
                nodePoolArr = remotePhyMacInfoPtr->interrupts.intNodesPool;

                for (event = 0; event < intNodesPoolSize; event++)
                {
                    if (nodePoolArr[event].uniEvCause != (GT_U32)uniEvent)
                    {
                        continue;
                    }
                    rc = nodePoolArr[event].intMaskSetFptr(&nodePoolArr[event], operation);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }

    }

    return GT_OK;
}


/**
* @internal internalRemoteDeviceEventMaskDeviceSet function
* @endinternal
*
* @brief   for remote device : This routine mask/unmasks an unified event on specific remote device.
*
* @param[in] devNum                   - device number (of the DX device)
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
* @param[in] operation                - the  : mask / unmask
*
* @param[out] allowToContinuePtr       - (pointer to) indication if allow to continue to
*                                      'local device'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
static GT_STATUS internalRemoteDeviceEventMaskDeviceSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation,
    OUT GT_BOOL                 *allowToContinuePtr
)
{
    GT_BOOL allowToContinue = GT_TRUE;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_STATUS rc;
    GT_U32  portIndex;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    switch(uniEvent)
    {
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
        case CPSS_PP_PORT_AN_COMPLETED_E:
            /* the evExtData is represent a port or 'all ports' */
            if(evExtData == PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
            {
                /* the evExtData represent 'All ports' */
                /* so we need to set ALL the 88e1690 ports on all 88e1690 devices */

                /* allow to configure also the DX device */
                allowToContinue = GT_TRUE;

                rc = internal88e1690EventMaskSet(devNum,uniEvent,operation,NULL);
                if(rc != GT_OK)
                {
                    return rc;
                }

            }
            else
            {
                portNum = evExtData;

                if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
                {
                    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(devNum,portNum,&portIndex);
                    if(remotePhyMacInfoPtr)
                    {
                        /* this port is remote port and belong to specific 88e1690 device */
                        allowToContinue = GT_FALSE;

                        rc = internal88e1690EventMaskSet(devNum,uniEvent,operation,remotePhyMacInfoPtr);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
            break;
        default:
            break;
    }

    *allowToContinuePtr = allowToContinue;
    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPort88e1690InitEvents_dxch function
* @endinternal
*
* @brief   init 88e1690 events per DX device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*/
static GT_STATUS prvCpssDxChCfgPort88e1690InitEvents_dxch
(
    IN GT_U8 devNum
)
{
    /* bind driver with mask events CB function */
    /* NOTE : the function is not in the 'OBJ' !!! */
    prvCpssDrvPpConfig[devNum]->intCtrl.remoteDevice_drvEventMaskDeviceFunc =
        internalRemoteDeviceEventMaskDeviceSet;

    return GT_OK;
}

/**
* @internal prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh_specific88e1690 function
* @endinternal
*
* @brief   call back function for doing MPP GPIO ISR scan on specific 88e1690.
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] remotePhyMacInfoPtr      - info about the 88e1690 that need ISR scanning
*                                       None.
*
* @note similar to prvCpssDrvInterruptPpSR(...).
*
*/
static GT_U8 prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh_specific88e1690
(
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    GT_U8   devNum;         /* The Pp device number to scan.            */
    GT_U32   portGroupId = 0;

    devNum = remotePhyMacInfoPtr->dxDevNum;

    /* call the generic ISR scanning */
    return prvCpssDrvIntEvReqDrvnScan(devNum,portGroupId,
                   remotePhyMacInfoPtr->interrupts.intNodesPool,
                   remotePhyMacInfoPtr->interrupts.intMaskShadow,
                   remotePhyMacInfoPtr->interrupts.intScanRoot);
}

/**
* @internal prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh function
* @endinternal
*
* @brief   call back function for doing MPP GPIO ISR scan on the 88e1690 .
*         the function supports multiple 88e1690 on the same MPP-GPIO.
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cookiePtr                - cookie that was given during 'GPP ISR connect'
*
* @note similar to interruptMainSr(...).
*
*/
static GT_U8 prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh
(
    IN void *cookiePtr
)
{
    PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STC *remoteIsrScanInfoPtr = cookiePtr;

    /* the while supports multiple 88e1690 on the same MPP-GPIO */
    while(remoteIsrScanInfoPtr &&
          remoteIsrScanInfoPtr->remotePhyMacInfoPtr)
    {
        while(
            prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh_specific88e1690(
                remoteIsrScanInfoPtr->remotePhyMacInfoPtr));

        /* proceed to the next 88e1690 device */
        remoteIsrScanInfoPtr = remoteIsrScanInfoPtr->nextDevicePtr;
    }

    return 0;/* NOTE: the return code is NOT used by the caller !!! */
}

/*
    the 32 bits of register address are :
regAddr88e1690 - 0..7   - 88e1690 register
extParam       - 8..15  - the port/PHY number (see bits 26..31)
dxPortNum      - 16..25 - DX cascade port number
smiDevAddr     - 26..31 - 88e1690 SMI address
                (0 = switch , 1 = port , 2 - PHY , G1 = 0x1b , G2 = 0x1c)
*/
#define BUILD_INT_REG_ADDR(regAddr88e1690 , extParam , dxPortNum , smiDevAddr) \
    ((smiDevAddr) << 26) | ((dxPortNum) << 16) | ((extParam) <<8) | (regAddr88e1690)

/* update the address with new associated dxPortNum */
#define UPDATE_INT_REG_ADDR_dxPortNum(regAddr88e1690 , dxPortNum)   \
    {                                                               \
        regAddr88e1690 &= ~(0x3ff << 16);/* clear old dxPortNum */  \
        regAddr88e1690 |=  (dxPortNum << 16);/*set new dxPortNum */   \
    }

#define GET_FROM_INT_REG_regAddr88e1690(compinationValue)   \
    (GT_U8)((compinationValue) & 0xff)

#define GET_FROM_INT_REG_extParam(compinationValue)   \
    (GT_U8)(((compinationValue)>>8) & 0xff)

#define GET_FROM_INT_REG_dxPortNum(compinationValue)   \
    (GT_U32)(((compinationValue)>>16) & 0x3ff)

#define GET_FROM_INT_REG_smiAddr(compinationValue)   \
    (GT_U8)((compinationValue)>>26)


/* function under ISR : read register of the 88e1690 via the DX device  */
static GT_STATUS ISR_generic_88e1690ScanNodeRead_byDxCh
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *dataPtr
)
{
    GT_STATUS   rc;
    GT_U16   U16_data;
    GT_U32  dxPortNum       = GET_FROM_INT_REG_dxPortNum(regAddr);
    GT_U8   regAddr88e1690  = GET_FROM_INT_REG_regAddr88e1690(regAddr);
    GT_U8   extParam        = GET_FROM_INT_REG_extParam(regAddr);
    GT_U8   smiDevAddr      = GET_FROM_INT_REG_smiAddr(regAddr);
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,dxPortNum);

    portGroupId = portGroupId;/* avoid warning */

    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(smiDevAddr == 2/*PHY*/)
    {
        rc = ISR_prvCpssDrvHwReadPagedPhyReg(portInfoPtr->drvInfoPtr,extParam/*portNum*/, PHY_PAGE(0) , regAddr88e1690, &U16_data);
    }
    else
    {
        rc = ISR_prvCpssDrvHwGetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,&U16_data);
    }

    *dataPtr = U16_data;

    return rc;
}

/* function under ISR : write register of the 88e1690 via the DX device  */
static GT_STATUS ISR_generic_88e1690ScanNodeWrite_byDxCh
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   data
)
{
    GT_STATUS   rc;
    GT_U16   U16_data;
    GT_U32  dxPortNum       = GET_FROM_INT_REG_dxPortNum(regAddr);
    GT_U8   regAddr88e1690  = GET_FROM_INT_REG_regAddr88e1690(regAddr);
    GT_U8   extParam        = GET_FROM_INT_REG_extParam(regAddr);
    GT_U8   smiDevAddr      = GET_FROM_INT_REG_smiAddr(regAddr);
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,dxPortNum);

    portGroupId = portGroupId;/* avoid warning */
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U16_data = (GT_U16)data;

    if(smiDevAddr == 2/*PHY*/)
    {
        rc = ISR_prvCpssDrvHwWritePagedPhyReg(portInfoPtr->drvInfoPtr,extParam/*portNum*/, PHY_PAGE(0) , regAddr88e1690, U16_data);
    }
    else
    {
        rc = ISR_prvCpssDrvHwSetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,U16_data);
    }

    return rc;
}


/* set values that will be updated/filled during runtime (initialization stage)*/
#define FILLED_IN_RUNTIME_CNS   0

typedef enum{
    /* global 1 register */
    PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_1_BASE_E = INT_EVENT_STEPS(0),
    PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_1_DEVICE_INT_E = PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_1_BASE_E + 7,

    /* global 2 register */
    PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_BASE_E = INT_EVENT_STEPS(1),
    PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_PORT_0_E = PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_BASE_E,
        /* .. ports 1..8 ... */
    PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_PORT_9_E = PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_PORT_0_E + 9 ,

    /* per PHY port - port 0 */
    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_BASE_E = INT_EVENT_STEPS(2 + 0/*port 0*/),
    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E = PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_BASE_E + 10,
    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E        = PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_BASE_E + 11,

    /* per PHY port - port 1 */
    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_1_BASE_E = INT_EVENT_STEPS(2 + 1/*port 1*/),
        /* .. PHY ports 2..8 ... */
    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_8_BASE_E = INT_EVENT_STEPS(2 + 8/*port 1*/),

    PRV_CPSS_INT_CAUSE_88E1690_AFTER_LAST_PORT_E = INT_EVENT_STEPS(2 + 9) - 1,/*end of port 8 */

    PRV_CPSS_88E1690_LAST_INT_E
}PRV_CPSS_88E1690_INT_CAUSE_ENT;

#define INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(bit,port)    \
    {(bit), GT_FALSE, 0,/* gppId */ NULL,           \
        BUILD_INT_REG_ADDR(QD_PHY_INT_STATUS_REG , port/*PHY number*/ , 0 , 2/*PHY*/) /* causeRegAddr */ , \
        BUILD_INT_REG_ADDR(QD_PHY_INT_ENABLE_REG , port/*PHY number*/ , 0 , 2/*PHY*/) /* maskRegAddr */,   \
        ISR_generic_88e1690ScanNodeRead_byDxCh,          \
        ISR_generic_88e1690ScanNodeWrite_byDxCh,         \
        PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E + INT_EVENT_STEPS(port), /* startIdx */ \
        PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E        + INT_EVENT_STEPS(port), /* endIdx */   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}                                                      \


/* the tree of the 88e1690 device */
/* NOTE: each 88e1690 device requires alloc of it own instance of this tree */
static /*const*/ PRV_CPSS_DRV_INTERRUPT_SCAN_STC  MacPhy88E1690_IntrScanArr[] =
{
    /* Global Interrupt Cause */
    {0, GT_FALSE, 0,/* gppId */ NULL,
        BUILD_INT_REG_ADDR(PRV_CPSS_QD_REG_GLOBAL_STATUS  , 0 , 0 , PRV_CPSS_GLOBAL1_DEV_ADDR) /* causeRegAddr */ ,
        BUILD_INT_REG_ADDR(PRV_CPSS_QD_REG_GLOBAL_CONTROL , 0 , 0 , PRV_CPSS_GLOBAL1_DEV_ADDR) /* maskRegAddr */,
        ISR_generic_88e1690ScanNodeRead_byDxCh,
        ISR_generic_88e1690ScanNodeWrite_byDxCh,
        PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_1_DEVICE_INT_E,
        PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_1_DEVICE_INT_E,
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1 , NULL, NULL},

        {7, GT_FALSE, 0,/* gppId */ NULL,
            BUILD_INT_REG_ADDR(PRV_CPSS_QD_REG_INT_SOURCE , 0 , 0 , PRV_CPSS_GLOBAL2_DEV_ADDR) /* causeRegAddr */ ,
            BUILD_INT_REG_ADDR(PRV_CPSS_QD_REG_INT_MASK   , 0 , 0 , PRV_CPSS_GLOBAL2_DEV_ADDR) /* maskRegAddr */,
            ISR_generic_88e1690ScanNodeRead_byDxCh,
            ISR_generic_88e1690ScanNodeWrite_byDxCh,
            PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_PORT_0_E + 1,  /* startIdx */
            PRV_CPSS_INT_CAUSE_88E1690_GLOBAL_2_PORT_0_E + 8,  /* endIdx   */
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 8 , NULL, NULL},

            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(1/*bit*/,1/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(2/*bit*/,2/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(3/*bit*/,3/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(4/*bit*/,4/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(5/*bit*/,5/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(6/*bit*/,6/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(7/*bit*/,7/*port*/),
            INTERRUPT_PHY_PORT_88E1690_LEAF_MAC(8/*bit*/,8/*port*/)
};

static GT_U32   MacPhy88E1690_IntrScanArr_elements =
    sizeof(MacPhy88E1690_IntrScanArr)/sizeof(MacPhy88E1690_IntrScanArr[0]);

#define MAC_PHY_88E1690_NUM_MASK_REGISTERS_CNS  \
    (PRV_CPSS_88E1690_LAST_INT_E / (INT_EVENT_STEPS(1)))

/* This array maps an interrupt index to its relevant interrupt mask reg    */
/* default value, cell i indicates the address for interrupts               */
/* (32 * i  --> 32 * (i + 1) - 1).                                          */
/* summary bits are turned on and the specific cause mask is enabled when   */
/* bounded by appl.                                                         */
static GT_U32 MacPhy88E1690_MaskRegDefaultSummaryArr[MAC_PHY_88E1690_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/* This array maps an interrupt index to its    */
/* relevant interrupt mask registers address,   */
/* cell i indicates the address for interrupts  */
/* (32 * i  --> 32 * (i + 1) - 1).              */
static GT_U32 MacPhy88E1690_MaskRegMapArr[MAC_PHY_88E1690_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};


static GT_U32 MacPhy88E1690_UniEvMapTableWithExtData[] = {
 CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
/*port1*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(1)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 1 ,
/*port2*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(2)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 2 ,
/*port3*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(3)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 3 ,
/*port4*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(4)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 4 ,
/*port5*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(5)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 5 ,
/*port6*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(6)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 6 ,
/*port7*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(7)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 7 ,
/*port8*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_LINK_STATUS_CHANGED_E +  INT_EVENT_STEPS(8)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 8 ,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_AN_COMPLETED_E,
/*port1*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(1)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 1 ,
/*port2*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(2)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 2 ,
/*port3*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(3)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 3 ,
/*port4*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(4)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 4 ,
/*port5*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(5)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 5 ,
/*port6*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(6)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 6 ,
/*port7*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(7)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 7 ,
/*port8*/    PRV_CPSS_INT_CAUSE_88E1690_PHY_PORT_0_AN_COMPLETED_E +  INT_EVENT_STEPS(8)   , MARK_REMOTE_PHYSICAL_PORT_INT_CNS | 8 ,
 MARK_END_OF_UNI_EV_CNS
};
/* Interrupt cause to unified event map for BOBK with extended data size */
#define MacPhy88E1690_NUM_UNI_EVENTS_CNS \
    (sizeof(MacPhy88E1690_UniEvMapTableWithExtData)/(sizeof(MacPhy88E1690_UniEvMapTableWithExtData[0])))

/**
* @internal MacPhy88E1690_IntCauseToUniEvConvertDedicatedTables function
* @endinternal
*
* @brief   88e1690 Converts Interrupt Cause event to unified event type.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The portGroupId. support the multi-port-groups device.-port-groups device.
*                                      ignored for non multi-port-groups device.
* @param[in] intCauseIndex            - The interrupt cause to convert.
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @param[out] uniEvPtr                 - (pointer to)The unified event type.
* @param[out] extDataPtr               - (pointer to)The event extended data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS MacPhy88E1690_IntCauseToUniEvConvertDedicatedTables
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr,

    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    GT_U32   ii,jj; /* iterator                     */
    GT_U32   *tableWithExtDataPtr;
    GT_U32   tableWithExtDataSize;
    GT_U32   tmpUniEvent;
    GT_U32   tmpExtData;
    GT_U32   localPort;

    CPSS_NULL_PTR_CHECK_MAC(uniEvPtr);
    CPSS_NULL_PTR_CHECK_MAC(extDataPtr);

    devNum = devNum;
    portGroupId = portGroupId;

    *uniEvPtr   = CPSS_UNI_RSRVD_EVENT_E;
    *extDataPtr = 0;

    /* get the tables */
    tableWithExtDataPtr     = MacPhy88E1690_UniEvMapTableWithExtData;
    tableWithExtDataSize    = MacPhy88E1690_NUM_UNI_EVENTS_CNS;

    ii = 0;
    /* Search the map table for unified event with extended data */
    while (ii < tableWithExtDataSize)
    {
        /* remember the uni event */
        tmpUniEvent = tableWithExtDataPtr[ii++];
        while (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS)
        {
            if (tableWithExtDataPtr[ii] ==  intCauseIndex)
            {
                /* found */
                *uniEvPtr = tmpUniEvent;
                tmpExtData = tableWithExtDataPtr[ii+1];

                if((tmpExtData & MARK_REMOTE_PHYSICAL_PORT_INT_CNS) == MARK_REMOTE_PHYSICAL_PORT_INT_CNS)
                {
                    localPort = CLEAR_3_MSBits_MARK_INT_MAC(tmpExtData);

                    tmpExtData = MARK_END_OF_UNI_EV_CNS;

                    for(jj = 0 ; jj < remotePhyMacInfoPtr->connectedPhyMacInfo.numOfRemotePorts ; jj++)
                    {
                        if(localPort != remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[jj].remoteMacPortNum)
                        {
                            continue;
                        }

                        /* this is the remote physical port number for the local port of the 88e1690 */
                        tmpExtData = remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[jj].remotePhysicalPortNum;
                        /* add the remote physical port indication */
                        tmpExtData |= MARK_REMOTE_PHYSICAL_PORT_INT_CNS;

                        /* see function drvEventExtDataConvert() that need this indication */
                        break;
                    }

                    if(tmpExtData == MARK_END_OF_UNI_EV_CNS)
                    {
                        /* error : the event not matched a remote port */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }
                }

                *extDataPtr = tmpExtData;

                return GT_OK;
            }
            ii +=2;
        }
        ii++;
    }

    PRV_CPSS_DBG_INFO(("file:%s line:%d event not found intCause = %d\n", __FILE__,
              __LINE__, intCauseIndex));

    return /* do not register as error to the LOG */GT_NOT_FOUND;
}

/**
* @internal prvCpssDrvInterruptMaskSet_88e1690 function
* @endinternal
*
* @brief   This function masks/unmasks a given interrupt bit in the relevant
*         interrupt mask register of the 88e1690
*         NOTE: the function is called from 'Application task' (and not from the ISR)
* @param[in] evNode                   - The interrupt node representing the interrupt to be
*                                      unmasked.
* @param[in] operation                - The  to perform, mask or unmask the interrupt
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*
* @note based on prvCpssDrvInterruptMaskSet(...)
*
*/
static GT_STATUS prvCpssDrvInterruptMaskSet_88e1690
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC              *evNode,
    IN CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS   rc;
    GT_U32  maskIdx;
    GT_U32  *intMaskShadow;
    GT_U8   devNum = evNode->devNum;
    GT_U32  regAddr = evNode->intMaskReg;
    GT_U32  dxPortNum       = GET_FROM_INT_REG_dxPortNum(regAddr);
    GT_U8   regAddr88e1690  = GET_FROM_INT_REG_regAddr88e1690(regAddr);
    GT_U8   extParam        = GET_FROM_INT_REG_extParam(regAddr);
    GT_U8   smiDevAddr      = GET_FROM_INT_REG_smiAddr(regAddr);
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,dxPortNum);
    GT_U16  U16_data;

    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    intMaskShadow = portInfoPtr->interrupts.intMaskShadow;

    maskIdx = evNode->intCause / INT_EVENT_STEPS(1);

    if (CPSS_EVENT_MASK_E == operation)
    {
        /* mask the interrupt */
        intMaskShadow[maskIdx] &= ~(evNode->intBitMask);
    }
    else
    {
        /* unmask the interrupt */
        intMaskShadow[maskIdx] |= (evNode->intBitMask);
    }

    U16_data = (GT_U16)(intMaskShadow[maskIdx]);

    if(smiDevAddr == 2/*PHY*/)
    {
        rc = prvCpssDrvHwWritePagedPhyReg(portInfoPtr->drvInfoPtr,extParam/*portNum*/, PHY_PAGE(0) , regAddr88e1690, U16_data);
    }
    else
    {
        rc = prvCpssDrvHwSetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,U16_data);
    }

    return rc;
}

/**
* @internal driverDxExMxInitIntMasks_88e1690 function
* @endinternal
*
* @brief   Set the interrupt mask for a given 88e1690 device.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note based on driverDxExMxInitIntMasks(...)
*
*/
static GT_STATUS driverDxExMxInitIntMasks_88e1690
(
    IN  GT_U8   devNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC     *intReqNodeList;
    GT_U32          *intMaskShadow;
    GT_U32          numOfIntBits;       /* Number of interrupt bits.    */
    GT_U32          i;
    GT_STATUS       rc;

    intMaskShadow   = remotePhyMacInfoPtr->interrupts.intMaskShadow;
    numOfIntBits    = remotePhyMacInfoPtr->interrupts.numOfIntBits;

    intReqNodeList = remotePhyMacInfoPtr->interrupts.intNodesPool;

    for(i = 0; i < (numOfIntBits / INT_EVENT_STEPS(1)); i++)
    {
        if(intReqNodeList[i * INT_EVENT_STEPS(1)].intMaskReg == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }

        {
            GT_U16   U16_data;
            GT_U32  regAddr = intReqNodeList[i * INT_EVENT_STEPS(1)].intMaskReg;
            GT_U32  dxPortNum       = GET_FROM_INT_REG_dxPortNum(regAddr);
            GT_U8   regAddr88e1690  = GET_FROM_INT_REG_regAddr88e1690(regAddr);
            GT_U8   extParam        = GET_FROM_INT_REG_extParam(regAddr);
            GT_U8   smiDevAddr      = GET_FROM_INT_REG_smiAddr(regAddr);
            PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,dxPortNum);

            if(portInfoPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(smiDevAddr)
            {
                case 2/*PHY*/:
                    U16_data = (GT_U16)intMaskShadow[i];
                    rc = prvCpssDrvHwWritePagedPhyReg(portInfoPtr->drvInfoPtr,extParam/*portNum*/, PHY_PAGE(0) , regAddr88e1690, U16_data);
                    break;
                case PRV_CPSS_GLOBAL1_DEV_ADDR:
                    /* Retain non-interrupt mask bits */
                    rc = prvCpssDrvHwGetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,&U16_data);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    U16_data &= ~GT_INT_MASK;
                    intMaskShadow[i] &= GT_INT_MASK;
                    intMaskShadow[i] |= (GT_U32)U16_data;
                    U16_data = (GT_U16)intMaskShadow[i];
                    rc = prvCpssDrvHwSetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,U16_data);
                    break;
                case PRV_CPSS_GLOBAL2_DEV_ADDR:
                    /* Retain non-interrupt mask bits */
                    rc = prvCpssDrvHwGetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,&U16_data);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    U16_data &= ~GT2_INT_MASK;
                    intMaskShadow[i] &= GT2_INT_MASK;
                    intMaskShadow[i] |= (GT_U32)U16_data;
                    U16_data = (GT_U16)intMaskShadow[i];
                    rc = prvCpssDrvHwSetAnyReg(portInfoPtr->drvInfoPtr, smiDevAddr,regAddr88e1690,U16_data);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}


/**
* @internal drvInterruptsInit_88e1690 function
* @endinternal
*
* @brief   Initialize the interrupts mechanism for a given device.
*
* @param[in] devNum                   - The device number to initialize the interrupts
*                                      mechanism for.
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
* @param[in] devIntPtr                - (pointer to) the info about interrupts of the 88e1690 device
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_SUPPORTED         - device not supported
*
* @note based on drvInterruptsInit.
*
*/
static GT_STATUS drvInterruptsInit_88e1690
(
    IN  GT_U8           devNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr,
    IN PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intScanRoot;
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intReqNodeListPtr;/* event node array      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intReqNodePtr = NULL;/* event node pointer */
    GT_U32                  *intMaskShadow;
    GT_U32                  scanArrIdx;
    GT_U32                  i;
    GT_U32                  portGroupId = 0;

    intReqNodeListPtr =
        cpssOsMalloc(devIntPtr->numOfInterrupts * sizeof(PRV_CPSS_DRV_EV_REQ_NODE_STC));
    if(intReqNodeListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* Initialize the Interrupts hierarchy tree.    */
    scanArrIdx = 0;
    rc = prvCpssDrvInterruptScanInit(
                devIntPtr->numOfScanElements,
                devIntPtr->interruptsScanArray,
                &scanArrIdx,
                &intScanRoot);
    if(rc != GT_OK)
    {
        return rc;
    }

    intMaskShadow =
        cpssOsMalloc(sizeof(GT_U32)* (devIntPtr->numOfInterrupts / INT_EVENT_STEPS(1)));
    if(intMaskShadow == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }


    remotePhyMacInfoPtr->interrupts.intMaskShadow = intMaskShadow;
    remotePhyMacInfoPtr->interrupts.intScanRoot = intScanRoot;
    remotePhyMacInfoPtr->interrupts.intNodesPool = intReqNodeListPtr;
    remotePhyMacInfoPtr->interrupts.numOfIntBits  = devIntPtr->numOfInterrupts;


    for(i = 0; i < devIntPtr->numOfInterrupts ; i++)
    {
        intReqNodePtr = &intReqNodeListPtr[i];

        intReqNodePtr->devNum           = devNum;
        intReqNodePtr->portGroupId      = portGroupId;
        intReqNodePtr->nextPtr          = NULL;
        intReqNodePtr->prevPtr          = NULL;
        intReqNodePtr->intStatus        = PRV_CPSS_DRV_EV_DRVN_INT_ACTIVE_E;
        intReqNodePtr->intRecStatus     = PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E;
        intReqNodePtr->intMaskSetFptr   = devIntPtr->intMaskSetFptr;
        intReqNodePtr->intCbFuncPtr     = NULL;
        intReqNodePtr->intCause         = i;
        intReqNodePtr->intMaskReg       = devIntPtr->maskRegistersAddressesArray[i / INT_EVENT_STEPS(1)];
        intReqNodePtr->intBitMask       = 1 << (i % INT_EVENT_STEPS(1));

        if(intReqNodePtr->intMaskReg == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            intReqNodePtr->uniEvCause = CPSS_UNI_RSRVD_EVENT_E;
            intReqNodePtr->uniEvExt   = 0;
        }
        else
        {
            MacPhy88E1690_IntCauseToUniEvConvertDedicatedTables(devNum, portGroupId, i,
                                       &intReqNodePtr->uniEvCause,
                                       &intReqNodePtr->uniEvExt,
                                       remotePhyMacInfoPtr);
        }

        /* Set the interrupt mask registers to their default values */
        if((i % (INT_EVENT_STEPS(1))) == 0)
        {
            intMaskShadow[i / INT_EVENT_STEPS(1)] =
                devIntPtr->maskRegistersDefaultValuesArray[i / INT_EVENT_STEPS(1)];
            /* Set the device's registers that may not be accessed  */
            /* before start init.                                   */
        }
    }

    rc = driverDxExMxInitIntMasks_88e1690(devNum,remotePhyMacInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPort88e1690InitISR function
* @endinternal
*
* @brief   88e1690 Interrupts initialization .
*
* @param[in] devNum                   - the DX device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
static GT_STATUS prvCpssDxChCfgPort88e1690InitISR
(
    IN GT_U8    devNum,
    IN GT_U32   cascadePortNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_INTERRUPT_SCAN_STC *currIntInfoPtr;
    GT_U32  ii,numOfNodes;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC MacPhy88E1690_InterruptInfo;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devInterruptInfoPtr = &MacPhy88E1690_InterruptInfo;

    /* we need to modify per 88e1690 to indication of the 'hiden port number' */
    numOfNodes = MacPhy88E1690_IntrScanArr_elements;
    currIntInfoPtr = &MacPhy88E1690_IntrScanArr[0];

    for(ii = 0 ; ii < numOfNodes; ii++ , currIntInfoPtr++)
    {
        /***********************/
        /* update causeRegAddr */
        /***********************/
        /* update DX port number indication in regAddr*/
        UPDATE_INT_REG_ADDR_dxPortNum(currIntInfoPtr->causeRegAddr,cascadePortNum);

        /**********************/
        /* update maskRegAddr */
        /**********************/
        /* update DX port number indication in regAddr*/
        UPDATE_INT_REG_ADDR_dxPortNum(currIntInfoPtr->maskRegAddr,cascadePortNum);
    }

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_START_DXCH_E,/* !!!! PATCH !!!!
                needed for prvCpssDrvExMxDxHwPpPexAddrGet to bypass 'PEX/PCI' address */
            numOfNodes,
            MacPhy88E1690_IntrScanArr,
            MAC_PHY_88E1690_NUM_MASK_REGISTERS_CNS,
            MacPhy88E1690_MaskRegDefaultSummaryArr,
            MacPhy88E1690_MaskRegMapArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(devInterruptInfoPtr,0,sizeof(*devInterruptInfoPtr));

    devInterruptInfoPtr->numOfInterrupts = PRV_CPSS_88E1690_LAST_INT_E;
    devInterruptInfoPtr->maskRegistersAddressesArray = MacPhy88E1690_MaskRegMapArr;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = MacPhy88E1690_MaskRegDefaultSummaryArr;
    devInterruptInfoPtr->interruptsScanArray = MacPhy88E1690_IntrScanArr;
    devInterruptInfoPtr->numOfScanElements = MAC_PHY_88E1690_NUM_MASK_REGISTERS_CNS;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
/*    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &MacPhy88E1690_IntCauseToUniEvConvertDedicatedTables;*/
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    devInterruptInfoPtr->intMaskSetFptr = prvCpssDrvInterruptMaskSet_88e1690;

    rc = drvInterruptsInit_88e1690(devNum,remotePhyMacInfoPtr,devInterruptInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPort88e1690InitEvents function
* @endinternal
*
* @brief   Init interrupt and events for the 88e1690
*
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690InitEvents
(
    IN GT_U8           devNum,
    IN GT_U32   cascadePortNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
)
{
    GT_STATUS              rc;
    CPSS_EVENT_GPP_ID_ENT  gppId;
    CPSS_EVENT_ISR_FUNC    isrFuncPtr = prvCpssDrvIntEvReqDrvnScan_MacPhy88E1690_fromDxCh;
    PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STC *cookiePtr;
    PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STC *existingCookiePtr;
    GT_U32                  regIndex;/* register index 0..1*/
    GT_U32                  regData;/* register data */

    if(remotePhyMacInfoPtr->connectedPhyMacInfo.interruptMultiPurposePinIndex ==
        0xFFFFFFFF)
    {
        /* the device not connected to MPP GPIO of the interrupts */
        return GT_OK;
    }

    /* per DX device */
    rc = prvCpssDxChCfgPort88e1690InitEvents_dxch(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_INT_SCAN_LOCK();
    /* init the ISR needed tree binding */
    rc = prvCpssDxChCfgPort88e1690InitISR(devNum , cascadePortNum , remotePhyMacInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_INT_SCAN_UNLOCK();


    gppId = CPSS_EVENT_MPP_INTERNAL_0_E +
        remotePhyMacInfoPtr->connectedPhyMacInfo.interruptMultiPurposePinIndex;

    if(gppId > CPSS_EVENT_MPP_INTERNAL_32_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cookiePtr =
        cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PORT_REMOTE_ISR_CB_SCAN_TREE_INFO_STC));
    if(cookiePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cookiePtr->remotePhyMacInfoPtr = remotePhyMacInfoPtr;
    cookiePtr->nextDevicePtr = NULL;

    if(prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr &&
       prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr[gppId] &&
       prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr[gppId]->cookie)
    {
        /* this GPIO already bound by different 88e1690 device */
        /* no need to re-connect ... only need to add 'myself' to the link-list */
        existingCookiePtr = prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr[gppId]->cookie;

        while(existingCookiePtr->nextDevicePtr)
        {
            existingCookiePtr = existingCookiePtr->nextDevicePtr;
        }

        existingCookiePtr->nextDevicePtr = cookiePtr;

        return GT_OK;
    }

    /* Set correct polarity of MPP bits used as interrupt lines */
    regIndex = remotePhyMacInfoPtr->connectedPhyMacInfo.interruptMultiPurposePinIndex >> 5;
    rc = cpssDrvPpHwInternalPciRegRead(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                    0x0001810C + 0x40*regIndex,
                                    &regData);
    if(rc != GT_OK)
    {
        return rc;
    }
    regData |= (1 << (remotePhyMacInfoPtr->connectedPhyMacInfo.interruptMultiPurposePinIndex & 0x1F));
    rc = cpssDrvPpHwInternalPciRegWrite(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                    0x0001810C + 0x40*regIndex,
                                    regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* bind the CB function to the 'driver' of the CPSS to be called from the ISR */
    rc = prvCpssDrvDxExMxGppIsrConnect(devNum,gppId,isrFuncPtr,cookiePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* unmask the MPP in the device */
    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
        CPSS_PP_GPP_E,
        MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS/*100*/ + remotePhyMacInfoPtr->connectedPhyMacInfo.interruptMultiPurposePinIndex,
        CPSS_EVENT_UNMASK_E);
    if(rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}


