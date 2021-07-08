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
* @file prvCpssDxChPortDynamicPAGCDAlgo.c
*
* @brief GCD (greatest common divider) algorithm
*
* @version   4
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAGCDAlgo.h>


/**
* @internal prvCpssDxChDynPAGCDAlgo function
* @endinternal
*
* @brief   GCD (greatest common divider) algorithm
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] a                        - first number
* @param[in] b                        - second number
*/
GT_U32 prvCpssDxChDynPAGCDAlgo
(
    IN GT_U32 a, 
    IN GT_U32 b
)
{
    while(b) 
    {
        /* b ^= a ^= b ^= a %= b; */   
        a %= b;
        /* exchange */
        b ^= a;
        a ^= b; 
        b ^= a;     
    }
    return a;
}

