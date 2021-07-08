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
* @file w32_cmdStreamSerial.c
*
* @brief This file contains mainCmd serial stream implementation for Win32
*
* @version   6
********************************************************************************
*/

/*
   There is a bug shown in NTDLL library (in MSSI only):
   Deadlock occured while trying to shutdown SLAN library.
   As a workaround pooling model used instead of WaitCommEvent()
*/
#define ENABLE_NTDLL_BUG_WORKAROUND

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;

#include <cmdShell/os/cmdStreamImpl.h>
#include <windows.h>
#include <gtOs/gtGenTypes.h>
#include <asicSimulation/SCIB/scib.h>

/* start protect win32 calles that uses internally semaphore ,
    this to avoid deadlock when a task in inside win32 function that takes
    semaphore and we 'lock' this task from other task that may also call win32 for
    same/other function and will be locked because win32 semaphore is not released */
/*
protect next:
    ClearCommError , ReadFile , WriteFile

*/
#define PROTECT_WIN32_CALL_MAC(win32FuncWithParameters) \
    scibAccessLock();                                   \
    win32FuncWithParameters;                            \
    scibAccessUnlock()

#define OS_DEF_BAUDRATE     115200  /* default baud rate    */

typedef struct {
    IOStreamSTC common;
    HANDLE handle;
} serialStreamSTC, *serialStreamPTR;

static int serialStreamDestroy(IOStreamPTR streamP)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;

    if (!stream)
        return -1;

    CloseHandle(stream->handle);

    cmdOsFree(stream);

    return 0;
}
static int serialStreamRead(IOStreamPTR streamP, void* bufferPtr, int bufferLength)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;
        DWORD status, bytes;
    COMSTAT stat;

    if (!stream)
        return -1;

    PROTECT_WIN32_CALL_MAC((
        ClearCommError(stream->handle, &status, &stat)));
    if (stat.cbInQue == 0)
    {
#ifdef ENABLE_NTDLL_BUG_WORKAROUND
        do
        {
            Sleep(100);
            PROTECT_WIN32_CALL_MAC((
                ClearCommError(stream->handle, &status, &stat)));
        } while (stat.cbInQue == 0);
#else /* !ENABLE_NTDLL_BUG_WORKAROUND */
        /* wait for serial event: recieve char */
        do
        {
            SetCommMask(stream->handle, EV_RXCHAR);
            WaitCommEvent(stream->handle, &status, 0);
        } while ((status & EV_RXCHAR) == 0);

        PROTECT_WIN32_CALL_MAC((
            ClearCommError(stream->handle, &status, &stat)));
        if (stat.cbInQue == 0)
        {
            return 0;
        }
#endif
    }

    bufferLength = (stat.cbInQue > (unsigned)bufferLength) ? bufferLength : stat.cbInQue;

    /* perform a read request */
    PROTECT_WIN32_CALL_MAC((
        status = ReadFile(stream->handle, bufferPtr, bufferLength, &bytes, 0)));
    if (status == 0)
    {
        switch (GetLastError())
        {
            case ERROR_COUNTER_TIMEOUT:
            case ERROR_TIMEOUT:
            case WAIT_TIMEOUT:
                return 0;
            default:
                return -1;
        }
    }

    return bytes;
}

static int serialStreamReadChar(IOStreamPTR streamP, void* charPtr, int timeOut)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;
        DWORD status, bytes;
    COMSTAT stat;

    if (!stream)
        return -1;

    PROTECT_WIN32_CALL_MAC((
        ClearCommError(stream->handle, &status, &stat)));
    if (stat.cbInQue == 0)
    {
#ifdef ENABLE_NTDLL_BUG_WORKAROUND
        do
        {
            Sleep(100);
            PROTECT_WIN32_CALL_MAC((
                ClearCommError(stream->handle, &status, &stat)));
        } while (stat.cbInQue == 0);
#else /* !ENABLE_NTDLL_BUG_WORKAROUND */
        /* wait for serial event: recieve char */
        do
        {
            SetCommMask(stream->handle, EV_RXCHAR);
            WaitCommEvent(stream->handle, &status, 0);
        } while ((status & EV_RXCHAR) == 0);

        PROTECT_WIN32_CALL_MAC((
            ClearCommError(stream->handle, &status, &stat)));
        if (stat.cbInQue == 0)
        {
            return 0;
        }
#endif
    }

    /* perform a read request */
    PROTECT_WIN32_CALL_MAC((
        status = ReadFile(stream->handle, charPtr, 1, &bytes, 0)));
    if (status == 0)
    {
        switch (GetLastError())
        {
            case ERROR_COUNTER_TIMEOUT:
            case ERROR_TIMEOUT:
            case WAIT_TIMEOUT:
                return 0;
            default:
                return -1;
        }
    }

    return bytes;
}

static int serialStreamWrite(IOStreamPTR streamP, const void* bufferPtr, int bufferLength)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;
    GT_U32 status, bytes;

    if (!stream)
        return -1;

    PROTECT_WIN32_CALL_MAC((
        status = WriteFile(stream->handle, bufferPtr, bufferLength, &bytes, 0)));
    if (status == 0)
        return -1;
    return (int)bytes;
}

static int serialStreamConnected(IOStreamPTR streamP)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;

    if (!stream)
        return -1;

    return 1;
}

static int serialGrabStd(IOStreamPTR streamP)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;

    if (!stream)
        return -1;

    SetStdHandle(STD_OUTPUT_HANDLE, stream->handle);
    SetStdHandle(STD_ERROR_HANDLE, stream->handle);

    return 1;
}

static int serialSetTtyMode(IOStreamPTR streamP, int mode)
{
    serialStreamPTR stream = (serialStreamPTR)streamP;

    if (mode == 0 || mode == 2)
    {
        /* raw mode */
        stream->common.flags &= ~(
                IO_STREAM_FLAG_O_CRLF |
                IO_STREAM_FLAG_I_CANON |
                IO_STREAM_FLAG_I_ECHO
            );
    }
    else
    {
        /* line edit mode */
        stream->common.flags |=
            IO_STREAM_FLAG_O_CRLF |
            IO_STREAM_FLAG_I_CANON |
            IO_STREAM_FLAG_I_ECHO;
    }
    return 0;
}

/**
* @internal cmdStreamSerialInit function
* @endinternal
*
* @brief   Initialize serial engine
*/
GT_STATUS cmdStreamSerialInit(void)
{
    return GT_OK;
}

/**
* @internal cmdStreamSerialFinish function
* @endinternal
*
* @brief   Close serial engine
*/
GT_STATUS cmdStreamSerialFinish(void)
{
    return GT_OK;
}

/**
* @internal cmdStreamCreateSerial function
* @endinternal
*
* @brief   Create serial port stream
*
* @param[in] devNum                   - the serial device port number (0 = COM1, ...)
*
* @retval stream                   - serial stream
*                                       NULL if error
*/
IOStreamPTR cmdStreamCreateSerial
(
    IN GT_U32 devNum
)
{
    serialStreamPTR stream;
    char pszDevice[16];
    HANDLE pHandle;
    DCB portDCB;

    /* create null terminated device port name */
    cmdOsSprintf(pszDevice, "COM%d:", devNum + 1);

    /* attempt to open device in read/write mode */
    pHandle = CreateFile(
        pszDevice,                      /* device port name         */
        GENERIC_READ | GENERIC_WRITE,   /* access read/write mode   */
        0,                              /* no sharing mode          */
        NULL,                           /* security attributes      */
        OPEN_EXISTING,
        0,                              /* serial port attributes   */
        NULL);

    /* check for successful operation */
    if (pHandle == INVALID_HANDLE_VALUE)
        return NULL;

    /* retrieve default port settings */
    portDCB.DCBlength = sizeof(DCB);
    GetCommState(pHandle, &portDCB);

    /* store custom port settings */
    portDCB.BaudRate = OS_DEF_BAUDRATE;     /* set baud rate        */
    portDCB.ByteSize = 8;                   /* set byte size: 8bit  */
    portDCB.Parity = NOPARITY;              /* set parity disable   */
    portDCB.StopBits = ONESTOPBIT;          /* set one stop bit     */
    portDCB.fBinary = TRUE;
    portDCB.fParity = TRUE;
    portDCB.fErrorChar = FALSE;
    portDCB.fNull = FALSE;
    portDCB.fAbortOnError = FALSE;
    if (!SetCommState(pHandle, &portDCB))
    {
        CloseHandle(pHandle);
        return NULL;
    }

    /* store handle and return */
    stream = (serialStreamPTR)cmdOsMalloc(sizeof(*stream));
    if (!stream)
    {
        CloseHandle(pHandle);
        return NULL;
    }
    memset(stream, 0, sizeof(*stream));
    stream->common.destroy = serialStreamDestroy;
    stream->common.read = serialStreamRead;
    stream->common.readChar = serialStreamReadChar;
    stream->common.readLine = IOStreamGenericReadLine;
    stream->common.write = serialStreamWrite;
    stream->common.writeBuf = IOStreamGenericWriteBuf;
    stream->common.writeLine = IOStreamGenericWriteLine;
    stream->common.connected = serialStreamConnected;
    stream->common.grabStd = serialGrabStd;
    stream->common.setTtyMode = serialSetTtyMode;
    stream->common.isConsole = GT_FALSE;
    stream->common.flags =
        IO_STREAM_FLAG_O_CRLF |
        IO_STREAM_FLAG_I_CANON |
        IO_STREAM_FLAG_I_ECHO;
    stream->handle = pHandle;

    return (IOStreamPTR)stream;
}


