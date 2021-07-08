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
* @file gtStringUtil.c
*
* @brief Tool for parsing parameter string;
*
* @version   1
********************************************************************************
*/

/********* include ************************************************************/

#include <gtUtil/gtStringUtil.h>

/*******************************************************************************
* gtStringUtilFindValueSubstring
*
* DESCRIPTION:
*       Find position of substring with value specified by key.
*
* INPUTS:
*       line          - string that contains keys and values
*       key           - string used as name of value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to found substring or NULL.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_CHAR* gtStringUtilFindValueSubstring
(
    IN GT_CHAR* line,
    IN GT_CHAR* key
)
{
    GT_CHAR* keyStr;
    GT_CHAR* valStr;
    GT_CHAR  c;

    /* loop needed to resolve finding keyword inside previous keyword */
    for (keyStr = cpssOsStrStr(line, key);
          (keyStr != NULL);
          keyStr = cpssOsStrStr(keyStr, key))
    {
        if (keyStr != line)
        {
            /* check blank or comma before keyword */
            c = *(keyStr - 1);
            if ((c != ' ') && (c != '\t') && (c != ',')) continue;
        }
        /* bypass key */
        valStr = keyStr + cpssOsStrlen(key);
        /* skip blanks and tabs */
        for (; (*valStr != 0); valStr++)
        {
            c = *valStr;
            if ((c != ' ') && (c != '\t')) break;
        }
        /* check and bypass "=" symbol */
        if (*valStr != '=') continue;
        valStr++;
        /* skip blanks and tabs */
        for (; (*valStr != 0); valStr++)
        {
            c = *valStr;
            if ((c != ' ') && (c != '\t')) break;
        }
        return valStr;
    }
    return NULL;
}

/**
* @internal gtStringUtilDecimalFromLine function
* @endinternal
*
* @brief   Extract value by key from parameter line.
*
* @param[in] line                     - string that contains keys and values
* @param[in] key                      - string used as name of value
*                                       value if found or 0.
*/
GT_U32 gtStringUtilDecimalFromLine
(
    IN GT_CHAR* line,
    IN GT_CHAR* key
)
{
    GT_CHAR* valStr;
    GT_U32   val;

    valStr = gtStringUtilFindValueSubstring(line, key);
    if (valStr == NULL) return 0;
    val = 0;
    for (; ((*valStr >= '0') && (*valStr <= '9')); valStr++)
    {
        val = (val * 10) + (GT_U32)(*valStr - '0');
    }
    return val;
}




