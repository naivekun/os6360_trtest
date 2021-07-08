/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplZlib.c
*
* DESCRIPTION:
*       zlib wrapper
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <lua.h>
#include "../../../cpssEnabler/mainCmd/src/cmdShell/FS/zlib/zlib.h"

#define ZLIB_COMPR_BUFFER_SIZE 0x4000

static voidpf cmdFS_zcalloc (voidpf opaque, uInt items, uInt size)
{
    if (opaque) items += size - size; /* make compiler happy */
    return (voidpf)cpssOsMalloc(items * size);
}

static void cmdFS_zcfree (voidpf opaque, voidpf ptr)
{
    cpssOsFree(ptr);
    if (opaque) return; /* make compiler happy */
}

/*******************************************************************************
* wraplZlibCompress: lua function
*
* DESCRIPTION:
*       Compress data with zlib
*
* INPUTS:
*       string
*
* RETURNS:
*       string or retcode
*
* COMMENTS:
*
*******************************************************************************/
int wraplZlibCompress(lua_State *L)
{
    z_stream strm;
    Bytef *input;
    size_t input_size;
    int res;

    if (!lua_isstring(L, 1))
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    input = (Bytef *)((GT_UINTPTR)lua_tolstring(L, 1, &input_size));

    cpssOsMemSet(&strm, 0, sizeof(strm));
    strm.zalloc = cmdFS_zcalloc;
    strm.zfree = cmdFS_zcfree;
    strm.opaque = (voidpf)cpssOsMalloc(ZLIB_COMPR_BUFFER_SIZE);
    if (strm.opaque == NULL)
    {
        lua_pushinteger(L, GT_OUT_OF_CPU_MEM);
        return 1;
    }
    res = deflateInit(&strm,Z_DEFAULT_COMPRESSION);
    if (res != Z_OK)
    {
        /* don't compress */
        cpssOsFree(strm.opaque);
        lua_pushinteger(L, (res == Z_MEM_ERROR) ? GT_OUT_OF_CPU_MEM : GT_FAIL);
        return 1;
    }
    strm.next_out = (Bytef*)strm.opaque;
    strm.avail_out = ZLIB_COMPR_BUFFER_SIZE;
    strm.next_in = input;
    strm.avail_in = input_size;

    lua_pushlstring(L, NULL, 0);
    res = Z_OK;
    while (strm.avail_in || res == Z_OK)
    {
        if (strm.avail_out == 0)
        {
            lua_pushlstring(L, strm.opaque, ZLIB_COMPR_BUFFER_SIZE);
            lua_concat(L, 2);
            strm.next_out = strm.opaque;
            strm.avail_out = ZLIB_COMPR_BUFFER_SIZE;
        }
        res = deflate(&strm, strm.avail_in ? Z_NO_FLUSH : Z_FINISH);
        if (res != Z_OK && res != Z_STREAM_END)
        {
            deflateEnd(&strm);
            cpssOsFree(strm.opaque);
            lua_pushinteger(L, GT_FAIL);
            return 1;
        }
    }
    if (strm.avail_out != ZLIB_COMPR_BUFFER_SIZE)
    {
        lua_pushlstring(L, strm.opaque, ZLIB_COMPR_BUFFER_SIZE-strm.avail_out);
        lua_concat(L, 2);
    }

    deflateEnd(&strm);
    cpssOsFree(strm.opaque);

    return 1;
}

/*******************************************************************************
* wraplZlibDecompress: lua function
*
* DESCRIPTION:
*       Decompress data with zlib
*
* INPUTS:
*       string
*
* RETURNS:
*       string or retcode
*
* COMMENTS:
*
*******************************************************************************/
int wraplZlibDecompress(lua_State *L)
{
    z_stream strm;
    Bytef *input;
    size_t input_size;
    int res;

    if (!lua_isstring(L, 1))
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    input = (Bytef *)((GT_UINTPTR)lua_tolstring(L, 1, &input_size));

    cpssOsMemSet(&strm, 0, sizeof(strm));
    strm.zalloc = cmdFS_zcalloc;
    strm.zfree = cmdFS_zcfree;
    strm.opaque = (voidpf)cpssOsMalloc(ZLIB_COMPR_BUFFER_SIZE);
    if (strm.opaque == NULL)
    {
        lua_pushinteger(L, GT_OUT_OF_CPU_MEM);
        return 1;
    }
    res = inflateInit(&strm);
    if (res != Z_OK)
    {
        /* don't compress */
        cpssOsFree(strm.opaque);
        lua_pushinteger(L, (res == Z_MEM_ERROR) ? GT_OUT_OF_CPU_MEM : GT_FAIL);
        return 1;
    }
    strm.next_out = (Bytef*)strm.opaque;
    strm.avail_out = ZLIB_COMPR_BUFFER_SIZE;
    strm.next_in = input;
    strm.avail_in = input_size;

    lua_pushlstring(L, NULL, 0);
    res = Z_OK;
    while (strm.avail_in || res == Z_OK)
    {
        if (strm.avail_out == 0)
        {
            lua_pushlstring(L, strm.opaque, ZLIB_COMPR_BUFFER_SIZE);
            lua_concat(L, 2);
            strm.next_out = strm.opaque;
            strm.avail_out = ZLIB_COMPR_BUFFER_SIZE;
        }
        res = inflate(&strm, Z_SYNC_FLUSH);
        if (res != Z_OK && res != Z_STREAM_END)
        {
            inflateEnd(&strm);
            cpssOsFree(strm.opaque);
            lua_pushinteger(L, GT_FAIL);
            return 1;
        }
        if (res == Z_STREAM_END)
            break;
    }
    if (strm.avail_out != ZLIB_COMPR_BUFFER_SIZE)
    {
        lua_pushlstring(L, strm.opaque, ZLIB_COMPR_BUFFER_SIZE-strm.avail_out);
        lua_concat(L, 2);
    }

    inflateEnd(&strm);
    cpssOsFree(strm.opaque);

    return 1;
}
