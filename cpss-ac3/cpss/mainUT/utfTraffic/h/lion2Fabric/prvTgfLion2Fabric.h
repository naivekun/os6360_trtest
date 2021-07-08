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
* @file prvTgfLion2Fabric.h
*
* @brief App Demo Lion2Fabric testing implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfLion2Fabrich
#define __prvTgfLion2Fabrich

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfLion2FabricToDev function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfLion2FabricToDev
(
    GT_VOID
);

/**
* @internal tgfLion2FabricToVidx function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToVidx
(
    GT_VOID
);

/**
* @internal tgfLion2FabricToVlan function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToVlan
(
    GT_VOID
);

/**
* @internal tgfLion2FabricToCpu function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToCpu
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfLion2Fabrich */

