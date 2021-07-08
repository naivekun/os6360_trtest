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
* @file appDemoDragonite.h
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   29
********************************************************************************
*/
#ifndef __appDemoDragoniteh
#define __appDemoDragoniteh

#include <appDemo/boardConfig/appDemoBoardConfig.h>

/*#if defined(_linux) && !defined(ASIC_SIMULATION) && !defined(LINUX_NOKM)*/
#if defined(_linux) && !defined(ASIC_SIMULATION)
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


GT_STATUS poeTest(GT_CHAR *cmdPtr);
GT_STATUS poeGetPortLoading(
	IN	GT_U8 port,
   	OUT GT_U16 *portLoadingPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoDragoniteh */




