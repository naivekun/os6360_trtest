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
* @file prvCpssDxChCfg.h
*
* @brief private DxCh initialization PPs functions.
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssDxChCfgh
#define __prvCpssDxChCfgh


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal prvCpssDxChCfgAddrUpInit function
* @endinternal
*
* @brief   This function initializes the Core Address Update mechanism, by
*         allocating the AU descs. block.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to init the Au unit for.
* @param[in] auDescBlockPtr           - (pointer to)A block to allocate the Au descriptors from.
* @param[in] auDescBlockSize          - Size in bytes of Au descBlock.
* @param[in] fuDescBlockPtr           - (pointer to)A block to allocate the Fu descriptors from.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] fuDescBlockSize          - Size in bytes of Fu descBlock.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] useFuQueue               - GT_TRUE  - Fu queue is used - for DxCh2 devices only.
*                                      - GT_FALSE - Fu queue is unused.
* @param[in] useDoubleAuq             - Support configuration of two AUQ memory regions.
*                                      GT_TRUE - CPSS manages two AU Queues with the same size:
* @param[in] auDescBlockSize          / 2.
*                                      GT_FALSE - CPSS manages single AU Queue with size:
* @param[in] auDescBlockSize
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChCfgAddrUpInit
(
    IN GT_U8    devNum,
    IN GT_U8    *auDescBlockPtr,
    IN GT_U32   auDescBlockSize,
    IN GT_U8    *fuDescBlockPtr,
    IN GT_U32   fuDescBlockSize,
    IN GT_BOOL  useFuQueue,
    IN GT_BOOL  useDoubleAuq
);
/**
* @internal prvCpssDxChCfgPort88e1690Bind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC for PHY '88E1690' to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgPort88e1690Bind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
);

/*******************************************************************************
* prvCpssDxChCfgPort88e1690ObjInit
*
* DESCRIPTION:
*       create 88e1690 object. (singleton)
*
* APPLICABLE DEVICES:
*        ALL.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       void
*
* OUTPUTS:
*
* RETURNS:
*       pointer to the '88e1690' object.
*       NULL - on GT_OUT_OF_CPU_MEM error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_MACDRV_OBJ_STC* prvCpssDxChCfgPort88e1690ObjInit(void);

/**
* @internal prvCpssDxChCfgPort88e1690ObjDestroy function
* @endinternal
*
* @brief   destroy 88e1690 object. (singleton)
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - always
*/
GT_STATUS prvCpssDxChCfgPort88e1690ObjDestroy(void);

/*******************************************************************************
* prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet
*
* DESCRIPTION:
*       get info about 88e1690 that hold the DX remote physical port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       remotePhysicalPortNum  - the remote physical port number
*
* OUTPUTS:
*       portIndexPtr - (pointer to) the port index in the array of remotePortInfoArr[]
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if 'remotePhysicalPortNum' not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(
    IN GT_U8    devNum,
    IN GT_U32   remotePhysicalPortNum,
    OUT GT_U32  *portIndexPtr
);

/*******************************************************************************
* prvCpssDxChCfgPort88e1690CascadePortToInfoGet
*
* DESCRIPTION:
*       get the 88e1690 info that is on the DX device on the cascade port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       physicalPort  - the physical port number (of the DX cascade port)
*
* OUTPUTS:
*       None.
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690CascadePortToInfoGet(
    IN GT_U8    devNum,
    IN GT_U32 physicalPort
);

/**
* @internal prvCpssDxChCfgPort88e1690InitEvents function
* @endinternal
*
* @brief   Init interrupt and events for the 88e1690
*
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690InitEvents
(
    IN GT_U8           devNum,
    IN GT_U32   cascadePortNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
);

/**
* @internal prvCpssDxChCfg88e1690RemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfg88e1690RemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
);

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold,
    IN  GT_U32                  timer,
    IN  GT_U32                  tcBitmap
);

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
*
* @param[out] xOffThresholdPtr         - xOff threshold in buffers
* @param[out] xOnThresholdPtr          - xOn threshold in buffers
* @param[out] timerPtr                 - PFC timer
* @param[out] tcBitmapPtr              - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr,
    OUT GT_U32                  *timerPtr,
    OUT GT_U32                  *tcBitmapPtr
);

/**
* @internal prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*       The output parameter are the parsed port number and MAC counter values.
*
*/
GT_STATUS prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
);

/**
* @internal prvCpssDxChCfgAreRemotePortsBound function
* @endinternal
*
* @brief   Check if remote ports were bound to device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] remotePortBoundPtr       - GT_TRUE - was bound, GT_FALSE - wasn't bound
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChCfgAreRemotePortsBound
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                            *remotePortBoundPtr
);

/**
* @internal prvCpssDxChCfg88e1690ReDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on error
*
* @note This function is called under ISR lock.
*
*/
GT_STATUS prvCpssDxChCfg88e1690ReDevNum
(
    IN  GT_U8 oldDevNum,
    IN  GT_U8 newDevNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChCfgh */

