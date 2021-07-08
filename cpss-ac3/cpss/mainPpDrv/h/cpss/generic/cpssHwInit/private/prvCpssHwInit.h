/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssHwInit.h
*
* @brief CPSS definitions for HW init.
*
* @version   48
********************************************************************************
*/
#ifndef __prvCpssHwInith
#define __prvCpssHwInith

#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/generic/port/cpssPortTx.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* macro to free an allocated pointer, and set it to NULL */
#if __WORDSIZE != 64
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_U32)ptr) != PRV_CPSS_SW_PTR_ENTRY_UNUSED)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#else
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_UINTPTR)ptr) != 0xFFFFFFFBFFFFFFFB)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#endif

/* sip_5_20 convert global txq-dq port number to local txq-dq port number */
#define SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_LOCAL_TXQ_DQ_PORT_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) % PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* sip_5_20 convert global txq-dq port number to DQ unit index */
#define SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) / PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* sip_5_20 convert local txq-dq port number in specific DQ unit to global txq-dq port number */
#define SIP_5_20_LOCAL_TXQ_DQ_PORT_TO_GLOBAL_TXQ_DQ_PORT_MAC(dev,localTxqDqPortIndex,dqIndex) \
    ((localTxqDqPortIndex) + ((dqIndex) * PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp))

/* sip_5_20 access to the DB of 'per port' the txq-Dq unit index */
#define SIP_5_20_TXQ_DQ_UNIT_INDEX_OF_PORT_MAC(dev,_physPort)   \
    PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.mapPhyPortToTxqDqUnitIndex[_physPort]

/* sip_5_20 the max number of global txq-Dq ports */
#define SIP_5_20_TXQ_DQ_MAX_PORTS_MAC(dev) \
    (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp * \
     PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp)

/* sip_5_20 the max number of port in TXDMA */
#define SIP_5_20_TXDMA_MAX_PORTS_MAC(dev) \
    (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txNumPortsPerDp)


#define LION2_DEVICE_INDICATION_CNS   0xFFFFFFFF
#define BOBK_DEVICE_INDICATION_CNS    0xFFFFFFFE
#define BC3_DEVICE_INDICATION_CNS     0xFFFFFFFD
#define ALDRIN2_DEVICE_INDICATION_CNS 0xFFFFFFFC
#define FALCON_DEVICE_INDICATION_CNS  0xFFFFFFFB

/**
* @internal prvCpssPpConfigDevDbInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         1. first call for device or device did HW reset :
*         function set the info about the device regarding DMA allocations
*         given by the application.
*         2. else the function check that the info given now match the info given
*         of first time after the HW reset.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] phase2InfoPtr            - (pointer to) phase2 init info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_STATE             - the new info it not the same info that was
*                                       set for this device (according to the bus
*                                       base address) before the 'shutdown'
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbInfoSet
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *phase2InfoPtr
);

/**
* @internal prvCpssPpConfigDevDbPrvInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the AUQ,FUQ private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] auqInternalInfoPtr       - (pointer to)AU queue private info , of all port groups
* @param[in] fuqInternalInfoPtr       - (pointer to)FU queue private info , of all port groups
* @param[in] secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info , of all port groups
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info
* @param[in] auq1InternalInfoPtr      - (pointer to)AU1 queue private info , of all port groups
* @param[in] secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info , of all port groups
* @param[in] auqDeadLockWaPtr         - (pointer to)AUQ deadlock private info (can be NULL when queue not used)
* @param[in] activeAuqIndexPtr        - (pointer to)AUQ 'active AUQ' private info (can be NULL when queue not used)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*
* @note NOTE: not checking for NULL pointers to speed up performances
*
*/
GT_STATUS prvCpssPpConfigDevDbPrvInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC *auqDeadLockWaPtr,
    IN GT_U32  *activeAuqIndexPtr
);

/**
* @internal prvCpssPpConfigDevDbInfoGet function
* @endinternal
*
* @brief   function to get the init parameters given during initialization.
*         it is needed for application that need to do re-init without HW reset ,
*         and want to use same init parameters, that relate to DMAs .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_SUPPORTED         - device not hold DMA info
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbInfoGet
(
    IN GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC*        *netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC*           *auqCfgPtrPtr,
    OUT GT_BOOL*                    *fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC*           *fuqCfgPtrPtr,
    OUT GT_BOOL*                    *useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC*  *multiNetIfCfgPtrPtr
);


/**
* @internal prvCpssPpConfigDevDbPrvInfoGet function
* @endinternal
*
* @brief   private (internal) function to :
*         get the AUQ,FUQ private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id , to support multi-port-groups device
*
* @param[out] auqInternalInfoPtr       - (pointer to)AU queue private info
* @param[out] fuqInternalInfoPtr       - (pointer to)FU queue private info (can be NULL when FUQ not used)
* @param[out] secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info (can be NULL when queue not used)
* @param[out] auq1InternalInfoPtr      - (pointer to)AU1 queue private info
* @param[out] secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info (can be NULL when queue not used)
* @param[out] auqDeadLockWaPtr         - (pointer to)AUQ deadlock private info (can be NULL when queue not used)
* @param[out] activeAuqIndexPtr        - (pointer to)AUQ 'active AUQ' private info (can be NULL when queue not used)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvInfoGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC *auqDeadLockWaPtr,
    OUT GT_U32  *activeAuqIndexPtr
);

/**
* @internal prvCpssPpConfigDevDbPrvNetInfQueueInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the Rx descriptors list per queue private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] netIfNum                 - The network interface number.
* @param[in] rxDescPtr                - (pointer to) the start of Rx descriptors block.
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfQueueInfoSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       netIfNum,
    IN GT_U32                       queueNum,
    IN PRV_CPSS_RX_DESC_STC         *rxDescPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr
);

/**
* @internal prvCpssDxChNetIfMultiNetIfNumberGet function
* @endinternal
*
* @brief   private (internal) function to :
*         build the information about all the network interfaces according to phase1 and phase2 params
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The device number.
*
* @param[out] numOfNetIfsPtr           - (pointer to) the number of network intrfaces according to port mapping
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssDxChNetIfMultiNetIfNumberGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *numOfNetIfsPtr
);
/**
* @internal prvCpssPpConfigPortToPhymacObjectBind function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - error
*/
GT_STATUS prvCpssPpConfigPortToPhymacObjectBind
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
);

