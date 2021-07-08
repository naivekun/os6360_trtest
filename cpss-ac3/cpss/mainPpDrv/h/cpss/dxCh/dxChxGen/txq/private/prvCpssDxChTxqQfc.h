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
* @file prvCpssDxChTxqQfc.h
*
* @brief CPSS SIP6 TXQ QFC low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqQfc
#define __prvCpssDxChTxqQfc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>


/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC
 *
 * @brief Global PFC configuration
*/

typedef struct
{
    GT_BOOL hrCountingEnable;
     /** @brief  number of buffers in the packet buffer. used to determent the congestion level:
            Enabling IA TC PFC on/off per TC - 8 bit vector
     */
    GT_U32 ingressAgregatorTcPfcBitmap;

     /** @brief  number of buffers in the packet buffer. used to determent the congestion level:
            Threshold = Guaranteed Threshold + congestion_factor X (Available buffers - PB congestion).
     */
    GT_U32 pbAvailableBuffers ;

     /** @brief  when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
                        0x0 = Disable; Disable; Disable PFC OFF messages for pb congestion
                        0x1 = Enable; Enable; Enable PFC OFF messages for pb congestion
     */
    GT_BOOL  pbCongestionPfcEnable;

     /** @brief  when Enabled, if the QFC receives PDX congestion indication, PFC OFF message will be sent to all port.tc
                        0x0 = Disable; Disable; Disable PFC OFF messages for pdx_congestion
                        0x1 = Enable; Enable; Enable PFC OFF messages for pdx_congestion
     */
    GT_BOOL  pdxCongestionPfcEnable;

     /** @brief  global PFC enable, when disabled, QFC will not issue PFC messages to D2D
                        0x0 = Disable; Disable; Disable PFC message generation
                        0x1 = Enable; Enable; Enable PFC message generation
     */
    GT_BOOL  globalPfcEnable;

} PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC;

/**
* @internal prvCpssFalconTxqQfcPortBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified local port.
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] index -                     if perPort then local port number (APPLICABLE RANGES:0..8) else local queue number (APPLICABLE RANGES:0..399).
* @param[in] perPort -                if equal GT_TRUE then it is per port querry,else per queue
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcBufNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 qfcNum,
    IN  GT_U32 index,
    IN  GT_BOOL  perPort,
    OUT  GT_U32    *numPtr
);

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortSet function
* @endinternal
*
* @brief   Map local DP port to global dma
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  device number
* @param[in] tileNum                -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[in] dmaPort             -  Global DMA number(0..263).
* @param[in] fcMode             - flow control mode
* @param[in] ignoreMapping             -  Ignore mapping configuration ,configure only PFC generation
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  localPort,
    IN  GT_U32  dmaPort,
    IN CPSS_DXCH_PORT_FC_MODE_ENT  fcMode,
    IN GT_BOOL ignoreMapping
);

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortGet function
* @endinternal
*
* @brief   Get local DP port to global dma mapping
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  device number
* @param[in] tileNum                -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[out] dmaPortPtr             -  (Pointer to)Global DMA number(0..263).
* @param[out] pfcEnablePtr             - (Pointer to) Enable PFC generation for the local port.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  localPort,
    OUT  GT_U32  * dmaPortPtr,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT  *  fcModePtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgSet function
* @endinternal
*
* @brief   Set global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] cfgPtr                     -   (pointer to)QFC PFC global configuration

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC * cfgPtr
 );
/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGet function
* @endinternal
*
* @brief   Get global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] cfgPtr                     -   (pointer to)QFC PFC global configuration

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC * cfgPtr
 );

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitSet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimit                     -   packet buffer limit

* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  GT_U32  globalPbLimit
 );
/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitGet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimitPtr                     -   (pointer to)packet buffer limit
*
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     OUT  GT_U32  * globalPbLimitPtr
 );

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdSet function
* @endinternal
*
* @brief   Set global TC threshold .
*    Note - Duplication to all units is done internally in prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdSet
(
  IN  GT_U8                           devNum,
  IN  GT_U32                          tc,
  IN  GT_BOOL                         enable,
  IN  CPSS_DXCH_PFC_THRESHOLD_STC     *thresholdCfgPtr,
  IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdGet function
* @endinternal
*
* @brief   Get global TC threshold .Note - The threshold is read from QFC0(all other DPs should contain the same value)
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr               - (pointer to )Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdGet
(
     IN  GT_U8                           devNum,
     IN  GT_U32                          tc,
     OUT GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable               - Port/TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdSet
(
     IN  GT_U8                       devNum,
     IN  GT_U32                      tileNum,
     IN  GT_U32                      qfcNum,
     IN  GT_U32                      localPort,
     IN  GT_U32                      tc,
     IN  GT_BOOL                     enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] tc                  -   traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr               - (pointer to )Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdGet
(
     IN  GT_U8                          devNum,
     IN  GT_U32                         tileNum,
     IN  GT_U32                         qfcNum,
     IN  GT_U32                         localPort,
     IN  GT_U32                         tc,
     IN  GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);
/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port threshold .
*
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort                  -   local port (APPLICABLE RANGES:0..8).
* @param[out] enablePtr               - (pointer to )Port PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdGet
(
     IN  GT_U8                         devNum,
     IN  GT_U32                        tileNum,
     IN  GT_U32                        qfcNum,
     IN  GT_U32                        localPort,
     OUT GT_BOOL                       *enablePtr,
     OUT  CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet function
* @endinternal
*
* @brief   Set PBB PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
*
*
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  pbCongestionEnable
 );

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet function
* @endinternal
*
* @brief   Set global PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
*
*
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  enable
 );
/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
* @endinternal
*
* @brief   Set amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8   tc,
     IN  GT_U32  availeblePoolBuffers
 );

/**
* @internal internal_cpssDxChPortPfcDbaTcAvailableBuffersGet
* @endinternal
*
* @brief   Get amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tileNum                  -   current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                   -  QFC that being quried number (APPLICABLE RANGES:0..7).

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/

GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8   tc,
     IN  GT_U32  *availeblePoolBuffersPtr
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPsi */

