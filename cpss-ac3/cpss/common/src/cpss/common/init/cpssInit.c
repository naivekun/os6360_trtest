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
* @file cpssInit.c
*
* @brief Initialization function for PP's , regardless to PP's types.
*
* @version   14
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/init/private/prvCpssCommonInitLog.h>
#endif /*CHX_FAMILY*/

#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/common/private/prvCpssGlobalMutex.h>

#define DEBUG_PRINT(x) cpssOsPrintf x

#define PRINT_FORMAT_d_MAC(field)    \
    " %s = [%d] \n",#field,field

#define PRINT_FORMAT_8_8x_MAC(field)    \
    " %s = [0x%8.8x] \n",#field,field

#define PRINT_FORMAT_4_4x_MAC(field)    \
    " %s = [0x%4.4x] \n",#field,field

/**
* @internal cpssPpInit function
* @endinternal
*
* @brief   This function initialize the internal DB of the CPSS regarding PPs.
*         This function must be called before any call to a PP functions ,
*         i.e before calling cpssExMxHwPpPhase1Init.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssPpInit
(
    void
)
{
    GT_STATUS rc;

    /************************************/
    /* initialize the cpss global mutex */
    /************************************/
    rc = cpssGlobalMtxInit();
    if(rc != GT_OK)
    {
        return rc;
    }

    /***********************/
    /* initialize the cpss */
    /***********************/
    rc = prvCpssSysConfigPhase1();
    if(rc != GT_OK)
    {
        return rc;
    }

    /*****************************/
    /* initialize the cpssDriver */
    /*****************************/
    rc = prvCpssDrvSysConfigPhase1();
    if(rc != GT_OK)
    {
        return rc;
    }

    sysGenGlobalInfo.cpssPpInitCalled = GT_TRUE;

    /* Disable the combining writes / read feature in the system controller */
    rc = cpssExtDrvEnableCombinedPciAccess(GT_FALSE,GT_FALSE);

    return rc;
}

/**
* @internal cpssPpDestroy function
* @endinternal
*
* @brief   This function destroy the internal DB of the CPSS regarding PPs.
*         This function should be called after all devices have been removed from
*         the CPSS and system need to 'clean up' before restart initialization again.
*         so function must be called before recalling cpssPpInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssPpDestroy
(
    void
)
{
    GT_STATUS   rc;

    /**************************/
    /* destroy the cpssDriver */
    /**************************/
    /* already done in prvCpssDrvHwPpDevRemove(...) when last device removed */

    /********************/
    /* destroy the cpss */
    /********************/
    rc = prvCpssSysConfigDestroy();
    if(rc != GT_OK)
    {
        return rc;
    }

#if 0 /* we can not destroy this semaphore as it is used by CMD engine to lock
        any access to galtisWrappers/extern function.
        so destroying it will cause the caller of lock/unlock to:
        1. lock 'old' mutex
        2. call cpss to 'System reset' -->
            a. destroy 'old' mutex
            b. create 'new' mutex
        3. unlock 'new' mutex !!!  --> ERROR !!!
      */
    /*********************************/
    /* destroy the cpss global mutex */
    /*********************************/
    (void)cpssGlobalMtxDelete();
#endif /*0*/

    sysGenGlobalInfo.cpssPpInitCalled = GT_FALSE;

    return GT_OK;
}

extern GT_STATUS prvCpssDrvPpDump(GT_U8   devNum);

/**
* @internal internal_cpssPpDump function
* @endinternal
*
* @brief   Dump function , to print the info on a specific PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPpDump
(
    IN GT_U8   devNum
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC      *devInfoPtr;/* pointer to device info */
    PRV_CPSS_PORT_INFO_ARRAY_STC    *portInfoPtr;/* pointer to current port info */
    GT_U8   port;/* port iterator*/
    GT_U32              ii,jj;   /* iterator */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        DEBUG_PRINT(("bad device number [%d] \n",devNum));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    DEBUG_PRINT(("\n"));
    DEBUG_PRINT(("start Info about device number [%d]: \n",devNum));

    devInfoPtr = PRV_CPSS_PP_MAC(devNum);

    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->devNum)));
    DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->functionsSupportedBmp)));

    DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->devType)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->revision)));
    DEBUG_PRINT((" devFamily = [%s] \n",
        devInfoPtr->devFamily == CPSS_PP_FAMILY_TWISTC_E    ? " CPSS_PP_FAMILY_TWISTC_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_TWISTD_E    ? " CPSS_PP_FAMILY_TWISTD_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_SAMBA_E     ? " CPSS_PP_FAMILY_SAMBA_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_SALSA_E     ? " CPSS_PP_FAMILY_SALSA_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH_E   ? " CPSS_PP_FAMILY_CHEETAH_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH2_E  ? " CPSS_PP_FAMILY_CHEETAH2_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH3_E  ? " CPSS_PP_FAMILY_CHEETAH3_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E  ? " CPSS_PP_FAMILY_DXCH_XCAT_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E  ? " CPSS_PP_FAMILY_DXCH_XCAT3_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E ? " CPSS_PP_FAMILY_DXCH_XCAT2_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION_E  ? " CPSS_PP_FAMILY_DXCH_LION_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E  ? " CPSS_PP_FAMILY_DXCH_LION2_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E  ? " CPSS_PP_FAMILY_DXCH_BOBCAT2_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E  ? " CPSS_PP_FAMILY_DXCH_BOBCAT3_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E  ? " CPSS_PP_FAMILY_DXCH_ALDRIN_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E  ? " CPSS_PP_FAMILY_DXCH_AC3X_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E  ? " CPSS_PP_FAMILY_DXCH_ALDRIN2_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_TIGER_E     ? " CPSS_PP_FAMILY_TIGER_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_PUMA_E     ? " CPSS_PP_FAMILY_PUMA_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_PUMA3_E    ? " CPSS_PP_FAMILY_PUMA3_E ":
        devInfoPtr->devFamily == CPSS_PX_FAMILY_PIPE_E    ? " CPSS_PX_FAMILY_PIPE_E ":
        devInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E  ? " CPSS_PP_FAMILY_DXCH_FALCON_E ":

        " unknown "
    ));

    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->numOfPorts)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->numOfVirtPorts)));

    DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[0])));
    if(devInfoPtr->numOfPorts > (32*1))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[1])));
    if(devInfoPtr->numOfPorts > (32*2))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[2])));
    if(devInfoPtr->numOfPorts > (32*3))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[3])));
    if(devInfoPtr->numOfPorts > (32*4))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[4])));
    if(devInfoPtr->numOfPorts > (32*5))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[5])));
    if(devInfoPtr->numOfPorts > (32*6))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[6])));
    if(devInfoPtr->numOfPorts > (32*7))
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->existingPorts.ports[7])));

    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->baseCoreClock)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->coreClock)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->hwDevNum)));


    DEBUG_PRINT((" mngInterfaceType = [%s] \n",
        PRV_CPSS_HW_IF_PEX_COMPATIBLE_MAC(devNum) ? " CPSS_CHANNEL_PEX_E " :
        PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum) ? " CPSS_CHANNEL_PCI_E " :
        PRV_CPSS_HW_IF_SMI_COMPATIBLE_MAC(devNum)  ? " CPSS_CHANNEL_SMI_E " :
        PRV_CPSS_HW_IF_TWSI_COMPATIBLE_MAC(devNum) ? " CPSS_CHANNEL_TWSI_E " :
        " unknown "
    ));

    DEBUG_PRINT((" configInitAfterHwResetDone = [%s] \n",
            devInfoPtr->configInitAfterHwResetDone == GT_TRUE   ? " GT_TRUE " :
            devInfoPtr->configInitAfterHwResetDone == GT_FALSE  ? " GT_FALSE " :
            " unknown "
    ));

    DEBUG_PRINT((" ppHAState = [%s] \n",
            devInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E  ? " CPSS_SYS_HA_MODE_ACTIVE_E " :
            devInfoPtr->ppHAState == CPSS_SYS_HA_MODE_STANDBY_E ? " CPSS_SYS_HA_MODE_STANDBY_E " :
            " unknown "
    ));


    if(devInfoPtr->phyPortInfoArray == NULL)
    {
        DEBUG_PRINT((" devInfoPtr->phyPortInfoArray ERROR -- NULL POINTER !!! \n"));
    }
    else
    {
        portInfoPtr = &devInfoPtr->phyPortInfoArray[0];
        for (port = 0 ; port < devInfoPtr->numOfPorts ; port++ , portInfoPtr++)
        {
            DEBUG_PRINT((PRINT_FORMAT_d_MAC(port)));

            DEBUG_PRINT((PRINT_FORMAT_d_MAC(portInfoPtr->portFcParams.xonLimit)));
            DEBUG_PRINT((PRINT_FORMAT_d_MAC(portInfoPtr->portFcParams.xoffLimit)));

            DEBUG_PRINT((" portInfoPtr->portType = [%s] \n",
                        portInfoPtr->portType == PRV_CPSS_PORT_NOT_EXISTS_E ? " PRV_CPSS_PORT_NOT_EXISTS_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_FE_E         ? " PRV_CPSS_PORT_FE_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_GE_E         ? " PRV_CPSS_PORT_GE_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_XG_E         ? " PRV_CPSS_PORT_XG_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_XLG_E         ? " PRV_CPSS_PORT_XLG_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_HGL_E         ? " PRV_CPSS_PORT_HGL_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_CG_E         ? " PRV_CPSS_PORT_CG_E " :
                        portInfoPtr->portType == PRV_CPSS_PORT_NOT_APPLICABLE_E         ? " PRV_CPSS_PORT_NOT_APPLICABLE_E " :
            " unknown "
            ));

            DEBUG_PRINT((PRINT_FORMAT_4_4x_MAC(portInfoPtr->smiIfInfo.smiInterface)));
            DEBUG_PRINT((PRINT_FORMAT_4_4x_MAC(portInfoPtr->smiIfInfo.smiPortAddr)));
        }

        DEBUG_PRINT(("\n"));
        DEBUG_PRINT(("============== end of per port info =========== \n"));
        DEBUG_PRINT(("\n"));
    }

    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->useIsr)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->cpuPortMode)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->netifSdmaPortGroupId)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->maxIterationsOfBusyWait)));
    DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->maxIterationsOfSmiWait)));

    if(devInfoPtr->portGroupsInfo.isMultiPortGroupDevice != GT_FALSE)
    {
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.isMultiPortGroupDevice)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.numOfPortGroups)));
        DEBUG_PRINT((PRINT_FORMAT_8_8x_MAC(devInfoPtr->portGroupsInfo.activePortGroupsBmp)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.firstActivePortGroup)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.lastActivePortGroup)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.cpuPortPortGroupId)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.loopbackPortGroupId)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.auqPortGroupIdLastServed)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.fuqPortGroupIdLastServed)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.secureBreachNextPortGroupToServe)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.ptpNextPortGroupToServe)));
        DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.phaViolationCapturedNextPortGroupToServe)));

        for (ii = 0; ii < PRV_CPSS_MAX_DIRECTIONS_CNS ; ii ++)
        {
            for (jj = 0; jj < PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS; jj++)
            {
                  DEBUG_PRINT((PRINT_FORMAT_d_MAC(devInfoPtr->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[ii][jj])));
            }
        }

    }


    /* print more specific info */
#if ( ((defined EX_FAMILY) || (defined MX_FAMILY)) && !(defined CPSS_PRODUCT))
    {
        PRV_CPSS_EXMX_PP_CONFIG_STC  *exMxDevInfoPtr;

        DEBUG_PRINT(("\n"));

        DEBUG_PRINT((" This is ExMx device \n"));

        exMxDevInfoPtr = PRV_CPSS_EXMX_PP_MAC(devNum);

        DEBUG_PRINT((" exMxDevInfoPtr->uplinkCfg = [%s] \n",
            exMxDevInfoPtr->uplinkCfg == CPSS_PP_UPLINK_BACK_TO_BACK_E  ? " CPSS_PP_UPLINK_BACK_TO_BACK_E " :
            exMxDevInfoPtr->uplinkCfg == CPSS_PP_UPLINK_FA_E            ? " CPSS_PP_UPLINK_FA_E " :
            exMxDevInfoPtr->uplinkCfg == CPSS_PP_NO_UPLINK_E            ? " CPSS_PP_NO_UPLINK_E " :
            " unknown "
            ));
    }
#endif /* (defined EX_FAMILY) || (defined MX_FAMILY) */

    DEBUG_PRINT(("finished Info about device number [%d]: \n",devNum));
    DEBUG_PRINT(("\n"));
    DEBUG_PRINT(("\n"));

    /* call the driver to print it's info */
    prvCpssDrvPpDump(devNum);

    return GT_OK;

}

/**
* @internal cpssPpDump function
* @endinternal
*
* @brief   Dump function , to print the info on a specific PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPpDump
(
    IN GT_U8   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPpDump(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssSystemDualDeviceIdModeEnableSet function
* @endinternal
*
* @brief   This function declare 'This system support dual device Id'.
*         The function MUST be called for any system with Lion2 ('Dual device Id' device).
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*         For such system this function must be called before any call to a PP functions ,
*         i.e before calling cpssDxChHwPpPhase1Init(...).
*         The function MUST be called only after cpssPpInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - the function called before call to cpssPpInit(...).
*/
static GT_STATUS internal_cpssSystemDualDeviceIdModeEnableSet
(
    IN GT_BOOL  enable
)
{
    if(sysGenGlobalInfo.cpssPpInitCalled == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    sysGenGlobalInfo.supportDualDeviceId = enable;

    return GT_OK;
}

/**
* @internal cpssSystemDualDeviceIdModeEnableSet function
* @endinternal
*
* @brief   This function declare 'This system support dual device Id'.
*         The function MUST be called for any system with Lion2 ('Dual device Id' device).
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*         For such system this function must be called before any call to a PP functions ,
*         i.e before calling cpssDxChHwPpPhase1Init(...).
*         The function MUST be called only after cpssPpInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - the function called before call to cpssPpInit(...).
*/
GT_STATUS cpssSystemDualDeviceIdModeEnableSet
(
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemDualDeviceIdModeEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, enable));

    rc = internal_cpssSystemDualDeviceIdModeEnableSet(enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssSystemDualDeviceIdModeEnableGet function
* @endinternal
*
* @brief   This function retrieve if 'This system support dual device Id'.
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - (pointer to)indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssSystemDualDeviceIdModeEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    *enablePtr = sysGenGlobalInfo.supportDualDeviceId;

    return GT_OK;
}

/**
* @internal cpssSystemDualDeviceIdModeEnableGet function
* @endinternal
*
* @brief   This function retrieve if 'This system support dual device Id'.
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - (pointer to)indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssSystemDualDeviceIdModeEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemDualDeviceIdModeEnableGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, enablePtr));

    rc = internal_cpssSystemDualDeviceIdModeEnableGet(enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, enablePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

#ifdef ASIC_SIMULATION

#if ((defined GT_PCI) || (defined GT_PEX))
    #define SUPPORT_SIM_CONVERT
#endif/*((defined GT_PCI) || (defined GT_PEX))*/
/* convert CPSS devNum, coreId to simulation deviceId */
extern GT_STATUS prvCpssDrvDevNumToSimDevIdConvert
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    OUT GT_U32      *simDeviceIdPtr
);
extern GT_STATUS skernelFatherDeviceIdFromSonDeviceIdGet
(
    IN  GT_U32                sonDeviceId,
    OUT GT_U32               *fatherDeviceIdPtr
);


/**
* @internal getSimDevIdFromSwDevNum function
* @endinternal
*
* @brief   simulation function that converts the cpss SW number to simulation deviceId
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] swDevNum                 - the (cpss) SW device number
* @param[in] portGroupId              - the port groupId
*
* @param[out] simDeviceIdPtr           - (pointer to) the simulation device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS   getSimDevIdFromSwDevNum
(
    IN GT_U8    swDevNum,
    IN  GT_U32  portGroupId,
    OUT GT_U32  *simDeviceIdPtr
)
{
    GT_STATUS rc;
    /* convert the SW device number to 'Simulation device ID' */
#ifdef SUPPORT_SIM_CONVERT
    rc = prvCpssDrvDevNumToSimDevIdConvert(swDevNum,portGroupId,simDeviceIdPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
#else /*! SUPPORT_SIM_CONVERT*/
    /* do no convert allow SMI/TWSI */
    *simDeviceIdPtr = swDevNum;
#endif /*! SUPPORT_SIM_CONVERT*/

    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(swDevNum))
    {
        /* in case of multi-core device we still need to get the 'father' that
           represent this device in terms of 'Global ports' */
        rc = skernelFatherDeviceIdFromSonDeviceIdGet((*simDeviceIdPtr),simDeviceIdPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

#endif /*ASIC_SIMULATION*/

/* return GT_U32-mask with n lowest bits set to 0. Rest of mask's bits are 1. */
#define LO_ZEROES_MAC(n) ((n)>=32 ? (GT_U32)0  : (GT_U32)-1<<(n))

/* return GT_U32-mask with n lowest bits set to 1. Rest of mask's bits are 0. */
#define LO_ONES_MAC(n)   ((n)>=32 ? (GT_U32)-1 : (GT_U32)(1<<(n))-1)

/**
* @internal fieldValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (0..31)
*
* @note copied from snetFieldValueGet()
*
*/
static GT_U32  fieldValueGet(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
)
{
    GT_U32  actualStartWord = startBit >> 5;/*/32*/
    GT_U32  actualStartBit  = startBit & 0x1f;/*%32*/
    GT_U32  actualValue;
    GT_U32  workValue;
    GT_U32  numBitsFirst;
    GT_U32  numBitsLeft;

    if (numBits > 32)
    {
        /*ERROR*/
        numBits = 32;
    }

    if ((actualStartBit + numBits) <= 32)
    {
        numBitsFirst = numBits;
        numBitsLeft  = 0;
    }
    else
    {
        numBitsFirst = 32 - actualStartBit;
        numBitsLeft  = numBits - numBitsFirst;
    }

    actualValue = U32_GET_FIELD_MAC(
        startMemPtr[actualStartWord], actualStartBit, numBitsFirst);

    if (numBitsLeft > 0)
    {
        /* retrieve the rest of the value from the second word */
        workValue = U32_GET_FIELD_MAC(
            startMemPtr[actualStartWord + 1], 0, numBitsLeft);

        /* place it to the high bits of the result */
        actualValue |= (workValue << numBitsFirst);
    }

    return actualValue;
}


/**
* @internal fieldValueSet function
* @endinternal
*
* @brief   set the value to field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (0..31)
* @param[in] value                    -  to write to
*
* @note copied from snetFieldValueSet
*
*/
static void  fieldValueSet(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  value
)
{
    GT_U32  actualStartWord = startBit >> 5;/*/32*/
    GT_U32  actualStartBit  = startBit & 0x1f;/*%32*/
    GT_U32  numBitsFirst;
    GT_U32  numBitsLeft;

    if (numBits > 32)
    {
        /*ERROR*/
        numBits = 32 ;
    }

    if ((actualStartBit + numBits) <= 32)
    {
        numBitsFirst = numBits;
        numBitsLeft  = 0;
    }
    else
    {
        numBitsFirst = 32 - actualStartBit;
        numBitsLeft  = numBits - numBitsFirst;
    }

    U32_SET_FIELD_MASKED_MAC(
        startMemPtr[actualStartWord], actualStartBit, numBitsFirst,value);

    if (numBitsLeft > 0)
    {
        /* place rest of value to the high bits of the result */
        U32_SET_FIELD_MASKED_MAC(
            startMemPtr[actualStartWord + 1], 0, numBitsLeft,(value>>numBitsFirst));
    }

    return ;
}

/**
* @internal copyBits function
* @endinternal
*
* @brief   copy bits from source to target.
*         NOTE: function not supports overlap of copied memories.
*         for overlapping use copyBitsInMemory(...)
* @param[in] targetMemPtr             - (pointer to) target memory to write to.
* @param[in] targetStartBit           - start bit in the target to write to.
* @param[in] sourceMemPtr             - (pointer to) source memory to copy from.
* @param[in] sourceStartBit           - start bit in the source to copy from.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  copyBits(
    IN GT_U32                  *targetMemPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  *sourceMemPtr,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
)
{
    GT_U32  ii;
    GT_U32  value;
    GT_U32  numOfFull32Bits = numBits / 32;
    GT_U32  numBitsLastWord = numBits % 32;

    /* copy 32 bits at each time */
    for(ii = 0 ; ii < numOfFull32Bits; ii++)
    {
        /* get 32 bits from source */
        value = fieldValueGet(sourceMemPtr,sourceStartBit + (ii*32), 32);

        /* set 32 bits into target */
        fieldValueSet(targetMemPtr,targetStartBit + (ii*32), 32,value);
    }

    if(numBitsLastWord)
    {
        /* get last less than 32 bits from source */
        value = fieldValueGet(sourceMemPtr,sourceStartBit + (ii*32), numBitsLastWord);

        /* set last less than 32 bits into target */
        fieldValueSet(targetMemPtr,targetStartBit + (ii*32), numBitsLastWord,value);
    }

}
/**
* @internal copyBitsInMemory function
* @endinternal
*
* @brief   copy bits from source to target in a memory.
*         the function support overlap of copied bits from src to dst
* @param[in] memPtr                   - (pointer to) memory to update.
* @param[in] targetStartBit           - start bit in the memory to write to.
* @param[in] sourceStartBit           - start bit in the memory to copy from.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  copyBitsInMemory(
    IN GT_U32                  *memPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
)
{
    GT_U32  ii;
    GT_U32  value;
    GT_U32  numOfFull32Bits = numBits / 32;
    GT_U32  numBitsLastWord = numBits % 32;
    GT_U32  numBitsWithoutOverLap;
    GT_U32  valueToSaveFromTarget;

    if(targetStartBit >= sourceStartBit &&
       targetStartBit <  (sourceStartBit + numBits))
    {
        /* target is in the range of source */
        numBitsWithoutOverLap = targetStartBit - sourceStartBit;

        if(numBitsWithoutOverLap >= 32)
        {
            /* we not care about the overlap */
            copyBits(memPtr,targetStartBit,memPtr,sourceStartBit,numBits);
            return;
        }
    }
    else
    if(sourceStartBit >= targetStartBit &&
       sourceStartBit <  (targetStartBit + numBits))
    {
        /* source is in the range of target */

        /* we not care about the overlap */
        copyBits(memPtr,targetStartBit,memPtr,sourceStartBit,numBits);
        return;
    }
    else
    {
        /* no overlap */
        copyBits(memPtr,targetStartBit,memPtr,sourceStartBit,numBits);
        return;
    }

    valueToSaveFromTarget = 0;

    /* copy 32 bits at each time */
    for(ii = 0 ; ii < numOfFull32Bits; ii++)
    {
        /* get 32 bits from source */
        value = fieldValueGet(memPtr,sourceStartBit + (ii*32), 32);

        if(ii != 0)
        {
            /* update the value with info saved from the target */
            fieldValueSet(&value,0,numBitsWithoutOverLap,valueToSaveFromTarget);
        }

        /* save bits from target before overriding it */
        valueToSaveFromTarget = fieldValueGet(memPtr,targetStartBit + ((ii+1)*32) - numBitsWithoutOverLap, numBitsWithoutOverLap);

        /* set 32 bits into target */
        fieldValueSet(memPtr,targetStartBit + (ii*32), 32,value);
    }

    if(numBitsLastWord)
    {
        /* get last less than 32 bits from source */
        value = fieldValueGet(memPtr,sourceStartBit + (ii*32), numBitsLastWord);

        if(numOfFull32Bits != 0)
        {
            /* update the value with info saved from the target */
            fieldValueSet(&value,0,numBitsWithoutOverLap,valueToSaveFromTarget);
        }

        /* set last less than 32 bits into target */
        fieldValueSet(memPtr,targetStartBit + (ii*32), numBitsLastWord,value);
    }

}

/**
* @internal resetBitsInMemory function
* @endinternal
*
* @brief   reset bits in memory.
*
* @param[in] memPtr                   - (pointer to) memory to update.
* @param[in] startBit                 - start bit in the memory to reset.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  resetBitsInMemory(
    IN GT_U32                  *memPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
)
{
    GT_U32  ii;
    GT_U32  value = 0;
    GT_U32  numOfFull32Bits = numBits / 32;
    GT_U32  numBitsLastWord = numBits % 32;

    /* copy 32 bits at each time */
    for(ii = 0 ; ii < numOfFull32Bits; ii++)
    {
        /* set 32 bits into target */
        fieldValueSet(memPtr,startBit + (ii*32), 32,value);
    }

    if(numBitsLastWord)
    {
        /* set last less than 32 bits into target */
        fieldValueSet(memPtr,startBit + (ii*32), numBitsLastWord,value);
    }

}

/**
* @internal copyBitsMasked function
* @endinternal
*
* @brief   copy bits from source to target. Only source bits set in the mask
*         will be copied. Rest of target bits stays the same as was.
*         NOTE: function not supports overlap of copied memories.
* @param[in,out] targetMemPtr             - (pointer to) target memory to write to.
* @param[in] targetStartBit           - start bit in the target to write to.
* @param[in] sourceMemPtr             - (pointer to) source memory to copy from.
* @param[in] sourceStartBit           - start bit in the source, mask to copy from.
* @param[in] numBits                  - number of bits (unlimited num of bits)
* @param[in] maskPtr                  - (pointer to) mask. The length of mask is numBits.
*                                      if NULL the function behaves like copyBits()
* @param[in,out] targetMemPtr             - (pointer to) target memory
*                                      COMMENTS:
*/
void copyBitsMasked
(
    INOUT GT_U32   *targetMemPtr,
    IN    GT_U32   targetStartBit,
    IN    GT_U32   *sourceMemPtr,
    IN    GT_U32   sourceStartBit,
    IN    GT_U32   numBits,
    IN    GT_U32   *maskPtr
)
{
    GT_U32  ii;
    GT_U32  value;
    GT_U32  dstValue;
    GT_U32  numOfFull32Bits = numBits / 32;
    GT_U32  numBitsLastWord = numBits % 32;
    GT_U32  mask;


    if (NULL == maskPtr)
    {
        copyBits(targetMemPtr, targetStartBit, sourceMemPtr, sourceStartBit, numBits);
        return;
    }

    /* copy 32 bits at each time */
    for(ii = 0 ; ii < numOfFull32Bits; ii++)
    {
        /* get 32 bits from source */
        value = fieldValueGet(sourceMemPtr,sourceStartBit + (ii*32), 32);

        /* get 32 bits from mask */
        mask = fieldValueGet(maskPtr, sourceStartBit + (ii*32), 32);

        /* get 32 bits from target */
        dstValue = fieldValueGet(targetMemPtr, targetStartBit + (ii*32), 32);

        value = (value & mask) | (dstValue & ~mask);

        /* set 32 bits into target */
        fieldValueSet(targetMemPtr,targetStartBit + (ii*32), 32, value);
    }

    if(numBitsLastWord)
    {
        /* get last less than 32 bits from source */
        value = fieldValueGet(sourceMemPtr,sourceStartBit + (ii*32), numBitsLastWord);

        /* get last less than 32 bits from mask */
        mask = fieldValueGet(maskPtr, sourceStartBit + (ii*32), numBitsLastWord);

        /* get last less than 32 bits from target */
        dstValue = fieldValueGet(targetMemPtr,targetStartBit + (ii*32), numBitsLastWord);


        value = (value & mask) | (dstValue & ~mask);

        /* set last less than 32 bits into target */
        fieldValueSet(targetMemPtr,targetStartBit + (ii*32), numBitsLastWord, value);
    }
}

#ifndef CPSS_LOG_ENABLE
#define PRV_LOG_STRING_BUFFER_SIZE_CNS      2048
GT_BOOL prvCpssLogEnabled = GT_FALSE;
/* Log numeric format */
const GT_CHAR *prvCpssLogFormatNumFormat            = "%s = %d";
/* String constants */
const GT_CHAR *prvCpssLogErrorMsgDeviceNotExist     = "PRV_CPSS_DEV_CHECK_MAC: Device %d not exist.\n";
const GT_CHAR *prvCpssLogErrorMsgFdbIndexOutOfRange = "PRV_CPSS_DXCH_FDB_INDEX_CHECK_MAC: Device %d, FDB index %d is out of range\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupNotValid  = "PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC: Multi-port group device %d, port group %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupNotActive = "PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC: Multi-port group device %d, port group %d not active\n";
const GT_CHAR *prvCpssLogErrorMsgIteratorNotValid   = "__MAX_NUM_ITERATIONS_CHECK_CNS: Device %d, current iterator %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgGlobalIteratorNotValid   = "__MAX_NUM_ITERATIONS_CHECK_CNS: current iterator %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupBitmapNotValid
                                                    = "PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC: Multi-port group device %d, port group bitmap %d not valid \n";
const GT_CHAR *prvCpssLogErrorMsgDeviceNotInitialized
                                                    = "PRV_CPSS_DRV_CHECK_PP_DEV_MAC: Device %d not initialized.\n";
const GT_CHAR *prvCpssLogErrorMsgLogContextCurrentPointer
                                                    = "Unable to get pointer to current log context. rc = %d\n";
typedef struct{
    GT_STATUS   rc;
    GT_CHAR*    rc_name;
}RC_NAMES_TYPE;

#ifdef STR
    #undef STR
#endif

#define STR(strname)    \
    #strname


static RC_NAMES_TYPE rcNamesArr[] =
{
     {(GT_STATUS)GT_ERROR          ,    STR(GT_ERROR                   )       }
    ,{GT_OK                        ,    STR(GT_OK                      )       }
    ,{GT_FAIL                      ,    STR(GT_FAIL                    )       }

    ,{GT_BAD_VALUE                 ,    STR(GT_BAD_VALUE               )       }
    ,{GT_OUT_OF_RANGE              ,    STR(GT_OUT_OF_RANGE            )       }
    ,{GT_BAD_PARAM                 ,    STR(GT_BAD_PARAM               )       }
    ,{GT_BAD_PTR                   ,    STR(GT_BAD_PTR                 )       }
    ,{GT_BAD_SIZE                  ,    STR(GT_BAD_SIZE                )       }
    ,{GT_BAD_STATE                 ,    STR(GT_BAD_STATE               )       }
    ,{GT_SET_ERROR                 ,    STR(GT_SET_ERROR               )       }
    ,{GT_GET_ERROR                 ,    STR(GT_GET_ERROR               )       }
    ,{GT_CREATE_ERROR              ,    STR(GT_CREATE_ERROR            )       }
    ,{GT_NOT_FOUND                 ,    STR(GT_NOT_FOUND               )       }
    ,{GT_NO_MORE                   ,    STR(GT_NO_MORE                 )       }
    ,{GT_NO_SUCH                   ,    STR(GT_NO_SUCH                 )       }
    ,{GT_TIMEOUT                   ,    STR(GT_TIMEOUT                 )       }
    ,{GT_NO_CHANGE                 ,    STR(GT_NO_CHANGE               )       }
    ,{GT_NOT_SUPPORTED             ,    STR(GT_NOT_SUPPORTED           )       }
    ,{GT_NOT_IMPLEMENTED           ,    STR(GT_NOT_IMPLEMENTED         )       }
    ,{GT_NOT_INITIALIZED           ,    STR(GT_NOT_INITIALIZED         )       }
    ,{GT_NO_RESOURCE               ,    STR(GT_NO_RESOURCE             )       }
    ,{GT_FULL                      ,    STR(GT_FULL                    )       }
    ,{GT_EMPTY                     ,    STR(GT_EMPTY                   )       }
    ,{GT_INIT_ERROR                ,    STR(GT_INIT_ERROR              )       }
    ,{GT_NOT_READY                 ,    STR(GT_NOT_READY               )       }
    ,{GT_ALREADY_EXIST             ,    STR(GT_ALREADY_EXIST           )       }
    ,{GT_OUT_OF_CPU_MEM            ,    STR(GT_OUT_OF_CPU_MEM          )       }
    ,{GT_ABORTED                   ,    STR(GT_ABORTED                 )       }
    ,{GT_NOT_APPLICABLE_DEVICE     ,    STR(GT_NOT_APPLICABLE_DEVICE   )       }

    ,{GT_UNFIXABLE_ECC_ERROR       ,    STR(GT_UNFIXABLE_ECC_ERROR     )       }




    ,{GT_UNFIXABLE_BIST_ERROR      ,    STR(GT_UNFIXABLE_BIST_ERROR    )       }
    ,{GT_CHECKSUM_ERROR            ,    STR(GT_CHECKSUM_ERROR          )       }
    ,{GT_DSA_PARSING_ERROR         ,    STR(GT_DSA_PARSING_ERROR       )       }
};
static GT_U32   numOfElementsIn_rcNamesArr = sizeof(rcNamesArr)/sizeof(rcNamesArr[0]);

static GT_CHAR* getNameForRc(IN GT_STATUS rc)
{
    GT_U32  ii;
    for(ii = 0 ; ii < numOfElementsIn_rcNamesArr; ii++)
    {
        if(rc == rcNamesArr[ii].rc)
        {
            return rcNamesArr[ii].rc_name;
        }
    }

    return NULL;
}

GT_VOID prvCpssLogError
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN GT_STATUS rc,
    IN const char * formatPtr,
    ...
)
{
    va_list args;
    static char buffer[PRV_LOG_STRING_BUFFER_SIZE_CNS];
    GT_CHAR* rcName = getNameForRc(rc);

    if(rc == GT_OK)
    {
        /* do not print 'GT_OK' as errors */
        return;
    }


    /* Lock the Logger */
    CPSS_ZERO_LEVEL_API_LOCK_MAC;

    va_start(args, formatPtr);
    cpssOsVsprintf(buffer, formatPtr, args);
    va_end(args);

    if(rcName == NULL)
    {
        cpssOsPrintf("ERROR : func[%s]file[%s]line[%d]errorCode=[%d]"
                     "%s\n"
                    , functionName,fileName,line, rc ,
                    buffer);
    }
    else
    {
        cpssOsPrintf("ERROR : func[%s]file[%s]line[%d]errorCode=[%s]"
                     "%s\n"
                    , functionName,fileName,line, rcName ,
                    buffer);
    }

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}
GT_VOID cpssLogEnableSet
(
     IN GT_BOOL enable
)
{
    prvCpssLogEnabled = enable;
}
GT_BOOL cpssLogEnableGet(GT_VOID)
{
    return prvCpssLogEnabled;
}


#endif /*! CPSS_LOG_ENABLE*/

