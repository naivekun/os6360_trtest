/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cmdPresteraUtils.c
*
* DESCRIPTION:
*       Galtis agent interface to Prestera specific functionality
*
* FILE REVISION NUMBER:
*       $Revision: 17 $
*******************************************************************************/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>

#ifdef WIN32
#include <stdarg.h>
#endif

static const GT_CHAR hexcode[] = "0123456789ABCDEF";

/* flag to state that the DXCH (xcat) devices need the WA of trunk-Id conversions */
GT_BOOL dxChTrunkWaNeeded = GT_FALSE;

/* indication the trunk convert done for first time */
static GT_BOOL firstTimeTrunkConvert = GT_TRUE;

/* use 64 instead of 56 , so HEX number will not change much */
#define WA_FIRST_TRUNK_CNS  64
#define WA_LAST_TRUNK_CNS  (124+1)

    /* currently support single device */
static GT_BOOL             utilMultiPortGroupsBmpEnable[PRV_CPSS_MAX_PP_DEVICES_CNS]={0};
static GT_PORT_GROUPS_BMP  utilMultiPortGroupsBmp[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* CPSS Lib reset callback functionality */
#define MAX_RESET_CB_NUM_CNS 256

static wrCpssResetLibFuncPtr wrCpssResetLibFuncDb[MAX_RESET_CB_NUM_CNS];
static GT_U32 currResetCbNum = 0;

/**
* @internal galtisMacAddr function
* @endinternal
*
* @brief   Create Ethernet MAC Address from hexadecimal coded string
*         6 elements, string size = 12 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_ETHERADDR structure
*                                       none
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisMacAddr
(
    OUT GT_ETHERADDR *dest,
    IN  GT_U8        *source
)
{
    int element;

    for (element = 0; element < 6; element++) {
        dest->arEther[element] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode) << 4;

        dest->arEther[element]+= (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode);
    }
}

/*******************************************************************************
* galtisByteArrayToString
*
* DESCRIPTION:
*       convert byte array to the string
*
* INPUTS:
*       byteArr - pointer to byte array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the string contans converted data
*
* COMMENTS:
* Toolkit:
*
*******************************************************************************/
char* galtisByteArrayToString
(
    IN GT_U8  *byteArr,
    IN GT_U32 arraySize
)
{
    static char   strBuf[256];       /* cyclic memory for result         */
    static GT_U32 bufUsedSize = 0;   /* size of used part of the buiffer */
    static char   cnvTab[] = "0123456789ABCDEF"; /* bin => char table    */
    GT_U32        i;                 /* loop index                       */
    char*         strPtr;

    if ((2 * arraySize) >= sizeof(strBuf))
    {
        return "*** galtisByteArrayToString ERROR ***";
    }

    if (((2 * arraySize) + bufUsedSize) >= sizeof(strBuf))
    {
        /* reset free memory pointer */
        bufUsedSize = 0;
    }

    strPtr = &(strBuf[bufUsedSize]);

    for (i = 0; (i < arraySize); i++)
    {
        strPtr[2 * i]       = cnvTab[(byteArr[i] & 0xF0) >> 4];
        strPtr[(2 * i) + 1] = cnvTab[byteArr[i] & 0x0F];
    }
    /* null termination of the string */
    strPtr[2 * arraySize] = 0;

    /* increment free memory pointer */
    bufUsedSize += (2 * arraySize) + 1;

    return strPtr;
}

/**
* @internal galtisIpAddr function
* @endinternal
*
* @brief   Create IPv4 Address from hexadecimal coded string of
*         4 elements, string size = 8 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_IPADDR structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisIpAddr
(
    OUT GT_IPADDR *dest,
    IN  GT_U8     *source
)
{
    int element;

    for (element = 0; element < 4; element++) {
        dest->arIP[element] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode) << 4;

        dest->arIP[element]+= (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode);
    }
}

/**
* @internal galtisIpv6Addr function
* @endinternal
*
* @brief   Create IPv6 Address from hexadecimal coded string of
*         16 elements, string size = 32 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_IPV6ADDR structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisIpv6Addr
(
    OUT GT_IPV6ADDR *dest,
    IN  GT_U8     *source
)
{
    int element;

    for (element = 0; element < 16; element++) {
        dest->arIP[element] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode) << 4;

        dest->arIP[element]+= (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode);
    }
}

/**
* @internal galtisFcoeAddr function
* @endinternal
*
* @brief   Create FCoE Address from hexadecimal coded string of
*         3 elements, string size = 24 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_FCID structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisFcoeAddr
(
    OUT GT_FCID   *dest,
    IN  GT_U8     *source
)
{
    int element;

    for (element = 0; element < 3; element++) {
        dest->fcid[element] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode) << 4;

        dest->fcid[element]+= (int)(cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) -
            hexcode);
    }
}

/**
* @internal galtisU64 function
* @endinternal
*
* @brief   Create GT_U64 structure from two elements
*
* @param[in] lo                       hi - low and high elements of GT_U64
*
* @param[out] big                      - initialized GT_U64 structure
*                                       None
*/
GT_VOID galtisU64
(
    OUT GT_U64 *big,
    IN  GT_U32 lo,
    IN  GT_U32 hi
)
{
    big->l[0] = lo;
    big->l[1] = hi;
}

/*******************************************************************************
* galtisU64COut
*
* DESCRIPTION:
*       Create Galtis compatible string output from GT_U64 for OUTPUT
*
* INPUTS:
*       big - 64-bit integer (GT_U64)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the GT_U64 value
*
* COMMENTS:
*
*******************************************************************************/
GT_U8* galtisU64COut
(
    IN  GT_U64 *big
)
{
    static GT_CHAR tmp[256];
    cmdOsSprintf(tmp, "%lx!!!%lx", big->l[0], big->l[1]);
    return (GT_U8*)strDuplicate(tmp);
}

/*******************************************************************************
* galtisU64FOut
*
* DESCRIPTION:
*       Create Galtis compatible string output from GT_U64 for FIELDS
*
* INPUTS:
*       big - 64-bit integer (GT_U64)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the GT_U64 value as table fileds
*
* COMMENTS:
*
*******************************************************************************/
GT_U8* galtisU64FOut
(
    IN GT_U64 *big
)
{
    static GT_CHAR tmp[256];
    cmdOsSprintf(tmp, "%lx???%lx", big->l[0], big->l[1]);
    return (GT_U8*)strDuplicate(tmp);
}

/*******************************************************************************
* galtisBArrayOut
*
* DESCRIPTION:
*       Create Galtis compatible string output from char buffer
*
* INPUTS:
*       bArray_Ptr - the buffer data and length in a struct
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the buffer data
*
* COMMENTS:
*
*******************************************************************************/
GT_U8* galtisBArrayOut
(
    IN GT_BYTE_ARRY *bArray_Ptr
)
{
    static GT_CHAR tmpArr[CMD_MAX_BUFFER];
    GT_U32 i, j;
    GT_U32 length;

    /* set the copy length */
    if (bArray_Ptr->length <= (CMD_MAX_BUFFER - 2))
    {
        length = bArray_Ptr->length;
    }
    else
    {
        /* save place for the last two zero bytes */
        length = CMD_MAX_BUFFER - 2;
    }

    /* convert the nibbles to ASCII bytes
     * the max copied data should not to be grater than
     * CMD_MAX_BUFFER
     */
    for (i = 0, j = 0; i < length && i < (CMD_MAX_BUFFER/2)-2 ; i++, j+=2)
    {
        tmpArr[j + 0] = hexcode[bArray_Ptr->data[i] >> 4];
        tmpArr[j + 1] = hexcode[bArray_Ptr->data[i] & 0xf];
    }

    /* set the last two bytes */
    tmpArr[j++] = 0;
    tmpArr[j]   = 0;

    return (GT_U8*)strDuplicate(tmpArr);
}


/**
* @internal galtisBArray function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string
*
* @param[in] source                   - byte array buffer
*
* @param[out] byteArray                - byte array data and size
*                                       None
*/
GT_VOID galtisBArray
(
    OUT GT_BYTE_ARRY *byteArray ,
    IN GT_U8* source
)
{
    GT_U32 element;
    GT_U32 len = cmdOsStrlen((GT_CHAR*)source)/2;

    byteArray->data = cmdOsMalloc(len);
    byteArray->length = len;
    for (element = 0; element < len; element++) {
        byteArray->data[element] = (int)
            (cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) - hexcode) << 4;

        byteArray->data[element]+= (int)
            (cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) - hexcode);
    }
}

/**
* @internal galtisBArraySized function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string expanding or trunkating to size
*
* @param[in] source                   - byte array buffer (hexadecimal string)
* @param[in] size                     - exact  in bytes
* @param[in] defData                  - size of default data
* @param[in] defData                  - default data for bytes, not specified in source
*
* @param[out] byteArray                - byte array data and size
*                                       None
*/
GT_VOID galtisBArraySized
(
    OUT GT_BYTE_ARRY *byteArray ,
    IN GT_U8* source,
    IN GT_U32 size,
    IN GT_U32  defDataSize,
    IN GT_U8* defData
)
{
    GT_U32 element;
    GT_U32 len = (cmdOsStrlen((GT_CHAR*)source) + 1)/2;

    /* length truncate */
    if (len > size)
    {
        len = size ;
    }

    byteArray->data = cmdOsMalloc(size);

    if (NULL == byteArray->data)
    {
        return;
    }

    /* zero out the packet data buffer */
    cmdOsBzero((GT_CHAR *)byteArray->data, size);

    byteArray->length = size;

    if (defDataSize > len)
    {
        /* length is shorter than default size, copy default data */
        cmdOsMemCpy(byteArray->data, defData, defDataSize);
    }
    else
    {
        for (element = 0; element < len; element++) {
            byteArray->data[element] = (int)
                (cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) - hexcode) << 4;

            byteArray->data[element]+= (int)
                (cmdOsStrChr(hexcode, cmdOsToUpper(*source++)) - hexcode);
        }
    }
}

