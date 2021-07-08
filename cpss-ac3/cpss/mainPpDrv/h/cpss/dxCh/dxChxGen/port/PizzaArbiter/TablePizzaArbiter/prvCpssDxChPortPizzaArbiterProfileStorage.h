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
* @file prvCpssDxChPortPizzaArbiterProfileStorage.h
*
* @brief CPSS user profile storage
*
* @version   8
********************************************************************************
*/

#ifndef __PRV_CPSS_DXCH_PIZZAARBITER_PROFILESTORAGE_H
#define __PRV_CPSS_DXCH_PIZZAARBITER_PROFILESTORAGE_H

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterActiverPortGroupList.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterTables.h>


typedef struct PizzaProfileStorage_STCT
{
    GT_U8                         devNum;
    PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC       portGroupList;  /* list of port greoups use the profile */
    CPSS_DXCH_PIZZA_PROFILE_STC   pizzaProfile;
    /*------------------------------------------*/
    /* usefull data structures for PizzaProfile */
    /*------------------------------------------*/
    CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC regModePizza;
    CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC extModePizza;
    CPSS_DXCH_PORT_PRIORITY_MATR_STC            portPriorityMatr;
    PortPizzaCfgSlicesListPool_STC      portPizzaCfgSliceListPool;
    /* usefull data structures for PizzaProfile */
}PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC;


/**
* @internal pizzaProfileStorageInit function
* @endinternal
*
* @brief   init pizza profile storage
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in,out] storagePtr               - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr is NULL
*/
GT_STATUS pizzaProfileStorageInit
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC *storagePtr
);

/**
* @internal pizzaProfileStorageCopy function
* @endinternal
*
* @brief   copy pizza for devNum, portGroupId into storage
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in] devNum                   - dev number
* @param[in] portGroupListPtr         - port group list for which the profile is used
* @param[in] pizzaProfilePtr          - (pointer to ) profile
* @param[in,out] storagePtr               - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS pizzaProfileStorageCopy
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC *storagePtr, 
       IN GT_U8 devNum, 
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr,
       IN CPSS_DXCH_PIZZA_PROFILE_STC              *pizzaProfilePtr
);


#define PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_PROFILE_STORAGE_POOL_SIZE_CNS 8

typedef struct PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC
{
    GT_U32                    profileStorageCur;
    GT_U32                    profileStorageRemains;
    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC   pizzaProfileStorageArr[PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_PROFILE_STORAGE_POOL_SIZE_CNS];
}PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC;

/*-------------------------------------------------------------------
**    Interface to the structure PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC
**------------------------------------------------------------------*/
/*
GT_STATUS pizzaProfileStoragePoolInit (INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC * pPoolPtr);
GT_STATUS pizzaProfileStoragePoolFind (INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC * pPoolPtr, 
                                          IN GT_U8 devNum, 
                                          IN GT_U32 portGroupId,
                                         OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC ** ppPizzaStoragePtrPtr);
GT_STATUS pizzaProfileStoragePoolAlloc(INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC * pPoolPtr,
                                         OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC ** ppPizzaStoragePtrPtr);
*/

/**
* @internal pizzaProfileStoragePoolInit function
* @endinternal
*
* @brief   init pizza profile storage pool
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS pizzaProfileStoragePoolInit 
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr
);

/**
* @internal pizzaProfileStoragePoolFind1 function
* @endinternal
*
* @brief   find pizza profile for devNum portGroupIdx
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port grouop id
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppPizzaStoragePtrPtr     - (pointer to) pointer to storage found or NULL
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_FAIL                  - not find
*/
GT_STATUS pizzaProfileStoragePoolFind1
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr, 
       IN GT_U8 devNum, 
       IN GT_U32 portGroupId,
      OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC **ppPizzaStoragePtrPtr
);

/**
* @internal pizzaProfileStoragePoolAlloc function
* @endinternal
*
* @brief   alloc place for pizza profile for devNum portGroupIdx
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] pizzaStoragePtrPtr       - pointer to where the allocated address is stored
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_NO_MORE               - no place in pool
*/
GT_STATUS pizzaProfileStoragePoolAlloc
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr,
      OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC     **pizzaStoragePtrPtr
);


typedef struct PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STCT
{
    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC * storagePoolPtr;
    GT_U32                                              curIdx;
}PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_ITER_STC;

/*-------------------------------------------------------------*/
/*  the structure to store the speed 2 slice conversion table  */
/*-------------------------------------------------------------*/
typedef struct PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STCT
{
    GT_U32                    cur;
    GT_U32                    remains;
    CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC    arr[150];
}PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC;

/*-------------------------------------------------------------------
**    Interface to the structure PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC
**------------------------------------------------------------------*/
/*
GT_STATUS PortSpeed2SliceNumPoolInit (INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC * pPool);
GT_STATUS PortSpeed2SliceNumPoolAlloc(INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC * pPool, 
                                         IN GT_U32 size, 
                                        OUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC ** pItemList);
*/
/**
* @internal portSpeed2SliceNumPoolInit function
* @endinternal
*
* @brief   init speed 2 slice convertion table storage pool
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS portSpeed2SliceNumPoolInit
(
    INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC *poolPtr
);

/**
* @internal portSpeed2SliceNumPoolAlloc function
* @endinternal
*
* @brief   alloc speed 2 slice convertion table from storage pool
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] size                     - how many elements to be allocated
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] itemListPtrPtr           - (pointer to) pointer on allocated items or NULL
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_NO_RESOURCE           - no place in pool
*/
GT_STATUS portSpeed2SliceNumPoolAlloc
(
    INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC *poolPtr, 
       IN GT_U32 size, 
      OUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC **itemListPtrPtr
);


typedef struct PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STCT
{
    GT_U8                                                  devNum;
    PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC               portGroupList;
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC   portSpeed2SlicesConvTbl;
    /**/
    PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC     portSpeed2SlicesConvPool;
}PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC;

/*
GT_STATUS SpeedConvTableStorageInit(INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC * storagePtr);
GT_STATUS SpeedConvTableStorageCopy(INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC * storagePtr, 
                                       IN GT_U8 devNum, 
                                       IN GT_U32 portGroupIdx,
                                       IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * portSpeed2SlicesConvTblPtr);
*/

/**
* @internal speedConvTableStorageInit function
* @endinternal
*
* @brief   init speed comnversion table storage (PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC)
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in,out] storagePtr               - pointer to Profile
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr is NULL
*/
GT_STATUS speedConvTableStorageInit
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC *storagePtr
);

/**
* @internal speedConvTableStorageCopy function
* @endinternal
*
* @brief   copy the speed conversion table for <dev,port group >
*         into the storage place (PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in] devNum                   - device Id
* @param[in] portGroupListPtr         - port group list
* @param[in] portSpeed2SlicesConvTblPtr - speed conversion table it self
* @param[in,out] storagePtr               - pointer to Profile
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr or portSpeed2SlicesConvTblPtr is NULL
* @retval GT_NO_RESOURCE           - not enough resources , consider increase constants
*/
GT_STATUS speedConvTableStorageCopy
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC *storagePtr, 
       IN GT_U8 devNum, 
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr,
       IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portSpeed2SlicesConvTblPtr
);


#define PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_SPEEDCONV_STORAGE_POOL_SIZE_CNS 8

typedef struct PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC
{
    GT_U32                      storageCur;
    GT_U32                      storageRemains;
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC   speedConvTableStorageArr[PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_SPEEDCONV_STORAGE_POOL_SIZE_CNS];
}PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC;

/*
GT_STATUS SpeedConvTableStoragePoolInit (INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * pPoolPtr);
GT_STATUS SpeedConvTableStoragePoolFind (INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * pPoolPtr, 
                                            IN GT_U8 devNum, 
                                            IN GT_U32 portGroupId,
                                           OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC ** ppSpeedConvTblStoragePtrPtr);
GT_STATUS SpeedConvTableStoragePoolAlloc(INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * pPoolPtr,
                                           OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC ** ppSpeedConvTblStoragePtrPtr);
*/
/**
* @internal speedConvTableStoragePoolInit function
* @endinternal
*
* @brief   init pool speed conversion table storages to empty one
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS speedConvTableStoragePoolInit
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr
);

/**
* @internal speedConvTableStoragePoolFind function
* @endinternal
*
* @brief   search in pool speed conversion table storages for table
*         for specific <dev , port group>
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] devNum                   - device Id
* @param[in] portGroupId              - port group idx
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppSpeedConvTblStoragePtrPtr - pointer to place where the pointer to found table is stored;
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS speedConvTableStoragePoolFind
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr, 
       IN GT_U8 devNum, 
       IN GT_U32 portGroupId,
      OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC **ppSpeedConvTblStoragePtrPtr
);

/**
* @internal speedConvTableStoragePoolAlloc function
* @endinternal
*
* @brief   alloc place from pool of speed conversion table storages
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppSpeedConvTblStoragePtrPtr - pointer to place where the pointer to alloced
*                                      storage is stored;
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
* @retval GT_NO_RESOURCE           - no place at pool
*/
GT_STATUS speedConvTableStoragePoolAlloc
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr,
      OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC     **ppSpeedConvTblStoragePtrPtr
);



typedef struct PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STCT
{
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * storagePoolPtr;
    GT_U32                                          curIdx;
}PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_ITER_STC;

/*--------------------------------------------------------------*/
typedef struct PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STCY
{
    GT_BOOL                                             isInit;
    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC   pizzaProfilePool;
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC       speedConvTablePool;
}PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC;

/*
GT_STATUS UserProfileSpeedConvTableStorageInit(INOUT PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC * pStoragePtr);
*/

/**
* @internal userProfileSpeedConvTableStorageInit function
* @endinternal
*
* @brief   init pools of user profile storage and speed conversion table storages to empty one
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to pool
* @param[in,out] storagePtr               - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS userProfileSpeedConvTableStorageInit
(
    INOUT PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC *storagePtr
);


/*******************************************************************************
* pizzaProfileStoragePoolGet
*
* DESCRIPTION: 
*      get user profile storage pool from global variable
*
* APPLICABLE DEVICES:
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC* pointer to storage
* 
* COMMENTS:
*    if global storage g_usedProfileSpeedConvTableStorage (invisible for user)
*         is not initialized already, it is initilized first
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC   * pizzaProfileStoragePoolGet
(
    void
); 

/*******************************************************************************
* speedConvTableStoragePoolGet
*
* DESCRIPTION: 
*      get user spped conversion storage pool from global variable
*
* APPLICABLE DEVICES:
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC* pointer to storage
* 
* COMMENTS:
*    if global storage g_usedProfileSpeedConvTableStorage (invisible for user)
*         is not initialized already, it is initilized first
*
*******************************************************************************/
PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * speedConvTableStoragePoolGet
(
    void
);


/**
* @internal prvCpssDxChPizzaArbiterSelectSlicesBySpeed function
* @endinternal
*
* @brief   selection slice number by speed
*         shall be used in cpss blocks that uses Pizza Arbiter
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group
* @param[in] localPort                - local port
* @param[in] portSpeed                - port speed
*
* @param[out] sliceNumUsedPtr          - number of slices to be used if speed is supported,  NULL othewise
*                                       GT_OK.       if speed is supported
*                                       GT_NOT_SUPPORTED  otherwise
*/
GT_STATUS prvCpssDxChPizzaArbiterSelectSlicesBySpeed
( 
    IN GT_U8               devNum,
    IN GT_U32              portGroupId,
    IN GT_U32              localPort,
    IN CPSS_PORT_SPEED_ENT portSpeed,
   OUT GT_U32             *sliceNumUsedPtr
);



#endif


