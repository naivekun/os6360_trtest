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
* @file prvCpssDxChTxqFcGopUtils.c
*
* @brief CPSS SIP6 TXQ Flow Control L1 abstraction layer
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPfcc.h>

extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);



#define PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(port) (GT_FALSE) /*TODO add REDUCE port*/

/**
* @internal prvCpssDxChPortSip6MtiMacSet function
* @endinternal
*
* @brief   Write data to MTI 100 and MTI 400 MACs.Same date is duplicated to both MACs
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] regAddrMti100           - adress of MTI 100 MAC
* @param[in] regAddrMti400           -adress of MTI 400 MAC
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static  GT_STATUS prvCpssDxChPortSip6MtiMacSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                  regAddrMti100,
    IN   GT_U32                  regAddrMti400,
    IN   GT_U32                  fieldData,
    IN   GT_U32                  fieldOffset,
    IN   GT_U32                  fieldLength
)
{

    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_STATUS rc;

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);


    if(regAddrMti100 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_64_E].regAddr = regAddrMti100;
        regDataArray[PRV_CPSS_PORT_MTI_64_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_64_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_64_E].fieldLength = fieldLength;
    }

    if(regAddrMti400 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddrMti400;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = fieldLength;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Enable/disable PRIORITY flow control
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcModeEnable           -If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcModeEnableSet
(
  IN  GT_U8                    devNum,
  IN  GT_PHYSICAL_PORT_NUM     portNum,
  IN  GT_BOOL                  pfcModeEnable
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig,
        BOOL2BIT_MAC(pfcModeEnable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE
        );


    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Enable/disable  flow control message forwarding to user application.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcPauseFwdEnable           -Terminate / Forward Pause Frames. If set to 'GT_TRUE', pause frames are forwarded to the user application.
*  If set to 'GT_FALSE' (0- Reset value), pause frames are terminated and discarded within the MAC.
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcPauseFwdEnableSet
 (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  pfcPauseFwdEnable
 )
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig,
        BOOL2BIT_MAC(pfcPauseFwdEnable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE
        );


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcPauseFwdEnableGet function
* @endinternal
*
* @brief   Get enable/disable  flow control message forwarding to user application.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcPauseFwdEnable           -Terminate / Forward Pause Frames. If set to 'GT_TRUE', pause frames are forwarded to the user application.
*  If set to 'GT_FALSE' (0- Reset value), pause frames are terminated and discarded within the MAC.
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseFwdEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  * pfcPauseFwdEnablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcPauseFwdEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Set "pause quanta" that will appear in pause frame sent by port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] pauseQuanta           -Pause quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseQuantaSet
    (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   pauseQuanta
    )
{
    GT_U32      regAddrMti100,regAddrMti400;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl01PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01PauseQuanta;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl23PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23PauseQuanta;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl45PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45PauseQuanta;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl67PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67PauseQuanta;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    /*Assumption same quanta value for MTI100 and MTI400*/

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,regAddrMti100,regAddrMti400,
        pauseQuanta,
        (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
        PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE
        );

    return rc;


}


/**
* @internal prvCpssDxChPortSip6PfcQuantaThreshSet function
* @endinternal
*
* @brief   Set interval  that determines how often to "refresh the Xoff frame",
* Meaning, if an Xoff frame sent, the Threshold is reached, and the Xoff_gen input is still high,
* A new Xoff frame will be transmitted.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] quantaThresh           -Interval  quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcQuantaThreshSet
    (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   quantaThresh
    )
{
    GT_U32      regAddrMti100,regAddrMti400;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc;

    if(quantaThresh>=1<<PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl01QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01QuantaThresh;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl23QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23QuantaThresh;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl45QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45QuantaThresh;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl67QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67QuantaThresh;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    /*Assumption same quanta value for MTI100 and MTI400*/

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,regAddrMti100,regAddrMti400,
        quantaThresh,
        (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
        PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE
        );

    return rc;


}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlEnableSet function
* @endinternal
*
* @brief   FCU RX/TX  Channel Enable/Disable. Also set channel ID on demand.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           -If GT_TRUE flow control is enabled on TX  on port,else disabled.
* @param[in] rxEnable           -If GT_TRUE flow control is enabled on RX  on port,else disabled.
* @param[in] setChannelId           -If GT_TRUE then channel ID is configured ,else not congfigured.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlEnableSet
 (
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 txEnable,
    GT_BOOL                 rxEnable,
    GT_BOOL                 setChannelId
 )
{
    GT_U32 mpfChannel;
    GT_U32 regAddrRx, regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);



    mpfChannel = portMacNum % 8;

    if (PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(portMacNum))
    {
        mpfChannel = 8;
    }


    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];
    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    /*set enable*/

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(txEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }


    rc = prvCpssHwPpSetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(rxEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }

    /*set channel ID*/

    if(setChannelId == GT_TRUE)
    {

        rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_OFFSET,
            PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_SIZE,mpfChannel);

        if(rc!=GT_OK)
        {
            return rc;
        }


        rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_OFFSET,
            PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_SIZE,mpfChannel);

        if(rc!=GT_OK)
        {
            return rc;
        }
    }



    return GT_OK;
}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlSegmentedChannelSet function
* @endinternal
*
* @brief   Mark channel as segmented.Both Rx and Tx
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] setSegmentedChannel           -If GT_TRUE mark channel as segmented(200G and 400G)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlSegmentedChannelSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 setSegmentedChannel
)
{
    GT_U32 mpfChannel;
    GT_U32 regAddrRx, regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* only modes 200 and 400 are multiSeg
    if ((portMode == _200GBase_KR4 ) || (portMode == _200GBase_KR8 ) || (portMode == _400GBase_KR8 ))
    {
    multiSeg = GT_TRUE;
    }
    */

    mpfChannel = portMacNum % 8;

    if (PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(portMacNum))
    {
        mpfChannel = 8;
    }

    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];
    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_SIZE,BOOL2BIT_MAC(setSegmentedChannel));

    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpSetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_SIZE,BOOL2BIT_MAC(setSegmentedChannel));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet function
* @endinternal
*
* @brief   Set Rx flow control resolution 8 bit /16 bit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxType16Bit           -If GT_TRUE then flow control resolution is 16bit ,otherwise 8 bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 rxType16Bit
)
{
    GT_U32 regAddr;
    GT_STATUS rc;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_SIZE,BOOL2BIT_MAC(rxType16Bit));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeGet function
* @endinternal
*
* @brief   Get Rx flow control resolution 8 bit /16 bit
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] rxType16BitPtr           -(pointer to)If GT_TRUE then flow control resolution is 16bit ,otherwise 8 bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTypeGet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 *rxType16BitPtr
)
{
    GT_U32 regAddr,regValue;
    GT_STATUS rc;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_SIZE,&regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    *rxType16BitPtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}



/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet function
* @endinternal
*
* @brief   Enable/disable Rx timer (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxTimerEnable           -If GT_TRUE then rx timer enabled,otherwise disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTimerEnableSet
(
   GT_U8                   devNum,
   GT_U32                  portNum,
   GT_BOOL                 rxTimerEnable
)
{
    GT_U32 portMacNum; /* MAC number */
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_SIZE,BOOL2BIT_MAC(rxTimerEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTimerValueSet function
* @endinternal
*
* @brief  Configure  Rx timer value (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxTimerValue           -Timer value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTimerValueSet
(
   GT_U8                   devNum,
   GT_U32                  portNum,
   GT_U32                  rxTimerValue
)
{
    GT_U32 portMacNum; /* MAC number */
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuRXTimer;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_SIZE,rxTimerValue);

    if(rc!=GT_OK)
    {
        return rc;
    }



    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet function
* @endinternal
*
* @brief  Configure  FCU TX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet
 (
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 txEnable
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;

    if (PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(portMacNum))
    {
        mpfChannel = 8;
    }


    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];


    /*set enable*/

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(txEnable));

    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet function
* @endinternal
*
* @brief  Configure  FCU TX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnablePtr           - (Pointer to)Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 *txEnablePtr
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 data;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;

    if (PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(portMacNum))
    {
        mpfChannel = 8;
    }

    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];

    /*get enable*/

    rc = prvCpssHwPpGetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE,&data);

    if(rc!=GT_OK)
    {
        return rc;
    }

    *txEnablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet function
* @endinternal
*
* @brief  Get configure  of FCU RX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxEnablePtr           - (Pointer to)Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet
 (
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 *rxEnablePtr
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrRx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 data;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;

    if (PRV_TXQ_D2D_IS_REDUCE_PORT_MAC(portMacNum))
    {
        mpfChannel = 8;
    }

    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    /*get enable*/

    rc = prvCpssHwPpGetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_SIZE,&data);

    if(rc!=GT_OK)
    {
        return rc;
    }

    *rxEnablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}



/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet function
* @endinternal
*
* @brief  Configure  port flow control mode (Both TxQ and L1)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6FlowControlModeSet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
        GT_STATUS rc;
        GT_U32                          tileNum,dpNum,localdpPortNum,index;
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
        CPSS_DXCH_PORT_FC_MODE_ENT      currentFcMode;
        PRV_CPSS_PFCC_CFG_ENTRY_STC     entry;

        /*Find the port*/

        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
        }

        if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeSet  failed for portNum  %d due to wrong mapping type ",portNum);
        }

        /*Get current configured mode*/
        rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,dpNum,localdpPortNum,NULL,&currentFcMode);

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for portNum %d",portNum);
        }

        if(currentFcMode == fcMode)
        {
            return GT_OK;
        }

        index= (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localdpPortNum+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));

         /*Configure PFCC -entry type= port ,only TC0 is set to avoid overhead*/

        if(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
            /*Check if the PFCC entry exist already*/
            rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,index,&entry);
            if(rc!=GT_OK)
            {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet failed for index %d",index);
            }

            if(entry.entryType ==PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
            {
                rc = prvCpssFalconTxqPffcTableSyncSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,dpNum,localdpPortNum,0x01,GT_TRUE);
            }

        }
        else
        {
             rc = prvCpssFalconTxqPffcTableSyncSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,dpNum,localdpPortNum,0x01,GT_FALSE);
        }

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPffcTableSyncSet failed for portNum %d",portNum);
        }

        /*Configure QFC - only port type ,mapping is done at the init*/

        rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum,tileNum,dpNum,localdpPortNum,0/*don't care*/,fcMode,GT_TRUE);

       if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortSet  failed for portNum  %d  ",portNum);
        }

        if(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
              /*Configure L1 MTI X COMMAND_CONFIG*/

             rc = prvCpssDxChPortSip6PfcModeEnableSet(devNum,portNum,(fcMode == CPSS_DXCH_PORT_FC_MODE_PFC_E)?GT_TRUE:GT_FALSE);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableSet  failed for portNum  %d  ",portNum);
            }
       }

         /*Configure L1 MSDB FCU*/

        rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet(devNum,portNum,(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)?GT_TRUE:GT_FALSE);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet  failed for portNum  %d  ",portNum);
        }

        return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet function
* @endinternal
*
* @brief  Get Configure  port flow control mode .
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6FlowControlModeGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT            *fcModePtr
)
{
    GT_STATUS rc;
    GT_U32 tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32 dummy;

    /*Find the port*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeSet  failed for portNum  %d due to wrong mapping type ",portNum);
    }

    /*Get configuration from QFC*/

    rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,dpNum,localdpPortNum,&dummy,fcModePtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcModeEnableGet function
* @endinternal
*
* @brief   Get enable/disable PRIORITY flow control
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcModeEnablePtr -(Pointer to)If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcModeEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *pfcModeEnablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);



    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);


    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcModeEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}



/**
* @internal prvCpssDxChPortSip6PfcPauseQuantaGet function
* @endinternal
*
* @brief  Get "pause quanta" that will appear in pause frame sent by port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] pauseQuantaPtr           -(pointer to)Pause quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseQuantaGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   *pauseQuantaPtr
)
{
    GT_U32      regAddrMti100,regAddrMti400;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc = GT_FAIL;
    PRV_CPSS_PORT_TYPE_ENT macType;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl01PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01PauseQuanta;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl23PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23PauseQuanta;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl45PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45PauseQuanta;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl67PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67PauseQuanta;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti100)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti100, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE,pauseQuantaPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti400)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti400, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE, pauseQuantaPtr);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "macType %d is not supported ",macType);
    }

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcQuantaThreshSet function
* @endinternal
*
* @brief   Set interval  that determines how often to "refresh the Xoff frame",
* Meaning, if an Xoff frame sent, the Threshold is reached, and the Xoff_gen input is still high,
* A new Xoff frame will be transmitted.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] quantaThresh           -Interval  quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcQuantaThreshGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   *quantaThreshPtr
)
{
    GT_U32      regAddrMti100,regAddrMti400;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc = GT_FAIL;
    PRV_CPSS_PORT_TYPE_ENT macType;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl01QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01QuantaThresh;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl23QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23QuantaThresh;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl45QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45QuantaThresh;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.cl67QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67QuantaThresh;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti100)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti100, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE,quantaThreshPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti400)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti400, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE, quantaThreshPtr);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "macType %d is not supported ",macType);
    }

    return rc;

}
/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet function
* @endinternal
*
* @brief   Get enable/disable Link Pause Ignore Mode.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseIgnoreEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig,
        BOOL2BIT_MAC(enable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE
        );


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet function
* @endinternal
*
* @brief   Enable/disable Link Pause Compatibility Mode.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enable           -If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssDxChPortSip6MtiMacSet(devNum, portNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig,
        BOOL2BIT_MAC(enable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE
        );


    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableGet function
* @endinternal
*
* @brief   Get enable/disable Link Paus Compatibility Mode.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *enablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcPauseIgnoreEnableGet function
* @endinternal
*
* @brief   Get enable/disable Link Pause Ignore Mode.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseIgnoreEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *pfcPauseIgnoreEnablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcPauseIgnoreEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcRxPauseStatusGet function
* @endinternal
*
* @brief   Get  Pause status .Status bit for software to read the current received pause status. One bit for each of the 16 classes.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note  NOT APPLICABLE DEVICES:  xCat3; Lion; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxPauseStatusPtr           -(pointer to)rx pause status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcRxPauseStatusGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                  *rxPauseStatusPtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_64_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI64_MAC.rxPauseStatus;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    if(rc==GT_OK)
    {
        *rxPauseStatusPtr = value;
    }

    return rc;
}