/**
* @internal galtisBArrayFree function
* @endinternal
*
* @brief   Free GT_BYTE_ARRY from string
*
* @note Toolkit:
*
*/
GT_VOID galtisBArrayFree
(
    IN GT_BYTE_ARRY *byteArray
)
{
    if (byteArray->data != NULL)
    {
        cmdOsFree(byteArray->data);
    }
}

/**
* @internal galtisBytesArrayDirect function
* @endinternal
*
* @brief   build Byte Array from string in the given buffer
*
* @param[in] source                   - byte array buffer (hexadecimal string)
* @param[in] size                     - exact  in bytes
*
* @param[out] byteArray                - byte array data
*                                       None
*/
GT_VOID galtisBytesArrayDirect
(
    OUT GT_U8  *byteArray,
    IN  GT_U8  *source,
    IN  GT_U32 size
)
{
    GT_U32 i,j;
    GT_U8  sym;
    GT_U8  val[2];
    GT_U32 len = (cmdOsStrlen((GT_CHAR*)source) + 1)/2;

    /* length truncate */
    if (len > size)
    {
        len = size ;
    }

    /* zero out the packet data buffer */
    cmdOsBzero((GT_CHAR *)byteArray, size);

    for (i = 0; (i < len); i++)
    {
        for (j = 0; (j < 2); j++)
        {
            sym = source[(2 * i) + j];
            val[j] = 0;
            if (('0' <= sym) && ('9' >= sym))
            {
                val[j] = (GT_U8)(sym - '0');
            }
            if (('a' <= sym) && ('f' >= sym))
            {
                val[j] = (GT_U8)(sym - 'a' + 10);
            }
            if (('A' <= sym) && ('F' >= sym))
            {
                val[j] = (GT_U8)(sym - 'A' + 10);
            }
        }
        byteArray[i] = (GT_U8)((val[0] << 4) | val[1]);
    }
}

/**
* @internal galtisIsMacGreater function
* @endinternal
*
* @brief   return true if one mac address is greater than the other.
*
* @param[in] firstMac                 - first MAC address.
* @param[in] secondMac                - second MAC address.
*                                       true if greater.
*/
GT_BOOL galtisIsMacGreater
(
    IN GT_ETHERADDR firstMac,
    IN GT_ETHERADDR secondMac
)
{
    GT_U32 i;

    for(i = 0; i < 6; i++)
    {
        if(firstMac.arEther[i] > secondMac.arEther[i])
        {
            return GT_TRUE;
        }

        if(firstMac.arEther[i] < secondMac.arEther[i])
        {
            return GT_FALSE;
        }
    }

    /* equal */
    return GT_FALSE;
}

/**
* @internal utilFindStructureElenentDescriptorByPath function
* @endinternal
*
* @brief   searches the structure element descriptor by path
*
* @param[in] startDscArr[]            - (pointer to) array of descriptors for root structure
* @param[in] dscEntrySize             - size of descriptor entry in bytes
*                                      dedicated to application definded descrioprs that
*                                      contain the GT_UTIL_STC_ELEMENT_DSC_STC as first member
*                                      0 means sizeof(GT_UTIL_STC_ELEMENT_DSC_STC)
* @param[in] nameSeparator            - character used as separator in the path
* @param[in] pathStr                  - string that contains the path of the searched field
*
* @param[out] foundDscPtrPtr           - (pointer to) pointer to found descriptor entry
* @param[out] summaryOffsetPtr         - (pointer to) summary offset of the root structure origin
*
* @retval GT_OK                    - found
* @retval GT_NO_SUCH               - not found
*/
GT_STATUS utilFindStructureElenentDescriptorByPath
(
    IN  GT_UTIL_STC_ELEMENT_DSC_STC   startDscArr[],
    IN  GT_U32                        dscEntrySize,
    IN  GT_U8                         nameSeparator,
    IN  GT_U8                         *pathStr,
    OUT GT_UTIL_STC_ELEMENT_DSC_STC   **foundDscPtrPtr,
    OUT GT_U32                        *summaryOffsetPtr
)
{
    GT_U32                      nameLength; /* name Length        */
    GT_UTIL_STC_ELEMENT_DSC_STC *dscPtr;    /* descriptor Pointer */
    GT_U8                       *pathPtr;   /* pointer into path  */

    /* to allow use the 0 as default record size */
    if (dscEntrySize < sizeof(GT_UTIL_STC_ELEMENT_DSC_STC))
    {
        dscEntrySize = sizeof(GT_UTIL_STC_ELEMENT_DSC_STC);
    }

    /* initialize sum */
    *summaryOffsetPtr = 0;

    /* to avoid compiler warning */
    pathPtr = NULL;

    for (dscPtr = startDscArr;
          ((dscPtr != NULL) && (*pathStr != 0)); pathStr = pathPtr)
    {
        /* search separator or end of line*/
        for (pathPtr = pathStr;
              ((*pathPtr != nameSeparator) && (*pathPtr != 0));
              pathPtr++) {};

        /* calculate the length of the name */
        nameLength = (GT_U32)(pathPtr - pathStr);

        /* bypass the separator */
        if (*pathPtr == nameSeparator)
        {
            pathPtr++;
        }

        /* searc the descriptor with the name from path */
        for (; (dscPtr->namePtr != NULL); dscPtr =
              (GT_UTIL_STC_ELEMENT_DSC_STC*)((GT_U8*)dscPtr + dscEntrySize))
        {
            if (dscPtr->namePtr[nameLength] != 0)
            {
                /* different length */
                continue;
            }

            if (cmdOsMemCmp(pathStr, dscPtr->namePtr, nameLength) == 0)
            {
                /* found */
                break;
            }
        }

        if (dscPtr->namePtr == NULL)
        {
            /* end of list reached */
            return GT_NO_SUCH;
        }

        /* the name found */
        *foundDscPtrPtr = dscPtr;
        *summaryOffsetPtr += dscPtr->offset;

        /* down to subtree */
        dscPtr = dscPtr->subtreePtr;
    }

    return ((pathStr[0] == 0) ? GT_OK : GT_NO_SUCH);
}

/**
* @internal utilListStructureElenentNamesByPath function
* @endinternal
*
* @brief   prints the structure element names by path
*
* @param[in] startDscArr[]            - (pointer to) array of descriptors for root structure
* @param[in] dscEntrySize             - size of descriptor entry in bytes
*                                      dedicated to application definded descrioprs that
*                                      contain the GT_UTIL_STC_ELEMENT_DSC_STC as first member
*                                      0 means sizeof(GT_UTIL_STC_ELEMENT_DSC_STC)
* @param[in] nameSeparator            - character used as separator in the path
* @param[in] pathStr                  - string that contains the path of the searched field
*
* @retval GT_OK                    - found
* @retval GT_NO_SUCH               - not found
*/
GT_STATUS utilListStructureElenentNamesByPath
(
    IN  GT_UTIL_STC_ELEMENT_DSC_STC   startDscArr[],
    IN  GT_U32                        dscEntrySize,
    IN  GT_U8                         nameSeparator,
    IN  GT_U8                         *pathStr
)
{
    GT_STATUS                         rc;
    GT_UTIL_STC_ELEMENT_DSC_STC       *dscPtr;    /* descriptor Pointer */
    GT_UTIL_STC_ELEMENT_DSC_STC       *foundDscPtr;
    GT_U32                            offset;
    GT_U8                             separatorStr[2];

    separatorStr[0] = nameSeparator;
    separatorStr[1] = 0;

    /* to allow use the 0 as default record size */
    if (dscEntrySize < sizeof(GT_UTIL_STC_ELEMENT_DSC_STC))
    {
        dscEntrySize = sizeof(GT_UTIL_STC_ELEMENT_DSC_STC);
    }

    if ((pathStr == NULL) || (*pathStr == 0))
    {
        foundDscPtr = startDscArr;
    }
    else
    {
        rc = utilFindStructureElenentDescriptorByPath(
            startDscArr, dscEntrySize, nameSeparator,
            pathStr, &foundDscPtr, &offset);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Specified path not found \n");
            return GT_NO_SUCH;
        }

        if (foundDscPtr->subtreePtr == NULL)
        {
            cpssOsPrintf("Specified path points to field (not to structure)\n");
            return GT_OK;
        }

        /* go to list of structure elements */
        foundDscPtr = foundDscPtr->subtreePtr;
    }

    cpssOsPrintf("Structures paths: \n");
    for (dscPtr = foundDscPtr; (dscPtr->namePtr != NULL);
          dscPtr = (GT_UTIL_STC_ELEMENT_DSC_STC*)((GT_U8*)dscPtr + dscEntrySize))
    {
        if (dscPtr->subtreePtr == NULL)
        {
            continue;
        }
        if ((pathStr == NULL) || (*pathStr == 0))
        {
            cpssOsPrintf("%s\n", dscPtr->namePtr);
        }
        else
        {
            cpssOsPrintf("%s%s%s\n", pathStr, separatorStr, dscPtr->namePtr);
        }
    }

    cpssOsPrintf("Field paths: \n");
    for (dscPtr = foundDscPtr; (dscPtr->namePtr != NULL);
          dscPtr = (GT_UTIL_STC_ELEMENT_DSC_STC*)((GT_U8*)dscPtr + dscEntrySize))
    {
        if (dscPtr->subtreePtr != NULL)
        {
            continue;
        }
        if ((pathStr == NULL) || (*pathStr == 0))
        {
            cpssOsPrintf("%s\n", dscPtr->namePtr);
        }
        else
        {
            cpssOsPrintf("%s%s%s\n", pathStr, separatorStr, dscPtr->namePtr);
        }
    }

    return GT_OK;
}

