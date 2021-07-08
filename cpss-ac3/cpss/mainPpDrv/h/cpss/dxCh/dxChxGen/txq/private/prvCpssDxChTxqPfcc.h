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
* @file prvCpssDxChTxqPfcc.h
*
* @brief CPSS SIP6 TXQ Pfcc low level configurations.
*
* @version   1
********************************************************************************
*/



#ifndef __prvCpssDxChTxqPfcc
#define __prvCpssDxChTxqPfcc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_CPSS_DXCH_SIP_6_PFCC_CFG_TABLE_SIZE       296
#define PRV_CPSS_DXCH_SIP_6_PFCC_CFG__LATEST_CIDER_TABLE_SIZE       314



/**
* @enum PRV_CPSS_PFCC_CFG_ENTRY_TYPE_ENT
 *
 * @brief Type of PFCC table entry
*/
typedef enum{

    /** Configures the source port and its TCs for PFC generation. Note: all the port's active TCs should be configured.*/

    PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT ,

    /** Configures a single TC for PFC generation.*/

    PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC ,

    /** Configures number of "bubbles" for the pfcc SM. bubble meaning - a cycle without PFC aggregation request.
        The purpose of this option is to avoid stress on the QFC 2 D2D interface*/

    PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE,

    /*Global TC HR; global_tc_hr; Configures a single TC for HR counters aggregation
        */
    PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR

}  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_ENT;


/**
* @struct PRV_CPSS_PFCC_CFG_ENTRY_STC
 *
 * @brief Entry at PFCC CFG table
*/
typedef struct{

    /**The type of the entry, instruct the PFCC SM how to parse the entry. */

    PRV_CPSS_PFCC_CFG_ENTRY_TYPE_ENT entryType;

    /** The number of clock cycles in which the SM is IDLE.The configured number must be a positive number.
     Valid only if entry type is  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE*/

    GT_U32 numberOfBubbles;

    /**    The TC on which to send Global TC aggregation request.
    Valid only if entry type is  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC*/

    GT_U32 globalTc;

    /** Dma of port for PFC generation.
    Valid only if entry type is  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT*/
    GT_U32 portDma;

        /**When marked as "trigger" - the aggregation message will be sent with "pfc group trigger"
    asserted which will initiate a transaction from the QFC to the D2D.
    Valid only if entry type is  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT */

    GT_BOOL pfcMessageTrigger;

    /** 8 bit vector that indicates on which TCs the PFC message is generated on .
    Valid only if entry type is  PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT*/


    GT_U32  tcBitVecEn;

} PRV_CPSS_PFCC_CFG_ENTRY_STC;


/**
* @struct PRV_CPSS_PFCC_CFG_ENTRY_STC
 *
 * @brief PFCC unit initialization params
*/
typedef struct{


  /**When enabled, the PFCC SM is initiating the aggregation and sum chain,
    Disabling this bit in the Master PFCC will Disable the PFC */

      GT_BOOL pfccEnable;

  /**Indication if the pfcc is in the master tile or slave tile.
    Only 1 tile can be marked as master, the others must be slave.
    The mater PFCC is the one initiating aggregation of PFC counters between tiles and the
    one publishes the results on the sum chain */

    GT_BOOL isMaster;

    /** The index of last entry that is configured (thus valid) in the PFCC table.
           This configuration determines the length of the PFCC cycle.
            Once the PFCC SM reached the line configured, PFCC cycle is ended and wraps around.*/
    GT_U32 pfccLastEntryIndex;

} PRV_CPSS_PFCC_TILE_INIT_STC;



/**
* @internal prvCpssFalconTxqPfccCfgTableEntrySet function
* @endinternal
*
* @brief   Write  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[in] entryPtr         - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccCfgTableEntrySet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    IN  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
);

/**
* @internal prvCpssFalconTxqPfccCfgTableEntryGet function
* @endinternal
*
* @brief   Read from  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[out] entryPtr         - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqPfccCfgTableEntryGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    OUT  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
);

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Write to  PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[in] initDbPtr         - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqPfccUnitCfgSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  PRV_CPSS_PFCC_TILE_INIT_STC * initDbPtr
);

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Read PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[out] initDbPtr         - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqPfccUnitCfgGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    OUT  PRV_CPSS_PFCC_TILE_INIT_STC * initDbPtr
);
/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  port and traffic class.
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] masterTileNum          -master tile number
* @param[in] tc           - traffic class [0..7](packet attribute) or 0xFF for source port
* @param[in] dmaNumber              - Source port dma number
* @param[out] pfcCounterValuePtr         -counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccPortTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  masterTileNum,
  IN  GT_U8   tc,
  IN  GT_U32  dmaNumber,
  OUT GT_U32  *pfcCounterValuePtr
);

/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  global  traffic class.
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -   PP's device number.
* @param[in] tileNum                         tile number
* @param[in] tc         -                        traffic class [0..7](packet attribute)
* @param[out] pfcCounterValuePtr         -counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccGlobalTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  tileNum,
  IN  GT_U8   tc,
  OUT GT_U32  *pfcCounterValuePtr
);
/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolSet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 * @param[in] trafClass                                        Traffic class [0..7]
 * @param[in]  poolIdPtr                                       Pool id[0..1]
 * @param[in]  update_EGF_QAG_TC_DP_MAPPER                   indication to update the CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E
 *                                                           (during init must be 'GT_FALSE' to reduce the number of HW accessing !)
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              update_EGF_QAG_TC_DP_MAPPER
);
/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 * @param[in] trafClass                                        Traffic class [0..7]
 *@param[out]poolIdPtr                                       (Pointer to)Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               *poolIdPtr
);
/**
 * @internal  prvCpssFalconTxqPfccTcMapVectorGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping vector between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in]  tileNum                                         Tile number
 *@param[out]valuePtr                                       (Pointer to)Mapping vector
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccTcMapVectorGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    OUT GT_U32                               *valuePtr
);
/**
* @internal prvCpssFalconTxqPfccTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] trafClass       Traffic class [0..7]
* @param[in] forceSet        If GT_TRUE set resource mode regardless of currently congiured mode,
                                                        else if resourse mode is PB do not set currenly requested mode
* @param[in] poolId -         Pool ID to configure
* @param[in] configureHeadroom        If GT_TRUE configure headroom subtruction field ,
                                                        else do not configure headroom subtruction field
* @param[in] headroomEnable -  headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_BOOL                              forceSet,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              configureHeadroom,
    IN  GT_BOOL                              headroomEnable
);

/**
* @internal prvCpssFalconTxqPfccTcResourceModeGet
* @endinternal
*
* @brief  Get configured  mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] trafClass       Traffic class [0..7]
* @param[in] poolIdPtr -         (pointer to)Pool ID to configured
* @param[in] headRoomPtr -  (pointer to)headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               *poolIdPtr,
    IN  GT_BOOL                              *headRoomPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPsi */

