/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssDxChHsuUT.c
*
* @brief Unit tests for HSU feature
*
* @version   13
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/cpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Deletes LpmDBs and Virtual Routers that were created by LPM tests and creates
   LPM and VR */
#if 0
static void deleteLpmDBsAndVRs(GT_VOID);
#endif

/* defines */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsuExport
(
    IN     CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    INOUT  GT_U32                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
)
*/
#if 0
UTF_TEST_CASE_MAC(cpssDxChHsuExport)
{
/*
(no test pattern, no device iteration)
1. Create 1000 ipv4 uc prefixes . Expected: GT_OK.
2. Call to size get API (
)and saved returned size. Expected: GT_OK.
3. Perform  cpssDxChHsuExport with single iteration. Expected: GT_OK.
4. Perform cpssDxChIpLpmDbExport with different iteration size and iteration number. Expected: GT_OK.
5. Delete all prefixes. Expected: GT_OK.
*/
    GT_STATUS                       st = GT_OK;
    GT_UINTPTR                      iter = 0;
    GT_IPADDR                       ipAddr;
    GT_U32                          numOfAddedPrefixes = 0;
    GT_U32                          i = 0;
    GT_U32                          iterationNumber = 0;
    GT_U32                          iterationCounter = 0;
    GT_U32                          remainedSize = 0;
    GT_U32                          hsuDataSize = 0;
    GT_U32                          iterationSize = 0;
    GT_U32                          size = 0;
    GT_U32                          tempHsuDataSize = 0;
    CPSS_DXCH_HSU_DATA_TYPE_ENT     dataType = CPSS_DXCH_HSU_DATA_TYPE_ALL_E;
    GT_U8                           *hsuBlockMemPtr0 = NULL;
    GT_U8                           *hsuBlockMemPtr1 = NULL;
    GT_U8                           *tempHsuBlockMemPtr = NULL;
    GT_BOOL                         exportComplete = GT_FALSE;
    GT_BOOL                         isEqual = GT_FALSE;
    GT_IPADDR                       lastIpAddrAddedPtr;
    CPSS_DXCH_CFG_DEV_INFO_STC      devInfo;
    GT_BOOL                         isWaImplemented;
    GT_U32                          numOfPrefixesToAdd;

    cpssDxChCfgDevInfoGet(0, &devInfo);
    /* Skip this test for Cheetah and Cheetah2,3 */
    if (devInfo.genDevInfo.devFamily <= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        prvUtfSkipTestsSet();
        return;
    }

    if (prvUtfIsPbrModeUsed())
    {
        /* The device is configured to use Policy Based Routing */
        numOfPrefixesToAdd = 350;
    }
    else
    {
        numOfPrefixesToAdd = 1000;
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* reduce time of test */
        numOfPrefixesToAdd = 50;
    }

    /* This test is intended to check HSU export functionality.
       At first export is performed with one iteration and data saved in special allocated memory.
       Further it does many times by different steps with multiple iterations. After each successfull
       export with multiple iterations current HSU memory contents is compared with the one of single
       iteration.
    */

    deleteLpmDBsAndVRs();
    cpssOsMemSet((GT_VOID*) &(ipAddr), 0, sizeof(ipAddr));

    /* 1. Create 1000 ipv4 uc prefixes . Expected: GT_OK. */
    st = cpssDxChIpLpmIpv4UcPrefixAddMany(0,
                                          0,
                                          ipAddr,
                                          11,
                                          numOfPrefixesToAdd,
                                          &lastIpAddrAddedPtr,
                                          &numOfAddedPrefixes);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAddMany");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfAddedPrefixes, "cpssDxChIpLpmIpv4UcPrefixAddMany");

    cpssDxChPpHwImplementWaGet(0, CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E, &isWaImplemented);
    if ((devInfo.genDevInfo.cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E) &&
        isWaImplemented == GT_FALSE)
    {
        systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_TRUE;
        systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_TRUE;

        /* 2. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
        st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
        tempHsuDataSize = hsuDataSize;

        hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
        cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

        hsuBlockMemPtr1 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr1, "cpssOsMalloc: Memory allocation error.");
        cpssOsMemSet(hsuBlockMemPtr1, 0, hsuDataSize*sizeof(GT_U8));

        /* 3. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
        st = cpssDxChHsuExport(dataType,
                               &iter,
                               &tempHsuDataSize,
                               hsuBlockMemPtr0,
                               &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
        if (st != GT_OK)
        {
            return;
        }

        /* 4. Perform cpssDxChIpLpmDbExport with different iteration size and iteration number. Expected: GT_OK.*/

        for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
        {
            iter = 0;
            tempHsuDataSize = size;
            iterationSize = size;
            if (hsuDataSize < size)
            {
                iterationNumber = 1;
                remainedSize = 0;
                tempHsuDataSize = hsuDataSize;
            }
            else
            {
                iterationNumber = hsuDataSize/size;
                remainedSize = hsuDataSize%size;
            }
            cpssOsMemSet(hsuBlockMemPtr1, 0, hsuDataSize*sizeof(GT_U8));
            tempHsuBlockMemPtr = hsuBlockMemPtr1;
            for( i = 0; i < iterationNumber; i++)
            {
                if (i == iterationNumber - 1)
                {
                    /* last iteration size */
                    tempHsuDataSize = tempHsuDataSize + remainedSize;
                    iterationSize = tempHsuDataSize;
                }
                st = cpssDxChHsuExport(dataType,
                                       &iter,
                                       &tempHsuDataSize,
                                       tempHsuBlockMemPtr,
                                       &exportComplete);

                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
                if (st != GT_OK)
                {
                    return;
                }

                if (i != iterationNumber -1)
                {
                    tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                    tempHsuDataSize = size + tempHsuDataSize;
                    iterationSize = tempHsuDataSize;
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
                }
            }
            /* iterator should be 0*/
             UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
             UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
             UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, exportComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);

             /* compare export memory created by one iteration with export memory of multiple iterations*/
             isEqual = (0 == cpssOsMemCmp((GT_VOID*) hsuBlockMemPtr0,
                                          (GT_VOID*) hsuBlockMemPtr1,
                                          sizeof(hsuDataSize))) ? GT_TRUE : GT_FALSE;
             UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual, "memories is not equal:iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
        }
        cpssOsFree(hsuBlockMemPtr0);
        cpssOsFree(hsuBlockMemPtr1);
    }

    systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_FALSE;
    systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_FALSE;

    /* 5. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
    st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
    tempHsuDataSize = hsuDataSize;

    hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
    cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

    hsuBlockMemPtr1 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr1, "cpssOsMalloc: Memory allocation error.");
    cpssOsMemSet(hsuBlockMemPtr1, 0, hsuDataSize*sizeof(GT_U8));

    /* 6. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
    st = cpssDxChHsuExport(dataType,
                           &iter,
                           &tempHsuDataSize,
                           hsuBlockMemPtr0,
                           &exportComplete);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
    if (st != GT_OK)
    {
        return;
    }

    /* 7. Perform cpssDxChIpLpmDbExport with different iteration size and iteration number. Expected: GT_OK.*/

    for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
    {
        iter = 0;
        tempHsuDataSize = size;
        iterationSize = size;
        if (hsuDataSize < size)
        {
            iterationNumber = 1;
            remainedSize = 0;
            tempHsuDataSize = hsuDataSize;
        }
        else
        {
            iterationNumber = hsuDataSize/size;
            remainedSize = hsuDataSize%size;
        }
        cpssOsMemSet(hsuBlockMemPtr1, 0, hsuDataSize*sizeof(GT_U8));
        tempHsuBlockMemPtr = hsuBlockMemPtr1;
        for( i = 0; i < iterationNumber; i++)
        {
            if (i == iterationNumber - 1)
            {
                /* last iteration size */
                tempHsuDataSize = tempHsuDataSize + remainedSize;
                iterationSize = tempHsuDataSize;
            }
            st = cpssDxChHsuExport(dataType,
                                   &iter,
                                   &tempHsuDataSize,
                                   tempHsuBlockMemPtr,
                                   &exportComplete);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
            if (st != GT_OK)
            {
                return;
            }

            if (i != iterationNumber -1)
            {
                tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                tempHsuDataSize = size + tempHsuDataSize;
                iterationSize = tempHsuDataSize;
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
            }
        }
        /* iterator should be 0*/
         UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
         UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, exportComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);

         /* compare export memory created by one iteration with export memory of multiple iterations*/
         isEqual = (0 == cpssOsMemCmp((GT_VOID*) hsuBlockMemPtr0,
                                      (GT_VOID*) hsuBlockMemPtr1,
                                      sizeof(hsuDataSize))) ? GT_TRUE : GT_FALSE;
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual, "memories is not equal:iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
    }

    /* 8. Delete all prefixes. Expected: GT_OK. */
    st = cpssDxChIpLpmIpv4UcPrefixesFlush(0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixesFlush");

    systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;

    cpssOsFree(hsuBlockMemPtr0);
    cpssOsFree(hsuBlockMemPtr1);
}

/*
GT_STATUS cpssDxChHsuImport
(
    IN     CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    INOUT  GT_U32                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHsuImport)
{
/*
(no test pattern, no device iteration)
1. Create 1000 ipv4 uc prefixes. Expected: GT_OK.
2. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK.
3. Perform  cpssDxChHsuExport with single iteration. Expected: GT_OK.
4. Perform cpssDxChHsuImport with different iteration size and iteration number. Expected: GT_OK.
5. Delete all prefixes. Expected: GT_OK.
*/
    GT_STATUS                        st = GT_OK;
    GT_UINTPTR                       iter = 0;
    GT_IPADDR                        ipAddr;
    GT_U32                           numOfAddedPrefixes = 0;
    GT_U32                           i = 0;
    GT_U32                           iterationNumber = 0;
    GT_U32                           iterationCounter = 0;
    GT_U32                           remainedSize = 0;
    GT_U32                           hsuDataSize = 0;
    GT_U32                           iterationSize = 0;
    GT_U32                           size = 0;
    GT_U32                           tempHsuDataSize = 0;
    CPSS_DXCH_HSU_DATA_TYPE_ENT      dataType = CPSS_DXCH_HSU_DATA_TYPE_ALL_E;
    GT_U8                            *hsuBlockMemPtr0 = NULL;
    GT_U8                            *tempHsuBlockMemPtr = NULL;
    GT_BOOL                          exportComplete = GT_FALSE;
    GT_BOOL                          importComplete = GT_FALSE;
    GT_IPADDR                        lastIpAddrAddedPtr;
    CPSS_DXCH_CFG_DEV_INFO_STC       devInfo;
    GT_BOOL                          isWaImplemented;
    GT_U32                           numOfPrefixesToAdd;
    GT_U8   dev = 0;

    cpssDxChCfgDevInfoGet(dev, &devInfo);
    /* Skip this test for Cheetah and Cheetah2,3 */
    if (devInfo.genDevInfo.devFamily <= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        prvUtfSkipTestsSet();
        return;
    }
    if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
    {
        /* LPM HSU is not implemented yet, force the test to fail */
        CPSS_TBD_BOOKMARK
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_NOT_IMPLEMENTED, "cpssDxChHsuImport");
        return;
    }

    if (prvUtfIsPbrModeUsed())
    {
        /* The device is configured to use Policy Based Routing */
        numOfPrefixesToAdd = 350;
    }
    else
    {
        numOfPrefixesToAdd = 1000;
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* reduce time of test */
        numOfPrefixesToAdd = 50;
    }

    /* This test is intended to check HSU export functionality.
       At first export is performed with one iteration and data saved in special allocated memory.
       Further it does many times by different steps with multiple iterations. After each successfull
       export with multiple iterations current HSU memory contents is compared with the one of single
       iteration.
    */
    cpssOsMemSet((GT_VOID*) &(ipAddr), 0, sizeof(ipAddr));

    /* 1. Create 1000 ipv4 uc prefixes for lpmDbId=0 and vrId = 0 . Expected: GT_OK. */
    st = cpssDxChIpLpmIpv4UcPrefixAddMany(0,
                                          0,
                                          ipAddr,
                                          11,
                                          numOfPrefixesToAdd,
                                          &lastIpAddrAddedPtr,
                                          &numOfAddedPrefixes);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAddMany");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfAddedPrefixes, "cpssDxChIpLpmIpv4UcPrefixAddMany");

    cpssDxChPpHwImplementWaGet(dev, CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E, &isWaImplemented);
    if ((devInfo.genDevInfo.cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E) &&
        isWaImplemented == GT_FALSE)
    {
        systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_TRUE;
        systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_TRUE;

        /* 2. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
        st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
        tempHsuDataSize = hsuDataSize;

        hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
        cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

        /* 3. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
        st = cpssDxChHsuExport(dataType,
                                 &iter,
                                 &tempHsuDataSize,
                                 hsuBlockMemPtr0,
                                 &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
        if (st != GT_OK)
        {
            return;
        }

        /* remove all ip configuration before import */
        st = cpssDxChIpLpmVirtualRouterDel(0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmVirtualRouterDelete");

        /* 4. Perform cpssDxChHsuImport with different iteration size and iteration number. Expected: GT_OK.*/
        tempHsuDataSize = hsuDataSize;
        iter = 0;
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

        for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
        {
            iter = 0;
            tempHsuDataSize = size;
            iterationSize = size;
            if (hsuDataSize < size)
            {
                iterationNumber = 1;
                remainedSize = 0;
                tempHsuDataSize = hsuDataSize;
            }
            else
            {
                iterationNumber = hsuDataSize/size;
                remainedSize = hsuDataSize%size;
            }
            tempHsuBlockMemPtr = hsuBlockMemPtr0;
            for( i = 0; i < iterationNumber; i++)
            {
                if (i == iterationNumber - 1)
                {
                    /* last iteration size */
                    tempHsuDataSize = tempHsuDataSize + remainedSize;
                    iterationSize = tempHsuDataSize;
                }
                st = cpssDxChHsuImport(dataType,
                                       &iter,
                                       &tempHsuDataSize,
                                       tempHsuBlockMemPtr,
                                       &importComplete);

                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
                if (st != GT_OK)
                {
                    return;
                }

                if (i != iterationNumber -1)
                {
                    tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                    tempHsuDataSize = size + tempHsuDataSize;
                    iterationSize = tempHsuDataSize;
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
                }
            }
            /* iterator should be 0*/
             UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
             UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
             UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, importComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
             /* remove all ip configuration before import */
             st = cpssDxChIpLpmIpv4UcPrefixesFlush(0, 0);
             UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixesFlush");
             if (hsuDataSize > size)
             {
                 /* Delete all virtual routers (virtual router 0) . Expected: GT_OK. */
                 st = cpssDxChIpLpmVirtualRouterDel(0, 0);
                 UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmVirtualRouterDelete");
             }
        }
        cpssOsFree(hsuBlockMemPtr0);
    }
    systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_FALSE;
    systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_FALSE;

    /* 5. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
    st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
    tempHsuDataSize = hsuDataSize;

    hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
    cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

    /* 6. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
    st = cpssDxChHsuExport(dataType,
                             &iter,
                             &tempHsuDataSize,
                             hsuBlockMemPtr0,
                             &exportComplete);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
    if (st != GT_OK)
    {
        return;
    }

    /* remove all ip configuration before import */
    st = cpssDxChIpLpmVirtualRouterDel(0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmVirtualRouterDelete");

    /* 7. Perform cpssDxChHsuImport with different iteration size and iteration number. Expected: GT_OK.*/
    tempHsuDataSize = hsuDataSize;
    iter = 0;
    systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

    for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
    {
        iter = 0;
        tempHsuDataSize = size;
        iterationSize = size;
        if (hsuDataSize < size)
        {
            iterationNumber = 1;
            remainedSize = 0;
            tempHsuDataSize = hsuDataSize;
        }
        else
        {
            iterationNumber = hsuDataSize/size;
            remainedSize = hsuDataSize%size;
        }
        tempHsuBlockMemPtr = hsuBlockMemPtr0;
        for( i = 0; i < iterationNumber; i++)
        {
            if (i == iterationNumber - 1)
            {
                /* last iteration size */
                tempHsuDataSize = tempHsuDataSize + remainedSize;
                iterationSize = tempHsuDataSize;
            }
            st = cpssDxChHsuImport(dataType,
                                   &iter,
                                   &tempHsuDataSize,
                                   tempHsuBlockMemPtr,
                                   &importComplete);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
            if (st != GT_OK)
            {
                return;
            }

            if (i != iterationNumber -1)
            {
                tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                tempHsuDataSize = size + tempHsuDataSize;
                iterationSize = tempHsuDataSize;
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
            }
        }
        /* iterator should be 0*/
         UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
         UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, importComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
         /* remove all ip configuration before import */
         st = cpssDxChIpLpmIpv4UcPrefixesFlush(0, 0);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixesFlush");
         if (hsuDataSize > size)
         {
             /* Delete all virtual routers (virtual router 0) . Expected: GT_OK. */
             st = cpssDxChIpLpmVirtualRouterDel(0, 0);
             UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmVirtualRouterDelete");
         }
    }

    /* 8. Delete all prefixes. Expected: GT_OK. */
    st = cpssDxChIpLpmIpv4UcPrefixesFlush(0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixesFlush");

    systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;

    cpssOsFree(hsuBlockMemPtr0);
}
#endif

#if 0
extern GT_VOID waitForPacketsToEnd(GT_VOID);

static GT_U32 packetId  = 0;
static GT_U32 packetId2 = 0;
/*
GT_STATUS cpssDxChHsuExportImportGlobalDataSerdesAndTxGeneratorOnly
(
    IN     CPSS_DXCH_HSU_DATA_TYPE_ENT      dataType,
    INOUT  GT_U32                           *iteratorPtr,
    INOUT  GT_U32                           *hsuBlockMemSizePtr,
    IN     GT_U8                            *hsuBlockMemPtr,
    OUT    GT_BOOL                          *importCompletePtr
)
*/
static void cpssDxChHsuExportImportGlobalDataSerdesAndTxGeneratorOnly(
    IN GT_BOOL *testNeedRecovaryPtr
)
{
/*
(no test pattern, no device iteration)
1. Create serdes. Expected: GT_OK.
2. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK.
3. Perform  cpssDxChHsuExport with single iteration. Expected: GT_OK.
4. Change serdes. Expected: GT_OK.
4. Perform cpssDxChHsuImport with different iteration size and iteration number.
   Expected: GT_OK. and serdes values as before of Export
5. Add packet to Tx Generator using: cpssDxChNetIfSdmaTxGeneratorPacketAdd. Expected: GT_OK.
6. Perform  cpssDxChHsuExport Expected: GT_OK.
7. Remove packet. Expected: GT_OK.
8. Perform cpssDxChHsuImport
   Expected: GT_OK. and packets values as before of Export

NOTE: all cpssDxChNetIfSdmaTxGeneratorPacketAdd/Remove manupulations are to test sw import.
      Since there is no real swReset the code do not check that the SW is equal to HW.
      After a remove the entry is deleted from HW but after the import the entry is
      returned to SW but not to hw.

      Same with cpssDxChPortSerdesTuningSet manipulations

*/
    GT_STATUS                        st = GT_OK;
    GT_UINTPTR                       iter = 0;
    GT_U32                           i = 0;
    GT_U32                           iterationNumber = 0;
    GT_U32                           iterationCounter = 0;
    GT_U32                           remainedSize = 0;
    GT_U32                           hsuDataSize = 0;
    GT_U32                           iterationSize = 0;
    GT_U32                           size = 0;
    GT_U32                           tempHsuDataSize = 0;
    CPSS_DXCH_HSU_DATA_TYPE_ENT      dataType = CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E;
    GT_U8                            *hsuBlockMemPtr0 = NULL;
    GT_U8                            *tempHsuBlockMemPtr = NULL;
    GT_BOOL                          exportComplete = GT_FALSE;
    GT_BOOL                          importComplete = GT_FALSE;
    GT_BOOL                          isWaImplemented;
    GT_U32                           k;
    GT_U8                            devNum;

    CPSS_DXCH_PORT_SERDES_TUNE_STC  tuneValues;
    CPSS_DXCH_PORT_SERDES_TUNE_STC  tuneValuesGet;
    CPSS_DXCH_PORT_SERDES_TUNE_STC  tuneValuesDef;

    /* txGenerator config */
    GT_PORT_GROUPS_BMP              portGroupsBmp       = 1;
    CPSS_DXCH_NET_TX_PARAMS_STC     packetParams;
    GT_U8                           packetData[1024];
    GT_U32                          packetDataLength;
    GT_U8                           txQueue             = 3;
    CPSS_DXCH_CFG_DEV_INFO_STC       devInfo;
    GT_U32      validPorts[1];
    GT_PHYSICAL_PORT_NUM    phyPortNum;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    devNum     = utfFirstDevNumGet();
    packetId   = 0;
    packetId2  = 0;

    st = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChCfgDevInfoGet");

    *testNeedRecovaryPtr = GT_TRUE;

    /* This test is intended to check HSU export functionality.
       At first export is performed with one iteration and data saved in special allocated memory.
       Further it does many times by different steps with multiple iterations. After each successfull
       export with multiple iterations current HSU memory contents is compared with the one of single
       iteration.
    */

    cpssOsBzero((GT_VOID*)&tuneValues, sizeof(tuneValues));
    cpssOsBzero((GT_VOID*)&tuneValuesGet, sizeof(tuneValuesGet));

    /* serdes configuration */
    tuneValues.dfe=1;
    tuneValues.ffeR=1;
    tuneValues.ffeC=1;
    tuneValues.sampler=1;
    tuneValues.sqlch=1;

    tuneValues.txEmphAmp=1;
    tuneValues.txAmp=1;
    tuneValues.txAmpAdj=1;
    tuneValues.ffeS=1;
    tuneValues.txEmphEn=GT_TRUE;

    tuneValues.txEmph1=1;
    tuneValues.align90=1;
    tuneValues.txEmphEn1=1;
    tuneValues.txAmpShft=1;
    tuneValues.dfeValsArray[0]=1;
    tuneValues.dfeValsArray[1]=2;
    tuneValues.dfeValsArray[2]=3;
    tuneValues.dfeValsArray[3]=4;
    tuneValues.dfeValsArray[4]=5;
    tuneValues.dfeValsArray[5]=6;

    /* values for restore */
    tuneValuesDef.dfe=2;
    tuneValuesDef.ffeR=2;
    tuneValuesDef.ffeC=2;
    tuneValuesDef.sampler=0;
    tuneValuesDef.sqlch=0;

    tuneValuesDef.txEmphAmp=0;
    tuneValuesDef.txAmp=0;
    tuneValuesDef.txAmpAdj=0;
    tuneValuesDef.ffeS=0;
    tuneValuesDef.txEmphEn=GT_FALSE;

    tuneValuesDef.txEmph1=0;
    tuneValuesDef.align90=0;
    tuneValuesDef.txEmphEn1=0;
    tuneValuesDef.txAmpShft=0;
    tuneValuesDef.dfeValsArray[0]=0;
    tuneValuesDef.dfeValsArray[1]=0;
    tuneValuesDef.dfeValsArray[2]=0;
    tuneValuesDef.dfeValsArray[3]=0;
    tuneValuesDef.dfeValsArray[4]=0;
    tuneValuesDef.dfeValsArray[5]=0;

    /* Tx Generator packet add */
    cpssOsMemSet(&packetParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    cpssOsMemSet(&packetData, 0, sizeof(packetData));
    packetDataLength    = 64;

    packetParams.packetIsTagged = GT_FALSE;
    packetParams.sdmaInfo.recalcCrc = GT_TRUE;
    packetParams.sdmaInfo.txQueue = txQueue;
    packetParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    packetParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = 0;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
    packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;
    packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

    /* get ports with MAC */
    prvUtfValidPortsGet(devNum ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
        0  /*start port */, &validPorts[0] , 1);
    phyPortNum = validPorts[0];

    cpssDxChPpHwImplementWaGet(devNum, CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E, &isWaImplemented);
    if ((devInfo.genDevInfo.cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E) &&
        isWaImplemented == GT_FALSE)
    {
        systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_TRUE;
        systemRecoveryInfo.systemRecoveryMode.continuousTx = GT_TRUE;
        systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_TRUE;

        st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValues);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

        st = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            txQueue,GT_FALSE,0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChNetIfSdmaTxGeneratorEnable");

        st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupsBmp,
                                            &packetParams, packetData,
                                            packetDataLength,&packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;
        packetParams.dsaParam.dsaInfo.fromCpu.tc = 4;

        st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupsBmp,
                                            &packetParams, packetData,
                                            packetDataLength,&packetId2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        #ifdef ASIC_SIMULATION
            /* let simulation start processing the SDMA thread before continue
                to configurations */
            cpssOsTimerWkAfter(100);
        #endif /*ASIC_SIMULATION*/

        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
        packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;

        /* 2. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
        st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
        tempHsuDataSize = hsuDataSize;

        hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
        cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

        /* 3. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
        st = cpssDxChHsuExport(dataType,
                                 &iter,
                                 &tempHsuDataSize,
                                 hsuBlockMemPtr0,
                                 &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
        UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
        if (st != GT_OK)
        {
            goto restore;
        }

        st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesDef);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

        st = cpssDxChPortSerdesTuningGet(devNum,phyPortNum,2,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningGet");

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.dfe,
                                  tuneValuesGet.dfe,
                                  "get another tuneValuesGet.dfe:%d than was set tuneValuesDef.dfe: %d",
                                   tuneValuesGet.dfe,tuneValuesDef.dfe);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeR,
                                  tuneValuesGet.ffeR,
                                  "get another tuneValuesGet.ffeR:%d than was set tuneValuesDef.ffeR: %d",
                                   tuneValuesGet.ffeR,tuneValuesDef.ffeR);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeC,
                                  tuneValuesGet.ffeC,
                                  "get another tuneValuesGet.ffeC:%d than was set tuneValuesDef.ffeC: %d",
                                   tuneValuesGet.ffeC,tuneValuesDef.ffeC);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.sampler,
                                  tuneValuesGet.sampler,
                                  "get another tuneValuesGet.sampler:%d than was set tuneValuesDef.sampler: %d",
                                   tuneValuesGet.sampler,tuneValuesDef.sampler);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.sqlch,
                                  tuneValuesGet.sqlch,
                                  "get another tuneValuesGet.sqlch:%d than was set tuneValuesDef.sqlch: %d",
                                   tuneValuesGet.sqlch,tuneValuesDef.sqlch);

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphAmp,
                                  tuneValuesGet.txEmphAmp,
                                  "get another tuneValuesGet.txEmphAmp:%d than was set tuneValuesDef.txEmphAmp: %d",
                                   tuneValuesGet.txEmphAmp,tuneValuesDef.txEmphAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmp,
                                  tuneValuesGet.txAmp,
                                  "get another tuneValuesGet.txAmp:%d than was set tuneValuesDef.txAmp: %d",
                                   tuneValuesGet.txAmp,tuneValuesDef.txAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmpAdj,
                                  tuneValuesGet.txAmpAdj,
                                  "get another tuneValuesGet.txAmpAdj:%d than was set tuneValuesDef.txAmpAdj: %d",
                                   tuneValuesGet.txAmpAdj,tuneValuesDef.txAmpAdj);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeS,
                                  tuneValuesGet.ffeS,
                                  "get another tuneValuesGet.ffeS:%d than was set tuneValuesDef.ffeS: %d",
                                   tuneValuesGet.ffeS,tuneValuesDef.ffeS);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphEn,
                                  tuneValuesGet.txEmphEn,
                                  "get another tuneValuesGet.txEmphEn:%d than was set tuneValuesDef.txEmphEn: %d",
                                   tuneValuesGet.txEmphEn,tuneValuesDef.txEmphEn);


        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmph1,
                                  tuneValuesGet.txEmph1,
                                  "get another tuneValuesGet.txEmph1:%d than was set tuneValuesDef.txEmph1: %d",
                                   tuneValuesGet.txEmph1,tuneValuesDef.txEmph1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.align90,
                                  tuneValuesGet.align90,
                                  "get another tuneValuesGet.align90:%d than was set tuneValuesDef.align90: %d",
                                   tuneValuesGet.align90,tuneValuesDef.align90);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphEn1,
                                  tuneValuesGet.txEmphEn1,
                                  "get another tuneValuesGet.txEmphEn1:%d than was set tuneValuesDef.txEmphEn1: %d",
                                   tuneValuesGet.txEmphEn1,tuneValuesDef.txEmphEn1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmpShft,
                                  tuneValuesGet.txAmpShft,
                                  "get another tuneValuesGet.txAmpShft:%d than was set tuneValuesDef.txAmpShft: %d",
                                   tuneValuesGet.txAmpShft,tuneValuesDef.txAmpShft);
        for(k=0;k<6;k++)
        {
         UTF_VERIFY_EQUAL4_STRING_MAC(tuneValuesDef.dfeValsArray[k],
                                      tuneValuesGet.dfeValsArray[k],
                                      "get another tuneValuesGet.dfeValsArray[%d]:%d than was set tuneValuesDef.dfeValsArray[%d]: %d",
                                      k,tuneValuesGet.dfeValsArray[k],k,tuneValuesDef.dfeValsArray[k]);
        }

        /* remove exist packet */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* try to remove again the packet - get GT_BAD_PARAM */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /* remove exist packet */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        st = cpssDxChNetIfSdmaTxGeneratorDisable(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,txQueue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChNetIfSdmaTxGeneratorDisable");

        /* 4. Perform cpssDxChHsuImport with different iteration size and iteration number. Expected: GT_OK.*/
        tempHsuDataSize = hsuDataSize;
        iter = 0;
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

#if 0
        /* IMPORT WITH SINGLE ITERATION */

        /* 3. Perform  cpssDxChHsuImport with single iteration. The imported data is saved in  tempHsuBlockMemPtr. Expected: GT_OK.*/
        st = cpssDxChHsuImport(dataType,
                               &iter,
                               &tempHsuDataSize,
                               hsuBlockMemPtr0,
                               &importComplete);

        /* iterator should be 0*/
         UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
         UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, importComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);

        if (st != GT_OK)
        {
            cpssOsFree(hsuBlockMemPtr0);
            return;
        }

        st = cpssDxChPortSerdesTuningGet(devNum,phyPortNum,2,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningGet");

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.dfe,
                                  tuneValuesGet.dfe,
                                  "get another tuneValuesGet.dfe:%d than was set tuneValues.dfe: %d",
                                   tuneValuesGet.dfe,tuneValues.dfe);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeR,
                                  tuneValuesGet.ffeR,
                                  "get another tuneValuesGet.ffeR:%d than was set tuneValues.ffeR: %d",
                                   tuneValuesGet.ffeR,tuneValues.ffeR);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeC,
                                  tuneValuesGet.ffeC,
                                  "get another tuneValuesGet.ffeC:%d than was set tuneValues.ffeC: %d",
                                   tuneValuesGet.ffeC,tuneValues.ffeC);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sampler,
                                  tuneValuesGet.sampler,
                                  "get another tuneValuesGet.sampler:%d than was set tuneValues.sampler: %d",
                                   tuneValuesGet.sampler,tuneValues.sampler);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sqlch,
                                  tuneValuesGet.sqlch,
                                  "get another tuneValuesGet.sqlch:%d than was set tuneValues.sqlch: %d",
                                   tuneValuesGet.sqlch,tuneValues.sqlch);

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphAmp,
                                  tuneValuesGet.txEmphAmp,
                                  "get another tuneValuesGet.txEmphAmp:%d than was set tuneValues.txEmphAmp: %d",
                                   tuneValuesGet.txEmphAmp,tuneValues.txEmphAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmp,
                                  tuneValuesGet.txAmp,
                                  "get another tuneValuesGet.txAmp:%d than was set tuneValues.txAmp: %d",
                                   tuneValuesGet.txAmp,tuneValues.txAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpAdj,
                                  tuneValuesGet.txAmpAdj,
                                  "get another tuneValuesGet.txAmpAdj:%d than was set tuneValues.txAmpAdj: %d",
                                   tuneValuesGet.txAmpAdj,tuneValues.txAmpAdj);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeS,
                                  tuneValuesGet.ffeS,
                                  "get another tuneValuesGet.ffeS:%d than was set tuneValues.ffeS: %d",
                                   tuneValuesGet.ffeS,tuneValues.ffeS);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn,
                                  tuneValuesGet.txEmphEn,
                                  "get another tuneValuesGet.txEmphEn:%d than was set tuneValues.txEmphEn: %d",
                                   tuneValuesGet.txEmphEn,tuneValues.txEmphEn);


        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmph1,
                                  tuneValuesGet.txEmph1,
                                  "get another tuneValuesGet.txEmph1:%d than was set tuneValues.txEmph1: %d",
                                   tuneValuesGet.txEmph1,tuneValues.txEmph1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.align90,
                                  tuneValuesGet.align90,
                                  "get another tuneValuesGet.align90:%d than was set tuneValues.align90: %d",
                                   tuneValuesGet.align90,tuneValues.align90);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn1,
                                  tuneValuesGet.txEmphEn1,
                                  "get another tuneValuesGet.txEmphEn1:%d than was set tuneValues.txEmphEn1: %d",
                                   tuneValuesGet.txEmphEn1,tuneValues.txEmphEn1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpShft,
                                  tuneValuesGet.txAmpShft,
                                  "get another tuneValuesGet.txAmpShft:%d than was set tuneValues.txAmpShft: %d",
                                   tuneValuesGet.txAmpShft,tuneValues.txAmpShft);
        for(k=0;k<6;k++)
        {
         UTF_VERIFY_EQUAL4_STRING_MAC(tuneValues.dfeValsArray[k],
                                      tuneValuesGet.dfeValsArray[k],
                                      "get another tuneValuesGet.dfeValsArray[%d]:%d than was set tuneValues.dfeValsArray[%d]: %d",
                                      k,tuneValuesGet.dfeValsArray[k],k,tuneValues.dfeValsArray[k]);
        }

        /* restore default values */
        st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesDef);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

        /* try to remove again the packet - this time we will get GT_OK since
           the original packet was imported again to the DB */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /* remove packetId2 that was imported gain to DB */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

#endif
        /* IMPORT WITH MULTI ITERATIONS */

        for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
        {
            iter = 0;
            tempHsuDataSize = size;
            iterationSize = size;
            if (hsuDataSize < size)
            {
                iterationNumber = 1;
                remainedSize = 0;
                tempHsuDataSize = hsuDataSize;
            }
            else
            {
                iterationNumber = hsuDataSize/size;
                remainedSize = hsuDataSize%size;
            }
            tempHsuBlockMemPtr = hsuBlockMemPtr0;
            for( i = 0; i < iterationNumber; i++)
            {
                if (i == iterationNumber - 1)
                {
                    /* last iteration size */
                    tempHsuDataSize = tempHsuDataSize + remainedSize;
                    iterationSize = tempHsuDataSize;
                }
                st = cpssDxChHsuImport(dataType,
                                       &iter,
                                       &tempHsuDataSize,
                                       tempHsuBlockMemPtr,
                                       &importComplete);

                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
                if (st != GT_OK)
                {
                    goto restore;
                }

                if (i != iterationNumber -1)
                {
                    tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                    tempHsuDataSize = size + tempHsuDataSize;
                    iterationSize = tempHsuDataSize;
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
                }
            }
            /* iterator should be 0*/
             UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
             UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
             UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, importComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);

        }

        cpssOsFree(hsuBlockMemPtr0);

        st = cpssDxChPortSerdesTuningGet(devNum,phyPortNum,2,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningGet");

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.dfe,
                                  tuneValuesGet.dfe,
                                  "get another tuneValuesGet.dfe:%d than was set tuneValues.dfe: %d",
                                   tuneValuesGet.dfe,tuneValues.dfe);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeR,
                                  tuneValuesGet.ffeR,
                                  "get another tuneValuesGet.ffeR:%d than was set tuneValues.ffeR: %d",
                                   tuneValuesGet.ffeR,tuneValues.ffeR);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeC,
                                  tuneValuesGet.ffeC,
                                  "get another tuneValuesGet.ffeC:%d than was set tuneValues.ffeC: %d",
                                   tuneValuesGet.ffeC,tuneValues.ffeC);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sampler,
                                  tuneValuesGet.sampler,
                                  "get another tuneValuesGet.sampler:%d than was set tuneValues.sampler: %d",
                                   tuneValuesGet.sampler,tuneValues.sampler);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sqlch,
                                  tuneValuesGet.sqlch,
                                  "get another tuneValuesGet.sqlch:%d than was set tuneValues.sqlch: %d",
                                   tuneValuesGet.sqlch,tuneValues.sqlch);

        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphAmp,
                                  tuneValuesGet.txEmphAmp,
                                  "get another tuneValuesGet.txEmphAmp:%d than was set tuneValues.txEmphAmp: %d",
                                   tuneValuesGet.txEmphAmp,tuneValues.txEmphAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmp,
                                  tuneValuesGet.txAmp,
                                  "get another tuneValuesGet.txAmp:%d than was set tuneValues.txAmp: %d",
                                   tuneValuesGet.txAmp,tuneValues.txAmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpAdj,
                                  tuneValuesGet.txAmpAdj,
                                  "get another tuneValuesGet.txAmpAdj:%d than was set tuneValues.txAmpAdj: %d",
                                   tuneValuesGet.txAmpAdj,tuneValues.txAmpAdj);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeS,
                                  tuneValuesGet.ffeS,
                                  "get another tuneValuesGet.ffeS:%d than was set tuneValues.ffeS: %d",
                                   tuneValuesGet.ffeS,tuneValues.ffeS);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn,
                                  tuneValuesGet.txEmphEn,
                                  "get another tuneValuesGet.txEmphEn:%d than was set tuneValues.txEmphEn: %d",
                                   tuneValuesGet.txEmphEn,tuneValues.txEmphEn);


        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmph1,
                                  tuneValuesGet.txEmph1,
                                  "get another tuneValuesGet.txEmph1:%d than was set tuneValues.txEmph1: %d",
                                   tuneValuesGet.txEmph1,tuneValues.txEmph1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.align90,
                                  tuneValuesGet.align90,
                                  "get another tuneValuesGet.align90:%d than was set tuneValues.align90: %d",
                                   tuneValuesGet.align90,tuneValues.align90);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn1,
                                  tuneValuesGet.txEmphEn1,
                                  "get another tuneValuesGet.txEmphEn1:%d than was set tuneValues.txEmphEn1: %d",
                                   tuneValuesGet.txEmphEn1,tuneValues.txEmphEn1);
        UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpShft,
                                  tuneValuesGet.txAmpShft,
                                  "get another tuneValuesGet.txAmpShft:%d than was set tuneValues.txAmpShft: %d",
                                   tuneValuesGet.txAmpShft,tuneValues.txAmpShft);
        for(k=0;k<6;k++)
        {
         UTF_VERIFY_EQUAL4_STRING_MAC(tuneValues.dfeValsArray[k],
                                      tuneValuesGet.dfeValsArray[k],
                                      "get another tuneValuesGet.dfeValsArray[%d]:%d than was set tuneValues.dfeValsArray[%d]: %d",
                                      k,tuneValuesGet.dfeValsArray[k],k,tuneValues.dfeValsArray[k]);
        }

        /* restore default values */
        st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesDef);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

        /* try to remove again the packet - this time we will get GT_OK since
           the original packet was imported again to the DB */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* try to remove the packetId2 - get GT_OK since - packet was not removed so no reconstruct */
        st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    }

    systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_FALSE;
    systemRecoveryInfo.systemRecoveryMode.continuousTx = GT_FALSE;
    systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_FALSE;

    /* serdes configuratio */
    st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValues);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

    /* Tx Genarator configuration */

    st = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            txQueue,GT_FALSE,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChNetIfSdmaTxGeneratorEnable");

    st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupsBmp,
                                        &packetParams, packetData,
                                        packetDataLength,&packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0;
    packetParams.dsaParam.dsaInfo.fromCpu.tc = 4;

    st = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupsBmp,
                                        &packetParams, packetData,
                                        packetDataLength,&packetId2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 8;
    packetParams.dsaParam.dsaInfo.fromCpu.tc = 5;

    /* 5. Call to size get API (cpssDxChHsuBlockSizeGet)and saved returned size. Expected: GT_OK. */
    st = cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");
    tempHsuDataSize = hsuDataSize;

    hsuBlockMemPtr0 = cpssOsMalloc(hsuDataSize*sizeof(GT_U8));
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)hsuBlockMemPtr0, "cpssOsMalloc: Memory allocation error.");
    cpssOsMemSet(hsuBlockMemPtr0, 0, hsuDataSize*sizeof(GT_U8));

    /* 6. Perform  cpssDxChHsuExport with single iteration. The exported data is saved in  hsuBlockMemPtr0. Expected: GT_OK.*/
    st = cpssDxChHsuExport(dataType,
                         &iter,
                         &tempHsuDataSize,
                         hsuBlockMemPtr0,
                         &exportComplete);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, iter, "cpssDxChHsuExport");
    UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,hsuDataSize);
    if (st != GT_OK)
    {
        goto restore_full;
    }

    st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");


    st = cpssDxChPortSerdesTuningGet(devNum,phyPortNum,4,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.dfe,
                              tuneValuesGet.dfe,
                              "get another tuneValuesGet.dfe:%d than was set tuneValuesDef.dfe: %d",
                               tuneValuesGet.dfe,tuneValuesDef.dfe);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeR,
                              tuneValuesGet.ffeR,
                              "get another tuneValuesGet.ffeR:%d than was set tuneValuesDef.ffeR: %d",
                               tuneValuesGet.ffeR,tuneValuesDef.ffeR);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeC,
                              tuneValuesGet.ffeC,
                              "get another tuneValuesGet.ffeC:%d than was set tuneValuesDef.ffeC: %d",
                               tuneValuesGet.ffeC,tuneValuesDef.ffeC);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.sampler,
                              tuneValuesGet.sampler,
                              "get another tuneValuesGet.sampler:%d than was set tuneValuesDef.sampler: %d",
                               tuneValuesGet.sampler,tuneValuesDef.sampler);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.sqlch,
                              tuneValuesGet.sqlch,
                              "get another tuneValuesGet.sqlch:%d than was set tuneValuesDef.sqlch: %d",
                               tuneValuesGet.sqlch,tuneValuesDef.sqlch);

    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphAmp,
                              tuneValuesGet.txEmphAmp,
                              "get another tuneValuesGet.txEmphAmp:%d than was set tuneValuesDef.txEmphAmp: %d",
                               tuneValuesGet.txEmphAmp,tuneValuesDef.txEmphAmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmp,
                              tuneValuesGet.txAmp,
                              "get another tuneValuesGet.txAmp:%d than was set tuneValuesDef.txAmp: %d",
                               tuneValuesGet.txAmp,tuneValuesDef.txAmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmpAdj,
                              tuneValuesGet.txAmpAdj,
                              "get another tuneValuesGet.txAmpAdj:%d than was set tuneValuesDef.txAmpAdj: %d",
                               tuneValuesGet.txAmpAdj,tuneValuesDef.txAmpAdj);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.ffeS,
                              tuneValuesGet.ffeS,
                              "get another tuneValuesGet.ffeS:%d than was set tuneValuesDef.ffeS: %d",
                               tuneValuesGet.ffeS,tuneValuesDef.ffeS);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphEn,
                              tuneValuesGet.txEmphEn,
                              "get another tuneValuesGet.txEmphEn:%d than was set tuneValuesDef.txEmphEn: %d",
                               tuneValuesGet.txEmphEn,tuneValuesDef.txEmphEn);


    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmph1,
                              tuneValuesGet.txEmph1,
                              "get another tuneValuesGet.txEmph1:%d than was set tuneValuesDef.txEmph1: %d",
                               tuneValuesGet.txEmph1,tuneValuesDef.txEmph1);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.align90,
                              tuneValuesGet.align90,
                              "get another tuneValuesGet.align90:%d than was set tuneValuesDef.align90: %d",
                               tuneValuesGet.align90,tuneValuesDef.align90);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txEmphEn1,
                              tuneValuesGet.txEmphEn1,
                              "get another tuneValuesGet.txEmphEn1:%d than was set tuneValuesDef.txEmphEn1: %d",
                               tuneValuesGet.txEmphEn1,tuneValuesDef.txEmphEn1);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValuesDef.txAmpShft,
                              tuneValuesGet.txAmpShft,
                              "get another tuneValuesGet.txAmpShft:%d than was set tuneValuesDef.txAmpShft: %d",
                               tuneValuesGet.txAmpShft,tuneValuesDef.txAmpShft);
    for(k=0;k<6;k++)
    {
     UTF_VERIFY_EQUAL4_STRING_MAC(tuneValuesDef.dfeValsArray[k],
                                  tuneValuesGet.dfeValsArray[k],
                                  "get another tuneValuesGet.dfeValsArray[%d]:%d than was set tuneValuesDef.dfeValsArray[%d]: %d",
                                  k,tuneValuesGet.dfeValsArray[k],k,tuneValuesDef.dfeValsArray[k]);
    }

     /* remove exist packet */
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

     /* try to remove again the packet - get GT_BAD_PARAM */
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

    /* remove packetId2 that was imported gain to DB */
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                      txQueue, packetId2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssDxChNetIfSdmaTxGeneratorDisable(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,txQueue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChNetIfSdmaTxGeneratorDisable");

    /* 7. Perform cpssDxChHsuImport with different iteration size and iteration number. Expected: GT_OK.*/
    tempHsuDataSize = hsuDataSize;
    iter = 0;
    systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

    for (size = _2K ; size < hsuDataSize + 1119 ; size = size + 1119)
    {
        iter = 0;
        tempHsuDataSize = size;
        iterationSize = size;
        if (hsuDataSize < size)
        {
            iterationNumber = 1;
            remainedSize = 0;
            tempHsuDataSize = hsuDataSize;
        }
        else
        {
            iterationNumber = hsuDataSize/size;
            remainedSize = hsuDataSize%size;
        }
        tempHsuBlockMemPtr = hsuBlockMemPtr0;
        for( i = 0; i < iterationNumber; i++)
        {
            if (i == iterationNumber - 1)
            {
                /* last iteration size */
                tempHsuDataSize = tempHsuDataSize + remainedSize;
                iterationSize = tempHsuDataSize;
            }
            st = cpssDxChHsuImport(dataType,
                                   &iter,
                                   &tempHsuDataSize,
                                   tempHsuBlockMemPtr,
                                   &importComplete);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", i,iterationNumber,size);
            if (st != GT_OK)
            {
                goto restore_full;
            }

            if (i != iterationNumber -1)
            {
                tempHsuBlockMemPtr = (GT_U8*)(tempHsuBlockMemPtr + (iterationSize - tempHsuDataSize));
                tempHsuDataSize = size + tempHsuDataSize;
                iterationSize = tempHsuDataSize;
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,iterOrigSize = %d", iterationCounter,iterationNumber,size);
            }
        }
        /* iterator should be 0*/
         UTF_VERIFY_EQUAL3_STRING_MAC(0, iter, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
         UTF_VERIFY_EQUAL2_STRING_MAC(0, tempHsuDataSize, "remained size = %d,origIterSize =%d", tempHsuDataSize,size);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, importComplete, "iterationCounter = %d,iterationNumber =%d,origIterSize =%d", i,iterationNumber,size);
    }

    st = cpssDxChPortSerdesTuningGet(devNum,phyPortNum,4,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.dfe,
                              tuneValuesGet.dfe,
                              "get another tuneValuesGet.dfe:%d than was set tuneValues.dfe: %d",
                               tuneValuesGet.dfe,tuneValues.dfe);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeR,
                              tuneValuesGet.ffeR,
                              "get another tuneValuesGet.ffeR:%d than was set tuneValues.ffeR: %d",
                               tuneValuesGet.ffeR,tuneValues.ffeR);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeC,
                              tuneValuesGet.ffeC,
                              "get another tuneValuesGet.ffeC:%d than was set tuneValues.ffeC: %d",
                               tuneValuesGet.ffeC,tuneValues.ffeC);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sampler,
                              tuneValuesGet.sampler,
                              "get another tuneValuesGet.sampler:%d than was set tuneValues.sampler: %d",
                               tuneValuesGet.sampler,tuneValues.sampler);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.sqlch,
                              tuneValuesGet.sqlch,
                              "get another tuneValuesGet.sqlch:%d than was set tuneValues.sqlch: %d",
                               tuneValuesGet.sqlch,tuneValues.sqlch);

    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphAmp,
                              tuneValuesGet.txEmphAmp,
                              "get another tuneValuesGet.txEmphAmp:%d than was set tuneValues.txEmphAmp: %d",
                               tuneValuesGet.txEmphAmp,tuneValues.txEmphAmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmp,
                              tuneValuesGet.txAmp,
                              "get another tuneValuesGet.txAmp:%d than was set tuneValues.txAmp: %d",
                               tuneValuesGet.txAmp,tuneValues.txAmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpAdj,
                              tuneValuesGet.txAmpAdj,
                              "get another tuneValuesGet.txAmpAdj:%d than was set tuneValues.txAmpAdj: %d",
                               tuneValuesGet.txAmpAdj,tuneValues.txAmpAdj);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.ffeS,
                              tuneValuesGet.ffeS,
                              "get another tuneValuesGet.ffeS:%d than was set tuneValues.ffeS: %d",
                               tuneValuesGet.ffeS,tuneValues.ffeS);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn,
                              tuneValuesGet.txEmphEn,
                              "get another tuneValuesGet.txEmphEn:%d than was set tuneValues.txEmphEn: %d",
                               tuneValuesGet.txEmphEn,tuneValues.txEmphEn);


    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmph1,
                              tuneValuesGet.txEmph1,
                              "get another tuneValuesGet.txEmph1:%d than was set tuneValues.txEmph1: %d",
                               tuneValuesGet.txEmph1,tuneValues.txEmph1);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.align90,
                              tuneValuesGet.align90,
                              "get another tuneValuesGet.align90:%d than was set tuneValues.align90: %d",
                               tuneValuesGet.align90,tuneValues.align90);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txEmphEn1,
                              tuneValuesGet.txEmphEn1,
                              "get another tuneValuesGet.txEmphEn1:%d than was set tuneValues.txEmphEn1: %d",
                               tuneValuesGet.txEmphEn1,tuneValues.txEmphEn1);
    UTF_VERIFY_EQUAL2_STRING_MAC(tuneValues.txAmpShft,
                              tuneValuesGet.txAmpShft,
                              "get another tuneValuesGet.txAmpShft:%d than was set tuneValues.txAmpShft: %d",
                               tuneValuesGet.txAmpShft,tuneValues.txAmpShft);
    for(k=0;k<6;k++)
    {
     UTF_VERIFY_EQUAL4_STRING_MAC(tuneValues.dfeValsArray[k],
                                  tuneValuesGet.dfeValsArray[k],
                                  "get another tuneValuesGet.dfeValsArray[%d]:%d than was set tuneValues.dfeValsArray[%d]: %d",
                                  k,tuneValuesGet.dfeValsArray[k],k,tuneValues.dfeValsArray[k]);
    }

restore_full:

   /* try to remove again the packet - this time we will still get GT_BAD_PARAM
      since the original packet was not imported again to the DB due to
      systemRecoveryInfo.systemRecoveryMode.continuousTx = GT_FALSE;
      in this case the DB in re-initialized and the init values are not overwritten */
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

    /* try to remove a packet that was deleted before the import (packetId2) -
      this time we will get GT_BAD_PARAM
      since systemRecoveryInfo.systemRecoveryMode.continuousTx = GT_FALSE;
      and the init values are not overwritten */
    st = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

restore:

    /* restore default values */
    st = cpssDxChPortSerdesTuningSet(devNum,phyPortNum,0xFFFFFFFF,CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,&tuneValuesDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPortSerdesTuningSet");

    systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;

    cpssOsFree(hsuBlockMemPtr0);

    *testNeedRecovaryPtr = GT_FALSE;
    return;
}


UTF_TEST_CASE_MAC(cpssDxChHsuExportImportGlobalDataSerdesAndTxGeneratorOnly)
{
    GT_U8   devNum;
    GT_BOOL testNeedRecovary = GT_FALSE;
    /* txGenerator config */
    GT_PORT_GROUPS_BMP              portGroupsBmp       = 1;
    GT_U8                           txQueue             = 3;

    devNum = utfFirstDevNumGet();

    /* Skip this test for Cheetah and Cheetah2,3 */
    /* Skip this test non eArch devices */
    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    cpssDxChHsuExportImportGlobalDataSerdesAndTxGeneratorOnly(&testNeedRecovary);

    if(testNeedRecovary == GT_TRUE)
    {
        cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId);

        cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp,
                                                  txQueue, packetId2);

        cpssDxChNetIfSdmaTxGeneratorDisable(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,txQueue);
    }

    waitForPacketsToEnd();
}
#endif

/*
GT_STATUS cpssDxChHsuBlockSizeGet
(
    IN   CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    OUT  GT_U32                        *sizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHsuBlockSizeGet)
{
/*
    1.1. Call function with correct parameters
        dataType [CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E/
                  CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E/
                  CPSS_DXCH_HSU_DATA_TYPE_RX_TX_CPU_E/
                  CPSS_DXCH_HSU_DATA_TYPE_ALL_E],
        and not NULL sizePtr.
    Expect GT_OK.
    1.2. Call function with NULL sizePtr
    Expect GT_BAD_PTR.

*/
    GT_STATUS                     st = GT_OK;

    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType = CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E;
    GT_U32                        size;
    CPSS_DXCH_CFG_DEV_INFO_STC    devInfo;
    GT_U8       devNum = utfFirstDevNumGet();;

    cpssDxChCfgDevInfoGet(devNum, &devInfo);
    /* Skip this test for Cheetah and Cheetah2,3 */
    if (devInfo.genDevInfo.devFamily <= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        prvUtfSkipTestsSet();
        return;
    }

    /*
        1.1. Call function with correct parameters
        dataType [CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E/
                  CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E/
                  CPSS_DXCH_HSU_DATA_TYPE_RX_TX_CPU_E/
                  CPSS_DXCH_HSU_DATA_TYPE_ALL_E],
        and not NULL sizePtr.
    */
    dataType = CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E;

    st = cpssDxChHsuBlockSizeGet(dataType, &size);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");

    dataType = CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E;

    st = cpssDxChHsuBlockSizeGet(dataType, &size);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");

    dataType = CPSS_DXCH_HSU_DATA_TYPE_ALL_E;

    st = cpssDxChHsuBlockSizeGet(dataType, &size);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHsuBlockSizeGet");

    dataType = CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E;

    /*
        1.2. Call function with NULL sizePtr
        Expect GT_BAD_PTR.
    */
    st = cpssDxChHsuBlockSizeGet(dataType, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "cpssDxChHsuBlockSizeGet");

    /*
        1.3. Call function out of range dataType
        Expect: GT_BAD_PARAM.
    */
    UTF_ENUMS_CHECK_MAC(cpssDxChHsuBlockSizeGet(dataType, &size),
                        dataType);
}
#if 0
/**
* @internal deleteLpmDBsAndVRs function
* @endinternal
*
* @brief   The function deletes unneeded LpmDBs and Virtual Routers
*
* @note Needed in order to clear configuration that remained after running the
*       LPM tests.
*
*/
static void deleteLpmDBsAndVRs(GT_VOID)
{
    GT_U32      lpmDbId;
    GT_U32      vrId;

    for (lpmDbId = 0; lpmDbId < 100; lpmDbId++)
    {
        for (vrId = 0; vrId < 4096; vrId++)
        {
            if ((lpmDbId == 0) && (vrId == 0))
            {
                continue;
            }
            cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
        }
        if (lpmDbId > 0)
        {
            cpssDxChIpLpmDBDelete(lpmDbId);
        }
    }
}
#endif

/*
 * Configuration of cpssDxChHsu suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChHsu)
    /*UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsuExport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsuImport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsuExportImportGlobalDataSerdesAndTxGeneratorOnly)*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsuBlockSizeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChHsu)