/**
* @internal utilHexStringToStcField function
* @endinternal
*
* @brief   Convers the string that contains pairs of hex numbers to the
*         contents of the "C" language defined data
* @param[in] dataStr                  - (pointer to) string with pairs of hex numbers
* @param[in] dataBytesSize            - amount of bytes to store
* @param[in] storeFormat              - the format conversion rule used during the strore
*
* @param[out] fieldPtr                 - (pointer to) the field tat the data stored to
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - wrong hex data in the string
*/
GT_STATUS utilHexStringToStcField
(
    IN  GT_U8                              *dataStr,
    IN  GT_U32                             dataBytesSize,
    IN  GT_UTIL_HEX_DATA_STORE_FORMAT_ENT  storeFormat,
    OUT GT_VOID                            *fieldPtr
)
{
    GT_U32 i;                       /* loop index                      */
    GT_U8  byte;                    /* parsed byte                     */
    GT_U8  highNum;                 /* the high hex number             */
    GT_U8  lowNum;                  /* the low hex number              */
    unsigned long int cpuInteger;   /* the long CPU stiled accomulator */

    cpuInteger = 0;

    for (i = 0; (i < dataBytesSize); i++)
    {
        /* HEX -> BIN */
        highNum = dataStr[i << 1];
        if ((highNum >= '0') && (highNum <= '9'))
        {
            highNum -= '0';
        }
        else if ((highNum >= 'A') && (highNum <= 'F'))
        {
            highNum -= 'A';
            highNum += 10;
        }
        else if ((highNum >= 'a') && (highNum <= 'f'))
        {
            highNum -= 'a';
            highNum += 10;
        }
        else
        {
            return GT_BAD_PARAM;
        }

        /* HEX -> BIN */
        lowNum = dataStr[(i << 1) + 1];
        if ((lowNum >= '0') && (lowNum <= '9'))
        {
            lowNum -= '0';
        }
        else if ((lowNum >= 'A') && (lowNum <= 'F'))
        {
            lowNum -= 'A';
            lowNum += 10;
        }
        else if ((lowNum >= 'a') && (lowNum <= 'f'))
        {
            lowNum -= 'a';
            lowNum += 10;
        }
        else
        {
            return GT_BAD_PARAM;
        }

        byte = (highNum << 4) + lowNum;

        switch (storeFormat)
        {
            case GT_UTIL_HEX_DATA_STORE_CPU_INTEGER_E:
                cpuInteger = (cpuInteger << 8) + byte;
                break;
            case GT_UTIL_HEX_DATA_STORE_NETWORK_ORDERED_E:
                ((GT_U8*)fieldPtr)[i] = byte;
                break;
            case GT_UTIL_HEX_DATA_STORE_REVERSE_ORDERED_E:
                ((GT_U8*)fieldPtr)[dataBytesSize - 1 - i] = byte;
                break;
            default: return GT_BAD_PARAM;
        }
    }

    if (storeFormat == GT_UTIL_HEX_DATA_STORE_CPU_INTEGER_E)
    {
        switch (dataBytesSize)
        {
            default: return GT_BAD_PARAM;
            case 1:
                *(GT_U8*)fieldPtr = (GT_U8)cpuInteger;
                break;
            case 2:
                *(GT_U16*)fieldPtr = (GT_U16)cpuInteger;
                break;
            case 4:
                *(GT_U32*)fieldPtr = (GT_U32)cpuInteger;
                break;
            /* reserved for very large CPU (64 bit ?)*/
            case 8:
                *(unsigned long int*)fieldPtr = cpuInteger;
                break;
        }
    }

    return GT_OK;
}

/**
* @internal firstTimeTrunkConvertInit_1 function
* @endinternal
*
* @brief   initialize the DB for the trunk Id conversion
*/
static GT_VOID firstTimeTrunkConvertInit_1(GT_VOID)
{
    GT_STATUS rc;
    GT_U32  ii;
    CMD_APP_PP_CONFIG   ppConfig;

    firstTimeTrunkConvert = GT_FALSE;

    /* Add element for each device in system    */
    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        rc = cmdAppPpConfigGet((GT_U8)ii,&ppConfig);
        if(rc != GT_OK || ppConfig.valid/*active*/ == GT_FALSE)
            continue;

        if(ppConfig.wa.trunkToCpuPortNum == GT_TRUE)
        {
            dxChTrunkWaNeeded = GT_TRUE;
            break;
        }
    }
}

/**
* @internal firstTimeTrunkConvertInit function
* @endinternal
*
* @brief   initialize the DB for the trunk Id conversion
*/
static GT_VOID firstTimeTrunkConvertInit(GT_VOID)
{
    if(firstTimeTrunkConvert == GT_FALSE)
    {
        /* initialization already done */
        return;
    }

    firstTimeTrunkConvertInit_1();
}

/**
* @internal gtTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
GT_VOID gtTrunkConvertFromTestToCpss
(
   INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    GT_TRUNK_ID newTrunkId;/* new trunkId s*/

    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */
    /* convert trunk-Id from test (1..30) to trunkId that fit the WA  (66..124, even) */

    if((*trunkIdPtr) > WA_LAST_TRUNK_CNS)  /* bigger then last (not first) */
    {
        /*no change*/
        return;
    }
    else if((*trunkIdPtr) == 0)
    {
        /*no change*/
        return;
    }

    newTrunkId = (GT_TRUNK_ID)(WA_FIRST_TRUNK_CNS + ((*trunkIdPtr) * 2));
    if(newTrunkId >= 128)
    {
        /* can't convert */
        return;
    }

    (*trunkIdPtr) = newTrunkId;

    return;
}

/**
* @internal gtTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   convert trunkId from CPSS value to value that the TEST expect (refresh)
*
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
GT_VOID gtTrunkConvertFromCpssToTest
(
   INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    if((*trunkIdPtr) > WA_LAST_TRUNK_CNS)
    {
        /*no change*/
        return;
    }
    else if((*trunkIdPtr) < WA_FIRST_TRUNK_CNS)
    {
        /* can't convert */
        return;
    }
    else if((*trunkIdPtr) == 0)
    {
        /*no change*/
        return;
    }

    (*trunkIdPtr) = (GT_TRUNK_ID)(((*trunkIdPtr) - WA_FIRST_TRUNK_CNS) / 2);

    return;
}


/**
* @internal gtPclMaskTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   for PCL MASK trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
GT_VOID gtPclMaskTrunkConvertFromTestToCpss
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID   *trunkIdPtr
)
{
    if(isTrunkMask == 0 || isTrunkPattern == 0)
    {
        return;
    }

    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */

    /* take 5 bits and shift them 1 , force match on bits 0,6 */
    (*trunkIdPtr) = (GT_TRUNK_ID)(BIT_6 | (((*trunkIdPtr)& 0x1f) << 1) | BIT_0);

    return;
}

/**
* @internal gtPclMaskTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   for PCL MASK trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from CPSS value to value that the TEST expect (refresh)
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
GT_VOID gtPclMaskTrunkConvertFromCpssToTest
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    if(isTrunkMask == 0 || isTrunkPattern == 0)
    {
        return;
    }

    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */

    /* take 5 bits and shift them 1 , set bits 5,6 */
    (*trunkIdPtr) = (GT_TRUNK_ID)(BIT_6 | BIT_5 | (((*trunkIdPtr) >> 1 )& 0x1f));

    return;
}

/**
* @internal gtPclPatternTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   for PCL Pattern trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
GT_VOID gtPclPatternTrunkConvertFromTestToCpss
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID   *trunkIdPtr
)
{
    if(isTrunkMask == 0 || isTrunkPattern == 0)
    {
        return;
    }

    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */

    /* take 5 bits and shift them 1 , clear bits 0,6 */
    (*trunkIdPtr) = (GT_TRUNK_ID)(((*trunkIdPtr) & 0x1f) << 1);

    return;
}

/**
* @internal gtPclPatternTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   for PCL Pattern trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from CPSS value to value that the TEST expect (refresh)
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
GT_VOID gtPclPatternTrunkConvertFromCpssToTest
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    if(isTrunkMask == 0 || isTrunkPattern == 0)
    {
        return;
    }

    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */

    /* take 5 bits and shift them 1 , clear bits 5,6 */
    (*trunkIdPtr) = (GT_TRUNK_ID)(((*trunkIdPtr) >> 1 )& 0x1f);

    return;
}

/**
* @internal gtPclMaskPclIdConvertFromTestToCpss function
* @endinternal
*
* @brief   convert pclId of the pcl mask from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in,out] pclIdPtr                 - (pointer to) the pclId of the test
* @param[in,out] pclIdPtr                 - (pointer to) the pclId of the CPSS
*                                       none
*/
GT_VOID gtPclMaskPclIdConvertFromTestToCpss
(
   INOUT GT_U32 *pclIdPtr
)
{
    firstTimeTrunkConvertInit();

    if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    (*pclIdPtr) |= BIT_0;/* set bit 0 to 'Exact match' */

    return;
}

/**
* @internal gtTrunkWaForceDownSet function
* @endinternal
*
* @brief   set if the trunk WA need to be forced down (disabled)
*
* @param[in] forceDown                - do we want to force down the WA
*                                      GT_TRUE - force down (disable) the WA
*                                      GT_FALSE - don't force down (allow to enable) the WA
*
* @retval GT_OK                    - success
*
* @note this function is to be called from the terminal -->
*       AFTER cpssInitSystem(...)
*
*/
GT_STATUS gtTrunkWaForceDownSet
(
    IN GT_U32    forceDown
)
{
    if(forceDown)
    {
        dxChTrunkWaNeeded = GT_FALSE;    /* shutdown the WA if already used */
        firstTimeTrunkConvert = GT_FALSE;/* do not allow to start the WA */
    }
    else if(dxChTrunkWaNeeded == GT_FALSE)
    {
        /* restart the check for WA */
        firstTimeTrunkConvertInit_1();
    }

    return GT_OK;
}

/**
* @internal utilMultiPortGroupsBmpSet function
* @endinternal
*
* @brief   set the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter
* @param[in] devNum                   - device number
* @param[in] enable                   -  / disable the use of APIs with portGroupsBmp parameter.
* @param[in] portGroupsBmp            - port groups bmp , relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - on bad device number
*/
GT_STATUS utilMultiPortGroupsBmpSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
)
{
    GT_U8               tmpDevNum = devNum;

    CONVERT_DEV_PORT_GROUPS_BMP_MAC(tmpDevNum,portGroupsBmp);

    if(tmpDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }

    utilMultiPortGroupsBmp[tmpDevNum] = portGroupsBmp;
    utilMultiPortGroupsBmpEnable[tmpDevNum] = enable;


    return GT_OK;
}

/**
* @internal utilMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS utilMultiPortGroupsBmpGet
(
    IN  GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(portGroupsBmpPtr);

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }

    /* currently support single device */
    *portGroupsBmpPtr = utilMultiPortGroupsBmp[devNum];
    *enablePtr = utilMultiPortGroupsBmpEnable[devNum];

    CONVERT_DEV_PORT_GROUPS_BMP_MAC(devNum,(*portGroupsBmpPtr));

    return GT_OK;
}

/**
* @internal wrCpssResetLib function
* @endinternal
*
* @brief   Reset all registered libs.
*
* @retval GT_OK                    - OK
* @retval GT_EMPTY                 - on empty list
*/
GT_STATUS wrCpssResetLib
(
    GT_VOID
)
{
    GT_U32 i;    /* loop iterator */

    cpssOsBzero((GT_CHAR *)utilMultiPortGroupsBmp, sizeof(utilMultiPortGroupsBmp));
    cpssOsBzero((GT_CHAR *)utilMultiPortGroupsBmpEnable, sizeof(utilMultiPortGroupsBmpEnable));

    if(currResetCbNum == 0)
    {
        return GT_EMPTY;
    }

    for(i = 0; i < currResetCbNum; i++)
    {
        if(wrCpssResetLibFuncDb[i] != NULL)
        {
            (wrCpssResetLibFuncDb[i])();
        }
    }

    return GT_OK;
}

/**
* @internal wrCpssRegisterResetCb function
* @endinternal
*
* @brief   Register reset callback.
*
* @retval GT_OK                    - OK
* @retval GT_FULL                  - on full list
*/
GT_STATUS wrCpssRegisterResetCb
(
    wrCpssResetLibFuncPtr funcPtr
)
{
    GT_U32 i;    /* loop iterator */
    if(currResetCbNum == (MAX_RESET_CB_NUM_CNS - 1))
    {
        return GT_FULL;
    }

    /* look for already registered function */
    for(i = 0; i < currResetCbNum; i++)
    {
        if(wrCpssResetLibFuncDb[i] == funcPtr)
        {
            /* we allow to register it more then once ...
               but not need it twice in the DB */
            return GT_OK;
        }
    }

    wrCpssResetLibFuncDb[currResetCbNum] = funcPtr;

    currResetCbNum++;

    return GT_OK;
}

/**
* @internal utilCpssToWrFamilyConvert function
* @endinternal
*
* @brief   Convert CPSS family type enumeration value to wrapper used enumeration.
*
* @param[in] cpssFamily               - CPSS family type
*
* @param[out] wrFamilyPtrPtr           - (pointer to) pointer to wrapper family type
*
* @retval GT_OK                    - convert done
* @retval GT_FAIL                  - wrong family type, no conversion
*/
GT_STATUS utilCpssToWrFamilyConvert
(
    IN  CPSS_PP_FAMILY_TYPE_ENT      cpssFamily,
    OUT GT_UTIL_PP_FAMILY_TYPE_ENT  *wrFamilyPtrPtr
)
{
    switch(cpssFamily)
    {
        case CPSS_PP_FAMILY_TWISTC_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_TWISTC_E;
            break;
        case CPSS_PP_FAMILY_TWISTD_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_TWISTD_E;
            break;
        case CPSS_PP_FAMILY_SAMBA_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_SAMBA_E;
            break;
        case CPSS_PP_FAMILY_TIGER_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_TIGER_E;
            break;
        case CPSS_PP_FAMILY_SALSA_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_SALSA_E;
            break;
        case CPSS_PP_FAMILY_CHEETAH_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_CHEETAH_E;
            break;
        case CPSS_PP_FAMILY_CHEETAH2_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_CHEETAH2_E;
            break;
        case CPSS_PP_FAMILY_CHEETAH3_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_CHEETAH3_E;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_XCAT_E;
            break;
        case CPSS_PP_FAMILY_DXCH_LION_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_LION_E;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_XCAT2_E;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_LION2_E;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_BOBCAT2_E;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_BOBCAT3_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_ALDRIN_E;
            break;
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_AC3X_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_ALDRIN2_E;
            break;
        case CPSS_PP_FAMILY_PUMA_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_PUMA_E;
            break;
        case CPSS_PP_FAMILY_PUMA3_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_PUMA3_E;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_XCAT3_E;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            *wrFamilyPtrPtr = GT_UTIL_PP_FAMILY_DXCH_FALCON_E;
            break;

        default: return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal utilCpssToWrSubFamilyConvert function
* @endinternal
*
* @brief   Convert CPSS sub-family type enumeration value to wrapper used enumeration.
*
* @param[in] cpssSubFamily            - CPSS family type
*
* @param[out] wrSubFamilyPtr           - (pointer to) wrapper family type
*
* @retval GT_OK                    - convert done
* @retval GT_FAIL                  - wrong sub-family type, no conversion
*/
GT_STATUS utilCpssToWrSubFamilyConvert
(
    IN  CPSS_PP_SUB_FAMILY_TYPE_ENT      cpssSubFamily,
    OUT GT_UTIL_PP_SUB_FAMILY_TYPE_ENT  *wrSubFamilyPtr
)
{
    switch(cpssSubFamily)
    {
        case CPSS_PP_SUB_FAMILY_NONE_E:
            *wrSubFamilyPtr = GT_UTIL_PP_SUB_FAMILY_NONE_E;
            break;

        case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
            *wrSubFamilyPtr = GT_UTIL_PP_SUB_FAMILY_BOBCAT2_BOBK_E;
            break;

        default: return GT_FAIL;
    }

    return GT_OK;
}


