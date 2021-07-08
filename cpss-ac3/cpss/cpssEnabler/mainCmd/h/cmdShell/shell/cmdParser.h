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
* @file cmdParser.h
*
* @brief parser engine api definitions
*
*
* @version   4
********************************************************************************
*/

#ifndef __cmdParser_h__
#define __cmdParser_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>



/*
 * enum CMD_DEFAULT_TOKENS
 *
 * Description:
 *      This enum defines default token types
 *
 * Fields:
 *  tokEOF  = -1,   - generic: END OF FILE
 *  tokUNKNOWN,     - item: unknown token
 *  tokDECIMAL,     - item: decimal numeric value
 *  tokHEXADECIMAL, - item: hexadecimal numeric value
 *  tokIDENTIFIER,  - item: identifier  constant
 *  tokSTRING,      - item: string constant
 *  tokDOT,         - operator: dot character
 *  tokCOMMA,       - operator: comma character
 *  tokSEMICOLON,   - operator: semicolon character
 *  tokLPAREN,      - operator: left parentheses
 *  tokRPAREN,      - operator; right parentheses
 *  tokLBRACK,      - operator: left brack
 *  tokRBRACK,      - operator: right brack
 *
 */
typedef enum
{
    tokEOF  = -1,   /* generic: END OF FILE             */
    tokUNKNOWN,     /* item: unknown token              */
    tokDECIMAL,     /* item: decimal numeric value      */
    tokHEXADECIMAL, /* item: hexadecimal numeric value  */
    tokIDENTIFIER,  /* item: identifier constant        */
    tokSTRING,      /* item: string constant            */
    tokDOT,         /* operator: dot character          */
    tokCOMMA,       /* operator: comma character        */
    tokSEMICOLON,   /* operator: semicolon character    */
    tokLPAREN,      /* operator: left parentheses       */
    tokRPAREN,      /* operator; right parentheses      */
    tokLBRACK,      /* operator: left brack             */
    tokRBRACK       /* operator: right brack            */
}CMD_DEFAULT_TOKENS;

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
);

/**
* @internal cmdParseDone function
* @endinternal
*
* @brief   shut down token reader module
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseDone(GT_VOID);

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
    IN  GT_CHAR* identifier,
    IN  GT_U32   token
);

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
);

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
);

/**
* @internal cmdParseRead function
* @endinternal
*
* @brief   read and parse next token
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdParseRead(GT_VOID);


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
);

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
);

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
);

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
    OUT GT_CHAR** constant
);


/**
* @internal cmdParseLine function
* @endinternal
*
* @brief   Parses single command stored in null terminated string.
*
* @param[in] inBuffer                 - null terminated string holding command buffer
* @param[in] inFieldBuffer            - null terminated string holding field values
*
* @param[out] isShellCmd               - GT_TRUE, if it is shell command.
*
* @retval CMD_OK                   - if there are no problems,
*/
CMD_STATUS cmdParseLine
(
    IN  GT_CHAR      *inBuffer,
    IN  GT_CHAR      *inFieldBuffer,
    OUT GT_BOOL      *isShellCmd
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __cmdParser_h__ */


