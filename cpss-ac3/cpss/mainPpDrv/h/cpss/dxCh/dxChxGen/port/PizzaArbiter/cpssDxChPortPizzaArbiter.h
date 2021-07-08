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
* @file cpssDxChPortPizzaArbiter.h
*
* @brief external interface for Pizza Arbiter
*
*
* @version   10
********************************************************************************
*/
#ifndef __CPSS_DXCH_PORT_PIZZA_ARBITER_H
#define __CPSS_DXCH_PORT_PIZZA_ARBITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterTables.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterProfileStorage.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterDevInfo.h>
/* slices management for specific units */
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesRxDMA.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesRxDMACTU.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxDMA.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxDMACTU.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesBM.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxQ.h>



/**
* @internal prvCpssDxChPortPizzaArbiterIfInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfInit
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfCheckSupport function
* @endinternal
*
* @brief   Check whether Pizza Arbiter can be configured to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfConfigure
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PORT_SPEED_ENT      portSpeed /* new port speed to be configured */
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfDelete function
* @endinternal
*
* @brief   Delete Pizza Arbiter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfDelete
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for CCFC port
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum;
*
* @param[in] devNum                   - device number
* @param[in] macNum                   - mac Num
*                                      txq       - txq number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  macNum,
    IN  GT_U32                  txqNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
);

/**
* @internal prvCpssDxChPortPizzaArbiterAllCCFCClientsConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for all CCFC ports - i.e. extended cascade ports
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterAllCCFCClientsConfigure
(
    IN  GT_U8                   devNum
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfTxQClientConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for new special TXQ client (remote physical ports)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not applicable CCFC client)
* @param[in] portSpeedMbps            - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfTxQClientConfigure
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT clientType,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  GT_U32                                    txq,
    IN  GT_U32                                    portSpeedMbps

);


/**
* @internal prvCpssDxChPortPizzaArbiterIfTxQClientDelete function
* @endinternal
*
* @brief   delete special TXQ client from Pizza Arbiter(remote physical ports)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] txq                      - clients' txq
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfTxQClientDelete
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    txq
);





/**
* @internal cpssDxChPortPizzaArbiterIfUserTableSet function
* @endinternal
*
* @brief   Set profile and/or speed conversion table
*         for specific device : port group
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupBmp             - port group bitmap
* @param[in] pizzaProfilePtr          - pointer to profile
* @param[in] portGroupPortSpeed2SliceNumPtr - pointer to speed 2 slice conversion list
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - no memory at memory pools (increase constants !!!)
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - profile or speed conversion table already exists
*
* @note 1. Supposed to be called before cpssInitSystem()
*       2. Traffic on all ports of PORT GROUP at the DEVICE shall be stopped
*       3. cpssDxChPortPizzaArbiterIfUserTableSet has two pointers. Only non-NULL pointers
*       are processed.
*       4. In function cpssDxChPortPizzaArbiterIfUserTableSet() non-zero user profile is processed
*       first. If it already exists processing is stopped that means speed conversion
*       table is not processed.
*
*/
GT_STATUS cpssDxChPortPizzaArbiterIfUserTableSet
(
    IN  GT_U8 devNum,
    IN  GT_U32 portGroupBmp,
    IN  CPSS_DXCH_PIZZA_PROFILE_STC                          *pizzaProfilePtr,
    IN  CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portGroupPortSpeed2SliceNumPtr
);


/**
* @internal cpssDxChPortPizzaArbiterIfUserTableGet function
* @endinternal
*
* @brief   Get profile and/or speed conversion table
*         for specific device : port group
*         initially searches among user defined tables and
*         if it is not found there , searches in system tables
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupBmp             - port group bitmap
*
* @param[out] pizzaProfilePtrPtrPtr    - pointer to profile
* @param[out] portGroupPortSpeed2SliceNumPtrPtr - pointer to speed 2 slice conversion list
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - any zero pointer
* @retval GT_NOT_FOUND             - if any table is found
*
* @note Only first port group is used.
*
*/
GT_STATUS cpssDxChPortPizzaArbiterIfUserTableGet
(
    IN   GT_U8 devNum,
    IN   GT_U32 portGroupBmp,
    OUT  CPSS_DXCH_PIZZA_PROFILE_STC                          **pizzaProfilePtrPtrPtr,
    OUT  CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC **portGroupPortSpeed2SliceNumPtrPtr
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfProfileIterInit function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         This function inits iterator
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfProfileIterInit
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC *iterPtr
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfProfileIterReset function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function resets iterator to beginning of the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  - pointer to iterator
* @param[in,out] iterPtr                  - pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfProfileIterReset
(
    INOUT  PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC *iterPtr
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfProfileIterGetCur function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function returns current item
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @param[out] devNumPtr                - device number
* @param[out] portGroupIdxPtr          - port group
* @param[out] pizzaProfilePtrPtr       - pointer to profile
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - all elements were passed already
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfProfileIterGetCur
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC *iterPtr,
    OUT   GT_U8  *devNumPtr,
    OUT   GT_U32 *portGroupIdxPtr,
    OUT   CPSS_DXCH_PIZZA_PROFILE_STC **pizzaProfilePtrPtr
);


/**
* @internal prvCpssDxChPortPizzaArbiterIfProfileIterGotoNext function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function goes to next item in the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfProfileIterGotoNext
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC *iterPtr
);



/**
* @internal prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterInit function
* @endinternal
*
* @brief   Set user speed conversion table list iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         This function inits iterator
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterInit
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STC *iterPtr
);

/**
* @internal prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterReset function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function resets iterator to beginning of the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterReset
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STC *iterPtr
);


/**
* @internal prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterGetCur function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function returns current item
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  - pointer to iterator
* @param[in,out] iterPtr                  - pointer to iterator
*
* @param[out] devNumPtr                - device number
* @param[out] portGroupIdxPtr          - port group
* @param[out] portGroupPortSpeed2SliceNumPtrPtr - pointer to slice conversion table
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - all elements were passed already
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterGetCur
(
    INOUT  PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STC *iterPtr,
    OUT    GT_U8  *devNumPtr,
    OUT    GT_U32 *portGroupIdxPtr,
    OUT    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC **portGroupPortSpeed2SliceNumPtrPtr
);


/**
* @internal prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterGotoNext function
* @endinternal
*
* @brief   Set user profile iterator
*         traverse over list of profiles/speed conv tables
*         returns dev, portGroup, profile or speed conversion table
*         this function goes to next item in the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] iterPtr                  pointer to iterator
* @param[in,out] iterPtr                  pointer to iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - invalide ptr
*
* @note Usage :
*       PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC iter;
*       IterInit(&iter);
*       IterReset(&iter);
*       while (GT_OK == (rc = IterGetCur(&iter, &dev,&portGroup))
*       (
*       // user action for each received data e.g. print
*       IterGotoNext(&iter)
*       )
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfSpeed2SliceConvTblIterGotoNext
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STC *iterPtr
);


/**
* @internal cpssDxChPortPizzaArbiterIfPortStateGet function
* @endinternal
*
* @brief   Get port state(how many slices are configured)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group
* @param[in] localPortNum             - local port number
*
* @param[out] sliceNumUsedPtr          -- number of slice occupied by port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortPizzaArbiterIfPortStateGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 portGroupId,
    IN  GT_PHYSICAL_PORT_NUM localPortNum,
    OUT GT_U32              *sliceNumUsedPtr
);


/**
* @internal cpssDxChPortPizzaArbiterIfSliceStateGet function
* @endinternal
*
* @brief   Get port slice state(is slice occupied by any port)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupIdx             - port group
* @param[in] sliceID                  - sliceID
*
* @param[out] isOccupiedPtr            - whether slice is occupied by any port
* @param[out] portNumPtr               - number of port occupied the slice .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPizzaArbiterIfSliceStateGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 portGroupIdx,
    IN  GT_U32 sliceID,
    OUT GT_BOOL              *isOccupiedPtr,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);




#define UNIT_TOTAL_SLICES RxDMA_TOTAL_SLICES_CNS
#if    (UNIT_TOTAL_SLICES != RxDMA_TOTAL_SLICES_CNS)     \
    || (UNIT_TOTAL_SLICES != RxDMACTU_TOTAL_SLICES_CNS)  \
    || (UNIT_TOTAL_SLICES != TxDMA_TOTAL_SLICES_CNS)     \
    || (UNIT_TOTAL_SLICES != TxDMACTU_TOTAL_SLICES_CNS)  \
    || (UNIT_TOTAL_SLICES != BM_TOTAL_SLICES_CNS)
    #error Slices number all units shall be same
#endif

/* CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    GT_U32  totalConfiguredSlices;
    GT_U32  totalSlicesOnUnit;
    GT_BOOL strictPriorityBit;
    GT_BOOL slice_enable[UNIT_TOTAL_SLICES];
    GT_U32  slice_occupied_by[UNIT_TOTAL_SLICES];
}CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC;

/* CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    GT_U32  totalConfiguredSlices;
    GT_U32  slicesNumInGop;
    GT_BOOL slice_enable[TxQ_TOTAL_AVAIBLE_SLICES_CNS];
    GT_U32  slice_occupied_by[TxQ_TOTAL_AVAIBLE_SLICES_CNS];
    CPSS_DXCH_CPUPORT_TYPE_ENT  cpu_port_type;
    GT_U32  cpu_port_sliceId          [CPSS_PA_NUMBER_OF_CPU_SLICES+1];
    GT_BOOL cpu_port_slice_is_enable  [CPSS_PA_NUMBER_OF_CPU_SLICES+1];
    GT_U32  cpu_port_slice_occupied_by[CPSS_PA_NUMBER_OF_CPU_SLICES+1];
}CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC;

/* CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC  rxDMA;
    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC  rxDMA_CTU;
    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC  txDMA;
    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC  txDMA_CTU;
    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC  BM;
    CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC TxQ;
}CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC;

/* CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    GT_U32  totalConfiguredSlices;
    GT_U32  totalSlicesOnUnit;
    GT_BOOL workConservingBit;
    GT_BOOL pizzaMapLoadEnBit;
    GT_BOOL slice_enable[1500];
    GT_U32  slice_occupied_by[1500];
}CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC;

/* CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  rxDMA;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  txDMA;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  TxQ;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  txFIFO;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  ethFxFIFO;
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  ilknTxFIFO;
}CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC;

/* CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT                             unitList[CPSS_DXCH_PA_UNIT_MAX_E+1]; /* last item CPSS_DXCH_PA_UNIT_UNDEFINED_E */
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC  unitState[CPSS_DXCH_PA_UNIT_MAX_E];
}CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC;


/* CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC */
typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT devType;
    union
    {
        CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC    lion2;
        CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC  bc2;
        CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC     bobK;
    }devState;
}CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC;


/**
* @internal cpssDxChPortPizzaArbiterDevStateInit function
* @endinternal
*
* @brief   Init the structure CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on pizzaDeviceStatePtr is NULL
*/
GT_STATUS cpssDxChPortPizzaArbiterDevStateInit
(
    INOUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);

/**
* @internal cpssDxChPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   Lion 2,3 Pizza Arbiter State which includes state of all
*         -
*         - state of slices
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
*
* @param[out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPizzaArbiterDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);


/*******************************************************************************
* gtLion2PortPizzaArbiterIfStateDump      -- print register state from Pizza Point of view
* prvCpssDxChPortPizzaArbiterIfDump        -- print register dump
* cpssDxChPortPizzaArbiterIfTablesDump     -- print list of user tables
*    all defined in appDemo\boardConfig\gtDbLion2_96XG.c
*******************************************************************************/
/*
** GT_STATUS gtLion2PortPizzaArbiterIfStateDump(IN  GT_U8  devNum,
**                                               IN  GT_U32 portGroupId);
**
** GT_STATUS gtLion2PortPizzaArbiterIfRegistersDump(IN GT_U8  devNum,
**                                                   IN GT_U32 portGroupId);
**
** GT_STATUS cpssDxChPortPizzaArbiterIfTablesDump  ( IN GT_U8  devNum,
**                                                   IN GT_U32 portGroupId);
**
*/

/**
* @internal cpssDxChPortPizzaArbiterIfConfigSet function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssDxChPortPizzaArbiterIfConfigSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
);

/**
* @internal cpssDxChPortResourcesConfigSet function
* @endinternal
*
* @brief   Caelum port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortResourcesConfigSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

