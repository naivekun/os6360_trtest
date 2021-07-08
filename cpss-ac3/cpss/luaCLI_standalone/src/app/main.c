#ifdef SHARED_MEMORY
#define _GNU_SOURCE
#include <gtOs/gtOsInit.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedMemory.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#else
#include <cmdFS.h>
#endif
#include <extUtils/IOStream/IOStream.h>

IOStreamPTR IOStreamCreateTTY(void);
IOStreamPTR bufferStreamInit(
    char    *buf,
    int      len,
    FILE    *outfile
);
IOStreamPTR fileStreamInit(
    FILE    *infile,
    FILE    *outfile
);
int luaCLI_EventLoop(IOStreamPTR IOStream, int interactive);

int print_number(int n)
{
    printf("print_number(%d) ret=%d\n",n,n+2);
    return n+2;
}
#ifdef SHARED_MEMORY
int multiProcessAppDemo = 1;
CPSS_OS_MEM_SET_FUNC           cmdOsMemSet = NULL;
CPSS_OS_MEM_CPY_FUNC           cmdOsMemCpy = NULL;
CPSS_OS_MEM_CMP_FUNC           cmdOsMemCmp = NULL;
CPSS_OS_STR_LEN_FUNC           cmdOsStrlen = NULL;
CPSS_OS_STR_CPY_FUNC           cmdOsStrCpy = NULL;
CPSS_OS_IO_SPRINTF_FUNC        cmdOsSprintf = NULL;
CPSS_OS_MALLOC_FUNC            cmdOsMalloc = NULL;
CPSS_OS_FREE_FUNC              cmdOsFree = NULL;
CPSS_OS_TIME_WK_AFTER_FUNC     cmdOsTimerWkAfter = NULL;

static GT_VOID * __memset
(
    IN GT_VOID * start,
    IN int       symbol,
    IN GT_U32    size
)
{
    return memset(start, symbol, size);
}
static GT_VOID * __memcpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
)
{
    if(destination == NULL || source == NULL)
        return destination;

    return memcpy(destination, source, size);
}
static GT_U32 __strlen
(
    IN const GT_VOID * source
)
{
    return strlen(source);
}
static GT_CHAR* __strcpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source
)
{
    return strcpy(dest, source);
}

static GT_32 __memcmp
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32       size
)
{
    if(str1 == NULL || str2== NULL)
        return GT_BAD_PARAM;

    return (GT_32) memcmp(str1, str2, size);
}


static int __sprintf(char * buffer, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buffer, format, args);
    va_end(args);

    return i;
}
static GT_VOID * __osMalloc
(
    IN GT_U32 size
)
{
    return malloc(size);
}
static GT_VOID __osFree
(
    IN GT_VOID* const memblock
)
{
    free(memblock);
}

/*****************************************************************************
** timespec_is_zero - returns 1 if value is zero
*****************************************************************************/
int timespec_is_zero(struct timespec *value)
{
    return (value->tv_sec == 0 && value->tv_nsec == 0) ? 1 : 0;
}

/*****************************************************************************
** timespec_gt - returns 1 if a greater than b
*****************************************************************************/
int timespec_gt(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_sec < b->tv_sec)
        return 0;
    return (a->tv_nsec > b->tv_nsec) ? 1 : 0;
}

/*****************************************************************************
** timespec_add - add value to accumulator
*****************************************************************************/
void timespec_add(struct timespec *accumulator, struct timespec *value)
{
    accumulator->tv_sec += value->tv_sec;
    accumulator->tv_nsec += value->tv_nsec;
    if (accumulator->tv_nsec >= 1000000000)
    {
        accumulator->tv_sec++;
        accumulator->tv_nsec -= 1000000000;
    }
}

/*****************************************************************************
** timespec_sub - sub b from a, delta = 0 if b >= a, otherwise delta = a - b
*****************************************************************************/
void timespec_sub(struct timespec *a, struct timespec *b, struct timespec *delta)
{
    if (timespec_gt(a,b))
    {
        delta->tv_sec = a->tv_sec - b->tv_sec;
        delta->tv_nsec = a->tv_nsec - b->tv_nsec;
        if (delta->tv_nsec < 0)
        {
            delta->tv_nsec += 1000000000;
            delta->tv_sec--;
        }
    } else
    {
        delta->tv_sec = 0;
        delta->tv_nsec = 0;
    }
}


/*****************************************************************************
** milliseconds2timespec - convert time in system ticks to timespec structure
**            ( one tick is currently implemented as ten milliseconds )
*****************************************************************************/
void milliseconds2timespec(int milliseconds, struct timespec *result)
{
    if (milliseconds < 1000)
    {
        result->tv_sec = 0;
        result->tv_nsec = milliseconds * 1000000;
    }
    else
    {
        result->tv_sec = milliseconds / 1000;
        result->tv_nsec = (milliseconds % 1000) * 1000000;
    }
}

static GT_STATUS __osTimerWkAfter
(
    IN GT_U32 mils
)
{
    struct timespec now, timeout, delta;

    if (!mils)
    {
        /*
        **  Delay of zero means yield CPU to other tasks of same priority
        **  Yield to any other task of same priority without blocking.
        */
        sched_yield();
        return GT_OK;
    }

    milliseconds2timespec(mils, &delta);

    /*
    **  Establish absolute time at expiration of delay interval
    */
    clock_gettime(CLOCK_REALTIME, &timeout);
    timespec_add(&timeout, &delta);

    /*
    **  Wait for the current time of day to reach the time of day calculated
    **  after the timeout expires.  The loop is necessary since the thread
    **  may be awakened by signals before the timeout has elapsed.
    */
    while ( !timespec_is_zero(&delta) )
    {
        /*
        **  Add a cancellation point to this loop,
        **  since there are no others.
        */
        nanosleep(&delta, NULL);

        clock_gettime(CLOCK_REALTIME, &now);
        timespec_sub(&timeout, &now, &delta);
    }

    return GT_OK;
}
int ed_main(char *filename, char *prompt, void* iostream)
{
    printf("ed not implemented in luaCLI_standalone\n");
    return 2;
}

#if 0
GT_STATUS cmdFS_tftp_put(
    IN  const char  *server,
    IN  GT_U16      port,
    IN  const char  *localfile,
    IN  const char  *remotefile,
    OUT char        *errorBuf
)
{
    strcpy(errorBuf, "TFTP not implemened in luaCLI_standalone mode");
    return GT_NOT_IMPLEMENTED;
}
GT_STATUS cmdFS_tftp_get(
    IN  const char  *server,
    IN  GT_U16      port,
    IN  const char  *localfile,
    IN  const char  *remotefile,
    OUT char        *errorBuf
)
{
    strcpy(errorBuf, "TFTP not implemened in luaCLI_standalone mode");
    return GT_NOT_IMPLEMENTED;
}
#endif

/* stubs */
GT_STATUS appDemoDxChNetRxPacketCbRegister(void)
{
    return GT_NOT_IMPLEMENTED;
}
GT_STATUS appDemoTraceHwAccessDbLine(void)
{
    return GT_NOT_IMPLEMENTED;
}
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    return (GT_VOIDFUNCPTR)dlsym(/* RTLD_DEFAULT */ ((void *) 0), (char*)funcName);
}
#endif

static char *input_buffer = NULL;
static int  input_buffer_len = 0;
static FILE *input_file = NULL;
static FILE *output_file = NULL;

void mainTask(void)
{
    IOStreamPTR stream;
    int interactive = 0;

    cmdFSinit();

    if (input_buffer)
        stream = bufferStreamInit(input_buffer, input_buffer_len, output_file);
    else if (input_file)
        stream = fileStreamInit(input_file, output_file);
    else
        stream = IOStreamCreateTTY();

    if (!stream)
        return;

    if (input_file == 0 && input_buffer == 0)
        interactive = 1;

    luaCLI_EventLoop(stream, interactive);

    stream->destroy(stream);

    osStopEngine();
}

void usage(char* argv0)
{
    printf("Usage: %s [...]\n", argv0);
    printf("Options:\n");
#ifdef GENFS_NIX
    printf("    -F _fs_root_   - luaCLI scripts directory\n");
#endif
    printf("    -c _command_   - execute _command_ and exit\n");
    printf("                     This option can be used multiple times\n");
    printf("    -f _script_    - load commands from script\n");
    printf("    -o _outfile_   - save output to file\n");
}

static struct termios tc_saved;
static void tc_restore(void)
{
    tcsetattr(0, TCSANOW, &tc_saved);
    printf("tty restored\n");
}

int main(int argc, char *argv[])
{
    int c, l;

    output_file = stdout;
    while ((c = getopt(argc, argv,
#ifdef GENFS_NIX
        "F:"
#endif
        "c:f:o:?")) != EOF)
    {
        switch (c)
        {
#ifdef GENFS_NIX
            case 'F':
                setenv("CMDFS_ROOT", optarg, 1);
                break;
#endif
            case 'f':
                if (input_file != NULL)
                {
                    fprintf(stderr, "Can't combine -f and -c options\n");
                    return 1;
                }
                if (strcmp(optarg, "-") == 0)
                {
                    input_file = stdin;
                }
                else
                {
                    input_file = fopen(optarg, "r");
                    if (!input_file)
                    {
                        perror("can't open input file");
                        return 1;
                    }
                }
                break;
            case 'c':
                if (input_file != NULL)
                {
                    fprintf(stderr, "Can't combine -f and -c options\n");
                    return 1;
                }
                l = strlen(optarg);
                input_buffer = realloc(input_buffer, input_buffer_len+l+2);
                strcpy(input_buffer+input_buffer_len,optarg);
                input_buffer_len += l;
                input_buffer[input_buffer_len++] = '\n';
                input_buffer[input_buffer_len] = 0;
                break;
            case 'o':
                if (strcmp(optarg, "-") == 0)
                {
                    output_file = stdout;
                }
                else
                {
                    output_file = fopen(optarg, "w");
                    if (!output_file)
                    {
                        perror("can't open output file");
                        return 1;
                    }
                }
                setbuf(output_file, NULL);
                break;
            case '?':
                usage(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Command line error\n");
                usage(argv[0]);
                return 1;
        }

    }
#ifdef SHARED_MEMORY
    if(shrMemInitSharedLibrary() != GT_OK)
    {
        return 1;
    }
    cmdOsMemSet = __memset;
    cmdOsMemCpy = __memcpy;
    cmdOsMemCmp = __memcmp;
    cmdOsStrlen = __strlen;
    cmdOsStrCpy = __strcpy;
    cmdOsSprintf = __sprintf;
    cmdOsMalloc = __osMalloc;
    cmdOsFree= __osFree;
    cmdOsTimerWkAfter = __osTimerWkAfter;
#endif /* SHARED_MEMORY */

    if (input_file == 0 && input_buffer == 0) /* interactive */
    {
        tcgetattr(0, &tc_saved);
        atexit(tc_restore);
    }

    if (osStartEngine(argc, (const char**)argv, "luaCLI", mainTask) != GT_OK)
        return 1;

    return 0;
}
