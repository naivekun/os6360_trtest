/**
********************************************************************************
* @file cmdFS.h
*
* @brief file system API.
* Currently required for Lua running on board
*
* @version   5
********************************************************************************
*/
/****************************************************
* cmdFS.h
*
* DESCRIPTION:
*       file system API.
*       Currently required for Lua running on board
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/
#ifndef __cmdFS_h__
#define __cmdFS_h__

#include <extUtils/FS/genFS.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Public Functions ************************************************/

/**
* @internal cmdFSinit function
* @endinternal
*
* @brief   Initialize cmdFS, initialize built-in files
*
* @retval 0                        - on success
*/
int cmdFSinit(void);

extern genFS_STC cmdFS;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdFS_h__ */

