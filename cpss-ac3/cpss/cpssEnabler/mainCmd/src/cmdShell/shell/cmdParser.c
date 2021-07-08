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
* @file cmdParser.c
*
* @brief token reader and lexical analyser module, responsible
* for parsing command line input
*
*
* @version   9
********************************************************************************
*/

/***** Include files ***************************************************/
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/shell/cmdParser.h>

/***** Global Macros ***************************************************/

#define CR          (0x0d)  /* carriage return character    */
#define LF          (0x0a)  /* line feed character          */
#define TAB         (0x09)  /* tabulator character          */
#define OS_EOF      (-1)

#define CMD_MAX_NAME    1024            /* maximum literal characters   */
#define CMD_MAX_CYCLE   4               /* maximum recycler items       */

    /* instance initialization macro */
#define initinstance()                      \
    PARSER_INSTANCE *safe;                  \
    if (queryInstance(&safe) != GT_OK)      \
        return GT_FAIL

    /* instance "safe return" macro */
#define safereturn(code)                    \
    return quitInstance(safe, code)


/***** Internal Type Definitions ***************************************/

/*
 * Typedef: struct PARSER_INSTANCE
 *
 * Description:
 *      thread-safe parser instance structure
 *
 * Fields:
 *      ownerThread                 - Owner thread
 *      ownerMutex                  - Owner mutex
 *      lookAhead                   - Look-ahead character value
 *      currentToken                - Current token code
 *      tokenNumeric                - Current numeric value
 *      tokenString[CMD_MAX_NAME]   - current string value
 *      inputBuffer                 - FIFO input buffer
 *      prev                        - prev item pointer
 *      next                        - next item pointer
 */
typedef struct STRUCT_PARS_INSTANCE
{
    GT_U32 ownerThread;
    GT_MUTEX ownerMutex;

    GT_32   lookAhead;
    GT_32   currentToken;
    GT_U32  tokenNumeric;
    GT_CHAR tokenString[CMD_MAX_NAME];
    GT_CHAR *inputBuffer;

    struct STRUCT_PARS_INSTANCE *prev;
    struct STRUCT_PARS_INSTANCE *next;
} PARSER_INSTANCE;

/*
 * Typedef: PARSER_TOKEN
 *
 * Description:
 *      user defined token structure
 *
 * Fields:
 *      tokenName[CMD_MAX_NAME] - token name
 *      tokenCode               - token code
 *      prev                   - prev item pointer
 *      next                   - next item pointer
 */
typedef struct STRUCT_PARS_TOKEN
{
    GT_CHAR tokenName[CMD_MAX_NAME];
    GT_U32  tokenCode;

    struct STRUCT_PARS_TOKEN *prev;
    struct STRUCT_PARS_TOKEN *next;
} PARSER_TOKEN;

extern GT_MUTEX parserMutex;

/*============================================================================
== local variable declaration ================================================
============================================================================*/
static PARSER_INSTANCE *safeHead = NULL;
static PARSER_INSTANCE *safeTail = NULL;
static PARSER_TOKEN *tokenHead = NULL;
static PARSER_TOKEN *tokenTail = NULL;

static GT_U32 currCycItem = 0;  /* current duplicated string */
/* duplicated string array */
static GT_CHAR   dupStringArray[CMD_MAX_CYCLE][CMD_MAX_NAME];

static GT_CHAR *cmdCodeDEC  = "0123456789";        /* decimal characters       */
static GT_CHAR *cmdCodeHEX  = "0123456789ABCDEF";  /* hexadecimal characters   */
static GT_CHAR *cmdCodeSKIP = "\t\r\n ";
static GT_CHAR *cmdCodeCHAR = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789_";

/*============================================================================
== thread-safe instance routines =============================================
============================================================================*/

static GT_VOID ioReadChar(PARSER_INSTANCE *safe);

/**
* @internal createInstance function
* @endinternal
*
* @brief   lookup and optionaly create owner thread instance
*
* @param[in] buffer                   - pointer to user input buffer
*
* @retval GT_OK                    - successful operation
* @retval GT_NO_RESOURCE           - memory allocation failure
*/
static GT_STATUS createInstance
(
    GT_CHAR* buffer
)
{
    GT_MUTEX mutex;
    GT_U32 thread;
    PARSER_INSTANCE *safe;

    cmdOsTaskGetSelf(&thread);
    cmdOsMutexLock(parserMutex);

    /* check whether instance already initialized */
    for (safe = safeHead; safe; safe = safe->next)
    {
        if (thread == safe->ownerThread)
        {
            /* store default values */
            safe->lookAhead = 0;
            safe->currentToken = tokUNKNOWN;
            safe->tokenNumeric = 0;
            safe->inputBuffer = buffer;
            cmdOsStrCpy(safe->tokenString, "(null)");
            ioReadChar(safe);

            cmdOsMutexUnlock(parserMutex);
            return GT_OK;
        }
    }

    /* create and initialize a new instance */
    safe = (PARSER_INSTANCE*) cmdOsStaticMalloc(sizeof(PARSER_INSTANCE));
    if (safe == NULL)
    {
        cmdOsMutexUnlock(parserMutex);
        return GT_NO_RESOURCE;
    }

    cmdOsMutexCreate(NULL, &mutex);
    safe->ownerThread = thread;
    safe->ownerMutex = mutex;
    safe->prev = NULL;
    safe->next = NULL;

    /* store default values */
    safe->lookAhead = 0;
    safe->currentToken = tokUNKNOWN;
    safe->tokenNumeric = 0;
    safe->inputBuffer = buffer;
    cmdOsStrCpy(safe->tokenString, "(null)");
    ioReadChar(safe);

    LINK_ADD(safe, safeHead, safeTail);
    cmdOsMutexUnlock(parserMutex);
    return GT_OK;
}

/**
* @internal queryInstance function
* @endinternal
*
* @brief   lookup owner thread instance
*
* @param[out] safe                     - pointer to parser thread instance structure
*
* @retval GT_OK                    - successful operation
* @retval GT_FAIL                  - parser instance not initialized
*/
static GT_STATUS queryInstance
(
    PARSER_INSTANCE **safe
)
{
    GT_U32 thread;
    PARSER_INSTANCE *self;

    cmdOsTaskGetSelf(&thread);
    cmdOsMutexLock(parserMutex);

    for (self = safeHead; self; self = self->next)
    {
        if (thread == self->ownerThread)
        {
            *safe = self;

            cmdOsMutexLock(self->ownerMutex);
            cmdOsMutexUnlock(parserMutex);
            return GT_OK;
        }
    }

    cmdOsMutexUnlock(parserMutex);
    return GT_FAIL;
}

/**
* @internal quitInstance function
* @endinternal
*
* @brief   release owner thread instance
*
* @param[in] safe                     - pointer to parser thread instance.
* @param[in] code                     - status returned from quitting thread.
*                                       trapped error value
*/
static GT_STATUS quitInstance
(
    PARSER_INSTANCE *safe,
    GT_STATUS code
)
{
    cmdOsMutexUnlock(safe->ownerMutex);
    return code;
}

/*============================================================================
== general purpose routines ==================================================
============================================================================*/

/*******************************************************************************
* duplicate
*
* DESCRIPTION:
*       cyclic string duplicator
*
* INPUTS:
*       string  - pointer to null terminated string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to duplicated string
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_CHAR* duplicate
(
    GT_CHAR *string
)
{
    if (cmdOsStrlen(string) > CMD_MAX_NAME - 1)
    {
        cmdOsPrintf("duplicate ERROR: string too long\n");
        return NULL;
    }

    currCycItem = (currCycItem + 1) % CMD_MAX_CYCLE;
    cmdOsStrCpy(dupStringArray[currCycItem], string);
    return(dupStringArray[currCycItem]);
}

/**
* @internal strcatChar function
* @endinternal
*
* @brief   concat single character value to null terminated string
*
* @param[in] dest                     - pointer to null terminated string
* @param[in] ch                       - character value
*                                       None
*/
static GT_VOID strcatChar
(
    GT_CHAR *dest,
    GT_32 ch
)
{
    GT_U32 len = cmdOsStrlen(dest);
    if (len > CMD_MAX_NAME - 1)
        return;

    dest[len + 0] = (GT_CHAR) ch;
    dest[len + 1] = 0;
}

/**
* @internal ioReadChar function
* @endinternal
*
* @brief   read single character from input buffer and store
*         character value to lookAhead variable
* @param[in] safe                     - pointer to parser thread instance
*                                       None
*/
static GT_VOID ioReadChar
(
    PARSER_INSTANCE *safe
)
{
    if (safe->inputBuffer != NULL)
        safe->lookAhead = (*safe->inputBuffer != 0) ?
            *safe->inputBuffer++ : OS_EOF;
    else
        safe->lookAhead = OS_EOF;
}

/**
* @internal ioSkip function
* @endinternal
*
* @brief   skip whitespace and CRLF characters from input buffer
*
* @param[in] safe                     - pointer to parser thread instance
*                                       None
*/
static GT_VOID ioSkip
(
    PARSER_INSTANCE *safe
)
{
    while (cmdOsStrChr(cmdCodeSKIP, safe->lookAhead) != NULL)
        ioReadChar(safe);
}

/*============================================================================
== lexiacal analyser (token parser) ==========================================
============================================================================*/

/**
* @internal cmdParseInit function
* @endinternal
*
* @brief   initialize token reader module
*
* @param[in] buffer                   = input character
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseInit
(
    IN  GT_CHAR*   buffer
)
{
    if(parserMutex == 0)
    {
        if (cmdOsMutexCreate("parser", &parserMutex) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if(buffer == NULL)
    {
        return GT_BAD_PARAM;
    }

    return createInstance(buffer);
}

/**
* @internal cmdParseDone function
* @endinternal
*
* @brief   shut down token reader module
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseDone(GT_VOID)
{
    return GT_OK;
}

/**
* @internal cmdParseAddToken function
* @endinternal
*
* @brief   add new global token identifier
*
* @param[in] identifier               - token  string constant
* @param[in] token                    -  identifier code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseAddToken
(
    IN  GT_CHAR*   identifier,
    IN  GT_U32  token
)
{
    PARSER_TOKEN *object;

    /* check for valid function arguments */
    if (identifier == NULL)
        return GT_BAD_PARAM;

    /* allocate and initialize new token object */
    object = (PARSER_TOKEN*) cmdOsStaticMalloc(sizeof(PARSER_TOKEN));
    if (object == NULL)
        return GT_NO_RESOURCE;

    object->prev = NULL;
    object->next = NULL;
    object->tokenCode = token;
    cmdOsStrCpy(object->tokenName, identifier);

    /* add object to global linked list */
    LINK_ADD(object, tokenHead, tokenTail);
    return GT_OK;
}

/**
* @internal cmdParseDelToken function
* @endinternal
*
* @brief   remove global token identifier
*
* @param[in] token                    -  identifier code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseDelToken
(
    IN  GT_U32  token
)
{
    PARSER_TOKEN *object;

    for (object = tokenHead; object; object = object->next)
    {
        if (token == object->tokenCode)
        {
            /* remove token object from list */
            LINK_DEL(object, tokenHead, tokenTail);
            cmdOsFree(object);
            return GT_OK;
        }
    }
    return GT_FAIL;
}

/**
* @internal cmdParseSkip function
* @endinternal
*
* @brief   skip single expected token
*
* @param[in] token                    -  identifier code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseSkip
(
    IN  GT_U32  token
)
{
    GT_32 errorCode = cmdParseRead();
    GT_U32 tokenCode;

    if (errorCode != GT_OK)
        return errorCode;

    if (cmdParseGet(&tokenCode) != GT_OK)
        return GT_FAIL;

    /* NOTE: OS_ERR_EOF error returned only here;       */
    /* cmdParseGet() and cmdParseRead() never return EOF    */
    /* as an error code                                 */

    if (tokenCode == token)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal cmdParseGet function
* @endinternal
*
* @brief   get current token code
*
* @param[out] token                    -  identifier code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseGet
(
    OUT GT_U32* token
)
{
    initinstance();

    *token = safe->currentToken;
    safereturn(GT_OK);
}

/**
* @internal cmdParseGetConstant function
* @endinternal
*
* @brief   get current constant identifier string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseGetConstant
(
    OUT GT_CHAR** constant
)
{
    initinstance();

    if (safe->currentToken != tokIDENTIFIER &&
        safe->currentToken != tokUNKNOWN)
        safereturn(GT_FAIL);

    *constant = duplicate(safe->tokenString);
    safereturn(GT_OK);
}

/**
* @internal cmdParseGetString function
* @endinternal
*
* @brief   get current string identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseGetString
(
    OUT GT_CHAR** string
)
{
    initinstance();

    if (safe->currentToken != tokSTRING)
        safereturn(GT_FAIL);

    *string = duplicate(safe->tokenString);

    if (*string == NULL)
        safereturn(GT_FAIL);

    safereturn(GT_OK);
}

/**
* @internal cmdParseGetNumeric function
* @endinternal
*
* @brief   get current numeric value
*
* @param[out] numeric                  - current  value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseGetNumeric
(
    OUT GT_UINTPTR* numeric
)
{
    initinstance();

    if (safe->currentToken != tokDECIMAL &&
        safe->currentToken != tokHEXADECIMAL)
        safereturn(GT_FAIL);

    *numeric = (GT_UINTPTR)safe->tokenNumeric;
    safereturn(GT_OK);
}

/**
* @internal parseStringToken function
* @endinternal
*
* @brief   read and parse next token
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Internal
*
*/
static GT_STATUS parseStringToken(PARSER_INSTANCE *safe)
{
    GT_U32 code;

    ioReadChar(safe);
    safe->tokenString[0] = 0;

    for (;;)
    {
        switch (safe->lookAhead) {
            case '\\':
                ioReadChar(safe);
                switch (cmdOsToUpper(safe->lookAhead)) {
                    case 'X':
                        ioReadChar(safe);
                        code = (GT_U32)(cmdOsStrChr(cmdCodeHEX,
                            cmdOsToUpper(safe->lookAhead)) - cmdCodeHEX) * 16;

                        ioReadChar(safe);
                        code+= (GT_U32)(cmdOsStrChr(cmdCodeHEX,
                                cmdOsToUpper(safe->lookAhead)) - cmdCodeHEX);

                        safe->lookAhead = code;
                        break;
                    case '"':   safe->lookAhead = '"';  break;
                    case '\\':  safe->lookAhead = '\\'; break;
                    case 'A':   safe->lookAhead = 0x07; break;
                    case 'B':   safe->lookAhead = 0x08; break;
                    case 'T':   safe->lookAhead = TAB;  break;
                    case 'R':   safe->lookAhead = CR;   break;
                    case 'N':   safe->lookAhead = LF;   break;
                    default:
                        return GT_FAIL;
                }
                strcatChar(safe->tokenString, safe->lookAhead);
                ioReadChar(safe);
                break;

            case CR:
            case LF:
            case OS_EOF:
                return GT_FAIL;

            case '"':
                ioReadChar(safe);
                if (safe->lookAhead != '"')
                {
                    safe->currentToken = tokSTRING;
                    return GT_OK;
                }
                strcatChar(safe->tokenString, safe->lookAhead);
                break;

            default:
                strcatChar(safe->tokenString, safe->lookAhead);
                ioReadChar(safe);
                break;
        }
    }
}

/**
* @internal cmdParseRead function
* @endinternal
*
* @brief   read and parse next token
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseRead(GT_VOID)
{
    PARSER_TOKEN *object;
    GT_STATUS status;
    GT_U32 multiplier;
    GT_CHAR *pos;

    initinstance();

    /* reset current token codes */
    safe->currentToken = tokUNKNOWN;
    safe->tokenNumeric = 0;
    safe->tokenString[0] = 0;

    /* ignore TAB and SPACE trailing characters */
    ioSkip(safe);

    /* check for end of line */
    if (safe->lookAhead == OS_EOF)
    {
        safe->currentToken = tokEOF;
        safereturn(GT_OK);
    }

    /* attempt to parse decimal constant */
    if (cmdOsStrChr(cmdCodeDEC, safe->lookAhead) != NULL)
    {
        while ((pos = cmdOsStrChr(cmdCodeDEC, safe->lookAhead)) != NULL)
        {
            safe->tokenNumeric = safe->tokenNumeric * 10 + (GT_U32)(pos - cmdCodeDEC);
            ioReadChar(safe);
        }

        /* attempt to parse hexadecimal constant */
        if (safe->lookAhead == 'x' && safe->tokenNumeric == 0)
        {
            ioReadChar(safe);
            while ((safe->lookAhead != OS_EOF) && ((pos = cmdOsStrChr(cmdCodeHEX,
                cmdOsToUpper(safe->lookAhead))) != NULL))
            {
                if(*pos == '\0') break;

                safe->tokenNumeric =
                    safe->tokenNumeric * 16 + (GT_U32)(pos - cmdCodeHEX);

                ioReadChar(safe);
            }
            safe->currentToken = tokHEXADECIMAL;
            safereturn(GT_OK);
        }

        /* check for multiplier presence */
        multiplier = (cmdOsToUpper(safe->lookAhead) == 'K') ? 1024 :
                     (cmdOsToUpper(safe->lookAhead) == 'M') ? 1024*1024 :
                     (cmdOsToUpper(safe->lookAhead) == 'G') ? 1024*1024*1024 : 1;

        if (multiplier != 1)
        {
            safe->tokenNumeric *= multiplier;
            ioReadChar(safe);
        }
        safe->currentToken = tokDECIMAL;
        safereturn(GT_OK);
    }

    /* attempt to parse name constant */
    if (cmdOsStrChr(cmdCodeCHAR, safe->lookAhead) != NULL)
    {
        while (cmdOsStrChr(cmdCodeCHAR, safe->lookAhead) != NULL)
        {
            strcatChar(safe->tokenString, safe->lookAhead);
            ioReadChar(safe);
        }

        for (object = tokenHead; object; object = object->next)
        {
            if (cmdOsStrCmp(safe->tokenString, object->tokenName) == 0)
            {
                safe->currentToken = object->tokenCode;
                safereturn(GT_OK);
            }
        }
        safe->currentToken = tokIDENTIFIER;
        safereturn(GT_OK);
    }

    /* attempt to parse operators and strings */
    status = GT_OK;
    switch (safe->lookAhead) {
        case '.':
            ioReadChar(safe);
            safe->currentToken = tokDOT;
            break;
        case ',':
            ioReadChar(safe);
            safe->currentToken = tokCOMMA;
            break;
        case ';':
            ioReadChar(safe);
            safe->currentToken = tokSEMICOLON;
            break;
        case '(':
            ioReadChar(safe);
            safe->currentToken = tokLBRACK;
            break;
        case ')':
            ioReadChar(safe);
            safe->currentToken = tokRBRACK;
            break;

        /* parse a string constant */
        case '"':
            status = parseStringToken(safe);
            break;

        default:
            safe->currentToken = tokUNKNOWN;
            strcatChar(safe->tokenString, safe->lookAhead);
            ioReadChar(safe);
            break;
    }

    safereturn(status);
}


