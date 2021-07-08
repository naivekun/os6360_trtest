/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <stdlib.h>

#include <gtOs/gtOsRand.h>


/************ Public Functions ************************************************/

/**
* @internal osRand function
* @endinternal
*
* @brief   Generates a pseudo-random integer between 0 and RAND_MAX
*
* @retval rValue                   - pseudo-random integer
*/
GT_32 osRand(void)
{
    return rand();
}

/**
* @internal osSrand function
* @endinternal
*
* @brief   Reset the value of the seed used to generate random numbers.
*
* @param[in] seed                     - random number  .
*                                       None.
*/
void osSrand
(
    GT_U32 seed
)
{
    srand(seed);
    return;
}





