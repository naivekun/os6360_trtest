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
* @file cpssDxChFabricHGLink.h
*
* @brief CPSS DxCh Fabric Connectivity HyperG.Link interface API.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChFabricHGLinkh
#define __cpssDxChFabricHGLinkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* in Lion2/3 6 xpcs lanes per port */
#define CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS 6

/**
* @struct CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC
 *
 * @brief HyperG.Link Rx error conters
*/
typedef struct{

    /** @brief Counts the number of dropped cells
     *  due to cell length error
     */
    GT_U32 badLengthCells;

    /** @brief Counts the number of dropped cells
     *  due to cell header error. An ECC or CRC
     *  check failure Or The cell type is
     *  invalid ( a value of 0 or 7)
     */
    GT_U32 badHeaderCells;

    /** @brief Counts the number of dropped cells
     *  due to an error in PCS to MAC Reformat
     */
    GT_U32 reformatErrorCells;

} CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC;

/**
* @enum CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT
 *
 * @brief Cell error correction algorithm
*/
typedef enum{

    /** Error Check algorithm ECC 7 bits(aka SECDED) */
    CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E,

    /** Error Check algorithm CRC 8 bits */
    CPSS_DXCH_FABRIC_HGL_ECC_CRC8_E

} CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT;

/**
* @enum CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT
 *
 * @brief Traffic direction.
*/
typedef enum{

    /** RX traffic. */
    CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E,

    /** TX traffic. */
    CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E

} CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT;

/**
* @enum CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT
 *
 * @brief Which type of cells to count.
*/
typedef enum{

    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E,

    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_MC_E,

    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_BC_E,

    CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_OTHER_E

} CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT;

/**
* @struct CPSS_DXCH_FABRIC_HGL_RX_DSA_STC
 *
 * @brief Fabric RX DSA tag fields.
*/
typedef struct{

    /** VID assigned to the DSA tag that is attached to the incoming cells */
    GT_U32 vid;

    /** @brief Source
     *  incoming cells.
     *  Comment:
     */
    GT_U32 srcId;

} CPSS_DXCH_FABRIC_HGL_RX_DSA_STC;

/**
* @struct CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC
 *
 * @brief Fabric FC RX E2E (QCN-like) fields.
*/
typedef struct{

    /** EtherType for the E2E packet */
    GT_U32 etherType;

    /** E2E Encapsulated VLAN ID and priority */
    GT_U32 prioVlanId;

    /** E2E congestion point ID */
    GT_U32 cpId;

    /** E2E Version */
    GT_U32 version;

    /** @brief bits[8:0] of E2E encapsulated destination MAC address,
     *  default value is the MAC's hardwired port number.
     *  Bits [47:9] are constant zero.
     *  Comment:
     */
    GT_U8 macLowByte;

} CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC;

/**
* @struct CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC
 *
 * @brief Fabric FC RX Link Layer (LL) packet and DSA tag fields.
*/
typedef struct{

    /** @brief LL
     *  This field defines the EtherType field indicating that
     *  the packet is PFC-like.
     */
    GT_U32 etherType;

    /** This field defines the Opcode field in the PFC header. */
    GT_U32 pfcOpcode;

    /** @brief The UP field assigned to the DSA tag that is attached to the
     *  incoming LL-FC cell.
     */
    GT_U32 up;

    /** @brief The QoS Profile field in the DSA tagged that is assigned
     *  by the HGL MAC to the incoming LL-FC cell.
     */
    GT_U32 qosProfile;

    /** @brief The target device field in the DSA tagged that is assigned by
     *  the HGL MAC to the incoming LL-FC cell.
     */
    GT_U8 trgDev;

    /** @brief The Target Port field assigned to the DSA tag that is attached
     *  to the incoming LL-FC cell.
     *  Comment:
     */
    GT_PHYSICAL_PORT_NUM trgPort;

} CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC;


/**
* @enum CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT
 *
 * @brief Types of fabric descriptors.
*/
typedef enum{

    /** byte descriptor */
    CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_20B_E,

    /** byte descriptor */
    CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_24B_E

} CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT;


/**
* @internal cpssDxChFabricHGLinkPcsLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/disable PCS loopback on Hyper.G Link
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  loopback
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
GT_STATUS cpssDxChFabricHGLinkPcsLoopbackEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChFabricHGLinkPcsLoopbackEnableGet function
* @endinternal
*
* @brief   Get PCS loopback on Hyper.G Link status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - loopback enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
GT_STATUS cpssDxChFabricHGLinkPcsLoopbackEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Idle counter in the Tx SM. When Idle counter is
*         disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] idleCntEnable            - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              idleCntEnable
);

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of the Idle counter in the Tx SM. When Idle
*         counter is disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] idleCntEnablePtr         - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - idleCntEnablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *idleCntEnablePtr
);

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntSet function
* @endinternal
*
* @brief   Configure the number of cycles between ||k|| sequences.
*         ||K|| sequence must be sent every programmable
*         number of cycles in order to keep a sufficient
*         frequency of commas for each lane and maintain
*         code group alignment.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] maxIdleCnt               - number of cycles between ||k|| sequences,
*                                      (APPLICABLE RANGES: 0..32768)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - if maxIdleCnt out of range
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxIdleCnt
);

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntGet function
* @endinternal
*
* @brief   Get the number of cycles between ||k|| sequences.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] maxIdleCntPtr            - number of cycles between ||k|| sequences
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if maxIdleCntPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxIdleCntPtr
);


/**
* @internal cpssDxChFabricHGLinkPcsRxStatusGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS] - array of statuses of lanes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if laneSyncOkArr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsRxStatusGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS]
);

/**
* @internal cpssDxChFabricHGLinkRxErrorCntrGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxErrorsCntrsPtr         - array of statuses of lanes (6 in Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if rxErrorsCntrsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxErrorCntrGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC *rxErrorsCntrsPtr
);

/**
* @internal cpssDxChFabricHGLinkConfigEccTypeSet function
* @endinternal
*
* @brief   This routine defines how to calculate
*         the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] eccType                  - the Error Correction Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device,eccType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkConfigEccTypeSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT eccType
);

/**
* @internal cpssDxChFabricHGLinkConfigEccTypeGet function
* @endinternal
*
* @brief   Get current type of the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] eccTypePtr               - current Error Correction Type of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if eccTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkConfigEccTypeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT *eccTypePtr
);

/**
* @internal cpssDxChFabricHGLinkRxCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxCrcCheckEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChFabricHGLinkRxCrcCheckEnableGet function
* @endinternal
*
* @brief   Get status of CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - enable CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxCrcCheckEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChFabricHGLinkLbiEnableSet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkLbiEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChFabricHGLinkLbiEnableGet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkLbiEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChFabricHGLinkCellsCntrTypeSet function
* @endinternal
*
* @brief   Configure the types of received/transmitted cells to be
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
* @param[in] cellType                 - counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    IN  CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  cellType
);

/**
* @internal cpssDxChFabricHGLinkCellsCntrTypeGet function
* @endinternal
*
* @brief   Get the types of received/transmitted cells
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellTypePtr              - current counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  *cellTypePtr
);

/**
* @internal cpssDxChFabricHGLinkCellsCntrGet function
* @endinternal
*
* @brief   Get the counter of received/transmitted cells
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellCntrPtr              - counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellCntrPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT GT_U32                              *cellCntrPtr
);

/**
* @internal cpssDxChFabricHGLinkFcMacSaSet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macPtr                   - new source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcMacSaSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_ETHERADDR            *macPtr
);

/**
* @internal cpssDxChFabricHGLinkFcMacSaGet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macPtr                   - source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcMacSaGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_ETHERADDR         *macPtr
);

/**
* @internal cpssDxChFabricHGLinkRxDsaParamsSet function
* @endinternal
*
* @brief   Configure some Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note Also used for E2E-FC and LL-FC DSA Tag parameters configuration
*
*/
GT_STATUS cpssDxChFabricHGLinkRxDsaParamsSet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
);

/**
* @internal cpssDxChFabricHGLinkRxDsaParamsGet function
* @endinternal
*
* @brief   Get some configurable Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxDsaParamsGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
);

/**
* @internal cpssDxChFabricHGLinkFcRxE2eParamsSet function
* @endinternal
*
* @brief   Configure some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcRxE2eParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
);

/**
* @internal cpssDxChFabricHGLinkFcRxE2eParamsGet function
* @endinternal
*
* @brief   Get some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcRxE2eParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
);

/**
* @internal cpssDxChFabricHGLinkRxLinkLayerFcParamsSet function
* @endinternal
*
* @brief   Configure some Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxLinkLayerFcParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
);

/**
* @internal cpssDxChFabricHGLinkRxLinkLayerFcParamsGet function
* @endinternal
*
* @brief   Get configurable Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxLinkLayerFcParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
);


/**
* @internal cpssDxChFabricHGLinkDescriptorTypeSet function
* @endinternal
*
* @brief   Configure the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkDescriptorTypeSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   type
);
 

 

/**
* @internal cpssDxChFabricHGLinkDescriptorTypeGet function
* @endinternal
*
* @brief   Gets the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] typePtr                  - (pointer to) descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkDescriptorTypeGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    OUT CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   *typePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChFabricHGLinkh */



