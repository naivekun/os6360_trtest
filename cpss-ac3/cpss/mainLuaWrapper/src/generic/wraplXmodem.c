/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplXmodem.c
*
* DESCRIPTION:
*       XMODEM protocol implementation
*       Basic XMODEM only (128 byte blocks, simple crc)
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

#define XMODEM_SOH  0x01
#define XMODEM_EOT  0x04
#define XMODEM_ACK  0x06
#define XMODEM_NAK  0x15
#define XMODEM_CAN  0x18
#define XMODEM_SUB  0x1a


#define XMODEM_TIMEOUT_MAX_NUM  10
#define XMODEM_TIMEOUT          3000 /* ms */

/*******************************************************************************
* wraplXmodemReceive: lua function
*
* DESCRIPTION:
*       Receive a data using XMODEM protocol
*
* INPUTS:
*       keepBinary  (optional) Don't switch stream back to linemode
*
* RETURNS:
*       string or retcode
*
* COMMENTS:
*
*******************************************************************************/
int wraplXmodemReceive(lua_State *L)
{
    IOStreamPTR IOStream;
    unsigned char buf[132];
    int timeout = 1;
    int num_timeouts = 0, rc, i;
    unsigned char seq = 1, crc;

    lua_getglobal(L, "_IOStream");
    IOStream = (IOStreamPTR)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (IOStream == NULL)
    {
        lua_pushinteger(L, GT_BAD_STATE);
        return 1;
    }

    IOStream->setTtyMode(IOStream, 2);
    lua_pushlstring(L, (char*)buf, 0);

    while (1)
    {
        if (timeout)
        {
            if (++num_timeouts > XMODEM_TIMEOUT_MAX_NUM)
            {
                lua_pop(L, 1);
                lua_pushinteger(L, GT_TIMEOUT);
                break;
            }
            buf[0] = XMODEM_NAK;
            IOStream->write(IOStream, buf, 1);
            timeout = 0;
        }
        /* receive code: SOH || EOT */
        rc = IOStream->readChar(IOStream, (char*)buf, XMODEM_TIMEOUT);
        if (rc < 0)
        {
            lua_pop(L, 1);
            lua_pushinteger(L, GT_FAIL);
            break;
        }
        if (rc == 0)
        {
            timeout = 1;
            continue;
        }
        timeout = 0;

        if (buf[0] == XMODEM_EOT)
        {
            buf[0] = XMODEM_ACK;
            IOStream->write(IOStream, buf, 1);
            break;
        }
        if (buf[0] != XMODEM_SOH)
        {
            continue;
        }
        /* SOH */
        for (i = 1; i < 132; i++)
        {
            rc = IOStream->readChar(IOStream, (char*)(buf+i), XMODEM_TIMEOUT);
            if (rc < 0)
            {
                lua_pop(L, 1);
                lua_pushinteger(L, GT_FAIL);
                timeout = 2;
                break;
            }
            if (rc == 0)
            {
                timeout = 1;
                break;
            }
        }
        if (timeout == 2)
            break;
        if (timeout)
            continue;
        /* got packet */
        /* check crc */
        for (i = 3, crc = 0; i < 131; i++)
        {
            crc += buf[i];
        }
        if (crc != buf[131])
        {
            /* bad crc, send NAK */
            timeout = 1;
            continue;
        }
        /* check seq */
        if (buf[1] + 1 == seq && buf[2] == (255-buf[1]))
        {
            num_timeouts = 0;
            buf[0] = XMODEM_ACK;
            IOStream->write(IOStream, buf, 1);
            continue;
        }
        if (seq != buf[1] || (255-seq) != buf[2])
        {
            timeout = 1;
            continue;
        }
        num_timeouts = 0;
        lua_pushlstring(L, (char*)(buf+3), 128);
        lua_concat(L, 2);
        buf[0] = XMODEM_ACK;
        IOStream->write(IOStream, buf, 1);
        seq++;
    }

    if (lua_toboolean(L, 1) == 0)
    {
        IOStream->setTtyMode(IOStream, 1);
    }
    return 1;
}

/*******************************************************************************
* wraplXmodemSend: lua function
*
* DESCRIPTION:
*       Send a data using XMODEM protocol
*
* INPUTS:
*       String
*       keepBinary  (optional) Don't switch stream back to linemode
*
* RETURNS:
*       retcode
*
* COMMENTS:
*
*******************************************************************************/
int wraplXmodemSend(lua_State *L)
{
    IOStreamPTR IOStream;
    const unsigned char *data;
    unsigned char buf[3];
    size_t len, p = 0;
    unsigned char seq = 1, crc;
    int rc, num_timeouts = 0, i;


    lua_getglobal(L, "_IOStream");
    IOStream = (IOStreamPTR)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (IOStream == NULL)
    {
        lua_pushinteger(L, GT_BAD_STATE);
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    data = (unsigned const char*)lua_tolstring(L, 1, &len);

    IOStream->setTtyMode(IOStream, 2);

    while (1)
    {
        if (p >= len)
        {
            buf[0] = XMODEM_EOT;
            IOStream->write(IOStream, (char*)buf, 1);
        }
        /* receive code: SOH || EOT */
        rc = IOStream->readChar(IOStream, (char*)buf, XMODEM_TIMEOUT);
        if (rc < 0)
        {
            lua_pushinteger(L, GT_FAIL);
            return 1;
        }
        if (rc == 0)
        {
            if (++num_timeouts > XMODEM_TIMEOUT_MAX_NUM)
            {
                lua_pushinteger(L, GT_TIMEOUT);
                return 1;
            }
            continue;
        }
        if (p >= len)
        {
            if (buf[0] == XMODEM_ACK)
            {
                /* EOT acknowledged */
                break;
            }
            if (++num_timeouts > XMODEM_TIMEOUT_MAX_NUM)
            {
                /* no ACK for EOT, assume it is OK */
                break;
            }
            continue;
        }
        num_timeouts = 0;
        if (buf[0] == XMODEM_CAN)
        {
            /* canceled */
            lua_pushinteger(L, GT_FAIL);
            return 1;
        }
        if (buf[0] != XMODEM_ACK && buf[0] != XMODEM_NAK)
            continue;
        if (buf[0] == XMODEM_ACK)
        {
            p += 128;
            if (p >= len)
                continue;
            seq++;
        }
        buf[0] = XMODEM_SOH;
        buf[1] = seq;
        buf[2] = 255-seq;
        rc = IOStream->write(IOStream, (char*)buf, 3);
        for (i = 0, crc = 0; i < 128 && p+i < len; i++)
        {
            crc += data[p+i];
        }
        rc = IOStream->write(IOStream, (char*)(data+p), i);
        /* padding */
        buf[0] = XMODEM_SUB;
        for (; i < 128; i++)
        {
            IOStream->write(IOStream, (char*)buf, 1);
            crc += buf[0];
        }
        buf[0] = crc;
        rc = IOStream->write(IOStream, (char*)buf, 1);
    }

    if (lua_toboolean(L, 2) == 0)
    {
        IOStream->setTtyMode(IOStream, 1);
    }
    lua_pushinteger(L, GT_OK);
    return 1;
}
