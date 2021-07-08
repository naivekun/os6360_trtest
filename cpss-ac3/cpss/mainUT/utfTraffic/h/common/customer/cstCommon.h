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
* @file cstCommon.h
*
* @brief Common definitions for customer UTs
*
* @version   2
********************************************************************************
*/
#ifndef __cstcommonh
#define __cstcommonh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
#define CHECK_RC_MAC(rc)                                                \
    {                                                                   \
        /*print the command(before execution) */                        \
            /*cpssOsPrintf("[%s]\n", */                                 \
            /*    #rc);   */                                            \
                                                                        \
        {                                                               \
            /* execute the command */                                   \
            GT_STATUS   __rc = (rc);                                    \
            if(__rc != GT_OK)                                           \
            {                                                           \
                cpssOsPrintf(" --> Error in file(%s) LINE(%d) error code(%d) \n", \
                    __FILE__,__LINE__,__rc);                            \
                return __rc;                                            \
            }                                                           \
        }                                                               \
    }

#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cstcommonh */


