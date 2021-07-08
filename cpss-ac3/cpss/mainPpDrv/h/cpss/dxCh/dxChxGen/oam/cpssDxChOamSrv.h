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
* @file cpssDxChOamSrv.h
*
* @brief CPSS DxCh OAM service CPU API.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChOamSrvh
#define __cpssDxChOamSrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

/* maximum flow id that is supported by the service CPU */
#define CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS     2047
/* number of reserved Rx queues for the service CPU */
#define CPSS_DXCH_OAM_RX_SDMA_QUEUE_NUM_CNS   2
/* number of reserved Tx queues for the service CPU */
#define CPSS_DXCH_OAM_TX_SDMA_QUEUE_NUM_CNS   1
/* maximum local mac address index that is supported by the service CPU */
#define CPSS_DXCH_OAM_SRV_MAX_LOCAL_MAC_ADDR_INDEX_CNS     127

/**
* @struct CPSS_DXCH_OAM_SRV_LB_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Loopback configuration.
*/
typedef struct{

    /** @brief When enabled, the packet’s sequence number field is
     *  compared to the previous packet sequence number field.
     *  GT_FALSE - disable out of sequence checking.
     *  GT_TRUE - enable out of sequence checking.
     */
    GT_BOOL outOfSequenceEnable;

    /** @brief When enabled, CRC32 is computed on the packet's
     *  data TLV and compared to tlvDataCrc32Val.
     *  GT_FALSE - disable invalid data TLV checking.
     *  GT_TRUE - enable invalid data TLV checking.
     */
    GT_BOOL invalidTlvEnable;

    /** The expected TLV data CRC32 value. */
    GT_U32 tlvDataCrc32Val;

} CPSS_DXCH_OAM_SRV_LB_CONFIG_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Loopback statistical counters
*/
typedef struct{

    /** The number of LBR packets that are out of sequence. */
    GT_U32 outOfSequenceCount;

    /** @brief The total number of LBR packets that were checked
     *  for invalid TLV data CRC32.
     */
    GT_U32 totalCheckedDataTlv;

    /** @brief The total number of LBR packets that their data
     *  TLV CRC32 value was wrong.
     */
    GT_U32 totalInvalidDataTlv;

} CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC
 *
 * @brief Structure of OAM Service CPU system paramaters.
*/
typedef struct{

    GT_U32 rxQueueNum[CPSS_DXCH_OAM_RX_SDMA_QUEUE_NUM_CNS];

    GT_U32 txQueueNum[CPSS_DXCH_OAM_TX_SDMA_QUEUE_NUM_CNS];

    /** @brief Number of Down MEPs.
     *  (APPLICABLE RANGES: 0..2047)
     */
    GT_U32 downMepsNum;

    /** @brief Number of Up MEPs.
     *  (APPLICABLE RANGES: 0..2047)
     */
    GT_U32 upMepsNum;

    /** @brief Number of Rx CCM flows.
     *  (APPLICABLE RANGES: 0..2047)
     */
    GT_U32 rxCcmFlowsNum;

} CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_DM_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Delay Measurement configuration.
*/
typedef struct{

    /** @brief GT_TRUE
     *  service CPU after each DM frame.
     *  GT_FALSE - DM statistical counters disabled
     */
    GT_BOOL countersEnable;

} CPSS_DXCH_OAM_SRV_DM_CONFIG_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_DM_BASIC_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Delay Measurement basic counters.
*/
typedef struct{

    /** A 32 */
    GT_U32 twoWayFrameDelay;

    /** A 32 */
    GT_U32 oneWayFrameDelayForward;

    /** A 32 */
    GT_U32 oneWayFrameDelayBackward;

} CPSS_DXCH_OAM_SRV_DM_BASIC_COUNTERS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Delay Measurement counters.
*/
typedef struct{

    /** basic DM counters. */
    CPSS_DXCH_OAM_SRV_DM_BASIC_COUNTERS_STC basicCounters;

} CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_LM_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Loss Measurement configuration.
*/
typedef struct{

    /** @brief GT_TRUE
     *  service CPU after each LM frame.
     *  GT_FALSE - LM statistical counters disabled.
     */
    GT_BOOL countersEnable;

} CPSS_DXCH_OAM_SRV_LM_CONFIG_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_LM_BASIC_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Loss Measurement basec counters.
*/
typedef struct{

    /** tx frame count in the forward direction */
    GT_U32 txFrameCntForward;

    /** rx frame count in the forward direction */
    GT_U32 rxFrameCntForward;

    /** tx frame count in the backward direction */
    GT_U32 txFrameCntBackward;

    /** rx frame count in the backward direction */
    GT_U32 rxFrameCntBackward;

    /** @brief calculation of far end frame loss
     *  |TxFCf[tc] – TxFCf[tp]| – |RxFCf[tc] – RxFCf[tp]|
     */
    GT_U32 farEndFrameLoss;

    /** @brief calculation of near end frame loss
     *  |TxFCb[tc] – TxFCb[tp]| – |RxFCl[tc] – RxFCl[tp]|
     */
    GT_U32 nearEndFrameLoss;

} CPSS_DXCH_OAM_SRV_LM_BASIC_COUNTERS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Loss Measurement counters.
*/
typedef struct{

    /** basic LM statistical counters. */
    CPSS_DXCH_OAM_SRV_LM_BASIC_COUNTERS_STC basicCounters;

} CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC;

/**
* @enum CPSS_DXCH_OAM_SRV_FRAME_TYPE_ENT
 *
 * @brief Enumeration of OAM Service frame type
*/
typedef enum{

    /** the flow is of type Y.1731 protocol */
    CPSS_DXCH_OAM_SRV_FRAME_TYPE_1731_E,

    /** the flow is of type Y.1711 protocol */
    CPSS_DXCH_OAM_SRV_FRAME_TYPE_1711_E

} CPSS_DXCH_OAM_SRV_FRAME_TYPE_ENT;

/**
* @enum CPSS_DXCH_OAM_SRV_MP_TYPE_ENT
 *
 * @brief Enumeration of OAM Service Maintenance-Point type
*/
typedef enum{

    /** the flow is of up-mep typedef */
    CPSS_DXCH_OAM_SRV_MP_TYPE_UP_MEP_E,

    /** the flow is of down-mep type */
    CPSS_DXCH_OAM_SRV_MP_TYPE_DOWN_MEP_E,

    /** the flow is of mip type */
    CPSS_DXCH_OAM_SRV_MP_TYPE_MIP_E,

    /** the flow is nor up-mp, down-mep or mip */
    CPSS_DXCH_OAM_SRV_MP_TYPE_OTHER_E

} CPSS_DXCH_OAM_SRV_MP_TYPE_ENT;

/**
* @struct CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC
 *
 * @brief Transmission Frames Parameters: (for Tx responding)
*/
typedef struct
{
    GT_BOOL                             l2HeaderValid;
    union
    {
        GT_U8                           header[64];
        GT_ETHERADDR     		        macAddr;
    } l2Header;

    GT_BOOL                             dsaParamValid;
    CPSS_DXCH_NET_DSA_PARAMS_STC        dsaParam;

} CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_1731_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU 1731 parameters configuration.
*/
typedef struct{

    CPSS_DXCH_OAM_SRV_MP_TYPE_ENT mpType;

    GT_U32 cfmOffset;

    /** Transmission Frames Parameters for Tx responding. */
    CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC transmitParams;

} CPSS_DXCH_OAM_SRV_1731_CONFIG_STC;

/**
* @struct CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC
 *
 * @brief Structure of OAM rx-flow parameters configuration in service CPU.
*/
typedef struct
{
    CPSS_DXCH_OAM_SRV_FRAME_TYPE_ENT		 frameType;
    union{
        CPSS_DXCH_OAM_SRV_1731_CONFIG_STC    frame1731;

    }frame;

}CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC;

/**
* @internal cpssDxChOamSrvLoopbackConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
);

/**
* @internal cpssDxChOamSrvLoopbackConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
);

/**
* @internal cpssDxChOamSrvLoopbackStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] clearOnRead              - perform clear on read.
*                                      GT_TRUE: The flows Loopback statistical
*                                      counters will be cleared.
*                                      GT_FALSE: The flows Loopback statistical
*                                      counters will not be cleared.
* @param[in] timeout                  - time to wait for the service CPU to response
*                                      0 - no wait.
*                                      1 to 999 - defined time in miliseconds.
*                                      0xFFFFFFFF - wait forever.
*
* @param[out] lbCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    IN  GT_U32                              timeout,
    OUT CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC   *lbCountersPtr  
);

/**
* @internal cpssDxChOamSrvLoopbackStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackStatisticalCountersClear
(
    IN  GT_U8   devNum,
    IN  GT_U32  flowId
);

/**
* @internal cpssDxChOamSrvDmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] dmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
);

/**
* @internal cpssDxChOamSrvDmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] dmConfigPtr              - pointer to struct of DM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
);

/**
* @internal cpssDxChOamSrvDmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the Delay Measurement statistical counters per flow ID from service CPU .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnRead              - GT_TRUE - clear the counter.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] dmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    OUT CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC   *dmCountersPtr  
);

/**
* @internal cpssDxChOamSrvDmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Delay Measurement statistical counters per flow ID in service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmStatisticalCountersClear
(
    IN  GT_U8   			        devNum,
    IN  GT_U32  			        flowId
);

/**
* @internal cpssDxChOamSrvLmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] lmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
);

/**
* @internal cpssDxChOamSrvLmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] lmConfigPtr              - pointer to struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
);

/**
* @internal cpssDxChOamSrvLmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnReadEnable        - GT_TRUE - clear the counter on read.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] lmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnReadEnable,
    OUT CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC   *lmCountersPtr  
);

/**
* @internal cpssDxChOamSrvLmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmStatisticalCountersClear
(
    IN  GT_U8   			        devNum,
    IN  GT_U32  			        flowId
);

/**
* @internal cpssDxChOamSrvLocalMacAddressSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry.
*                                      This index will be used in cpssDxChOamSrvLocalInterfaceMacIndexSet.
*                                      (APPLICABLE RANGES:0..127)
* @param[in] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalMacAddressSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,    
    IN  GT_ETHERADDR     		       *macAddrPtr
);

/**
* @internal cpssDxChOamSrvLocalMacAddressGet function
* @endinternal
*
* @brief   Get the MAC address table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry
*                                      (APPLICABLE RANGES:0..127)
*
* @param[out] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalMacAddressGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,    
    OUT GT_ETHERADDR     		        *macAddrPtr
);

/**
* @internal cpssDxChOamSrvLocalInterfaceMacIndexSet function
* @endinternal
*
* @brief   Globally configure the MAC address index of a local interface.
*         If OAM packet that come to service CPU is multicast packet, and this packet
*         need to be send back from the CPU - the source mac for this packet will be
*         taken from this table according to source interface that packet comes from.
*         the destination mac will be the incoming source mac.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
* @param[in] macAddIndex              - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet)
*                                      (APPLICABLE RANGES:0..127)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalInterfaceMacIndexSet
(
    IN  GT_U8                            	devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    IN  GT_U32	     		            	macAddIndex
);

/**
* @internal cpssDxChOamSrvLocalInterfaceMacIndexGet function
* @endinternal
*
* @brief   Get the MAC address index of a local interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
*
* @param[out] macAddIndexPtr           - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalInterfaceMacIndexGet
(
    IN  GT_U8                            	devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    OUT GT_U32	     		             	*macAddIndexPtr
);

/**
* @internal cpssDxChOamSrvRxFlowEntrySet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] cookiePtr                - for application use
* @param[in] frameParamsPtr           - (pointer to) parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvRxFlowEntrySet
(
    IN  GT_U8                               	    devNum,
    IN  GT_U32                               	    flowId,
    IN  GT_PTR				                        cookiePtr,
    IN  CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC	*frameParamsPtr
);

/**
* @internal cpssDxChOamSrvRxFlowEntryGet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] cookiePtr                - for application use
* @param[out] frameParamsPtr           - parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvRxFlowEntryGet
(
    IN  GT_U8                            	    	devNum,
    IN  GT_U32                           	    	flowId,
    OUT GT_PTR					                    *cookiePtr,
    OUT CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC	*frameParamsPtr
);

/**
* @internal cpssDxChOamSrvSystemInit function
* @endinternal
*
* @brief   Initialize the service CPU OAM Firmware.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] sysParamsPtr             - (pointer to) struct of system paramaters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Must be called after cpssDxChNetIfInit Is called.
*
*/
GT_STATUS cpssDxChOamSrvSystemInit
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC   *sysParamsPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChOamSrvh */

