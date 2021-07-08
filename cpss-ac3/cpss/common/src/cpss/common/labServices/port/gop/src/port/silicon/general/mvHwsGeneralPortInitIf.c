/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsGeneralPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#ifndef ASIC_SIMULATION
extern int mvHwsAvagoSerdesSpicoInterrupt
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    unsigned int    interruptCode,
    unsigned int    interruptData,
    int             *result
);
#endif /* ASIC_SIMULATION */

#define POSITIVE_NEGATIVE_VALUE(_val) \
    (((_val & 0x10))?                                  \
     ( (GT_8)255 - (GT_8)(((_val & 0xF) *2) - 1) ) :    \
     (_val)*2)

#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
/**
* @internal mvHwsGeneralPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortInit
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD      portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32 tmpData = 0;
    GT_U32 data = 0;
#endif /* MV_HWS_REDUCED_BUILD */
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        /* Turning BobK OOB port 90 with mac 62 to network port */
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (curPortParams.portMacNumber==62))
        {
            /* Setting serdes RX If selector to 1 */
            CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, 0, MV_HWS_SERDES_NUM(curLanesList[0]), HWS_DEV_SERDES_TYPE(devNum), 1 << 3));

            /* Change Serdes 20 MUX to mac 62 by configurting bit[12] of a DFX server register to 0 */
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, 0xf829c, &tmpData));
            data  = tmpData & 0xFFFFEFFF;
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, 0xf829c, data));
        }
#endif /* MV_HWS_REDUCED_BUILD */
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
            || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) ||
            (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
            hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        }

        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));

        /* set P Sync Bypass */
        CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, portGroup, phyPortNum, portMode));
        /* Configures the DP/Core Clock Selector on port */
        CHECK_STATUS(mvHwsClockSelectorConfig(devNum, portGroup, phyPortNum, portMode));
    }

    /* Configure MAC/PCS */
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode));

    /* Implement WA for 1G/SGMII2_5 portModes: when on the same GOP there is RXAUI and 1G/SGMII2_5 port,
       corrupted packets receive on RXAUI port */
    if ((curPortParams.portMacType == GEMAC_SG) || (curPortParams.portMacType == GEMAC_X))
    {
        /* set speed MAC mode in 10G */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, curPortParams.portMacNumber, MSM_PORT_MAC_CONTROL_REGISTER3, (1 << 13), (7 << 13)));

        /* set DMA MAC mode in 1G */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, curPortParams.portMacNumber, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 12), (1 << 12)));
    }

    /* in lbPort mode, only for MMPCS configure PCS with External PLL */
    if ((lbPort == GT_TRUE) && (curPortParams.portPcsType == MMPCS))
    {
        CHECK_STATUS(mvHwsPcsExtPllCfg(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType));
    }

    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    if (serdesInit == GT_TRUE)
    {
        /* run SERDES digital reset / unreset */
        switch (portMode)
        {
            case SGMII2_5:
                CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
                break;
            case QSGMII:
                {
                    /* perform Serdes Digital Reset */
                    CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[0]), HWS_DEV_SERDES_TYPE(devNum), GT_FALSE, GT_TRUE, GT_FALSE));

                    /* delay 1ms */
                    hwsOsExactDelayPtr(devNum, portGroup, 10);

                    /* perform Serdes Digital Unreset */
                    CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[0]), HWS_DEV_SERDES_TYPE(devNum), GT_FALSE, GT_FALSE, GT_FALSE));
                }
                break;

            default:
                break;
        }
    }

    return GT_OK;
}

#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */

/*******************************************************************************
* mvHwsGeneralApPortInit
*
* DESCRIPTION:
*       Init port for 802.3AP protocol.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       phyPortNum  - physical port number
*       portMode    - port standard metric
*       lbPort      - if true, init port without serdes activity
*       refClock    - Reference clock frequency
*       refClockSrc - Reference clock source line
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
extern MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTunePresetParams;
extern MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTuneInitParams[2];
#if defined(MV_HWS_FREE_RTOS)
extern GT_U16 mvHwsAvagoSerdesManualTxOffsetDb[2][MV_HWS_IPC_MAX_PORT_NUM];
#endif
GT_STATUS mvHwsGeneralApPortInit
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
    GT_BOOL mask;
#if defined(MV_HWS_FREE_RTOS)
    GT_BOOL updateTxParam = GT_FALSE;
#endif
#ifndef ASIC_SIMULATION
    GT_U8   sdSpeed = 0;
    GT_U32  initAmpValue;
    GT_U32  initPreCursor, initPostCursor;
    GT_U32  i;
#endif /* ASIC_SIMULATION */
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
            || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) ||
            (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
            hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        }
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));

#ifndef ASIC_SIMULATION
        if((AVAGO == HWS_DEV_SERDES_TYPE(devNum)) || (AVAGO_16NM == HWS_DEV_SERDES_TYPE(devNum)))
        {


#if defined(MV_HWS_FREE_RTOS)
            /* supported speeds are currently 10G and 25G */
            if ((curPortParams.serdesSpeed == _10_3125G) || (curPortParams.serdesSpeed == _25_78125G))
            {
                updateTxParam = GT_TRUE;
                sdSpeed = (curPortParams.serdesSpeed == _10_3125G )? 0 : 1;
            }
#endif

            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                initAmpValue   = (0xA<<12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txAmp;
                initPostCursor = (0xB<<12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph0;
                initPreCursor  = (0x9<<12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph1;
#if defined(MV_HWS_FREE_RTOS)
                if (updateTxParam) {
                    initAmpValue = 0xFF & (hwsAvagoSerdesTxTuneInitParams[sdSpeed].txAmp +
                        (GT_32)POSITIVE_NEGATIVE_VALUE(mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] & 0x1F));
                    initAmpValue |= (0xA<<12);
                    initPostCursor = 0xFF & (hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph0 +
                        (GT_32)POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 5 )& 0x1F));
                    initPostCursor |= (0xB<<12);
                    initPreCursor = 0xFF & (hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph1 +
                        POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 10 )& 0x1F));
                    initPreCursor |= (0x9<<12);
                }
#endif

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, initAmpValue, NULL));

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, initPostCursor, NULL));

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, initPreCursor, NULL));
            }
        }
#endif /* ASIC_SIMULATION */
    }

    /* set P Sync Bypass - must for 1000baseX, otherwise will see bad octets in traffic */
    CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, portGroup, phyPortNum, portMode));

    mask = (portMode == _1000Base_X) ? GT_TRUE : GT_FALSE;
    /* mask/unmask signal detect */
    CHECK_STATUS(mvHwsGeneralPortSignalDetectMask(devNum, portGroup, phyPortNum, portMode, mask));
#if 0
    /* Configure MAC/PCS */
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode));

    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    /* Enable the Tx signal, the signal was disabled during Serdes init */
    CHECK_STATUS(mvHwsPortTxEnable(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
#endif
    return GT_OK;
}

/**
* @internal mvHwsGeneralPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortReset
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }
    if (HWS_DEV_SILICON_INDEX(devNum) == Falcon) {
        hwsOsPrintf("TODO Falcon mvHwsGeneralPortReset: dev %d port %d mode %d action %d\n", devNum, phyPortNum, portMode, action);
        return GT_OK;
    }

    /* mark port as under delete */
    switch (portMode)
    {
        case SGMII2_5:
            /* clear P Sync Bypass */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                            GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));
            break;
        default:
            break;
    }

    CHECK_STATUS(mvHwsPortStopCfg(devNum, portGroup, phyPortNum, portMode, action, &(curLanesList[0]), FULL_RESET, FULL_RESET));

/* REMOVING THIS BACKDORE MASKING. MASKING SHOULD BE DONE IN THE ORDINARY WAY (XPCS LANE INTERRUPT)*/
#if 0
    /* mask signal detect */
    CHECK_STATUS(mvHwsGeneralPortSignalDetectMask(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
#endif

    /* PORT_RESET on each related serdes */
    for (i = 0; (PORT_RESET == action) && (i < curPortParams.numOfActLanes); i++) {
        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                      HWS_DEV_SERDES_TYPE(devNum), GT_TRUE, GT_TRUE, GT_TRUE));
    }

    return GT_OK;
}

