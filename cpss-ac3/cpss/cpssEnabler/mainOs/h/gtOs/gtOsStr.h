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
* @file gtOsStr.h
*
* @brief Operating System wrapper. String facility.
*
* @version   9
********************************************************************************
*/

#ifndef __gtOsStrh
#define __gtOsStrh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/**
* @internal osStrlen function
* @endinternal
*
* @brief   Determine the length of a string.
*
* @param[in] source                   - string
*
* @retval size                     - number of characters in string, not including EOS.
*/
GT_U32 osStrlen
(
    IN const GT_VOID * source
);

/*******************************************************************************
* osStrCpy
*
* DESCRIPTION:
*       Copies string 'source' (including EOS) to string 'dest'.
*
* INPUTS:
*       dest    - pointer to a buffer for the copied string
*       source  - string to copy
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the 'dest'.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source
);

/*******************************************************************************
* osStrNCpy
*
* DESCRIPTION:
*       Copies string 'source' (including EOS) to string 'dest'.
*
* INPUTS:
*       dest    - pointer to a buffer for the copied string
*       source  - string to copy
*       len     - copy no more than len characters
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the 'dest'.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrNCpy
(
    IN GT_CHAR         *dest,
    IN const GT_CHAR   *source,
    IN GT_U32          len
);

/*******************************************************************************
* osStrChr
*
* DESCRIPTION:
*       Find the first occurrence of a character in a string.
*
* INPUTS:
*       source      - string to look in to
*       character   - character to look for
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the found character or
*       NULL - if character were not found
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrChr
(
    IN const GT_CHAR *source,
    IN GT_32         character
);

/*******************************************************************************
* osStrStr
*
* DESCRIPTION:
*       Locate a substring
*
* INPUTS:
*       source      - string to look in to
*       needle      - substring to find
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the found substring or
*       NULL - if character were not found
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrStr
(
    IN const GT_CHAR *source,
    IN const GT_CHAR *needle
);

/*******************************************************************************
* osStrrChr
*
* DESCRIPTION:
*       Find the last occurrence of character in a string.
*
* INPUTS:
*       source      - string to look in to
*       character   - character to look for
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the found character or
*       NULL - if character were not found
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrrChr
(
    IN const GT_CHAR   *source,
    IN GT_32           character
);

/**
* @internal osStrCmp function
* @endinternal
*
* @brief   Compares lexicographically the null terminating strings str1 and str2.
*
* @param[in] str1                     - string to look in to
* @param[in] str2                     - character to look for
*
* @retval > 0                      - if str1 is alfabetic bigger than str2
* @retval == 0                     - if str1 is equal to str2
* @retval < 0                      - if str1 is alfabetic smaller than str2
*/
GT_32 osStrCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2
);

/**
* @internal osStrNCmp function
* @endinternal
*
* @brief   Compares lexicographically the null terminating strings str1 and str2.
*
* @param[in] str1                     - string to look in to
* @param[in] str2                     - character to look for
* @param[in] len                      - number of characters to compare
*
* @retval > 0                      - if str1 is alfabetic bigger than str2
* @retval == 0                     - if str1 is equal to str2
* @retval < 0                      - if str1 is alfabetic smaller than str2
*/
GT_32 osStrNCmp
(
    IN const GT_CHAR   *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
);

/*******************************************************************************
* osStrCat
*
* DESCRIPTION:
*       Appends a copy of string 'str2' to the end of string 'str1'.
*
* INPUTS:
*       str1   - destination string
*       str2   - sthring to add the destination string
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the destination string (str1)
*
* COMMENTS:
*       The resulting string is null-terminated.
*
*******************************************************************************/
GT_CHAR *osStrCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
);

/*******************************************************************************
* osStrNCat
*
* DESCRIPTION:
*       Appends up to 'len' characters from string 'str1' to the end
*       of string 'str2'.
*
* INPUTS:
*       str1   - destination string
*       str2   - sthring to add the destination string
*       len    - number of characters to concat
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the destination string (str1)
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_CHAR *osStrNCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2,
    IN GT_U32          len
);

/**
* @internal osToUpper function
* @endinternal
*
* @brief   Converts a lower-case letter to the corresponding upper-case letter.
*
* @param[in] character                - a  to convert to upper case
*                                       The upper case character
*/
GT_32 osToUpper
(
    IN const GT_32 character
);

/**
* @internal osStrTo32 function
* @endinternal
*
* @brief   Converts the initial portion of the string s to long integer
*         representation.
* @param[in] string                   - a  to convert
*                                       The converted value represented as a long.
*/
GT_32 osStrTo32
(
    IN const GT_CHAR *string
);


/**
* @internal osStrToU32 function
* @endinternal
*
* @brief   Converts the initial portion of the string s to unsigned long integer
*         representation.
* @param[in] string                   - a  to convert
*                                      endptr   - ptr to final string
* @param[in] base                     - radix
*                                       The converted value or ZERO, if no conversion could be performed.
*/
GT_U32 osStrToU32
(
    IN const GT_CHAR *string,
    IN GT_CHAR **    endptr,
    IN GT_32         base
);

/**
* @internal osStrTol function
* @endinternal
*
* @brief   Converts the initial portion of the string s to long integer
*         representation.
* @param[in] string     - a  to convert
* @param[in] endPtr     - ptr to final string
* @param[in] base       - radix
*
* @retval               - the converted value or ZERO, if no conversion could be performed.
*/
GT_32 osStrTol
(
    IN const GT_CHAR *string,
    IN GT_CHAR **    endPtr,
    IN GT_32         base
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsStrh */
/* Do Not Add Anything Below This Line */



