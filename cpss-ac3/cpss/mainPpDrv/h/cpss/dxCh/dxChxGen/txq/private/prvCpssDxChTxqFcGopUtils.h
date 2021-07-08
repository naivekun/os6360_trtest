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
* @file prvCpssDxChTxqFcGopUtils.h
*
* @brief CPSS SIP6 TXQ Flow Control L1 abstraction layer
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqFcGopUtils
#define __prvCpssDxChTxqFcGopUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* register COMMAND_CONFIG for both MTI 100 and MTI 400*/


/*
    Link Pause compatible with PFC mode. Pause is only indicated but does not stop TX.
 */


#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET         20

#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE           1




/*Priority Flow Control Mode enable. If set to 1, the Core generates and processes PFC control frames according
   to the Priority Flow Control Interface signals. If set to 0 (Reset Value), the Core operates in legacy Pause Frame
   mode and generates and processes standard Pause Frames.
 */

#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET         19

#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE           1

/*

    Ignore received Pause frame quanta. If set to '1', received pause frames are ignored by the MAC.
    If set to '0' (Reset value), the transmit process is stopped for the amount of time specified in the pause quanta received within a pause frame.

 */


#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET  8

#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE           1


  /*
      Terminate / Forward Pause Frames. If set to '1', pause frames are forwarded to the user application.
       If set to '0' (Reset value), pause frames are terminated and discarded within the MAC.
   */


#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET         7

#define      PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE           1

/*Link Pause or PFC Class X quanta.
The Quanta of a priority is 16bits wide, and defines the required stop time for the specific priority.
In this case we are talking TX, so it expresses the amount of the time we want the peer to not transmit.
The stop time = 512bit time of port speed * Quanta.
*/

#define      PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET         0

#define      PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET         16

#define      PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE         16


  /*Link Pause and PFC Class X threshold. Periodic interval for sending pause frame*/

#define      PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET         0

#define      PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET         16

#define      PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE         16


/*FCU Channel %n TX Control*/

#define      PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET                                31
#define      PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE                                         1



 #define      PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_OFFSET         30
#define      PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_SIZE                  1


 #define      PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_OFFSET                                   0
#define      PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_SIZE                                           6




  /*FCU Channel %n RX Control*/

#define      PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_OFFSET         31
#define      PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_SIZE         1

 #define      PRV_PFC_MSDB_FCU_RX_CHANNEL_SEGMENTED_MODE_FIELD_OFFSET                                   30
#define      PRV_PFC_MSDB_FCU_RX_CHANNEL_SEGMENTED_MODE_FIELD_SIZE                                           1


 #define      PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_OFFSET         16
#define      PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_SIZE         6



    /*FCU Control*/

#define      PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_OFFSET         0
#define      PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_SIZE         1

#define     PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_OFFSET  1
#define     PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_SIZE  1

      /*FCU RX Timer*/

#define      PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_OFFSET         0
#define      PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_SIZE         16

/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Enable/disable PRIORITY flow control
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
);

 /**
 * @internal prvCpssDxChPortSip6PfcPauseFwdEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable  flow control message forwarding to user application.
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     pfcPauseFwdEnable
);

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
);

 /**
 * @internal prvCpssDxChPortSip6PfcPauseQuantaSet function
 * @endinternal
 *
 * @brief   Set "pause quanta" that will appear in pause frame sent by port .
*  The Quanta of a priority is 16bits wide, and defines the required stop time for the specific priority.
 * In this case we are talking TX, so it expresses the amount of the time we want the peer to not transmit.
 * The stop time = 512bit time of port speed * Quanta.
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
 );

 /**
 * @internal prvCpssDxChPortSip6PfcQuantaThreshSet function
 * @endinternal
 *
 * @brief   Set interval  that determines how often to "refresh the Xoff frame",
 * Meaning, if an Xoff frame sent, the Threshold is reached, and the Xoff_gen input is still high,
 * A new Xoff frame will be transmitted.
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
      IN  GT_U8                     devNum,
      IN  GT_PHYSICAL_PORT_NUM      portNum,
      IN  GT_U32                    tc,
      IN  GT_U32                    quantaThresh
  );

 /**
 * @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlEnableSet function
 * @endinternal
 *
 * @brief   FCU RX/TX  Channel Enable/Disable. Also set channel ID on demand.
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
  );

 /**
 * @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlSegmentedChannelSet function
 * @endinternal
 *
 * @brief   Mark channel as segmented.Both Rx and Tx
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
 );

 /**
 * @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet function
 * @endinternal
 *
 * @brief   Set Rx flow control resolution 8 bit /16 bit
 *
 * @note   APPLICABLE DEVICES:      Falcon
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
);


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
 );

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTimerEnableSet function
* @endinternal
*
* @brief   Enable/disable Rx timer (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
    GT_U8                  devNum,
    GT_U32                 portNum,
    GT_BOOL                rxTimerEnable
);

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTimerValueSet function
* @endinternal
*
* @brief  Configure  Rx timer value (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
     GT_U8                  devNum,
     GT_U32                 portNum,
     GT_U32                 rxTimerValue
 );
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
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT  fcMode
);

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
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT         *fcModePtr
);

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
);

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
);

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
);
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
);

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
);

/**
* @internal prvCpssDxChPortSip6PfcPauseIgnoreEnableSet function
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
);

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
);
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
);

/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet function
* @endinternal
*
* @brief   Get enable/disable Link Paus eCompatibility Mode.
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
);

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
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqFcGopUtils */

