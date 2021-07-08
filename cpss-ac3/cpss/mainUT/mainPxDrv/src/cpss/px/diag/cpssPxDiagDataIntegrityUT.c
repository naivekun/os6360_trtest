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
* @file cpssPxDiagDataIntegrityUT.c
*
* @brief Unit tests for cpssPxDiagDataIntegrity
* CPSS DXCH Diagnostic API
*
* @version   5
********************************************************************************
*/
/* includes */

#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#define MAX_LOCATION_NUM_PER_MEMORY_CNS       512

#define PRV_PIPE_DFX_ECC_PROTECTED_MEMORY    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E

/* PIPE non DFX ECC:
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E
*/
/* PIPE non DFX parity:
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E
   CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E */


/* PIPE memories not supporting error injection ??*/




#ifndef ASIC_SIMULATION
typedef GT_STATUS PX_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

#endif

/*----------------------------------------------------------------------------*/



typedef enum
{
    PRV_UTF_PX_DATA_INTEGRITY_OP_ERROR_INJECTION_E,
    PRV_UTF_PX_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E,
    PRV_UTF_PX_DATA_INTEGRITY_OP_EVENT_MASK_E
}
PRV_UTF_PX_DATA_INTEGRITY_OP_ENT;
#if 0
/**
* @internal prvUtfCheckMemSkip function
* @endinternal
*
* @brief   Check if specified operation shouldn't be applied to
*          specified memory
* @param[in] dev                      - device number
* @param[in] memType                  - memory type
* @param[in] op                       - operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the mapping HW table->Logical Table is not found
* @retval GT_NOT_FOUND             - if memType is not found in DB
*/
static GT_BOOL prvUtfCheckMemSkip
(
    IN GT_U8 dev,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    IN PRV_UTF_PX_DATA_INTEGRITY_OP_ENT op
)
{
    GT_STATUS doSkip = 0;
    switch (op)
    {
        case PRV_UTF_PX_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E:
            switch (memType)
            {
            case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E:
                    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E:
                    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E:
                        /* skipped in bobcat3. See Jira CPSS-6627 */
                        doSkip = 1;
                        break;
                    default:
                        break;
                }
            }
        case PRV_UTF_PX_DATA_INTEGRITY_OP_EVENT_MASK_E:
            switch (memType)
            {
                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
                    /* false interrupt in lion2 > could not be enabled */
                    doSkip = PRV_CPSS_PX_LION2_FAMILY_CHECK_MAC(dev);
                    break;

                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
                    /* lion2, sip5, xcat3 */
                    /* false interrupt > could not be enabled. For sip5 see erratum
                       PRV_CPSS_PX_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E
                    */
                    doSkip = 1;
                    break;

                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
                    /* the BMA interrupt mask register is out of bobcat3
                       interrupts tree */
                    doSkip =  PRV_CPSS_PX_BOBCAT3_CHECK_MAC(dev);
                    break;


                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                    /* skipped in bobcat3. See Jira CPSS-6627 */
                    doSkip = PRV_CPSS_PX_BOBCAT3_CHECK_MAC(dev);
                    break;

                default:
                    break;
            }
            break;

        case PRV_UTF_PX_DATA_INTEGRITY_OP_ERROR_INJECTION_E:
            switch (memType)
            {
                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E:
                    /* lion2, sip5: because of HW behavior it is impossible to check it */
                    doSkip = 1;
                    break;

                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E:
                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E:
                    /* sip5. skip this memories. They are updated constantly by device.
                       So Error Injection bit can be reset before we read it */
                    doSkip = PRV_CPSS_SIP_5_CHECK_MAC(dev);
                    break;

                case PRV_CPSS_PX_BOBCAT2_CPU_MEMORIES_TO_SKIP_CASES_MAC:
                    /* should be skipped in bobcat2. In aldrin, ac3x, bobcat3
                       this memories are absent */
                    doSkip = PRV_CPSS_PX_BOBCAT2_CHECK_MAC(dev);
                    break;

                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_DESKEW_MEM_E:
                    /* TODO: in bobcat3 injection doesn't work somewhy.
                       in bobcat2, aldrin, ac3x this memory is absent */
                    doSkip = PRV_CPSS_PX_BOBCAT3_CHECK_MAC(dev);
                    break;

                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                    /* skipped in bobcat3. See Jira CPSS-6627 */
                    doSkip = PRV_CPSS_PX_BOBCAT3_CHECK_MAC(dev);
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }
    return  (doSkip? GT_TRUE: GT_FALSE);
}
#endif

