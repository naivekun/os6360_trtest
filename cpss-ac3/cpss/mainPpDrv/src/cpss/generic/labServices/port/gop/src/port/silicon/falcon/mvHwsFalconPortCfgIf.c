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
* mvHwsFalconPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 48 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

/**
* @internal mvHwsMsdbFcuEnable function
* @endinternal
*
* @brief  set fcu in D2D unit
*
* @param[in] devNum                   - system device number
* @param[in] portNum               - physical port number
* @param[in] enable                   -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMsdbFcuEnable
(
    GT_U8   devNum,
    GT_U32  portNum,
    GT_BOOL enable
)
{
    GT_U32 msdbUnit, d2dIdx;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

    regMask = 0x7001C;
    if (portNum != 0xFFFFFFFF /*d2dNum != HWS_D2D_ALL*/)
    {
        msdbUnit = (PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(portNum)) % 2;
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MSDB, portNum, &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MSDB_FCU_CONTROL;
        if (enable)
            regData = ((msdbUnit+1) << 16) | ((msdbUnit+1) << 2);
        else
            regData = 0;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            msdbUnit = d2dIdx % 2;
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MSDB, (d2dIdx * 8), &baseAddr, &unitIndex, &unitNum ));
               /*FCU Control*/
            regAddr = baseAddr + MSDB_FCU_CONTROL;
            if (enable)
                regData = ((msdbUnit+1) << 16) | ((msdbUnit+1) << 2);
            else
                regData = 0;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMpfTsuEnable function
* @endinternal
*
* @brief  set TSU in mpf unit
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.Tx_Pipe_Status_Delay.set(2);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.update(status);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.PTP_unit_enable.set(1);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.update(status);
*/

GT_STATUS mvHwsMpfTsuEnable
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 d2dIdx;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 controlRegAddr, regData, i, idx;

    if (enable)
        regData = 1;
    else
        regData = 0;

    if (portNum != 0xFFFFFFFF /*d2dNum != HWS_D2D_ALL*/)
    {

        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_TSU,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_TSU, portNum, &baseAddr, &unitIndex, &unitNum));
        }

        controlRegAddr = baseAddr + TSU_CONTROL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, (regData << 1), 0x2));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
           /* msdbUnit = d2dIdx % 2;*/
           /* ravenIdx = d2dIdx / 2;*/
            idx = d2dIdx * 8;
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_TSU, idx, &baseAddr, &unitIndex, &unitNum ));
            for (i = 0; i < 10; i++) {
                controlRegAddr = baseAddr + i*HWS_MPF_CHANNEL_OFFSET + TSU_CONTROL;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, (regData << 1), 0x2));
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsMpfConfigChannel function
* @endinternal
*
* @brief  configur MSDB unit with the port mode (single/multi)
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMpfConfigChannel
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 mpfChannel;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddrRx, regAddrTx, regData, regMask;
    GT_U32 regAddrFcuTx,regAddrFcuRx;
    GT_BOOL segmentedMode = GT_FALSE;
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfConfigChannel ******\n");
    }
#endif

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MSDB, portNum, &baseAddr, &unitIndex, &unitNum));
    mpfChannel = portNum % 8;

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(portNum))
    {
        mpfChannel = 8;
    }

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode) == GT_FALSE)
    {
        regAddrTx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_CHANNEL_TX_CONTROL;
        regAddrRx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_CHANNEL_RX_CONTROL;
    }
    else
    {
        regAddrTx = baseAddr + (mpfChannel/4) * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_TX_CONTROL;
        regAddrRx = baseAddr + (mpfChannel/4) * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_RX_CONTROL;
        segmentedMode = GT_TRUE;
    }

    if (enable)
    {
        regMask = 0x3F;
        regData = mpfChannel;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrTx, regData, regMask));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrRx, regData, regMask));
    }

    regMask = 0x80000000;
    regData = enable<<31;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrTx, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrRx, regData, regMask));

    /*FCU Channel control*/
    regAddrFcuTx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_FCU_CHANNEL_TX_CONTROL;
    regAddrFcuRx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_FCU_CHANNEL_RX_CONTROL;

    if (enable)
    {
        regMask = 0x3F;
        regData = mpfChannel;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuTx, regData, regMask));
        regMask = 0x3F0000;
        regData = mpfChannel<<16;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuRx, regData, regMask));
     }

    regMask = 0xC0000000;
    regData = enable<<31;
    regData|=segmentedMode<<30;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuTx, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuRx, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfConfigChannel ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMpfSetPchMode function
* @endinternal
*
* @brief  Set mpf pch mode
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] pchMode         - pch mode
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSetPchMode
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 pchMode
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfSetPchMode ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(portNum))
    {
        /*TODO get address*/
        baseAddr = 0x516000;
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MPFS,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MPFS, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }
    regAddr = baseAddr + MPFS_PAU_CONTROL;
    /* tx bits[10,9], rx bits[2,1] */
    regMask = (3 << 1)/*RX PCH Mode*/ | (3 << 9)/*TX PCH Mode*/;

    if (pchMode == GT_FALSE) {
        regData = 0;
    }
    else
    {
        regData = (2 << 1)/*RX PCH Mode*/ | (1  << 9)/*TX PCH Mode*/;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* EOP at link down - EOP is generate when a link failure is detected */
    regAddr = baseAddr + MPFS_SAU_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 10, 1, 1));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfSetPchMode ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMpfSauStatusGet function
* @endinternal
*
* @brief  Set MPFS SAU status/counter get - this counter is incremented on
*         every packet discard when the port is in link down
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] dropCntArrSize  - port mode
* @param[out] dropCntValueArr - (pointer to drop counter)
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_U32                  dropCntArrSize,
    GT_U32                  *dropCntValueArr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask = 0xFFFFFFFF;
    GT_U32 tempPort = portNum;
    GT_U32 i;

    if(dropCntValueArr == NULL)
    {
        return GT_BAD_PTR;
    }

    for(i = 0; i < dropCntArrSize; i++)
    {
        if((i == 8) || (i == 9))
        {
            tempPort = portNum & 0xfffffff8;
        }
        else
        {
            tempPort = portNum + i;
        }

        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MPFS, tempPort, &baseAddr, &unitIndex, &unitNum));

        if(i == 8)
        {
            baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
        }
        else if(i == 9)
        {
            baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
        }
        else
        {
            /*do nothing*/
        }

        regAddr = baseAddr + MPFS_SAU_STATUS;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        dropCntValueArr[i] = regData;
    }

    return GT_OK;
}


/**
* @internal mvHwsMtipExtReset function
* @endinternal
*
* @brief  Reset/unreset MTIP unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtReset
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 reset
)
{
    GT_U32 baseAddr, unitIndex, unitNum, portGroup, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtReset ******\n");
    }
#endif
    /* bit [7:0] v = m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_sd_tx_reset_.get();*/
    /* for RAVEN_UNIT_MTI_EXT we check only port 0 or 8 */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;
    regData = 0;
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regMask = 0;
    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if (reset == GT_FALSE)
        {
            /* set the rx[0-7] and the tx[8-15] bits */
            regData |= ((1<<(localPortIndex+i) )| (1<<(8+localPortIndex+i)));
        }

        regMask |= ((1<<(localPortIndex+i) )| (1<<(8+localPortIndex+i)));
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtReset ******\n");
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsMtipExtSetLaneWidth function
* @endinternal
*
* @brief  Set port lane width (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtSetLaneWidth
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_SERDES_BUS_WIDTH_ENT width
)
{
    GT_U32 baseAddr, unitIndex, unitNum, portGroup, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtSetLaneWidth ******\n");
    }
#endif
    regData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_PMA_CONTROL;
    /*CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFF));*/
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    regMask = 0;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (width) {
            case _80BIT_ON:
                regData |=  (0x100<<(localPortIndex+i));
                break;
            case _40BIT_ON:
                regData |=  (0x0<<(localPortIndex+i));
                break;
            case _20BIT_ON:
                regData |=  (0x1<<(localPortIndex+i));
                break;
            case _10BIT_OFF:
                regData =  0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetLaneWidth hws bad width param"));
                break;
        }
        regMask |=  (0x101<<(localPortIndex+i));
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtSetLaneWidth ******\n");
    }
#endif
    return GT_OK;
}




/**
* @internal mvHwsMtipExtFecTypeSet function
* @endinternal
*
* @brief  Set port fec type
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecTypeSet
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_FEC_MODE        fecType
)
{
    GT_U32 baseAddr, unitIndex, unitNum, portGroup, i;
    GT_U32 fecRegAddr, fecRegData, regMask, localPortIndex;
    GT_U32 channelRegAddr, channelRegData;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtFecTypeSet ******\n");
    }
#endif

    fecRegData = 0;
    channelRegData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    fecRegAddr = baseAddr + MTIP_EXT_GLOBAL_FEC_CONTROL;
    channelRegAddr  = baseAddr + MTIP_EXT_GLOBAL_CHANNEL_CONTROL;
    /*CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFF));*/
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regMask = 0;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (fecType) {
            case RS_FEC_544_514:
                if ((portMode != _50GBase_KR ) && (portMode != _100GBase_KR2) &&
                   (portMode != _200GBase_KR4) && (portMode != _200GBase_KR8) &&
                   (portMode != _400GBase_KR8)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                /* set gc_fec91_ena_in[31,24] and gc_kp_mode_in[23,16]*/
                fecRegData |=  (0x1010000 << (localPortIndex + i));
                break;
            case RS_FEC:
                if ((portMode != _25GBase_KR ) && (portMode != _50GBase_KR2) &&
                   (portMode != _100GBase_KR4)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData |=  (0x0010000<<(localPortIndex+i));
                break;
            case FC_FEC:
                if ((portMode != _5GBaseR ) && (portMode != _10GBase_KR) &&
                    (portMode != _25GBase_KR) &&(portMode != _40GBase_KR) &&
                    (portMode != _50GBase_KR2)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData |=  (0x0000001<<(localPortIndex+i));
                break;
            case FEC_OFF:
            case FEC_NA:
                fecRegData =  0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetLaneWidth hws bad width param"));
                break;
        }
        regMask |= (0x0000001<<(localPortIndex+i)) | (0x1010000 << (localPortIndex + i));
     }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, fecRegAddr, fecRegData, regMask));

    if ((localPortIndex % 2) == 0) {
        /* we need to udpate Rs FEC Enable in global channel control register */
        regMask = 0x4400<<(localPortIndex/2);
        if (portMode == _25GBase_KR)
        {
            channelRegData = (fecType == RS_FEC)? 0x400<<(localPortIndex/2): 0;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, channelRegAddr, channelRegData, regMask));
        }
        /* we need to udpate Rs FEC Enable in global channel control register */
        if (portMode == _50GBase_KR2)
        {
            channelRegData |= ((fecType == RS_FEC) || (fecType == FEC_NA))? 0 : 0x4000<<(localPortIndex/2);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, channelRegAddr, channelRegData, regMask));
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtFecTypeSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtipExtSetChannelMode function
* @endinternal
*
* @brief  for speeds 100G/200G/400G Set port channel mode
*         (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/

GT_STATUS mvHwsMtipExtSetChannelMode
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    GT_BOOL                     enable

)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, localPortIndex, evenPort, quadPort;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtSetChannelMode ******\n");
    }
#endif
    if ((portMode != _40GBase_KR4)  &&
        (portMode != _100GBase_KR2) &&
        (portMode != _100GBase_KR4) &&
        (portMode != _200GBase_KR4) &&
        (portMode != _200GBase_KR8) &&
        (portMode != _400GBase_KR8))
    {
        return GT_OK;
    }
    else
    {
        localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);
        if (localPortIndex%2 != 0)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port param"));
        }
        else if ((portMode != _100GBase_KR2) && (localPortIndex%4 != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port param"));
        }
    }
    regMask = 0xFF;
    regData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CHANNEL_CONTROL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    evenPort = localPortIndex / 2;
    quadPort = localPortIndex / 4;

    switch (portMode){
        case _40GBase_KR:
            if (enable)
            {
                regData |= 1 << quadPort;
            }
            else
            {
                regData &= ~(1 << quadPort);
            }
            break;
        case _100GBase_KR2:
            if (enable)
            {
                regData |= 1 << (2 + evenPort);
            }
            else
            {
                regData &= ~(1 << (2 + evenPort));
            }
            break;
        case _100GBase_KR4:
            if (enable)
            {
                regData |= 1 << (2 + evenPort);
            }
            else
            {
                regData &= ~(1 << (2 + evenPort));
            }
            break;
        case _200GBase_KR4:
            if (enable)
            {
                regData |= 1 << (6 + quadPort);
            }
            else
            {
                regData &= ~(1 << (6 + quadPort));
            }
            break;
        case _200GBase_KR8:
        case _400GBase_KR8:
            if (enable)
            {
                regData |= 3 << 6;
            }
            else
            {
                regData &= ~(3 << 6);
            }
            break;
        default:
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port mode"));
            break;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtSetChannelMode ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMtipExtClockEnable function
* @endinternal
*
* @brief  EXT unit clock enable + EXT MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtClockEnable
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, channel;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtClockEnable ******\n");
    }
#endif

    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regMask = 0x100; /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regMask = 0x200; /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regMask = 1 << channel;
    }
    regData = (enable == GT_TRUE) ? regMask : 0;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacResetRelease function
* @endinternal
*
* @brief  EXT unit MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacResetRelease
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regOffset, channel;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtMacResetRelease ******\n");
    }
#endif

    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regOffset = 24;     /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regOffset = 25;     /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regOffset = 16 + channel;
    }

    regData = (enable == GT_TRUE) ? 1 : 0;

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, regOffset, 1, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtMacResetRelease ******\n");
    }
#endif

    return GT_OK;
}

GT_STATUS hwsD2dMacDump
(
    GT_U8   devNum,
    GT_U32  d2dIdx,
    GT_BOOL isRaven
)
{
    GT_U32 regAddr;                 /* register address */
    GT_U32 regData;                 /* register value */
    GT_U32 regMask1 = 0xFFFFFFFF;   /* register mask */
    GT_U32 regMask2 = 0xFFFF;       /* register mask */
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 channelNum;
    GT_BOOL nonZeroCounterFound = GT_FALSE;

    if(d2dIdx >= hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    if (isRaven == GT_FALSE)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIdx, &baseAddr, &unitIndex, &unitNum ));
        cpssOsPrintf("EAGLE D2D\n");
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIdx, &baseAddr, &unitIndex, &unitNum ));
        cpssOsPrintf("RAVEN D2D\n");
    }

    /* MAC RX section */
    for(channelNum = 0; channelNum < 17; channelNum++)
    {
        regAddr = baseAddr + D2D_MAC_RX_STAT_GOOD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC RX[d2d=%d] Good Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_RX_STAT_GOOD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC RX[d2d=%d] Good Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }

        regAddr = baseAddr + D2D_MAC_RX_STAT_BAD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("ERROR: MAC RX[d2d=%d] Bad Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_RX_STAT_BAD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("ERROR: MAC RX[d2d=%d] Bad Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
    }

    regAddr = baseAddr + 0x4F08;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - ERROR CAUSE[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F0C;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - TX First FIFO Overflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F10;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error TX First Interface Sequencing[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F14;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error TX First Credit Underflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F18;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error RX First FIFO Overflow 0[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F1C;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX First Credit Underflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F20;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX First FIFO Overflow 1[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F24;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX Bad Address[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }


    /* MAC TX section */
    for(channelNum = 0; channelNum < 17; channelNum++)
    {
        regAddr = baseAddr + D2D_MAC_TX_STAT_GOOD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Good Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_TX_STAT_GOOD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Good Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }

        regAddr = baseAddr + D2D_MAC_TX_STAT_BAD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Bad Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_TX_STAT_BAD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Bad Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
    }
    regAddr = baseAddr + 0x4400;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("MAC TX -  MAC Error TX Bad Address[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x4600;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    cpssOsPrintf("MAC TX - MAC TX Error Cause[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

    if(nonZeroCounterFound == GT_FALSE)
    {
        cpssOsPrintf("\nMAC RX/TX[d2d=%d] ALL COUNTERS ARE ZERO\n", d2dIdx);
    }

    cpssOsPrintf("\nD2D PCS\n");
    regAddr = baseAddr + 0x7040;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PCS -  PCS Event Cause[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x7058;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PCS - PCS Receive Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

    cpssOsPrintf("\nD2D PMA\n");
    regAddr = baseAddr + 0x8030;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Sticky Status[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8034;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA 66b Alignment Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8038;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Debug Clock Domain Crossing Sticky Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8060;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Event Cause[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

#if 0
    /* PCS section */
    nonZeroCounterFound = GT_FALSE;
    /* PCS Transmit Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_TX_BLOCK_LOW;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS TX[d2d=%d] Stat TX Low = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Transmit Statistic High part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_TX_BLOCK_HIGH;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS TX[d2d=%d] Stat TX High = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Receive Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_RX_BLOCK_LOW;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS RX[d2d=%d] Stat RX Low = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Receive Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_RX_BLOCK_HIGH;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS RX[d2d=%d] Stat RX High = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }

    if(nonZeroCounterFound == GT_FALSE)
    {
        cpssOsPrintf("PCS RX/TX[d2d=%d] ALL COUNTERS ARE ZERO\n", d2dIdx);
    }
#endif


    return GT_OK;
}



