/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file appStreamNonInteractive.c
*
* @brief Non interactive streams
*
* @version   1
********************************************************************************
*/

#include <extUtils/IOStream/IOStream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(GT_UNUSED) && defined(__GNUC__)
# define GT_UNUSED __attribute__ ((unused))
#endif
#ifndef GT_UNUSED
# define GT_UNUSED
#endif

typedef struct bufferStreamSTCT {
    IOStreamSTC common;
    char        *buf;
    int          bufLen;
    int          bufPos;
    FILE        *outfile;
} bufferStreamSTC;
static int bufferStreamDestroy(IOStreamPTR stream)
{
    bufferStreamSTC *strm = (bufferStreamSTC*)stream;
    fflush(strm->outfile);
    return 0;
}
static int genericStreamRead(IOStreamPTR stream, void* bufferPtr, int bufferLength)
{
    int i, c;
    char ch;
    for (c = 0; c < bufferLength; c++)
    {
        i = stream->readChar(stream, &ch, 0);
        if (i < 1)
            break;
        ((char*)bufferPtr)[c] = ch;
    }
    return c;
}
static int bufferStreamReadChar(IOStreamPTR stream, char* charPtr, int timeOut GT_UNUSED)
{
    bufferStreamSTC *strm = (bufferStreamSTC*)stream;
    if (strm->bufPos < strm->bufLen)
    {
        *charPtr = strm->buf[strm->bufPos++];
        return 1;
    }
    return -1;
}
static int bufferStreamWrite(IOStreamPTR stream, const void* bufferPtr, int bufferLength)
{
    bufferStreamSTC *strm = (bufferStreamSTC*)stream;
    return fwrite(bufferPtr, 1, bufferLength, strm->outfile);
}
static int bufferStreamConnected(IOStreamPTR stream)
{
    bufferStreamSTC *strm = (bufferStreamSTC*)stream;
    return (strm->bufPos < strm->bufLen) ? 1 : 0;
}
static int bufferStreamSetTtyMode(IOStreamPTR stream GT_UNUSED, int mode GT_UNUSED)
{
    return 0;
}

/**
* @internal bufferStreamInit function
* @endinternal
*
* @brief   Initialize bufferStreamSTC
*
* @retval stream                   - destination stream
*/
IOStreamPTR bufferStreamInit(
    char    *buf,
    int      len,
    FILE    *outfile
)
{
    bufferStreamSTC *stream;
    stream = (bufferStreamSTC*)malloc(sizeof(*stream));

    memset(stream, 0, sizeof(*stream));
    stream->common.destroy = bufferStreamDestroy;
    stream->common.read = genericStreamRead;
    stream->common.readChar = bufferStreamReadChar;
    stream->common.readLine = IOStreamGenericReadLine;
    stream->common.write = bufferStreamWrite;
    stream->common.writeBuf = IOStreamGenericWriteBuf;
    stream->common.writeLine = IOStreamGenericWriteLine;
    stream->common.connected = bufferStreamConnected;
    stream->common.grabStd = NULL;
    stream->common.setTtyMode = bufferStreamSetTtyMode;
    stream->common.getFd = NULL;
    stream->common.isConsole = 0;
    stream->common.flags =
        IO_STREAM_FLAG_I_CANON |
        IO_STREAM_FLAG_I_ECHO;
    stream->common.wasCR = 0;
    stream->buf = buf;
    stream->bufLen = len;
    stream->bufPos = 0;
    stream->outfile = outfile;

    return (IOStreamPTR)stream;
}




typedef struct fileStreamSTCT {
    IOStreamSTC common;
    FILE        *infile;
    FILE        *outfile;
} fileStreamSTC;
static int fileStreamDestroy(IOStreamPTR stream)
{
    fileStreamSTC *strm = (fileStreamSTC*)stream;
    fflush(strm->outfile);
    return 0;
}
static int fileStreamReadChar(IOStreamPTR stream, char* charPtr, int timeOut GT_UNUSED)
{
    fileStreamSTC *strm = (fileStreamSTC*)stream;
    if (feof(strm->infile))
        return -1;
    return (fread(charPtr, 1, 1, strm->infile) == 1) ? 1 : -1;
}
static int fileStreamWrite(IOStreamPTR stream, const void* bufferPtr, int bufferLength)
{
    fileStreamSTC *strm = (fileStreamSTC*)stream;
    return fwrite(bufferPtr, 1, bufferLength, strm->outfile);
}
static int fileStreamConnected(IOStreamPTR stream)
{
    fileStreamSTC *strm = (fileStreamSTC*)stream;
    return feof(strm->infile) ? 0 : 1;
}
static int fileStreamSetTtyMode(IOStreamPTR stream GT_UNUSED, int mode GT_UNUSED)
{
    return 0;
}

/**
* @internal fileStreamInit function
* @endinternal
*
* @brief   Initialize fileStreamSTC
*
* @retval stream                   - destination stream
*/
IOStreamPTR fileStreamInit(
    FILE    *infile,
    FILE    *outfile
)
{
    fileStreamSTC *stream;
    stream = (fileStreamSTC*)malloc(sizeof(*stream));

    memset(stream, 0, sizeof(*stream));
    stream->common.destroy = fileStreamDestroy;
    stream->common.read = genericStreamRead;
    stream->common.readChar = fileStreamReadChar;
    stream->common.readLine = IOStreamGenericReadLine;
    stream->common.write = fileStreamWrite;
    stream->common.writeBuf = IOStreamGenericWriteBuf;
    stream->common.writeLine = IOStreamGenericWriteLine;
    stream->common.connected = fileStreamConnected;
    stream->common.grabStd = NULL;
    stream->common.setTtyMode = fileStreamSetTtyMode;
    stream->common.getFd = NULL;
    stream->common.isConsole = 0;
    stream->common.flags =
        IO_STREAM_FLAG_I_CANON |
        IO_STREAM_FLAG_I_ECHO;
    stream->common.wasCR = 0;
    stream->infile = infile;
    stream->outfile = outfile;

    return (IOStreamPTR)stream;
}