/**
* @internal prvUtfInitLocationsFromMemType function
* @endinternal
*
* @brief   Fill RAM/HW Table/Logical Table locations according
*          to referenced memory type
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] memEntryPtr              - (pointer to) RAM-location.
* @param[out] hwEntryPtr               - (pointer to) HW table-location
* @param[out] ltEntryPtr               - (pointer to) Logical Table-location
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the mapping HW table->Logical Table is not found
* @retval GT_NOT_FOUND             - if memType is not found in DB
*/
static GT_STATUS prvUtfInitLocationsFromMemType
(
    IN GT_U8                                     devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    OUT CPSS_PX_LOCATION_SPECIFIC_INFO_STC       *memEntryPtr,
    OUT CPSS_PX_LOCATION_SPECIFIC_INFO_STC       *hwEntryPtr,
    OUT CPSS_PX_LOCATION_SPECIFIC_INFO_STC       *ltEntryPtr
)
{
    GT_STATUS rc;
    CPSS_PX_LOCATION_FULL_INFO_STC locationFull;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectType;
    GT_U32  arrSize = 1;

    cpssOsMemSet(&locationFull, 0, sizeof(locationFull));

    locationFull.ramEntryInfo.memType = memType;

    /* fill DFX pipe, client, memory according to memType. */

    rc = prvCpssPxDiagDataIntegrityMemoryIndexesGet(
        devNum,
        locationFull.ramEntryInfo.memType,
        &arrSize,
        &locationFull.ramEntryInfo.memLocation,
        &protectType);

    if (rc != GT_OK && rc != GT_BAD_SIZE)
    {
         /* GT_BAD_SIZE is not error. We intentionally specified arrSize==1
            to get only first RAM of specified type from DB */

        return rc; /* can be GT_NOT_FOUND */
    }

    rc = prvCpssPxDiagDataIntegrityFillHwLogicalFromRam(devNum, &locationFull);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsMemSet(memEntryPtr, 0, sizeof(*memEntryPtr));
    cpssOsMemSet(hwEntryPtr, 0, sizeof(*hwEntryPtr));
    cpssOsMemSet(ltEntryPtr, 0, sizeof(*ltEntryPtr));

    memEntryPtr->type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
    memEntryPtr->info.ramEntryInfo    = locationFull.ramEntryInfo;

    hwEntryPtr->type  = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
    hwEntryPtr->info.hwEntryInfo      = locationFull.hwEntryInfo;

    ltEntryPtr->type  = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    ltEntryPtr->info.logicalEntryInfo = locationFull.logicalEntryInfo;

    return GT_OK;
}


