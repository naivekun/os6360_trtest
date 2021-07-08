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
* @file noPpBoards.c
*
* @brief Includes board specific initialization definitions and data-structures.
* Boars without PP.
*
* @version   1.0.0.3
********************************************************************************
*/
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>



/*******************************************************************************
 * Local usage variables
 ******************************************************************************/
const GT_BOARD_LIST_ELEMENT   boardsList[] =
{

    {gtFpgaRhodesFaBoard,                "DB-RHODES-2FA",     /*  1,1,0,0  - Rhodes  */
     1,                                     {"Rev 0.1"}}

};

const GT_U32 boardListLen = sizeof(boardsList) / sizeof(GT_BOARD_LIST_ELEMENT);



