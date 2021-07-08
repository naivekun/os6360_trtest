/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortPacketBuffer.c
*
* DESCRIPTION:
*       CPSS Packet Buffer management.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPacketBuffer.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBuffer.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBufferInternal.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>

/* Functions internal used in Packet Buffer code */

/**
* @internal hwFalconDriverGenericAddressGet function
* @endinternal
*
* @brief   Converts relative address in Tile to driver absolute address.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum      - device number
* @param[in] tileIndex   - tile index
* @param[in] regAddress  - relative register address
*
* @retval - absolute address
*
*/
static GT_U32 hwFalconDriverGenericAddressGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddress
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;

    return ((devPtr->multiPipe.tileOffset * tileIndex) + regAddress);
}

/**
* @internal hwFalconDriverGenericSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS hwFalconDriverGenericSetRegField
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
)
{
    return prvCpssDrvHwPpPortGroupSetRegField(
        devNum, 0/*portGroupId*/,
        hwFalconDriverGenericAddressGet(devNum, tileIndex, regAddr),
        fieldOffset, fieldLength, fieldData);
}

/**
* @internal hwFalconDriverWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
* @param[in] devNum          - The PP to write to.
* @param[in] regAddr         - The register's address to write to.
* @param[in] regData         - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS hwFalconDriverWriteRegister
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 regData
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32 portGropusPerTile = devPtr->multiPipe.numOfPipesPerTile;
    GT_U32 portGropId = (tileIndex * portGropusPerTile);
    return prvCpssHwPpPortGroupWriteRegister(
        devNum, portGropId, regAddr, regData);
}

/**
* @internal hwFalconDriverReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] regDataPtr              - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS hwFalconDriverReadRegister
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 *regDataPtr
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32 portGropusPerTile = devPtr->multiPipe.numOfPipesPerTile;
    GT_U32 portGropId = (tileIndex * portGropusPerTile);
    return prvCpssHwPpPortGroupReadRegister(
        devNum, portGropId, regAddr, regDataPtr);
}

/**
* @internal hwFalconDriverSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS hwFalconDriverSetRegField
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32 portGropusPerTile = devPtr->multiPipe.numOfPipesPerTile;
    GT_U32 portGropId = (tileIndex * portGropusPerTile);
    return prvCpssHwPpPortGroupSetRegField(
        devNum, portGropId, regAddr, fieldOffset, fieldLength, fieldData);
}

/**
* @internal prvCpssDxChPortPacketBufferInternalChannelSpeedTo25GUnits function
* @endinternal
*
* @brief   Converts channel speed values to speed in 25G units.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] channelSpeed   - channel speed enum value
*
* @param[out] speedUnitsPtr - (pointer to)channel speed in 25G units
*
* @retval GT_OK             - on success
* @retval GT_BAD_PARAM      - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannelSpeedTo25GUnits
(
    IN   PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed,
    OUT  GT_U32                                                *speedUnitsPtr
)
{
    switch (channelSpeed)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E:
            *speedUnitsPtr = 0;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E:
            *speedUnitsPtr = 25;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E:
            *speedUnitsPtr = 50;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E:
            *speedUnitsPtr = 100;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E:
            *speedUnitsPtr = 200;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E:
            *speedUnitsPtr = 400;
            return GT_OK;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed function
* @endinternal
*
* @brief   Converts speed in 25G units to channel speed enum values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speedUnits      - speed in 25G units
*
* @param[out]channelSpeedPtr - (pointer to)of channel speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed
(
    IN   GT_U32                                              speedUnits,
    OUT  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT  *channelSpeedPtr
)
{
    switch (speedUnits)
    {
        case 0:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E;
            return GT_OK;
        case 25:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E;
            return GT_OK;
        case 50:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E;
            return GT_OK;
        case 100:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E;
            return GT_OK;
        case 200:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E;
            return GT_OK;
        case 400:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E;
            return GT_OK;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChPortPacketBufferInternalPortToChannelSpeedConvert function
* @endinternal
*
* @brief   Converts port speed values to channel speed values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] portSpeed        - port speed in enum values
*
* @param[out] channelSpeedPtr - (pointer to)of channel speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Device number parameter currently not used.
*    It definded for resolving problems of port speeds supported not by all devices..
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalPortToChannelSpeedConvert
(
    IN  GT_U8                                                 devNum,
    IN  CPSS_PORT_SPEED_ENT                                   portSpeed,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    *channelSpeedPtr
)
{
    /* avoid compiler warning */
    devNum = devNum;

    switch (portSpeed)
    {
        case CPSS_PORT_SPEED_NA_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E;
            return GT_OK;
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_5000_E:
        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_12000_E:
        case CPSS_PORT_SPEED_12500_E:
        case CPSS_PORT_SPEED_13600_E:
        case CPSS_PORT_SPEED_11800_E:
        case CPSS_PORT_SPEED_15000_E:
        case CPSS_PORT_SPEED_16000_E:
        case CPSS_PORT_SPEED_20000_E:
        case CPSS_PORT_SPEED_22000_E:
        case CPSS_PORT_SPEED_23600_E:
        case CPSS_PORT_SPEED_25000_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E;
            return GT_OK;
        case CPSS_PORT_SPEED_26700_E:
        case CPSS_PORT_SPEED_29090_E:
        case CPSS_PORT_SPEED_40000_E:
        case CPSS_PORT_SPEED_47200_E:
        case CPSS_PORT_SPEED_50000_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E;
            return GT_OK;
        case CPSS_PORT_SPEED_75000_E:
        case CPSS_PORT_SPEED_100G_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E;
            return GT_OK;
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_107G_E:
        case CPSS_PORT_SPEED_140G_E:
        case CPSS_PORT_SPEED_200G_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E;
            return GT_OK;
        case CPSS_PORT_SPEED_400G_E:
            *channelSpeedPtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E;
            return GT_OK;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChPortPacketBufferInternalChannelToPortSpeedConvert function
* @endinternal
*
* @brief   Converts port speed values to channel speed values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum          - device number
* @param[in]  channel Speed   - channel speed in enum values
*
* @param[out] portSpeedPtr    - (pointer to)of port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Device number parameter currently not used.
*    It definded for resolving problems of port speeds supported not by all devices..
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalChannelToPortSpeedConvert
(
    IN  GT_U8                                                 devNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed,
    OUT CPSS_PORT_SPEED_ENT                                   *portSpeedPtr
)
{
    /* avoid compiler warning */
    devNum = devNum;

    switch (channelSpeed)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E:
            *portSpeedPtr = CPSS_PORT_SPEED_NA_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E:
            *portSpeedPtr = CPSS_PORT_SPEED_25000_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E:
            *portSpeedPtr = CPSS_PORT_SPEED_50000_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E:
            *portSpeedPtr = CPSS_PORT_SPEED_100G_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E:
            *portSpeedPtr = CPSS_PORT_SPEED_200G_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E:
            *portSpeedPtr = CPSS_PORT_SPEED_400G_E;
            return GT_OK;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChPortPacketBufferInternalPortToChannelConvert function
* @endinternal
*
* @brief   Converts port number to tripple <tileIndex, gpcIndex, channel>.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] portNum          - port number
*
* @param[out] tileIndexPtr    - (pointer to) tile index
* @param[out] gpcIndexPtr     - (pointer to) GPC index
* @param[out] channelPtr      - (pointer to) Read GPC Channel index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalPortToChannelConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *tileIndexPtr,
    OUT GT_U32                          *gpcIndexPtr,
    OUT GT_U32                          *channelPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_SHADOW_PORT_MAP_STC detailedPortMap;

    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortPhysicalPortMapGet(
        devNum, portNum,/*OUT*/&detailedPortMap);
    if (GT_OK != rc )
    {
        return rc;
    }
    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(
        devNum, detailedPortMap.txDmaNum, tileIndexPtr, gpcIndexPtr, channelPtr);

    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalHwValueGet function
* @endinternal
*
* @brief Get different HW values and HW array indexes.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum          - device number
* @param[in]  hwValType       - type of required value
* @param[in]  param0          - additional parameter depending of hwValType
* @param[in]  param1          - additional parameter depending of hwValType
* @param[in]  param2          - additional parameter depending of hwValType
*
* @retval required value or 0xFFFFFFFF on wrong parameters
*
*/
GT_U32 prvCpssDxChPortPacketBufferInternalHwValueGet
(
    IN  GT_U8                                         devNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_ENT hwValType,
    IN  GT_U32                                        param0,
    IN  GT_U32                                        param1,
    IN  GT_U32                                        param2
)
{
    /* avoid compiler warning */
    devNum = devNum;
    param0  = param0;
    param1  = param1;
    param2  = param2;

    switch (hwValType)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_NUMOF_E:
            return (FALCON_PORTS_PER_DP_CNS + 2);
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_CPU_E:
            return FALCON_PORTS_PER_DP_CNS;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_TXQ_E:
            return (FALCON_PORTS_PER_DP_CNS + 1);
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_VSD_BMP_E:
            /* VSD - virtual serdes concept represent 25G bandwidth                */
            /* as part of real serdes or as other BW: CPU and TXQ                  */
            /* real serdes 0-7 represented by VSD0-15, CPU - VSD16, TXQ - VSD17-18 */
            /* parameters: param0 - channel, param1 - speed                        */
            /* for CPU and TXQ speed ignored.                                      */
            /* All supported cases:                                                */
            /* channel 8 (CPU) VSD bitmap 0x010000                                 */
            /* channel 9 (TXQ) VSD bitmap 0x060000                                 */
            /* channel 0 400G  VSD bitmap 0x00FFFF                                 */
            /* channel 0 200G  VSD bitmap 0x0000FF                                 */
            /* channel 4 200G  VSD bitmap 0x00FF00                                 */
            /* channel 0 100G  VSD bitmap 0x00000F                                 */
            /* channel 2 100G  VSD bitmap 0x0000F0                                 */
            /* channel 4 100G  VSD bitmap 0x000F00                                 */
            /* channel 6 100G  VSD bitmap 0x00F000                                 */
            /* channel 0  50G  VSD bitmap 0x000003                                 */
            /* channel 1  50G  VSD bitmap 0x00000C                                 */
            /* channel 2  50G  VSD bitmap 0x000030                                 */
            /* channel 3  50G  VSD bitmap 0x0000C0                                 */
            /* channel 4  50G  VSD bitmap 0x000300                                 */
            /* channel 5  50G  VSD bitmap 0x000C00                                 */
            /* channel 6  50G  VSD bitmap 0x003000                                 */
            /* channel 7  50G  VSD bitmap 0x00C000                                 */
            /* channel 0  25G  VSD bitmap 0x000001                                 */
            /* channel 1  25G  VSD bitmap 0x000004                                 */
            /* channel 2  25G  VSD bitmap 0x000010                                 */
            /* channel 3  25G  VSD bitmap 0x000040                                 */
            /* channel 4  25G  VSD bitmap 0x000100                                 */
            /* channel 5  25G  VSD bitmap 0x000400                                 */
            /* channel 6  25G  VSD bitmap 0x001000                                 */
            /* channel 7  25G  VSD bitmap 0x004000                                 */
            {
                GT_U32 channel = param0;
                PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed =
                    (PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT)param1;
                if (channel == 8) return 0x00010000; /*CPU*/
                if (channel == 9) return 0x00060000; /*TXQ*/
                if (channel > 9)
                    return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                switch (speed)
                {
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E:
                        if ((channel % 8) != 0)
                            return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                        return (0xFFFF << (channel * 2));
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E:
                        if ((channel % 4) != 0)
                            return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                        return (0xFF << (channel * 2));
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E:
                        if ((channel % 2) != 0)
                            return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                        return (0x0F << (channel * 2));
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E:
                        return (0x03 << (channel * 2));
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E:
                        return (0x01 << (channel * 2));
                    case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E:
                        return 0;
                    default: return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                }
            }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E:
            return 0x1F;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_ALL_NUMOF_E:
            return 20;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E:
            return 3;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E:
            switch (param0) /* arbiter RR index */
            {
                case 0: return 1;
                case 1: return 5;
                case 2: return 5;
                default: return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
            }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_CPU_E:
            return 1;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_CPU_E:
            return 4;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_TXQ_E:
            return 2;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_TXQ_E:
            return 4;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_BMP_E:
        {
            /* all possible connections Arbiter Points to channels */
            /* channel can be connected to more then one AP        */
            /* 0/2 means second connection of channel 0            */
            /* RR0-AP0 0/0(400G) - High Speed Arbiter Point */
            /* RR1-AP0 0/0(200G), 0/0(100G), 0(50G and less)*/
            /* RR1-AP1 0/1(200G), 0/1(100G), 1(50G and less)*/
            /* RR1-AP2 0/2(200G), 2/0(100G), 2(50G and less)*/
            /* RR1-AP3 0/3(200G), 2/1(100G), 3(50G and less)*/
            /* RR1-AP4 CPU channel 8  */
            /* RR2-AP0 4/0(200G), 4/0(100G), 4(50G and less)*/
            /* RR2-AP1 4/1(200G), 4/1(100G), 5(50G and less)*/
            /* RR2-AP2 4/2(200G), 6/0(100G), 6(50G and less)*/
            /* RR2-AP3 4/3(200G), 6/1(100G), 7(50G and less)*/
            /* RR2-AP4 TXQ channel 9  */

            /* RR0-AP0 0/0(400G)supported separately - other HW configuration */

            GT_U32 rrIndex = param0;
            GT_U32 channel = param1;
            PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed =
                (PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT)param2;
            if (rrIndex >= 3)
            {
                return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
            }
            /* CPU case */
            if (channel == 8)
            {
                return ((rrIndex == 1) ? 0x10 : 0);
            }
            /* TXQ case */
            if (channel == 9)
            {
                return ((rrIndex == 2) ? 0x10 : 0);
            }
            switch (speed)
            {
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E:
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E:
                    if (rrIndex != (1 + (channel / 4))) return 0; /* no Arbiter Points in RR */
                    return (0x01 << (channel % 4));
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E:
                    if ((channel % 2) != 0)
                    {
                        return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                    }
                    if (rrIndex != (1 + (channel / 4))) return 0; /* no Arbiter Points in RR */
                    return (0x03 << (channel % 4)); /* channel is even */
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E:
                    if ((channel % 4) != 0)
                    {
                        return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                    }
                    if (rrIndex != (1 + (channel / 4))) return 0; /* no Arbiter Points in RR */
                    return 0x0F;
                default: return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
            }
        }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_AP_DISCONNECTED_E:
            return 0x0F;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_VALUE_E:
            switch (param0) /* arbiter RR index */
            {
                /* Arbiter RR0 (High Speed) contains groups 0, RR1 - group 1, RR2 - group 2 */
                case 0: return (0 | (1 << 2) | (2 << 4));
                case 1: return (1 | (2 << 2) | (0 << 4));
                case 2: return (2 | (0 << 2) | (1 << 4));
                default: return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
            }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_INDEX_E:
        {
            /* Current algorithm maps all Arbiter points for channel */
            /* to one RR - absolute virtual serdes index             */
            /* passed in param2 ignored                              */
            /* Searches used RR via recursive call of this function  */
            GT_U32 channel = param0;
            PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed =
                (PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT)param1;
            GT_U32 rrIndex;
            GT_U32 apBitmap = 0;
            if (speed == PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E)
            {
                return 0;
            }
            for (rrIndex = 1; (1); rrIndex ++)
            {
                apBitmap = prvCpssDxChPortPacketBufferInternalHwValueGet(
                    devNum,
                    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_BMP_E,
                    rrIndex, channel, speed);
                if (apBitmap == PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS)
                {
                    return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS;
                }
                if (apBitmap != 0) return rrIndex;
            }
        }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_MAX_SLOTS_E:
            return 19;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_VSD_BY_SLOT_E:
        {
            static GT_U32 virtSdArr[] =
                {0,8,4,12,2,16/*CPU*/,10,6,14,1,9,17/*TXQ*/,5,13,3,11,7,18/*TXQ*/,15};
            GT_U32 slot = param0;
            return virtSdArr[slot];
        }
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_SHAPER_E:
        {
            PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed =
                (PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT)param0;
            GT_U32 maxBucket, leakRate, reorderEnable, mergeEnable;

            switch (speed)
            {
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E:
                    return 0; /* not active channel */
                default : /* should not occur */
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_25G_E:
                    maxBucket = 400; leakRate = 15; break;
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_50G_E:
                    maxBucket = 400; leakRate = 30; break;
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_100G_E:
                    maxBucket = 400; leakRate = 60; break;
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_200G_E:
                    maxBucket = 400; leakRate = 120; break;
                case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E:
                    maxBucket = 800; leakRate = 240; break;
            }
            reorderEnable = 1;
            mergeEnable   = 1;
            return ((maxBucket << 10) | (leakRate << 2) | (reorderEnable << 1) | mergeEnable);
        }
        default: break;
    }
    return PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_BAD_PARAM_CNS; /* not supported value */
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet function
* @endinternal
*
* @brief Configure GPC packet read channel Fifo Segments.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
* @param[in] fifoSegBmp     - bitmap of Fifo segments.
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    IN  GT_U32                                                fifoSegBmp
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 workBmp = fifoSegBmp;
    GT_U32 firstBmpBit = 0xFFFFFFFF;
    GT_U32 prevBmpBit  = 0xFFFFFFFF;

    /* FIFO segment-next ring */
    for (i = 0; (i < 32); i++)
    {
        if ((workBmp & (1 << i)) == 0) continue;
        if (firstBmpBit == 0xFFFFFFFF)
        {
            firstBmpBit = i;
        }
        if (prevBmpBit != 0xFFFFFFFF)
        {
            /* write to previous segment pointer to the current */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentNext[prevBmpBit];
            regVal = i;
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        prevBmpBit  = i;
        workBmp &= (~ (1 << i)); /* remove treated bit */
        if (workBmp == 0) /* treated last bit */
        {
            /* write to last segment pointer to the first */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentNext[i];
            regVal = firstBmpBit;
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        }
    }
    /* FIFO segment-start */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[channel];
    regVal = firstBmpBit;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet function
* @endinternal
*
* @brief Get GPC packet read channel Fifo Segment bitmap.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  tileIndex      - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in]  gpcIndex       - index of GPC (Group of Port Channels)
*                              (APPLICABLE RANGES: 0..7)
* @param[in]  channel        - index of channel
*
* @param[out] fifoSegBmpPtr  - (pointer to)bitmap of Fifo segments.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    OUT GT_U32                                                *fifoSegBmpPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 notUsedStartFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E, 0,0,0);
    GT_U32 numofFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_ALL_NUMOF_E, 0,0,0);

    /* initial */
    *fifoSegBmpPtr = 0;

    /* FIFO segment-start */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[channel];
    rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    #ifdef GM_USED
        /* the GM not supports the PB related units */
        regVal = notUsedStartFifoSeg;
    #endif

    if (regVal == notUsedStartFifoSeg) return GT_OK; /* empty bitmap */

    while (1)
    {
        if (regVal >= numofFifoSeg) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* wrong value got from register */

        /* reached ring element already stored to bitmap */
        if ((*fifoSegBmpPtr) & (1 << regVal)) return GT_OK;

        /* add ring element to bitmap */
        *fifoSegBmpPtr |= (1 << regVal);

        /* get next ring element */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentNext[regVal];
        rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentsDisable function
* @endinternal
*
* @brief Disable GPC packet read channel Fifo Segments.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentsDisable
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel
)
{
    GT_U32 regAddr;
    GT_U32 notUsedStartFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E, 0,0,0);

    /* FIFO segment-start */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[channel];
    return hwFalconDriverWriteRegister(
        devNum, tileIndex, regAddr, notUsedStartFifoSeg);
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet function
* @endinternal
*
* @brief Configure GPC packet read channels arbiter points for given channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
* @param[in] speed          - channel speed enum value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    speed
)
{
    GT_STATUS rc;
    GT_U32 i, j;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 rrAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E, 0,0,0);
    GT_U32 rrApAmount;
    GT_U32 channelApBmp;

    if (speed == PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_400G_E)
    {
        /* 400G */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointHighSpeedChannel0En;
        regVal = 1;
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        return rc;
    }

    /* init Arbiter Points segments */
    for (i = 1; (i < rrAmount); i++)
    {
        rrApAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E, i,0,0);
        channelApBmp = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_BMP_E,
            i, channel, speed);
        for (j = 0; (j < rrApAmount); j++)
        {
            if ((channelApBmp & (1 << j)) == 0) continue;
            switch (i)
            {
                case 1:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup1[j];
                    break;
                case 2:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup2[j];
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            regVal = channel;
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsDisable function
* @endinternal
*
* @brief Disable GPC packet read channels arbiter points for given channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channel        - index of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsDisable
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channel
)
{
    GT_STATUS rc;
    GT_U32 i, j;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 regValGet;
    GT_U32 notUsedRrAp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_AP_DISCONNECTED_E, 0,0,0);
    GT_U32 rrAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E, 0,0,0);
    GT_U32 rrApAmount;

    /* init Arbiter Points segments */
    if (channel == 0)
    {
        /* 400G */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointHighSpeedChannel0En;
        regVal = 0;
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    for (i = 1; (i < rrAmount); i++)
    {
        rrApAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E, i,0,0);
        for (j = 0; (j < rrApAmount); j++)
        {
            switch (i)
            {
                case 1:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup1[j];
                    break;
                case 2:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup2[j];
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regValGet);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (regValGet != channel) continue;

            regVal = notUsedRrAp;
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet function
* @endinternal
*
* @brief Get configurations of all channels from HW.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum           - device number
* @param[in]  tileIndex        - index of tile
*                                (APPLICABLE RANGES: 0..3)
* @param[in]  gpcIndex         - index of GPC (Group of Port Channels)
*                                (APPLICABLE RANGES: 0..7)
* @param[out] channelsNumOfPtr - (pointer to) number of configured channels
* @param[out] channelsCfgArr   - (pointer to) array of channels configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    OUT GT_U32                                                *channelsNumOfPtr,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[]
)
{
    GT_STATUS rc;
    GT_U32 channelsNum = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_NUMOF_E, 0,0,0);
    GT_U32 channel;
    GT_U32 index;
    GT_U32 fifoSegBmp;
    GT_U32 vsdNum;
    GT_U32 workBmp;
    GT_U32 i;

    index = 0;
    for (channel = 0; (channel < channelsNum); channel++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet(
            devNum, tileIndex, gpcIndex, channel, &fifoSegBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (fifoSegBmp == 0) continue;
        channelsCfgArr[index].channel = channel;
        channelsCfgArr[index].vsdBitmap = fifoSegBmp;
        /* calculate channel speed by VSD bitmap - 25G each VSD */
        workBmp = fifoSegBmp;
        vsdNum = 0;
        for (i = 0; (workBmp != 0); i++)
        {
            if (workBmp & (1 << i))
            {
                workBmp &= (~ (1 << i));
                vsdNum ++;
            }
        }
        rc = prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed(
            (vsdNum * PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_FIFO_SEG_SPEED_CNS),
            &(channelsCfgArr[index].channelSpeed));
        if (rc != GT_OK)
        {
            return rc;
        }
        index++;
    }
    *channelsNumOfPtr = index;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet function
* @endinternal
*
* @brief Set all Arbiter Calendar by given configuration of all ports.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]   devNum         - device number
* @param[in]   tileIndex      - index of tile
*                               (APPLICABLE RANGES: 0..3)
* @param[in]   gpcIndex       - index of GPC (Group of Port Channels)
*                               (APPLICABLE RANGES: 0..7)
* @param[in]   channelsNum    - number of configured channels
* @param[in]   channelsCfgArr - (pointer to) array of channels configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelsNum,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[]
)
{
    GT_STATUS rc;
    GT_U32 slotsMaxNum = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_MAX_SLOTS_E, 0,0,0);
    GT_U32 slot;
    GT_U32 slotPatternIdex;
    GT_U32 index;
    GT_U32 channel;
    GT_U32 vsdIndex;
    GT_U32 rrIndex;
    GT_U32 rrCalendarValue;
    GT_U32 regAddr;
    GT_U32 regVal;

    slot = 0;
    for (slotPatternIdex = 0; (slotPatternIdex < slotsMaxNum); slotPatternIdex++)
    {
        vsdIndex = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_VSD_BY_SLOT_E,
            slotPatternIdex,0,0);
        channel = 0xFFFFFFFF; /* init as not found */
        for (index = 0; (index < channelsNum); index++)
        {
            if (channelsCfgArr[index].vsdBitmap & (1 << vsdIndex))
            {
                channel = channelsCfgArr[index].channel;
                break;
            }
        }
        if (channel == 0xFFFFFFFF) continue;
        rrIndex  = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_INDEX_E,
            channel, channelsCfgArr[index].channelSpeed, vsdIndex);
        rrCalendarValue  = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_RR_VALUE_E,
            rrIndex, 0, 0);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterCalendarSlot[slot];
        regVal = rrCalendarValue;
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        slot++;
    }
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterCalendarMax;
    regVal = slot; /* amount of written slots */
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalTxCpuTxqReset function
* @endinternal
*
* @brief Reset TX_DMA channels for CPU and TXQ.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalTxDmaCpuTxqReset
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 cpuChannel = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_CPU_E, 0,0,0);
    GT_U32 txDmaInPipeLocalIndex;
    GT_U32 txDmaUnitAbsBase;
    GT_U32 txDmaUnit0Offset;
    GT_U32 txDmaUnitOffset;
    GT_BOOL txDmaUnitBaseError;
    GT_BOOL txDmaAddrMask;
    GT_BOOL txDmaChannelResetStep;

    txDmaAddrMask = 0x0000FFFF;
    txDmaUnitBaseError = GT_FALSE;
    txDmaUnitAbsBase = prvCpssDxChHwUnitBaseAddrGet(
        devNum, PRV_CPSS_DXCH_UNIT_TXDMA_E, &txDmaUnitBaseError);
    if (txDmaUnitBaseError != GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    txDmaInPipeLocalIndex = gpcIndex;
    /* offset of 0-th instance - if tile not mirrored will be 0 */
    txDmaUnit0Offset = prvCpssSip6OffsetFromFirstInstanceGet(
        devNum, 0, PRV_CPSS_DXCH_UNIT_TXDMA_E);
    if (txDmaUnit0Offset != 0)
    {
        /* tile is mirrored - fix calculation back */
        txDmaInPipeLocalIndex = MAX_DP_PER_TILE_CNS - txDmaInPipeLocalIndex;
    }
    txDmaUnitOffset = prvCpssSip6OffsetFromFirstInstanceGet(
        devNum, txDmaInPipeLocalIndex, PRV_CPSS_DXCH_UNIT_TXDMA_E);

    /* reset CPU channel Tx */
    txDmaChannelResetStep =
        (PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.
            channelConfigs.channelReset[1] -
         PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.
            channelConfigs.channelReset[0]);
    regAddr = txDmaUnitAbsBase + txDmaUnitOffset +
        (PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.
        channelConfigs.channelReset[0] & txDmaAddrMask) +
        (txDmaChannelResetStep * cpuChannel);
    for (i = 0; (i < 3); i++)
    {
        regVal = (i & 1); /* sequence 0,1,0 */
        rc = hwFalconDriverGenericSetRegField(
            devNum, tileIndex, regAddr, 0, 1, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* reset TXQ channel Tx */
    regAddr = txDmaUnitAbsBase + txDmaUnitOffset +
        (PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).TDSConfigs.channelReset
         & txDmaAddrMask);
    for (i = 0; (i < 3); i++)
    {
        regVal = (i & 1); /* sequence 0,1,0 */
        rc = hwFalconDriverGenericSetRegField(
            devNum, tileIndex, regAddr, 0, 1, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcPacketReadInit function
* @endinternal
*
* @brief Init speed of GPC packet read channels.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcPacketReadInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
)
{
    GT_STATUS rc;
    GT_U32 i, j;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 numOfChannels = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_NUMOF_E, 0,0,0);
    GT_U32 cpuChannel = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_CPU_E, 0,0,0);
    GT_U32 cpuVsdBmp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_VSD_BMP_E, cpuChannel,0,0);
    GT_U32 txqChannel = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_TXQ_E, 0,0,0);
    GT_U32 txqVsdBmp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_VSD_BMP_E, txqChannel,0,0);
    GT_U32 notUsedStartFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E, 0,0,0);
    GT_U32 notUsedRrAp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_AP_DISCONNECTED_E, 0,0,0);
    GT_U32 rrAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E, 0,0,0);
    GT_U32 rrApAmount;
    GT_U32 cfgChannelsNum;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[
        PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_MAX_CHANNELS_CNS];

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E
        > prvCpssDxchFalconCiderVersionGet())
    {
        /* reset PacketCount */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].packetCountReset;
        regVal = 1;
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* init FIFO segments for ports + CPU(1 segment) and TXQ(2 segments) */
    /* Each channel gets FIFO segments with the same indexes that        */
    /* virtual serdes used by the channel.                               */
    /* FIFO Start values all as unused */
    for (i = 0; (i < numOfChannels); i++)
    {
        regVal = notUsedStartFifoSeg;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[i];
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* CPU FIFO Segments - start pointer and next-ring */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet(
        devNum, tileIndex, gpcIndex, cpuChannel, cpuVsdBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* TXQ FIFO Segments - start pointer and next-ring */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet(
        devNum, tileIndex, gpcIndex, txqChannel, txqVsdBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* retrieve configurations from FIFO segments */
    /* CPU and TXQ only should be found           */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet(
        devNum, tileIndex, gpcIndex, &cfgChannelsNum, channelsCfgArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* init Arbiter Points segments */
    /* disable all Arbiter points in all RRs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointHighSpeedChannel0En;
    regVal = 0;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    for (i = 1; (i < rrAmount); i++)
    {
        rrApAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E, i,0,0);
        for (j = 0; (j < rrApAmount); j++)
        {
            switch (i)
            {
                case 1:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup1[j];
                    break;
                case 2:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup2[j];
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            regVal = notUsedRrAp;
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /* Configure Arbiter points of CPU and TXQ */
    for (i = 0; (i < cfgChannelsNum); i++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet(
            devNum, tileIndex, gpcIndex, channelsCfgArr[i].channel,
            channelsCfgArr[i].channelSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* init Arbiter Calendar */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet(
        devNum, tileIndex, gpcIndex, cfgChannelsNum, channelsCfgArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* trigger arbiter update configuration */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].cfgUpdateTrigger;
    regVal = 1;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
        <= prvCpssDxchFalconCiderVersionGet())
    {
        for (i = 0; (i < cfgChannelsNum); i++)
        {
            /* confugure shaper register */
            regVal = prvCpssDxChPortPacketBufferInternalHwValueGet(
                devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_SHAPER_E,
                channelsCfgArr[i].channelSpeed,0,0);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].channelShaper[channelsCfgArr[i].channel];
            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* reset CPU channel */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].channelReset[cpuChannel];
    regVal = 1;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* reset TXQ channel */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].channelReset[txqChannel];
    regVal = 1;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);

    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet function
* @endinternal
*
* @brief Set speed of GPC read channel. All relevant subunits configured.
*    Assumed that the port that their speed being changed stopped,
*    but other ports connected to the same GPC
*    continue to receive/send packets and should not be affected.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports - configured only at runtime
*                             channel 8 related to CPU port - initialized to 25G speed,
*                             can be changed at runtime,
*                             channel 9 related to TXQ - initialized to 50G speed,
*                             cannot be changed at runtime,
*                             (APPLICABLE RANGES: 0..9)
* @param[in] speed          - speed of the channel enum value
*                             The speeds of channels 0..7 speeds should be also valid for connected MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    speed
)
{
    GT_STATUS                                          rc;
    GT_U32                                             regAddr;
    GT_U32                                             regVal;
    GT_U32 i;
    GT_U32 oldChannelVsdBmp;
    GT_U32 newChannelVsdBmp;
    GT_U32 cfgChannelsNum;
    GT_U32 otherVsdBmp;
    GT_U32 channelFoundIndex;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_CHANNELS_CFG_STC channelsCfgArr[
        PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_MAX_CHANNELS_CNS];

    /* retrieve configurations from FIFO segments */
    /* CPU and TXQ only should be found           */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadCfgGet(
        devNum, tileIndex, gpcIndex, &cfgChannelsNum, channelsCfgArr);
    if (rc != GT_OK)
    {
        return rc;
    }
    newChannelVsdBmp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_VSD_BMP_E,
        channelIndex, speed, 0);
    /* save old values vor configured channel and update array by ne values */
    /* the disabled channel (i.e. speed == 0) supported                     */
    otherVsdBmp      = 0;
    oldChannelVsdBmp = 0;
    channelFoundIndex = cfgChannelsNum;
    for (i = 0; (i < cfgChannelsNum); i++)
    {
        if (channelsCfgArr[i].channel == channelIndex)
        {
            channelFoundIndex = i;
            oldChannelVsdBmp = channelsCfgArr[i].vsdBitmap;
            channelsCfgArr[i].vsdBitmap    = newChannelVsdBmp;
            channelsCfgArr[i].channelSpeed = speed;
        }
        else
        {
            otherVsdBmp |= channelsCfgArr[i].vsdBitmap;
        }
    }
    if (oldChannelVsdBmp == newChannelVsdBmp)
    {
        /* nothing to change */
        return GT_OK;
    }
    if (otherVsdBmp & newChannelVsdBmp)
    {
        /* overlapping with other ports virtual SERDES */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (oldChannelVsdBmp == 0)
    {
        /* new channel, add to list */
        channelsCfgArr[cfgChannelsNum].channel      = channelIndex;
        channelsCfgArr[cfgChannelsNum].vsdBitmap    = newChannelVsdBmp;
        channelsCfgArr[cfgChannelsNum].channelSpeed = speed;
        cfgChannelsNum ++;
    }
    if (newChannelVsdBmp == 0)
    {
        /* remove channel from the list */
        for (i = channelFoundIndex; ((i + 1) < cfgChannelsNum); i++)
        {
            channelsCfgArr[i] = channelsCfgArr[i + 1];
        }
        cfgChannelsNum --;
    }

    /* Configure Fifo segments */
    if (newChannelVsdBmp != 0)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapSet(
            devNum, tileIndex, gpcIndex, channelIndex, newChannelVsdBmp);
    }
    else
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentsDisable(
            devNum, tileIndex, gpcIndex, channelIndex);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Configure Arbiter Points - disable old and configure the new */
    /* The HW representation is table Arbiter Point ==> channel     */
    /* The new configuration may not override the old one           */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsDisable(
        devNum, tileIndex, gpcIndex, channelIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (speed != PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterPointsSet(
            devNum, tileIndex, gpcIndex, channelIndex, speed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Configure Arbiter Calendar */
    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadArbiterCalendarSet(
        devNum, tileIndex, gpcIndex, cfgChannelsNum, channelsCfgArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* trigger arbiter update configuration */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].cfgUpdateTrigger;
    regVal = 1;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
        <= prvCpssDxchFalconCiderVersionGet())
    {
        /* confugure shaper register */
        regVal = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_SHAPER_E,
            speed,0,0);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].channelShaper[channelIndex];
        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* reset channel after changing configuration */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].channelReset[channelIndex];
    regVal = 1;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);

    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet function
* @endinternal
*
* @brief Set speed of GPC read channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports
*                             channel 8 related to CPU port
*                             channel 9 related to TXQ
*                             (APPLICABLE RANGES: 0..9)
*
* @param[out] speedPtr       - (pointer to)speed of the channel enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex,
    IN  GT_U32                                                channelIndex,
    OUT PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    *speedPtr
)
{
    GT_STATUS rc;
    GT_U32 fifoSegBmp;
    GT_U32 vsdNum;
    GT_U32 workBmp;
    GT_U32 i;

    rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadFifoSegmentBitmapGet(
        devNum, tileIndex, gpcIndex, channelIndex, &fifoSegBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* calculate channel speed by VSD bitmap - 25G each VSD */
    workBmp = fifoSegBmp;
    vsdNum = 0;
    for (i = 0; (workBmp != 0); i++)
    {
        if (workBmp & (1 << i))
        {
            workBmp &= (~ (1 << i));
            vsdNum ++;
        }
    }
    rc = prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed(
        (vsdNum * PRV_CPSS_DXCH_PORT_PACKET_BUFFER_GPC_FIFO_SEG_SPEED_CNS), speedPtr);
    return rc;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalGenericInit function
* @endinternal
*
* @brief Generic init of all tile subunits.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalGenericInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex
)
{
    GT_STATUS     rc;
    GT_U32        regAddr;
    GT_U32        regVal;
    GT_U32        i,j;
    GT_U32        offset;

    /* this code imported manually from dp_cl_pb_init_vseq_c.sv file                   */
    /* and should be syncronized with it.                                              */
    /* All Tile - id dependend values missed here and configured in                    */
    /* prvCpssDxChPortPacketBufferInternalTileModeSet called before this function      */
    /* All GPC Packet Read subunit registers missed here and configured in             */
    /* prvCpssDxChPortPacketBufferInternalGpcPacketReadInit called after this function */


    /* second initial sequence - dp_cl_pb_init_vseq_c.sv::config_pb */

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_02_22_E
        >= prvCpssDxchFalconCiderVersionGet())
    {
        /* smbWriteArbiter - RX should be enabled before NPM_MC and SBM_MC */
        /* for next versions default reg values used                       */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.smbWriteArbiter.cfgEnable;
        /* CFG_EN_EN_RX=0x1 from TF_180313 called CFG_EN_EN and initialyzed later */
        regVal = 1;
        rc = hwFalconDriverSetRegField(
            devNum, tileIndex, regAddr, 0, 1, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* NPM_MC - configure_npm_mc_init */
    for (i = 0; (i < 3); i++)
    {
        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
            >= prvCpssDxchFalconCiderVersionGet())
        {
            /* for next versions default reg values used */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.npmMc[i].cfgEnable;
            /* CFG_ENABLE_MODULE=0x1 */
            regVal = 1;
            rc = hwFalconDriverSetRegField(
                devNum, tileIndex, regAddr, 0, 1, regVal);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.npmMc[i].cfgRefs;
            regVal = 0x05f5e100; /* CFG_REFS_AGING=0x05f5e100 */
            rc = hwFalconDriverWriteRegister(
                devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        /* clear insList[1-3], leave insList[0] as was */
        for (j = 1; (j < 4); j++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.npmMc[i].insList[j];
            regVal = 0; /* INS_LIST_ADDR */
            rc = hwFalconDriverWriteRegister(
                devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
            >= prvCpssDxchFalconCiderVersionGet())
        {
            /* for next versions default reg values used */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.npmMc[i].cfgUnit;
            regVal = 0xFF; /* CFG_UNIT_REINSERT=0xff */
            rc = hwFalconDriverWriteRegister(
                devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.npmMc[i].intReinsertMask;
        /* INT_REINSERT_MASK_0_0=0x1 INT_REINSERT_MASK_0_1=0x1 INT_REINSERT_MASK_0_2=0x1 */
        /* INT_REINSERT_MASK_0_3=0x1 INT_REINSERT_MASK_0_4=0x1 INT_REINSERT_MASK_1_0=0x1 */
        /* INT_REINSERT_MASK_1_1=0x1 INT_REINSERT_MASK_1_2=0x1 INT_REINSERT_MASK_1_3=0x1 */
        /* INT_REINSERT_MASK_1_4=0x1 INT_REINSERT_MASK_2_0=0x1 INT_REINSERT_MASK_2_1=0x1 */
        /* INT_REINSERT_MASK_2_2=0x1 INT_REINSERT_MASK_2_3=0x1 INT_REINSERT_MASK_2_4=0x1 */
        /* INT_REINSERT_MASK_3_0=0x1 INT_REINSERT_MASK_3_1=0x1 INT_REINSERT_MASK_3_2=0x1 */
        /* INT_REINSERT_MASK_3_3=0x1 INT_REINSERT_MASK_3_4=0x1 */
        regVal = 0x000FFFFF;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* gpcPacketWrite - 8 units */
    for (i = 0; (i < MAX_DP_PER_TILE_CNS); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketWrite[i].cfgEnable;
        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
            >= prvCpssDxchFalconCiderVersionGet())
        {
            /* CFG_ENABLE_MODULE=0x1 CFG_ENABLE_SLICE=0xf CFG_ENABLE_LIST=0x0 */
            /* CFG_ENABLE_PORT=0x7 CFG_ENABLE_CHANNEL=0x1fff, bits 27:22 not described */
            /* CFG_ENABLE_LIST - 1 bit */
            regVal = 0x0FFFFFDF;
        }
        else
        {
            /* CFG_ENABLE_MODULE=0x1 CFG_ENABLE_SLICE=0xf CFG_ENABLE_LIST=0x0 */
            /* CFG_ENABLE_PORT=0x7 CFG_ENABLE_CHANNEL=0x1fff  */
            /* CFG_ENABLE_LIST - 2 bits */
            regVal = 0x007FFF9F;
        }

        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* dp_cl_pb_init_vseq_c.sv configures here GPC READ PACKET FIFO Segments         */
    /* and GPC READ PACKET arbiter                                                   */
    /* CPSS will do it later in prvCpssDxChPortPacketBufferInternalGpcPacketReadInit */

    /* pbCenter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.npmRandomizerEnable;
    /* CFG_NPM_RAND_ENABLE_MODULE_TILE_0=0x1 CFG_NPM_RAND_ENABLE_MODULE_TILE_1=0x1 */
    /* CFG_NPM_RAND_ENABLE_MODULE_TILE_2=0x1 CFG_NPM_RAND_ENABLE_MODULE_TILE_3=0x1 */
    regVal = 0xF;
    rc = hwFalconDriverSetRegField(
        devNum, tileIndex, regAddr, 0, 4, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* gpcGrpPacketWrite - 2 units */
    for (i = 0; (i < (MAX_DP_PER_TILE_CNS / 4)); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcGrpPacketWrite[i].npmRandomizerEnable;
        /* CFG_NPM_RAND_ENABLE_MODULE=0x1 */
        regVal = 1;
        rc = hwFalconDriverSetRegField(
            devNum, tileIndex, regAddr, 0, 1, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* SMB_MC */
    for (i = 0; (i < 6); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.smbMc[i].cfgEnable;
        regVal = 1; /* CFG_EN_EN=0x1 */
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* NPM_MC - configure_npm_mc_crd */
    for (i = 0; (i < 3); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.npmMc[i].cfgEnable;
        /* CFG_ENABLE_UNIT=0xfffff */
        /* CFG_ENABLE_REFS=0xff */
        /* CFG_ENABLE_LIST=0xff */
        /* CFG_ENABLE_RAND=0x1 */
        /* fields as in register */
        regVal = (1 << 1) | (0xF << 2) | (0xF << 6) | (0xFFFFF << 10);
        /* contents of united field in register [29:1] */
        regVal = (regVal >> 1);
        rc = hwFalconDriverSetRegField(
            devNum, tileIndex, regAddr, 1, 29, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* smbWriteArbiter */
    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_02_22_E
        >= prvCpssDxchFalconCiderVersionGet())
    {
        /* CFG_EN_EN_TX=0x1 - field removed at FT_180313 */
        offset = 1;
    }
    else
    {
        /* common field CFG_EN_EN from FT_180313 */
        offset = 0;
    }
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.smbWriteArbiter.cfgEnable;
    regVal = 1;
    rc = hwFalconDriverSetRegField(
        devNum, tileIndex, regAddr, offset, 1, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    {
        /* pbCounter */
        /* Enable PB counter module and configuration for one tile
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_EN.write(status,'h1);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_TILE.write(status,32'h1); //one tile
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_SMB_CELLS.write(status,32'hc08);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_SMB_MC_CELLS.write(status,32'h7d);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_NPM_WORDS.write(status,32'hc08);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_NPM_MC_WORDS.write(status,32'h104);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_SMB_SUM_CELLS.write(status,32'h7d);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_RS_NPM_SUM_WORDS.write(status,32'h104);
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.CFG_FILL_THRS.write(status,32'h60);
        from TF_180313
        reg_model.pb_center.pb_counter_reg_model.pb_counter_RegFile.EN_MASTER.write(status,'h1);
        */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.enablePbc;
        /* EN_PBC=0x1 */
        regVal = 1;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /*sip6_packetBuffer.pbCounter.cfgTile configureg in tile mode depended function */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeSmbCells;
        /* CFG_RS_SMB_CELLS=0xC08 */
        regVal = 0xC08;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeSmbMcCells;
        /* CFG_RS_SMB_MC_CELLS=0x7D */
        regVal = 0x7D;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeNpmWords;
        /* CFG_RS_NPM_WORDS=0xC08 */
        regVal = 0xC08;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeNpmMcWords;
        /* CFG_RS_NPM_MC_WORDS=0x104 */
        regVal = 0x104;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeSmbSumCells;
        /* CFG_RS_SMB_SUM_CELLS=0x7D */
        regVal = 0x7D;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.resizeNpmSumWords;
        /* CFG_RS_NPM_SUM_WORDS=0x104 */
        regVal = 0x104;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.pbCounter.cfgFillThreshold;
        /* CFG_FILL_THRS=0x60 */
        regVal = 0x60;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
            <= prvCpssDxchFalconCiderVersionGet())
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.pbCounter.enableMasterMode;
            /* EN_MASTER == 1 */
            regVal = 1;
            rc = hwFalconDriverSetRegField(
                devNum, tileIndex, regAddr, 0, 1, regVal);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

    }

    /* pbCenter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.npmRandomizerEnable;
    /* CFG_NPM_RAND_ENABLE_RAND_TILE_0=0x1 CFG_NPM_RAND_ENABLE_RAND_TILE_1=0x1 */
    /* CFG_NPM_RAND_ENABLE_RAND_TILE_2=0x1 CFG_NPM_RAND_ENABLE_RAND_TILE_3=0x1 */
    regVal = 0xF;
    rc = hwFalconDriverSetRegField(
        devNum, tileIndex, regAddr, 4, 4, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* gpcGrpPacketWrite - 2 units */
    for (i = 0; (i < (MAX_DP_PER_TILE_CNS / 4)); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcGrpPacketWrite[i].npmRandomizerEnable;
        /* CFG_NPM_RAND_ENABLE_MODULE=0x1 CFG_NPM_RAND_ENABLE_RAND=0x1 */
        regVal = 3;
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* gpcPacketWrite - 8 units */
    for (i = 0; (i < MAX_DP_PER_TILE_CNS); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketWrite[i].cfgEnable;
        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
            >= prvCpssDxchFalconCiderVersionGet())
        {
            /* CFG_ENABLE_MODULE=0x1 CFG_ENABLE_SLICE=0xf CFG_ENABLE_LIST=0x1 */
            /* CFG_ENABLE_PORT=0x7 CFG_ENABLE_CHANNEL=0x1fff */
            /* CFG_ENABLE_LIST - 1 bit */
            regVal = 0x003FFFFF;
        }
        else
        {
            /* CFG_ENABLE_MODULE=0x1 CFG_ENABLE_SLICE=0xf CFG_ENABLE_LIST=0x3 */
            /* CFG_ENABLE_PORT=0x7 CFG_ENABLE_CHANNEL=0x1fff */
            /* CFG_ENABLE_LIST - 2 bits */
            regVal = 0x007FFFFF;
        }
        rc = hwFalconDriverWriteRegister(
            devNum, tileIndex, regAddr, regVal);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* sip6_packetBuffer.pbCenter.tileMode; configureg in tile mode depended function */

    /* sip6_packetBuffer.gpcPacketRead[i].packetCountReset written in */
    /* prvCpssDxChPortPacketBufferInternalGpcPacketReadInit            */

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInternalTileModeSet function
* @endinternal
*
* @brief Set tile mode and id. All relevant subunits configured.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] tilemode       - enum value for 1,2 or 4-tile systems
* @param[in] tileId         - tile Id
*                             (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Typically tiles configured according to given mode.
*      Configured tiles by pairs of tileIndex and tileId below:
*       1 tile:  <0, 0>
*       2 tiles: <0, 0>, <1, 1>
*       4 tiles: <0, 0>, <1, 1>, <2, 2>, <3, 3>
*       This function should allow to configure only part of tiles
*       for debugging purposes(using only part of ports).
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalTileModeSet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         tileIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT tilemode,
    IN  GT_U32                                         tileId
)
{
    GT_STATUS     rc;
    GT_U32        regAddr;
    GT_U32        regVal;

    /* pbCenter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.tileMode;
    switch (tilemode)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_1_TILE_E: regVal = 3; break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E: regVal = 1; break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E: regVal = 0; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = hwFalconDriverWriteRegister(
        devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.tileId;
    regVal = tileId;
    rc = hwFalconDriverWriteRegister(
        devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* pbConter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCounter.cfgTile;
    switch (tilemode)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_1_TILE_E: regVal = 1; break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E: regVal = 2; break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E: regVal = 3; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = hwFalconDriverWriteRegister(
        devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*=================================================================================================*/
/* Functions to be used for other CPSS features */

/**
* @internal prvCpssDxChPortPacketBufferTileModeGet function
* @endinternal
*
* @brief Get tile mode and id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
*
* @param[out] tilemodePtr   - (pointer to)enum value for 1,2 or 4-tile systems
* @param[out] tileIdPtr     - (pointer to)tile Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong register content
* @retval GT_BAD_PTR               - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferTileModeGet
(
    IN   GT_U8                                          devNum,
    IN   GT_U32                                         tileIndex,
    OUT  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT *tilemodePtr,
    OUT  GT_U32                                         *tileIdPtr
)
{
    GT_STATUS     rc;
    GT_U32        regAddr;
    GT_U32        regVal;

    /* pbCenter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.tileMode;
    rc = hwFalconDriverReadRegister(
        devNum, tileIndex, regAddr, &regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    switch (regVal)
    {
        case 0: *tilemodePtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E; break;
        case 1: *tilemodePtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E; break;
        case 3: *tilemodePtr = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.pbCenter.tileId;
    rc = hwFalconDriverReadRegister(
        devNum, tileIndex, regAddr, &regVal);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    *tileIdPtr = regVal;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferInit function
* @endinternal
*
* @brief Initialize all subunits of tile not depended of port speeds.
*    Packet Buffer unit yet not ready for traffic.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] tilemode       - enum value for 1,2 or 4-tile systems
* @param[in] tileId         - tile Id
*                             (APPLICABLE RANGES: 0..3)
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note
*    Typically tiles configured according to given mode.
*    Configured tiles by pairs of tileIndex and tileId below:
*    1 tile:  <0, 0>
*    2 tiles: <0, 0>, <1, 1>
*    4 tiles: <0, 0>, <1, 1>, <2, 2>, <3, 3>
*    This function should allow to configure only part of tiles
*    for debugging purposes(using only part of ports).
*/
GT_STATUS prvCpssDxChPortPacketBufferInit
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         tileIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT tilemode,
    IN  GT_U32                                         tileId
)
{
    GT_STATUS rc;
    GT_U32    numOfTiles =
        PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    GT_U32    numDpPerTile =
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp / numOfTiles;
    GT_U32    gpcIndex;

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E
        > prvCpssDxchFalconCiderVersionGet())
    {
        /* from RTLF V2.0 GPC_PACKET_READ subunit only initialized and configured */
        /* and this code should be skipped                                        */
        rc = prvCpssDxChPortPacketBufferInternalTileModeSet(
            devNum, tileIndex, tilemode, tileId);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChPortPacketBufferInternalGenericInit(devNum, tileIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* reset TX_DMD channels matching CPU (8) and TXQ (9) */
    for (gpcIndex = 0; (gpcIndex < numDpPerTile); gpcIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalTxDmaCpuTxqReset(
            devNum, tileIndex, gpcIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* initialization of GPC Packet Read units */
    for (gpcIndex = 0; (gpcIndex < numDpPerTile); gpcIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcPacketReadInit(
            devNum, tileIndex, gpcIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* initialization of RxDMA units */
    for (gpcIndex = 0; (gpcIndex < numDpPerTile); gpcIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalRxDmaPbFillLevelThresholdSet(
            devNum, tileIndex, gpcIndex, tilemode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferAllTilesInit function
* @endinternal
*
* @brief Initialize all subunits of all tiles not depended of port speeds.
*    Packet Buffer unit yet not ready for traffic.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferAllTilesInit
(
    IN  GT_U8     devNum
)
{
    GT_STATUS                                      rc;
    GT_STATUS                                      badRc;
    GT_U32                                         tileIndex;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT tilemode;
    GT_U32                                         numOfTiles =
        PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    switch (numOfTiles)
    {
        case 1:
            tilemode = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_1_TILE_E;
            break;
        case 2:
            tilemode = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E;
            break;
        case 4:
            tilemode = PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    badRc = GT_OK;
    for (tileIndex = 0; (tileIndex < numOfTiles); tileIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInit(
            devNum, tileIndex, tilemode, tileIndex/*tileId*/);
        if (rc != GT_OK)
        {
            /* save RC and continue */
            badRc = rc;
        }
    }
    return badRc;
}

/**
* @internal prvCpssDxChPortPacketBufferGpcChannelSpeedSet function
* @endinternal
*
* @brief Set speed of GPC read channel. All relevant subunits configured.
*    Assumed that the port that their speed being changed stopped,
*    but other ports connected to the same GPC
*    continue to receive/send packets and should not be affected.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
* @param[in] speed          - port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcChannelSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS                                             rc;
    GT_U32                                                tileIndex;
    GT_U32                                                gpcIndex;
    GT_U32                                                channel;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed;

    rc = prvCpssDxChPortPacketBufferInternalPortToChannelSpeedConvert(
        devNum, speed, &channelSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortPacketBufferInternalPortToChannelConvert(
        devNum, portNum, &tileIndex, &gpcIndex, &channel);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet(
        devNum, tileIndex, gpcIndex, channel,channelSpeed);
}

/**
* @internal prvCpssDxChPortPacketBufferGpcChannelSpeedGet function
* @endinternal
*
* @brief Set speed of GPC read channel.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  portNum        - port number
*
* @param[out] speedPtr       - (pointer to)port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NULL_PTR              - on NULL pointer parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcChannelSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_SPEED_ENT             *speedPtr
)
{
    GT_STATUS                                             rc;
    GT_U32                                                tileIndex;
    GT_U32                                                gpcIndex;
    GT_U32                                                channel;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT    channelSpeed;

    rc = prvCpssDxChPortPacketBufferInternalPortToChannelConvert(
        devNum, portNum, &tileIndex, &gpcIndex, &channel);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet(
        devNum, tileIndex, gpcIndex, channel, &channelSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChPortPacketBufferInternalChannelToPortSpeedConvert(
        devNum, channelSpeed, speedPtr);
}

/**
* @internal prvCpssDxChPortPacketBufferGpcSpeedsPrint function
* @endinternal
*
* @brief Print speed configurations of GPC read subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcSpeedsPrint
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 bitmap;
    GT_U32 i,j;
    GT_U32 numOfChannels = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_NUMOF_E, 0,0,0);
    GT_U32 numofFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_ALL_NUMOF_E, 0,0,0);
    GT_U32 rrAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RRS_NUMOF_E, 0,0,0);
    GT_U32 slotsMaxNum = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_CALENDAR_MAX_SLOTS_E, 0,0,0);
    GT_U32 notUsedStartFifoSeg = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_CHANNEL_FIFO_START_NOT_USED_E, 0,0,0);
    GT_U32 notUsedRrAp = prvCpssDxChPortPacketBufferInternalHwValueGet(
        devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_AP_DISCONNECTED_E, 0,0,0);
    GT_U32 rrApAmount;
    GT_U32 usedSlotsNum;

    cpssOsPrintf(
        "Packet Buffer GPC Configuration devNum %d tileIndex %d gpcIndex %d\n",
        devNum, tileIndex, gpcIndex);

    cpssOsPrintf("Chanel FIFO Segments\n");
    for (i = 0; (i < numOfChannels); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[i];
        rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (regVal == notUsedStartFifoSeg) continue;

        cpssOsPrintf("Channel %d segments: %d", i, regVal);
        j = (regVal % numofFifoSeg);
        bitmap = (1 << j);
        while (1)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentNext[j];
            rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            j = (regVal % numofFifoSeg);
            if (bitmap & (1 << j)) break;
            bitmap |= (1 << j);
            cpssOsPrintf(", %d", regVal);
        }
        cpssOsPrintf("\n");
    }


    /* Arbiter points */
    cpssOsPrintf("Arbiter points\n");
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointHighSpeedChannel0En;
    rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (regVal)
    {
        cpssOsPrintf("High Speed Channel0 Enable 0x%X \n", regVal);
    }

    for (j = 1; (j < rrAmount); j++)
    {
        rrApAmount = prvCpssDxChPortPacketBufferInternalHwValueGet(
            devNum, PRV_CPSS_DXCH_PORT_PACKET_BUFFER_HW_VALUE_GPC_ARBITER_POINT_RR_AP_NUMOF_E, j,0,0);
        for (i = 0; (i < rrApAmount); i++)
        {
            switch (j)
            {
                case 1:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup1[i];
                    break;
                case 2:
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterPointGroup2[i];
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (regVal == notUsedRrAp) continue;
            cpssOsPrintf("Arbiter RR%d/%d channel %d \n", j, i, regVal);
        }
    }

    /*Arbiter Calendar*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterCalendarMax;
    rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    usedSlotsNum = regVal;
    cpssOsPrintf("Arbiter Calendar - used [0x%X] slots\n", usedSlotsNum);
    cpssOsPrintf("Arbiter Calendar slots with tripple arbiter points RRs \n");
    if (usedSlotsNum > slotsMaxNum)
    {
        usedSlotsNum = slotsMaxNum;
    }
    for (i = 0; (i < usedSlotsNum); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].arbiterCalendarSlot[i];
        rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf(
            "%d[%d,%d,%d] ", i, (regVal & 3), ((regVal >> 2) & 3), ((regVal >> 4) & 3));
    }
    cpssOsPrintf("\n");

    if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
        <= prvCpssDxchFalconCiderVersionGet())
    {
        cpssOsPrintf("Chanel Shapers\n");
        for (i = 0; (i < numOfChannels); i++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].channelShaper[i];
            rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsPrintf(
                "Channel %d shaper: maxBucket %d leakRate %d reorderEn %d mergeEn %d\n",
                i, ((regVal >> 10) & 0x3FF), ((regVal >> 2) & 0xFF),
                ((regVal >> 1) & 1), (regVal & 1));
        }
    }
        cpssOsPrintf("\n");
    return GT_OK;
}

/* Print refgisters lists */

typedef struct
{
    GT_U32 regAddrOff;
    char *regName;
    char *separator;
    GT_U32 arraySize;
} REG_PRINT_STC;

#define REG_ADDR_ROOT ((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *)0)
#define REG_ADDR_OFF(path, reg) ((char*)&(REG_ADDR_ROOT->path.reg) - (char*)0)
#define REG_ADDR_ENTRY(path, reg, separator) {REG_ADDR_OFF(path, reg), #reg, separator, 0}
#define REG_ADDR_ARRAY_ENTRY(path, reg, separator, size) \
    {REG_ADDR_OFF(path, reg), #reg, separator, size}
#define REG_ADDR_VAL(regAddrTreePtr, regAddrOff) (*(GT_U32*)((char*)regAddrTreePtr + regAddrOff))
#define REG_ADDR_SIZE(path) sizeof(REG_ADDR_ROOT->path)
#define REG_ADDR_DIM(path) (sizeof(REG_ADDR_ROOT->path) / sizeof(REG_ADDR_ROOT->path[0]))

/**
* @internal prvCpssDxChPortPacketBufferGlobalPrint function
* @endinternal
*
* @brief Print global configurations of PB unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] tileIndex       - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in] regPrintArrPtr  - array of description of registers to print
* @param[in] regPrintArrSize - amount of registers to print
* @param[in] regPrintArrAddOffset - additional offset in register address tree for
*                                   repeating subunits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS prvRegListPrint
(
    IN  GT_U8          devNum,
    IN  GT_U32         tileIndex,
    IN  REG_PRINT_STC  *regPrintArrPtr,
    IN  GT_U32         regPrintArrSize,
    IN  GT_U32         regPrintArrAddOffset
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regVal;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 i, j;

    for (i = 0; (i < regPrintArrSize); i++)
    {
        if ((regPrintArrPtr[i].regAddrOff & 3) != 0)
        {
            /* bypass not initialyzed addresses           */
            /* PRV_CPSS_SW_PTR_ENTRY_UNUSED == 0xFFFFFFFB */
            continue;
        }
        if (regPrintArrPtr[i].arraySize == 0)
        {
            /* single register */
            regAddr = REG_ADDR_VAL(
                regsAddrPtr, (regPrintArrPtr[i].regAddrOff + regPrintArrAddOffset));
            rc = hwFalconDriverReadRegister(
                devNum, tileIndex, regAddr, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            cpssOsPrintf("%s: 0x%X %s", regPrintArrPtr[i].regName, regVal, regPrintArrPtr[i].separator);
        }
        else
        {
            /* registers array */
            cpssOsPrintf("%s", regPrintArrPtr[i].regName);
            for (j = 0; (j < regPrintArrPtr[i].arraySize); j++)
            {
                regAddr = REG_ADDR_VAL(
                    regsAddrPtr,
                    (regPrintArrPtr[i].regAddrOff + (j * sizeof(GT_U32)) + regPrintArrAddOffset));
                rc = hwFalconDriverReadRegister(
                    devNum, tileIndex, regAddr, &regVal);
                if (rc != GT_OK)
                {
                    return rc;
                }
                cpssOsPrintf(" %d: 0x%X", j, regVal);
            }
            cpssOsPrintf(" %s", regPrintArrPtr[i].separator);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferGpcPacketReadRegsDump function
* @endinternal
*
* @brief Dump registers of GPC read subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGpcPacketReadRegsDump
(
    IN  GT_U8      devNum,
    IN  GT_U32     tileIndex,
    IN  GT_U32     gpcIndex
)
{
    GT_STATUS rc;

    REG_PRINT_STC arr[] =
    {
         REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketRead[0], packetCountEnable, "")
        ,REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketRead[0], packetCountReset, "\n")
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], channelEnable, "\n", 10)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], channelReset, "\n", 10)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], channelShaper, "\n", 10)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], fifoSegmentStart, "\n", 10)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], fifoSegmentNext[0], "\n", 10)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], fifoSegmentNext[10], "\n", 10)
        ,REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterPointHighSpeedChannel0En, "\n")
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterPointGroup1, "\n", 5)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterPointGroup2, "\n", 5)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterCalendarSlot[0],  "\n", 8)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterCalendarSlot[8],  "\n", 8)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterCalendarSlot[16], "\n", 8)
        ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterCalendarSlot[24], "\n", 8)
        ,REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketRead[0], arbiterCalendarMax, "")
        ,REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketRead[0], cfgUpdateTrigger, "\n")
    };
    GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

    cpssOsPrintf(
        "Dump of GPC Packet Read registers devNum %d tileIndex %d gpcIndex %d\n",
        devNum, tileIndex, gpcIndex);

    rc = prvRegListPrint(
        devNum, tileIndex, arr, arrSize,
        (gpcIndex * REG_ADDR_SIZE(sip6_packetBuffer.gpcPacketRead[0])));
    if (rc != GT_OK)
    {
        cpssOsPrintf("Register access error\n");
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferGlobalDump function
* @endinternal
*
* @brief Dump global configurations of PB unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferGlobalDump
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex
)
{

    GT_STATUS rc;
    GT_U32    i;

    cpssOsPrintf(
        "Packet Buffer Global Configuration devNum %d tileIndex %d\n",
        devNum, tileIndex);

    /* pbCenter */
    cpssOsPrintf("pbCenter\n");
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.pbCenter, npmRandomizerEnable, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCenter, tileId, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCenter, tileMode, "")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize, 0/*regPrintArrAddOffset*/);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    cpssOsPrintf("pbCounter\n");
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, enablePbc, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, enableMasterMode, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, enableInterTileTransact, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, cfgTile, "\n")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeSmbCells, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeSmbMcCells, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeSmbSumCells, "\n")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeNpmWords, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeNpmMcWords, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, resizeNpmSumWords, "\n")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.pbCounter, cfgFillThreshold, "\n")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize, 0/*regPrintArrAddOffset*/);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    cpssOsPrintf("smbWriteArbiter\n");
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.smbWriteArbiter, cfgEnable, "\n")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize, 0/*regPrintArrAddOffset*/);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    /*npmMc*/
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.npmMc[0], cfgEnable, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.npmMc[0], cfgUnit, "")
            ,REG_ADDR_ENTRY(sip6_packetBuffer.npmMc[0], cfgRefs, "\n")
            ,REG_ADDR_ARRAY_ENTRY(sip6_packetBuffer.npmMc[0], insList[0],  "\n", 4)
            ,REG_ADDR_ENTRY(sip6_packetBuffer.npmMc[0], intReinsertMask, "\n")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        for (i = 0; (i < REG_ADDR_DIM(sip6_packetBuffer.npmMc)); i++)
        {
            cpssOsPrintf("npmMc %d\n", i);
            rc = prvRegListPrint(
                devNum, tileIndex, arr, arrSize, (i * REG_ADDR_SIZE(sip6_packetBuffer.npmMc[0])));
            if (rc != GT_OK)
            {
                cpssOsPrintf("Register access error\n");
                return rc;
            }
        }
    }

    /*smbMc*/
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.smbMc[0], cfgEnable, "")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        for (i = 0; (i < REG_ADDR_DIM(sip6_packetBuffer.smbMc)); i++)
        {
            cpssOsPrintf("smbMc %d\n", i);
            rc = prvRegListPrint(
                devNum, tileIndex, arr, arrSize, (i * REG_ADDR_SIZE(sip6_packetBuffer.smbMc[0])));
            if (rc != GT_OK)
            {
                cpssOsPrintf("Register access error\n");
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPacketBufferRegsPerGpcDump function
* @endinternal
*
* @brief Dump per GPC configurations of all per GPC subunits.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChPortPacketBufferRegsPerGpcDump
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileIndex,
    IN  GT_U32      gpcIndex
)
{
    GT_STATUS rc;
    GT_U32 index;
    GT_U32 divider;

    /*gpcGrpPacketWrite*/
    divider = (REG_ADDR_DIM(sip6_packetBuffer.gpcPacketWrite)
        / REG_ADDR_DIM(sip6_packetBuffer.gpcGrpPacketWrite));
    index = (gpcIndex / divider);
    cpssOsPrintf("gpcGrpPacketWrite %d\n", index);
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.gpcGrpPacketWrite[0], npmRandomizerEnable, "")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize,
            (index * REG_ADDR_SIZE(sip6_packetBuffer.gpcGrpPacketWrite[0])));
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    /*gpcPacketWrite*/
    index = gpcIndex;
    cpssOsPrintf("gpcPacketWrite %d\n", index);
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.gpcPacketWrite[0], cfgEnable, "")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize,
            (index * REG_ADDR_SIZE(sip6_packetBuffer.gpcPacketWrite[0])));
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    /*gpcGrpPacketRead*/
    divider = (REG_ADDR_DIM(sip6_packetBuffer.gpcPacketRead)
        / REG_ADDR_DIM(sip6_packetBuffer.gpcGrpPacketRead));
    index = (gpcIndex / divider);
    cpssOsPrintf("gpcGrpPacketRead %d\n", index);
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.gpcGrpPacketRead[0], badAddress, "\n")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize,
            (index * REG_ADDR_SIZE(sip6_packetBuffer.gpcGrpPacketRead[0])));
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    /*gpcPacketRead*/
    index = gpcIndex;
    rc = prvCpssDxChPortPacketBufferGpcPacketReadRegsDump(
        devNum, tileIndex, index);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Register access error\n");
        return rc;
    }

    /*gpcGrpCellRead*/
    divider = (REG_ADDR_DIM(sip6_packetBuffer.gpcCellRead)
        / REG_ADDR_DIM(sip6_packetBuffer.gpcGrpCellRead));
    index = (gpcIndex / divider);
    cpssOsPrintf("gpcGrpCellRead %d\n", index);
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.gpcGrpCellRead[0], badRegister, "\n")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize,
            (index * REG_ADDR_SIZE(sip6_packetBuffer.gpcGrpCellRead[0])));
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }

    /*gpcCellRead*/
    index = gpcIndex;
    cpssOsPrintf("gpcCellRead %d\n", index);
    {
        REG_PRINT_STC arr[] =
        {
             REG_ADDR_ENTRY(sip6_packetBuffer.gpcCellRead[0], initialization, "")
        };
        GT_U32 arrSize = (sizeof(arr) / sizeof(arr[0]));

        rc = prvRegListPrint(
            devNum, tileIndex, arr, arrSize,
            (index * REG_ADDR_SIZE(sip6_packetBuffer.gpcCellRead[0])));
        if (rc != GT_OK)
        {
            cpssOsPrintf("Register access error\n");
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPacketBufferFillCounterGet function
* @endinternal
*
* @brief Get memory fill-counter value.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  tileIndex      - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in]  counterMemory  - (enum value) - SMB cells or NPM entries
* @param[in]  counterScope   - (enum value) - snapshot global or local or overtime-global
* @param[in]  counterValue   - (enum value) - MIN, MAX, SUM and others
*
* @param[out] receivedValuePtr   - (pointer to)counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChPortPacketBufferFillCounterGet
(
    IN   GT_U8                                           devNum,
    IN   GT_U32                                          tileIndex,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT counterMemory,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT  counterScope,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT  counterValue,
    OUT  GT_U32                                          *receivedValuePtr
)
{
#define M_SMB    1
#define M_NPM    2
#define S_LOC    (1 << 2)
#define S_GLO    (2 << 2)
#define S_OT_MIN (3 << 2)
#define S_OT_MAX (4 << 2)
#define V_MAX    (1 << 5)
#define V_MIN    (2 << 5)
#define V_DIFF   (3 << 5)
#define V_SUM    (4 << 5)
#define V_MC_SUM (5 << 5)
#define V_UC_SUM (6 << 5)

    GT_U32 regAddr;
    GT_U32 counterId;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    if (tileIndex > PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(receivedValuePtr);

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    counterId = 0;

    switch (counterMemory)
    {
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_SMB_CELLS_E:
            counterId |= M_SMB;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_NPM_ENTRIES_E:
            counterId |= M_NPM;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (counterScope)
    {
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_GLOBAL_E:
            counterId |= S_GLO;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_LOCAL_E:
            counterId |= S_LOC;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_OVERTIME_MAX_E:
            counterId |= S_OT_MAX;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_OVERTIME_MIN_E:
            counterId |= S_OT_MIN;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (counterValue)
    {
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_MAX_E:
            counterId |= V_MAX;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_MIN_E:
            counterId |= V_MIN;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_ALL_E:
            counterId |= V_SUM;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_MC_E:
            counterId |= V_MC_SUM;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_UC_E:
            counterId |= V_UC_SUM;
            break;
        case CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_DIFF_MAX_MIN_E:
            counterId |= V_DIFF;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(counterId)
    {
        case (M_SMB | S_LOC | V_MAX):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMax;
            break;
        case (M_SMB | S_LOC | V_MIN):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMin;
            break;
        case (M_SMB | S_LOC | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalSum;
            break;
        case (M_SMB | S_LOC | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMcSum;
            break;
        case (M_NPM | S_LOC | V_MAX):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMax;
            break;
        case (M_NPM | S_LOC | V_MIN):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMin;
            break;
        case (M_NPM | S_LOC | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalSum;
            break;
        case (M_NPM | S_LOC | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMcSum;
            break;
        case (M_SMB | S_GLO | V_MAX):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMax;
            break;
        case (M_SMB | S_GLO | V_MIN):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMin;
            break;
        case (M_SMB | S_GLO | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbSum;
            break;
        case (M_SMB | S_GLO | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMcSum;
            break;
        case (M_SMB | S_GLO | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbUcSum;
            break;
        case (M_NPM | S_GLO | V_MAX):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMax;
            break;
        case (M_NPM | S_GLO | V_MIN):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMin;
            break;
        case (M_NPM | S_GLO | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmSum;
            break;
        case (M_NPM | S_GLO | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMcSum;
            break;
        case (M_NPM | S_GLO | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmUcSum;
            break;
        case (M_SMB | S_GLO | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbDiff;
            break;
        case (M_NPM | S_GLO | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmDiff;
            break;
        case (M_SMB | S_OT_MAX | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxSum;
            break;
        case (M_SMB | S_OT_MAX | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxMcSum;
            break;
        case (M_SMB | S_OT_MAX | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxUcSum;
            break;
        case (M_SMB | S_OT_MIN | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinSum;
            break;
        case (M_SMB | S_OT_MIN | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinMcSum;
            break;
        case (M_SMB | S_OT_MIN | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinUcSum;
            break;
        case (M_SMB | S_OT_MAX | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxDiff;
            break;
        case (M_SMB | S_OT_MIN | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinDiff;
            break;
        case (M_NPM | S_OT_MAX | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxSum;
            break;
        case (M_NPM | S_OT_MAX | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxMcSum;
            break;
        case (M_NPM | S_OT_MAX | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxUcSum;
            break;
        case (M_NPM | S_OT_MIN | V_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinSum;
            break;
        case (M_NPM | S_OT_MIN | V_MC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinMcSum;
            break;
        case (M_NPM | S_OT_MIN | V_UC_SUM):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinUcSum;
            break;
        case (M_NPM | S_OT_MAX | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxDiff;
            break;
        case (M_NPM | S_OT_MIN | V_DIFF):
            regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinDiff;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return hwFalconDriverReadRegister(
        devNum, tileIndex, regAddr, receivedValuePtr);

#undef M_SMB
#undef M_NPM
#undef S_LOC
#undef S_GLO
#undef S_OT_MIN
#undef S_OT_MAX
#undef V_MAX
#undef V_MIN
#undef V_DIFF
#undef V_SUM
#undef V_MC_SUM
#undef V_UC_SUM
}

/**
* @internal cpssDxChPortPacketBufferFillCounterGet function
* @endinternal
*
* @brief Get memory fill-counter value.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  tileIndex      - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in]  counterMemory  - (enum value) - SMB cells or NPM entries
* @param[in]  counterScope   - (enum value) - snapshot global or local or overtime-global
* @param[in]  counterValue   - (enum value) - MIN, MAX, SUM and others
*
* @param[out] receivedValuePtr   - (pointer to)counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChPortPacketBufferFillCounterGet
(
    IN   GT_U8                                           devNum,
    IN   GT_U32                                          tileIndex,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT counterMemory,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT  counterScope,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT  counterValue,
    OUT  GT_U32                                          *receivedValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPacketBufferFillCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, tileIndex, counterMemory, counterScope, counterValue, receivedValuePtr));

    rc = internal_cpssDxChPortPacketBufferFillCounterGet(
        devNum, tileIndex, counterMemory, counterScope, counterValue, receivedValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, tileIndex, counterMemory, counterScope, counterValue, receivedValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortPacketBufferInternalRxDmaPbFillLevelThresholdSet function
* @endinternal
*
* @brief Set PB Fill Level Thresholds.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] tilemode       - enum value for 1,2 or 4-tile systems
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of range parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChPortPacketBufferInternalRxDmaPbFillLevelThresholdSet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              tileIndex,
    IN  GT_U32                                              gpcIndex,
    IN  PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_ENT      tilemode
)
{
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 rxDmaGlobalIndex;
    GT_U32 xonLevel;
    GT_U32 xoffLevel;
 
    if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        gpcIndex = 7 - gpcIndex;
    }

    /* Global RxDMA unit index */
    rxDmaGlobalIndex = tileIndex * 8 + gpcIndex;

    /* PB Fill Level Thresholds register */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, rxDmaGlobalIndex).configs.dataAccumulator.PBFillLevelThresholds;
    switch (tilemode)
    {
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_1_TILE_E: 
            xonLevel = 175; 
            xoffLevel = 185; 
            break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_2_TILE_E: 
            xonLevel = 355; 
            xoffLevel = 365; 
            break;
        case PRV_CPSS_DXCH_PORT_PACKET_BUFFER_TILE_MODE_4_TILE_E: 
            xonLevel = 715; 
            xoffLevel = 725; 
            break;
        default: 
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set registers PB fill Thresholds Xon/Xoff fields */
    regVal = xonLevel | xoffLevel << 16;

    /* Write register */
    return prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
}