/**
* @internal prvUtfDiagDataIntegrityMemProtection function
* @endinternal
*
* @brief   Scan device Data Integrity DB for first item of specified memory type
*         and return protection types of the item.
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] dfxProtectionPtr         - DFX protection type
* @param[out] extProtectionPtr         - external (non-DFX) protection type
*
* @retval GT_OK                    - if memory is found in DB
* @retval GT_NOT_FOUND             - if memory is not found in DB
*/
static GT_STATUS  prvUtfDiagDataIntegrityMemProtection
(
    IN  GT_U8                                                  devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT               memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT *dfxProtectionPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT *extProtectionPtr
)
{
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   *memDbArr;
    GT_U32                                        memDbSize;
    GT_U32                                        i;


    prvCpssPxDiagDataIntegrityDbPointerSet(&memDbArr, &memDbSize);

    CPSS_NULL_PTR_CHECK_MAC(memDbArr);

    for (i=0; i<memDbSize; i++)
    {
        if (memDbArr[i].memType == memType)
        {
            if (dfxProtectionPtr)
                *dfxProtectionPtr = memDbArr[i].protectionType;
            if (extProtectionPtr)
                *extProtectionPtr = memDbArr[i].externalProtectionType;
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}


/**
* @internal prvUtfDiagDataIntegrityHwTableIsDfxProtected
*           function
* @endinternal
*
* @brief   Check if the memory type is DFX protected
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - HW table type
*
* @retval GT_TRUE                  - memory is DFX protected
* @retval GT_FALSE                 - memory either not protected by DFX
*/


static GT_BOOL  prvUtfDiagDataIntegrityHwTableIsDfxProtected
(
    IN  GT_U8                                                  devNum,
    IN  GT_U32                                                 hwTable
)
{
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *memDbArr;
    GT_U32                                        memDbSize;
    GT_U32                                        i;


    prvCpssPxDiagDataIntegrityDbPointerSet(&memDbArr, &memDbSize);

    CPSS_NULL_PTR_CHECK_MAC(memDbArr);

    for (i=0; i<memDbSize; i++)
    {
        if (memDbArr[i].hwTableName == hwTable)
        {
            break;
        }
    }
    if (memDbArr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
        return GT_FALSE;
    else
        return GT_TRUE;

}

/**
* @internal prvUtfDiagDataIntegrityMemIsDfxProtected function
* @endinternal
*
* @brief   Check if the memory type is DFX protected
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @retval GT_TRUE                  - memory is DFX protected
* @retval GT_FALSE                 - memory either not protected by DFX or is not relevant to device
*/
static GT_BOOL prvUtfDiagDataIntegrityMemIsDfxProtected
(
    IN  GT_U8                                      devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType
)
{
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT dfxProtection;
    GT_STATUS rc;
    rc = prvUtfDiagDataIntegrityMemProtection(devNum, memType, &dfxProtection, NULL);

    if (rc == GT_OK && dfxProtection != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }

}


/**
* @internal prvUtfDiagDataIntegrityIsEventMaskedAsExpected function
* @endinternal
*
* @brief   Check if cpssPxDiagDataIntegrityEventMaskGet returned
*         values are appropriate to the previously called
*         cpssPxDiagDataIntegrityEventMaskSet
*
* @retval GT_TRUE                  - if read values corresponds to written values
* @retval GT_FALSE                 - otherwise
*/
static GT_BOOL prvUtfDiagDataIntegrityIsEventMaskedAsExpected
(
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT         typeSet,
    IN CPSS_EVENT_MASK_SET_ENT                               opSet,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT         typeGet,
    IN CPSS_EVENT_MASK_SET_ENT                               opGet
)
{

    GT_U32 isOk;

    if (typeGet == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E ||
        typeGet == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
    {
        isOk = (opSet == opGet);
    }
    else
    {
        /* ECC protected, not SINGLE_AND_MULTIPLE */

        /* a result "op:unmasked: type:<ECC multiple>" is satisfactory
           for a Set-request "op:mask, type:<ECC single>" */
        isOk =
            ((opSet == CPSS_EVENT_MASK_E) && (opSet != opGet) && (typeSet != typeGet))
            ||
            ((opSet == CPSS_EVENT_UNMASK_E) && (opSet == opGet) && (typeSet == typeGet));
    }
    return BIT2BOOL_MAC(isOk);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityEventMaskSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityEventMaskSet)
{
/*
    ITERATE_DEVICES PIPE
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E - parameter
                     relevant only for ECC protected memories.
        operation = CPSS_EVENT_UNMASK_E
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorTypeGet;
    CPSS_EVENT_MASK_SET_ENT                             operation;
    CPSS_EVENT_MASK_SET_ENT                             operationGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  dfxProtection;
     CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT         memType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                            locationIter;
    GT_U32                                            opIter;
    GT_BOOL                                           isOkExpected;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
           /* if (GT_TRUE == prvUtfCheckMemSkip(
                    dev, memType, PRV_UTF_PX_DATA_INTEGRITY_OP_EVENT_MASK_E))
            {
                continue;
            }*/
            st = prvUtfDiagDataIntegrityMemProtection (dev, memType,  &dfxProtection, NULL);
            if (st == GT_OK && (dfxProtection == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E))
            {/* no masking in DFX parity protection*/
                continue;
            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* shouldn't happen  - RAM is not found in DataIntegrity DB
                cpssOsPritnf ("EventMaskSetTest :RAM %d is not found in DataIntegrity DB\n", memType);*/
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND)
                {
                    /* shouldn't happen  -  memType is not found in DataIntegrity DB
                    cpssOsPritnf ("EventMaskSetTest :RAM %d is not found in DataIntegrity DB\n", memType);*/
                    continue;
                }
                for (errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                     errorType <= CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                     errorType++)
                {

                    if (((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E) &&
                         (errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E))
                        ||
                        ((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E) &&
                         (errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)))
                    {
                        isOkExpected = GT_FALSE;
                    }
                    else
                    {
                        isOkExpected = GT_TRUE;
                    }

                    for (opIter = 0; opIter < 3; opIter++)
                    {
                        /* 1) To ensure mask/unmask really works previous event
                               state should be unmasked/masked.
                           2) Last operation should be "mask" to avoid  unmasking
                              of events (masked by default) after UT finished
                           So operations will be {mask, unmask, mask} */
                        operation = (opIter % 2) ?  CPSS_EVENT_UNMASK_E : CPSS_EVENT_MASK_E;
                        st = cpssPxDiagDataIntegrityEventMaskSet(dev, locationPtr,
                                                                 errorType, operation);
                        if (GT_FALSE == isOkExpected)
                        {
                            UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                            errorType, operation);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        errorType, operation);
                        }
                        if(st == GT_OK)
                        {
                            st = cpssPxDiagDataIntegrityEventMaskGet(dev, locationPtr,
                                                                     &errorTypeGet, &operationGet);
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        errorTypeGet, operationGet);
                            if(st == GT_OK)
                            {
                                UTF_VERIFY_EQUAL7_STRING_MAC(GT_TRUE,
                                                             prvUtfDiagDataIntegrityIsEventMaskedAsExpected(errorType, operation,
                                                                                                            errorTypeGet, operationGet),
                                                             "bad masking: dev %d, mem %d, iter %d,"" errorType %d(%d), operation %d(%d)",
                                                             dev, memType, locationIter,
                                                             errorType, errorTypeGet, operation, operationGet);
                            }
                        }
                    }
                }
            }
        }

        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        operation = CPSS_EVENT_MASK_E;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityEventMaskSet
                                          (dev, &memEntry, errorType, operation),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /* set valid ECC protected memType. */
        memType = PRV_PIPE_DFX_ECC_PROTECTED_MEMORY;
        memEntry.info.ramEntryInfo.memType = memType;

        /*
              1.2. Call with wrong errorType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityEventMaskSet
                            (dev, &memEntry, errorType, operation),
                            errorType);

        /*
              1.2. Call with wrong operation (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityEventMaskSet
                            (dev, &memEntry, errorType, operation),
                            operation);
    }

    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E;
    memEntry.info.ramEntryInfo.memType = memType;
    errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    operation = CPSS_EVENT_UNMASK_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityEventMaskSet(dev, &memEntry, errorType, operation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityEventMaskSet(dev, &memEntry, errorType, operation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPx_DiagDataIntegrityEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityEventMaskGet)
{
/*
    ITERATE_DEVICES PIPE
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
        errorType = CPSS_PX_DIAG_DATA_INTEGRITY_SINGLE_AND_MULTIPLE_ECC_ERROR_TYPE_E - parameter
                     relevant only for ECC protected memories.
        operation = CPSS_EVENT_UNMASK_E
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.
    1.3 Call with NULL ptr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType;
    CPSS_EVENT_MASK_SET_ENT                             operation;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                            locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    GM_NOT_SUPPORT_THIS_TEST_MAC
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];


                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter, errorType, operation);

                st = cpssPxDiagDataIntegrityEventMaskGet(dev, locationPtr, &errorType, &operation);
                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, operation);
                }
                else
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, operation);
                }
            }
        }

        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityEventMaskGet
                                          (dev, &memEntry, &errorType, &operation),
                                          memEntry.info.ramEntryInfo.memType,
                                          GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = PRV_PIPE_DFX_ECC_PROTECTED_MEMORY;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssPxDiagDataIntegrityEventMaskGet(dev, &memEntry, NULL, &operation);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    /* set valid memType */
    memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E;
    memEntry.info.ramEntryInfo.memType = memType;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, &operation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, &operation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigSet)
{
/*
    ITERATE_DEVICES Pipe
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectModeGet;
    GT_BOOL                                         injectEnable = GT_TRUE;
    GT_BOOL                                         injectEnableGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_U32                                          i;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            memEntry; /* RAM location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            if ((memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E) ||
                (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E) ||
                (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E))
            {
                continue;
            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


                /* 1.1 a) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E
                          injectEnable = GT_TRUE
                       b) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E
                          injectEnable = GT_TRUE
                       c) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E
                          injectEnable = GT_FALSE

                        Expect (for all cases): GT_OK for all memTypes that support injection,
                                                otherwise GT_NOT_SUPPORTED */
                for (i=0; i<3; i++)
                {
                    switch (i)
                    {
                        case 0: /* 1.1.a */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
                            injectEnable = GT_TRUE;
                            break;
                        case 1: /* 1.1.b */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
                            injectEnable = GT_TRUE;
                            break;
                        default: /* 1.1.c */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
                            injectEnable = GT_FALSE;
                            break;
                    }

                    st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(
                        dev, locationPtr, injectMode, injectEnable);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectMode, injectEnable);

                    if (protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectMode, injectEnable);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        injectMode, injectEnable);
                    }

                    if(st == GT_OK)
                    {
                        st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(
                            dev, locationPtr, &injectModeGet, &injectEnableGet);
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectModeGet, injectEnableGet);
                        if(st == GT_OK)
                        {
                            if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                            {
                                UTF_VERIFY_EQUAL5_STRING_MAC(injectMode, injectModeGet,
                                                             "get another injectModeGet than was set: %d %d %d %d %d",
                                                             dev, memType, locationIter, injectModeGet, injectEnableGet);
                            }
                            UTF_VERIFY_EQUAL5_STRING_MAC(injectEnable, injectEnableGet,
                                                         "get another injectEnableGet than was set: %d %d %d %d %d",
                                                         dev, memType, locationIter, injectModeGet, injectEnableGet);
                        }
                    }
                }
            }
        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigSet
                                          (dev, &memEntry, injectMode, injectEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);
        }
        /*
              1.2. Call with wrong injectMode (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigSet
                            (dev, &memEntry, injectMode, injectEnable),
                            injectMode);
    }

    /* set valid values */
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E;
    memEntry.info.ramEntryInfo.memType = memType;
    injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    injectEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &memEntry, injectMode, injectEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &memEntry, injectMode, injectEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigGet)
{
/*
    ITERATE_DEVICES(PIPE)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode;
    GT_BOOL                                         injectEnable;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            memEntry; /* RAM location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                        locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);


                injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
                injectEnable = GT_TRUE;

                st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(
                    dev, locationPtr, &injectMode, &injectEnable);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev,
                                            memType, locationIter,
                                            injectMode, injectEnable);

            }
        }

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigGet
                                          (dev, &memEntry, &injectMode, &injectEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, NULL, &injectEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, &injectEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, &injectEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityErrorCountEnableSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorTypeGet;
    GT_BOOL                                         countEnable = GT_TRUE;
    GT_BOOL                                         countEnableGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                         memFound;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType, memTypeLast;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        memFound = GT_FALSE;
        memTypeLast = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E;
        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            /* skip non-dfx protected memories */
            if (GT_FALSE == prvUtfDiagDataIntegrityMemIsDfxProtected(dev, memType))
            {
                continue;
            }
            if ((memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E) ||
                (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E) ||
                (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E) ||
                (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E))
            {
                continue;
            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
            /*if (GT_TRUE == prvUtfCheckMemSkip(
                    dev, memType, PRV_UTF_PX_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E))
            {
                continue;
            }*/

            memFound = GT_TRUE;
            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);
                /*if HW or logical internal table */
                if (((locationIter == 1)  &&
                     (locationPtr->info.hwEntryInfo.hwTableType > CPSS_PX_TABLE_LAST_E)) ||
                    ((locationIter == 2)  &&
                     (locationPtr->info.logicalEntryInfo.logicalTableType >= CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E)))
                {
                    /*check again if that table is DFX protected*/
                    if (!prvUtfDiagDataIntegrityHwTableIsDfxProtected(dev, hwEntry.info.hwEntryInfo.hwTableType))
                        continue;
                }
                memTypeLast = memType;
                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E
                        countEnable = GT_TRUE

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */

                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                countEnable = GT_TRUE;

                st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);
                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK)
                    {
                        UTF_VERIFY_EQUAL5_STRING_MAC(errorType, errorTypeGet,
                                                     "get another errorType than was set: %d %d %d %d %d",
                                                     dev, memType, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL5_STRING_MAC(countEnable, countEnableGet,
                                                     "get another countEnable than was set: %d %d %d %d %d",
                                                     dev, memType, locationIter, errorTypeGet, countEnableGet);
                    }
                }

                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;

                st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);

                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK)
                    {
                        UTF_VERIFY_EQUAL5_STRING_MAC(
                            errorType, errorTypeGet,
                            "get another errorType than was set: %d %d %d %d %d",
                            dev, memType, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL5_STRING_MAC(
                            countEnable, countEnableGet,
                            "get another countEnable than was set: %d %d %d %d %d",
                            dev, memType, locationIter, errorTypeGet, countEnableGet);
                    }
                }


                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;

                st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);

                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK)
                    {
                         UTF_VERIFY_EQUAL5_STRING_MAC(
                            errorType, errorTypeGet,
                            "get another errorType than was set: %d %d %d %d %d",
                            dev, memType, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL5_STRING_MAC(
                            countEnable, countEnableGet,
                            "get another countEnable than was set: %d %d %d %d",
                            dev, memType, locationIter, errorTypeGet, countEnableGet);
                    }
                }
            }
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, memFound,
                                     "no one valid memory is found in device %d ",
                                     dev);

        /* use last valid */
        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        memEntry.info.ramEntryInfo.memType = memTypeLast;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityErrorCountEnableSet
                                          (dev, &memEntry, errorType, countEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.2. Call with wrong errorType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityErrorCountEnableSet
                            (dev, &memEntry, errorType, countEnable),
                            errorType);
    }
    /* set valid values */
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_HA_TABLE_E;
    memEntry.info.ramEntryInfo.memType = memType;
    errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    countEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, &memEntry, errorType, countEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, &memEntry, errorType, countEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityErrorCountEnableGet)
{
/*
    ITERATE_DEVICES PIPE
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    GT_BOOL                                         countEnable;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationPtr;
    GT_U32                                        locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];
                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);

                /*
                    1.1 Call with

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */

                st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorType, &countEnable);
                if(protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }
            }
        }

        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */

        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityErrorCountEnableGet
                                          (dev, &memEntry, &errorType, &countEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);


        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */

        memType = PRV_PIPE_DFX_ECC_PROTECTED_MEMORY;
        memEntry.info.ramEntryInfo.memType = memType;
        st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, NULL, &countEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    /* set valid values */
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_HA_TABLE_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, &countEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, &countEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityProtectionTypeGet
(
    IN  GT_U8                                                    devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                      *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityProtectionTypeGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                            locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];
                st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, protectionType);
            }
        }

        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssPxDiagDataIntegrityProtectionTypeGet
                                          (dev, &memEntry, &protectionType),
                                          memEntry.info.ramEntryInfo.memType,
                                          GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, &memEntry, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);


    }

    /* set valid values */
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, &memEntry, &protectionType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityProtectionTypeGet(dev, &memEntry, &protectionType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityErrorInfoGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityErrorInfoGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;

    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmLocation = {1, 1, 0};
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmLocationPtr = NULL;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC*           locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_PX_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        for (memType = 0; memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {

                locationPtr = locationsArr[locationIter];

                mppmLocationPtr = NULL;
                if(memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E)
                {
                    mppmLocationPtr = &mppmLocation;
                }
                st = cpssPxDiagDataIntegrityErrorInfoGet(dev, locationPtr, mppmLocationPtr,
                                                           &errorCounter, &failedRow,
                                                           &failedSegment, &failedSyndrome);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
            }
        }

        memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityErrorInfoGet
                            (dev, &memEntry, NULL,  &errorCounter, &failedRow,
                             &failedSegment, &failedSyndrome),
                            memEntry.info.ramEntryInfo.memType);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, NULL,
                                                   &failedRow, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   NULL, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, NULL, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, &failedSegment, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* restore valid values */
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                               &failedRow, &failedSegment, &failedSyndrome);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxDiagDataIntegrity suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxDiagDataIntegrity)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityEventMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityEventMaskGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityErrorInjectionConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityErrorCountEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityErrorCountEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityProtectionTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityErrorInfoGet)

UTF_SUIT_END_TESTS_MAC(cpssPxDiagDataIntegrity)
