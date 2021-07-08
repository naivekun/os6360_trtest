/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssGenPortAp.c
*
* DESCRIPTION:
*       CPSS implementation for 802.3ap standard (defines the auto negotiation
*       for backplane Ethernet) configuration and control facility.
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*******************************************************************************/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>


#define AP_DBG_EN
#ifdef AP_DBG_EN
    static GT_U32   allowPrint=0;/* option to disable the print in runtime*/
    #define AP_DBG_PRINT_MAC(x) if(allowPrint) cpssOsPrintSync x

extern GT_BOOL aldrinDebugCpllInitFlag;

extern GT_U32 apPrintControl(IN GT_U32  allowPrintNew)
{
    GT_U32  oldState = allowPrint;
    allowPrint = allowPrintNew;

    return oldState;
}

static GT_U32   apSemPrintEn = 0; /* option to disable the print in runtime*/
extern GT_U32 apSemPrint(IN GT_U32  allowPrintNew)
{
    GT_U32  oldState = apSemPrintEn;
    apSemPrintEn = allowPrintNew;

    return oldState;
}

#else
    #define AP_DBG_PRINT_MAC(x)
#endif

/* TODO - move to common */
/* CPSS suggested defaults for AP configuration */
CPSS_PORT_AP_PARAMS_STC prvCpssPortApDefaultParams =
{
    /* fcPause */  GT_TRUE,
    /* fcAsmDir */ CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E,
    /* fecSupported */ GT_TRUE,
    /* fecRequired */ GT_FALSE,
    /* noneceDisable */ GT_TRUE, CPSS_TBD_BOOKMARK_LION2 /* to fix when Z80 algorithm fixed */
    /* laneNum */       0,
    /* modesAdvertiseArr */
    {
        {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, CPSS_PORT_SPEED_1000_E}
        ,{CPSS_PORT_INTERFACE_MODE_XGMII_E, CPSS_PORT_SPEED_10000_E}
        ,{CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E}
        ,{CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_40000_E}
        ,{CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E, CPSS_PORT_SPEED_NA_E}
    },
    {
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E
    },
    {
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E
    }
};

/**
* @internal internal_cpssDxChPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of cores where to run AP engine
*                                      (0x7FFFFFF - for ALL)
* @param[in] enable                   - GT_TRUE  -  AP on port group
*                                      GT_FALSE - disbale
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Pay attention: for
*       BC2/Caelum/Bobcat3/Falcon/Aldrin/Aldrin2/AC3X must be
*       engaged before port LIB init i.e. before phase1 init!
*
*/
static GT_STATUS internal_cpssDxChPortApEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_BOOL             enable
)
{
    GT_U32      portGroupId;/* local core number */
    GT_STATUS   rc;         /* return code */

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, prvCpssLogErrorMsgDeviceNotExist, devNum);

    if(NULL == PRV_CPSS_PP_MAC(devNum))
    {/* Bobcat2; Caelum; xCat3 - engaged before info about devices configured */
        AP_DBG_PRINT_MAC(("cpssDxChPortApEnableSet:devNum=%d,portGroupsBmp=%d,enable=%d\n",
                            devNum, portGroupsBmp, enable));

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnable(devNum[%d], enable[%d])", devNum, enable);
        rc = mvHwsServiceCpuEnable(devNum, enable);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }
    else
    {/*Lion2*/
        PRV_CPSS_DEV_CHECK_MAC(devNum);
        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN2_E);

        AP_DBG_PRINT_MAC(("cpssDxChPortApEnableSet:devNum=%d,portGroupsBmp=%d,enable=%d\n",
                            devNum, portGroupsBmp, enable));

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp,
                                                          portGroupId)
        {
            if(enable)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApEngineInit(devNum[%d], portGroupId[%d])", devNum, portGroupId);
                rc = mvHwsApEngineInit(devNum,portGroupId);
            }
            else
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApEngineStop(devNum[%d], portGroupId[%d])", devNum, portGroupId);
                rc = mvHwsApEngineStop(devNum,portGroupId);
            }
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp,
                                                        portGroupId)
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of cores where to run AP engine
*                                      (0x7FFFFFF - for ALL)
* @param[in] enable                   - GT_TRUE  -  AP on port group
*                                      GT_FALSE - disbale
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Pay attention: for
*       BC2/Caelum/Bobcat3/Falcon/Aldrin/Aldrin2/AC3X must be
*       engaged before port LIB init i.e. before phase1 init!
*
*/
GT_STATUS cpssDxChPortApEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_BOOL             enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, enable));

    rc = internal_cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional on port group (local core) status.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_BOOL *enabledPtr
)
{
    GT_STATUS       rc;         /* return code */
    GT_U32 counter1;   /* AP watchdog value first read */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(enabledPtr);

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApEngineInitGet(devNum[%d], portGroupNum[%d], *enabledPtr)", devNum, portGroupNum);
        (GT_VOID)mvHwsApEngineInitGet(devNum,portGroupNum,enabledPtr);

        if(GT_FALSE == *enabledPtr)
        {
            AP_DBG_PRINT_MAC(("mvHwsApEngineInitGet:enabled=%d\n", *enabledPtr));
            return GT_OK;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvApCheckCounterGet(devNum[%d], portGroupNum[%d], *counter1)", devNum, portGroupNum);
        rc = mvApCheckCounterGet(devNum,portGroupNum,&counter1);
        AP_DBG_PRINT_MAC(("mvApCheckCounterGet_1:rc=%d,counter=%d\n", rc, counter1));
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnableGet(devNum[%d])", devNum);
        AP_DBG_PRINT_MAC(("mvHwsServiceCpuEnableGet(%d)\n", devNum));
        *enabledPtr = mvHwsServiceCpuEnableGet(devNum);
        CPSS_LOG_INFORMATION_MAC("service cpu %s", (*enabledPtr) ? "enabled" : "disabled");
        AP_DBG_PRINT_MAC(("service cpu %s\n", (*enabledPtr) ? "enabled" : "disabled"));
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional on port group (local core) status.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_BOOL *enabledPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, enabledPtr));

    rc = internal_cpssDxChPortApEnableGet(devNum, portGroupNum, enabledPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, enabledPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortLion2ApModesVectorBuild function
* @endinternal
*
* @brief   Get array of port modes in CPSS format and build advertisement array in
*         HWS format.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] modesAdvertiseArrPtr     - array of advertised modes in CPSS format
*
* @param[out] modesVectorPtr           - bitmap of advertised modes in HWS format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortLion2ApModesVectorBuild
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PORT_MODE_SPEED_STC *modesAdvertiseArrPtr,
    OUT GT_U32                   *modesVectorPtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      i;          /* iterator */
    GT_U32      localPort;  /* number of port in core */
    MV_HWS_PORT_STANDARD        portMode = NON_SUP_MODE;
    MV_HWS_PORT_INIT_PARAMS     portParams;
    GT_U32                      tmpModesVectorMask;

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    for(i = 0, *modesVectorPtr = 0;
         (i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS)
             && (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
             && (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E);
         i++)
    {
        switch(modesAdvertiseArrPtr[i].ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_1000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _1000Base_X;
                tmpModesVectorMask = _1000Base_KX_Bit0;
                break;
            case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_10000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _10GBase_KX4;
                tmpModesVectorMask = _10GBase_KX4_Bit1;
                break;
            case CPSS_PORT_INTERFACE_MODE_KR_E:
                if(CPSS_PORT_SPEED_10000_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _10GBase_KR;
                    tmpModesVectorMask = _10GBase_KR_Bit2;
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _40GBase_KR;
                    tmpModesVectorMask = _40GBase_KR4_Bit3;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        CPSS_LOG_INFORMATION_MAC("Calling: hwsPortModeParamsGetToBuffer(devNum[%d], portGroup[%d], portNum[%d], portMode[%d], portParams)", devNum, 0, localPort, portMode);
        rc = hwsPortModeParamsGetToBuffer(devNum,0,localPort,portMode,&portParams);
        if(GT_OK != rc)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return GT_BAD_PARAM");
            return rc;
        }

        if(NA_NUM == portParams.portMacNumber)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return GT_BAD_PARAM");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *modesVectorPtr |= tmpModesVectorMask;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortBc2ApModesVectorBuild function
* @endinternal
*
* @brief   Get array of port modes in CPSS format and build advertisement array in
*         HWS format.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] modesAdvertiseArrPtr     - array of advertised modes in CPSS format
* @param[in] fecRequestedArrPtr       - array of requested FEC modes for every pair in
* @param[in] modesAdvertiseArrPtr
* @param[in] fecSupported             - FEC supported negotiation advertisement parameter for
*                                      interfaces supporting only single type of FEC (or FC or RS)
* @param[in] fecRequired              - FEC required negotiation advertisement parameter for
*                                      interfaces supporting only single type of FEC (or FC or RS)
*
* @param[out] sdVecSizeMaxPtr          - maximum number of Serdes required in current configuration
*                                      e.g. if advertised 10G KR only sdVecSizeMax = 1;
*                                      if advertised 10G KR and 40G KR or 40G KR only sdVecSizeMax = 4;
* @param[out] sdVectorMaxPtr           - list of serdes lanes used by most "fat" interface
* @param[out] apCfgPtr                 - advertisement structure in HWS format
* @param[out] singleModePtr            - true - single interface advertised
*                                      false - multiple interfaces advertised
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortBc2ApModesVectorBuild
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MODE_SPEED_STC    *modesAdvertiseArrPtr,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT *fecAbilityArrPtr,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT *fecRequestedArrPtr,
    IN  GT_BOOL                     fecSupported,
    IN  GT_BOOL                     fecRequired,
    MV_HWS_PORT_INIT_PARAMS         *portParams,
    OUT MV_HWS_AP_CFG               *apCfgPtr,
    OUT GT_BOOL                     *singleModePtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      i;          /* iterator */
    GT_U32      localPort;  /* number of port in core */
    MV_HWS_PORT_STANDARD        portMode = NON_SUP_MODE;
    MV_HWS_PORT_INIT_PARAMS     tmpPortParams;
    GT_U32      *modesVectorPtr; /* pointer to bitmap of suggested interfaces */
    GT_U32      *fecAdvertiseArray; /* pointer to bitmap of suggested FEC modes */
    GT_U32      *fecRequestArray; /* pointer to bitmap of requested FEC modes */

    modesVectorPtr = &(apCfgPtr->modesVector);
    fecAdvertiseArray = &(apCfgPtr->fecAdvanceAbil);
    fecRequestArray = &(apCfgPtr->fecAdvanceReq);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 localPort);

    portParams->numOfActLanes = 0;
    for(i = 0, *modesVectorPtr = 0, *fecRequestArray = 0, *fecAdvertiseArray = 0;
         (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
             && (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
             && (i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS);
         i++)
    {
        switch(modesAdvertiseArrPtr[i].ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_1000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _1000Base_X;
                AP_CTRL_1000Base_KX_SET(*modesVectorPtr, 1);
                break;
            case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_10000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _10GBase_KX4;
                AP_CTRL_10GBase_KX4_SET(*modesVectorPtr, 1);
                break;
            case CPSS_PORT_INTERFACE_MODE_KR_E:
                if(CPSS_PORT_SPEED_10000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _10GBase_KR;
                    AP_CTRL_10GBase_KR_SET(*modesVectorPtr, 1);
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _40GBase_KR4;
                    AP_CTRL_40GBase_KR4_SET(*modesVectorPtr, 1);
                }
                else if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    switch(fecRequestedArrPtr[i])
                    {
                        case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= (AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC)
                                                                    << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        default:
                            /* do nothing to not delete advertisement from other modes */
                            break;
                    }
                    portMode = _25GBase_KR;
                    AP_CTRL_25GBase_KR1_SET(*modesVectorPtr, 1);
                    AP_CTRL_25GBase_KR1S_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_CR_E:
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    switch(fecRequestedArrPtr[i])
                    {
                        case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= (AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC)
                                                                    << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        default:
                            /* do nothing to not delete advertisement from other modes */
                            break;
                    }
                    portMode = _25GBase_CR;
                    AP_CTRL_25GBase_CR1_SET(*modesVectorPtr, 1);
                    AP_CTRL_25GBase_CR1S_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            case CPSS_PORT_INTERFACE_MODE_KR_S_E:
                 switch(fecRequestedArrPtr[i])
                 {
                    case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                    case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                        *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                        break;
                    default:
                        /* do nothing to not delete advertisement from other modes */
                        break;
                }
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR_S_E)
                    {
                        portMode = _25GBase_KR;
                        AP_CTRL_25GBase_KR1S_SET(*modesVectorPtr, 1);
                    }
                    else
                    {
                        portMode = _25GBase_CR;
                        AP_CTRL_25GBase_CR1S_SET(*modesVectorPtr, 1);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_KR_C_E:
            case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
            case CPSS_PORT_INTERFACE_MODE_CR_C_E:
            case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
                switch (fecAbilityArrPtr[i])
                {
                case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_BOTH << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
                default:
                    break;
                }
                switch (fecRequestedArrPtr[i])
                {
                case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_BOTH << FEC_ADVANCE_CONSORTIUM_SHIFT;
                case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
                default:
                    break;
                }
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR_C_E)
                    {
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:KR1 CONSORTIUM \n"));
                        portMode = _25GBase_KR_C;
                        AP_CTRL_25GBase_KR_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:KR1 CONSORTIUM rc %d\n",rc));
                    }
                    else
                    {
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:CR1 CONSORTIUM \n"));
                        portMode = _25GBase_CR_C;
                        AP_CTRL_25GBase_CR_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:CR1 CONSORTIUM rc %d\n",rc));
                    }
                }
                else if(CPSS_PORT_SPEED_50000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR2_C_E)
                    {
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:KR2 CONSORTIUM \n"));
                        portMode = _50GBase_KR2_C;
                        AP_CTRL_50GBase_KR2_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:KR2 CONSORTIUM rc %d\n",rc));
                    }
                    else
                    {
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:CR2 CONSORTIUM \n"));
                        portMode = _50GBase_CR2_C;
                        AP_CTRL_50GBase_CR2_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:CR2 CONSORTIUM rc %d\n",rc));
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_KR4_E:
                if(CPSS_PORT_SPEED_100G_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _100GBase_KR4;
                    AP_CTRL_100GBase_KR4_SET(*modesVectorPtr, 1);
                    /* AP_CTRL_100GBase_CR4_SET(*modesVectorPtr, 1); */
                    /* for 100G RS-FEC is mandatory - not subject for negotiation */
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _40GBase_KR4;
                    AP_CTRL_40GBase_KR4_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;

            case CPSS_PORT_INTERFACE_MODE_CR4_E:
                if(CPSS_PORT_SPEED_100G_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _100GBase_CR4;
                    AP_CTRL_100GBase_CR4_SET(*modesVectorPtr, 1);
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _40GBase_CR4;
                    AP_CTRL_40GBase_CR4_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Not allow CR and KR simultaneously */
        if((AP_CTRL_40GBase_KR4_GET(*modesVectorPtr) || AP_CTRL_10GBase_KR_GET(*modesVectorPtr) || AP_CTRL_10GBase_KX4_GET(*modesVectorPtr) || AP_CTRL_1000Base_KX_GET(*modesVectorPtr) || AP_CTRL_100GBase_KR4_GET(*modesVectorPtr) ||
            AP_CTRL_25GBase_KR1S_GET(*modesVectorPtr) || AP_CTRL_25GBase_KR1_GET(*modesVectorPtr) || AP_CTRL_25GBase_KR_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_50GBase_KR2_CONSORTIUM_GET(*modesVectorPtr)) &&
           (AP_CTRL_100GBase_CR4_GET(*modesVectorPtr) || AP_CTRL_25GBase_CR_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_50GBase_CR2_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_40GBase_CR4_GET(*modesVectorPtr) ||
            AP_CTRL_25GBase_CR1S_GET(*modesVectorPtr) || AP_CTRL_25GBase_CR1_GET(*modesVectorPtr)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: hwsPortModeParamsGetToBuffer(devNum[%d], portGroup[%d], portNum[%d], portMode[%d], portParams)", devNum, 0, localPort, portMode);
        rc = hwsPortModeParamsGetToBuffer(devNum,0,localPort,portMode,&tmpPortParams);
        if(GT_OK != rc)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return GT_BAD_PARAM");
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if((portParams->numOfActLanes < tmpPortParams.numOfActLanes))
        {
            hwsOsMemCopyFuncPtr(portParams, &tmpPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        }
    }

    if (0 == *modesVectorPtr)
    {
        CPSS_LOG_INFORMATION_MAC("Advertisement modes not defined");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *singleModePtr = (1 == i) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortApPortConfigSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now allowed negotiation on serdes lanes of port 0-3, because in Lion2
*       just these options are theoreticaly possible.
*
*/
static GT_STATUS internal_cpssDxChPortApPortConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         apEnable,
    IN  CPSS_DXCH_PORT_AP_PARAMS_STC    *apParamsPtr
)
{
    GT_STATUS                rc;        /* return code */
    GT_U32                   portGroup; /* local core number */
    GT_U32                   phyPortNum;/* port number in local core */
    MV_HWS_AP_CFG            apCfg;     /* AP parameters of port in HWS format */
    CPSS_DXCH_PORT_AP_PARAMS_STC  *localApParamsPtr;/* temporary pointer to
                                        AP configuration parameters structure */
    GT_BOOL                  singleMode = GT_FALSE; /* just one ifMode/speed pair adverised */
    GT_U8                    sdVecSizeMax;
    GT_U16                  *sdVectorMaxPtr;
    MV_HWS_PORT_INIT_PARAMS  curPortParams,portParamsBuffer;
    MV_HWS_PORT_STANDARD     portMode;
    GT_BOOL                ctleBiasOverride;
    CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT                 ctleBiasValue;
    GT_U32         regAddr;
    GT_U8         sdVecSize;
    GT_U16       *sdVectorPtr;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portEgfLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 phyPortNum);

    AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigSet:devNum=%d,portNum=%d,apEnable=%d mode %d speed %d fecAbilityArr %d fecRequesredArr %d\n",
                        devNum, portNum, apEnable, apParamsPtr->modesAdvertiseArr[0].ifMode, apParamsPtr->modesAdvertiseArr[0].speed,apParamsPtr->fecAbilityArr[0], apParamsPtr->fecRequestedArr[0]));

    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                    PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                    CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                    portNum,
                                                    apEnable? CPSS_EVENT_MASK_E: CPSS_EVENT_UNMASK_E);
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);
    if(GT_FALSE == apEnable)
    {
        if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortStop(devNum[%d], portGroup[%d], phyPortNum[%d], action[%s])", devNum, portGroup, phyPortNum, "PORT_POWER_DOWN");
            rc = mvHwsApPortStop(devNum,portGroup,phyPortNum,PORT_POWER_DOWN);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            }
        }
        else
        {
            MV_HWS_AP_PORT_STATUS   apStatus;

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], phyPortNum[%d] &apStatus)", devNum, portGroup, phyPortNum);
            cpssOsMemSet(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));

            rc = mvHwsApPortCtrlStatusGet(devNum, portGroup, phyPortNum, &apStatus);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

            if(apStatus.smState != 0)/* if AP enabled on port stop it as required */
            {
                PRV_CPSS_DXCH_PORT_STATE_STC  portStateStc; /* (dummy) current enable port state */
                rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &portStateStc);
                if (rc != GT_OK) {
                    return rc;
                }

                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStop(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);
                rc = mvHwsApPortCtrlStop(devNum, portGroup, phyPortNum);
                if (rc != GT_OK) {
                    CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                    return rc;
                }
                if (PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, phyPortNum) != CPSS_PORT_INTERFACE_MODE_NA_E) {
                    CPSS_PORTS_BMP_STC portsBmp;
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

                    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, phyPortNum) + CPSS_PORT_INTERFACE_MODE_NA_E, PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, phyPortNum) + CPSS_PORT_SPEED_NA_E);
                    if (rc != GT_OK) {
                        CPSS_LOG_INFORMATION_MAC("cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, portNum);
                        return rc;
                    }
                }

                /* enable port MAC after it was disabled when AP enabled and
                   could be left disabled if there was no link */
                rc = prvCpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                if (rc != GT_OK) {
                    AP_DBG_PRINT_MAC(("prvCpssDxChPortEnableSet:devNum=%d,portNum=%d,TRUE\n",
                                      devNum, portNum));
                    return rc;
                }
            }
            else
            {
                if (PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, phyPortNum) != CPSS_PORT_INTERFACE_MODE_NA_E && PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, phyPortNum) != CPSS_PORT_SPEED_NA_E )
                {
                    /*meaning not AP - we can not disable "regular" port with AP API*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
        }
        return rc;
    }

    localApParamsPtr = (apParamsPtr != NULL) ? apParamsPtr :
                                                (CPSS_DXCH_PORT_AP_PARAMS_STC*)(&prvCpssPortApDefaultParams);

    cpssOsMemSet(&apCfg, 0, sizeof(apCfg));
    sdVecSizeMax = 0;
    sdVectorMaxPtr = NULL;
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        rc = prvCpssDxChPortLion2ApModesVectorBuild(devNum, portNum,
                                               localApParamsPtr->modesAdvertiseArr,
                                               &apCfg.modesVector);
    }
    else
    {
        rc = prvCpssDxChPortBc2ApModesVectorBuild(devNum, portNum,
                                                  localApParamsPtr->modesAdvertiseArr,
                                                  localApParamsPtr->fecAbilityArr,
                                                  localApParamsPtr->fecRequestedArr,
                                                  localApParamsPtr->fecSupported,
                                                  localApParamsPtr->fecRequired,
                                                  &portParamsBuffer,
                                                  &apCfg,
                                                  &singleMode);
        sdVecSizeMax    = portParamsBuffer.numOfActLanes;
        sdVectorMaxPtr  = portParamsBuffer.activeLanesList;
    }

    if(rc != GT_OK)
    {
        AP_DBG_PRINT_MAC(("ApModesVectorBuild:devNum=%d,portNum=%d failed rc=%d\n",
                            devNum, portNum, rc));
        return rc;
    }

    if((localApParamsPtr->fcAsmDir != CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E)
        && (localApParamsPtr->fcAsmDir != CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    apCfg.fcAsmDir  = localApParamsPtr->fcAsmDir;
    apCfg.fcPause   = localApParamsPtr->fcPause;
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        if((apCfg.modesVector & _10GBase_KR_Bit2) ||
           (apCfg.modesVector & _40GBase_KR4_Bit3))
        {
            apCfg.fecReq    = localApParamsPtr->fecRequired;
            apCfg.fecSup    = localApParamsPtr->fecSupported;
        }
        else
        {
            apCfg.fecReq    = GT_FALSE;
            apCfg.fecSup    = GT_FALSE;
        }
    }

    apCfg.nonceDis  = localApParamsPtr->noneceDisable;
    if(localApParamsPtr->laneNum > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        apCfg.apLaneNum = localApParamsPtr->laneNum;
    }
    else
    {
        GT_U32 i;
        PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC   polarityValues;

        if (NULL == sdVectorMaxPtr)
        {/* added check here to prevent compiler warnings */
            AP_DBG_PRINT_MAC(("devNum=%d,portNum=%d:sdVectorMaxPtr==NULL\n",
                                devNum, portNum));
            return rc;
        }
        /* convert from relative laneNum to absolute laneNum */
        apCfg.apLaneNum = sdVectorMaxPtr[localApParamsPtr->laneNum];

        /* set the Polarity values on Serdeses if SW DB values initialized - only for xCat3 + Caelum (Bobk) device and above */
        apCfg.polarityVector = 0;
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr != NULL)
            {
                for(i=0; i < sdVecSizeMax; i++)
                {
                    polarityValues = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[sdVectorMaxPtr[i]];

                    if(polarityValues.txPolarity == GT_TRUE)
                    {
                        apCfg.polarityVector |= (1 << (2*i+1));
                    }

                    if(polarityValues.rxPolarity == GT_TRUE)
                    {
                        apCfg.polarityVector |= (1 << (2*i));
                    }
                }
            }
        }
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        /* For Aldrin: secondary means CPLL ref clock */
        apCfg.refClockCfg.refClockSource = SECONDARY_LINE_SRC;
        if(aldrinDebugCpllInitFlag == GT_FALSE)
        {
            apCfg.refClockCfg.refClockSource = PRIMARY_LINE_SRC;
        }
    }
    else
    {
        apCfg.refClockCfg.refClockSource = PRIMARY_LINE_SRC;
    }

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portRefClock.enableOverride == GT_TRUE)
        {
            switch (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portRefClock.portRefClockSource)
            {
                case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                    apCfg.refClockCfg.refClockSource = PRIMARY_LINE_SRC;
                    break;
                case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                    apCfg.refClockCfg.refClockSource = SECONDARY_LINE_SRC;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if (apCfg.refClockCfg.refClockSource == SECONDARY_LINE_SRC) { /* <-- EXTERNAL REF CLOCK IS USED */
                /* When BC3/Armstrong uses CPLL (which it's the default SW configuration) it is needed to  configure the following:
                                a. PECL_EN=0 /DFX/Units/DFX Server Registers/Device General Control 1 [0] = 0x0 (PECL Disable)
                                b. In addition, the software must configure it back to normal, once using on board ref clocks.
                */
                regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl1;
                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 1, 1);
                if (rc != GT_OK) {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }


            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, apParamsPtr->modesAdvertiseArr[0].ifMode, apParamsPtr->modesAdvertiseArr[0].speed, &portMode);

            if (rc != GT_OK) {
                cpssOsPrintf("error in prvCpssCommonPortIfModeToHwsTranslate,rc = %d, portMacMap = %d\n",
                             rc, apParamsPtr->modesAdvertiseArr[0].ifMode, apParamsPtr->modesAdvertiseArr[0].speed);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                cpssOsPrintf("error in hwsPortModeParamsGetToBuffer,rc = %d, portMacMap = %d\n", rc, phyPortNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            sdVecSize   = curPortParams.numOfActLanes;
            sdVectorPtr = curPortParams.activeLanesList;
            /* CPLL ref clock update */
            rc = prvCpssDxChPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, apCfg.refClockCfg.refClockSource, &apCfg.refClockCfg.refClockFreq);
            if (rc != GT_OK) {
                cpssOsPrintf("error in prvCpssDxChPortRefClockUpdate,rc = %d, portNum = %d\n", rc, portNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

        }

        /* translate cpss ifMode and speed to single Hws port mode */
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,localApParamsPtr->modesAdvertiseArr[0].ifMode,
                                                   localApParamsPtr->modesAdvertiseArr[0].speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate returned rc %d", rc);
        }

        /* get hws parameters for a given Hws port mode */
        if (hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "hwsPortModeParamsGetToBuffer returned null ");
        }

        apCfg.refClockCfg.cpllOutFreq = prvCpssDxChPortCpllConfigArr[curPortParams.activeLanesList[0]].outFreq;
        apCfg.refClockCfg.isValid = prvCpssDxChPortCpllConfigArr[curPortParams.activeLanesList[0]].valid;
    }

    rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum,
                                                 &apCfg.refClockCfg.refClockFreq);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update the CTLE Bias default value if override is enable */
    if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)))
    {
         if ( PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
         PRV_CPSS_PP_MAC(devNum)->revision == 0 )
         {
             apCfg.ctleBiasValue    = 0;
         }
         else
         {
            rc = cpssDxChPortCtleBiasOverrideEnableGet(devNum, portNum, &ctleBiasOverride, &ctleBiasValue);
            if (rc != GT_OK)
            {
                return rc;
            }
            else
            {
                /* if override enabled, update apCfg struct with the new value */
                if (ctleBiasOverride == 1)
                {
                    apCfg.ctleBiasValue    = ctleBiasValue;
                }
                else
                {
                    /* by default CTLE Value is zero */
                    apCfg.ctleBiasValue    = 0;
                }
            }
         }
    }
    else
    {
         apCfg.ctleBiasValue    = 0;
    }

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
CPSS_TBD_BOOKMARK_LION2 /* for now optimization algorithms cause AP excide 500 mSec */
        rc = cpssDxChPortSerdesAutoTuneOptAlgSet(devNum, portNum,
                                                 CPSS_PORT_SERDES_TRAINING_OPTIMISATION_NONE_E);
        if (rc != GT_OK)
        {
            return rc;
        }
        phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortStart(devNum[%d], portGroup[%d], phyPortNum[%d], apCfg{apLaneNum[%d], modesVector[%d], fcPause[%d], fcAsmDir[%d], fecSup[%d], fecReq[%d], nonceDis[%d], refClockCfg{refClockFreq[%d], refClockSource[%d]}})",
                            devNum, portGroup, phyPortNum, apCfg.apLaneNum,apCfg.modesVector,apCfg.fcPause,apCfg.fcAsmDir,apCfg.fecSup,apCfg.fecReq, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,apCfg.refClockCfg.refClockSource);

        AP_DBG_PRINT_MAC(("mvHwsApPortStart:devNum=%d,portGroup=%d,phyPortNum=%d,\
    laneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
    refClock=%d,refClockSource=%d\n",
                          devNum, portGroup, phyPortNum, apCfg.apLaneNum,
                          apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                          apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                          apCfg.refClockCfg.refClockSource));
        rc = mvHwsApPortStart(devNum, portGroup, phyPortNum, &apCfg);
    }
    else
    {
        /* if AC3 disable port MAC to prevent traffic during link establish, if not, disable is done by SrvCPU */
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            /*Doorbell interrupts is not supported for AC3*/
            if(singleMode)
            {
                rc = prvCpssDxChPortEnableSet(devNum, portNum, GT_FALSE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }

        if (singleMode)
        {/* if just single option (ifMode/speed pair) advertised resources can
            be allocated already */

            CPSS_PORTS_BMP_STC portsBmp;
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

            rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, localApParamsPtr->modesAdvertiseArr[0].ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, localApParamsPtr->modesAdvertiseArr[0].speed + CPSS_PORT_SPEED_NA_E);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n",rc, portNum);
                return rc;
            }
        }

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                     portNum,
                                                                     phyPortNum);

        /* in sip_5_20 egress filtering is set when MAC status is changed, i.e when
           link is down or mac is disabled the egress fileting is enabled, and vise versa.
           the default configuration for egress filtering is enabled, and due to the fact
           that Hws portEnble API does not configure egress filtering for disable when
           performing portEnable, we will set it here now.
           Note - the complementing action on egress filtering is done when disabling AP
           port by calling portEnable with False.
           TODO - portEnable API in Hws should be redesigned */
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                                             PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                                             PRV_CPSS_DXCH_EGF_CONVERT_MAC(portEgfLinkStatusState));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "egf filter (force link up) failed=%d ", rc);
            }
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStart(devNum[%d], portGroup[%d], phyPortNum[%d], apCfg{apLaneNum[%d], modesVector[%d], fcPause[%d], fcAsmDir[%d], fecSup[%d], fecReq[%d], nonceDis[%d], refClockCfg{refClockFreq[%d], refClockSource[%d]}})",
                            devNum, portGroup, phyPortNum, apCfg.apLaneNum,apCfg.modesVector,apCfg.fcPause,apCfg.fcAsmDir,apCfg.fecSup,apCfg.fecReq, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,apCfg.refClockCfg.refClockSource);
        AP_DBG_PRINT_MAC(("mvHwsApPortCtrlStart:devNum=%d,portGroup=%d,phyPortNum=%d,\
    laneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
    refClock=%d,refClockSource=%d,polarity=%d fecAdvanceReq=0x%x, fecAdvanceAbil=0x%x\n",
                          devNum, portGroup, phyPortNum, apCfg.apLaneNum,
                          apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                          apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                          apCfg.refClockCfg.refClockSource, apCfg.polarityVector, apCfg.fecAdvanceReq, apCfg.fecAdvanceAbil));
        rc = mvHwsApPortCtrlStart(devNum, portGroup, phyPortNum, &apCfg);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortApPortConfigSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now allowed negotiation on serdes lanes of port 0-3, because in Lion2
*       just these options are theoreticaly possible.
*
*/
GT_STATUS cpssDxChPortApPortConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         apEnable,
    IN  CPSS_DXCH_PORT_AP_PARAMS_STC    *apParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApPortConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apEnable, apParamsPtr));

    rc = internal_cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, apParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apEnable, apParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
* @param[out] apParamsPtr              - (ptr to) AP parameters of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApPortConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *apEnablePtr,
    OUT CPSS_DXCH_PORT_AP_PARAMS_STC    *apParamsPtr
)
{
    GT_STATUS       rc;         /* return code */
    MV_HWS_AP_CFG   apCfg;      /* AP configuration parameters */
    GT_U32          portGroup;  /* local core number */
    GT_U32          phyPortNum; /* port number in local core */
    GT_U32          i,j;          /* iterators */
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(devNum,
                                                                 portNum,
                                                                 phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(apParamsPtr);

    if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        /* Remote port is not AP enabled */
        *apEnablePtr = GT_FALSE;
        cpssOsMemSet(apParamsPtr, 0, sizeof(*apParamsPtr));
        return GT_OK;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    cpssOsMemSet(&apCfg, 0, sizeof(MV_HWS_AP_CFG));
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        /* Lion2 doesn't support AP on CPU port */
        if (portNum == CPSS_CPU_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApEngineInitGet(devNum[%d], portGroup[%d], *apEnablePtr)", devNum, portGroup);
        (GT_VOID)mvHwsApEngineInitGet(devNum,portGroup,apEnablePtr);
        if(GT_FALSE == *apEnablePtr)
        {
            return GT_OK;
        }
        phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortConfigGet(devNum[%d], portGroup[%d], phyPortNum[%d] *apEnablePtr, *apCfg)", devNum, portGroup, phyPortNum);
        rc = mvHwsApPortConfigGet(devNum,portGroup,phyPortNum,apEnablePtr,&apCfg);
        AP_DBG_PRINT_MAC(("mvHwsApPortConfigGet:rc=%d,devNum=%d,portGroup=%d,phyPortNum=%d,\
    apLaneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
    refClockFreq=%d,refClockSource=%d,apEnable=%d\n",
                          rc, devNum, portGroup, phyPortNum, apCfg.apLaneNum,
                          apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                          apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                          apCfg.refClockCfg.refClockSource, *apEnablePtr));
    }
    else
    {
        MV_HWS_AP_PORT_STATUS   apStatus;

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnableGet(devNum[%d])", devNum);
        *apEnablePtr = mvHwsServiceCpuEnableGet(devNum);
        if(GT_FALSE == *apEnablePtr)
        {
            return GT_OK;
        }
        if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                        PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
        {
            *apEnablePtr = GT_FALSE;
            return GT_OK;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], phyPortNum[%d] &apStatus)", devNum, portGroup, phyPortNum);
        cpssOsMemSet(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));
        rc = mvHwsApPortCtrlStatusGet(devNum, portGroup, phyPortNum, &apStatus);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        *apEnablePtr = (0 == apStatus.smState) ? GT_FALSE : GT_TRUE;
        if(GT_TRUE == *apEnablePtr)/* if AP disabled on port no need to read garbage */
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlCfgGet(devNum[%d], portGroup[%d], phyPortNum[%d] *apCfg)", devNum, portGroup, phyPortNum);
            rc = mvHwsApPortCtrlCfgGet(devNum,portGroup,phyPortNum,&apCfg);
            AP_DBG_PRINT_MAC(("mvHwsApPortCtrlCfgGet:rc=%d,devNum=%d,portGroup=%d,phyPortNum=%d,\
apLaneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
refClockFreq=%d,refClockSource=%d, fecAdvanceReq=0x%x, fecAdvanceAbil=0x%x\n",
                              rc, devNum, portGroup, phyPortNum, apCfg.apLaneNum,
                              apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                              apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                              apCfg.refClockCfg.refClockSource, apCfg.fecAdvanceReq, apCfg.fecAdvanceAbil));
        }
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    apParamsPtr->fcAsmDir = apCfg.fcAsmDir;
    apParamsPtr->fcPause = apCfg.fcPause;
    apParamsPtr->fecRequired = apCfg.fecReq;
    apParamsPtr->fecSupported = apCfg.fecSup;
    apParamsPtr->noneceDisable = apCfg.nonceDis;

    i = 0;
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        apParamsPtr->laneNum = apCfg.apLaneNum;

        if(apCfg.modesVector & _10GBase_KX4_Bit1)
        {
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_XGMII_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
        }

        if(apCfg.modesVector & _1000Base_KX_Bit0)
        {
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_1000_E;
        }

        if(apCfg.modesVector & _10GBase_KR_Bit2)
        {
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
        }

        if(apCfg.modesVector & _40GBase_KR4_Bit3)
        {
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_40000_E;
        }
    }
    else
    {
        if (apCfg.modesVector != 0)
        {
            MV_HWS_PORT_STANDARD    portMode;
            CPSS_DXCH_PORT_FEC_MODE_ENT fecR,fecA;

            if(AP_CTRL_10GBase_KX4_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_XGMII_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
            }

            if(AP_CTRL_1000Base_KX_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_1000_E;
            }

            if(AP_CTRL_10GBase_KR_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
            }

            if(AP_CTRL_40GBase_KR4_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_40000_E;
            }

            if(AP_CTRL_40GBase_CR4_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_DXCH_PORT_FEC_MODE_ENABLED_E: CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR4_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_40000_E;
            }

            if(AP_CTRL_100GBase_KR4_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                apParamsPtr->fecAbilityArr[i] = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_100G_E;
            }

            if(AP_CTRL_100GBase_CR4_GET(apCfg.modesVector))
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                apParamsPtr->fecAbilityArr[i] = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR4_E;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_100G_E;
            }

            if(AP_CTRL_25GBase_KR1S_GET(apCfg.modesVector)|| AP_CTRL_25GBase_CR1S_GET(apCfg.modesVector))
            {
                GT_U32 fecAdv = apCfg.fecAdvanceReq >> FEC_ADVANCE_BASE_R_SHIFT;
                if (AP_ST_HCD_FEC_RES_FC == fecAdv)
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
                }
                else
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                }
                if(AP_CTRL_25GBase_KR1S_GET(apCfg.modesVector))
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_KR_S_E;
                }
                else
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_CR_S_E;
                }
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
            }

            if(AP_CTRL_25GBase_KR1_GET(apCfg.modesVector) || AP_CTRL_25GBase_CR1_GET(apCfg.modesVector))
            {
                GT_U32 fecAdv = apCfg.fecAdvanceReq >> FEC_ADVANCE_BASE_R_SHIFT;
                if (AP_ST_HCD_FEC_RES_FC == fecAdv)
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
                }
                else if (AP_ST_HCD_FEC_RES_RS == fecAdv)
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                }
                else if ((AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC) == fecAdv)
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E;
                }
                else
                {
                    apParamsPtr->fecRequestedArr[i] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                }
                if(AP_CTRL_25GBase_KR1_GET(apCfg.modesVector))
                {
                        apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                }
                else
                {
                        apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_CR_E;
                }
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
            }
            if(AP_CTRL_ADV_CONSORTIUM_GET(apCfg.modesVector))
            {
                switch ((apCfg.fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_MASK) {
                case AP_ST_HCD_FEC_RES_FC:
                    fecA = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_RS:
                    fecA = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_BOTH:
                    fecA = CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_NONE:
                default:
                    fecA = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                    break;
                }
                switch ((apCfg.fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_MASK) {
                case AP_ST_HCD_FEC_RES_FC:
                    fecR = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_RS:
                    fecR = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_BOTH:
                    fecR = CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E;
                    break;
                case AP_ST_HCD_FEC_RES_NONE:
                default:
                    fecR = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                    break;
                }
                AP_DBG_PRINT_MAC(("cpssDxChPortApPortConfigGet:CONSORTIUM, %d fecAbilityArr=%d fecRequestedArr=%d\n",i,
                                apParamsPtr->fecAbilityArr[i], apParamsPtr->fecRequestedArr[i]));
                if(AP_CTRL_25GBase_KR_CONSORTIUM_GET(apCfg.modesVector))
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode =
                        CPSS_PORT_INTERFACE_MODE_KR_C_E;
                    apParamsPtr->fecAbilityArr[i] = fecA;
                    apParamsPtr->fecRequestedArr[i] = fecR;
                    apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
                }
                if(AP_CTRL_25GBase_CR_CONSORTIUM_GET(apCfg.modesVector))
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode =
                        CPSS_PORT_INTERFACE_MODE_CR_C_E;
                    apParamsPtr->fecAbilityArr[i] = fecA;
                    apParamsPtr->fecRequestedArr[i] = fecR;
                    apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
                }
                if(AP_CTRL_50GBase_KR2_CONSORTIUM_GET(apCfg.modesVector))
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode =
                        CPSS_PORT_INTERFACE_MODE_KR2_C_E;
                    apParamsPtr->fecAbilityArr[i] = fecA;
                    apParamsPtr->fecRequestedArr[i] = fecR;
                    apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_50000_E;
                }
                if(AP_CTRL_50GBase_CR2_CONSORTIUM_GET(apCfg.modesVector))
                {
                    apParamsPtr->modesAdvertiseArr[i].ifMode =
                        CPSS_PORT_INTERFACE_MODE_CR2_C_E;
                    apParamsPtr->fecAbilityArr[i] = fecA;
                    apParamsPtr->fecRequestedArr[i] = fecR;
                    apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_50000_E;
                }
            }
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,apParamsPtr->modesAdvertiseArr[i-1].ifMode,
                                                        apParamsPtr->modesAdvertiseArr[i-1].speed,
                                                        &portMode);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                cpssOsPrintf("error in hwsPortModeParamsGetToBuffer,rc = %d, portMacMap = %d\n", rc, phyPortNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            /* find the relative lane number */
            for(j=0 ; j<4 ; j++)
            {
                if (apCfg.apLaneNum == curPortParams.activeLanesList[j])
                {
                    break;
                }
            }
            if(4 == j)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            else
            {
                (apParamsPtr->laneNum = j);
            }
        }
        else
        {/* HWS pass garbage in such case */
            cpssOsMemSet(apParamsPtr, 0, sizeof(CPSS_DXCH_PORT_AP_PARAMS_STC));
            apParamsPtr->fecRequestedArr[0] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[0] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
        }
    }

    for(;i<CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS;i++)
    {
        apParamsPtr->modesAdvertiseArr[i].ifMode =
            CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
        apParamsPtr->modesAdvertiseArr[i].speed = CPSS_PORT_SPEED_NA_E;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
* @param[out] apParamsPtr              - (ptr to) AP parameters of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApPortConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *apEnablePtr,
    OUT CPSS_DXCH_PORT_AP_PARAMS_STC    *apParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApPortConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apEnablePtr, apParamsPtr));

    rc = internal_cpssDxChPortApPortConfigGet(devNum, portNum, apEnablePtr, apParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apEnablePtr, apParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApPortStatusGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum. xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApPortStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_AP_STATUS_STC   *apStatusPtr
)
{
    GT_STATUS   rc; /* return status */
    GT_U32                   portGroup;
    GT_U32                   apPortNum;
    MV_HWS_AP_PORT_STATUS    apResult;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 apPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apStatusPtr);

    cpssOsMemSet(&apResult, 0, sizeof(MV_HWS_AP_PORT_STATUS));
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, apPortNum);
    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        apPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortStatusGet(devNum[%d], portGroup[%d], apPortNum[%d], *apResult)", devNum, portGroup, apPortNum);
        rc = mvHwsApPortStatusGet(devNum, portGroup, apPortNum, &apResult);
    }
    else
    {
        if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                        PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, apPortNum))
        {
            AP_DBG_PRINT_MAC(("GE_only port\n"));
            apStatusPtr->hcdFound = GT_FALSE;
            return GT_OK;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], apPortNum[%d], *apResult)", devNum, portGroup, apPortNum);
        rc = mvHwsApPortCtrlStatusGet(devNum, portGroup, apPortNum, &apResult);
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    AP_DBG_PRINT_MAC(("cpssDxChPortApPortStatusGet:mvHwsApPortStatusGet:portGroup=%d,apPortNum=%d,apResult:apLaneNum=%d,hcdFound=%d,hcdLinkStatus=%d,\
hcdFecEn=%d,hcdFcPause=%d,hcdFcAsmDir=%d,postApPortMode=%d,postApPortNum=%d,preApPortNum=%d\n",
                      portGroup, apPortNum,
                      apResult.apLaneNum,
                      apResult.hcdResult.hcdFound,
                      apResult.hcdResult.hcdLinkStatus,
                      apResult.hcdResult.hcdFecEn,
                      apResult.hcdResult.hcdFcRxPauseEn,
                      apResult.hcdResult.hcdFcTxPauseEn,
                      apResult.postApPortMode,
                      apResult.postApPortNum,
                      apResult.preApPortNum));

    cpssOsMemSet(apStatusPtr, 0, sizeof(CPSS_DXCH_PORT_AP_STATUS_STC));
    if((apStatusPtr->hcdFound = apResult.hcdResult.hcdFound) != GT_TRUE)
    {/* nothing interesting any more */
        return GT_OK;
    }

    apStatusPtr->fecEnabled = apResult.hcdResult.hcdFecEn;

    switch(apResult.hcdResult.hcdFecType)
    {
        case AP_ST_HCD_FEC_RES_FC:
            apStatusPtr->fecType = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
            break;
        case AP_ST_HCD_FEC_RES_RS:
            apStatusPtr->fecType = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
            break;
        case AP_ST_HCD_FEC_RES_NONE:
        default:
            apStatusPtr->fecType = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            break;
    }

    switch(apResult.postApPortMode)
    {
        case _1000Base_X:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_1000_E;
            break;
        case _10GBase_KX4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case _10GBase_KR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case _40GBase_KR4:
        case _40GBase_KR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_40000_E;
            break;
        case _40GBase_CR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_40000_E;
            break;
        case _100GBase_KR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case _100GBase_CR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case _25GBase_KR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_CR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_KR_S:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_S_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_CR_S:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_S_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_KR_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _50GBase_KR2_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_50000_E;
            break;
        case _25GBase_CR_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _50GBase_CR2_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR2_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_50000_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        apStatusPtr->postApPortNum  = apResult.postApPortNum;
    }
    else
    {
        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum,
            PRV_CPSS_DXCH_PORT_TYPE_MAC_E, apResult.postApPortNum,
            &apStatusPtr->postApPortNum);
        if(rc != GT_OK)
        {
            AP_DBG_PRINT_MAC(("prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum=%d,PRV_CPSS_DXCH_PORT_TYPE_MAC_E,apResult.postApPortNum=%d):rc=%d\n",
                      devNum, apResult.postApPortNum, rc));
            return rc;
        }
    }
    apStatusPtr->fcTxPauseEn = apResult.hcdResult.hcdFcTxPauseEn;
    apStatusPtr->fcRxPauseEn = apResult.hcdResult.hcdFcRxPauseEn;

    AP_DBG_PRINT_MAC(("apStatusPtr:fcTxPauseEn=%d,fcRxPauseEn=%d,fecEnabled=%d,hcdFound=%d,ifMode=%d,speed=%d,postApPortNum=%d\n",
                      apStatusPtr->fcTxPauseEn,apStatusPtr->fcRxPauseEn,apStatusPtr->
                      fecEnabled,apStatusPtr->hcdFound,
                      apStatusPtr->portMode.ifMode,apStatusPtr->portMode.speed,
                      apStatusPtr->postApPortNum));

    return GT_OK;
}

/**
* @internal cpssDxChPortApPortStatusGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApPortStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_AP_STATUS_STC   *apStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApPortStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apStatusPtr));

    rc = internal_cpssDxChPortApPortStatusGet(devNum, portNum, apStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApResolvedPortsBmpGet function
* @endinternal
*
* @brief   Get bitmap of ports on port group (local core) where AP process finished
*         with agreed for both sides resolution
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core)
*
* @param[out] apResolvedPortsBmpPtr    - 1's set for ports of local core where AP
*                                      resolution acheaved
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - apResolvedPortsBmpPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApResolvedPortsBmpGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_U32  *apResolvedPortsBmpPtr
)
{
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(apResolvedPortsBmpPtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortResolutionMaskGet(devNum[%d], portGroupNum[%d], *apResolvedPortsBmpPtr)", devNum, portGroupNum);
    rc = mvHwsApPortResolutionMaskGet(devNum,portGroupNum,apResolvedPortsBmpPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        AP_DBG_PRINT_MAC(("mvHwsApPortResolutionMaskGet:portGroupNum=%d,rc=%d\n",
                          portGroupNum, rc));
        return rc;
    }

    AP_DBG_PRINT_MAC(("cpssDxChPortApResolvedPortsBmpGet:devNum=%d,portGroupNum=%d,apResolvedPortsBmp=0x%x\n",
                      devNum, portGroupNum, *apResolvedPortsBmpPtr));

    return GT_OK;
}

/**
* @internal cpssDxChPortApResolvedPortsBmpGet function
* @endinternal
*
* @brief   Get bitmap of ports on port group (local core) where AP process finished
*         with agreed for both sides resolution
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core)
*
* @param[out] apResolvedPortsBmpPtr    - 1's set for ports of local core where AP
*                                      resolution acheaved
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - apResolvedPortsBmpPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApResolvedPortsBmpGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_U32  *apResolvedPortsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApResolvedPortsBmpGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, apResolvedPortsBmpPtr));

    rc = internal_cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum, apResolvedPortsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, apResolvedPortsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApSetActiveMode function
* @endinternal
*
* @brief   Update port's AP active lanes according to new interface.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApSetActiveMode
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;        /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  phyPortNum;/* port number in local core */
    MV_HWS_PORT_STANDARD    portMode;  /* port i/f mode and speed translated to
                                            BlackBox enum */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

    AP_DBG_PRINT_MAC(("cpssDxChPortApSetActiveMode:devNum=%d,portNum=%d,ifMode=%d,speed=%d\n",
                        devNum, portNum, ifMode, speed));

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        AP_DBG_PRINT_MAC(("cpssDxChPortApSetActiveMode(%d,%d,%d,%d):prvCpssLion2CpssIfModeToHwsTranslate:rc=%d\n",
                          devNum, portNum, ifMode, speed, rc));
        return rc;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortSetActiveLanes(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d])", devNum, portGroup, phyPortNum, portMode);
    rc = mvHwsApPortSetActiveLanes(devNum, portGroup, phyPortNum, portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    AP_DBG_PRINT_MAC(("mvHwsApPortSetActiveLanes(%d,%d,%d,%d):rc=%d\n",
                      devNum, portGroup, phyPortNum, portMode, rc));

    return rc;
}

/**
* @internal cpssDxChPortApSetActiveMode function
* @endinternal
*
* @brief   Update port's AP active lanes according to new interface.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApSetActiveMode
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApSetActiveMode);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifMode, speed));

    rc = internal_cpssDxChPortApSetActiveMode(devNum, portNum, ifMode, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifMode, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPortApLock function
* @endinternal
*
* @brief   Acquires lock so host and AP machine won't access the same
*         resource at the same time.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) port state:
*                                      GT_TRUE - locked by HOST - can be configured
*                                      GT_FALSE - locked by AP processor - access forbidden
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_BAD_PTR               - statePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApLock
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroup;  /* local core number */
    GT_U32      phyPortNum; /* number of port in local core */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    if(apSemPrintEn)
    {
        cpssOsPrintf("cpssDxChPortApLock:devNum=%d,portNum=%d,",
                            devNum, portNum);
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvApLockGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);
    rc = mvApLockGet(devNum, portGroup, phyPortNum);
    *statePtr = (GT_OK == rc) ? GT_TRUE : GT_FALSE;
    if(apSemPrintEn)
    {
        cpssOsPrintf("state=%d\n", *statePtr);
    }
    if((GT_OK == rc) || (GT_NO_RESOURCE == rc))
    {
        return GT_OK;
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
}

/**
* @internal cpssDxChPortApLock function
* @endinternal
*
* @brief   Acquires lock so host and AP machine won't access the same
*         resource at the same time.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) port state:
*                                      GT_TRUE - locked by HOST - can be configured
*                                      GT_FALSE - locked by AP processor - access forbidden
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_BAD_PTR               - statePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApLock
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApLock);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortApLock(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortApUnLock function
* @endinternal
*
* @brief   Releases the synchronization lock (between Host and AP machine).
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApUnLock
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    GT_U32      portGroup;  /* local core number */
    GT_U32      phyPortNum; /* number of port in local core */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if(apSemPrintEn)
    {
        cpssOsPrintf("cpssDxChPortApUnLock:devNum=%d,portNum=%d\n",
                            devNum, portNum);
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvApLockRelease(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);
    rc = mvApLockRelease(devNum, portGroup, phyPortNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortApUnLock function
* @endinternal
*
* @brief   Releases the synchronization lock (between Host and AP machine).
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApUnLock
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApUnLock);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPortApUnLock(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apStatsPtr               - (ptr to) AP statistics information
* @param[out] intropAbilityMaxIntervalPtr - (ptr to) Introp Ability Max Interval
*                                      parameter - will help to represent
*                                      number of failed HCD cycles
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApStatsGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_AP_STATS_STC     *apStatsPtr,
    OUT GT_U16                          *intropAbilityMaxIntervalPtr
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portGroup;  /* local core number */
    GT_U32                  phyPortNum; /* port number in local core */
    MV_HWS_AP_PORT_STATS    apStats;    /* AP statistics information in HWS format*/
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apStatsPtr);
    CPSS_NULL_PTR_CHECK_MAC(intropAbilityMaxIntervalPtr);

    cpssOsMemSet(&apStats, 0, sizeof(MV_HWS_AP_PORT_STATS));
    cpssOsMemSet(&apIntrop, 0, sizeof(MV_HWS_AP_PORT_INTROP));

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    if (PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        return GT_OK;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatsGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);
    rc = mvHwsApPortCtrlStatsGet(devNum, portGroup, phyPortNum, &apStats);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlStatsGet: Hws return code is %d", rc);
        return rc;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);
    rc = mvHwsApPortCtrlIntropGet(devNum,portGroup,phyPortNum,&apIntrop);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlIntropGet:Hws return code is %d", rc);
        return rc;
    }

    *intropAbilityMaxIntervalPtr = apIntrop.abilityMaxInterval;

    apStatsPtr->txDisCnt = apStats.txDisCnt;
    apStatsPtr->abilityCnt = apStats.abilityCnt;
    apStatsPtr->abilitySuccessCnt = apStats.abilitySuccessCnt;
    apStatsPtr->linkFailCnt = apStats.linkFailCnt;
    apStatsPtr->linkSuccessCnt = apStats.linkSuccessCnt;
    apStatsPtr->hcdResoultionTime = apStats.hcdResoultionTime;
    apStatsPtr->linkUpTime = apStats.linkUpTime;

    return GT_OK;

}

/**
* @internal cpssDxChPortApStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apStatsPtr               - (ptr to) AP statistics information
* @param[out] intropAbilityMaxIntervalPtr - (ptr to) Introp Ability Max Interval
*                                      parameter - will help to represent
*                                      number of failed HCD cycles
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApStatsGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_AP_STATS_STC     *apStatsPtr,
    OUT GT_U16                          *intropAbilityMaxIntervalPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApStatsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apStatsPtr));

    rc = internal_cpssDxChPortApStatsGet(devNum, portNum, apStatsPtr, intropAbilityMaxIntervalPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apStatsPtr, intropAbilityMaxIntervalPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortApStatsReset
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    GT_U32      portGroup;  /* local core number */
    GT_U32      phyPortNum; /* number of port in local core */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortStatsReset(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);

    rc = mvHwsApPortCtrlStatsReset(devNum, portGroup, phyPortNum);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortApStatsReset
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApStatsReset);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPortApStatsReset(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
static GT_STATUS internal_cpssDxChPortApIntropSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_AP_INTROP_STC    *apIntropPtr
)
{
    GT_STATUS               rc;        /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  phyPortNum;/* port number in local core */
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apIntropPtr);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    apIntrop.attrBitMask = apIntropPtr->attrBitMask;
    apIntrop.txDisDuration = apIntropPtr->txDisDuration;
    apIntrop.abilityDuration = apIntropPtr->abilityDuration;
    apIntrop.abilityMaxInterval = apIntropPtr->abilityMaxInterval;
    apIntrop.abilityFailMaxInterval = apIntropPtr->abilityFailMaxInterval;
    apIntrop.apLinkDuration = apIntropPtr->apLinkDuration;
    apIntrop.apLinkMaxInterval = apIntropPtr->apLinkMaxInterval;
    apIntrop.pdLinkDuration = apIntropPtr->pdLinkDuration;
    apIntrop.pdLinkMaxInterval = apIntropPtr->pdLinkMaxInterval;

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropSet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);

    rc = mvHwsApPortCtrlIntropSet(devNum,portGroup,phyPortNum,&apIntrop);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;

}

/**
* @internal cpssDxChPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssDxChPortApIntropSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_AP_INTROP_STC    *apIntropPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApIntropSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apIntropPtr));

    rc = internal_cpssDxChPortApIntropSet(devNum, portNum, apIntropPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apIntropPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApIntropGet function
* @endinternal
*
* @brief   Returns AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*
* @param[out] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
static GT_STATUS internal_cpssDxChPortApIntropGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT  CPSS_DXCH_PORT_AP_INTROP_STC    *apIntropPtr
)
{
    GT_STATUS               rc;        /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  phyPortNum;/* port number in local core */
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apIntropPtr);

    cpssOsMemSet(&apIntrop, 0, sizeof(MV_HWS_AP_PORT_INTROP));

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, portGroup, phyPortNum);

    rc = mvHwsApPortCtrlIntropGet(devNum,portGroup,phyPortNum,&apIntrop);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    apIntropPtr->attrBitMask = apIntrop.attrBitMask;
    apIntropPtr->txDisDuration = apIntrop.txDisDuration;
    apIntropPtr->abilityDuration = apIntrop.abilityDuration;
    apIntropPtr->abilityMaxInterval = apIntrop.abilityMaxInterval;
    apIntropPtr->abilityFailMaxInterval = apIntrop.abilityFailMaxInterval;
    apIntropPtr->apLinkDuration = apIntrop.apLinkDuration;
    apIntropPtr->apLinkMaxInterval = apIntrop.apLinkMaxInterval;
    apIntropPtr->pdLinkDuration = apIntrop.pdLinkDuration;
    apIntropPtr->pdLinkMaxInterval = apIntrop.pdLinkMaxInterval;

    return rc;

}

/**
* @internal cpssDxChPortApIntropGet function
* @endinternal
*
* @brief   Returns AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*
* @param[out] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssDxChPortApIntropGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT  CPSS_DXCH_PORT_AP_INTROP_STC    *apIntropPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApIntropGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apIntropPtr));

    rc = internal_cpssDxChPortApIntropGet(devNum, portNum, apIntropPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apIntropPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApDebugInfoGet function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] cpssLog                  - boolean value indicate if log will be part of CPSS log
*                                      mechanism or terminal print
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssDxChPortApDebugInfoGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                cpssLog
)
{

    GT_STATUS               rc;         /* return code */
    GT_U32                  phyPortNum; /* port number in local core */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    if(portNum == AP_LOG_ALL_PORT_DUMP)
    {
        phyPortNum = portNum;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvApPortCtrlDebugInfoShow(apPortNum[%d], cpssLog[%d])", phyPortNum, cpssLog);
    rc = mvApPortCtrlDebugInfoShow(devNum, phyPortNum,cpssLog);

    if(rc != GT_OK)
    {
          CPSS_LOG_INFORMATION_MAC("mvApPortCtrlDebugInfoShow: Hws return code is %d", rc);
          return rc;
    }

    return rc;
}

/**
* @internal cpssDxChPortApDebugInfoGet function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] cpssLog                  - boolean value indicate if log will be part of CPSS log mechanism or terminal print
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssDxChPortApDebugInfoGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                cpssLog
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApDebugInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cpssLog));

    rc = internal_cpssDxChPortApDebugInfoGet(devNum, portNum, cpssLog);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cpssLog));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssDxChPortApPortEnableCtrlSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     srvCpuEnable
)
{

    GT_STATUS               rc;         /* return code */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    rc = mvHwsApPortCtrlEnablePortCtrlSet(devNum,srvCpuEnable);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;

}

/**
* @internal cpssDxChPortApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssDxChPortApPortEnableCtrlSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     srvCpuEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApPortEnableCtrlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srvCpuEnable));

    rc = internal_cpssDxChPortApPortEnableCtrlSet(devNum, srvCpuEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srvCpuEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*                                      portNum       - number of physical port
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssDxChPortApPortEnableCtrlGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *srvCpuEnablePtr
)
{

    GT_STATUS               rc;         /* return code */


    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    rc = mvHwsApPortCtrlEnablePortCtrlGet(devNum,srvCpuEnablePtr);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;

}

/**
* @internal cpssDxChPortApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssDxChPortApPortEnableCtrlGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *srvCpuEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApPortEnableCtrlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srvCpuEnablePtr));

    rc = internal_cpssDxChPortApPortEnableCtrlGet(devNum, srvCpuEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srvCpuEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
*
* @note   APPLICABLE DEVICES:      Caelum; ; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxOffsetsPtr       - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Offsets are limited to the range of minimum -15 and maximum +15 step size(2)
*
*/
static GT_STATUS internal_cpssDxChPortApSerdesTxParametersOffsetSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U16      offsetsPacked;
    GT_8        txAmpOffset, txEmph0Offset, txEmph1Offset;
    MV_HWS_PORT_STANDARD portMode;
    GT_U32      phyPortNum, portGroup;
    MV_HWS_PORT_INIT_PARAMS curPortParamsPtr;

    /* sanity checks */
    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_BOBCAT2_E);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    CPSS_NULL_PTR_CHECK_MAC(serdesTxOffsetsPtr);

    /* getting mac number */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 phyPortNum);
    /* getting port group number */
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    /* fectching values from staructure */
    txAmpOffset = serdesTxOffsetsPtr->txAmpOffset;
    txEmph0Offset = serdesTxOffsetsPtr->txEmph0Offset;
    txEmph1Offset = serdesTxOffsetsPtr->txEmph1Offset;

    /* range check */
    if (txAmpOffset>15 || txAmpOffset<-15 ||
        txEmph0Offset>15 || txEmph0Offset<-15 ||
        txEmph1Offset>15 || txEmph1Offset<-15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                "offsets range is -15 and +15. given ranges: txAmpOffset=%d, txEmph0Offset=%d, txEmph1Offset=%d",
                                      txAmpOffset, txEmph0Offset,txEmph1Offset);
    }

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,serdesTxOffsetsPtr->ifMode,
                                            serdesTxOffsetsPtr->speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate returned rc %d",rc );
    }

    /* get hws parameters for a given Hws port mode */
    if (hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParamsPtr) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "hwsPortModeParamsGetToBuffer returned null " );
    }

    /* lane index check */
    if (serdesLane >= curPortParamsPtr.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"lane index %d not belong to port ", serdesLane);

    }
    /* packing values */
    offsetsPacked =
         ( ( (txAmpOffset<0) ? (((txAmpOffset*-1) & 0xF) | (0x10)) : (txAmpOffset & 0xF))
       |
         ((txEmph0Offset<0) ? ((((txEmph0Offset*-1) & 0xF) <<5) | (0x10<<5)) : ((txEmph0Offset & 0xF)<<5))
       |
         ((txEmph1Offset<0) ? ((((txEmph1Offset*-1) & 0xF) <<10) | (0x10<<10)) : ((txEmph1Offset & 0xF)<<10)));

    CPSS_LOG_INFORMATION_MAC("given values: txAmpOffset=%d, txEmph0Offset=%d, txEmph1Offset=%d",
                                            txAmpOffset, txEmph0Offset,txEmph1Offset);
    CPSS_LOG_INFORMATION_MAC("offsetsPacked=0x%08x", offsetsPacked);

    /* passing packed ofssets with the appropriated hws port mode to AP */
    rc = mvHwsApSerdesTxParametersOffsetSet(devNum, portGroup, phyPortNum,
                                curPortParamsPtr.activeLanesList[serdesLane], offsetsPacked, curPortParamsPtr.serdesSpeed);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsApSerdesTxParametersOffsetSet- Hws return code is %d", rc);
    }

    return GT_OK;

}

/**
* @internal cpssDxChPortApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
*
* @note   APPLICABLE DEVICES:      Caelum; ; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
*                                      serdesNumber  - serdes lane number to configure the offsets to
* @param[in] serdesTxOffsetsPtr       - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Offsets are limited to the range of minimum -15 and maximum +15 step size(2)
*
*/
GT_STATUS cpssDxChPortApSerdesTxParametersOffsetSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApSerdesTxParametersOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesLane, serdesTxOffsetsPtr));

    rc = internal_cpssDxChPortApSerdesTxParametersOffsetSet(devNum, portNum, serdesLane, serdesTxOffsetsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesLane, serdesTxOffsetsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/* The below function is a backdoor to application to check the ability to change TX parameters without the need for LUA/Galtis support */
GT_STATUS cpssDxChPortApSerdesTxParametersOffsetSet1
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  GT_8                    txAmpOffset,
    IN  GT_8                    txEmph0Offset,
    IN  GT_8                    txEmph1Offset,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed

)
{
    GT_STATUS rc;
    CPSS_PORT_AP_SERDES_TX_OFFSETS_STC serdesTxOffsets;

    serdesTxOffsets.txAmpOffset = txAmpOffset;
    serdesTxOffsets.txEmph0Offset = txEmph0Offset;
    serdesTxOffsets.txEmph1Offset = txEmph1Offset;
    serdesTxOffsets.ifMode = ifMode;
    serdesTxOffsets.speed = speed;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    rc = internal_cpssDxChPortApSerdesTxParametersOffsetSet(devNum, portNum, serdesLane, &serdesTxOffsets);
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssDxChPortApPortConfigSetConvert function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now allowed negotiation on serdes lanes of port 0-3, because in Lion2
*       just these options are theoreticaly possible.
*
*/
GT_STATUS prvCpssDxChPortApPortConfigSetConvert
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_BOOL                    apEnable,
    IN  CPSS_PORT_AP_PARAMS_STC    *apParamsPtr
)
{
    return cpssDxChPortApPortConfigSet(devNum,portNum,apEnable,(CPSS_DXCH_PORT_AP_PARAMS_STC*)apParamsPtr);
}

/**
* @internal prvCpssDxChPortApPortStatusGetConvert function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortApPortStatusGetConvert
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT CPSS_PORT_AP_STATUS_STC   *apStatusPtr
)
{
    return cpssDxChPortApPortStatusGet(devNum,portNum, (CPSS_DXCH_PORT_AP_STATUS_STC*)apStatusPtr);
}

/**
* @internal internal_cpssDxChPortApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] rxOverrideParamsPtr      - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChPortApSerdesRxParametersManualSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_RX_CONFIG_STC *rxOverrideParamsPtr

)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroup, phyPortNum;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  rxCfg;

    /* sanity checks */
    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION2_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_BOBCAT2_E);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    CPSS_NULL_PTR_CHECK_MAC(rxOverrideParamsPtr);
    /* getting mac number */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 phyPortNum);

    if((rxOverrideParamsPtr->lowFrequency > 15)  ||
       (rxOverrideParamsPtr->highFrequency > 15) ||
       (rxOverrideParamsPtr->bandWidth > 15) ||
       (rxOverrideParamsPtr->loopBandwidth > 15) ||
       (rxOverrideParamsPtr->squelch > 308)||
       ((rxOverrideParamsPtr->serdesSpeed != CPSS_PORT_SERDES_SPEED_25_78125_E) && (rxOverrideParamsPtr->serdesSpeed != CPSS_PORT_SERDES_SPEED_10_3125_E)) ||
       (rxOverrideParamsPtr->etlMinDelay > 31) ||
       (rxOverrideParamsPtr->etlMaxDelay > 31))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    /* lane index check */
    if (serdesLane >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"lane index %d not belong to port ", serdesLane);

    }
    /* getting port group number */
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, (phyPortNum+serdesLane));

    rxCfg.ctleParams.dcGain         = rxOverrideParamsPtr->dcGain;
    rxCfg.ctleParams.lowFrequency   = rxOverrideParamsPtr->lowFrequency;
    rxCfg.ctleParams.highFrequency  = rxOverrideParamsPtr->highFrequency;
    rxCfg.ctleParams.bandWidth      = rxOverrideParamsPtr->bandWidth;
    rxCfg.ctleParams.loopBandwidth  = rxOverrideParamsPtr->loopBandwidth;
    rxCfg.ctleParams.squelch        = rxOverrideParamsPtr->squelch;
    rxCfg.fieldOverrideBmp          = rxOverrideParamsPtr->fieldOverrideBmp;
    rxCfg.serdesSpeed               = (rxOverrideParamsPtr->serdesSpeed == CPSS_PORT_SERDES_SPEED_10_3125_E)? _10_3125G: _25_78125G;
    rxCfg.etlParams.etlMinDelay     = rxOverrideParamsPtr->etlMinDelay;
    rxCfg.etlParams.etlMaxDelay     = rxOverrideParamsPtr->etlMaxDelay;
    rxCfg.etlParams.etlEnableOverride = (GT_U8)rxOverrideParamsPtr->etlEnable;


    /* passing rx params to AP */
    rc = mvHwsApSerdesRxParametersManualSet(devNum, portGroup, (phyPortNum+serdesLane), &rxCfg);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsApSerdesRxParametersManualSet- Hws return code is %d", rc);
    }

    return GT_OK;

}


/**
* @internal cpssDxChPortApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] rxOverrideParamsPtr      - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortApSerdesRxParametersManualSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_RX_CONFIG_STC *rxOverrideParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortApSerdesRxParametersManualSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesLane, rxOverrideParamsPtr));
    rc = internal_cpssDxChPortApSerdesRxParametersManualSet(devNum, portNum, serdesLane, rxOverrideParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesLane, rxOverrideParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


