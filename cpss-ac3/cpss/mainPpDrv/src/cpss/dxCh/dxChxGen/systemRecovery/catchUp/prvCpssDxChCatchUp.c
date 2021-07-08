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
* @file prvCpssDxChCatchUp.c
*
* @brief CPSS DxCh CatchUp functions.
*
* @version   19
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/generic/private/utils/prvCpssUnitGenArrDrv.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2B0ResourceTables.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

extern GT_STATUS prvCpssDxChPortMacroConrtolDBrecovery
(
    IN   GT_U8      devNum,
    IN   GT_U32     portGroupNum,
    IN   GT_U32     portNum
);

static PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC *auqMsgEnableStatus[PRV_CPSS_MAX_PP_DEVICES_CNS] = {NULL};

static prvCpssCatchUpFuncPtr  catchUpFuncPtrArray[] = {
                                                prvCpssDxChHwDevNumCatchUp,prvCpssDxChCpuPortModeCatchUp,
                                                prvCpssDxChDevTableCatchUp,prvCpssDxChCpuSdmaPortGroupCatchUp,
                                                prvCpssDxChFdbHashParamsModeCatchUp,prvCpssDxChFdbActionHwDevNumActionHwDevNumMaskCatchUp,
                                                prvCpssDxChPrePendTwoBytesCatchUp, prvCpssDxChPolicerMemorySizeModeCatchUp,
                                                prvCpssDxChSecurBreachPortDropCntrModeAndPortGroupCatchUp,prvCpssDxChPortEgressCntrModeCatchUp,
                                                prvCpssDxChBridgeIngressCntrModeCatchUp,prvCpssDxChBridgeIngressDropCntrModeCatchUp,
                                                prvCpssDxChBridgeTag1VidFdbEnCatchUp,prvCpssDxChBridgeEgrVlanPortFltTabAccessModeCatchUp,
                                                prvCpssDxChPortModeParamsCatchUp,prvCpssDxChPortResourcesCatchUp,
                                                prvCpssDxChPortMacCounterOffsetCatchUp,prvCpssDxChCfgPpLpmLogicalInitCatchUp,
                                                prvCpssDxChSharedCncUnitsCatchUp,
                                                NULL
                                                };

GT_STATUS prvCpssDxChNetIfPrePendTwoBytesHeaderFromHwGet
(
    IN  GT_U8        devNum,
    OUT  GT_BOOL    *enablePtr
);


