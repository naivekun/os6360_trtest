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
* @file pxFamilyBoards.c
*
* @brief Includes board specific initialization definitions and data-structures.
* Px Family board types.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
/*******************************************************************************
 * External variables
 ******************************************************************************/
#define EMPTY_BOARD ""
#define EMPTY_LINE  {NULL , EMPTY_BOARD ,1,{""}}

#define EMPTY_4_LINES \
        EMPTY_LINE,   \
        EMPTY_LINE,   \
        EMPTY_LINE,   \
        EMPTY_LINE

#define EMPTY_8_LINES \
    EMPTY_4_LINES, \
    EMPTY_4_LINES

#define EMPTY_16_LINES \
    EMPTY_8_LINES, \
    EMPTY_8_LINES

#define EMPTY_32_LINES \
    EMPTY_16_LINES, \
    EMPTY_16_LINES

/*******************************************************************************
 * Local usage variables
 ******************************************************************************/
#ifndef CHX_FAMILY
#ifdef PX_FAMILY
GT_BOARD_LIST_ELEMENT   boardsList[] =
{
    /*NOTE: this is place HOLDER for the DXCH initialization
      in the DXCH image. */
    EMPTY_32_LINES,

    {gtDbPxPipeBoardReg      ,"PX-Pipe"     ,3,{"12*10G + 4*25G",
#ifdef REFERENCEHAL_BPE_EXISTS
                                                "12*10G + 4*25G: 802.1BR reference HAL"
#else
                                                GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS
#endif
                                                ,"SMI driver, SMI devId==5"
                                           }},  /* 33,x */
    {gtRdPxPipeBoard4DevsReg ,"PX-Pipe * 4 (RD board)" ,1,{"4 devs each 12*10G + 4*25G"
                                           }},  /* 34,x */
    EMPTY_LINE,
    EMPTY_LINE
};
const GT_U32 boardListLen = sizeof(boardsList) / sizeof(GT_BOARD_LIST_ELEMENT);
#endif
#endif



