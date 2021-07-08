/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortApCtrlIf.c
*
* @brief
*
* @version   42
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
/* #include <cpss/common/labServices/port/gop/port/private/mvHwsPortApInitIfPrv.h> */
#include "../portCtrl/h/mvHwsPortCtrlAp.h"
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>

/**
* @internal mvHwsApPortCtrlStart function
* @endinternal
*
* @brief   Init AP port capability.
*         Runs AP protocol(802.3ap Serdes AutoNeg) and configures the best port
*         mode and all it's elements accordingly.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStart
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApStartIpc(devNum, portGroup, phyPortNum, (GT_U32*)apCfg));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStop function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApStopIpc(devNum, portGroup, phyPortNum));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlSysAck function
* @endinternal
*
* @brief   Acknowledge port resources were allocated at application level
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlSysAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApSysAckIpc(devNum, portGroup, phyPortNum, portMode));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlCfgGet function
* @endinternal
*
* @brief   Returns the AP port configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @param[out] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlCfgGet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApCfgGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apCfg));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatus                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatusGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_STATUS *apStatus
)
{
    GT_STATUS rcode;

    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    apStatus->preApPortNum  = phyPortNum;
    apStatus->postApPortNum = phyPortNum;

    rcode = mvHwsPortApStatusGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apStatus);
    if (rcode == GT_OK)
    {
        switch (apStatus->hcdResult.hcdResult)
        {
        case Port_100GBASE_KR4:
            apStatus->postApPortMode = _100GBase_KR4;
            break;
        case Port_100GBASE_CR4:
            apStatus->postApPortMode = _100GBase_CR4;
            break;
        case Port_40GBase_R:
            apStatus->postApPortMode = _40GBase_KR4;
            break;
        case Port_40GBASE_CR4:
            apStatus->postApPortMode = _40GBase_CR4;
            break;
        case Port_10GBase_R:
            apStatus->postApPortMode = _10GBase_KR;
            break;
        case Port_10GBase_KX4:
            apStatus->postApPortMode = _10GBase_KX4;
            break;
        case Port_1000Base_KX:
            apStatus->postApPortMode = _1000Base_X;
            break;
        case Port_25GBASE_KR:
            apStatus->postApPortMode = _25GBase_KR;
            break;
        case Port_25GBASE_KR_S:
            apStatus->postApPortMode = _25GBase_KR_S;
            break;
        case Port_25GBASE_CR:
            apStatus->postApPortMode = _25GBase_CR;
            break;
        case Port_25GBASE_CR_S:
            apStatus->postApPortMode = _25GBase_CR_S;
            break;
        case Port_25GBASE_KR_C:
            apStatus->postApPortMode = _25GBase_KR_C;
            break;
        case Port_50GBASE_KR2_C:
            apStatus->postApPortMode = _50GBase_KR2_C;
            break;
        case Port_25GBASE_CR_C:
            apStatus->postApPortMode = _25GBase_CR_C;
            break;
        case Port_50GBASE_CR2_C:
            apStatus->postApPortMode = _50GBase_CR2_C;
            break;
        }
    }

    return rcode;
}

/**
* @internal mvHwsApPortCtrlStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_AP_PORT_STATS *apStats
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApStatsGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apStats));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsReset
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApStatsResetIpc(devNum, portGroup, phyPortNum));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropSet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApIntropSetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apIntrop));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlIntropGet function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApIntropGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apIntrop));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlDebugGet function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlDebugGet
(
    GT_U8           devNum,
    GT_U32          portGroup,
    GT_U32          phyPortNum,
    MV_HWS_FW_LOG   *apDebug
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApDebugGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apDebug));

    return GT_OK;
}

/**
* @internal mvHwsFwHwsLogGet function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
* @param[in] fwHwsLogPtr              - FW HWS log parameters pointer
*
* @param[out] fwHwsLogPtr              - FW HWS log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFwHwsLogGet
(
    GT_U8          devNum,
    MV_HWS_FW_LOG  *fwHwsLogPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsLogGetIpc(devNum, (GT_U32*)fwHwsLogPtr));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlAvagoGuiSet function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlAvagoGuiSet
(
    GT_U8 devNum,
    GT_U8 state
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApAvagoGuiSetIpc(devNum, state));

    return GT_OK;
}


/**
* @internal mvHwsApPortCtrlEnablePortCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlSet
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApEnablePortCtrlSetIpc(devNum, srvCpuEnable));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlEnablePortCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlGet
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApEnablePortCtrlGetIpc(devNum, srvCpuEnablePtr));

    return GT_OK;
}

/**
* @internal mvHwsApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              - port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxParametersOffsetSet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApSerdesTxParametersOffsetSetIpc(devNum, portGroup, phyPortNum, serdesNumber, offsets, serdesSpeed));

    return GT_OK;
}


/**
* @internal mvHwsApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxParametersManualSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  *rxCfgPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (apPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }
    if ((rxCfgPtr->serdesSpeed != _10_3125G) && (rxCfgPtr->serdesSpeed != _25_78125G)) {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsPortApSerdesRxParametersManualSetIpc(devNum, portGroup, apPortNum, rxCfgPtr));

    return GT_OK;
}



