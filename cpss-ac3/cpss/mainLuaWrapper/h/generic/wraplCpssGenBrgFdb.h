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
* @file wraplCpssGenBrgFdb.h
*
* @brief A lua wrapper for FDB tables facility CPSS implementation
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>

#include <lua.h>


/***** declarations ********/


/*******************************************************************************
*   FDB mac address-table count statistic struct                               *
*******************************************************************************/
typedef struct{
    GT_U32      total_entries_count;
    GT_U32      free_entries_count;
    GT_U32      used_entries_count;
    GT_U32      static_entries_count;
    GT_U32      dynamic_entries_count;
    GT_U32      skipped_entries_count;
} WRAPL_CPSS_MAC_ADDRESS_TABLE_COUT_STATICTIC;