/**
* @internal prvCpssDxChIsPortOob function
* @endinternal
*
* @brief   Check if given Port Mac is an OOB Port (On BobK devices only, on other devices output is GT_FALSE).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMac                  - port Mac
*
* @param[out] isPortOobPtr             - (pointer to) is port OOB:
*                                      GT_TRUE - port is OOB.
*                                      GT_FALSE - port is not OOB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChIsPortOob
(
    IN GT_U8     devNum,
    IN GT_U32    portMac,
    OUT GT_BOOL *isPortOobPtr
)
{
    if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E) )
    {
        GT_STATUS rc;
        GT_U32 serdesTxIfNum;
        GT_U32 tmpData;
        if (portMac == 62)
        {
            /* check if this is network port */
            rc = mvHwsSerdesTxIfSelectGet(devNum,0,20,HWS_DEV_SERDES_TYPE(devNum),&serdesTxIfNum);
           if (rc != GT_OK)
            {
                return rc;
            }
            /* Read bit 12 of a DFX server register. (Serdes 20 MUX to mac 62 by configurting bit[12]  to 0) */
            rc = hwsServerRegGetFuncPtr(devNum, 0xf829c, &tmpData);
            if (rc != GT_OK)
            {
                return rc;
            }
            tmpData  = tmpData & (~0xFFFFEFFF);
            if ( (serdesTxIfNum != 1) || (tmpData != 0) )
            {
                /* OOB port */
                *isPortOobPtr = GT_TRUE;
            }
            else
            {
                *isPortOobPtr = GT_FALSE;
            }
        }
        else
        {
            *isPortOobPtr = GT_FALSE;
        }
    }
    else
    {
        *isPortOobPtr = GT_FALSE;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHwDevNumCatchUp function
* @endinternal
*
* @brief   Synchronize hw device number in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwDevNumCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;
    GT_HW_DEV_NUM oldHwDevNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc =  cpssDxChCfgHwDevNumGet(devNum,&hwDevNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save old hwDevNum */
    oldHwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    /* save actual HW devNum to the DB */
    PRV_CPSS_HW_DEV_NUM_MAC(devNum) = hwDevNum;
    /* for lion2 must set additional data base */
     if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) &&
          (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum) ) )
    {
        /* unmark the old HW dev number to default single mode */
        rc = cpssDxChCscdDbRemoteHwDevNumModeSet(oldHwDevNum,
                                                 CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* mark the new HW dev as daul device in the DB */
        rc = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum,
                                                 CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChCpuPortModeCatchUp function
* @endinternal
*
* @brief   Synchronize cpu port mode in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCpuPortModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regAddr;
    GT_U32 regValue = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* no such bit as there is no special MAC for CPU */
        regValue = 1;/*sdma*/
    }
    else
    {
        if (PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum) == GT_TRUE)
        {
            /* there is erratum : SDMA emulation*/
            regValue = 1; /* sdma*/
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;
            rc = prvCpssHwPpGetRegField(devNum,regAddr,20,1,&regValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    switch(regValue)
    {
        case 0:
            PRV_CPSS_PP_MAC(devNum)->cpuPortMode = CPSS_NET_CPU_PORT_MODE_MII_E;
            break;
        case 1:
            PRV_CPSS_PP_MAC(devNum)->cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal prvCpssDxChCpuSdmaPortGroupCatchUp function
* @endinternal
*
* @brief   Synchronize cpu sdma port group in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCpuSdmaPortGroupCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(!PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        return GT_OK;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            /* Not used any more */
            return GT_OK;
        }
        else
        {
            /* bit 0 - <CpuPortMode> set to 0 'global mode' */
            /* bits 1..4 -  <CpuTargetCore> set to the 'SDMA_PORT_GROUP_ID'  */
            rc = prvCpssHwPpGetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.cpuPortDist,
                    0,5,
                    &regValue);
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.global.cpuPortDistribution;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,3,&regValue);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    if ((regValue & 0x1) == 0)
    {
        /* Global CPU port mode */
        /* update device DB */
        PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId = regValue >> 1;
    }
    else
    {
        /* cpss doesn't support Local CPU port mode */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDevTableCatchUp function
* @endinternal
*
* @brief   Synchronize device table BMP in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDevTableCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    devTableBmp;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.deviceTableBaseAddr;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &devTableBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save device table to the DB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.devTable = devTableBmp;
    return rc;
}

/**
* @internal prvCpssDxChFdbHashParamsModeCatchUp function
* @endinternal
*
* @brief   Synchronize fdb hash params in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbHashParamsModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_MAC_VL_ENT vlanMode;
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc =  cpssDxChBrgFdbMacVlanLookupModeGet(devNum,&vlanMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChBrgFdbHashModeGet(devNum,&hashMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Update FDB hash parameters */
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode = vlanMode;
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode = hashMode;
    return rc;
}

/**
* @internal prvCpssDxChFdbActionHwDevNumActionHwDevNumMaskCatchUp function
* @endinternal
*
* @brief   Synchronize Active device number and active device number mask
*         in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbActionHwDevNumActionHwDevNumMaskCatchUp
(
    IN GT_U8    devNum
)
{
    GT_U32    actDev;
    GT_U32    actDevMask;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc =  cpssDxChBrgFdbActionActiveDevGet(devNum,&actDev,&actDevMask);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update data */
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum = actDev;
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask = actDevMask;
    return rc;
}

/**
* @internal prvCpssDxChPrePendTwoBytesCatchUp function
* @endinternal
*
* @brief   Synchronize enable/disable pre-pending a two-byte header
*         in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPrePendTwoBytesCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_BOOL  readValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* get the value from the HW */
    rc = prvCpssDxChNetIfPrePendTwoBytesHeaderFromHwGet(devNum,&readValue);

    if(rc == GT_OK)
    {
        /* save info to the DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader = readValue;
    }

    return rc;
}

/**
* @internal prvCpssDxChPolicerMemorySizeModeCatchUp function
* @endinternal
*
* @brief   Synchronize Policer Memory Size Mode in software DB by its hw values.
*
* @note   APPLICABLE DEVICES:      Lion;  Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPolicerMemorySizeModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      iplr0MemSize;   /* IPLR 0 memory size                */
    GT_U32      iplr1MemSize;   /* IPLR 1 memory size                */
    GT_U32      eplrMemSize;    /* EPLR  memory size                */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     mode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc =  cpssDxChPolicerMemorySizeModeGet(devNum,&mode,&iplr0MemSize,&iplr1MemSize,&eplrMemSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call the 'Set' .. it will also update the DB */
    rc =  cpssDxChPolicerMemorySizeModeSet(devNum,mode,iplr0MemSize,iplr1MemSize);

    /* update memory size in the policer db */
    /*
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[0] = iplr0MemSize;
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[1] = iplr1MemSize;
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[2] = eplrMemSize;
    */

    return rc;
}

/**
* @internal prvCpssDxChSecurBreachPortDropCntrModeAndPortGroupCatchUp function
* @endinternal
*
* @brief   Synchronize Secure Breach Port Drop Counter Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSecurBreachPortDropCntrModeAndPortGroupCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc = GT_OK;             /* return code */
    GT_U32      regAddr;                /* hw register address */
    GT_U32      portGroupId;            /*the port group Id - support multi-port-groups device */
    GT_U32      regValue = 0;
    GT_U32      fieldOffset;            /* The start bit number in the register */
    GT_U32      fieldLength;            /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
        fieldOffset = 17;
    }
    else
    {
        /* get address of Bridge Configuration Register1 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeGlobalConfigRegArray[1];
            fieldOffset = 19;
    }

    /* called without portGroupId , loop done inside the driver */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &regValue);
    if (rc != GT_OK)
        return rc;
    if (regValue  & 0x1)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                        bridgeEngineConfig.bridgeSecurityBreachDropCntrCfg0;
            fieldOffset = 0;
            fieldLength = 13;
        }
        else
        {
            fieldOffset = 1;
            fieldLength = 6;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, fieldOffset, fieldLength, &regValue);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (regValue != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId = portGroupId;
                break;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortEgressCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Port Egress Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEgressCntrModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32  i;
    GT_U32  cntrSetNum;
    GT_U32 portValue = 0;
    GT_U32 globalPort = 0;
    GT_U32 portGroupId = 0;
    GT_U32 txqNum = 0;
    GT_U32 *regValuePtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    txqNum = PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum);
    regValuePtr = (GT_U32 *)cpssOsMalloc(sizeof(GT_U32) * txqNum);
    if (regValuePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(regValuePtr,0,sizeof(GT_U32)* txqNum);
    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                portEgressCntrModeInfo[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
           continue;
        }
        else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        egrTxQConf.txQCountSet[cntrSetNum].txQConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        txqVer1.queue.peripheralAccess.egressMibCounterSet.config[cntrSetNum];
        }
        for (i =0; i < txqNum; i++)
        {
            rc = prvCpssHwPpPortGroupReadRegister(devNum,
                   i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                   regAddr, &value);
            if(rc != GT_OK)
            {
                cpssOsFree(regValuePtr);
                return rc;
            }
            regValuePtr[i] = value;
        }
        /* check if all values are the same */
        for (i = 0; i < txqNum; i++ )
        {
            if (regValuePtr[0] != regValuePtr[i])
            {
                break;
            }
        }
        if ((i == txqNum) && (txqNum > 1))
        {
            /* it means all values are the same */
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                portEgressCntrModeInfo[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
        else
            if ((txqNum == 1)&&(regValuePtr[0] & 0x1) == 0x0)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                    portEgressCntrModeInfo[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            }
            else
            {
                for (i = 0; i < txqNum; i++ )
                {
                    portValue = (regValuePtr[i]>>4) & 0x3f;
                    if (portValue != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
                    {
                        /* check if it is CPU port */
                        if ((portValue & 0xf) == 0xf)
                        {
                            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                                portEgressCntrModeInfo[cntrSetNum].portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);
                        }
                        else
                        {
                            /* convert port value from port local HEM to global*/
                            globalPort = PRV_CPSS_DXCH_HEM_LOCAL_TO_GLOBAL_PORT(devNum,i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,portValue);
                             /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, globalPort);
                            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                                portEgressCntrModeInfo[cntrSetNum].portGroupId = portGroupId;
                        }
                        break;
                    }
                }
                if (i == txqNum)
                {
                    /* it means that all port values are PRV_CPSS_DXCH_NULL_PORT_NUM_CNS*/
                    cpssOsFree(regValuePtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
    }
    cpssOsFree(regValuePtr);
    return rc;
}

/**
* @internal prvCpssDxChBridgeIngressCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Bridge Ingress Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeIngressCntrModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 regAddr;
    GT_U32 value = 0;
    GT_U32  portGroupId;
    GT_U32  i = 0;
    GT_U32  entriesCounter = 0;
    GT_U32 cntrSetNum = 0;
    GT_U32 regValue[8][2] = {{0,0}};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        entriesCounter = 0;
        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                bridgeIngressCntrMode[cntrSetNum].portGroupId  = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            continue;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.
                                             brgCntrSet[cntrSetNum].cntrSetCfg;
        /* loop on all port groups :
            on the port group that 'own' the port , set the needed configuration
            on other port groups put 'NULL port'
        */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,regAddr,&value);
            if(rc != GT_OK)
            {
                return rc;
            }
            regValue[entriesCounter][0] = value;
            regValue[entriesCounter][1] = portGroupId;
            entriesCounter++;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

        /* check if all values are the same */
        for (i = 0; i < entriesCounter; i++ )
        {
            if (regValue[0][0] != regValue[i][0])
            {
                break;
            }
        }

        if ((i == entriesCounter) && (entriesCounter > 1))
        {
             /* it means all values are the same */
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                    bridgeIngressCntrMode[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
        else
        {
            if ((entriesCounter == 1) && ((regValue[0][0] & 0x3) == 0x1))
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                    bridgeIngressCntrMode[cntrSetNum].portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            }
            else
            {
                for (i = 0; i < entriesCounter; i++ )
                {
                    regValue[i][0] = (value >> 2) & 0x3f;

                    if (regValue[i][0] != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
                    {
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                                bridgeIngressCntrMode[cntrSetNum].portGroupId = regValue[i][1];
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChBridgeIngressDropCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Ingress Drop Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeIngressDropCntrModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32  portGroupId;
    GT_U32  i = 0;
    GT_U32  entriesCounter = 0;
    GT_U32 regValue[8][2] = {{0,0}};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    entriesCounter = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
         PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
         return GT_OK;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.ingressDropCntrConfReg;
    }

    /* loop on all port groups :
    */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,regAddr,&value);
        if(rc != GT_OK)
        {
            return rc;
        }
        regValue[entriesCounter][0] = value;
        regValue[entriesCounter][1] = portGroupId;
        entriesCounter++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    /* check if all values are the same */
    for (i = 0; i < entriesCounter; i++ )
    {
        if (regValue[0][0] != regValue[i][0])
        {
            break;
        }
    }
    if ((i == entriesCounter) && (entriesCounter > 1))
    {
        /* it means all values are the same */
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        if ((entriesCounter == 1) && ((regValue[0][0] & 0x3) != 0x2))
        {
            /* it is not port mode */
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
        else
        {
            for (i = 0; i < entriesCounter; i++ )
            {
                regValue[i][0] =  (regValue[i][0] >> 2) & 0xfff;
                if (regValue[i][0] != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
                {
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId = regValue[i][1];
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChBridgeTag1VidFdbEnCatchUp function
* @endinternal
*
* @brief   Synchronize tag1VidFdbEn in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeTag1VidFdbEnCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_BOOL     enable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    /* get the value from the HW */
    rc = cpssDxChBrgFdbVid1AssignmentEnableGet(devNum,&enable);

    if(rc == GT_OK)
    {
        /* save info to the DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.tag1VidFdbEn = enable;
    }

    return rc;
}

/**
* @internal prvCpssDxChBridgeEgrVlanPortFltTabAccessModeCatchUp function
* @endinternal
*
* @brief   Synchronize vlanfltTabAccessMode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeEgrVlanPortFltTabAccessModeCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    /* get the value from the HW */
    rc = cpssDxChBrgEgrFltVlanPortAccessModeGet(devNum,&fltTabAccessMode);

    if(rc == GT_OK)
    {
        /* save info to the DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.portVlanfltTabAccessMode = fltTabAccessMode;
    }

    return rc;
}

/**
* @internal prvCpssDxChPortModeParamsCatchUp function
* @endinternal
*
* @brief   Synchronize Port Mode parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortModeParamsCatchUp
(
    IN GT_U8    devNum
)
{
    GT_PHYSICAL_PORT_NUM port;
    GT_STATUS rc = GT_OK;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT            speed;
    GT_BOOL isValid;
    GT_U32 portMac;
    GT_U32 portSpeedInMBit;
    GT_BOOL linkDownState = GT_FALSE;
    GT_BOOL isPortOob;
    GT_BOOL cpuPortIsMapped = GT_FALSE;
    GT_BOOL apEnable = GT_FALSE;
    CPSS_DXCH_PORT_AP_PARAMS_STC apParams;
    GT_U32 maxNumberOfPorts = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);
    /* Skip for Bobk and Aldrin, AC3X, because SerDes firmware not loaded in simulation */
    PRV_CPSS_DXCH_CATCHUP_RETURN_IF_DEV_IS_BOBK_OR_ALDRIN_SIMULATION_MAC(devNum);
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        maxNumberOfPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    }
    else
    {
        maxNumberOfPorts = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
    }

    /* loop over all GE and FE ports */
    for (port = 0; port < maxNumberOfPorts; port++)
    {
        if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, port, &isValid);
            if((rc != GT_OK) || (isValid != GT_TRUE))
            {
                continue;
            }
        }
        else
        {
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, port);
        }

        if ((port == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            /* cpu port */
            cpuPortIsMapped = GT_TRUE;
            continue;
        }

        /* BC3 has special 3 additional CPU SDMAs */
        if(((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)) &&
                (port == 80 || port == 81 || port == 82))
        {
            continue;
        }

        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMac);
        rc = prvCpssDxChIsPortOob(devNum, portMac, &isPortOob);
        if ( rc != GT_OK)
        {
            return rc;
        }
        else if ( isPortOob )
        {
            continue;
        }

#ifdef ASIC_SIMULATION
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if(port < 24 && port % 4 == 0)
            {
                ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
                speed = CPSS_PORT_SPEED_100G_E;
            }
            else if(port < 24)
            {
                ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                speed = CPSS_PORT_SPEED_NA_E;
            }
            else
            {
                ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                speed = CPSS_PORT_SPEED_10000_E;
            }
        }
        else
#endif
        {
            rc =  prvCpssDxChPortInterfaceModeHwGet(devNum, port, &ifMode);
            if (rc == GT_NOT_INITIALIZED)
            {
                continue;
            }
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChPortSpeedHwGet(devNum, port, &speed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* portType updated inside prvCpssDxChPortInterfaceModeHwGet */
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;

        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = speed;
        if((CPSS_PORT_SPEED_2500_E == speed) &&
            (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode))
        {
            /* SGMII 2.5G in HW implemented as 1000BaseX */
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode =
                                            CPSS_PORT_INTERFACE_MODE_SGMII_E;
        }

        /* recover Egf force status */
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portEgfLinkStatusState;
            if (PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "portEgfForceStatusBitmap allocation failed");
            }

            rc = cpssDxChBrgEgrFltPortLinkEnableGet(devNum, port, &portEgfLinkStatusState);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChBrgEgrFltPortLinkEnableGet failed=%d ", rc);
            }

            if (portEgfLinkStatusState == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, port);
            }
            else
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, port);
            }
        }

        /* recover speed for pizza */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            /* SIP5 devices that support QSGMII */
            if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)) ||
                ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)) ||
                ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)))
            {
                if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
                {
                    /* check if port is link down. Only one port of 4 that is not force link down  */
                    /* get QSGMII configuration others - NA                                        */
                    rc =  cpssDxChPortForceLinkDownEnableGet(devNum,port,&linkDownState);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    if (linkDownState == GT_TRUE)
                    {
                        ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;
                        speed = CPSS_PORT_SPEED_NA_E;
                        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = speed;
                    }
                }
                else if ( (ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E) )
                {
                    if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) && (port <= 47)) ||
                        ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) && (port >= 8)))
                    {
                        if (port != (port & 0xFFFFFFFC))
                        {
                            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;
                            speed = CPSS_PORT_SPEED_NA_E;
                            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = speed;
                        }
                    }
                }
            }
            if ( (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) && (speed != CPSS_PORT_SPEED_NA_E) )
            {
                rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(devNum,port,speed,/*OUT*/&portSpeedInMBit);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (port >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* update pizza DB */
                PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.prv_portDB[port].isInitilized = GT_TRUE;
                PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.prv_portDB[port].portSpeedInMBit = portSpeedInMBit;
                PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.numInitPorts++;
            }
        }

        if (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            if((ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) &&
               (speed != CPSS_PORT_SPEED_NA_E))
            {
                rc = prvCpssDxChLion2PortTypeSet(devNum, port, ifMode, speed);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                prvCpssDxChPortTypeSet(devNum, port, ifMode, speed);
            }
        }

        if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
             (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) )
        {
            /* check if port is AP */
            if ( (speed == CPSS_PORT_SPEED_NA_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) )
            {
                cpssOsMemSet(&apParams, 0, sizeof(CPSS_DXCH_PORT_AP_PARAMS_STC));
                rc = cpssDxChPortApPortConfigGet(devNum,port,&apEnable,&apParams);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (apEnable == GT_TRUE)
                {
                     PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = apParams.modesAdvertiseArr[0].ifMode;
                     PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = apParams.modesAdvertiseArr[0].speed;
                }
            }
        }

        /* Macro control DB Recovery for Aldrin 2 */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
#if 0 /* Ignore Serdes MUX */
            rc = prvCpssDxChPortMacroConrtolDBrecovery(devNum, 0, port);
            if (rc != GT_OK)
            {
                return rc;
            }
#endif
        }

        /* update addresses of mac registers accordingly to used MAC 1G/XG/XLG */
        rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, port, ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if (CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E == systemRecoveryInfo.systemRecoveryProcess)
    {
        if ( ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE ) && (cpuPortIsMapped == GT_TRUE) )
        {
            rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, CPSS_CPU_PORT_NUM_CNS, CPSS_PORT_SPEED_1000_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortResourcesBobkCatchUp function
* @endinternal
*
* @brief   Synchronize Port resources parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortResourcesBobkCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    #ifdef GM_USED
      devNum = devNum;
    #else
        GT_PHYSICAL_PORT_NUM port;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC resource0;
        PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC    resource1;
        GT_U32  *txQDescrCreditsPtr = NULL;
        GT_U32  *txFifoHeaderCreditsPtr = NULL;
        GT_U32  *txFifoPayloadCreditsPtr = NULL;
        GT_U32  dpIndex;
        GT_U32  maxDp;
        CPSS_PORT_SPEED_ENT speed;
        GT_U32 portMac;
        GT_U32 speedValueMbps;
        GT_U32 speedIndex;
        GT_BOOL tmPortEnable = GT_FALSE;
        GT_BOOL isPortOob;
        PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
              CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
              CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E );
        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_TRUE)
        {
            /* tm is not supported */
            return GT_OK;
        }
        maxDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
        if (maxDp == 0)
        {
            /* we deal with single data path */
            maxDp = 1;
        }
        for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
        {
            /* zero overall summary speed */
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex] = 0;
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex] = 0;
            /* zero the credits in shadow */
            rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, dpIndex,
                                                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex],
                                                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex],
                                                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        txQDescrCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txQDescrCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        txFifoHeaderCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txFifoHeaderCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        txFifoPayloadCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txFifoPayloadCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(txQDescrCreditsPtr, 0, sizeof(GT_U32)*maxDp);
        cpssOsMemSet(txFifoHeaderCreditsPtr, 0, sizeof(GT_U32)*maxDp);
        cpssOsMemSet(txFifoPayloadCreditsPtr, 0, sizeof(GT_U32)*maxDp);

        /* loop over all GE and FE ports */
        for (port = 0; port < CPSS_MAX_PORTS_NUM_CNS; port++)
        {
            /* Get Port Mapping DataBase */
            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, port, &portMapShadowPtr);
            if(rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
            if ((portMapShadowPtr->valid == GT_FALSE) || (portMapShadowPtr->portMap.mappingType >= CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E) )
            {
                continue;
            }
            if ( (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE) && (tmPortEnable == GT_TRUE) )
            {
                continue;
            }
            if (portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE)
            {
                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, portMapShadowPtr->portMap.txDmaNum, /*OUT*/&dpIndex, NULL);
                if(GT_OK != rc)
                {
                    cpssOsFree(txQDescrCreditsPtr);
                    cpssOsFree(txFifoHeaderCreditsPtr);
                    cpssOsFree(txFifoPayloadCreditsPtr);
                    return rc;
                }
            }
            else
            {
                dpIndex = 1;
            }
            /*PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMac);*/
            if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                portMac = portMapShadowPtr->portMap.macNum;
                rc = prvCpssDxChIsPortOob(devNum, portMac, &isPortOob);
                if ( rc != GT_OK)
                {
                    return rc;
                }
                else if ( isPortOob )
                {
                    continue;
                }
                /* get the port speed */
                speed = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed;
            }
            else
            {
                speed = CPSS_PORT_SPEED_1000_E;
            }
            if (speed == CPSS_PORT_SPEED_NA_E)
            {
                /* port is not used */
                continue;
            }
            if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E) )
            {
                /* convert speed to Mbps value */
                rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndex,speed,&speedIndex,&speedValueMbps);
                if (rc != GT_OK)
                {
                    cpssOsFree(txQDescrCreditsPtr);
                    cpssOsFree(txFifoHeaderCreditsPtr);
                    cpssOsFree(txFifoPayloadCreditsPtr);
                    return rc;
                }
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex] += speedValueMbps;

                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex] =
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex];
                rc = prvCpssDxChCaelumPortResourceConfigGet(devNum,port,&resource0);
                if (rc != GT_OK)
                {
                    cpssOsFree(txQDescrCreditsPtr);
                    cpssOsFree(txFifoHeaderCreditsPtr);
                    cpssOsFree(txFifoPayloadCreditsPtr);
                    return rc;
                }
                txQDescrCreditsPtr[dpIndex]      += resource0.txdmaCreditValue;
                txFifoHeaderCreditsPtr[dpIndex]  += resource0.txdmaTxfifoHeaderCounterThresholdScdma;
                txFifoPayloadCreditsPtr[dpIndex] += resource0.txdmaTxfifoPayloadCounterThresholdScdma;
            }
            else
            {
                /* convert speed to Mbps value */
                prvCpssDxChBcat2_B0_PortResources_SpeedIdxGet(speed,&speedValueMbps);
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex] += speedValueMbps;
                rc = prvCpssDxChBobcat2PortResourseConfigGet(devNum,port,&resource1);
                if (rc != GT_OK)
                {
                    cpssOsFree(txQDescrCreditsPtr);
                    cpssOsFree(txFifoHeaderCreditsPtr);
                    cpssOsFree(txFifoPayloadCreditsPtr);
                    return rc;
                }
                txQDescrCreditsPtr[dpIndex] += resource1.txdmaTxQCreditValue;
                txFifoHeaderCreditsPtr[dpIndex] += resource1.txdmaTxfifoHeaderCounterThresholdScdma;
                txFifoPayloadCreditsPtr[dpIndex] += resource1.txdmaTxfifoPayloadCounterThresholdScdma;
            }
            if (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
            {
                tmPortEnable = GT_TRUE;
            }

        }
        if (tmPortEnable == GT_TRUE)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] +=
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1]=
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1];
        }
        else
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps = 0;
        }


        for (dpIndex=0; dpIndex < maxDp; dpIndex++)
        {
            /* update credits in shadow */
            rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, dpIndex, txQDescrCreditsPtr[dpIndex],
                                                     txFifoHeaderCreditsPtr[dpIndex], txFifoPayloadCreditsPtr[dpIndex]);
            if (rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
        }
        cpssOsFree(txQDescrCreditsPtr);
        cpssOsFree(txFifoHeaderCreditsPtr);
        cpssOsFree(txFifoPayloadCreditsPtr);

    #endif

    return rc;
}


/**
* @internal prvCpssDxChPortResourcesCatchUp function
* @endinternal
*
* @brief   Synchronize Port resources parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* Skip for Bobk and Aldrin, AC3X, because SerDes firmware not loaded in simulation */
    PRV_CPSS_DXCH_CATCHUP_RETURN_IF_DEV_IS_BOBK_OR_ALDRIN_SIMULATION_MAC(devNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE) ||
        (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_TRUE) ||
        (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == GT_TRUE))
    {
        /* BobK, Bobcat2 or Bobcat3 port resources catch up */
        rc = prvCpssDxChPortResourcesBobkCatchUp(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* port resources are not supported in Bobcat2 A0 - catchup should do nothing*/
        rc = GT_OK;
    }

    return rc;
}

/**
* @internal prvCpssDxChSerdesPolarityCatchUp function
* @endinternal
*
* @brief   Synchronize Serdes polarity parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSerdesPolarityCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32      lanesNumInDev;      /* number of serdes lanes in current device. */
    GT_U32      laneNum;            /* for loop iterator. */
    GT_U32      memSize;            /* SW DB memory size. */
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* Skip for Bobk and Aldrin simulation, because SerDes firmware not loaded in simulation */
    PRV_CPSS_DXCH_CATCHUP_RETURN_IF_DEV_IS_BOBK_OR_ALDRIN_SIMULATION_MAC(devNum);

    if ( !PRV_CPSS_SIP_5_CHECK_MAC(devNum) )
    {
        return GT_OK;
    }
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_BOBCAT3
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);
    /* allocate SW DB. */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr == NULL)
    {
        memSize = sizeof(PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC) * lanesNumInDev;
        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr = (PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC *)cpssOsMalloc(memSize);
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /* Zero the Tx/Rx polarity SW DB for all lanes */
        cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr, 0, memSize);
    }
    /* get Serdes polarity values from HW and upadte SW DB. */
    for (laneNum = 0; laneNum < lanesNumInDev; laneNum++ )
    {
        rc = mvHwsSerdesPolarityConfigGet(devNum, 0, laneNum, HWS_DEV_SERDES_TYPE(devNum), &invertTx, &invertRx);
        /* Because it is not known which lanes are used, we call mvHwsSerdesPolarityConfigGet with all possible lane values.
           rc != GT_OK is probably caused by bad lane num and not an actual error, because of this we have a validity array
           isSerdesPolarityLaneNumValidPtr[] for each lane num. */
        if (rc == GT_OK)
        {
            if ( ppConfigCatchUpParams[devNum].isValidityCheckEnabled )
            {
                ppConfigCatchUpParams[devNum].isSerdesPolarityLaneNumValidPtr[laneNum] = GT_TRUE;
            }
            PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNum].txPolarity = invertTx;
            PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNum].rxPolarity = invertRx;
        }
        else
        {
            if ( ppConfigCatchUpParams[devNum].isValidityCheckEnabled )
            {
                ppConfigCatchUpParams[devNum].isSerdesPolarityLaneNumValidPtr[laneNum] = GT_FALSE;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacCounterOffsetCatchUp function
* @endinternal
*
* @brief   Set port Mac counter offset parameters in software
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacCounterOffsetCatchUp
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    return cpssDxChPortStatInit(devNum);
}

/**
* @internal prvCpssDxChCfgPpLpmLogicalInitCatchUp function
* @endinternal
*
* @brief   Synchronize LPM index of start entry for PBR and LPM memory mode
*         parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgPpLpmLogicalInitCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32      indexForPbr;
    GT_U32      lpmMemMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if ( PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) )
    {
        if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ) /* Bobcat3 */
        {
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15,
                                        25, 1, &lpmMemMode);
            if (rc == GT_OK)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode =
                    (lpmMemMode == 1) ? PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E : PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
            }
            else
            {
                return rc;
            }
        }
        rc = prvCpssHwPpGetRegField(devNum,
                                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.directAccessMode,
                                    0, 19, &indexForPbr);
        if (rc == GT_OK)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr = indexForPbr;
        }
        else
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChSharedCncUnitsCatchUp function
* @endinternal
*
* @brief   Synchronize info about the shared CNC units between port groups control
*         in SW DB.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSharedCncUnitsCatchUp
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) )
    {
        /* relevant struct is PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC */
        return GT_OK;
    }
    else
    {
        return GT_OK;
    }
}

/**
* @internal dxChAuFuPtrUpdate function
* @endinternal
*
* @brief   The function scan the AU/FU queues and update AU/FU software queue pointer.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxChAuFuPtrUpdate
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType
)
{
    PRV_CPSS_AU_DESC_STC        *descPtr;          /*pointer to the current descriptor*/
    PRV_CPSS_AU_DESC_STC        *descBlockPtr;     /* AU descriptors block */
    PRV_CPSS_AU_DESC_CTRL_STC   *descCtrlPtr;      /* pointer to the descriptors DB of the device */
    PRV_CPSS_AU_DESC_CTRL_STC   *descCtrl1Ptr = 0; /* pointer to the descriptors DB
                                                      for additional primary AUQ of the device */
    GT_U32                      ii;                /* iterator */
    GT_U32                      auMessageNumBytes;
    GT_U32                      auMessageNumWords;
    GT_U32                      numberOfQueues = 1; /* number of queues */
    GT_U32                      currentQueue;       /* iterator */
    GT_BOOL                     useDoubleAuq;       /* support configuration of two AUQ memory regions */
    PRV_CPSS_AU_DESC_EXT_8_STC  *descExtPtr;        /*pointer to the current descriptor*/
     GT_U32                     *auMemPtr = NULL;   /* pointer to start of current message */
    CPSS_MAC_ENTRY_EXT_KEY_STC  macEntry;
    GT_U32                      portGroupsBmp;
    GT_U32                      qa_counter;
    GT_PORT_GROUPS_BMP          completedPortGroupsBmp = 0;
    GT_PORT_GROUPS_BMP          succeededPortGroupsBmp = 0;
    CPSS_MAC_UPDATE_MSG_EXT_STC auFuMessage;
    GT_STATUS rc = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;
    tempSystemRecoveryInfo.systemRecoveryState = systemRecoveryInfo.systemRecoveryState;

    cpssOsMemSet(&macEntry,0,sizeof(CPSS_MAC_ENTRY_EXT_KEY_STC));
    cpssOsMemSet(&auFuMessage,0,sizeof(CPSS_MAC_UPDATE_MSG_EXT_STC));
    macEntry.entryType                      = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.macVlan.vlanId             = 0;
    macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

    auMessageNumWords = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;
    auMessageNumBytes = 4 * auMessageNumWords;

    useDoubleAuq = PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.useDoubleAuq;

    switch (queueType)
    {
    case MESSAGE_QUEUE_PRIMARY_FUQ_E:
        descCtrlPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId]);
        break;
    case MESSAGE_QUEUE_PRIMARY_AUQ_E:
        descCtrlPtr = (PRV_CPSS_AUQ_INDEX_MAC(devNum, portGroupId) == 0) ?
            &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]) :
            &(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl[portGroupId]);

        if (useDoubleAuq == GT_TRUE)
        {
            numberOfQueues = 2;
            descCtrl1Ptr = (PRV_CPSS_AUQ_INDEX_MAC(devNum, portGroupId) == 1) ?
                &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]) :
                &(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl[portGroupId]);
        }
        break;
    case MESSAGE_QUEUE_SECONDARY_AUQ_E:
        descCtrlPtr = (PRV_CPSS_SECONDARY_AUQ_INDEX_MAC(devNum, portGroupId) == 0) ?
            &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId]) :
            &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl[portGroupId]);
        if (useDoubleAuq == GT_TRUE)
        {
            numberOfQueues = 2;
            descCtrl1Ptr = (PRV_CPSS_SECONDARY_AUQ_INDEX_MAC(devNum, portGroupId) == 1) ?
                &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId]) :
                &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl[portGroupId]);
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* get address of AU descriptors block */
    descBlockPtr = (PRV_CPSS_AU_DESC_STC*)(descCtrlPtr->blockAddr);

    /* pointer to the current descriptor */
    descPtr = &(descBlockPtr[descCtrlPtr->currDescIdx]);
    PRV_CPSS_INT_SCAN_LOCK();
    for(currentQueue = 0; currentQueue < numberOfQueues; currentQueue++)
    {
        /* Second iteration for additional AUQ */
        if(currentQueue == 1)
        {
             descCtrlPtr = descCtrl1Ptr;
             descPtr = &(descBlockPtr[descCtrlPtr->currDescIdx]);
        }
        for (ii = descCtrlPtr->currDescIdx; ii < descCtrlPtr->blockSize; ii++ ,descPtr++)
        {
            if(!(AU_DESC_IS_NOT_VALID(descPtr)))
            {
                /* no more not valid descriptors */
                break;
            }
            /* increment software descriptor pointer*/
             descCtrlPtr->currDescIdx =
                (( descCtrlPtr->currDescIdx + 1) %  descCtrlPtr->blockSize);
        }
        if (ii == descCtrlPtr->blockSize)
        {
            descCtrlPtr->currDescIdx = 0;
            descPtr = &(descBlockPtr[descCtrlPtr->currDescIdx]);
            if (queueType == MESSAGE_QUEUE_PRIMARY_AUQ_E)
            {
                /*In this case there is no new real message in the queue. In order to understand where software pointer is*/
                /* quary is sent from cpu to pp and pp will reply to cpu with quary response and this response should be found*/
                /* in AUQ*/
                portGroupsBmp = 0;
                portGroupsBmp = portGroupsBmp | (1<<portGroupId);

                qa_counter = 0;
                systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
                do
                {
                    rc =  cpssDxChBrgFdbPortGroupQaSend( devNum, portGroupsBmp,&macEntry);
                     if(rc != GT_OK)
                     {
                #ifdef ASIC_SIMULATION
                         cpssOsTimerWkAfter(1);
                #endif
                         qa_counter++;
                         if(qa_counter > 20)
                         {
                             PRV_CPSS_INT_SCAN_UNLOCK();
                             systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                             return rc;
                         }
                     }
                 } while (rc != GT_OK);
                 systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                 /* verify that action is completed */
                 do
                 {
                     rc = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(devNum,portGroupsBmp,&completedPortGroupsBmp,
                                                                       &succeededPortGroupsBmp);
                     if(rc != GT_OK)
                     {
                         PRV_CPSS_INT_SCAN_UNLOCK();
                         return rc;
                     }
                 }
                 while ((completedPortGroupsBmp & portGroupsBmp)!= portGroupsBmp);
                 /* now perform search again */
                 for (ii = descCtrlPtr->currDescIdx; ii < descCtrlPtr->blockSize; ii++ ,descPtr++)
                 {
                     if(!(AU_DESC_IS_NOT_VALID(descPtr)))
                     {
                         /* no more not valid descriptors */
                         break;
                     }
                     /* increment software descriptor pointer*/
                      descCtrlPtr->currDescIdx =
                         (( descCtrlPtr->currDescIdx + 1) %  descCtrlPtr->blockSize);
                 }

                 if (ii == descCtrlPtr->blockSize)
                 {
                     descCtrlPtr->currDescIdx = 0;
                 }
                 else
                 {
                     /* entry was found */
                     /* check that this entry is QR and delete it*/
                     /* the pointer to start of 'next message to handle'  */
                     auMemPtr = (GT_U32 *)(descCtrlPtr->blockAddr + (auMessageNumBytes * descCtrlPtr->currDescIdx));
                     descExtPtr= (PRV_CPSS_AU_DESC_EXT_8_STC*)auMemPtr;
                     rc = auDesc2UpdMsg(devNum, portGroupId ,descExtPtr , GT_TRUE, &auFuMessage);
                     if(rc != GT_OK)
                     {
                         return rc;
                     }
                     if ( (auFuMessage.macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)        &&
                          (0 == cpssOsMemCmp((GT_VOID*)&auFuMessage.macEntry.key.key.macVlan.macAddr,
                                             (GT_VOID*)&macEntry.key.macVlan.macAddr,
                                             sizeof (GT_ETHERADDR)))                                        &&
                          (auFuMessage.macEntry.key.key.macVlan.vlanId  == macEntry.key.macVlan.vlanId) )
                     {
                         AU_DESC_RESET_MAC(descPtr);
                         descCtrlPtr->currDescIdx++;
                     }
                 }
            }
        }
    }
    PRV_CPSS_INT_SCAN_UNLOCK();
    return GT_OK;
}


/**
* @internal dxChHaAuFuSameMemCatchUp function
* @endinternal
*
* @brief   Synchronize software DB AU/FU pointers by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxChHaAuFuSameMemCatchUp
(
    IN GT_U8    devNum
)
{
    MESSAGE_QUEUE_ENT             queueType;
    GT_U32                        portGroupId;
    GT_STATUS                     rc = GT_OK;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId].blockAddr != 0)
        {
            /* handle AUQs for this portGroup */
            queueType = MESSAGE_QUEUE_PRIMARY_AUQ_E;
            rc = dxChAuFuPtrUpdate(devNum, portGroupId, queueType);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId].blockAddr != 0)
        {
            /* handle secondary AUQs for this portGroup */
            queueType = MESSAGE_QUEUE_SECONDARY_AUQ_E;
            rc = dxChAuFuPtrUpdate(devNum,portGroupId, queueType);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId].blockAddr != 0)
        {
            /* handle FUQs for this portGroup */
            queueType = MESSAGE_QUEUE_PRIMARY_FUQ_E;
            rc = dxChAuFuPtrUpdate(devNum,portGroupId, queueType);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}
/**
* @internal prvCpssDxChSystemRecoveryCatchUpSameMemoryAuFuHandle function
* @endinternal
*
* @brief   Synchronize AUQ/FUQ software pointers by its hw values.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function could be used only if application guarantees constant
*       surviving cpu restart memory for AUQ/FUQ allocation.
*
*/
static GT_STATUS prvCpssDxChSystemRecoveryCatchUpSameMemoryAuFuHandle
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;/* pointer to the module
                                                configure of the PP's database*/
    GT_U32 portGroupId;
    GT_STATUS rc = GT_OK;
    GT_UINTPTR                  phyAddr;/* The physical address of the AU block*/
    GT_UINTPTR virtAddr;
    GT_U32 queueSize;
    GT_U32 regAddr;
    GT_U32 auMessageNumBytes; /* number of bytes in AU/FU message */
    GT_U32 auqTotalSize = 0;      /* auq total size of all port groups*/
    GT_U32 fuqTotalSize = 0;      /* fuq total size of all port groups*/
    GT_U32 secondaryAuqDescBlockSize;
    GT_U32 secondaryAuqDescBlockPtr;

    PRV_CPSS_AU_DESC_CTRL_STC    *auDescCtrlPtr = NULL;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* Configure the module configruation struct.   */
    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    if (moduleCfgPtr->useDoubleAuq == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    auMessageNumBytes = 4 * PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;
    /* restore AUQ/FUQ DB */
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl));
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl));
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl));
    cpssOsMemSet(&(PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa),0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa));
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex));
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl));
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl,0,sizeof(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl));

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        auDescCtrlPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;

        rc = prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)&phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPhy2Virt(phyAddr,&virtAddr);
        auDescCtrlPtr->blockAddr = virtAddr;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
        rc = prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,&queueSize);
        if (rc != GT_OK)
        {
            return rc;
        }
        auDescCtrlPtr->blockSize = queueSize;
        auqTotalSize += queueSize;
        auDescCtrlPtr->currDescIdx = 0;
        auDescCtrlPtr->unreadCncCounters = 0;     

        if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E) == GT_TRUE)
        {
            secondaryAuqDescBlockSize = CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS;
            secondaryAuqDescBlockPtr = auDescCtrlPtr->blockAddr + queueSize * auMessageNumBytes;
            /* Set Secondary AUQ bloack size and address */
            PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId].blockAddr = secondaryAuqDescBlockPtr;
            PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId].blockSize = secondaryAuqDescBlockSize;
        }

        if (moduleCfgPtr->fuqUseSeparate == GT_TRUE)
        {
            auDescCtrlPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId]);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
            rc = prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)&phyAddr);
            if (rc != GT_OK)
            {
                return rc;
            }
            cpssOsPhy2Virt(phyAddr,&virtAddr);
            auDescCtrlPtr->blockAddr = virtAddr;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
            rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,0,31,&queueSize);/*num of entries in the block*/
            if (rc != GT_OK)
            {
                return rc;
            }
            auDescCtrlPtr->blockSize = queueSize;
            fuqTotalSize += queueSize;
            auDescCtrlPtr->currDescIdx = 0;
            auDescCtrlPtr->unreadCncCounters = 0;
        }
        /* Set primary AUQ index */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroupId] = 0;
        /* Set primary AUQ init state - 'FULL';
        When WA triggered for the first time - all primary AUQs are full */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].primaryState =
            PRV_CPSS_AUQ_STATE_ALL_FULL_E;
        /* Set secondary AUQ index */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].activeSecondaryAuqIndex = 0;
        /* Set secondary AUQ state - 'EMPTY' */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].secondaryState =
            PRV_CPSS_AUQ_STATE_ALL_EMPTY_E;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    moduleCfgPtr->auCfg.auDescBlock = (GT_U8*)PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[0].blockAddr;
    moduleCfgPtr->auCfg.auDescBlockSize = auqTotalSize * auMessageNumBytes;
    if (moduleCfgPtr->fuqUseSeparate == GT_TRUE)
    {
        moduleCfgPtr->fuCfg.fuDescBlock = (GT_U8*)PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[0].blockAddr;
        moduleCfgPtr->fuCfg.fuDescBlockSize = fuqTotalSize * auMessageNumBytes;
    }

    /* perform auq/fuq sw pointer tuning */
    rc = dxChHaAuFuSameMemCatchUp(devNum);
    return rc;
}

/**
* @internal prvCpssDxChSystemRecoveryCatchUpDiffMemoryAuHandle function
* @endinternal
*
* @brief   Synchronize AUQ software pointers by its hw values.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function could be used only if application couldn't guarantee constant
*       surviving cpu restart memory for AUQ/FUQ allocation.
*
*/
static GT_STATUS prvCpssDxChSystemRecoveryCatchUpDiffMemoryAuHandle
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc =  prvCpssDxChRestoreAuqCurrentStatus(devNum);
    return rc;
}

/**
* @internal prvCpssDxChSystemRecoveryCatchUpDiffMemoryFuHandle function
* @endinternal
*
* @brief   Synchronize FUQ software pointers by its hw values.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function could be used only if application couldn't guarantee constant
*       surviving cpu restart memory for AUQ/FUQ allocation.
*
*/
static GT_STATUS prvCpssDxChSystemRecoveryCatchUpDiffMemoryFuHandle
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;
    GT_U32                          regAddr;
    GT_U32                          portGroupId;
    GT_U32                          portGroupsBmp;/* multi-port groups support */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    if (moduleCfgPtr->fuqUseSeparate == GT_FALSE)
    {
        return GT_OK;
    }
    /* After this action on chip FIFO contents (if at all) is transferred */
    /* into FUQ defined during cpss init                                  */
    /* now sinchronization hw and sw FUQ pointers is required */

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_MG_E);
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        if(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId].blockAddr != 0)
        {
            /* Enable FUQ for each portgroup  */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
            rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,31,1,1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDxChSystemRecoveryCatchUpHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpHandle
(
   GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8 devNum;
    GT_U32 i = 0;
    GT_U8 rxQueue = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecoveryInfo = systemRecoveryInfo;

    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
#ifdef    CPSS_LOG_ENABLE
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
#endif

    /* take time from the CatchUp start */
    cpssOsTimeRT(&start_sec, &start_nsec);

    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if ( (prvCpssPpConfig[devNum] == NULL) ||
             (PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum) == 0) )
        {
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            continue;
        }
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            while (catchUpFuncPtrArray[i] != NULL)
            {
                /* perform catch up*/
                rc = (*catchUpFuncPtrArray[i])(devNum);
                if (rc != GT_OK)
                {
                    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    return rc;
                }
                i++;
            }
        }
        if ( (systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
             (systemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_TRUE) )
        {
            /* HA mode - application provide the same memory for AUQ -- handle AUQ pointer*/
            rc = prvCpssDxChSystemRecoveryCatchUpSameMemoryAuFuHandle(devNum);
            if (rc != GT_OK)
            {
                CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                return rc;
            }
        }
        if ( (systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
             ( (systemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE) ||
               (systemRecoveryInfo.systemRecoveryMode.continuousFuMessages == GT_FALSE) ) )
        {
            /*HA mode - application can't guarantee the same memory for AUQ */
            /* during cpss init stage special AUQ WA was done and AUQ was disable for messages.*/
            systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
            if (systemRecoveryInfo.systemRecoveryMode.continuousFuMessages == GT_FALSE)
            {
                /*Restore FUQ enable status. In this case hw should be written */

                rc = prvCpssDxChSystemRecoveryCatchUpDiffMemoryFuHandle(devNum);
                if (rc != GT_OK)
                {
                    systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    return rc;
                }
            }

            if (systemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE)
            {
                /*Restore AUQ enable status. In this case hw should be written */
                rc =  prvCpssDxChSystemRecoveryCatchUpDiffMemoryAuHandle(devNum);
                if (rc != GT_OK)
                {
                    systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    return rc;
                }
            }

            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            /* enable RxSDMA queues */
            /* write Rx SDMA Queues Reg - Enable Rx SDMA Queues */
            if (systemRecoveryInfo.systemRecoveryMode.continuousRx == GT_FALSE)
            {
                GT_U32 rxSdmaRegVal = 0;        /* Rx SDMA Queues Register value        */
                GT_U32         portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);
                PRV_CPSS_DXCH_PP_REGS_ADDR_STC *regsAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
                for(rxQueue = 0; rxQueue < NUM_OF_RX_QUEUES; rxQueue++)
                {
                    /* Enable Rx SDMA Queue */
                    /* set Enable bit to 1, and Disable bit to 0.*/
                    rxSdmaRegVal |= 1 << rxQueue;
                    rxSdmaRegVal &= (~(1 << (rxQueue + NUM_OF_RX_QUEUES)));
                    /* write Rx SDMA Queues Reg - Enable Rx SDMA Queue */
                    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,regsAddr->sdmaRegs.rxQCmdReg,rxSdmaRegVal);
                    if(rc != GT_OK)
                    {
                        systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                        return rc;
                    }
                }
            }
            systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
        }
        else
        {
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        }
        /* prepare iterator for next device*/
        i = 0;

    }

    /* Calculate time from CatchUp start */
    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
#ifdef    CPSS_LOG_ENABLE
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;
#endif

    CPSS_LOG_INFORMATION_MAC("Catchup time processing is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}


/**
* @internal dxChEnableFdbUploadActionAndSaveFuqCurrentStatus function
* @endinternal
*
* @brief   This function configure FDB upload action for specific entry
*         and save current FUQ action status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] actionDataPtr            - pointer to action data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on bad device.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS dxChEnableFdbUploadActionAndSaveFuqCurrentStatus
(
    IN  GT_U8                             devNum,
    OUT PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC *actionDataPtr
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChBrgFdbUploadEnableGet(devNum,&actionDataPtr->fdbUploadState);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Enable/Disable reading FDB entries via AU messages to the CPU*/
    rc =  cpssDxChBrgFdbUploadEnableSet(devNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* configure FDB to upload only this specific entry*/

    /* save vid and vid mask */
    rc =  cpssDxChBrgFdbActionActiveVlanGet(devNum,&actionDataPtr->currentVid,
                                            &actionDataPtr->currentVidMask);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set current vid and vid mask */
    rc =  cpssDxChBrgFdbActionActiveVlanSet(devNum,9,0xfff);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save current action dev */
    rc =  cpssDxChBrgFdbActionActiveDevGet(devNum,&actionDataPtr->actDev,
                                           &actionDataPtr->actDevMask);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* set new action device */
    rc = cpssDxChBrgFdbActionActiveDevSet(devNum,30,0x1f);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save current action interface */
    rc = cpssDxChBrgFdbActionActiveInterfaceGet(devNum,&actionDataPtr->actIsTrunk,&actionDataPtr->actIsTrunkMask,
                                                &actionDataPtr->actTrunkPort,&actionDataPtr->actTrunkPortMask);

    if (rc != GT_OK)
    {
        return rc;
    }
    /* set new action interface */
    rc =  cpssDxChBrgFdbActionActiveInterfaceSet(devNum,0,0,62,0x3f);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save action trigger mode */
    rc = cpssDxChBrgFdbMacTriggerModeGet(devNum,&actionDataPtr->triggerMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* set new action trigger mode */
    rc = cpssDxChBrgFdbMacTriggerModeSet(devNum,CPSS_ACT_TRIG_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* save action mode */
    rc =  cpssDxChBrgFdbActionModeGet(devNum,&actionDataPtr->actionMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set fdb upload action mode */
    rc = cpssDxChBrgFdbActionModeSet(devNum,CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save action enable state*/
    rc = cpssDxChBrgFdbActionsEnableGet(devNum,&actionDataPtr->actionEnable);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* set action enable set */
    rc = cpssDxChBrgFdbActionsEnableSet(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        /* save maskAuFuMsg2CpuOnNonLocal state*/
        rc = prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableGet(devNum,&actionDataPtr->maskAuFuMsg2CpuOnNonLocal);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* set maskAuFuMsg2CpuOnNonLocal state*/
        rc = prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet(devNum,GT_FALSE);
    }

    return rc;
}

/**
* @internal dxChRestoreCurrentFdbActionStatus function
* @endinternal
*
* @brief   This function restore FDB action data and apply it on the device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] actionDataPtr            - pointer to action data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxChRestoreCurrentFdbActionStatus
(
    IN GT_U8    devNum,
    IN PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC *actionDataPtr
)
{
    GT_STATUS rc = GT_OK;

    rc =  cpssDxChBrgFdbActionActiveVlanSet(devNum,actionDataPtr->currentVid,
                                            actionDataPtr->currentVidMask);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* restore saved action device */
    rc =  cpssDxChBrgFdbActionActiveDevSet(devNum,actionDataPtr->actDev,
                                           actionDataPtr->actDevMask);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* restore saved action interface */
    rc = cpssDxChBrgFdbActionActiveInterfaceSet(devNum,actionDataPtr->actIsTrunk,
                                                actionDataPtr->actIsTrunkMask,
                                                actionDataPtr->actTrunkPort,
                                                actionDataPtr->actTrunkPortMask);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* restore saved action trigger mode */
    rc = cpssDxChBrgFdbMacTriggerModeSet(devNum,actionDataPtr->triggerMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* restore  saved action mode */
    rc = cpssDxChBrgFdbActionModeSet(devNum,actionDataPtr->actionMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* restore saved action enable/disable mode */
    rc = cpssDxChBrgFdbActionsEnableSet(devNum,actionDataPtr->actionEnable);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* restore fdbUploadState enable/disable state */
    rc = cpssDxChBrgFdbUploadEnableSet(devNum,actionDataPtr->fdbUploadState);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        /* restore  maskAuFuMsg2CpuOnNonLocal state*/
        rc = prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet(devNum,actionDataPtr->maskAuFuMsg2CpuOnNonLocal);
    }

    return rc;
}

/**
* @internal dxChAuqStatusMemoryFree function
* @endinternal
*
* @brief   This function free previously allocated AUQ status memory.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*                                       None.
*/
static GT_VOID dxChAuqStatusMemoryFree
(
    IN GT_U8    devNum
)
{
   if (auqMsgEnableStatus[devNum]->naToCpuPerPortPtr != NULL)
    {
        cpssOsFree(auqMsgEnableStatus[devNum]->naToCpuPerPortPtr);
        auqMsgEnableStatus[devNum]->naToCpuPerPortPtr = NULL;
    }
    if (auqMsgEnableStatus[devNum]->naStormPreventPortPtr != NULL)
    {
        cpssOsFree(auqMsgEnableStatus[devNum]->naStormPreventPortPtr);
        auqMsgEnableStatus[devNum]->naStormPreventPortPtr = NULL;
    }
    if (auqMsgEnableStatus[devNum] != NULL)
    {
        cpssOsFree(auqMsgEnableStatus[devNum]);
        auqMsgEnableStatus[devNum] = NULL;
    }
}

/**
* @internal prvCpssDxChRestoreAuqCurrentStatus function
* @endinternal
*
* @brief   This function retieve AUQ enable/disable message status
*         and apply it on the device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRestoreAuqCurrentStatus
(
    IN GT_U8    devNum
)
{
    GT_U32 i = 0;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        GT_U32  regAddr;
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, auqMsgEnableStatus[devNum]->AUMessageToCPUStop);
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }

    /*Restore sending NA update messages to the CPU per port*/

    for(i=0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        /* skip not existed ports */
        if (! PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, i))
            continue;

        rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,(GT_U8)i,auqMsgEnableStatus[devNum]->naToCpuPerPortPtr[i]);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
        rc =  cpssDxChBrgFdbNaStormPreventSet(devNum,(GT_U8)i,auqMsgEnableStatus[devNum]->naStormPreventPortPtr[i]);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
    }

    /*Restore sending NA messages to the CPU indicating that the device
    cannot learn a new SA. */

    rc = cpssDxChBrgFdbNaMsgOnChainTooLongSet(devNum,auqMsgEnableStatus[devNum]->naToCpuLearnFail);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }
    /* Restore the status of Tag1 VLAN Id assignment in vid1 field of the NA AU
      message */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        rc = cpssDxChBrgFdbNaMsgVid1EnableSet(devNum,auqMsgEnableStatus[devNum]->naTag1VLANassignment);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
    }

    /* restore sending to CPU status of AA and TA messages*/
    rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum,auqMsgEnableStatus[devNum]->aaTaToCpu);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }
    /* restore Sp AA message to CPU status*/
    rc =  cpssDxChBrgFdbSpAaMsgToCpuSet(devNum,auqMsgEnableStatus[devNum]->spAaMsgToCpu);
    dxChAuqStatusMemoryFree(devNum);

    return rc;
}

/**
* @internal dxChAuqFillByQuery function
* @endinternal
*
* @brief   The function fills AUQ and return the queueu state full/not full.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] macEntryPtr              - pointer to mac entry.
*
* @param[out] isAuqFullPtr             - (pointer to) AUQ status:
*                                      GT_TRUE - AUQ is full.
*                                      GT_FALSE - otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp or queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS dxChAuqFillByQuery
(
    IN GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP         portGroupsBmp,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC  *macEntryPtr,
    OUT GT_BOOL                    *isAuqFullPtr
)
{
    GT_U32              qa_counter             = 0;
    GT_BOOL             auqIsFull              = GT_FALSE;
    GT_PORT_GROUPS_BMP  isFullPortGroupsBmp    = 0;
    GT_STATUS           rc                     = GT_OK;
    GT_PORT_GROUPS_BMP  completedPortGroupsBmp = 0;
    GT_PORT_GROUPS_BMP  succeededPortGroupsBmp = 0;

    while (auqIsFull == GT_FALSE)
    {
        /* check if AUQ full bit is set */
        rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_AU_E,&isFullPortGroupsBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
        if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
        {
            /* queue is full */
            *isAuqFullPtr = GT_TRUE;
            return rc;
        }

        /* send quary */
        qa_counter = 0;
        do
        {
            rc =  cpssDxChBrgFdbPortGroupQaSend( devNum, portGroupsBmp, macEntryPtr);
            if(rc != GT_OK)
            {
    #ifdef ASIC_SIMULATION
                cpssOsTimerWkAfter(1);
    #endif
                qa_counter++;
                if(qa_counter > 20)
                {
                    return rc;
                }
            }
        } while (rc != GT_OK);

        /* verify that action is completed */
        completedPortGroupsBmp = 0;
        while ((completedPortGroupsBmp & portGroupsBmp)!= portGroupsBmp)
        {
            rc = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(devNum,portGroupsBmp,&completedPortGroupsBmp,
                                                              &succeededPortGroupsBmp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}



/**
* @internal prvCpssDxChDisableAuqAndSaveAuqCurrentStatus function
* @endinternal
*
* @brief   This function disable AUQ for messages and save current AUQ messages enable status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDisableAuqAndSaveAuqCurrentStatus
(
    IN GT_U8    devNum
)
{
    GT_U32 i = 0;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* alocate memory for given device */
    auqMsgEnableStatus[devNum] = (PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC));
    if (auqMsgEnableStatus[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(auqMsgEnableStatus[devNum],0,sizeof(PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC));
    auqMsgEnableStatus[devNum]->naToCpuPerPortPtr = (GT_BOOL*)cpssOsMalloc(sizeof(GT_BOOL)* PRV_CPSS_PP_MAC(devNum)->numOfPorts);
    if (auqMsgEnableStatus[devNum]->naToCpuPerPortPtr == NULL)
    {
        dxChAuqStatusMemoryFree(devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(auqMsgEnableStatus[devNum]->naToCpuPerPortPtr,0,sizeof(GT_BOOL)* PRV_CPSS_PP_MAC(devNum)->numOfPorts);
    auqMsgEnableStatus[devNum]->naStormPreventPortPtr = (GT_BOOL*)cpssOsMalloc(sizeof(GT_BOOL)* PRV_CPSS_PP_MAC(devNum)->numOfPorts);
    if (auqMsgEnableStatus[devNum]->naStormPreventPortPtr == NULL)
    {
        dxChAuqStatusMemoryFree(devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(auqMsgEnableStatus[devNum]->naStormPreventPortPtr,0,sizeof(GT_BOOL)* PRV_CPSS_PP_MAC(devNum)->numOfPorts);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        GT_U32  regAddr, data;
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 30, 1, &data);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
        auqMsgEnableStatus[devNum]->AUMessageToCPUStop = data;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 1);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
        return rc;
    }

    /*Disable sending NA update messages to the CPU per port*/
    for(i=0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        /* skip not existed ports */
        if (! PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, i))
            continue;

        /* at first save staus per port*/
        rc = cpssDxChBrgFdbNaToCpuPerPortGet(devNum,(GT_U8)i,&(auqMsgEnableStatus[devNum]->naToCpuPerPortPtr[i]));
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }

        rc =  cpssDxChBrgFdbNaStormPreventGet(devNum, (GT_U8)i,&(auqMsgEnableStatus[devNum]->naStormPreventPortPtr[i]));
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }

        rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,(GT_U8)i,GT_FALSE);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
        rc =  cpssDxChBrgFdbNaStormPreventSet(devNum, (GT_U8)i,GT_FALSE);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
    }

    /* save status (enabled/disabled) of sending NA messages to the CPU
       indicating that the device cannot learn a new SA */
    rc = cpssDxChBrgFdbNaMsgOnChainTooLongGet(devNum,&auqMsgEnableStatus[devNum]->naToCpuLearnFail);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }
    /* disable sending NA messages to the CPU
       indicating that the device cannot learn a new SA */
    rc = cpssDxChBrgFdbNaMsgOnChainTooLongSet(devNum,GT_FALSE);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }
    /* Get the status of Tag1 VLAN Id assignment in vid1 field of the NA AU
      message */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        rc = cpssDxChBrgFdbNaMsgVid1EnableGet(devNum,&auqMsgEnableStatus[devNum]->naTag1VLANassignment);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
        /* Disable sending of Tag1 VLAN Id assignment in vid1 field of the NA AU
          message */
        rc = cpssDxChBrgFdbNaMsgVid1EnableSet(devNum,GT_FALSE);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            return rc;
        }
    }

    /* save status of AA and AT messages */
    rc = cpssDxChBrgFdbAAandTAToCpuGet(devNum,&auqMsgEnableStatus[devNum]->aaTaToCpu);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }

   /* Disable AA and AT messages */
    rc =  cpssDxChBrgFdbAAandTAToCpuSet(devNum,GT_FALSE);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }

    /* save Sp AA message to CPU status*/
    rc =  cpssDxChBrgFdbSpAaMsgToCpuGet(devNum,&auqMsgEnableStatus[devNum]->spAaMsgToCpu);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }

    /* Disable sending AA messages to the CPU indicating that the
    device aged-out storm prevention FDB entry */
    rc =  cpssDxChBrgFdbSpAaMsgToCpuSet(devNum,GT_FALSE);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHaAuqNonContinuousMsgModeHandle function
* @endinternal
*
* @brief   This function performs AUQ workaround after HA event. It makes PP to consider that
*         queue is full and to be ready for reprogramming.
*         The workaround should be used when application can't guarantee the same memory
*         allocated for AUQ before and after HA event.
*         Before calling this function application should disable access of device to CPU memory.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHaAuqNonContinuousMsgModeHandle
(
    IN GT_U8    devNum
)
{
    GT_STATUS                        rc;
    CPSS_MAC_ENTRY_EXT_KEY_STC       macEntry;
    GT_U32                           portGroupId;
    GT_PORT_GROUPS_BMP               portGroupsBmp = 0;
    GT_PORT_GROUPS_BMP               isFullPortGroupsBmp = 0;
    GT_BOOL                          auqIsFull = GT_FALSE;
    GT_U32                           regAddr = 0;
    GT_UINTPTR                       phyAddr;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum)) &&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.
        enabled == GT_FALSE))
    {
        /* disable AUQ for messages and save current AUQ enable status for given device */
        rc = prvCpssDxChDisableAuqAndSaveAuqCurrentStatus(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsTimerWkAfter(10);

        cpssOsMemSet(&macEntry,0,sizeof(CPSS_MAC_ENTRY_EXT_KEY_STC));
        macEntry.entryType                      = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.macVlan.vlanId             = 0;
        macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);
            auqIsFull = GT_FALSE;
            /* first check if AUQ is full*/
            rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_AU_E,&isFullPortGroupsBmp);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                return rc;
            }
            if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
            {
                /* WA sarts with AUQ full. In this case FIFO should be handled as well. */
                /* For this reason new AUQ of 64 messages size is defined. If there is  */
                /* something in FIFO it would be splashed into the new queue. After that*/
                /* this queue would be filled till the end by QR messages               */

                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    rc = prvCpssDxChStopAuq(devNum, portGroupId);
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        return rc;
                    }
                }

                /* define queue size */
                regAddr =PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
                rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0,31,64);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
                /* read physical AUQ address from PP */
                rc =  prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)(&phyAddr));
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
                /* define queue base address */
                rc = prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,(GT_U32)phyAddr);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }

                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    rc = prvCpssDxChEnableAuq(devNum, portGroupId);
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        return rc;
                    }
                    cpssOsTimerWkAfter(10);
                    continue;
                }

                /* now fill the FIFO queue */
                rc = dxChAuqFillByQuery(devNum,portGroupsBmp,&macEntry,&auqIsFull);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
                if (auqIsFull == GT_TRUE)
                {
                    /* handle another port group */
                    continue;
                }
                else
                {
                    /* didn't succeed to fill AUQ */
                    dxChAuqStatusMemoryFree(devNum);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
             if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                 continue;
            /* now fill the queue */
            rc = dxChAuqFillByQuery(devNum,portGroupsBmp,&macEntry,&auqIsFull);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                return rc;
            }
            if (auqIsFull == GT_TRUE)
            {
                /* handle another port group */
                continue;
            }
            else
            {
                /* didn't succeed to fill AUQ */
                dxChAuqStatusMemoryFree(devNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    }
    return GT_OK;
}

/**
* @internal dxChScanFdbAndAddEntries function
* @endinternal
*
* @brief   This function scan FDB for valid entries and add special entires in order to
*         perform FUQ WA (making FUQ FULL) by optimal manner.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] fdbNumOfEntries          - number of entries in FDB
* @param[in] fuqSizeInEntries         - number of entries in current fuq
* @param[in] fdbEntryPtr              - pointer to special fdb entry
*
* @param[out] deleteEntryPtr           - pointer to boolean array contained indexes of added entries
*                                      that should be deleted later.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxChScanFdbAndAddEntries
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        fdbNumOfEntries,
    IN  GT_U32                        fuqSizeInEntries,
    IN  CPSS_MAC_ENTRY_EXT_STC        *fdbEntryPtr,
    OUT GT_BOOL                       *deleteEntryPtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          numberOfFdbEntriesToAdd = 0;
    GT_BOOL         valid;
    GT_U32          index = 0;
    GT_BOOL         skip = GT_FALSE;

    /* Initialize numberOfFdbEntriesToAdd to the the maximum of FU queue size and FDB size. */
    if (fuqSizeInEntries > fdbNumOfEntries)
    {
        numberOfFdbEntriesToAdd = fdbNumOfEntries;
    }
    else
    {
        numberOfFdbEntriesToAdd = fuqSizeInEntries;
    }
    /* First scan of the FDB: find how many entries to add to the FDB */
    for (index = 0; index < fdbNumOfEntries; index++)
    {
        /* call cpss api function */
        rc = cpssDxChBrgFdbPortGroupMacEntryStatusGet(devNum, portGroupsBmp, index, &valid, &skip);
        if (rc != GT_OK)
        {
            cpssOsFree(deleteEntryPtr);
            return rc;
        }
        if ((valid == GT_TRUE) && (skip == GT_FALSE))
        {
            numberOfFdbEntriesToAdd--;
        }
        if (numberOfFdbEntriesToAdd == 0)
        {
            break;
        }
    }
    /* Second scan of the FDB: add entries */
    for (index = 0; index < fdbNumOfEntries; index++)
    {
        if (numberOfFdbEntriesToAdd == 0)
        {
            break;
        }

        rc = cpssDxChBrgFdbPortGroupMacEntryStatusGet(devNum,portGroupsBmp, index, &valid, &skip);
        if (rc != GT_OK)
        {
            cpssOsFree(deleteEntryPtr);
            return rc;
        }
        if ((valid == GT_FALSE) || (skip == GT_TRUE))
        {
            /* write the entry to the FDB */
            rc = cpssDxChBrgFdbPortGroupMacEntryWrite(devNum, portGroupsBmp, index, GT_FALSE, fdbEntryPtr);
            if (rc != GT_OK)
            {
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            deleteEntryPtr[index] = GT_TRUE;
            numberOfFdbEntriesToAdd--;
        }
    }
    return GT_OK;
}



/**
* @internal dxChFuqFillByUploadAction function
* @endinternal
*
* @brief   The function fills FUQ and return the queueu state full/not full.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      macEntryPtr   - pointer to mac entry.
*
* @param[out] isFuqFullPtr             - (pointer to) FUQ status:
*                                      GT_TRUE - FUQ is full.
*                                      GT_FALSE - otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp or queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS dxChFuqFillByUploadAction
(
    IN GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP         portGroupsBmp,
    OUT GT_BOOL                    *isFuqFullPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_PORT_GROUPS_BMP isFullPortGroupsBmp;
    GT_BOOL fuqIsFull = GT_FALSE;
    GT_BOOL actFinished = GT_FALSE;
    GT_U32 trigCounter = 0;

    rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_FU_E,&isFullPortGroupsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
    {
        fuqIsFull = GT_TRUE;
    }
    else
    {
        fuqIsFull = GT_FALSE;
    }
    /* fill all FUQs on the device*/
    while (fuqIsFull == GT_FALSE)
    {
        /*  force the upload trigger */
        rc =  cpssDxChBrgFdbMacTriggerToggle(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* verify that action is completed */
        actFinished = GT_FALSE;
        trigCounter = 0;
        while (actFinished == GT_FALSE)
        {
            rc = cpssDxChBrgFdbTrigActionStatusGet(devNum,&actFinished);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (actFinished == GT_FALSE)
            {
                trigCounter++;
            }
            else
            {
                trigCounter = 0;
                break;
            }
            if (trigCounter > 500)
            {
                rc =  prvCpssHwPpSetRegField(devNum,
                                                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.macTblAction0,
                                                1, 1, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_FU_E,&isFullPortGroupsBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
        if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
        {
            fuqIsFull = GT_TRUE;
        }
        else
        {
            fuqIsFull = GT_FALSE;
        }
    }
    *isFuqFullPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChHaFuqNonContinuousMsgModeHandle function
* @endinternal
*
* @brief   This function performs FUQ workaround after HA event. It makes PP to consider that
*         queue is full and to be ready for reprogramming.
*         The workaround should be used when application can't guarantee the same memory
*         allocated for FUQ before and after HA event.
*         Before calling this function application should disable access of device to CPU memory.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHaFuqNonContinuousMsgModeHandle
(
    IN GT_U8    devNum
)
{
    GT_STATUS                         rc = GT_OK;
    GT_U32                            regAddr;
    GT_BOOL                           actionCompleted = GT_FALSE;
    GT_U32                            inProcessBlocksBmp = 0;
    GT_U32                            inProcessBlocksBmp1 = 0;
    GT_U32                            fdbNumOfEntries;
    GT_BOOL                           *deleteEntryPtr = NULL;
    CPSS_MAC_ENTRY_EXT_STC            fdbEntry;
    GT_PORT_GROUPS_BMP                isFullPortGroupsBmp = 0;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 0;
    GT_UINTPTR                        phyAddr;
    GT_BOOL                           fuqIsFull = GT_FALSE;
    GT_U32                            i = 0;
    GT_U32                            fuqSize = 0;
    PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC actionData;
    GT_U32                            portGroupId = 0;
    GT_BOOL                           cncUploadIsHandled = GT_FALSE;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    cpssOsMemSet(&actionData,0,sizeof(PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC));
    fdbNumOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Get address of FDB Action general register */
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBAction.FDBActionGeneral;
    }
    else
    {
        /* Get address of FDB Action0 register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.macTblAction0;
    }

    /* check that there is not not-finished FDB upload */
    rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &actionCompleted);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(actionCompleted == GT_FALSE)
    {
        /* clear the trigger */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* check there are no CNC blocks yet being uploaded */
    rc = cpssDxChCncBlockUploadInProcessGet(devNum,&inProcessBlocksBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (inProcessBlocksBmp != 0)
    {
        /* if CNC upload is under way let it be finished */
        cpssOsTimerWkAfter(10);
    }
    inProcessBlocksBmp = 0;
    deleteEntryPtr = (GT_BOOL *)cpssOsMalloc(sizeof(GT_BOOL) * fdbNumOfEntries);
    if (deleteEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* disable AUQ for messages and save current AUQ enable status for given device */
    rc = prvCpssDxChDisableAuqAndSaveAuqCurrentStatus(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* configure FDB upload action for specific entry and save current FUQ action status */
    rc = dxChEnableFdbUploadActionAndSaveFuqCurrentStatus(devNum,&actionData);
    if (rc != GT_OK)
    {
        dxChAuqStatusMemoryFree(devNum);
        cpssOsFree(deleteEntryPtr);
        return rc;
    }

    cpssOsMemSet(&fdbEntry,0,sizeof(CPSS_MAC_ENTRY_EXT_STC));

    /* fill very specific fdb entry  */
    fdbEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    fdbEntry.key.key.macVlan.vlanId = 9;
    fdbEntry.key.key.macVlan.macAddr.arEther[0] = 0;
    fdbEntry.key.key.macVlan.macAddr.arEther[1] = 0x15;
    fdbEntry.key.key.macVlan.macAddr.arEther[2] = 0x14;
    fdbEntry.key.key.macVlan.macAddr.arEther[3] = 0x13;
    fdbEntry.key.key.macVlan.macAddr.arEther[4] = 0x12;
    fdbEntry.key.key.macVlan.macAddr.arEther[5] = 0x11;
    fdbEntry.dstInterface.devPort.hwDevNum = 30;
    fdbEntry.dstInterface.devPort.portNum = 62;
    fdbEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        cpssOsMemSet(deleteEntryPtr,0,sizeof(GT_BOOL)* fdbNumOfEntries);
        portGroupsBmp = (1 << portGroupId);
        /* first check if AUQ is full*/
        rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_AU_E,&isFullPortGroupsBmp);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            cpssOsFree(deleteEntryPtr);
            return rc;
        }
        if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
        {
            /* WA sarts with AUQ full. In this case FIFO can be full as well. */
            /* For this reason new AUQ of 64 messages size is defined. If there is  */
            /* something in FIFO it would be splashed into the new queue.         */

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                rc = prvCpssDxChStopAuq(devNum, portGroupId);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
            }

            /* define queue size */
            regAddr =PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
            rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0,31,64);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
            /* read physical AUQ address from PP */
            rc =  prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)(&phyAddr));
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            /* define queue base address */
            rc = prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,(GT_U32)phyAddr);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                rc = prvCpssDxChEnableAuq(devNum, portGroupId);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
            }
        }

        /* check if FUQ is full */
        rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_FU_E,&isFullPortGroupsBmp);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            cpssOsFree(deleteEntryPtr);
            return rc;
        }

        if((isFullPortGroupsBmp & portGroupsBmp)== portGroupsBmp)
        {
            /* WA sarts with FUQ full. In this case CNC upload action check is performed. */
            /* If upload action is ongoing lets it be finished by defining new fuq with   */
            /* CNC block size. (Asumtion is CNC upload command is given for different CNC */
            /* blocks sequentially  - one in a time).This process is proceeded until CNC  */
            /* is not finished.                                                           */
            /* If CNC upload is not a factor, FIFO should be handled as well.             */
            /* For this reason new FUQ of 64 messages size is defined. If there is        */
            /* something in FIFO it would be splashed into the new queue. After that      */
            /* this queue would be filled till the end by FDB upload action               */

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                rc = prvCpssDxChStopAuq(devNum, portGroupId);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                /* Get address of FDB Action general register */
                regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBAction.FDBActionGeneral;
            }
            else
            {
                /* Get address of FDB Action0 register */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.macTblAction0;
            }
            /* reset FDB action trigger if any */
            rc =  prvCpssHwPpSetRegField(devNum,
                    regAddr,
                    1, 1, 0);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            /* check if CNC upload takes place */
            rc =  cpssDxChCncPortGroupBlockUploadInProcessGet(devNum, portGroupsBmp, &inProcessBlocksBmp);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            if (inProcessBlocksBmp == 0)
            {
                /* cnc was not triggered and queue is full. The FIFO contents is unknown */
                /* In order to avoid CNC entries in FIFO define new queue by FIFO size   */
                fuqSize = 64;
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
                rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0,30,fuqSize);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    cpssOsFree(deleteEntryPtr);
                    return rc;
                }
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
                /* read physical FUQ address from PP */
                rc =  prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)(&phyAddr));
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    cpssOsFree(deleteEntryPtr);
                    return rc;
                }
                /* define queue base address */
                rc = prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,(GT_U32)phyAddr);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    cpssOsFree(deleteEntryPtr);
                    return rc;
                }
            }
            else
            {
                cncUploadIsHandled = GT_TRUE;
                while (inProcessBlocksBmp != 0)
                {
                    /* only one block can be uploaded in given time */
                    inProcessBlocksBmp1 = inProcessBlocksBmp;
                    isFullPortGroupsBmp = 0;
                    rc = cpssDxChBrgFdbPortGroupQueueFullGet(devNum,portGroupsBmp,CPSS_DXCH_FDB_QUEUE_TYPE_FU_E,&isFullPortGroupsBmp);
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        cpssOsFree(deleteEntryPtr);
                        return rc;
                    }
                    if((isFullPortGroupsBmp & portGroupsBmp)!= portGroupsBmp)
                    {
                        /* queue is not full and cnc dump is not finished */
                        dxChAuqStatusMemoryFree(devNum);
                        cpssOsFree(deleteEntryPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    fuqSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlockNumEntries + 64;/* cnc block size + FIFO */
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
                    rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0,30,fuqSize);
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        cpssOsFree(deleteEntryPtr);
                        return rc;
                    }
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
                    /* read physical FUQ address from PP */
                    rc =  prvCpssHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,(GT_U32*)(&phyAddr));
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        cpssOsFree(deleteEntryPtr);
                        return rc;
                    }
                    /* define queue base address */
                    rc = prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,(GT_U32)phyAddr);
                    if (rc != GT_OK)
                    {
                        dxChAuqStatusMemoryFree(devNum);
                        cpssOsFree(deleteEntryPtr);
                        return rc;
                    }
                    i = 0;
                    while (inProcessBlocksBmp == inProcessBlocksBmp1)
                    {
                        i++;
                        if (i > 1000)
                        {
                            dxChAuqStatusMemoryFree(devNum);
                            cpssOsFree(deleteEntryPtr);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                        /* check if CNC upload takes place */
                        rc =  cpssDxChCncPortGroupBlockUploadInProcessGet(devNum, portGroupsBmp, &inProcessBlocksBmp);
                        if (rc != GT_OK)
                        {
                            dxChAuqStatusMemoryFree(devNum);
                            cpssOsFree(deleteEntryPtr);
                            return rc;
                        }
                    }
                }
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                rc = prvCpssDxChEnableAuq(devNum, portGroupId);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    return rc;
                }
            }
        }
        else
        {
            /* fuq is not full*/
            /* get current fuq size */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
            rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,0,30,&fuqSize);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
        }
        /* in this point queue is not full : or from begining or new queue size 64  was defined  or  */
        /* new queue CNC size was defined.                                                           */
        /* now fill the current fuq by means of fdb upload action                                    */
        if (cncUploadIsHandled == GT_TRUE)
        {
            /* cnc upload was already handled */
            rc =  cpssDxChCncPortGroupBlockUploadInProcessGet(devNum, portGroupsBmp, &inProcessBlocksBmp);
            if (rc != GT_OK)
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                return rc;
            }
            if (inProcessBlocksBmp == 0)
            {
                /* set FDB to be the Queue owner */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 14, 1, 1);
                if (rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    cpssOsFree(deleteEntryPtr);
                    return rc;
                }
            }
            else
            {
                dxChAuqStatusMemoryFree(devNum);
                cpssOsFree(deleteEntryPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) || CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            cpssOsTimerWkAfter(10);
            continue;
        }

        rc = dxChScanFdbAndAddEntries(devNum,portGroupsBmp,fdbNumOfEntries,fuqSize,&fdbEntry,deleteEntryPtr);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            cpssOsFree(deleteEntryPtr);
            return rc;
        }
        rc =  dxChFuqFillByUploadAction(devNum, portGroupsBmp,&fuqIsFull);
        if (rc != GT_OK)
        {
            dxChAuqStatusMemoryFree(devNum);
            cpssOsFree(deleteEntryPtr);
            return rc;
        }
        if (fuqIsFull == GT_FALSE)
        {
            /* queue is still not full*/
            dxChAuqStatusMemoryFree(devNum);
            cpssOsFree(deleteEntryPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* Now queue is full . Restore FDB configuration */
        for (i = 0; i < fdbNumOfEntries; i++)
        {
            if (deleteEntryPtr[i] == GT_TRUE)
            {
                rc = prvCpssDxChPortGroupWriteTableEntryField(devNum,
                                                              portGroupId,
                                                              CPSS_DXCH_TABLE_FDB_E,
                                                              i,
                                                              0,
                                                              1,
                                                              1,
                                                              1);
                if(rc != GT_OK)
                {
                    dxChAuqStatusMemoryFree(devNum);
                    cpssOsFree(deleteEntryPtr);
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    cpssOsFree(deleteEntryPtr);
    /* restore configuration */
    rc =  dxChRestoreCurrentFdbActionStatus(devNum, &actionData);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc =  prvCpssDxChRestoreAuqCurrentStatus(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        /* Disable FUQ for each portgroup to prevent splashing FIFO into new defined queue */
        /* Further in the catch up stage fuq would be reenabled                            */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
        rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,31,1,0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}


/**
* @internal prvCpssDxChStopAuq function
* @endinternal
*
* @brief   AUQ stop sequence
*
* @note   APPLICABLE DEVICES:      Bobcat2; Catus; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - the port group Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS prvCpssDxChStopAuq
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_STATUS                      rc = GT_OK; /* return code */
    GT_U32                         regAddr;
    CPSS_SYSTEM_RECOVERY_INFO_STC  tempSystemRecoveryInfo;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_hostConfig;
    tempSystemRecoveryInfo = systemRecoveryInfo; /* save recovery state*/

    if(tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /* Stop the AUQ */
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 1, 1);
        if(rc != GT_OK)
        {
            systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
            return rc;
        }
    }
    systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
    return GT_OK;
}

/**
* @internal prvCpssDxChEnableAuq function
* @endinternal
*
* @brief   Re-enable AUQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Catus; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - the port group Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS prvCpssDxChEnableAuq
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_STATUS                      rc = GT_OK; /* return code */
    GT_U32                         regAddr;
    CPSS_SYSTEM_RECOVERY_INFO_STC  tempSystemRecoveryInfo;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_hostConfig;
    tempSystemRecoveryInfo = systemRecoveryInfo; /* save recovery state*/

    /* Re-enable the AUQ */
    if(tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        /* Reset the NA/FU counters */
        if(tempSystemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE)
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 1, 2, 3);
            if(rc != GT_OK)
            {
                systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                return rc;
            }
            rc = prvCpssPortGroupBusyWait(devNum, portGroupId, regAddr, 1, GT_FALSE);
            if(rc != GT_OK)
            {
                systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                return rc;
            }
            rc = prvCpssPortGroupBusyWait(devNum, portGroupId, regAddr, 2, GT_FALSE);
            if(rc != GT_OK)
            {
                systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
                return rc;
            }
        }

        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 1, 0);
        if(rc != GT_OK)
        {
            systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;
            return rc;
        }
    }
    systemRecoveryInfo.systemRecoveryState = tempSystemRecoveryInfo.systemRecoveryState;

    return GT_OK;
}


