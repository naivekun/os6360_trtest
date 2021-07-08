/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplReadJSON.c
*
* DESCRIPTION:
*       A lua wrapper to read JSON
*       the JSON should end with \n}\n
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/IOStream/IOStream.h>
#include <lua.h>

/*******************************************************************************
* wraplReadJSON: lua function
*
* DESCRIPTION:
*       Read JSON from IOStream
*
* INPUTS:
*       filename   - string or nil
*       prompt     - optional ED command prompt
*       the JSON should end with \n}\n
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
int wraplReadJSON(lua_State *L)
{
    IOStreamPTR IOStream;
    int ret, i;
    static const struct {
        char    state;
        char    ch;
        char    newstate;
    } state_machine[] = {
        { 0, '\r', 1 }, /* \r */
        { 0, '\n', 2 }, /* \n */
        { 1, '\n', 3 }, /* \r\n */
        { 1, '}',  4 }, /* \r} */
        { 2, '}',  5 }, /* \n} */
        { 3, '}',  6 }, /* \r\n} */
        { 4, '\r',10 }, /* \r}\r */
        { 5, '\n',10 }, /* \n}\n */
        { 6, '\r', 7 }, /* \r\n}\r */
        { 7, '\n',10 }, /* \r\n}\r\n */
        {100,   0, 0 }
    };
    char buf[256], state=0;
    int p = 0;

    lua_getglobal(L, "_IOStream");
    IOStream = (IOStreamPTR)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (IOStream == NULL)
        return 0;

    IOStream->setTtyMode(IOStream, 0);
    lua_pushliteral(L, "");

    while (state != 10 && IOStream->connected(IOStream))
    {
        ret = IOStream->readChar(IOStream, buf+p, -1);
        if (ret <= 0)
            break;
        if (buf[p] == 0)
            continue; /* ignore \0 in input */
        for (i = 0; state_machine[i].state != 100; i++)
            if (state_machine[i].state == state && state_machine[i].ch == buf[p])
                break;
        state = state_machine[i].newstate;
        p++;
        if (p >= sizeof(buf))
        {
            lua_pushlstring(L, buf, p);
            lua_concat(L, 2);
            p = 0;
        }
    }
    if (p > 0)
    {
        lua_pushlstring(L, buf, p);
        lua_concat(L, 2);
    }
    IOStream->setTtyMode(IOStream, 1);
    return 1;
}
