/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssDxChHsu.c
* @version   6
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/cpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>

#ifdef ASIC_SIMULATION
#include <stdio.h>
#endif

/**
* @internal wrCpssDxChHsuBlockSizeGet function
* @endinternal
*
* @brief   This function gets the memory size needed to export the required HSU
*         data block.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChHsuBlockSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_U32 hsuDataSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dataType = (CPSS_DXCH_HSU_DATA_TYPE_ENT)inArgs[0];
    /* call cpss api function */
    result = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", hsuDataSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHsuExport function
* @endinternal
*
* @brief   This function exports required HSU data block to survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, dataType.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
CMD_STATUS wrCpssDxChHsuExport
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8  *hsuBlockMemPtr;
    GT_STATUS result;
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_U32 hsuDataSize;
    GT_U32 iterationSize;
    GT_U32 origIterationSize;
    GT_U32 tempIterSize;
    GT_BOOL exportComplete;
    GT_UINTPTR   iter;
    GT_U32 iterationNumber;
    GT_U32 i;
    GT_U32 remainedSize = 0;
    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

#ifndef ASIC_SIMULATION
    GT_U32 *hsuPtr;
#else
    FILE *ptr_fp;
    GT_U8  *tempHsuBlockMemPtr;
    GT_U32      fileCount = 0;
#endif
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dataType = (CPSS_DXCH_HSU_DATA_TYPE_ENT)inArgs[0];
    iter = inArgs[1];
    iterationNumber = (GT_U32)inArgs[2];
    result =  cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
#ifdef ASIC_SIMULATION
    hsuBlockMemPtr = cmdOsMalloc(hsuDataSize*sizeof(GT_U8));
    cmdOsMemSet(hsuBlockMemPtr, 0, hsuDataSize*sizeof(GT_U8));
    tempHsuBlockMemPtr = hsuBlockMemPtr;
#else
    result = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);

    cpssOsPrintf("hsu memory started at %x\n",hsuPtr);
    hsuBlockMemPtr = (GT_U8*)hsuPtr;
    if (result != GT_OK)
    {
        return result;
    }
#endif
    origIterationSize = hsuDataSize/iterationNumber;
    remainedSize = hsuDataSize%iterationNumber;
    iterationSize  = origIterationSize;
    tempIterSize   = origIterationSize;
    /* call cpss api function */
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    for( i = 0; i < iterationNumber; i++)
    {
        if (i == iterationNumber - 1)
        {
            /* last iteration size */
            iterationSize += remainedSize;
            tempIterSize = iterationSize;
        }
        result = cpssDxChHsuExport(dataType,
                                   &iter,
                                   &iterationSize,
                                   hsuBlockMemPtr,
                                   &exportComplete);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "");
            return CMD_OK;
        }
        if (i != iterationNumber -1)
        {
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + (tempIterSize - iterationSize));
            tempIterSize = origIterationSize + iterationSize;
            iterationSize = tempIterSize;
        }
        else
        {
            /* last iteration is done. Promote hsuBlockMemPtr for case of another
               export call */
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + tempIterSize);
        }
    }
    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    #ifdef ASIC_SIMULATION
    if((ptr_fp = fopen("hsu_export.txt", "wb")) == NULL)
    {
        cpssOsPrintf("\n fopen hsuexp is fail!!!!!!!! = %d\n");
    }
    else
    {
        fileCount = (GT_U32)fwrite(tempHsuBlockMemPtr, hsuDataSize*sizeof(GT_U8), 1, ptr_fp);
        if (fileCount != 1)
        {
            cpssOsPrintf("fwrite is fail\n");
        }
        fclose(ptr_fp);
        ptr_fp = NULL;
    }
    cmdOsFree(tempHsuBlockMemPtr);
    #endif
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",iter,iterationSize,exportComplete);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHsuImport function
* @endinternal
*
* @brief   This function imports required HSU data block from survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
CMD_STATUS wrCpssDxChHsuImport
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8  *hsuBlockMemPtr;
    GT_STATUS result = GT_OK;
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_U32 hsuDataSize;
    GT_BOOL importComplete;
    GT_UINTPTR  iter;
    GT_U32 iterationNumber;
    GT_U32 i;
    GT_U32 iterationSize;
    GT_U32 origIterationSize;
    GT_U32 tempIterSize;
    GT_U32 remainedSize = 0;
    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

#ifndef ASIC_SIMULATION
    GT_U32 *hsuPtr;
#else
    FILE *ptr_fp;
    GT_U32 fileSize = 0;
    GT_U8  *tempHsuBlockMemPtr;
#endif
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dataType = (CPSS_DXCH_HSU_DATA_TYPE_ENT)inArgs[0];
    hsuDataSize = (GT_U32)inArgs[2];
    iter = inArgs[1];
    iterationNumber = (GT_U32)inArgs[3];

#ifndef ASIC_SIMULATION
    result = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);
    cpssOsPrintf("hsu memory started at %x\n",hsuPtr);
    hsuBlockMemPtr = (GT_U8*)hsuPtr;
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
#else
    ptr_fp = fopen ( "hsu_export.txt" , "rb" );
    if (ptr_fp == NULL)
    {
        cpssOsPrintf("hsuexp is not exist\n");
    }
    else
    {
        /* obtain file size:*/
        fseek (ptr_fp , 0 , SEEK_END);
        fileSize = ftell (ptr_fp);
        rewind (ptr_fp);
    }
    /* allocate memory to contain the whole file: */
    hsuBlockMemPtr = cmdOsMalloc(fileSize*sizeof(GT_U8));
    cmdOsMemSet(hsuBlockMemPtr, 0, fileSize*sizeof(GT_U8));
    tempHsuBlockMemPtr = hsuBlockMemPtr;
    fclose (ptr_fp);
#endif

    origIterationSize = hsuDataSize/iterationNumber;
    remainedSize = hsuDataSize%iterationNumber;
    iterationSize  = origIterationSize;
    tempIterSize   = origIterationSize;
    /* call cpss api function */
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    for( i = 0; i < iterationNumber; i++)
    {
        if (i == iterationNumber - 1)
        {
            /* last iteration size */
            iterationSize += remainedSize;
            tempIterSize = iterationSize;
        }
        result = cpssDxChHsuImport(dataType,
                                     &iter,
                                     &iterationSize,
                                     hsuBlockMemPtr,
                                     &importComplete);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "");
            return CMD_OK;
        }
        if (i != iterationNumber-1)
        {
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + (tempIterSize - iterationSize));
            tempIterSize = origIterationSize + iterationSize;
            iterationSize = tempIterSize;
        }
        else
        {
            /* last iteration is done. Promote hsuBlockMemPtr for case of another
               import call */
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + tempIterSize );
        }
    }

    /* allocated space need to be freed after Importing the data */
#ifdef ASIC_SIMULATION
    cmdOsFree(tempHsuBlockMemPtr);
#endif
    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",iter,iterationSize,importComplete);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHsuExport_1 function
* @endinternal
*
* @brief   This function exports required HSU data block to survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, dataType.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
CMD_STATUS wrCpssDxChHsuExport_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
#ifndef ASIC_SIMULATION
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_UINTPTR                    iter;
    GT_U32                        hsuBlockMemSize;
    GT_U8                         *hsuBlockPtr;
    GT_BOOL                       exportComplete;
    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

    GT_U32 *hsuPtr;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dataType = (CPSS_DXCH_HSU_DATA_TYPE_ENT)inArgs[0];
    iter     = inArgs[1];
    hsuBlockMemSize  = (GT_U32)inArgs[2];
    hsuBlockPtr = (GT_U8 *)inArgs[3];
    result = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);
    if (result != GT_OK)
    {
        return result;
    }
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    result = cpssDxChHsuExport(dataType,
                                 &iter,
                                 &hsuBlockMemSize,
                                 hsuBlockPtr,
                                 &exportComplete);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",iter,hsuBlockMemSize,exportComplete);
#else
    result = GT_FAIL;
    cpssOsPrintf(" not working with simulation\n");
    galtisOutput(outArgs, result, "");
#endif

    return CMD_OK;
}


/**
* @internal wrCpssDxChHsuImport_1 function
* @endinternal
*
* @brief   This function imports required HSU data block from survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
CMD_STATUS wrCpssDxChHsuImport_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result = GT_OK;
#ifndef ASIC_SIMULATION
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_UINTPTR                    iter;
    GT_U32                        hsuBlockMemSize;
    GT_U8                         *hsuBlockPtr;
    GT_BOOL                       importComplete;
    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

    GT_U32 *hsuPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dataType = (CPSS_DXCH_HSU_DATA_TYPE_ENT)inArgs[0];
    iter            = inArgs[1];
    hsuBlockMemSize = (GT_U32)inArgs[2];
    hsuBlockPtr = (GT_U8 *)inArgs[3];
    result = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    result = cpssDxChHsuImport(dataType,
                                 &iter,
                                 &hsuBlockMemSize,
                                 hsuBlockPtr,
                                 &importComplete);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",iter,hsuBlockMemSize,importComplete);
#else
    result = GT_FAIL;
    cpssOsPrintf(" not working with simulation\n");
    galtisOutput(outArgs, result, "");
#endif

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
        {"cpssDxChHsuBlockSizeGet",
         &wrCpssDxChHsuBlockSizeGet,
         1, 0},
        {"cpssDxChHsuExport",
         &wrCpssDxChHsuExport,
         3, 0},
        {"cpssDxChHsuImport",
         &wrCpssDxChHsuImport,
         4, 0},
        {"cpssDxChHsuBaseExport",
         &wrCpssDxChHsuExport_1,
         4, 0},
        {"cpssDxChHsuBaseImport",
         &wrCpssDxChHsuImport_1,
         4, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChHsu function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChHsu
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


