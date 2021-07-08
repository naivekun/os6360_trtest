/*******************************************************************************
*              (c), Copyright 2007, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cmdStream.c
*
* DESCRIPTION:
*       This file contains common functions for mainCmd I/O streams
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/
#include <extUtils/IOStream/IOStream.h>
#ifdef SHARED_MEMORY
#include <cmdShell/os/cmdStream.h>
#else
#include <cmdStream.h>
#endif
#include <string.h>

/************* generic methods implementation *********************************/

/* ascii codes */
#define BKSP        (0x08)  /* back space character         */
#define DEL         (0x7f)  /* back space character         */
#define XON_CHAR_CNS   17 /*0x11*/
#define XOFF_CHAR_CNS  19 /*0x13*/

/**
* @internal streamGenericReadLine function
* @endinternal
*
* @brief   Generic readLine method for cmdStreamSTC
*         Read line from stream. Perform input editing/translations
*         Returned line is zero terminated with stripped EOL symbols
*/
int streamGenericReadLine(cmdStreamPTR streamP, char* bufferPtr, int bufferLength)
{
    int k, p;
    char ch;
    p = 0;

    if (!streamP)
        return -1;

    while (streamP->connected(streamP) && p < bufferLength)
    {
        k = streamP->read(streamP, &ch, 1);
        if (k < 0)
        {
            return -1;
        }
        if (k == 0)
            continue;
        if (ch == '\n')
        {
            if (streamP->flags & STREAM_FLAG_I_CANON)
            {
                if (streamP->wasCR)
                {
                    streamP->wasCR = 0;
                    continue;
                }
                if (streamP->flags & STREAM_FLAG_I_ECHO)
                {
                    streamP->writeBuf(streamP, "\r\n", 2);
                }
            }
            break;
        }
        streamP->wasCR = 0;
        if (streamP->flags & STREAM_FLAG_I_CANON)
        {
            switch (ch) {
                case '\0':
                    break;

                case BKSP:
                case DEL:
                    if (p)
                    {
                        p--;
                        if (streamP->flags & STREAM_FLAG_I_ECHO)
                        {
                            streamP->write(streamP, "\b \b", 3);
                        }
                    }
                    break;
                case '\r':
                    streamP->wasCR = 1;
                    if (streamP->flags & STREAM_FLAG_I_ECHO)
                    {
                        streamP->write(streamP, "\r\n", 2);
                    }
                    break;
                case XON_CHAR_CNS:
                case XOFF_CHAR_CNS:
                    /* characters send from the other size when , we send it
                       large amount of characters during 'osPrintf' */
                    /* we need to ignore those characters */
                    break;
                default:
                    ((char*)bufferPtr)[p++] = ch;
                    if (streamP->flags & STREAM_FLAG_I_ECHO)
                    {
                        streamP->write(streamP, &ch, 1);
                    }
            }
            if (streamP->wasCR)
                break;
        } else {
            if (ch != '\r')
            {
                ((char*)bufferPtr)[p++] = ch;
                if (streamP->flags & STREAM_FLAG_I_ECHO)
                {
                    streamP->write(streamP, &ch, 1);
                }
            }
        }
    }
    if (p < bufferLength)
    {
        bufferPtr[p] = 0;
    }
    return p;

    /* set input mode */
}

/**
* @internal streamGenericWriteBuf function
* @endinternal
*
* @brief   Generic writeBuf method for cmdStreamSTC
*         Write to stream. Perform output translations
*/
int streamGenericWriteBuf(cmdStreamPTR streamP, const char *s, int len)
{
    const char *p, *e;
    int result = 0;
    int ret;

    if (!streamP)
        return -1;

    if (!(streamP->flags & STREAM_FLAG_O_CRLF))
    {
        return streamP->write(streamP, (char*)s, len);
    }

    e = s + len;
    for (p = s; p < e; p++)
    {
        if (*p == '\n')
        {
            if (p != s)
            {
                ret = streamP->write(streamP, (char*)s, (int)(p-s));
                if (ret < 0)
                    return result ? result : -1;
                result += ret;
            }
            if (streamP->write(streamP, "\r", 1) < 0)
                return result ? result : -1;
            s = p;
        }
    }
    if (p != s)
    {
        ret = streamP->write(streamP, (char*)s, (int)(p-s));
        if (ret < 0)
            return result ? result : -1;
        result += ret;
    }
    return result;
}

/**
* @internal streamGenericWriteLine function
* @endinternal
*
* @brief   Generic writeLine method for cmdStreamSTC
*         Write line to stream. Perform output translations
*/
int streamGenericWriteLine(cmdStreamPTR streamP, const char *s)
{
    if (!streamP)
        return -1;

    return streamP->writeBuf(streamP, s, strlen(s));
}

