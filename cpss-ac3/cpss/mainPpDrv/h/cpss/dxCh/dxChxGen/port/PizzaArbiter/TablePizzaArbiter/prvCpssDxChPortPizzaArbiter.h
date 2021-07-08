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
* @file prvCpssDxChPortPizzaArbiter.h
*
* @brief define Pizza Arbiter for Lion2 A0/B0
*
*
* @version   6
********************************************************************************
*/

#ifndef __private_CPSS_DXCH_PORT_PIZZA_ARBITER_H
#define __private_CPSS_DXCH_PORT_PIZZA_ARBITER_H

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterDevInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*----------------------------------------------------------------------------------------------*/
/* Table for selection of HW interface function for different devices                            */
/* shall be used in cpss blocks that uses Pizza Arbiter                                         */
/*----------------------------------------------------------------------------------------------*/
typedef struct DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STC
{
    CPSS_PP_FAMILY_TYPE_ENT                         devFamilyType;
    GT_U32                                          devRevision;
    /* */
    PRV_CPSS_DXCH_PORT_PIZZA_SLICE_MODIF_FUN_STC                        sliceManipuldFun;
}DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STC;

typedef struct LIST_DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STCT
{
    GT_U32                           size;
    DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STC * pArr;
}LIST_DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STC;

/**
* @internal prvCpssDxChDevRevTypeSliceManipulFunGet function
* @endinternal
*
* @brief   select slice manipulation function set from the table by devFamilyType and revision
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] listPtr                  - list of functions' sets.
* @param[in] devFamilyType            - device family
* @param[in] devRevision              - device reviosion
*
* @param[out] sliceManipulFunPtrPtr    - (pointer to ) pointer to slice manipulation function set
*                                       GT_OK.       on sucess
*                                       GT_BAD_PTR     on bad pointer
*                                       GT_NOT_SUPPORTED  on suitable set not found
*/
GT_STATUS  prvCpssDxChDevRevTypeSliceManipulFunGet
(
    IN  LIST_DEV_REV_TYPE_TO_SLICE_MANIPUL_FUN_STC *listPtr,
    IN  CPSS_PP_FAMILY_TYPE_ENT  devFamilyType,
    IN  GT_U32                   devRevision,
    OUT PRV_CPSS_DXCH_PORT_PIZZA_SLICE_MODIF_FUN_STC  **sliceManipulFunPtrPtr
);

/*-------------------------------------------------------------------------------------
**
**             +------------------------------------------------+     
**             |  Interface                                     |     knows nothing about 
**             |    -prvCpssDxChPizzaArbiterInit                |          - whether port group is active or not  
**             |    -prvCpssDxChPizzaArbiterPortSetState        |          - How to use pizza profile
**             |    -prvCpssDxChPizzaArbiterPortGetState        |          - How manu units there are in systems
**             |    -prvCpssDxChPizzaArbiterSliceGetState       |     
**             +------------------------------------------------+     
**                     |                       
**                    \|/
**            +-------------------------------------------------+       
**            |  PizzaArbiterDevice  (has several port group)   |
**            |    -PizzaArbiterDevInit                         |
**            |    -PizzaArbiterDevPortSetState                 |               +----------------------------------+
**            |    -PizzaArbiterDevPortGetState                 |-------------->|  Active PortGroup List           |
**            |    -PizzaArbiterDevSliceGetState                |               |     - ActivePortGroupListSearch  |
**            +-------------------------------------------------+               +----------------------------------+
**                     |                       
**                    \|/
**            +-------------------------------------------------+                        
**            |  PizzaArbiterDevPortGroup                       |--+                        
**            |    -PizzaArbiterDevPortGroupInit                |  |            +----------------------------------+         
**            |    -PizzaArbiterDevPortGroupPortSetState        |  |----------->|  PizzaProfile                    |    
**            |    -PizzaArbiterDevPortGroupPortGetState        |  |            |         - GetPizzaByMode         |
**            |    -PizzaArbiterDevPortGroupSliceGetState       |  |            |                                  |
**            +-------------------------------------------------+  |            +----------------------------------+
**                +------------------------------------------------+                   
**                     |                       
**                     |                       
**                     |                       
**                    \|/
**            +-------------------------------------------------+                        
**            |  Slice Manipulation Function                    |
**            |    -Init                                        |        
**            |    -SliceOccupy                                 |        
**            |    -SliceRelease                                |---------------------+   
**            |    -SliceGetState                               |                     |
**            |    -PizzaArbiterDevPortGroupSliceGetState       |----+                |
**            +-------------------------------------------------+    |                |
**                     |            |            |          |        |                |
**                    \|/           |           \|/         |       \|/               |
**            +-------------------+ | +-------------------+ | +-------------------+   |
**            |  RxDMA            | | |  RxDMA CTU        | | |  TxDMA            |   |
**            |    -Init          | | |    -Init          | | |    -Init          |   |
**            |    -SlicesNumSet  | | |    -SlicesNumSet  | | |    -SlicesNumSet  |   |
**            |    -SlicesNumGet  | | |    -SlicesNumGet  | | |    -SlicesNumGet  |   |
**            |    -SliceOccupy   | | |    -SliceOccupy   | | |    -SliceOccupy   |   |
**            |    -SliceRelease  | | |    -SliceRelease  | | |    -SliceRelease  |   |
**            |    -SliceGetState | | |    -SliceGetState | | |    -SliceGetState |   |
**            +-------------------+ | +-------------------+ | +-------------------+   |
**                                  |                       |                         |
**                                 \|/                     \|/                       \|/
**                      +-------------------+   +-------------------+   +-------------------+  In Lion2 B0 the number of slices is doubled 
**                      |  TxDMA CTU        |   |  BM               |   |  TxQ              |  against original pizza. But ONLY TxQ knows 
**                      |    -Init          |   |    -Init          |   |    -Init          |  about that !!!
**                      |    -SlicesNumSet  |   |    -SlicesNumSet  |   |    -SlicesNumSet  |
**                      |    -SlicesNumGet  |   |    -SlicesNumGet  |   |    -SlicesNumGet  |
**                      |    -SliceOccupy   |   |    -SliceOccupy   |   |    -SliceOccupy   |
**                      |    -SliceRelease  |   |    -SliceRelease  |   |    -SliceRelease  |
**                      |    -SliceGetState |   |    -SliceGetState |   |    -SliceGetState |
**                      +-------------------+   +-------------------+   +-------------------+
** 
** 
** 
**------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------*/
/* high level interface Pizza interface                                                     */
/*     prvCpssDxChPizzaArbiterDeviceInit()   - init device and all slices                	*/
/*     prvCpssDxChPizzaArbiterPortGetState() - returns number of slices used by port     	*/ 
/*     prvCpssDxChPizzaArbiterPortSetState() - assign to port specified number of slices 	*/
/*     prvCpssDxChPizzaArbiterSliceGetState() - returns port number if is used by some port */ 
/*------------------------------------------------------------------------------------------*/


/**
* @internal prvCpssDxChPizzaArbiterDeviceInit function
* @endinternal
*
* @brief   inteface to Pizza Arbiter :
*         DeviceInit()  init the device pizza and slices on various queues
*         PortGetState() get port state : how many slices are assigned to port
*         PortSetState() assign specified number of slices to port according to Pizza
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] pizzaDevInfoPtr          - device info required for Pizza Arbiter Operation
*
* @retval GT_OK.                   - on suceess
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPizzaArbiterDeviceInit  
(
    IN GT_U8 devNum,       
	IN PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC *pizzaDevInfoPtr
);

/**
* @internal prvCpssDxChPizzaArbiterPortGetState function
* @endinternal
*
* @brief   PortGetState() get port state : how many slices are assigned to port
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupIdx             - port group id
* @param[in] portNum                  - local port number (inside the port group)
* @param[in] pizzaDevInfoPtr          - device info required for Pizza Arbiter Operation
*
* @param[out] sliceNumUsedPtr          - sliceNumber atored at  address pSliceNumUsed
*                                       GT_OK.       operation finished successfully
*                                       GT_BAD_PTR     pizzaDevInfoPtr == NULL or pSliceNumUsed == NULL
*                                       GT_NOT_SUPPORTED  device not supprted
*/
GT_STATUS prvCpssDxChPizzaArbiterPortGetState 
(
    IN  GT_U8 devNum, 
    IN  GT_U32 portGroupIdx, 
    IN  GT_PHYSICAL_PORT_NUM portNum, 
    IN  PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC *pizzaDevInfoPtr,
    OUT GT_U32 *sliceNumUsedPtr
);


typedef enum 
{
     PA_PORT_ACTION_VERIFY,
     PA_PORT_ACTION_PERFORM
}PA_PORT_ACTION_ENT;

/**
* @internal prvCpssDxChPizzaArbiterPortSetState function
* @endinternal
*
* @brief   PortSetState() set the port state : assign slices to port
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupIdx             - port group id
* @param[in] portNum                  - local port number (inside the port group)
* @param[in] sliceNum2Use             - number of slice to
* @param[in] isExtModeEnable          - use regular mode pizza if FALSE; use extended mode pizza if TRUE
* @param[in] pizzaDevInfoPtr          - device info required for Pizza Arbiter Operation
* @param[in] action                   - perform assignment or check feasibility
*                                       GT_BAD_PTR     if pizzaDevInfoPtr is NULL
*                                       GT_NOT_SUPPORTED  if suitable pizza not found, slices nnumber to be assigned not supported for this port
*                                       GT_OK.       if operation finished by success.
*/
GT_STATUS prvCpssDxChPizzaArbiterPortSetState 
(
    IN GT_U8 devNum, 
    IN GT_U32 portGroupIdx, 
    IN GT_PHYSICAL_PORT_NUM portNum, 
    IN GT_BOOL isExtModeEnable,
    IN GT_U32  sliceNum2Use,
    IN PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC *pizzaDevInfoPtr,
    IN PA_PORT_ACTION_ENT action
);

/**
* @internal prvCpssDxChPizzaArbiterSliceGetState function
* @endinternal
*
* @brief   get state of specific slice (behind lies assumption :
*         the pizza is equal for each unit, i.e.
*         PizzaArbiter knows nothing about the differences between units
*         Differences appear at the layer Slice Manipualtion function
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupIdx             - port group id
* @param[in] sliceID                  - slice id which state to obtain
* @param[in] pizzaDevInfoPtr          - device info required for Pizza Arbiter Operation
*
* @param[out] isOccupiedPtr            : is slice occupied
* @param[out] portNumPtr               : if the slice is occupied , by which port it is occupied
*                                       GT_OK.       if suitable pizza found
*                                       GT_BAD_PTR     if any pointer is NULL
*                                       GT_NOT_SUPPORTED  if suitable pizza not found
*/
GT_STATUS prvCpssDxChPizzaArbiterSliceGetState
(
    IN  GT_U8 devNum, 
    IN  GT_U32 portGroupIdx, 
    IN  GT_U32 sliceID, 
    IN  PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC *pizzaDevInfoPtr,
    OUT GT_BOOL              *isOccupiedPtr, 
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif


