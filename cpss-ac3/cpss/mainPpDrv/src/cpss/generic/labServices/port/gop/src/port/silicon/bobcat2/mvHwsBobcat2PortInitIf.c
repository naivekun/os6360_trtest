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
* @file mvHwsBobcat2PortInitIf.c
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
#include <silicon/bobcat2/mvHwsBobcat2PortIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal mvHwsBobcat2PortInit function
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
GT_STATUS mvHwsBobcat2PortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 serdesTxIfNum;
    GT_BOOL serdesInit = GT_TRUE;
    GT_U32 i;
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

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));

        /* set the default Tx parameters on port */
        CHECK_STATUS(mvHwsPortTxDefaultsSet(devNum, portGroup, phyPortNum, portMode));

        /* set P Sync Bypass */
        CHECK_STATUS(mvHwsBobcat2PortPsyncBypassCfg(devNum, portGroup, phyPortNum, portMode));

        /* Configures the DP/Core Clock Selector on port */
        CHECK_STATUS(mvHwsClockSelectorConfig(devNum, portGroup, phyPortNum, portMode));

        /* tx interface select */
        switch (curPortParams.portPcsType)
        {
            case MMPCS:
                serdesTxIfNum = 0;
                break;
            case CGPCS:
                serdesTxIfNum = 1;
                break;
            case INTLKN_PCS:
                serdesTxIfNum = 1;
                break;
            default:
                serdesTxIfNum = 0;
                break;
        }

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            if (((curLanesList[i] >> 16) & 0xFFFF) &&
                ((portMode == INTLKN_24Lanes_6_25G) || (portMode == INTLKN_24Lanes_3_125G))) /* loopback port 32 */
            {
                serdesTxIfNum = 3;
            }

            CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                                               (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), serdesTxIfNum));
        }
    }

    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode));

    /* in lbPort mode, only for MMPCS configure PCS with External PLL */
    if ((lbPort == GT_TRUE) && (curPortParams.portPcsType == MMPCS))
    {
        CHECK_STATUS(mvHwsPcsExtPllCfg(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType));
    }

    /* Set MAC & PCS UNRESET */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    if (serdesInit == GT_TRUE)
    {
        /* run SERDES digital reset / unreset */
        switch (portMode)
        {
            case SGMII2_5:
                CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
                break;
            case HGL:
                CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 100));
                break;
            default:
                break;
        }

        /* enable AC termination, if needed */
        switch (portMode)
        {
            case _1000Base_X:
            case _10GBase_SR_LR:
            case _20GBase_SR_LR:
            case _40GBase_SR_LR:
            case _12GBaseR:
            case _24GBase_KR2:
            case _48GBaseR:
            case _12GBase_SR:
            case _48GBase_SR:
                for (i = 0; i < curPortParams.numOfActLanes; i++)
                {
                    CHECK_STATUS(mvHwsSerdesAcTerminationCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                             (curLanesList[i] & 0xFFFF), (HWS_DEV_SERDES_TYPE(devNum)), GT_TRUE));
                }
                break;
            default:
                break;
        }
    }

    /* mark port as created */
    switch (portMode)
    {
        case _10GBase_KR:
        case _40GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _10GBase_SR_LR:
        case _40GBase_SR_LR:
        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR:
        case _12GBase_SR:
        case _48GBase_SR:
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber, FEC_DEC_DMA_WR_DATA, 0x8000, 0));

            /* MMPCS Connect/Disconnect WA */
            if (HWS_DEV_IS_BOBCAT2_A0(devNum))
            {
                CHECK_STATUS(mvHwsMmPcs28nm40GBackWa(devNum, portGroup, phyPortNum, portMode));
            }
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal mvHwsBobcat2ApPortInit function
* @endinternal
*
* @brief   Init port for 802.3AP protocol.
*
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
GT_STATUS mvHwsBobcat2ApPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
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

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortApSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
    }
#if 0
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode));

    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));
#endif
    /* enable AC termination, if needed */
    if ((serdesInit == GT_TRUE) && (portMode == _1000Base_X))
    {
        CHECK_STATUS(mvHwsSerdesAcTerminationCfg(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)),
                                                 (curLanesList[0] & 0xFFFF), (HWS_DEV_SERDES_TYPE(devNum)), GT_TRUE));
    }

    return GT_OK;
}

/**
* @internal mvHwsBobcat2PortReset function
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
GT_STATUS mvHwsBobcat2PortReset
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

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* mark port as under delete */
    switch (portMode)
    {
        case SGMII2_5:
            /* clear P Sync Bypass */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                            GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));
            break;
        case _10GBase_KR:
        case _40GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _10GBase_SR_LR:
        case _40GBase_SR_LR:
        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR:
        case _12GBase_SR:
        case _48GBase_SR:
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber,
                                            FEC_DEC_DMA_WR_DATA, 0x8001, 0));
            break;
        default:
            break;
    }

    CHECK_STATUS(mvHwsPortStopCfg(devNum, portGroup, phyPortNum, portMode, action, &(curLanesList[0]), FULL_RESET, FULL_RESET));

    /* PORT_RESET on each related serdes */
    for (i = 0; (PORT_RESET == action) && (i < curPortParams.numOfActLanes); i++) {
        CHECK_STATUS(mvHwsSerdesReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                GT_TRUE,GT_TRUE,GT_TRUE));
    }

    /* mark port as deleted */
    switch (portMode)
    {
        case _10GBase_KR:
        case _40GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _10GBase_SR_LR:
        case _40GBase_SR_LR:
        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR:
        case _12GBase_SR:
        case _48GBase_SR:
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber,
                                            FEC_DEC_DMA_WR_DATA, 0, 0));
            break;
        default:
            break;
    }

    return GT_OK;
}