/**
* @internal prvCpssDxChHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      there are some special 'devNum' :
*                                      see LION2_DEVICE_INDICATION_CNS ,
*                                      BOBK_DEVICE_INDICATION_CNS ,
*                                      BC3_DEVICE_INDICATION_CNS
* @param[in] unitId                   - the ID of the address space unit.
*
* @param[out] errorPtr                 - (pointer to) indication that function did error.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssDxChHwUnitBaseAddrGet
(
    IN GT_U32                   devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_BOOL                 *errorPtr
);

/**
* @internal prvCpssDxChHwSubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of subunit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] subunitId                - the ID of the address space unit.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssDxChHwSubunitBaseAddrGet
(
    IN GT_U8                       devNum,
    IN PRV_CPSS_SUBUNIT_ENT        subunitId,
    IN GT_U32                      *indexArr
);

/**
* @internal prvCpssDxChHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
);


/**
* @internal convertRegAddrToNewUnitsBaseAddr function
* @endinternal
*
* @brief   the assumption is that the 'old address' is based on Lion2 units base addresses !
*         so need to convert it to address space of current device
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in,out] oldNewRegPtr             - pointer to the 'old' reg address (from Lion2)
* @param[in,out] oldNewRegPtr             - pointer to the 'new' reg address - after aligned to the current device.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS convertRegAddrToNewUnitsBaseAddr
(
    IN GT_U8    devNum,
    INOUT GT_U32 *oldNewRegPtr
);


/**
* @internal prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet function
* @endinternal
*
* @brief   get the DMA index that is reserved for the CPU port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] txqPortNumber            - sip5 : The global txq port number of the cpu port
*                                       sip6 : The global DMA port number of the cpu port
*
* @param[out] indexPtr                 - pointer to the index
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  txqPortNumber,
    OUT GT_U32  *indexPtr
);


/**
* @internal prvCpssDxChHwPpSdmaCpuPortReservedMacIdxGet function
* @endinternal
*
* @brief   get the index that is reserved for the CPU port for 'link status' filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] portNum                  - The Physical port number of the cpu port
*
* @param[out] indexPtr                 - pointer to the index
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpSdmaCpuPortReservedMacIdxGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *indexPtr
);


/**
* @internal prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
);

/**
* @internal prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
);

/**
* @internal prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
);

/**
* @internal prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert function
* @endinternal
*
* @brief   convert the physical port number in device to port group index.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] physicalPortNum          - the physical port number.
*
* @param[out] portGroupIdPtr           - (pointer to) the port group of the port
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    physicalPortNum,
    OUT GT_U32                 *portGroupIdPtr
);

/**
* @internal prvCpssDxChHwEgfQagTcDpMapperByParamsSet function
* @endinternal
*
* @brief   (sip5_20) set the EGF QAG 'TC,DP' mapping table , according to parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*                                      next parameters are part of the key , which define the 'index' to the table
* @param[in] origTc                   - orig TC (traffic class).
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] origDp                   - orig DP (Drop precedence).
* @param[in] packetCommand            - packet command (on of CPSS_DXCH_NET_DSA_CMD_ENT)
* @param[in] isMultiDestination       - GT_TRUE - multi-destination
*                                      GT_FALSE - single destination
* @param[in] targetPortTcProfile      - Target Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] sourcePortTcProfile      - Source Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
*                                      next parameters are part of the 'data' to set according to the key
* @param[in] newTxqDp                 - the DP to assign in the TXQ
* @param[in] newTxqTc                 - the TC to assign in the TXQ
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] newTxqPfcTc              - the PFC TC to assign in the TXQ
*                                      (APPLICABLE RANGES: 0..7)
*                                       GT_OK on success
* @param[in]  mcastPriority            -defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon).
*/
GT_STATUS prvCpssDxChHwEgfQagTcDpMapperByParamsSet
(
    IN  GT_U8   devNum,
    /* key */
    IN  GT_U32  origTc,
    IN  CPSS_DP_LEVEL_ENT   origDp,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT  packetCommand,
    IN  GT_BOOL   isMultiDestination,
    IN  GT_U32  targetPortTcProfile,
    IN  GT_U32  sourcePortTcProfile,
    /* data */
    IN  GT_U32  newTxqDp,
    IN  GT_U32  newTxqTc,
    IN  GT_U32  newTxqPfcTc,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     mcastPriority
);

/**
* @internal prvCpssDxChHwEgfQagTcDpMapperByParamsGet function
* @endinternal
*
* @brief   (sip5_20) Get the EGF QAG 'TC,DP' mapping table entry, according to parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*                                      next parameters are part of the key , which define the 'index' to the table
* @param[in] origTc                   - orig TC (traffic class).
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] origDp                   - orig DP (Drop precedence).
* @param[in] packetCommand            - packet command (on of CPSS_DXCH_NET_DSA_CMD_ENT)
* @param[in] isMultiDestination       - GT_TRUE - multi-destination
*                                      GT_FALSE - single destination
* @param[in] targetPortTcProfile      - Target Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] sourcePortTcProfile      - Source Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
*                                      next parameters are 'output' to get according to the key
* @param[in] txqDpPtr                 - (pointer to) the DP assigned to the TXQ
* @param[in] txqTcPtr                 - (pointer to) the TC to assigned to the TXQ
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] txqPfcTcPtr              - (pointer to) the PFC TC assigned to the TXQ
*                                      (APPLICABLE RANGES: 0..7)
*                                       GT_OK on success
* @param[out]  mcastPriorityPtr            -(pointer to)defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon).
*/
GT_STATUS prvCpssDxChHwEgfQagTcDpMapperByParamsGet
(
    IN  GT_U8   devNum,
    /* key */
    IN  GT_U32  origTc,
    IN  CPSS_DP_LEVEL_ENT   origDp,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT  packetCommand,
    IN  GT_BOOL   isMultiDestination,
    IN  GT_U32  targetPortTcProfile,
    IN  GT_U32  sourcePortTcProfile,
    /* output parameters */
    OUT  CPSS_DP_LEVEL_ENT  * txqDpPtr,
    OUT  GT_U32  * txqTcPtr,
    OUT  GT_U32  * txqPfcTcPtr,
    OUT  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT       *mcastPriorityPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwInith */


