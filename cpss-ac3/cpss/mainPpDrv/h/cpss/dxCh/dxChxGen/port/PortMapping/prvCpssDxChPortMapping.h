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
* @file prvCpssDxChPortMapping.h
*
* @private Port Mapping functions
*
* @version   1
********************************************************************************
*/

#ifndef __PRV_CPSS_DXCH_PORTMAPPING_H
#define __PRV_CPSS_DXCH_PORTMAPPING_H

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* prvCpssDxChPortPhysicalPortMapGet
*
* DESCRIPTION:
*       get ports mapping
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* INPUTS:
*       devNum   - device number
*       physicalPortNum - physical port to bring
*
* OUTPUTS:
*       portMapPtr  - pointer to ports mapping
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum
*       GT_BAD_PTR      - on bad pointer
*       GT_FAIL         - when cannot deliver requested number of items
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        physicalPortNum,
    OUT CPSS_DXCH_SHADOW_PORT_MAP_STC  *portMapPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__PRV_CPSS_DXCH_PORTMAPPING_H*/

