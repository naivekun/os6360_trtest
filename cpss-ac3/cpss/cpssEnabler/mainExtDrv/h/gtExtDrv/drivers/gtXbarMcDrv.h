/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtXbarMcDrv.h
*
* @brief multicast goup device members.
*
*/


#ifndef __gtXbarMcDrvh
#define __gtXbarMcDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>


/**
* @internal extDrvAddDev2XbarMcGroup function
* @endinternal
*
* @brief   Add a device to a crossbar multicast group
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note If the packet processor vidx range is greater than the crossbar
*       fabric vidx range, the ppVidx may may need to be folded over into
*       the crossbar 'vidx' range.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvAddDev2XbarMcGroup
(
    IN GT_U16               vidx,
    IN GT_U8                devId
);


/**
* @internal extDrvRemDevFromXbarMcGroup function
* @endinternal
*
* @brief   Remove a device from a crossbar multicast group
*
* @param[in] vidx                     - TAPI multicast group index
*                                      devNum   - Prestera device number joining the multicast group
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note If the packet processor vidx range is greater than the crossbar
*       fabric vidx range, the ppVidx may may need to be folded over into
*       the crossbar 'vidx' range.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvRemDevFromXbarMcGroup
(
    IN GT_U16               vidx,
    IN GT_U8                devId
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtXbarMcDrvh */



