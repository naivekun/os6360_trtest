/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortDynamicPAPortSpeedDB.c
*
* DESCRIPTION:
*       bobcat2 and higher dynamic (algorithmic) pizza arbiter Port Speed DB
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_PORT_SPEED_DB_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_PORT_SPEED_DB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

extern GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBInit
(
    IN    GT_U8    devNum
);

extern GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32                      speedInMBit
);

extern GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
);

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet function
* @endinternal
*
* @brief   Set TX_FIFO Pizza Arbiter Channel Speed factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[in] speedFactor              - Speed Factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
extern GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    GT_U32                     speedFactor
);

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet function
* @endinternal
*
* @brief   Get TX_FIFO Pizza Arbiter Channel Speed factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[out] speedFactor             - (pointer to)Speed Factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
extern GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_U32                     *speedFactorPtr
);

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoAdjustedSpeedDBGet function
* @endinternal
*
* @brief   Calculate TX_FIFO Pizza Arbiter Channel Speed by Port Speed and Channel Speed Factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[out] isInitilizedPtr         - (pointer to)GT_TRUE - initialized, GT_FALSE - not initialized.
* @param[out] speedInMBitPtr          - (pointer to)Channel Speed in MBit/Second
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
extern GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoAdjustedSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                    *isInitilizedPtr,
    OUT   GT_U32                     *speedInMBitPtr
);

extern GT_STATUS prvCpssDxChPortDynamicPAIlknChannelSpeedDBSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32                      speedInMBit
);

extern GT_STATUS prvCpssDxChPortDynamicPAIlknChannelSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
);


extern GT_STATUS prvCpssDxChPortDynamicPAIlknSpeedResolutionSet
(
    IN    GT_U8                       devNum,
    IN    GT_U32                      speedResulutionMBps,
    IN    GT_U32                      ilknIFnaxBWMbps
);


extern GT_STATUS prvCpssDxChPortDynamicPAIlknSpeedResolutionGet
(
    IN    GT_U8                       devNum,
    OUT   GT_U32                     *speedResulutionMBpsPtr,
    OUT   GT_U32                     *ilknIFnaxBWMbpsPtr
);


/*---------------------------------------------------------------------*
 *   PA client BW database for clinets having fixed BW (provisioning)  *
 *      operations:                                                    *
 *          Init                                                       *
 *          Add/Update                                                 *
 *          Remove                                                     *
 *  example (on base of TM):                                           *
 *       TxQ    64    25000                                            *
 *       TxDMA  73    25000                                            *
 *       TxFIFO 73    25000                                            *
 *---------------------------------------------------------------------*/
#define BAD_CLIENT_ID  (GT_U32)(~0)

GT_STATUS prvCpssDxChPortDynamicPAClientBWListInit
(
    IN   GT_U8                 devNum
);


GT_STATUS prvCpssDxChPortDynamicPAClientBWListUpdate
(
    IN   GT_U8                 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId,
    IN   GT_U32                bwMbps
);


GT_STATUS prvCpssDxChPortDynamicPAClientBWListDelete
(
    IN   GT_U8                 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId
);


GT_STATUS prvCpssDxChPortDynamicPAClientBWGet
(
    IN   GT_U8                 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId,
    OUT  GT_U32               *bwMbpsPtr
);

/* for iteration */
GT_U32 prvCpssDxChPortDynamicPAClientBWListLenGet
(
    IN   GT_U8                 devNum
);

PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC * prvCpssDxChPortDynamicPAClientBWListEntryByIdxGet
(
    IN   GT_U8                 devNum,
    IN   GT_U32                idx
);

/*----------------------------------------------------------------------------*
 *   PA special TXQ client group (used for extender-cascade-ports clients)    *
 *          Init (groupId)                                                    *
 *          Add                                                               *
 *          Remove                                                            *
 *  example :                                                                 *
 *       type    phys Txq  BWMbps                                             *
 *      -------                                                               *
 *       ccfc    xxx  70   1000                                               *
 *       remote  101  51   1000                                               *
 *       remote  102  51   1000                                               *
 *----------------------------------------------------------------------------*
 *
 * DEV DB
 *    |
 *    +----PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC
 *    |            |
 *    |      PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC
 *    |            |
 *    |            |
 *    |            |----- groupIdPlaceList < groupId, place in clientGroup DB)
 *    |            |
 *    |            |----- clientGroup DB (N client group) group id -- mac of extended-cascade-port
 *    |            |         |
 *    |            |   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC
 *    |            |                    |                                   |
 *    |            |                    +--- group Id                       +--- group Id
 *    |            |                    |                                   |
 *    |            |                    +--- List-of-Clients                +--- List-of-Clients
 *    |            |                    |        |
 *    |            |                    |        +---- client type (ccfc / remote port)
 *    |            |                    |        +---- physical port
 *    |            |                    |        +---- txq (*)          -- clients key is txq (ccfc has no physical port)
 *    |            |                    |        +---- speedMbps
 *    |            |                    +------functions
 *    |            |                             |
 *    |            |                             +--- prvDynamicPATxQClientGroupInit        (groupPtr,groupId)
 *    |            |                             +--- prvDynamicPATxQClientGroupClientAdd   (groupPtr,clientType,portNum, Txq, speedMbps)
 *    |            |                             +--- prvDynamicPATxQClientGroupClientDelete(groupPtr,                    Txq)
 *    |            |
 *    |            +-------- functions
 *    |                        |
 *    |                        +---- prvDynamicPATxQClientGroupListInit  (groupListPtr);
 *    |                        +---- prvDynamicPATxQClientGroupListGet   (groupListPtr,groupId,OUT groupPtrPtr)
 *    |                        +---- prvDynamicPATxQClientGroupListAdd   (groupListPtr,groupId);
 *    |                        +---- prvDynamicPATxQClientGroupListDelete(groupListPtr,groupId);
 *    +-------- functions
 *                |
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListInit                          (devNum);
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListGet                           (devNum,groupId,OUT groupPtrPtr)
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListAdd                           (devNum,groupId);
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListDelete                        (devNum,groupId);
 *                |-- for iterator
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListLenGet                        (devNum,OUT lenPtr);
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet                      (devNum,idx, OUT groupPtrPtr)
 *                |-- for group update
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListClientAdd                     (devNum,groupId,clientType,portNum, Txq, speedMbps)
 *                +---- prvCpssDxChPortDynamicPATxQClientGroupListClientDelete                  (devNum,groupId,                    Txq           )
 *                +---- GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet(devNum,groupId,status)
 *
 *
 *------------------------------------------------------------------------------------------------------*/
GT_STATUS prvDynamicPATxQClientGroupListInit
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr
);


GT_STATUS prvDynamicPATxQClientGroupListGet
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
);

GT_STATUS prvDynamicPATxQClientGroupListAdd
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId
);

GT_STATUS prvDynamicPATxQClientGroupListDelete
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId
);

/*------------------------*
 * dev level interface    *
 *------------------------*/
GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListInit
(
    IN   GT_U8  devNum
);


GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
);

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListAdd
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId
);

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListDelete
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId
);

/* iterator procedures */
GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListLenGet
(
    IN   GT_U8   devNum,
    OUT  GT_U32  *lenPtr
);

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 idx,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
);


/* client operation on specific group */
GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListClientAdd
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT   clientType,
    IN   GT_PHYSICAL_PORT_NUM                        physicalPortNum,
    IN   GT_U32                                      txqPort,
    IN   GT_U32                                      speedMbps
);

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListClientDelete
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   GT_U32                                      txqPort
);

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   GT_BOOL                                     status   /* GT_TRUE -- exists , GT_FALSE -- no */
);


/**
* @internal prvCpssDxChRemotePortSpeedGet function
* @endinternal
*
* @brief   Get speed of remote port from Pizza Arbiter TXQ special clients
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin, Bobcat3; Aldrin2; Falcon;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] speedMbpsPtr             - (pointer to) port speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong port number or not Remote Port
*/
GT_STATUS prvCpssDxChRemotePortSpeedGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 *speedMbpsPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


