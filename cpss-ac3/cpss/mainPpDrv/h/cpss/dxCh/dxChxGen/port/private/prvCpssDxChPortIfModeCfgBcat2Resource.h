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
* @file prvCpssDxChPortIfModeCfgBcat2Resource.h
*
* @brief CPSS BC2 implementation for Port interface mode resource configuration.
*
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_BCAT2_RESOURCE_H
#define __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_BCAT2_RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

/*--------------------------------------------------------------------------------------------------------------------------------
// Packet travel guide :
//
//  Regular port x with mapping (mac-x,rxdma-x, txdma-x, txq-x,txfifo-x)
//        packet --> port-rx --> RxDMA -----------------> TxQ -->---------------- TxDMA ----------> TXFIIFO --> port-tx
//                    mac-x     rxdma-x                  txq-x                    txdma-x           txfifo-x     mac-x
//
//                           RXDMA_IF_WIDTH                ?                    TxQ Credits       TXFIFO IF width
//                                                                           TxDMA speed(A0)      Shifter Threshold (A0)
//                                                                            Rate Limit(A0)      Payload StartTrasmThreshold
//                                                                           Burst Full Limit(B0)
//                                                                           Burst Amost Full Limit(B0)
//                                                                           Payload Credits
//                                                                           Headers Credits
//
//  port with TM (mac-x,rxdma-x,TxDMA = TM,TxQ = TM , TxFIFO = TM, Eth-TxFIFO == eth-txfifo-x)
//
//        packet --> port-rx --> RxDMA ------------------> TxQ ---------> TxDMA ---------------> TxFIFO -------------------> ETH-THFIFO -----------> port-tx
//                    mac-x      rxdma-x                  64(TM)          73(TM)                 73 (TM)                     eth-txfifo-x             mac-x
//
//                            RXDMA_IF_WIDTH                            TxQ Credits             TXFIFO IF width               Eth-TXFIFO IF width
//                                                                      TxDMA speed(A0)         Shifter Threshold(A0)         Shifter Threshold (A0)
//                                                                      Rate Limit(A0)          Payload StartTrasmThreshold   Payload StartTrasmThreshold
//                                                                      Burst Full Limit(B0)
//                                                                      Burst Amost Full Limit(B0)
//                                                                      Payload Credits
//                                                                      Headers Credits
//
//----------------------------------------------------------------------------------------------------------------------------------
*/

GT_STATUS prvCpssDxChPortBcat2CreditsCheckSet
(
    IN GT_BOOL val
);


#define PRV_CPSS_DXCH_INVALID_RESOURCE_CNS  (GT_U16)(~0)


typedef enum CPSS_DXCH_BC2_PORT_RESOURCES_FLD_ENT
{
     BC2_PORT_FLD_RXDMA_IfWidth  = 0                                   /* rev(A0, B0               ), speed             */
    ,BC2_PORT_FLD_TXDMA_SCDMA_TxQDescriptorCredit                      /* rev(A0, B0               ), speed, core clock */
    ,BC2_PORT_FLD_TXDMA_SCDMA_speed                                    /* rev(A0                   ), speed             */
    ,BC2_PORT_FLD_TXDMA_SCDMA_rateLimitThreshold                       /* rev(A0                   ), speed, core clock */
    ,BC2_PORT_FLD_TXDMA_SCDMA_burstAlmostFullThreshold                 /* rev(    B0               ), speed, core clock */
    ,BC2_PORT_FLD_TXDMA_SCDMA_burstFullThreshold                       /* rev(    B0               ), speed, core clock */
    ,BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold              /* rev(A0, B0               ), speed, core clock */
    ,BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold             /* rev(A0, B0               ), speed, core clock */
    ,BC2_PORT_FLD_TXFIFO_IfWidth                                       /* rev(A0, B0               ), speed             */
    ,BC2_PORT_FLD_TXFIFO_shifterThreshold                              /* rev(A0, B0-ReadOnly      ), speed, core clock */
    ,BC2_PORT_FLD_TXFIFO_payloadThreshold                              /* rev(A0, B0               ), speed, core clock */
    ,BC2_PORT_FLD_Eth_TXFIFO_IfWidth                                   /* rev(A0, B0               ), speed */
    ,BC2_PORT_FLD_Eth_TXFIFO_shifterThreshold                          /* rev(A0,                  ), speed , core clock */
    ,BC2_PORT_FLD_Eth_TXFIFO_payloadThreshold                          /* rev(A0, B0               ), speed , core clock */
    ,BC2_PORT_FLD_FCA_BUS_WIDTH
    ,BC2_PORT_FLD_MAX
    ,BC2_PORT_FLD_INVALID_E = ~0
}CPSS_DXCH_BC2_PORT_RESOURCES_FLD_ENT;


typedef struct
{
    CPSS_DXCH_BC2_PORT_RESOURCES_FLD_ENT fldId;
    GT_U32                         fldArrIdx;
    GT_U32                         fldVal;
}prvCpssDxChBcat2PortResourse_IdVal_STC;


typedef struct
{
    GT_U32 fldN;
    prvCpssDxChBcat2PortResourse_IdVal_STC arr[BC2_PORT_FLD_MAX];
}CPSS_DXCH_BCAT2_PORT_RESOURCE_LIST_STC;


typedef struct
{
    GT_U32  rxdmaScdmaIncomingBusWidth;
    GT_U32  txdmaTxQCreditValue;
    GT_U32  txdmaBurstAlmostFullThreshold;
    GT_U32  txdmaBBurstFullThreshold;
    GT_U32  txdmaTxfifoHeaderCounterThresholdScdma;
    GT_U32  txdmaTxfifoPayloadCounterThresholdScdma;
    GT_U32  txfifoScdmaOutgoingBusWidth;
    GT_U32  txfifoScdmaShiftersThreshold;
    GT_U32  txfifoScdmaPayloadThreshold;
    GT_U32  ethTxfifoOutgoingBusWidth;
    GT_U32  ethTxFifoShifterThreshold;
    GT_U32  ethTxfifoScdmaPayloadThreshold;
}PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC;



typedef enum
{
    PRV_CPSS_RXDMA_IfWidth_64_E = 0,
    PRV_CPSS_RXDMA_IfWidth_256_E = 2,
    PRV_CPSS_RXDMA_IfWidth_512_E = 3,
    PRV_CPSS_RXDMA_IfWidth_MAX_E
}PRV_CPSS_RXDMA_IfWidth_ENT;


typedef enum
{
    PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E = 0,
    PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E = 3,
    PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E = 5,
    PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E = 6,
    PRV_CPSS_TxFIFO_OutGoungBusWidth_MAX_E
}PRV_CPSS_TxFIFO_OutGoungBusWidth_ENT;

typedef enum
{
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_1B_E = 0,
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_8B_E = 3,
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_32B_E = 5,
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_64B_E = 6,
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_MAX_E
}PRV_CPSS_EthTxFIFO_OutGoungBusWidth_ENT;


/**
* @internal prvCpssDxChBcat2PortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBcat2PortResourcesInit
(
    IN    GT_U8                   devNum
);


/**
* @internal prvCpssDxChBcat2PortResourcesConfig function
* @endinternal
*
* @brief   Allocate/free resources of port per it's current status/interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
* @param[in] allocate                 - allocate/free resources:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBcat2PortResourcesConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         allocate
);

/**
* @internal prvCpssDxChBcat2PortResoursesStateGet function
* @endinternal
*
* @brief   read port resources
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] resPtr                   - pointer to list of resources
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported revision
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note just revion B0 and above are supported
*
*/
GT_STATUS prvCpssDxChBcat2PortResoursesStateGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
   OUT  CPSS_DXCH_BCAT2_PORT_RESOURCE_LIST_STC  *resPtr
);


/**
* @internal prvCpssDxChPortBcat2PtpInterfaceWidthSelect function
* @endinternal
*
* @brief   Set PTP interface width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2PtpInterfaceWidthSelect
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChPortBcat2PtpInterfaceWidthGet function
* @endinternal
*
* @brief   Get PTP interface width in bits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
*
* @param[out] busWidthBitsPtr          - pointer to bus with in bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2PtpInterfaceWidthGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                         *busWidthBitsPtr
);


/**
* @internal prvCpssDxChPortBcat2InterlakenTxDmaEnableSet function
* @endinternal
*
* @brief   Set Tx DMA enable state for Interlaken interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2InterlakenTxDmaEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal prvCpssDxChPortBcat2FcaBusWidthSet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortBcat2FcaBusWidthSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChPortBcat2FcaBusWidthGet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] widthBitsPtr             - (pointer to) width
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on wrong pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortBcat2FcaBusWidthGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *widthBitsPtr
);


/**
* @internal prvCpssDxChPortXcat3FcaBusWidthSet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortXcat3FcaBusWidthSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
);


#ifndef GM_USED
/**
* @internal prvCpssDxChLedPortTypeConfig function
* @endinternal
*
* @brief   Configures the type of the port connected to the LED.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - port is powered Up(GT_TRUE) or powered down(GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChLedPortTypeConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp
);

#endif

/**
* @internal prvCpssDxChPortBcat2B0ResourcesCalculateFromHW function
* @endinternal
*
* @brief   Calculate cummulative port resources from HW
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] resourceId               - resource id.
*                                      Applicable values:
*                                      BC2_PORT_FLD_TXDMA_SCDMA_TxQDescriptorCredit,
*                                      BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold,
*                                      BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold
*
* @param[out] totalResourcesNumPtr     - (pointer to) total number of resources
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortBcat2B0ResourcesCalculateFromHW
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_BC2_PORT_RESOURCES_FLD_ENT    resourceId,
    OUT GT_U32                                  *totalResourcesNumPtr
);

/**
* @internal prvCpssDxChBobcat2PortResourseConfigGet function
* @endinternal
*
* @brief   Calculate cummulative port resources from HW
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] resourcePtr              - (pointer to) resources of given port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBobcat2PortResourseConfigGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC      *resourcePtr
);

/**
* @internal prvCpssDxChBcat2B0PortResourceTmSet function
* @endinternal
*
* @brief   Bcat B0 TM port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS prvCpssDxChBcat2B0PortResourceTmSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

