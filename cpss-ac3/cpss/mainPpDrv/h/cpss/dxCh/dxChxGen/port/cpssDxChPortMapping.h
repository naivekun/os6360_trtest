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
* @file cpssDxChPortMapping.h
*
* @brief CPSS physical port mapping
*
* @version   9
********************************************************************************
*/
#ifndef __CPSS_DXCH_PORT_MAPPING_H
#define __CPSS_DXCH_PORT_MAPPING_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS ((GT_U32)(~0))

/**
* @enum CPSS_DXCH_PORT_MAPPING_TYPE_ENT
 *
 * @brief interface types of physical ports.
*/
typedef enum{

    /** Ethernet MAC */
    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E = 0

    /** CPU SDMA */
    ,CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E

    /** Interlaken Channel (all channels 0-63). */
    ,CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E

    /** Port represent physical interface on remote device or Interlaken channel. */
    ,CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E

    /** max definiton */
    ,CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E

    ,CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E = ~0

} CPSS_DXCH_PORT_MAPPING_TYPE_ENT;


#define PRV_CPSS_DXCH_BC2_PORT_NUM_CPU_DMA_CNS        72
#define PRV_CPSS_DXCH_BC3_PORT_NUM_CPU_DMA_CNS        74

#define PRV_CPSS_DXCH_PORT_NUM_ILKN_DMA_CNS           64
#define PRV_CPSS_DXCH_PORT_NUM_ILKN_MAC_CNS           PRV_CPSS_DXCH_PORT_NUM_ILKN_DMA_CNS
#define PRV_CPSS_DXCH_PORT_NUM_TM_TxDMA_CNS           73
#define PRV_CPSS_DXCH_PORT_NUM_TM_TxQ_CNS             64


#define GT_NA ((GT_U32)~0)  /* used in mapping */
/**
* @struct CPSS_DXCH_PORT_MAP_STC
 *
 * @brief General physical port map structure
 *        define the 'connection' between the 'physical port number' and the
 *        actual global 'MAC/DMA' and global 'TXQ queue' that it uses.
 *
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
*/
typedef struct{

    /** @brief physical port number
     *  (APPLICABLE RANGES: Bobcat2: 0.255; Caelum,Aldrin,AC3X,Aldrin2: 0..127;
     *      Bobcat3: 0..255/511; Falcon: 0..63/127/255/511/1023)
     */
    GT_PHYSICAL_PORT_NUM physicalPortNumber;

    /** mapping type */
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

    /** @brief port group number of interface.
     *  Relevant for multi port group devices only.
     *  (APPLICABLE DEVICES: None)
    */
    GT_U32 portGroup;

    /** @brief interfaceNumber  - interface number.
     *     when mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E
     *  or when mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E:
     *  (APPLICABLE RANGES :Bobcat2 0..71, Bobcat3; Aldrin2 0..31; Falcon: 0..64(3.2T),0..129(6.4T),0..257(12.8T))
     *  when mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E:
     *  (APPLICABLE RANGES :Bobcat2 0..63)
     *
     *  when mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:
     *  Falcon : the 'interfaceNum' serves as 'Global SDMA port number'
     *          NOTE: the 8 queue of this port that used by the 'networkIf' functions ,
     *              are determined by the 'Global SDMA port number' from 0,8,16..,(x-1)*8
     *  (APPLICABLE RANGES :Falcon : 65..68(3.2T),130..137(6.4T),258..275(12.8T))
     *  Other devices: ignored
     *
     */
    GT_U32 interfaceNum;

    /** @brief TXQ port number
     *  (APPLICABLE RANGES: Bobcat2,BobK:0..71 ;Bobcat3: 0..576 ;Aldrin2:0..99)
     *  Falcon : Ignored ! (internally determined by the CPSS)
     */
    GT_U32 txqPortNumber;

    /** @brief enable Traffic Manager processing on egress physical port
     *  GT_FALSE - TM disabled for port, packet bypass TM
     *  GT_TRUE - TM enabled for port, packets handled by TM
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    GT_BOOL tmEnable;

    /** @brief TM port index.
     *  Relevant only when tmEnable == GT_TRUE.
     *  (APPLICABLE RANGES: 0..71 and 128..191)
     *  (APPLICABLE DEVICES: Bobcat2)
     *  Comments:
     */
    GT_U32 tmPortInd;

} CPSS_DXCH_PORT_MAP_STC;



/**
* @struct CPSS_DXCH_SHADOW_PORT_MAP_STC
 *
 * @brief A structure to hold detailed information about
 * physical port mapping.
*/
typedef struct{

    /** mapping type */
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

    /** ports traffic forwarded through TM */
    GT_BOOL trafficManagerEn;

    /** @brief port group number of interface.
     *  Relevant for multi port group devices only.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 portGroup;

    /** @brief MAC Number for ETHERNET and REMOTE_PHYSICAL map types
     *  Parameter is ignored for other mapping types.
     *  (APPLICABLE RANGES: Bobcat2, Caelum 0..71, Bobcat3; Aldrin2, Aldrin, AC3X 0..31)
     */
    GT_U32 macNum;

    /** RXDMA port number */
    GT_U32 rxDmaNum;

    /** TXDMA port number */
    GT_U32 txDmaNum;

    /** TxFIFO port number */
    GT_U32 txFifoPortNum;

    /** @brief TXQ port number
     *  (APPLICABLE RANGES: 0..71)
     */
    GT_U32 txqNum;

    /** @brief Interlaken channel number.
     *  Relevant only for mapping type
     *  CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E.
     *  (APPLICABLE RANGES: 0..63)
     */
    GT_U32 ilknChannel;

    /** traffic manager port index */
    GT_U32 tmPortIdx;

    /** whether port is declared as extended */
    GT_U32 isExtendedCascadePort;

} CPSS_DXCH_SHADOW_PORT_MAP_STC;


/**
* @struct CPSS_DXCH_DETAILED_PORT_MAP_STC
 *
 * @brief A structure to hold shadow of physical port mapping.
*/
typedef struct{

    /** @brief valid mapping
     *  GT_TRUE - valid mapping, was checked and set by cpssDxChPortPhysicalPortMapSet
     *  GT_FALSE - not valid mapping
     */
    GT_BOOL valid;

    /** structure that hold detailed port mapping */
    CPSS_DXCH_SHADOW_PORT_MAP_STC portMap;

} CPSS_DXCH_DETAILED_PORT_MAP_STC;

/**
* @internal prvCpssDxChPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port is CPU port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it was not mapped as CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
);

/**
* @internal cpssDxChPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. when CPSS_DXCH_CFG_PORT_MAPPING_TYPE_INLK_CHANNEL_E is defined the first channel
*       phsical port is used for DMA configuration
*       4. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       #define GT_NA ((GT_U32)-1)
*       CPSS_DXCH_CFG_PORT_MAP_STC bc2ExampleMap[] =
*       (
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   0, 0, GT_FALSE),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   1, 1, GT_FALSE),
*       ( 50, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   50, 2, GT_FALSE),
*       ( 53, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   53, 3, GT_FALSE),
*       ( 63, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, GT_NA, 71, GT_FALSE),
*       (100, _INTRFACE_TYPE_ILNK_MAC_E,   GT_NA, GT_NA, 40, GT_TRUE),
*       (101, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 41, GT_TRUE),
*       (102, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 42, GT_TRUE),
*       (103, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 43, GT_TRUE)
*       );
*       rc = cpssDxChCfgPhysicalPortMapSet(devNum, sizeof(bc2ExampleMap)/sizeof(bc2ExampleMap[0]), &bc2ExampleMap[0]);
*
*/
GT_STATUS cpssDxChPortPhysicalPortMapSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr
);

/**
* @internal cpssDxChPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mappping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_DXCH_PORT_MAP_STC    *portMapArrayPtr
);


/**
* @internal cpssDxChPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapIsValidGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
);

/**
* @internal cpssDxChPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port is CPU port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it was not mapped as CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
);

/**
* @internal cpssDxChPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] origPortType             - original port type
* @param[in] origPortNum              - MAC/TXQ/DMA port number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - when this functionality is not implemented
*/
GT_STATUS cpssDxChPortPhysicalPortMapReverseMappingGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
);

/**
* @internal cpssDxChPortPhysicalPortDetailedMapGet function
* @endinternal
*
* @brief   Function returns physical port detailed mapping
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMapShadowPtr         - (pointer to) detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortDetailedMapGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

