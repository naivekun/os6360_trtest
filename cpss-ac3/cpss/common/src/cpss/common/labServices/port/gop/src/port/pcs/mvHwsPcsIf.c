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
* @file mvHwsPcsIf.c
*
* @brief PCS level APIs
*
* @version   22
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifdef __GNUC__
/* Test for GCC > 4.4 */
# if  (__GNUC__*100+__GNUC_MINOR__) >=404
#  define __NOOPT__ __attribute__((optimize("O0")))
# endif
#endif

#ifdef HWS_DEBUG
extern GT_FILEP fdHws;
#endif

static MV_HWS_PCS_FUNC_PTRS hwsPcsFuncsPtr[HWS_MAX_DEVICE_NUM][LAST_PCS];

/**
* @internal mvHwsPcsSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendFaultSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_STRING_MAC("pcs type NA"), LOG_ARG_GEN_PARAM_MAC(pcsType));
    }

    if (hwsPcsFuncsPtr[devNum][pcsType].pcsSendFaultSetFunc == NULL)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_STRING_MAC("function is NULL"), LOG_ARG_GEN_PARAM_MAC(pcsType));
    }
#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Send Fault\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc());
    osFclose(fdHws);
#endif

    CHECK_STATUS_EXT(hwsPcsFuncsPtr[devNum][pcsType].pcsSendFaultSetFunc(devNum, portGroup, pcsNum, portMode, send),
                     LOG_ARG_STRING_MAC("pcs type"), LOG_ARG_GEN_PARAM_MAC(pcsType));

    return GT_OK;
}

/**
* @internal mvHwsPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType = pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPcsReset null hws param ptr"));
    }

    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    if (hwsPcsFuncsPtr[devNum][pcsType].pcsResetFunc == NULL)
    {
        return GT_OK;
    }
#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Reset: pcsType %s, action %d.(0 - for reset)\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(), action);
    osFclose(fdHws);
#endif
    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        /* default value in port elements DB is CGPCS (for NO-FEC and RS-FEC), for FC-FEC should be updated*/
        /* only port #0 in MSPG can be configured to 50G and FC-FEC */
        if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard) && (curPortParams.portFecMode == FC_FEC))
        {
            portPcsType = MMPCS;
        }
        /* default value in port elements DB is MMPCS (for NO-FEC and FC-FEC), for RS-FEC should be updated */
        if((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            portPcsType = CGPCS;
        }
    }

    CHECK_STATUS_EXT(hwsPcsFuncsPtr[devNum][portPcsType].pcsResetFunc(devNum, portGroup, portMacNum, portMode, action), LOG_ARG_STRING_MAC("serdes type"), LOG_ARG_GEN_PARAM_MAC(pcsType));
    return GT_OK;
}

/**
* @internal mvHwsPcsModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS) ||
        (hwsPcsFuncsPtr[devNum][curPortParams.portPcsType].pcsModeCfgFunc == NULL))
    {
      return GT_BAD_PARAM;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Mode Cfg: pcsType %s, number of lanes %d.\n",
      hwsPcsFuncsPtr[devNum][curPortParams.portPcsType].pcsTypeGetFunc(), curPortParams.numOfActLanes);
    osFclose(fdHws);
#endif

    portPcsType = curPortParams.portPcsType;

    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        /* default value in port elements DB is MMPCS (for NO-FEC and FC-FEC), for RS-FEC should be updated */
        if((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            portPcsType = CGPCS;
        }

        /* default value in port elements DB is CGPCS (for NO-FEC and RS-FEC), for FC-FEC should be updated*/
        /* only port #0 in MSPG can be configured to 50G and FC-FEC */
        if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard) && (curPortParams.portFecMode == FC_FEC))
        {
            portPcsType = MMPCS;
        }
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][portPcsType].pcsModeCfgFunc(devNum, portGroup, phyPortNum, portMode));

    return GT_OK;
}


/**
* @internal mvHwsPcsLoopbackCfg function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_RESET pcsState = UNRESET;
    GT_U32       numOfLanes;
    GT_STATUS res;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    tmpPcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS) ||
        (hwsPcsFuncsPtr[devNum][pcsType].pcsLbCfgFunc == NULL))
    {
      return GT_BAD_PARAM;
    }

    /* check if PCS us under reset or not */
    res = mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, portMode, &numOfLanes);
    if(res == GT_OK) /* check if this function is supported for this PCS */
    {
        pcsState = (numOfLanes == 0) ? RESET : UNRESET;
    }
    else if(res != GT_NOT_SUPPORTED)
    {
        return res;
    }

    /* rest PCS if it's not in reset*/
    if(pcsState == UNRESET)
    {
        CHECK_STATUS(mvHwsPcsReset (devNum,portGroup,phyPortNum,portMode,pcsType,RESET));
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Loopback Cfg: pcsType %s, type %d.\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(), lbType);
    osFclose(fdHws);
#endif
    tmpPcsType = pcsType;
    if ((CGPCS == pcsType) && HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
    {   /* LB services are supplied from MMPCS even though the elements database is set to CGPCS for 2-lanes mode */
        tmpPcsType = MMPCS;
    }
    CHECK_STATUS(hwsPcsFuncsPtr[devNum][tmpPcsType].pcsLbCfgFunc(devNum, portGroup, phyPortNum, portMode, lbType));

    if(pcsState == UNRESET)
    {
        CHECK_STATUS(mvHwsPcsReset (devNum,portGroup,phyPortNum,portMode,pcsType,UNRESET));
    }

    return GT_OK;
}

/**
* @internal mvHwsPcsLoopbackCfgGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      pcsNum    - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfgGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;

    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            portPcsType = MMPCS;
        }

        if (hwsPcsFuncsPtr[devNum][portPcsType].pcsLbCfgGetFunc == NULL)
        {
            return GT_NOT_SUPPORTED;
        }
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][portPcsType].pcsLbCfgGetFunc(devNum, portGroup, phyPortNum, portMode, lbType));

    return GT_OK;
}

/**
* @internal mvHwsPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*                                      pattern   - pattern to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_TEST_GEN_PATTERN  portPattern
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS) ||
        (hwsPcsFuncsPtr[devNum][pcsType].pcsTestGenFunc == NULL))
    {
      return GT_BAD_PARAM;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Test Generator Cfg: pcsType %s, pattern is %d.\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(),portPattern);
    osFclose(fdHws);
#endif
    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsTestGenFunc(devNum, portGroup, pcsNum, portPattern));

    return GT_OK;
}

/**
* @internal mvHwsPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS) ||
        (hwsPcsFuncsPtr[devNum][pcsType].pcsTestGenFunc == NULL))
    {
      return GT_BAD_PARAM;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Test Generator Status: pcsType %s, pattern is %d.\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(),pattern);
    osFclose(fdHws);
#endif
    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsTestGenStatusFunc(devNum, portGroup, pcsNum, pattern, status));

    return GT_OK;
}

/**
* @internal mvHwsPcsRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsRxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_RESET            action
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS) ||
        (hwsPcsFuncsPtr[devNum][pcsType].pcsRxResetFunc == NULL))
    {
      return GT_BAD_PARAM;
    }
#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run RX PCS Reset: pcsType %s, action %d.(0 - for reset)\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(), action);
    osFclose(fdHws);
#endif

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsRxResetFunc(devNum, portGroup, pcsNum, action));
    return GT_OK;
}

/**
* @internal mvHwsPcsExtPllCfg function
* @endinternal
*
* @brief   Set the selected PCS type and number to external PLL mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsExtPllCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType
)
{
    if ((pcsType != MMPCS) || (hwsPcsFuncsPtr[devNum][pcsType].pcsExtPllCfgFunc == NULL))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\nRun External PLL configuration: pcsType %s, pscNum %d\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(), pcsNum);
    osFclose(fdHws);
#endif

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsExtPllCfgFunc(devNum, portGroup, pcsNum));
    return GT_OK;
}

/**
* @internal mvHwsPcsSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    GT_BOOL                 maskEn
)
{
    if (hwsPcsFuncsPtr[devNum][pcsType].pcsSignalDetectMaskEn == NULL)
    {
      return GT_OK;
    }
#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\nRun Signal Detect Mask configuration: pcsType %s, pscNum %d\n",
      hwsPcsFuncsPtr[devNum][pcsType].pcsTypeGetFunc(), pcsNum);
    osFclose(fdHws);
#endif

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsSignalDetectMaskEn(devNum, portGroup, pcsNum, maskEn));
    return GT_OK;
}

/**
* @internal mvHwsPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*                                      fecType   - 0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC, 2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_FEC_MODE    portFecType
)
{
    if (hwsPcsFuncsPtr[devNum][pcsType].pcsFecCfgFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsFecCfgFunc(devNum, portGroup,
                                                        pcsNum, portFecType));
    return GT_OK;
}

/**
* @internal mvHwsPcsFecConfigGet function
* @endinternal
*
* @brief   Return the FEC state on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsType = curPortParams.portPcsType;

    if (hwsPcsFuncsPtr[devNum][pcsType].pcsFecCfgGetFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsFecCfgGetFunc(devNum, portGroup,
                                                        phyPortNum, portMode, portFecTypePtr));
    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsType = curPortParams.portPcsType;

    if((MMPCS == pcsType) || (CGPCS == pcsType))
    {
        if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            pcsType = MMPCS;
        }
    }

    if (hwsPcsFuncsPtr[devNum][pcsType].pcsActiveStatusGetFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsActiveStatusGetFunc(devNum, portGroup, curPortParams.portPcsNumber, numOfLanes));
    return GT_OK;
}
#endif

/**
* @internal mvHwsPcsCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes (currently used only on MMPCS).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS) || (curPortParams.numOfActLanes == 0))
    {
      return GT_BAD_PARAM;
    }

    pcsType = curPortParams.portPcsType;

    if((MMPCS == pcsType) || (CGPCS == pcsType))
    {
        if((curPortParams.portFecMode == FC_FEC) && HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            /* in case of FC-FEC the PCS type for GearBox locked is MMPCS */
            pcsType = MMPCS;
        }
        /*the PCS active mode is MMPCS but PCS lock services for RS_FEC are supplied from CGPCS unit*/
        if((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            pcsType = CGPCS;
        }
    }
    if (hwsPcsFuncsPtr[devNum][pcsType].pcsCheckGearBoxFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][pcsType].pcsCheckGearBoxFunc(devNum, portGroup, portMacNum, portMode, laneLock));
    return GT_OK;
}

/**
* @internal mvHwsPcsAlignLockGet function
* @endinternal
*
* @brief   Read align status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsAlignLockGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *lock
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;
    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        /* default value in port elements DB is CGPCS (for NO-FEC and RS-FEC) */
        if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            portPcsType = MMPCS;
        }
    }

    if (hwsPcsFuncsPtr[devNum][portPcsType].pcsAlignLockGetFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][portPcsType].pcsAlignLockGetFunc(devNum, portGroup, curPortParams.portPcsNumber, lock));
    return GT_OK;
}

/**
* @internal mvHwsPcsPortLinkStatusGet function
* @endinternal
*
* @brief   Read link status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus                - link Status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsPortLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, phyPortNum, portMode);

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams->portPcsType == PCS_NA) || (curPortParams->portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams->portPcsType;

    if (hwsPcsFuncsPtr[devNum][portPcsType].pcsLinkStatusGetFunc == NULL)
    {
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(hwsPcsFuncsPtr[devNum][portPcsType].pcsLinkStatusGetFunc(devNum, portGroup, curPortParams->portPcsNumber, linkStatus));

    return GT_OK;
}

/**
* @internal hwsPcsGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsGetFuncPtr
(
    GT_U8                 devNum,
    MV_HWS_PCS_FUNC_PTRS  **hwsFuncsPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_NOT_SUPPORTED;
    }

    *hwsFuncsPtr = &hwsPcsFuncsPtr[devNum][0];

    return GT_OK;
}

