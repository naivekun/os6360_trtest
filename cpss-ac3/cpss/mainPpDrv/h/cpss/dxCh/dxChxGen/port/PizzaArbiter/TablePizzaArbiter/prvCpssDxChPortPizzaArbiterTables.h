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
* @file prvCpssDxChPortPizzaArbiterTables.h
*
* @brief Pizza arbiter Table definitons
*
* @version   9
********************************************************************************
*/

#ifndef __private_CPSS_DXCH_PORT_PIZZA_ARBITER_TABLES_H
#define __private_CPSS_DXCH_PORT_PIZZA_ARBITER_TABLES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h> 
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>

/**
* @internal sliceNum2PortStateStateConvertFromNum function
* @endinternal
*
* @brief   Conversion number of slices to port state
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] sliceNum                 - slice number to convert into state
*
* @param[out] portStatePtr             -   where to store teh result
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad pointer to store result
*
* @note shall be removed , currently is nonsense
*
*/
GT_STATUS sliceNum2PortStateStateConvertFromNum
(
    IN   GT_U32 sliceNum,
    OUT  PRV_CPSS_DXCH_PORT_PA_SLICE_NUM_PER_PORT *portStatePtr
);

/**
* @internal sliceNum2PortStateStateConvert2Num function
* @endinternal
*
* @brief   Conversion of port state to number of slice number
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] portState                slice number to convert into state
*
* @param[out] sliceNumPtr              where to store teh result
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad pointer to store result
*
* @note shall be removed , currently is nonsense
*
*/
GT_STATUS sliceNum2PortStateStateConvert2Num   
(
    OUT  GT_U32 *sliceNumPtr, 
    IN   PRV_CPSS_DXCH_PORT_PA_SLICE_NUM_PER_PORT portState
);


/*
//GT_STATUS PortSpeed2SliceNumInitEmpty(INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC * pSpeed2SlicePtr);
//GT_STATUS PortSpeed2SliceNumInit     (INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC * pSpeed2SlicePtr,     
//                                         IN CPSS_PORT_SPEED_ENT speed,
//                                         IN GT_U32              slicesNum);
*/
/**
* @internal portSpeed2SliceNumListInit function
* @endinternal
*
* @brief   Init speed 2 slice num list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] listPtr                  - pointer to Object
* @param[in] size                     - port group id
* @param[in,out] listPtr                  - pointer to Object
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad devNum, portGroupId
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, size is privided by user.
*       List shall be ended by CPSS_INVALID_SLICE_NUM is last item in list
*       Initially all items are initialized as empty (i.e. slices number is initialized
*       CPSS_INVALID_SLICE_NUM
*
*/
GT_STATUS portSpeed2SliceNumListInit
(
    INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC *listPtr,
    IN    GT_U32 size
);

/**
* @internal portSpeed2SliceNumListCopy function
* @endinternal
*
* @brief   Init speed 2 slice num list to another list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] listPtr                  - pointer Object (destination)
* @param[in] listSrcPtr               - pointer to src
* @param[in,out] listPtr                  - pointer Object (destination)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad devNum, portGroupId
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_INVALID_SLICE_NUM in
*       last item of list
*
*/
GT_STATUS portSpeed2SliceNumListCopy
(
    INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC *listPtr,
    IN  CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC   *listSrcPtr
);

/**
* @internal PortSpeed2SliceNumListLen function
* @endinternal
*
* @brief   return the length of the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] listPtr                  - pointer Object
* @param[in,out] listPtr                  - pointer Object
*
* @param[out] lenPtr                   - pointer to length of list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_INVALID_SLICE_NUM in
*       last item of list
*
*/
GT_STATUS PortSpeed2SliceNumListLen
(
     INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC *listPtr,
     OUT   GT_U32 *lenPtr
);

/**
* @internal portSpeed2SliceNumListFind function
* @endinternal
*
* @brief   find slices num by speed in list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] listPtr                  - pointer Object
* @param[in] portSpeed                - port speed
* @param[in,out] listPtr                  - pointer Object
*
* @param[out] sliceNumUsedPtr          - size of list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_INVALID_SLICE_NUM in
*       last item of list
*
*/
GT_STATUS portSpeed2SliceNumListFind
(
    INOUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC *listPtr,
    IN    CPSS_PORT_SPEED_ENT portSpeed,
    OUT   GT_U32      *sliceNumUsedPtr
);


/*-------------------------------------------------------------------------
** the following oprations are used with the CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC
**            - Init()
**            - Append()
**            - Find()
** 
** GT_STATUS PortGroupSpeed2SliceNumListInit  (INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * portGroupSpeed2SliceTblPtr);
** GT_STATUS PortGroupSpeed2SliceNumListAppend(INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * portGroupSpeed2SliceTblPtr,
**                                                IN GT_U32 portN,
**                                                IN CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC * portSpeed2SliceNumListPtr);
** GT_STATUS PortGroupSpeed2SliceNumListFind  (INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * portGroupSpeed2SliceTblPtr,
**                                                IN GT_U32              portNum,
**                                                IN CPSS_PORT_SPEED_ENT portSpeed,
**                                               OUT GT_U32 *  sliceNumUsedPtr);
** GT_STATUS portGroupSpeed2SliceNumListCmp   (IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * tbl1Ptr,
**                                             IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * tbl2Ptr);
**
** 
*/

/**
* @internal portGroupSpeed2SliceNumListInit function
* @endinternal
*
* @brief   Init list of port group : pointer to Speed2Slice number list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_PA_INVALID_PORT in
*       last item of list
*
*/
GT_STATUS portGroupSpeed2SliceNumListInit
(
    INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portGroupSpeed2SliceTblPtr
);

/**
* @internal portGroupSpeed2SliceNumListAppend function
* @endinternal
*
* @brief   list of port group : pointer to Speed2Slice number list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
* @param[in] portNum                  - port number
* @param[in] portSpeed2SliceNumListPtr - pointer to corresponding speed 2 slice conversion table
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_PA_INVALID_PORT in
*       last item of list
*
*/
GT_STATUS portGroupSpeed2SliceNumListAppend
(
    INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portGroupSpeed2SliceTblPtr,
    IN    GT_PORT_NUM portNum,
    IN    CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC *portSpeed2SliceNumListPtr
);

/**
* @internal portGroupSpeed2SliceNumListFind function
* @endinternal
*
* @brief   search in list of <port group : pointer to Speed2Slice number list>
*         number of sslices to be used for specific port and speed
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
* @param[in] portNum                  - port number
* @param[in] portSpeed                - speed to convert
* @param[in,out] portGroupSpeed2SliceTblPtr - pointer Object
*
* @param[out] sliceNumUsedPtr          - where to store result
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
* @retval GT_NOT_FOUND             - if not found the port or speed
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_PA_INVALID_PORT in
*       last item of list
*
*/
GT_STATUS portGroupSpeed2SliceNumListFind
(
    INOUT CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portGroupSpeed2SliceTblPtr,
    IN    GT_PORT_NUM         portNum,
    IN    CPSS_PORT_SPEED_ENT portSpeed,
    OUT   GT_U32             *sliceNumUsedPtr
);

/**
* @internal portGroupSpeed2SliceNumListCmp function
* @endinternal
*
* @brief   compare two speed to slice conversion tables and
*         returns GT_OK if equals.
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] tbl1Ptr                  - pointer to first table
* @param[in] tbl2Ptr                  - pointer to first table
*
* @retval GT_OK                    - on success, if equal
* @retval GT_BAD_PTR               - pListPtr
* @retval GT_FAIL                  - on success, if no equal
*
* @note List is array of CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC, shall be ended by CPSS_PA_INVALID_PORT in
*       last item of list
*
*/
GT_STATUS portGroupSpeed2SliceNumListCmp
(
    IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *tbl1Ptr,
    IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *tbl2Ptr
);


/*-------------------------------------------------------
**  Pizza Profile Definition
**-------------------------------------------------------
*/



/*---------------------------------------------------------------------*/
/* Port Priority Graph representation in sparse Matrix form            */
/*    for each port list of ports having priority smaller than its one */
/*    End of list is indicated by CPSS_PA_INVALID_PORT  value          */
/*    Empty list consists of CPSS_PA_INVALID_PORT value                */
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------
** GT_STATUS PortPrioritiesMatrixInit               (INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC * matrDst);
** GT_STATUS PortPrioritiesMatrixCopy               (INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC * matrDst, 
**                                                     IN CPSS_DXCH_PORT_PRIORITY_MATR_STC * matrSrc);
** GT_STATUS PortPrioritiesMatrixPortPriorityListGet(INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC * pPriorityMatr, 
**                                                    OUT CPSS_DXCH_PIZZA_PORT_ID **pPortPriorityList, 
**                                                     IN GT_PHYSICAL_PORT_NUM portNum);
** GT_STATUS PortPrioritiesListIsPriorTo            (INOUT CPSS_DXCH_PIZZA_PORT_ID *pPortPriorityList, 
**                                                    OUT GT_BOOL *p_isPrior, 
**                                                     IN GT_PHYSICAL_PORT_NUM portNum);
**-------------------------------------------------------------------------------------*/
/**
* @internal portPrioritiesMatrixInit function
* @endinternal
*
* @brief   port priority list Matrix Initialization
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] matrPtr                  - pointer Object
* @param[in,out] matrPtr                  - pointer Object
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
*
* @note Matrix is binary. It is implemented as sparse array (i.e. array of list)
*       if M(i,j) == 1 means that port i has greater priority than port j
*       and can use slices of port j
*
*/
GT_STATUS portPrioritiesMatrixInit
(
    INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC *matrPtr
);

/**
* @internal portPrioritiesMatrixCopy function
* @endinternal
*
* @brief   port priority list Matrix copy
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] matrDstPtr               - Dst Matrix
* @param[in] matrSrcPtr               - Src Matrix
* @param[in,out] matrDstPtr               - Dst Matrix
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pListPtr
* @retval GT_CREATE_ERROR          - not enough place
*
* @note Matrix is binary. It is implemented as sparse array (i.e. array of list)
*       if M(i,j) == 1 means that port i has greater priority than port j
*       and can use slices of port j
*
*/
GT_STATUS portPrioritiesMatrixCopy
(
    INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC *matrDstPtr,
    IN  CPSS_DXCH_PORT_PRIORITY_MATR_STC   *matrSrcPtr
);

/**
* @internal portPrioritiesMatrixPortPriorityListGet function
* @endinternal
*
* @brief   Get port priority list from Priority Matrix
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] priorityMatrPtr          - Dst Matrix
* @param[in] portNum                  - port num
* @param[in,out] priorityMatrPtr          - Dst Matrix
*
* @param[out] portPriorityListPtrPtr   - pointer to port priority list where result is stored
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_RANGE          - port not found
* @retval GT_BAD_PTR               - pListPtr
*
* @note Matrix is binary. It is implemented as sparse array (i.e. array of list)
*       if M(i,j) == 1 means that port i has greater priority than port j
*       and can use slices of port j
*
*/
GT_STATUS portPrioritiesMatrixPortPriorityListGet
(
    INOUT CPSS_DXCH_PORT_PRIORITY_MATR_STC *priorityMatrPtr,
       IN GT_PHYSICAL_PORT_NUM              portNum,
      OUT CPSS_DXCH_PIZZA_PORT_ID         **portPriorityListPtrPtr
);


/**
* @internal portPrioritiesListIsPriorTo function
* @endinternal
*
* @brief   Search port priority list for specific port
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portPriorityListPtr      - list
* @param[in] portNum                  - port num
* @param[in,out] portPriorityListPtr      - list
*
* @param[out] isPriorPtr               : pointer to where store the results
*
* @retval GT_OK                    - on success,
* @retval GT_FALSE                 - port not found
* @retval GT_BAD_PTR               - pListPtr
*
* @note CPSS_PA_INVALID_PORT shall be last item in the list
*
*/
GT_STATUS portPrioritiesListIsPriorTo
(
    INOUT CPSS_DXCH_PIZZA_PORT_ID *portPriorityListPtr,
       IN GT_PHYSICAL_PORT_NUM     portNum,
      OUT GT_BOOL                 *isPriorPtr
);


/*
GT_STATUS PortPizzaCfgSlicesListInit(OUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC * pizzaPtr);
GT_STATUS PortPizzaCfgSlicesListCopy(OUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC * dstPizzaPtr,
                                      IN CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC * srcPizzaPtr);
*/
/**
* @internal portPizzaCfgSlicesListInit function
* @endinternal
*
* @brief   init structure CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaPtr                 - list
* @param[in,out] pizzaPtr                 - list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pizzaPtr is NULL
*
* @note initialization to slice list for each port to CPSS_PA_INVALID_SLICE
*
*/
GT_STATUS portPizzaCfgSlicesListInit
(
    INOUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *pizzaPtr
);

/**
* @internal portPizzaCfgSlicesListCopy function
* @endinternal
*
* @brief   copy one structure into another
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] dstPizzaPtr              - destination structure
* @param[in] srcPizzaPtr              - source structure
* @param[in,out] dstPizzaPtr              - destination structure
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pizzaPtr is NULL
*
* @note initialization to slice list for each port to CPSS_PA_INVALID_SLICE
*
*/
GT_STATUS portPizzaCfgSlicesListCopy
(
    INOUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *dstPizzaPtr,
       IN CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *srcPizzaPtr
);


/*--------------------------------------------------------------------*/
/* PortPizzaCfgSlicesListPool - Allocator                                         */
/*--------------------------------------------------------------------*/

#define PortPizzaCfgSlicesListPoolSIZE_CNS  20
typedef struct PortPizzaCfgSlicesListPool_STC
{
    GT_U32                      portPizzaCfgCur;
    GT_U32                      portPizzaCfgRemains;
    CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC  portPizzaCfgArr[PortPizzaCfgSlicesListPoolSIZE_CNS];
}PortPizzaCfgSlicesListPool_STC;
/*
GT_STATUS PortPizzaCfgSlicesListPoolInit (INOUT PortPizzaCfgSlicesListPool_STC * poolPtr);
GT_STATUS PortPizzaCfgSlicesListPoolAlloc(INOUT PortPizzaCfgSlicesListPool_STC * poolPtr, 
                                            OUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC ** pPizzaPtrPtr);
*/

/**
* @internal portPizzaCfgSlicesListPoolInit function
* @endinternal
*
* @brief   init structure PortPizzaCfgSlicesListPool_STC
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
GT_STATUS portPizzaCfgSlicesListPoolInit
(
    INOUT PortPizzaCfgSlicesListPool_STC *poolPtr
);

/**
* @internal portPizzaCfgSlicesListPoolAlloc function
* @endinternal
*
* @brief   init structure PortPizzaCfgSlicesListPool_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] pPizzaPtrPtr             : pointer to where store the results
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS portPizzaCfgSlicesListPoolAlloc
(
    INOUT PortPizzaCfgSlicesListPool_STC                *poolPtr, 
      OUT CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC **pPizzaPtrPtr
);


/*--------------------------------------------------------------------*/
/* Pizza : Pizza Config for each possible number of slices            */
/*                          port priority graph                       */
/*--------------------------------------------------------------------*/
/*
GT_STATUS PortPizzaCfgSliceListBySlicesNumInit(INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaDstPtr,
                                                  IN GT_U32 portNum,
                                                  IN GT_U32 totalSlices);

GT_STATUS PortPizzaCfgSliceListBySlicesNumAppend(INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaDstPtr, 
                                                    IN GT_U32 sliceNum2Config,
                                                    IN CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC * pizzaPtr);

GT_STATUS PortPizzaCfgSliceListBySlicesNumCopy(INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaDstPtr,
                                                  IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaSrcPtr); 

GT_STATUS PortPizzaCfgSliceListBySlicesNumCopy_wAlloc(INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaDstPtr,
                                                         IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaSrcPtr,
                                                         IN PortPizzaCfgSlicesListPool_STC * pPoolPtr);

GT_STATUS PortPizzaCfgSliceListBySlicesNumGet(INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizza, 
                                                OUT CPSS_DXCH_PIZZA_SLICE_ID **          ppSliceList,
                                                 IN GT_U32                     portNum,
                                                 IN PRV_CPSS_DXCH_PORT_PA_SLICE_NUM_PER_PORT         sliceNum2Config);
*/

/**
* @internal portPizzaCfgSliceListBySlicesNumInit function
* @endinternal
*
* @brief   init structure CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaConfPtr             - list
* @param[in] portNum                  - number of ports
* @param[in] totalSlices              - number of slices
* @param[in,out] pizzaConfPtr             : list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS portPizzaCfgSliceListBySlicesNumInit
(
    INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pizzaConfPtr,
       IN GT_PORT_NUM portNum,
       IN GT_U32 totalSlices
);

/**
* @internal portPizzaCfgSliceListBySlicesNumAppend function
* @endinternal
*
* @brief   append to list an item
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaConfPtr             - pizza configuration
* @param[in] sliceNum2Config          - number of slice
* @param[in] pizzaPtr                 - pointer to prtial Pizza
* @param[in,out] pizzaConfPtr             - pizza configuration
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
* @retval GT_CREATE_ERROR          - on error
*/
GT_STATUS portPizzaCfgSliceListBySlicesNumAppend
(
    INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pizzaConfPtr,
       IN GT_U32 sliceNum2Config,
       IN CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *pizzaPtr
);

/**
* @internal portPizzaCfgSliceListBySlicesNumCopy function
* @endinternal
*
* @brief   copy one list into another
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pPizzaDstPtr             - list dst
* @param[in] pPizzaSrcPtr             - list src
* @param[in,out] pPizzaDstPtr             : pointer to where store the results
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS portPizzaCfgSliceListBySlicesNumCopy
(
    INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pPizzaDstPtr,
       IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pPizzaSrcPtr
);

/**
* @internal portPizzaCfgSliceListBySlicesNumCopy_wAlloc function
* @endinternal
*
* @brief   copy one list into another with allocation from pool
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pPizzaDstPtr             - destination
* @param[in] pPizzaSrcPtr             - src
* @param[in] pPoolPtr                 - pool from which list is allocated
* @param[in,out] pPizzaDstPtr             : pointer to where store the results
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS portPizzaCfgSliceListBySlicesNumCopy_wAlloc
(
    INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pPizzaDstPtr,
       IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pPizzaSrcPtr,
       IN PortPizzaCfgSlicesListPool_STC *pPoolPtr
);


/**
* @internal portPizzaCfgSliceListBySlicesNumGet function
* @endinternal
*
* @brief   get list of slices to be used by port when alloced
*         specific number of slices
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaPtr                 - pointer to pizza
* @param[in] portNum                  - port Index
* @param[in] sliceNum2Config          - number of slices
* @param[in,out] pizzaPtr                 : pointer to where store the results
*
* @param[out] ppSliceListPtrPtr        - pointer to slice list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_BAD_VALUE             - port is out of range
* @retval GT_NOT_SUPPORTED         - the specified slice number configuration is not supported
*/
GT_STATUS portPizzaCfgSliceListBySlicesNumGet
(
    INOUT CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *pizzaPtr,
      OUT CPSS_DXCH_PIZZA_SLICE_ID                       **ppSliceListPtrPtr,
       IN GT_PORT_NUM                                      portNum,
       IN PRV_CPSS_DXCH_PORT_PA_SLICE_NUM_PER_PORT         sliceNum2Config
);


/*---------------------------------------------------*/
/* Iterator for */
/*---------------------------------------------------*/
typedef struct PortPizzaCfgSliceNum2SlicesListIter_STC
{
    CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPortPizzaCfgSliceNum2SlicesListPtr;
    SliceNumXPortPizzaCfgSlicesList_STC             * pSliceNumXPortPizzaCfgSlicesListPtr;
}PortPizzaCfgSliceNum2SlicesListIter_STC;

/**
* @internal portPizzaCfgSliceNum2SlicesListIterInit function
* @endinternal
*
* @brief   init iterator of port's slices list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pIterPtr                 - pointer to iterator
* @param[in] portPizzaCfgSliceNum2SlicesListPtr - pointer to list
* @param[in,out] pIterPtr                 - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*
* @note Usage :
*       PortPizzaCfgSliceNum2SlicesListIter_STC iter;
*       SliceNumXPortPizzaCfgSlicesList_STC  pSliceNumXPortPizzaCfgSlicesListPtr;
*       PortPizzaCfgSliceNum2SlicesListIterInit(&iter);
*       PortPizzaCfgSliceNum2SlicesListIterReste(&iter);
*       while (GT_OK == (rc = PortPizzaCfgSliceNum2SlicesListIterCurGet(&iter,
*       &pSliceNumXPortPizzaCfgSlicesListPtr)
*       (
*       // User action
*       PortPizzaCfgSliceNum2SlicesListIterGotoNext(&iter);
*       )
*
*/
GT_STATUS portPizzaCfgSliceNum2SlicesListIterInit
(
    INOUT PortPizzaCfgSliceNum2SlicesListIter_STC         *pIterPtr, 
       IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *portPizzaCfgSliceNum2SlicesListPtr
);

/**
* @internal portPizzaCfgSliceNum2SlicesListIterReset function
* @endinternal
*
* @brief   reset the iterator to initial state
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pIterPtr                 - pointer to iterator
* @param[in,out] pIterPtr                 - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*
* @note Usage :
*       PortPizzaCfgSliceNum2SlicesListIter_STC iter;
*       SliceNumXPortPizzaCfgSlicesList_STC  pSliceNumXPortPizzaCfgSlicesListPtr;
*       PortPizzaCfgSliceNum2SlicesListIterInit(&iter);
*       PortPizzaCfgSliceNum2SlicesListIterReste(&iter);
*       while (GT_OK == (rc = PortPizzaCfgSliceNum2SlicesListIterCurGet(&iter,
*       &pSliceNumXPortPizzaCfgSlicesListPtr)
*       (
*       // User action
*       PortPizzaCfgSliceNum2SlicesListIterGotoNext(&iter);
*       )
*
*/
GT_STATUS portPizzaCfgSliceNum2SlicesListIterReset
(
    INOUT PortPizzaCfgSliceNum2SlicesListIter_STC *pIterPtr
);

/**
* @internal portPizzaCfgSliceNum2SlicesListIterCurGet function
* @endinternal
*
* @brief   get current item of the list
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pIterPtr                 - pointer to iterator
* @param[in,out] pIterPtr                 - pointer to iterator
*
* @param[out] ppSliceNumXPortPizzaCfgSlicesListPtrPtr = (pointer to)current slice list
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*
* @note Usage :
*       PortPizzaCfgSliceNum2SlicesListIter_STC iter;
*       SliceNumXPortPizzaCfgSlicesList_STC  pSliceNumXPortPizzaCfgSlicesListPtr;
*       PortPizzaCfgSliceNum2SlicesListIterInit(&iter);
*       PortPizzaCfgSliceNum2SlicesListIterReste(&iter);
*       while (GT_OK == (rc = PortPizzaCfgSliceNum2SlicesListIterCurGet(&iter,
*       &pSliceNumXPortPizzaCfgSlicesListPtr)
*       (
*       // User action
*       PortPizzaCfgSliceNum2SlicesListIterGotoNext(&iter);
*       )
*
*/
GT_STATUS portPizzaCfgSliceNum2SlicesListIterCurGet
(
    INOUT PortPizzaCfgSliceNum2SlicesListIter_STC *pIterPtr,
      OUT SliceNumXPortPizzaCfgSlicesList_STC    **ppSliceNumXPortPizzaCfgSlicesListPtrPtr
);

/**
* @internal portPizzaCfgSliceNum2SlicesListIterGotoNext function
* @endinternal
*
* @brief   goto to the nxt item
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pIterPtr                 - pointer to iterator
* @param[in,out] pIterPtr                 - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*
* @note Usage :
*       PortPizzaCfgSliceNum2SlicesListIter_STC iter;
*       SliceNumXPortPizzaCfgSlicesList_STC  pSliceNumXPortPizzaCfgSlicesListPtr;
*       PortPizzaCfgSliceNum2SlicesListIterInit(&iter);
*       PortPizzaCfgSliceNum2SlicesListIterReste(&iter);
*       while (GT_OK == (rc = PortPizzaCfgSliceNum2SlicesListIterCurGet(&iter,
*       &pSliceNumXPortPizzaCfgSlicesListPtr)
*       (
*       // User action
*       PortPizzaCfgSliceNum2SlicesListIterGotoNext(&iter);
*       )
*
*/
GT_STATUS portPizzaCfgSliceNum2SlicesListIterGotoNext
(
    INOUT PortPizzaCfgSliceNum2SlicesListIter_STC *pIterPtr
);


/**
* @internal pizzaProfileInit function
* @endinternal
*
* @brief   init the pizza Profile to empty one
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pPizzaProfilePtr         - pointer to Profile
* @param[in,out] pPizzaProfilePtr         - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS pizzaProfileInit
(
    INOUT CPSS_DXCH_PIZZA_PROFILE_STC *pPizzaProfilePtr
);



/*---------------------------------------------------------------------------*/
/* List : pizza for specific <devices type, revision, core clock             */
/*---------------------------------------------------------------------------*/

typedef struct DevRevTypeClockPizza_STC
{
    /* key */
    CPSS_PP_FAMILY_TYPE_ENT                         devFamilyType;
    GT_U32                                          devRevision;
    GT_U32                                          devCoreClock;  /* if devClock == 0 does not matter what clock is*/ 
    /* data */
    CPSS_DXCH_PIZZA_PROFILE_STC                             *  pizzaProfilePtr; 
}DevRevTypeClockPizza_STC;

typedef struct ListDevRevTypeClockPizza_STC
{
    GT_U32 size;
    DevRevTypeClockPizza_STC * pArr;
}ListDevRevTypeClockPizza_STC;

/*
** GT_STATUS ListDevRevPizza_Get(INOUT ListDevRevTypeClockPizza_STC       *   pListPtr,
**                                     OUT CPSS_DXCH_PIZZA_PROFILE_STC     **  ppPizzaProfilePtr,
**                                     IN  CPSS_PP_FAMILY_TYPE_ENT  devFamilyType,
**                                     IN  GT_U32                   devRevision,
**                                     IN  GT_U32                   devCoreClock
**                              );
*/

/**
* @internal prvCpssDxChPizzaArbiterSystemProfileGet function
* @endinternal
*
* @brief   Select Pizza Definition by PP Family type
*         revision
*         dev clock
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devFamilyType            - device family
* @param[in] devRevision              - device revison
* @param[in] devCoreClock             - clock speed in MHz
*
* @param[out] ppPizzaProfilePtrPtr     (ppPizzaDefPtr == pizza if device suppoted)
*                                      NULL  otherwise
*                                       GT_OK.       if suitable pizza found
*                                       GT_NOT_SUPPORTED  if suitable pizza not found
*/
GT_STATUS prvCpssDxChPizzaArbiterSystemProfileGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT    devFamilyType,
    IN  GT_U32                     devRevision,
    IN  GT_U32                     devCoreClock,
    OUT CPSS_DXCH_PIZZA_PROFILE_STC **ppPizzaProfilePtrPtr
);


/*--------------------------------*/
/* default Profiles               */
/*--------------------------------*/
extern const CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC prvPortGroupPortSpeed2SliceNumDefault;
extern const ListDevRevTypeClockPizza_STC    prvListDevRevTypeClock2PizzaDefault;

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif



