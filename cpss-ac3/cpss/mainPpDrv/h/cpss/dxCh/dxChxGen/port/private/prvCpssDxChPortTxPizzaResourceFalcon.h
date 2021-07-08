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
* @file prvCpssDxChPortTxPizzaResourceFalcon.h
*
* @brief  CPSS Falcon Data Path Tx unit implementation for Port Pizza and resource configuration.
*
* @version   0
********************************************************************************
*/
#ifndef __prvCpssDxChPortTxPizzaResourceFalconh
#define __prvCpssDxChPortTxPizzaResourceFalconh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_CPSS_SPEED_PROFILE_ENT
 *
 * @brief Enumeration of speed profiles
*/
typedef enum
{
    /** speed profile 0 . */
    PRV_CPSS_SPEED_PROFILE_0_E,

    /** speed profile 1 . */
    PRV_CPSS_SPEED_PROFILE_1_E,

    /** speed profile 2 . */
    PRV_CPSS_SPEED_PROFILE_2_E,

    /** speed profile 3 . */
    PRV_CPSS_SPEED_PROFILE_3_E,

    /** speed profile 4 . */
    PRV_CPSS_SPEED_PROFILE_4_E,

    /** speed profile 5 . */
    PRV_CPSS_SPEED_PROFILE_5_E,

    /** speed profile 6 . */
    PRV_CPSS_SPEED_PROFILE_6_E,

    /** speed profile 7 . */
    PRV_CPSS_SPEED_PROFILE_7_E,

    PRV_CPSS_PORT_SPEED_PROFILE_NA_E

}PRV_CPSS_SPEED_PROFILE_ENT;


/**
* @internal prvCpssDxChPortTxSpeedProfileGet function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] txIndex                - tx global index.  
* 
* @param[out] txSpeedProfilePtr     - (pointer to) speed profile array in size FALCON_PORTS_PER_DP_CNS (8).
*                                     each node should hold the number of his speedProfile
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxSpeedProfileGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      txIndex,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *txSpeedProfilePtr
);

/**
* @internal prvCpssDxChTxDPPizzaSliceBitMapGet function
* @endinternal
*
* @brief   Gets ports pizza slices bitmap for specified dp.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] txIndex                - tx global index.  
* 
* @param[out] txPizzaSliceArrBitMapPtr     - (pointer to) slices 
*                                     bit map array in size
*                                     FALCON_PORTS_PER_DP_CNS
*                                     (8). each node should hold
*                                     bit map of the port slices
*                                     in the pizza.
*                                     0: the slice not belong to
*                                     the port
*                                     1: the slice belong to the
*                                     port
* 
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device  
*/
GT_STATUS prvCpssDxChTxDPPizzaSliceBitMapGet
(
    IN   GT_U8                       devNum,
    IN   GT_U32                      txIndex,
    OUT  GT_U32                      *txPizzaSliceArrBitMapPtr
);

/**
* @internal prvCpssDxChGetTxIndexAndChannelIndexfromPortNum function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - pysical port number
* 
* @param[out] txDmaIndexPtr       - (pointer to)  Tx global 
*       index (0..31)
* @param[out] portGroupPtr        - (pointer to)  channel local 
*      index in DP (0..7)
* 
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device  
*/
GT_STATUS prvCpssDxChGetTxIndexAndChannelIndexfromPortNum      
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *txDmaGlobalIndexPtr,
    OUT GT_U32                       *channelIndexPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSet function
* @endinternal
*
* @brief    Sets speed for specified port on specified device in falcon pizza arbiter .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - pysical port number
* @param[in] ifMode                 - interface mode 
* @param[in] portSpeed              - port speed
* @param[in] enable                 - set new resorce and pizza or delete pizza
* 
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device  
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    IN  GT_BOOL                      enable
);

/**
* @internal prvCpssDxChTxPortSpeedProfileAndPizzaSliceGet function
* @endinternal
*
* @brief    Get speed profile and pizza slices.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - interface mode 
* @param[in] portSpeed              - port speed
* @param[in] portGroup              - port group
* 
* @param[out] portSlicesNumPtr      - (pointer to) number of slice for the port
* @param[out] portSpeedProfilePtr   - (pointer to) the port speed profile
* 
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedProfileAndPizzaSliceGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    OUT  GT_U32                      *portSlicesNumPtr,
    OUT  PRV_CPSS_SPEED_PROFILE_ENT  *portSpeedProfilePtr
);


/**
* @internal prvCpssDxChFalconPortPizzaArbiterInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present for falcon
*       the function set the slices number and  go over All pizza slices (except cpu slice) in all DP and set the slice valid bit to 0.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode 
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device 
*/
GT_STATUS prvCpssDxChFalconPortPizzaArbiterInit
(
    IN  GT_U8                   devNum
);

/**
* @internal prvCpssDxChTxPizzaConfigurationSet function
* @endinternal
*
* @brief  Pizza arbiter in txDMA configuration set.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txDmaIndex            - global Tx index
* @param[in] localChannelIndex     - local channel index ( 0..7)
* @param[in] portPizzaSlicesBitMap - bit map representing the slices sould be configured
* @param[in] enable                -  set new slices ( by add port number to the slice and by set bit valid to 1) 
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E, 
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] isCpu                 - is cpu port 
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPizzaConfigurationSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txGlobalDmaIndex,
    IN  GT_U32                       localChannelIndex, 
    IN  GT_U32                       portPizzaSlicesBitMap, 
    IN  GT_BOOL                      enable,
    IN  PRV_CPSS_DXCH_UNIT_ENT       unitType,
    IN  GT_BOOL                      isCpu
);

/**
* @internal prvCpssDxChTxPortPizzaArbiterMaxSliceSet function
* @endinternal
*
* @brief   Pizza arbiter set max slice ( pizza size) in pizza configuration in txdma
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txDmaIndex            - global Tx index 
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E, 
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] maxSliceNum
*       - max slice number to
*   confige
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode 
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device 
*/
GT_STATUS prvCpssDxChTxPortPizzaArbiterMaxSliceSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  txGlobalDmaIndex,
    IN  PRV_CPSS_DXCH_UNIT_ENT  unitType,
    IN  GT_U32                  maxSliceNum
);

/**
* @internal prvCpssDxChPortTxPizzaSliceGet function
* @endinternal
*
* @brief   Get Pizza arbiter slices per port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
* @param[out] txPizzaSliceArrPtr   - (pointer to) array
*             in size FALCON_PORTS_PER_DP_CNS (8). each node
*             should hold the number of the port slices. the
*             index is the port number in the pizza slice.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxPizzaSliceGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      txGlobalIndex,
    OUT  GT_U32     *txPizzaSliceArryPtr 
);


/**
* @internal prvCpssDxChPortTxPizzaSizeGet function
* @endinternal
*
* @brief   Get Pizza arbiter slices per port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
* 
* @param[out] txPizzaSliceNumPtr   - (pointer to) the number of
*       max pizza slices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_NOT_SUPPORTED         - wrong media interface mode 
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device 
*/
GT_STATUS prvCpssDxChPortTxPizzaSizeGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      txGlobalIndex,
    OUT  GT_U32      *txPizzaSliceNumPtr
);

/**
* @internal prvCpssDxChTxCpuPizzaGet function
* @endinternal
*
* @brief   Get Pizza arbiter slices for cpu port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
* @param[out] cpuSliceNumber        - (pointer to) amount of 
*       slices for CPU port
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
**/
GT_STATUS prvCpssDxChTxCpuPizzaGet
(
    IN   GT_U8       devNum,
    IN   GT_U32      txIndex,
    OUT  GT_U32      *cpuSliceNumber
);


/**
* @internal prevCpssDxChPortTxDumpFalconAll function
* @endinternal
*
* @brief   Dumps all related Tx information:
*       go over all DP units and print the pizza slices configurations and the speed profile configuration
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* 
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prevCpssDxChPortTxDumpFalconAll
(
    IN  GT_U8 devNum
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

