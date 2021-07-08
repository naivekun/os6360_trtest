#ifndef __PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_PORT_GROUP_LIST_H
#define __PRV_CPSS_DXCH_PORT_PIZZA_ARBITER_PORT_GROUP_LIST_H

/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortPizzaArbiterActivePortGroupList.h
*
* DESCRIPTION:
*       port greoup list
*
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#define PRV_CPSS_BAD_PORTGROUP_CNS              0xFF
#define PRV_CPSS_MAX_PORT_GROUP_NUM_CNS        16

/*******************************************************************************
* PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC 
*
* DESCRIPTION:
*       list of active port group on PP of fixed length(up to max portgroup on PP 
*
* FIELDS:
*       maxSize      - max size of list
*       size         - actual size of list
*       list         - list itsels
*
* 		interfaces:
* 			ActivePortGroupListInit() -- init the list
*           		         Append() -- Append PortGroup to list
*                            Search() -- search the portGroup in the list
*
* APPLICABLE DEVICES:
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*       pPortGroupList    - pointer to PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC structure
*       portGroupId       - port group id (core Id)
*
* OUTPUTS:
*       Search()
*       	pIsFoundPtr   - GT_TRUE if port group is in list 
*                         - GT_FALSE othewise
* RETURNS:
*       Append()
*       	GT_OUT_OF_RANGE : if there is no place to store port group id
*           GT_OK.            
*       Search()
*           GT_OK.            any case 
* COMMENTS:
*
*******************************************************************************/
typedef struct PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LISTt_STCT
{
	GT_U32 size;
	GT_U32 maxSize;
	GT_U32 list[PRV_CPSS_MAX_PORT_GROUP_NUM_CNS];
}PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC;


/**
* @internal ActivePortGroupListInit function
* @endinternal
*
* @brief   The function inits PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC structure to be empty
*         list
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] portGroupListPtr         - structure to be initialized
* @param[in,out] portGroupListPtr         - structure to be initialized
*
* @retval GT_BAD_PTR               - if pPortGroupList is NULL
* @retval GT_OK                    -
*/
GT_STATUS ActivePortGroupListInit  
(
    INOUT PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr
);


/**
* @internal ActivePortGroupListCopy function
* @endinternal
*
* @brief   The function inits PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC structure to be empty
*         list
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] portGroupListPtr         - structure to be initialized
* @param[in] portGroupListFromPtr     - list to copy from
* @param[in,out] portGroupListPtr         - structure to be initialized
*
* @retval GT_BAD_PTR               - if pPortGroupList is NULL
*                                       GT_OK
*/
GT_STATUS ActivePortGroupListCopy
(
    INOUT PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr,
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListFromPtr
);

/**
* @internal ActivePortGroupListAppend function
* @endinternal
*
* @brief   The function appends pord group to list of port gropupsinits
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] portGroupListPtr         - port group list Ptr
* @param[in] portGroupId              - port group to append to the list
* @param[in,out] portGroupListPtr         - port group list Ptr
*                                       GT_BAD_PTR if pPortGroupList is NULL
*                                       GT_NO_RESOURCE if no place remains
*                                       GT_OK
*/
GT_STATUS ActivePortGroupListAppend
(
    INOUT PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr, 
       IN GT_U32 portGroupId
);


/**
* @internal ActivePortGroupListSearch function
* @endinternal
*
* @brief   The function search pord group list for specific port group
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portGroupListPtr         - port group list
* @param[in] portGroupId              - port group to append to the list
* @param[in,out] portGroupListPtr         - port group list
*
* @param[out] isFoundPtr               - pointer to boolean where result of search is stored   GT_TRUE  if found,  GT_FALSE if not found.
*                                       GT_BAD_PTR = on pPortGroupList is NULL or pIsFoundPtr is NULL
*
* @retval GT_OK                    - on sucess
*/
GT_STATUS ActivePortGroupListSearch
(
    INOUT PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr, 
       IN GT_U32   portGroupId, 
      OUT GT_BOOL *isFoundPtr
);  

/**
* @internal ActivePortGroupListBuildFromBmp function
* @endinternal
*
* @brief   The function build list of active portgroup by bitmap
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] activePortGroupListPtr   - port group list
* @param[in] portGroupBmp             - port group bitmap
* @param[in] firstActivePortGroup     - first active  port groups
* @param[in] lastActivePortGroup      - last active  port groups
* @param[in,out] activePortGroupListPtr   - pointer to boolean where result of search is stored
*                                       GT_BAD_PTR if pPortGroupList is NULL
*                                       GT_OK in any case
*
* @note if numOfPortGroups == 0 uses max available (i.e. 32 )
*
*/
GT_STATUS ActivePortGroupListBuildFromBmp
(
    INOUT PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *activePortGroupListPtr,
       IN GT_U32 portGroupBmp,
       IN GT_U32 firstActivePortGroup,
       IN GT_U32 lastActivePortGroup
);



typedef struct ActivePortGroupListIter_STCT
{
    PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC * activePortGroupListPtr;
	GT_U32 cur;
}ActivePortGroupListIter_STC;

/**
* @internal ActivePortGroupListIterInit function
* @endinternal
*
* @brief   The function build iterator over list of active portgroup
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] iterPtr                  - iterator struct
* @param[in] portGroupListPtr         - port group list
* @param[in,out] iterPtr                  - iterator struct
*
* @retval GT_BAD_PTR               -om pPortGroupList is NULL
* @retval GT_OK                    -on sucess
*/
GT_STATUS ActivePortGroupListIterInit
(
    INOUT ActivePortGroupListIter_STC              *iterPtr,
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr
);

/**
* @internal ActivePortGroupListIterReset function
* @endinternal
*
* @brief   The function resets iterator over list of active portgroup
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] iterPtr                  - iterator struct
* @param[in,out] iterPtr                  = iterator struct
*
* @retval GT_BAD_PTR               - on iterPtr is NULL
* @retval GT_OK                    - on sucess
*/
GT_STATUS ActivePortGroupListIterReset
(
    INOUT ActivePortGroupListIter_STC *iterPtr
);

/**
* @internal ActivePortGroupListIterGetCur function
* @endinternal
*
* @brief   The function get current port id
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] iterPtr                  - iterator struct
* @param[in,out] iterPtr                  - iterator struct
*
* @param[out] portIdxPtr               - (pointer to) portIdx
*
* @retval GT_BAD_PTR               - on iterPtr is NULL or portIdxPtr is NULL
* @retval GT_OK                    - on sucess
*/
GT_STATUS ActivePortGroupListIterGetCur
(
    INOUT ActivePortGroupListIter_STC *iterPtr,
      OUT GT_U32 *portIdxPtr
);

/**
* @internal ActivePortGroupListIterGotNext function
* @endinternal
*
* @brief   The function go to next item in list of active portgroup
*
* @note   APPLICABLE DEVICES:      none
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in,out] iterPtr                  - iterator struct
* @param[in,out] iterPtr                  - iterator struct
*
* @retval GT_BAD_PTR               - on iterPtr is NULL or portIdxPtr is NULL
* @retval GT_OK                    - on sucess
*/
GT_STATUS ActivePortGroupListIterGotNext
(
    INOUT ActivePortGroupListIter_STC *iterPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

