/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChTcamManager.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChTcamManager, that provides
*       TCAM manager module.
*
* FILE REVISION NUMBER:
*       $Revision: 11 $
*******************************************************************************/

/* includes */
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManager.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* defines */

/*this used for points to client registration functions*/
static GT_STATUS prvClientMoveToLocation
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *fromPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *toPtr,
    IN  GT_VOID                                     *clientCookiePtr
);

/*this used for points to client registration functions*/
static GT_STATUS prvClientMoveToAnywhere(IN GT_VOID *clientCookiePtr);

/*this used for points to client registration functions*/
static GT_STATUS prvClientCheckIfDefaultLocation
(
    IN GT_VOID *clientCookiePtr,
    OUT GT_BOOL *isDefaultEntryLocationPtr
);

/* create default tcam manager handler */
static GT_STATUS prvCreateDefaultTcamManager(OUT GT_VOID **tcamManagerPtrPtr);

/* create default tcam manager handler with clients */
static GT_STATUS prvCreateDefaultTcamManagerWithClients
(
    OUT GT_U32       *clientId,
    OUT GT_VOID     **tcamManagerPtrPtr
);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerCreate
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC        *rangePtr,
    OUT GT_VOID                                 **tcamManagerHandlerPtrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerCreate)
{
/*
    1. Call with range.firstLine[0], range.lastLine[10],
    tcamType[CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E /
    CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E].
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerConfigGet with the same tcamManagerHandlerPtr.
    Expected: GT_OK and the same params.
    3. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
    to restore configuration.
    Expected: GT_OK.
    4. Call with wrong enum values tcamType.
    Expected: GT_BAD_PARAM.
    5. Call with wrong value range.firstLine [0x3FFFF + 1] (out of range).
    Expected: NOT GT_OK.
    6. Call with wrong value range.lastLine [0x3FFFF + 1] (out of range).
    Expected: NOT GT_OK.
    7. Call with wrong values range (First line > Last line).
    Expected: NOT GT_OK.
    8. Call with wrong rangePtr[NULL].
    Expected: GT_BAD_PTR.
    9. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType =
                            CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamTypeGet;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        range;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        rangeGet;
    GT_VOID                                 *tcamManagerHandlerPtr;

    GT_U8      dev       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1. Call with range.firstLine[0], range.lastLine[10],
               tcamType[CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E /
                        CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E].
            Expected: GT_OK.
        */
        range.firstLine = 0;
        range.lastLine  = 10;

        /* call with tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E */
        tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E;

        st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        if(GT_OK == st)
        {
            /*
                2. Call cpssDxChTcamManagerConfigGet with the same tcamManagerHandlerPtr.
                Expected: GT_OK and the same params.
            */
            st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, &tcamTypeGet, &rangeGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerConfigGet failed");

            if(GT_OK == st)
            {
                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tcamType, tcamTypeGet,
                          "got another tcamType as was written: %d, %d", tcamType, tcamTypeGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(range.firstLine, rangeGet.firstLine,
                          "got another range.firstLine as was written: %d, %d",
                                             range.firstLine, rangeGet.firstLine);
                UTF_VERIFY_EQUAL2_STRING_MAC(range.lastLine, rangeGet.lastLine,
                          "got another range.lastLine as was written: %d, %d",
                                             range.lastLine, rangeGet.lastLine);
            }
            /*
                3. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
                to restore configuration.
                Expected: GT_OK.
            */
            st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
        }


        /* call with tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E */
        tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;

        st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        if(GT_OK == st)
        {
            /*
                1. Call cpssDxChTcamManagerConfigGet with the same tcamManagerHandlerPtr.
                Expected: GT_OK and the same params.
            */
            st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, &tcamTypeGet, &rangeGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerConfigGet failed");

            if(GT_OK == st)
            {
                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tcamType, tcamTypeGet,
                          "got another tcamType as was written: %d, %d", tcamType, tcamTypeGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(range.firstLine, rangeGet.firstLine,
                          "got another range.firstLine as was written: %d, %d",
                                             range.firstLine, rangeGet.firstLine);
                UTF_VERIFY_EQUAL2_STRING_MAC(range.lastLine, rangeGet.lastLine,
                          "got another range.lastLine as was written: %d, %d",
                                             range.lastLine, rangeGet.lastLine);
            }
            /*
                1. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
                to restore configuration.
                Expected: GT_OK.
            */
            st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
        }

        /*
            4. Call with wrong enum values tcamType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTcamManagerCreate
                            (tcamType, &range, &tcamManagerHandlerPtr),
                            tcamType);

        /*
            5. Call with wrong value range.firstLine [0x3FFFF + 1] (out of range).
            Expected: NOT GT_OK.
        */
        range.firstLine  = 0x3FFFF + 1;

        st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        range.firstLine  = 10;

        /*
            6. Call with wrong value range.lastLine [0x3FFFF + 1] (out of range).
            Expected: NOT GT_OK.
        */
        range.lastLine  = 0x3FFFF + 1;

        st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        range.lastLine  = 10;

        /*
            7. Call with wrong values range (First line > Last line).
            Expected: NOT GT_OK.
        */
        range.firstLine  = 11;
        range.lastLine  = 10;

        st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        range.firstLine  = 0;
        range.lastLine  = 10;

        /*
            8. Call with wrong rangePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerCreate(tcamType, NULL, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rangePtr = NULL", tcamType);

        /*
            9. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerCreate(tcamType, &range, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "%d, tcamManagerHandlerPtr = NULL", tcamType);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerConfigGet
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    OUT CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    *tcamTypePtr,
    OUT CPSS_DXCH_TCAM_MANAGER_RANGE_STC        *rangePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerConfigGet)
{
/*
    1. Call cpssDxChTcamManagerConfigGet with not null pointers.
    Expected: GT_OK.
    2. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    3. Call with wrong tcamTypeGetPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong rangePtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
    to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamTypeGet;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        rangeGet;

    GT_U8      dev       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerConfigGet with not null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, &tcamTypeGet, &rangeGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerConfigGet(NULL, &tcamTypeGet, &rangeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            3. Call with wrong tcamTypeGetPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, NULL, &rangeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamTypePtr = NULL");

        /*
            4. Call with wrong rangePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, &tcamTypeGet, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, rangePtr = NULL");

        /*
            5. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
            to restore configuration.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerDelete
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerDelete)
{
/*
    1. Call cpssDxChTcamManagerDelete with correct tcamManagerHandlerPtr.
    Expected: GT_OK.
    2. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    GT_VOID                                 *tcamManagerHandlerPtr;

    GT_U8      dev       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerDelete with correct tcamManagerHandlerPtr.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerDelete(NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerClientRegister
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_8                                    clientName[CPSS_DXCH_TCAM_MANAGER_CLIENT_NAME_NAX_LENGTH_CNS],
    IN  CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  *clientFuncPtr,
    OUT GT_U32                                  *clientIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerClientRegister)
{
/*
    1. Call with correct params and client name ["Client1"].
    Expected: GT_OK.
    2. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    3. Call with wrong clientFunc.moveToLocationFuncPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong clientFunc.moveToAnywhereFuncPtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call with wrong clientNamePtr[NULL].
    Expected: GT_BAD_PTR.
    6. Call with wrong clientFuncPtr[NULL].
    Expected: GT_BAD_PTR.
    7. Call with wrong clientIdPtr[NULL].
    Expected: GT_BAD_PTR.
    8. Call CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS times to check client num counter.
    Expected: GT_OK CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS times, and not GT_OK after it.
    9. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;
    GT_U32                                  clientNum = 0;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call with correct params and client name ["Client1"].
            Expected: GT_OK.
        */
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore configuration */
        st = cpssDxChTcamManagerClientUnregister(tcamManagerHandlerPtr, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(NULL, "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            3. Call with wrong clientFunc.moveToLocationFuncPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        clientFunc.moveToLocationFuncPtr = NULL;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st,
                "%d, clientFunc.moveToLocationFuncPtr = NULL");

        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;

        /*
            4. Call with wrong clientFunc.moveToAnywhereFuncPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        clientFunc.moveToAnywhereFuncPtr = NULL;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st,
                "%d, clientFunc.moveToAnywhereFuncPtr = NULL");

        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;

        /*
            Call with wrong clientFunc.checkIfDefaultLocationFuncPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        clientFunc.checkIfDefaultLocationFuncPtr = NULL;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st,
                "%d, clientFunc.checkIfDefaultLocationFuncPtr = NULL");


        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        /*
            5. Call with wrong clientNamePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                NULL, &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, clientName = NULL");

        /*
            6. Call with wrong clientFuncPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", NULL, &clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, clientFunc = NULL");

        /*
            7. Call with wrong clientIdPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, clientId = NULL");

        /*
            8. Call CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS times to check client num counter.
            Expected: GT_OK CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS times,
            and not GT_OK after it.
        */
        for(clientNum = 0; clientNum < CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS; clientNum++)
        {
            st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                    "Client1", &clientFunc, &clientId);
            if (clientNum == CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS)
            {
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, clientNum);
            }
        }

        /*
            9. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerClientUnregister
(
    IN  GT_VOID     *tcamManagerHandlerPtr,
    IN  GT_U32      clientId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerClientUnregister)
{
/*
    1. Call cpssDxChTcamManagerClientUnregister for registered client
    with the same clientId.
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerClientUnregister for not registered client
    with the same clientId.
    Expected: NOT GT_OK (client already unregistered).
    3. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    5. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerClientUnregister for registered client
            with the same clientId.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerClientUnregister(tcamManagerHandlerPtr, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerClientUnregister for not registered client
            with the same clientId.
            Expected: NOT GT_OK (client already unregistered).
        */
        st = cpssDxChTcamManagerClientUnregister(tcamManagerHandlerPtr, clientId);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientUnregister(NULL, clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            4. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerClientUnregister(tcamManagerHandlerPtr, clientId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        /*
            5. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerAvailableEntriesCheck
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArray[],
    IN  GT_U32                                  sizeOfArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerAvailableEntriesCheck)
{
/*
    1. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E,
       requestedEntriesArray[0].amount = 10 and sizeOfArray [1].
    Expected: GT_OK.
    2. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E,
       requestedEntriesArray[0].amount = 20 and sizeOfArray [1].
    Expected: GT_OK.
    3. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E,
       requestedEntriesArray[0].amount = 5 and sizeOfArray [1].
    Expected: GT_OK.
    4. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E,
       requestedEntriesArray[0].amount = 1 and sizeOfArray [1].
    Expected: GT_OK.
    5. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
            requestedEntriesArray[2].amount    = 3;
       and sizeOfArray [3].
    Expected: GT_OK.
    6. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
            requestedEntriesArray[2].amount    = 3;
            requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[3].amount    = 1;
       and sizeOfArray [4].
    Expected: GT_OK.
    7. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[2].amount    = 1;
       and sizeOfArray [3].
    Expected: GT_OK.
    8. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[0].amount    = 3;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[2].amount    = 1;
       and sizeOfArray [3].
    Expected: NOT GT_OK (the same entryType twice).
    9. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    10. Call with non-existent clientId.
    Expected: NOT GT_OK.
    11. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    12. Call with wrong requestedEntriesArray[NULL].
    Expected: GT_BAD_PTR.
    13. Call with wrong enum values requestedEntriesArray.entryType.
    Expected: GT_BAD_PARAM.
    14. Call with wrong value requestedEntriesArray.amount[0].
    Expected: NOT GT_OK (out of range).
    15. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId = 0;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArray[4];
    GT_U32                                  sizeOfArray = 4;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E,
               requestedEntriesArray[0].amount = 10 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 10;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            2. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E,
               requestedEntriesArray[0].amount = 20 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[0].amount    = 20;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            3. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E,
               requestedEntriesArray[0].amount = 5 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 5;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            4. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E,
               requestedEntriesArray[0].amount = 1 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[0].amount    = 1;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            5. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
                    requestedEntriesArray[2].amount    = 3;
               and sizeOfArray [3].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[2].amount    = 3;

        sizeOfArray = 3;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            6. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
                    requestedEntriesArray[2].amount    = 3;
                    requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[3].amount    = 1;
               and sizeOfArray [4].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[2].amount    = 3;
        requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[3].amount    = 1;

        sizeOfArray = 4;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            7. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[2].amount    = 1;
               and sizeOfArray [3].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[2].amount    = 1;

        sizeOfArray = 3;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            8. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[0].amount    = 3;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[2].amount    = 1;
               and sizeOfArray [3].
            Expected: NOT GT_OK (the same entryType twice).
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[0].amount    = 1;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[2].amount    = 1;
        sizeOfArray = 3;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            9. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerAvailableEntriesCheck(NULL,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            10. Call with non-existent clientId.
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                ((clientId != 0) ? 0 : 1), requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            11. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);
        /* restore clientId */
        clientId = 0;

        /*
            12. Call with wrong requestedEntriesArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, NULL, sizeOfArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st,
                                "%d, requestedEntriesArray = NULL");

        /*
            13. Call with wrong enum values requestedEntriesArray.entryType.
            Expected: GT_BAD_PARAM.
        */
        requestedEntriesArray[0].amount = 10;
        sizeOfArray = 1;

        UTF_ENUMS_CHECK_MAC(cpssDxChTcamManagerAvailableEntriesCheck
                            (tcamManagerHandlerPtr, clientId,
                             requestedEntriesArray, sizeOfArray),
                            requestedEntriesArray[0].entryType);

        /*
            14. Call with wrong value requestedEntriesArray.amount[0].
            Expected: NOT GT_OK (out of range).
        */
        requestedEntriesArray[0].amount = 0;

        st = cpssDxChTcamManagerAvailableEntriesCheck(tcamManagerHandlerPtr,
                                clientId, requestedEntriesArray, sizeOfArray);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        requestedEntriesArray[0].amount = 10;

        /*
            15. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerDump
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT    dumpType,
    IN  GT_U32                                  clientId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerDump)
{
/*
    1. Call with dumpType
           [CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E                            /
            CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_ALL_CLIENTS_E            /
            CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_AND_ENTRIES_FOR_ALL_CLIENTS_E]
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerClientRegister with
            clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
            clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
    Expected: GT_OK.
    3. Call with dumpType [CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E]
    and correct clientId (registered).
    Expected: GT_OK.
    4. Call with dumpType [CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E]
    and wrong clientId (unregistered).
    Expected: NOT GT_OK.
    5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    6. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    7. Call with wrong enum values dumpType.
    Expected: GT_BAD_PARAM.
    8. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT    dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E;
    GT_U32                                  clientId = 0;

    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call with dumpType
                   [CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E                            /
                    CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_ALL_CLIENTS_E            /
                    CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_AND_ENTRIES_FOR_ALL_CLIENTS_E]
            Expected: GT_OK.
        */
        /* call with dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E */
        dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E;

        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* call with dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_ALL_CLIENTS_E */
        dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_ALL_CLIENTS_E;

        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* call with dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_AND_ENTRIES_FOR_ALL_CLIENTS_E */
        dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_AND_ENTRIES_FOR_ALL_CLIENTS_E;

        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerClientRegister with
                    clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
                    clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
            Expected: GT_OK.
        */
        /* client registrations */
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                            "Client1", &clientFunc, &clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call with dumpType [CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E]
            and correct clientId (registered).
            Expected: GT_OK.
        */
        dumpType = CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E;

        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, clientId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            4. Call with dumpType [CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E]
            and wrong clientId (unregistered).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, (clientId != 0)?0:1);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerDump(tcamManagerHandlerPtr, dumpType, clientId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        /*
            6. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerDump(NULL, dumpType, clientId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            7. Call with wrong enum values dumpType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTcamManagerDump
                            (tcamManagerHandlerPtr, dumpType, clientId),
                            dumpType);

        /*
            8. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerEntriesReservationSet
(
    IN  GT_VOID                                             *tcamManagerHandlerPtr,
    IN  GT_U32                                              clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             requestedEntriesArray[],
    IN  GT_U32                                              sizeOfArray,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    *reservationRangePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerEntriesReservationSet)
{
/*
    1. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E,
       requestedEntriesArray[0].amount = 10,
       reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E,
       and sizeOfArray [1].
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    3. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E,
       requestedEntriesArray[0].amount = 20 and sizeOfArray [1].
    Expected: GT_OK.
    4. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    5. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E,
       requestedEntriesArray[0].amount = 5 and sizeOfArray [1].
    Expected: GT_OK.
    6. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    7. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E,
       requestedEntriesArray[0].amount = 1 and sizeOfArray [1].
    Expected: GT_OK.
    8. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    9. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
            requestedEntriesArray[2].amount    = 3;
       and sizeOfArray [3].
    Expected: GT_OK.
    10. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    11. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
            requestedEntriesArray[2].amount    = 3;
            requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[3].amount    = 1;
       and sizeOfArray [3].
    Expected: GT_OK.
    12. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    13. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            requestedEntriesArray[0].amount    = 7;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[2].amount    = 1;
       and sizeOfArray [3].
    Expected: GT_OK.
    14. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    15. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
    Expected: NOT GT_OK (reservation type for the same client can't be changed)
    16. Register second client to test static reservation
    Expected: GT_OK.
    17. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E].
    Expected: GT_OK.
    18. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
    Expected: GT_OK and the same values.
    19. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
    and wrong reservation range (the same static range again).
    Expected: NOT GT_OK.
    20. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
    and wrong reservation range (out of tcam range).
    Expected: NOT GT_OK.
    21. Call with reservationType = CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E,
        and reservationRange.lastLine  = 0x3FFFF + 1 (out of range).
    Expected: NOT GT_OK.
    22. Call with reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E,
        and reservationRange.lastLine  = 0x3FFFF + 1 (not relevant for dynamic).
    Expected: GT_OK.
    23. Call with reservationRange.lastLine less than firstline.
    Expected: NOT GT_OK.
    24. Call with
            requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[0].amount    = 3;
            requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
            requestedEntriesArray[1].amount    = 5;
            requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            requestedEntriesArray[2].amount    = 1;
       and sizeOfArray [3].
    Expected: GT_BAD_PARAM (the same entryType twice).
    25. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    26. Call with non-existent clientId.
    Expected: NOT GT_OK.
    27. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    28. Call with wrong requestedEntriesArray[NULL].
    Expected: GT_BAD_PTR.
    29. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId = 0;
    CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArray[4];
    GT_U32                                  sizeOfArray = 4;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        reservationRange;

    CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationTypeGet;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArrayGet[4];
    GT_U32                                  sizeOfArrayGet = 4;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        reservationRangeGet;

    GT_U32                                  tempClientId;
    GT_U32                                  clientId2 = 0;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    GT_U32                                  i = 0; /*index to iterate in array*/

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(requestedEntriesArray,0,sizeof(requestedEntriesArray));
        cpssOsMemSet(requestedEntriesArrayGet,0,sizeof(requestedEntriesArrayGet));

        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E,
               requestedEntriesArray[0].amount = 10,
               reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E,
               and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 10;
        sizeOfArray = 1;
        reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E;
        reservationRange.firstLine = 0;
        reservationRange.lastLine  = 10;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            2. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].entryType,
                                     requestedEntriesArrayGet[0].entryType,
             "got another requestedEntriesArray[0].entryType as was written: %d, %d",
             requestedEntriesArray[0].entryType, requestedEntriesArrayGet[0].entryType);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].amount,
                                     requestedEntriesArrayGet[0].amount,
             "got another requestedEntriesArray[0].amount as was written: %d, %d",
             requestedEntriesArray[0].amount, requestedEntriesArrayGet[0].amount);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/

        /*
            3. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E,
               requestedEntriesArray[0].amount = 20 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[0].amount    = 20;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            4. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].entryType,
                                     requestedEntriesArrayGet[0].entryType,
             "got another requestedEntriesArray[0].entryType as was written: %d, %d",
             requestedEntriesArray[0].entryType, requestedEntriesArrayGet[0].entryType);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].amount,
                                     requestedEntriesArrayGet[0].amount,
             "got another requestedEntriesArray[0].amount as was written: %d, %d",
             requestedEntriesArray[0].amount, requestedEntriesArrayGet[0].amount);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/

        /*
            5. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E,
               requestedEntriesArray[0].amount = 5 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 5;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            6. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].entryType,
                                     requestedEntriesArrayGet[0].entryType,
             "got another requestedEntriesArray[0].entryType as was written: %d, %d",
             requestedEntriesArray[0].entryType, requestedEntriesArrayGet[0].entryType);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].amount,
                                     requestedEntriesArrayGet[0].amount,
             "got another requestedEntriesArray[0].amount as was written: %d, %d",
             requestedEntriesArray[0].amount, requestedEntriesArrayGet[0].amount);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/

        /*
            7. Call with requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E,
               requestedEntriesArray[0].amount = 1 and sizeOfArray [1].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[0].amount    = 1;
        sizeOfArray = 1;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            8. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].entryType,
                                     requestedEntriesArrayGet[0].entryType,
             "got another requestedEntriesArray[0].entryType as was written: %d, %d",
             requestedEntriesArray[0].entryType, requestedEntriesArrayGet[0].entryType);

        UTF_VERIFY_EQUAL2_STRING_MAC(requestedEntriesArray[0].amount,
                                     requestedEntriesArrayGet[0].amount,
             "got another requestedEntriesArray[0].amount as was written: %d, %d",
             requestedEntriesArray[0].amount, requestedEntriesArrayGet[0].amount);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/


        /*
            9. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
                    requestedEntriesArray[2].amount    = 3;
               and sizeOfArray [3].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[2].amount    = 3;

        sizeOfArray = 3;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            10. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        for(i = 0; i > sizeOfArray; i++)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].entryType,
                                         requestedEntriesArrayGet[i].entryType,
                 "got another requestedEntriesArray[%d].entryType as was written: %d, %d",
               i, requestedEntriesArray[i].entryType, requestedEntriesArrayGet[i].entryType);

            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].amount,
                                         requestedEntriesArrayGet[i].amount,
                 "got another requestedEntriesArray[%d].amount as was written: %d, %d",
               i, requestedEntriesArray[i].amount, requestedEntriesArrayGet[i].amount);
        }

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/

        /*
            11. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
                    requestedEntriesArray[2].amount    = 3;
                    requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[3].amount    = 1;
               and sizeOfArray [3].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E;
        requestedEntriesArray[2].amount    = 3;
        requestedEntriesArray[3].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[3].amount    = 1;

        sizeOfArray = 4;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            12. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        for(i = 0; i > sizeOfArray; i++)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].entryType,
                                         requestedEntriesArrayGet[i].entryType,
                 "got another requestedEntriesArray[%d].entryType as was written: %d, %d",
               i, requestedEntriesArray[i].entryType, requestedEntriesArrayGet[i].entryType);

            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].amount,
                                         requestedEntriesArrayGet[i].amount,
                 "got another requestedEntriesArray[%d].amount as was written: %d, %d",
               i, requestedEntriesArray[i].amount, requestedEntriesArrayGet[i].amount);
        }

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/


        /*
            13. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    requestedEntriesArray[0].amount    = 7;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[2].amount    = 1;
               and sizeOfArray [3].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 7;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[2].amount    = 1;

        sizeOfArray = 3;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            14. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray as it is INOUT param for get func*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        for(i = 0; i > sizeOfArray; i++)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].entryType,
                                         requestedEntriesArrayGet[i].entryType,
                 "got another requestedEntriesArray[%d].entryType as was written: %d, %d",
               i, requestedEntriesArray[i].entryType, requestedEntriesArrayGet[i].entryType);

            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].amount,
                                         requestedEntriesArrayGet[i].amount,
                 "got another requestedEntriesArray[%d].amount as was written: %d, %d",
               i, requestedEntriesArray[i].amount, requestedEntriesArrayGet[i].amount);
        }

        /*reservationRangeGet not verified cause not relevant for dynamic type allocation*/

        /*
            15. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
            Expected: NOT GT_OK (reservation type for the same client can't be changed)
        */
        reservationType = CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E;
        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);
        /* restore reservationType */
        reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E;

        /*
            16. Register second client to test static reservation
            Expected: GT_OK.
        */
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;
        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                            "Client2", &clientFunc, &clientId2);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            17. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E].
            Expected: GT_OK.
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        requestedEntriesArray[0].amount    = 10;
        sizeOfArray = 1;
        reservationType = CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E;
        reservationRange.firstLine = 0;
        reservationRange.lastLine  = 10;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        /*
            18. Call cpssDxChTcamManagerEntriesReservationGet with the same parameters.
            Expected: GT_OK and the same values.
        */
        /*save sizeOfArray (it is INOUT param for get func)*/
        sizeOfArrayGet = sizeOfArray;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId2,
          &reservationTypeGet, requestedEntriesArrayGet, &sizeOfArrayGet, &reservationRangeGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        /* verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(reservationType, reservationTypeGet,
                  "got another reservationType as was written: %d, %d",
                                     reservationType, reservationTypeGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(sizeOfArray, sizeOfArrayGet,
                  "got another sizeOfArray as was written: %d, %d",
                                     sizeOfArray, sizeOfArrayGet);

        for(i = 0; i > sizeOfArray; i++)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].entryType,
                                         requestedEntriesArrayGet[i].entryType,
                 "got another requestedEntriesArray[%d].entryType as was written: %d, %d",
               i, requestedEntriesArray[i].entryType, requestedEntriesArrayGet[i].entryType);

            UTF_VERIFY_EQUAL3_STRING_MAC(requestedEntriesArray[i].amount,
                                         requestedEntriesArrayGet[i].amount,
                 "got another requestedEntriesArray[%d].amount as was written: %d, %d",
               i, requestedEntriesArray[i].amount, requestedEntriesArrayGet[i].amount);
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(reservationRange.firstLine, reservationRangeGet.firstLine,
                  "got another reservationRange.firstLine as was written: %d, %d",
                                     reservationRange.firstLine, reservationRangeGet.firstLine);

        UTF_VERIFY_EQUAL2_STRING_MAC(reservationRange.lastLine, reservationRangeGet.lastLine,
                  "got another reservationRange.lastLine as was written: %d, %d",
                                     reservationRange.lastLine, reservationRangeGet.lastLine);

        /*
            19. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
            and wrong reservation range (the same static range again).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        /*
            20. Call with static reservationType [CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E]
            and wrong reservation range (out of tcam range).
            Expected: NOT GT_OK.
        */
        reservationRange.firstLine = 1000;
        reservationRange.lastLine  = 2000;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        reservationRange.firstLine = 0;
        reservationRange.lastLine  = 10;

        /*
            21. Call with reservationType = CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E,
                and reservationRange.lastLine  = 0x3FFFF + 1 (out of range).
            Expected: NOT GT_OK.
        */
        reservationType = CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E;
        reservationRange.lastLine  = 0x3FFFF + 1;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        reservationRange.lastLine  = 10;

        /*
            22. Call with reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E,
                and reservationRange.lastLine  = 0x3FFFF + 1 (not relevant for dynamic).
            Expected: NOT GT_OK.
        */
        reservationType = CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E;
        reservationRange.lastLine  = 0x3FFFF + 1;

        /* current client is set as static client, therefore this API call is trying
           to change the reservation type and will result NOT GT_OK; in order to
           check that reservation range is not relevant for dynamic clients,
           a dynamic client should be used */
        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        reservationRange.lastLine  = 10;

        /*
            23. Call with reservationRange.lastLine less than firstline.
            Expected: NOT GT_OK.
        */
        reservationRange.firstLine = 11;
        reservationRange.lastLine  = 10;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId2, sizeOfArray);

        reservationRange.firstLine = 0;
        reservationRange.lastLine  = 10;

        /*
            24. Call with
                    requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[0].amount    = 3;
                    requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                    requestedEntriesArray[1].amount    = 5;
                    requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
                    requestedEntriesArray[2].amount    = 1;
               and sizeOfArray [3].
            Expected: GT_BAD_PARAM (the same entryType twice).
        */
        requestedEntriesArray[0].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[0].amount    = 1;
        requestedEntriesArray[1].entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        requestedEntriesArray[1].amount    = 5;
        requestedEntriesArray[2].entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        requestedEntriesArray[2].amount    = 1;
        sizeOfArray = 3;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, clientId2, sizeOfArray);

        /*
            25. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationSet(NULL, clientId2,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            26. Call with non-existent clientId.
            Expected: NOT GT_OK.
        */
        if ((clientId == 0) || (clientId2 == 0))
            tempClientId = (((clientId == 1) || (clientId2 == 1)) ? 2 : 1);
        else
            tempClientId = 0;
        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr,tempClientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, tempClientId, sizeOfArray);

        /*
            27. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        tempClientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, tempClientId,
                reservationType, requestedEntriesArray, sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            28. Call with wrong requestedEntriesArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationSet(tcamManagerHandlerPtr, clientId,
                reservationType, NULL, sizeOfArray, &reservationRange);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, requestedEntriesArray = NULL");

        /*
            29. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerEntriesReservationGet
(
    IN      GT_VOID                                             *tcamManagerHandlerPtr,
    IN      GT_U32                                              clientId,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT *reservationTypePtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             reservedEntriesArray[],
    INOUT   GT_U32                                              *sizeOfArrayPtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    *reservationRangePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerEntriesReservationGet)
{
/*
    1. Call cpssDxChTcamManagerEntriesReservationGet with correct parameters.
    Expected: GT_OK.
    2. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    3. Call with non-existent clientId.
    Expected: NOT GT_OK.
    4. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    5. Call with wrong reservationTypeGetPtr[NULL].
    Expected: GT_BAD_PTR.
    6. Call with wrong requestedEntriesArray[NULL].
    Expected: GT_BAD_PTR.
    7. Call with requestedEntriesArray size[3].
    Expected: GT_OK.
    8. Call with wrong sizeOfArrayPtr[NULL].
    Expected: GT_BAD_PTR.
    9. Call with wrong reservationRangePtr[NULL].
    Expected: GT_BAD_PTR.
    10. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId = 0;
    CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArray[4];
    GT_U32                                  sizeOfArray = 4;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        reservationRange;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    cpssOsBzero((GT_VOID*)requestedEntriesArray, sizeof(requestedEntriesArray));

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntriesReservationGet with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, requestedEntriesArray, &sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            2. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(NULL, clientId,
          &reservationType, requestedEntriesArray, &sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            3. Call with non-existent clientId.
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr,
                                                      ((clientId != 0) ? 0 : 1),
          &reservationType, requestedEntriesArray, &sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, clientId, sizeOfArray);

        /*
            4. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, requestedEntriesArray, &sizeOfArray, &reservationRange);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            5. Call with wrong reservationTypeGetPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          NULL, requestedEntriesArray, &sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, reservationTypePtr = NULL");

        /*
            6. Call with wrong requestedEntriesArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, NULL, &sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, requestedEntriesArray = NULL");

        /*
            7. Call with requestedEntriesArray size[3].
            Expected: GT_OK.
        */
        sizeOfArray = 3;

        /* sizeOfArray is the size of the array allocated by the caller of the API */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, requestedEntriesArray, &sizeOfArray, &reservationRange);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "%d, requestedEntriesArray");

        sizeOfArray = 4;

        /*
            8. Call with wrong sizeOfArrayPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, requestedEntriesArray, NULL, &reservationRange);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, sizeOfArrayPtr = NULL");

        /*
            9. Call with wrong reservationRangePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntriesReservationGet(tcamManagerHandlerPtr, clientId,
          &reservationType, requestedEntriesArray, &sizeOfArray, NULL);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, reservationRangePtr = NULL");

        /*
            10. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerEntryAllocate
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType,
    IN  GT_U32                                  lowerBoundToken,
    IN  GT_U32                                  upperBoundToken,
    IN  CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod,
    IN  GT_BOOL                                 defragEnable,
    IN  GT_VOID                                 *clientCookiePtr,
    OUT GT_U32                                  *allocEntryTokenPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerEntryAllocate)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
        entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        lowerBoundToken = previous allocEntryToken;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MID_E;
        defragEnable = GT_TRUE;
    Expected: GT_OK.
    3. Call with wrong entryTokens (not exists)
    Expected: NOT GT_OK.
    4. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    7. Call with wrong enum values entryType.
    Expected: GT_BAD_PARAM.
    8. Call with wrong enum values allocMethod.
    Expected: GT_BAD_PARAM.
    9. Call with wrong allocEntryTokenPtr[NULL].
    Expected: GT_BAD_PTR.
    10. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;
    GT_VOID                                 *clientCookiePtr = 0;
    GT_U32                                  allocEntryToken;
    GT_U32                                  i; /*index to iterate tokens allocation*/

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        for(i = 0; i < 6; i++)
        {
            st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                        entryType, lowerBoundToken, upperBoundToken, allocMethod,
                        defragEnable, clientCookiePtr, &allocEntryToken);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            /*save allocated tokens to use as boundary later*/
            if(GT_OK == st && i == 0)
            {
                lowerBoundToken = allocEntryToken;
            }
            else if (GT_OK == st)
            {
                st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr,
                                                    clientId, allocEntryToken);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /*
            2. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
                lowerBoundToken = previous allocated allocEntryToken;
                upperBoundToken = 0;
                allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MID_E;
                defragEnable = GT_TRUE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MID_E;
        defragEnable = GT_FALSE;
        upperBoundToken = 0;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &allocEntryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call with wrong entryTokens (not exists)
            Expected: NOT GT_OK.
        */
        lowerBoundToken = 5;
        upperBoundToken = 20;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &allocEntryToken);

        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        lowerBoundToken = 0;
        upperBoundToken = 0;

        /*
            4. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntryAllocate(NULL, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &allocEntryToken);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            5. Call with wrong clientId (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr,
                                              ((clientId != 0) ? 0 : 1),
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &allocEntryToken);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId");

        /*
            6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &allocEntryToken);

        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            7. Call with wrong enum values entryType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTcamManagerEntryAllocate
                            (tcamManagerHandlerPtr, clientId, entryType,
                             lowerBoundToken, upperBoundToken, allocMethod,
                             defragEnable, clientCookiePtr, &allocEntryToken),
                            entryType);
        /*
            8. Call with wrong enum values allocMethod.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTcamManagerEntryAllocate
                            (tcamManagerHandlerPtr, clientId, entryType,
                             lowerBoundToken, upperBoundToken, allocMethod,
                             defragEnable, clientCookiePtr, &allocEntryToken),
                            allocMethod);

        /*
            9. Call with wrong allocEntryTokenPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, allocEntryTokenPtr = NULL");

        /*
            10. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerEntryClientCookieUpdate
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  GT_U32                                   entryToken,
    IN  GT_VOID                                 *clientCookiePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerEntryClientCookieUpdate)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerEntryClientCookieUpdate for registered client
    with the same clientId and created entryToken.
    Expected: GT_OK.
    3. Call cpssDxChTcamManagerEntryClientCookieUpdate for
    entryToken allocated to other client (wrong client entryToken).
    Expected: NOT GT_OK.
    4. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    7. Call with wrong entryToken (not existent).
    Expected: NOT GT_OK.
    8. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId1;
    GT_U32                                  entryToken = 0;
    GT_VOID                                 *clientCookiePtr = 0;

    GT_U32                                  clientId2;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManagerWithClients(&clientId1, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId1,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerEntryClientCookieUpdate for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerEntryClientCookieUpdate(tcamManagerHandlerPtr, clientId1,
                                                        entryToken, clientCookiePtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call cpssDxChTcamManagerEntryClientCookieUpdate for
            entryToken allocated to other client (wrong client entryToken).
            Expected: NOT GT_OK.
        */

        /*register clients*/
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client2", &clientFunc, &clientId2);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*allocate entry token*/
        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId2,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*call with incorrect entryToken (for client2, but we call to client1)*/
        st = cpssDxChTcamManagerEntryClientCookieUpdate(tcamManagerHandlerPtr, clientId1,
                                                        entryToken, clientCookiePtr);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            4. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntryClientCookieUpdate(NULL, clientId2,
                                                        entryToken, clientCookiePtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            5. Call with wrong clientId (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntryClientCookieUpdate(tcamManagerHandlerPtr,
                    ((clientId2 != 0) ? 0 : 1), entryToken, clientCookiePtr);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId");

        /*
            6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId2 = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerEntryClientCookieUpdate(tcamManagerHandlerPtr, clientId2,
                                                        entryToken, clientCookiePtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId2);

        clientId2 = 0;

        /*
            7. Call with wrong entryToken (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntryClientCookieUpdate(tcamManagerHandlerPtr, clientId2,
                                           ((entryToken != 0) ? 0 : 1), clientCookiePtr);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent entryToken");

        /*
            8. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerEntryFree
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  GT_U32                                  entryToken,
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerEntryFree)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerEntryFree for registered client
    with the same clientId and created entryToken.
    Expected: GT_OK.
    3. Call entryToken allocated to other client (wrong client entryToken).
    Expected: NOT GT_OK.
    4. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    7. Call with wrong entryToken (not existent).
    Expected: NOT GT_OK.
    8. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;
    GT_U32                                  entryToken = 0;
    GT_VOID                                 *clientCookiePtr = 0;

    GT_U32                                  clientId2;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerEntryFree for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId, entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call entryToken allocated to other client (wrong client entryToken).
            Expected: NOT GT_OK.
        */
        /*register clients*/
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client2", &clientFunc, &clientId2);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*allocate entry token*/
        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId2,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*call with incorrect entryToken (for client2, but we call to client1)*/
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId, entryToken);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            4. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerEntryFree(NULL, clientId2, entryToken);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            5. Call with wrong clientId2 (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr,
                                            ((clientId2 != 0) ? 0 : 1), entryToken);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId2");

        /*
            6. Call with wrong clientId2 [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId2 = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId2, entryToken);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId2);

        clientId2 = 0;

        /*
            7. Call with wrong entryToken (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId2,
                                            ((entryToken != 0) ? 0 : 1));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent entryToken");

        /*
            8. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerRangeUpdate
(
    IN  GT_VOID                                         *tcamManagerHandlerPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC                *rangePtr,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT  rangeUpdateMethod
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerRangeUpdate)
{
/*
    1. Call with range.firstLine[50], range.lastLine[100],
    rangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerConfigGet with the same tcamManagerHandlerPtr.
    Expected: GT_OK and the same params.
    3. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong rangePtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
    to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamTypeGet;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        range;

    GT_VOID                                 *tcamManagerHandlerPtr;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType =
                            CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        rangeGet;
    CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT  rangeUpdateMethod =
                        CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call with range.firstLine[50], range.lastLine[100],
            rangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E;
            Expected: GT_OK.
        */
        range.firstLine = 50;
        range.lastLine  = 100;
        rangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E;

        st = cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr, &range, rangeUpdateMethod);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, tcamType);

        if(GT_OK == st)
        {
            /*
                2. Call cpssDxChTcamManagerConfigGet with the same tcamManagerHandlerPtr.
                Expected: GT_OK and the same params.
            */
            st = cpssDxChTcamManagerConfigGet(tcamManagerHandlerPtr, &tcamTypeGet, &rangeGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerConfigGet failed");

            if(GT_OK == st)
            {
                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tcamType, tcamTypeGet,
                          "got another tcamType as was written: %d, %d", tcamType, tcamTypeGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(range.firstLine, rangeGet.firstLine,
                          "got another range.firstLine as was written: %d, %d",
                                             range.firstLine, rangeGet.firstLine);
                UTF_VERIFY_EQUAL2_STRING_MAC(range.lastLine, rangeGet.lastLine,
                          "got another range.lastLine as was written: %d, %d",
                                             range.lastLine, rangeGet.lastLine);
            }
        }

        /*
            3. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerRangeUpdate(NULL, &range, rangeUpdateMethod);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "%d, tcamManagerHandlerPtr = NULL", tcamType);

        /*
            4. Call with wrong rangePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr, NULL, rangeUpdateMethod);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rangePtr = NULL", tcamType);

        /*
            5. Call cpssDxChTcamManagerDelete with the same tcamManagerHandlerPtr.
            to restore configuration.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerTokenCompare
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      firstToken,
    IN  GT_U32                                      secondToken,
    OUT GT_COMP_RES                                 *resultPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerTokenCompare)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    3. Call cpssDxChTcamManagerTokenCompare for two created tokens.
    Expected: GT_OK and correct results of compare.
    4. Call cpssDxChTcamManagerEntryFree for registered client
    with the same clientId and created entryToken.
    Expected: GT_OK.
    5. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    6. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    7. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    8. Call with wrong resutlPtr[NULL].
    Expected: GT_BAD_PTR.
    9. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;
    GT_U32                                  entryToken = 0;
    GT_VOID                                 *clientCookiePtr = 0;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;

    GT_U32                                  firstToken = 0;
    GT_U32                                  secondToken = 0;
    GT_COMP_RES                             result;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*save first token*/
        firstToken = entryToken;

        /*
            2. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MAX_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*save second token*/
        secondToken = entryToken;

        /*
            3. Call cpssDxChTcamManagerTokenCompare for two created tokens.
            Expected: GT_OK and correct results of compare.
        */
        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr, clientId,
                                             firstToken, secondToken, &result);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_SMALLER, result, "wrong result %d", result);

        /*call with changed tokens*/
        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr, clientId,
                                             secondToken, firstToken, &result);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_GREATER, result, "wrong result %d", result);

        /*call with the same tokens */
        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr, clientId,
                                             firstToken, firstToken, &result);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_EQUAL, result, "wrong result %d", result);

        /*
            4. Call cpssDxChTcamManagerEntryFree for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId, firstToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId, secondToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            5. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenCompare(NULL, clientId,
                                             firstToken, secondToken, &result);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            6. Call with wrong clientId (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr,
                                            ((clientId != 0) ? 0 : 1),
                                             firstToken, secondToken, &result);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId");

        /*
            7. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr, clientId,
                                             firstToken, firstToken, &result);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            8. Call with wrong resutlPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenCompare(tcamManagerHandlerPtr, clientId,
                                             firstToken, secondToken, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, resultPtr = NULL");

        /*
            9. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerTokenToTcamLocation
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      entryToken,
    OUT CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *tcamLocationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerTokenToTcamLocation)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerTokenToTcamLocation for registered client
    with the same clientId and created entryToken.
    Expected: GT_OK.
    3. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    6. Call with wrong entryToken (not existent).
    Expected: NOT GT_OK.
    7. Call with wrong tcamLocationPtr[NULL].
    Expected: GT_BAD_PTR.
    8. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_U32                                  clientId;
    GT_U32                                  entryToken = 0;
    CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC  tcamLocation;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;
    GT_VOID                                 *clientCookiePtr = 0;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerTokenToTcamLocation for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerTokenToTcamLocation(tcamManagerHandlerPtr, clientId,
                                                    entryToken, &tcamLocation);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenToTcamLocation(NULL, clientId,
                                                    entryToken, &tcamLocation);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            4. Call with wrong clientId (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerTokenToTcamLocation(tcamManagerHandlerPtr,
                                                    ((clientId != 0) ? 0 : 1),
                                                    entryToken, &tcamLocation);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId");

        /*
            5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerTokenToTcamLocation(tcamManagerHandlerPtr, clientId,
                                                    entryToken, &tcamLocation);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            6. Call with wrong entryToken (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerTokenToTcamLocation(tcamManagerHandlerPtr, clientId,
                                                    ((entryToken != 0) ? 0 : 1),
                                                    &tcamLocation);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent entryToken");

        /*
            7. Call with wrong tcamLocationPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenToTcamLocation(tcamManagerHandlerPtr, clientId,
                                                    entryToken, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, LocationPtr = NULL");

        /*
            8. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/* API cpssDxChTcamManagerUsageGet not implemented */
#if 0
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerUsageGet
(
    IN      GT_VOID                                 *tcamManagerHandlerPtr,
    IN      GT_BOOL                                 usageOfSpecificClient,
    IN      GT_U32                                  clientId,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC reservedEntriesArray[],
    INOUT   GT_U32                                  *sizeOfReservedArrayPtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC allocatedEntriesArray[],
    INOUT   GT_U32                                  *sizeofAllocatedArrayPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerUsageGet)
{
/*
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
    with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerUsageGet for created token.
    Expected: GT_OK.
    3. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    4. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    6. Call with wrong reservedEntriesArray[NULL].
    Expected: GT_BAD_PTR.
    7. Call with wrong sizeOfReservedArray[NULL].
    Expected: GT_BAD_PTR.
    8. Call with wrong allocatedEntriesArray[NULL].
    Expected: GT_BAD_PTR.
    9. Call with wrong sizeOfAllocatedArray[NULL].
    Expected: GT_BAD_PTR.
    10. Call cpssDxChTcamManagerEntryFree for registered client
    with the same clientId and created entryToken.
    Expected: GT_OK.
    11. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID                                 *tcamManagerHandlerPtr;
    GT_BOOL                                 usageOfSpecificClient = GT_TRUE;
    GT_U32                                  clientId;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC reservedEntriesArray[1];
    GT_U32                                  sizeOfReservedArray = 1;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC allocatedEntriesArray[1];
    GT_U32                                  sizeofAllocatedArray = 1;

    GT_U32                                  entryToken = 0;

    GT_VOID                                 *clientCookiePtr = 0;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;

    GT_U8      dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerUsageGet for created token.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                clientId, reservedEntriesArray, &sizeOfReservedArray,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerUsageGet(NULL, usageOfSpecificClient,
                                clientId, reservedEntriesArray, &sizeOfReservedArray,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            4. Call with wrong clientId (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerUsageGet(NULL, usageOfSpecificClient,
                                         ((clientId != 0) ? 0 : 1),
                                         reservedEntriesArray, &sizeOfReservedArray,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId");

        /*
            5. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                clientId, reservedEntriesArray, &sizeOfReservedArray,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId);

        clientId = 0;

        /*
            6. Call with wrong reservedEntriesArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                         clientId, NULL, &sizeOfReservedArray,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, reservedEntriesArray = NULL");

        /*
            7. Call with wrong sizeOfReservedArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                         clientId, reservedEntriesArray, NULL,
                                         allocatedEntriesArray, &sizeofAllocatedArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, sizeOfReservedArrayPtr = NULL");

        /*
            8. Call with wrong allocatedEntriesArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                         clientId, NULL, &sizeOfReservedArray,
                                         NULL, &sizeofAllocatedArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, allocatedEntriesArray = NULL");

        /*
            9. Call with wrong sizeOfAllocatedArray[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerUsageGet(tcamManagerHandlerPtr, usageOfSpecificClient,
                                clientId, reservedEntriesArray, &sizeOfReservedArray,
                                         allocatedEntriesArray, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, sizeOfAllocatedArrayPtr = NULL");

        /*
            10. Call cpssDxChTcamManagerEntryFree for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerEntryFree(tcamManagerHandlerPtr, clientId, entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            11. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}
#endif

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerHsuSizeGet
(
    IN  GT_VOID     *tcamManagerHandlerPtr,
    OUT GT_U32      *sizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerHsuSizeGet)
{
/*
    1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
    Expected: GT_OK.
    1.2. Call with not null tcamManagerHandlerPtr
                   not null sizePtr.
    Expected: GT_OK.
    1.3. Call with wrong tcamManagerHandlerPtr[NULL].
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.4. Call with wrong sizePtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;

    GT_VOID *tcamManagerHandlerPtr;
    GT_U32  size;
    GT_U8   dev = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
            Expected: GT_OK.
        */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call with not null tcamManagerHandlerPtr
                           not null sizePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerHsuSizeGet(tcamManagerHandlerPtr, &size);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.3. Call with wrong tcamManagerHandlerPtr[NULL].
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuSizeGet(NULL, &size);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "tcamManagerHandlerPtr = NULL");

        /*
            1.4. Call with wrong sizePtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuSizeGet(tcamManagerHandlerPtr, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "sizePtr = NULL");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerClientFuncUpdate
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  *clientFuncPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerClientFuncUpdate)
{
/*
    1.1. Call prvCreateDefaultTcamManagerWithClients to create default TCAM manager
    Expected: GT_OK.
    1.2. Call with valid tcamManagerHandlerPtr
                   valid clientId
            clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
            clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
    Expected: GT_OK.
    1.3. Call with wrong tcamManagerHandlerPtr[NULL].
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.4. Call with wrong clientId[100].
                   and other valid parameters
    Expected: NOT GT_OK.
    1.5. Call with wrong clientFuncPtr[NULL].
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.6. Call with wrong clientFunc.moveToLocationFuncPtr [NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.7. Call with wrong clientFunc.moveToAnywhereFuncPtr [NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.

    1.8. Call with wrong clientFunc.checkIfDefaultLocationFuncPtr [NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    GT_VOID    *tcamManagerHandlerPtr;
    GT_U32     clientId = 0;
    GT_U32     clientIdTemp = 0;
    GT_U8      dev = 0;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC clientFunc;

    /*register clients*/
    clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
    clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
    clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCreateDefaultTcamManagerWithClients to create default TCAM manager
            Expected: GT_OK.
        */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call with valid tcamManagerHandlerPtr
                           valid clientId
                    clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
                    clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr,
                                                 clientId, &clientFunc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.3. Call with wrong tcamManagerHandlerPtr[NULL].
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientFuncUpdate(NULL, clientId, &clientFunc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "tcamManagerHandlerPtr = NULL");

        /*
            1.4. Call with wrong clientId[100].
                           and other valid parameters
            Expected: NOT GT_OK.
        */
        clientIdTemp = clientId;
        clientId = 100;

        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr,
                                                 clientId, &clientFunc);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        clientId = clientIdTemp;

        /*
            1.5. Call with wrong clientFuncPtr[NULL].
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr, clientId, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "clientFuncPtr = NULL");

        /*
            1.6. Call with wrong clientFunc.moveToLocationFuncPtr [NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        clientFunc.moveToLocationFuncPtr = NULL;

        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr,
                                                 clientId, &clientFunc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "clientFunc.moveToLocationFuncPtr = NULL");

        /* restore clientFunc.moveToLocationFuncPtr value */
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;

        /*
            1.7. Call with wrong clientFunc.moveToAnywhereFuncPtr [NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        clientFunc.moveToAnywhereFuncPtr = NULL;

        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr,
                                                 clientId, &clientFunc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "clientFunc.moveToAnywhereFuncPtr = NULL");

        /* restore clientFunc.moveToLocationFuncPtr value */
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;

        /*
            1.8. Call with wrong clientFunc.checkIfDefaultLocationFuncPtr [NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        clientFunc.checkIfDefaultLocationFuncPtr = NULL;

        st = cpssDxChTcamManagerClientFuncUpdate(tcamManagerHandlerPtr,
                                                 clientId, &clientFunc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "clientFunc.checkIfDefaultLocationFuncPtr = NULL");

        /* restore clientFunc.moveToLocationFuncPtr value */
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerHsuExport
(
    IN    GT_VOID   *tcamManagerHandlerPtr,
    INOUT GT_U32    *iteratorPtr,
    INOUT GT_U32    *hsuBlockMemSizePtr,
    IN    GT_U8     *hsuBlockMemPtr,
    OUT   GT_BOOL   *exportCompletePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerHsuExport)
{
/*
    1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
    Expected: GT_OK.
    1.2. Call cpssDxChTcamManagerHsuSizeGet to get HSU data size
    Expected: GT_OK.
    1.3. Call with valid tcamManagerHandlerPtr,
                   iteratorPtr [0],
                   hsuBlockMemSizePtr[CPSS_HSU_SINGLE_ITERATION_CNS],
                   non NULL hsuBlockMemPtr,
                   non NULL exportCompletePtr.
    Expected: GT_OK.
    1.4. Call with valid tcamManagerHandlerPtr,
                   iteratorPtr [0],
                   hsuBlockMemSizePtr[1024],
                   non NULL hsuBlockMemPtr,
                   non NULL exportCompletePtr.
    Expected: GT_OK.
    1.5. Call with hsuBlockMemSize[CPSS_HSU_SINGLE_ITERATION_CNS]
                   iterator[100] (not relevant)
                   and other valid parameters
    Expected: GT_OK.
    1.6. Call with null tcamManagerHandlerPtr [NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.7. Call with wrong iteratorPtr[NULL].
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.8. Call with wrong hsuBlockMemSize[1023] (out of range)
                   and other valid parameters
    Expected: NOT GT_OK.
    1.9. Call with wrong hsuBlockMemSizePtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.10. Call with wrong hsuBlockMemPtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.11. Call with wrong exportCompletePtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_VOID   *tcamManagerHandlerPtr;
    GT_U8     dev = 0;
    GT_UINTPTR iterator = 0;
    GT_U32    hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;
    GT_U8     *hsuBlockMemPtr;
    GT_BOOL   exportComplete = GT_FALSE;
    GT_U32    size = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
            Expected: GT_OK.
        */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call cpssDxChTcamManagerHsuSizeGet to get HSU data size
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerHsuSizeGet(tcamManagerHandlerPtr, &size);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        hsuBlockMemPtr = cpssOsMalloc(size);
        st = NULL == hsuBlockMemPtr ? GT_NO_RESOURCE : GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "hsuBlockMemPtr = NULL");
        if (NULL == hsuBlockMemPtr)
        {
            return;
        }

        /*
            1.3. Call with valid tcamManagerHandlerPtr,
                           iteratorPtr [0],
                           hsuBlockMemSizePtr[CPSS_HSU_SINGLE_ITERATION_CNS],
                           non NULL hsuBlockMemPtr,
                           non NULL exportCompletePtr.
            Expected: GT_OK.
        */
        iterator = 0;
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.4. Call with valid tcamManagerHandlerPtr,
                           iteratorPtr [0],
                           hsuBlockMemSizePtr[1024],
                           non NULL hsuBlockMemPtr,
                           non NULL exportCompletePtr.
            Expected: GT_OK.
        */
        iterator = 0;
        hsuBlockMemSize = 1024;

        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.5. Call with hsuBlockMemSize[CPSS_HSU_SINGLE_ITERATION_CNS]
                   iterator[100] (not relevant)
                   and other valid parameters
            Expected: GT_OK.
        */
        iterator = 100;
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore valid values */
        iterator = 0;

        /*
            1.6. Call with null tcamManagerHandlerPtr [NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuExport(NULL, &iterator, &hsuBlockMemSize,
                                          hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "tcamManagerHandlerPtr = NULL");

        /*
            1.7. Call with wrong iteratorPtr[NULL].
                 and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, NULL,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "iteratorPtr = NULL");

        /*
            1.8. Call with hsuBlockMemSize[1023]
                           and other valid parameters
            Expected: GT_OK.
        */
        hsuBlockMemSize = 1023;

        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore valid values */
        iterator = 0;
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        /*
            1.9. Call with wrong hsuBlockMemSizePtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          NULL, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "hsuBlockMemSizePtr = NULL");

        /*
            1.10. Call with wrong hsuBlockMemPtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, NULL, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "hsuBlockMemPtr = NULL");

        /*
            1.11. Call with wrong exportCompletePtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "exportCompletePtr = NULL");

        if (NULL != hsuBlockMemPtr)
        {
            cpssOsFree(hsuBlockMemPtr);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerHsuImport
(
    IN    GT_VOID   *tcamManagerHandlerPtr,
    INOUT GT_U32    *iteratorPtr,
    INOUT GT_U32    *hsuBlockMemSizePtr,
    IN    GT_U8     *hsuBlockMemPtr,
    OUT   GT_BOOL   *exportCompletePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerHsuImport)
{
/*
    1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
    Expected: GT_OK.
    1.2. Call cpssDxChTcamManagerHsuSizeGet to get HSU data size
    Expected: GT_OK.
    1.3. Call cpssDxChTcamManagerHsuExport to export HSU data
    Expected: GT_OK.
    1.4. Call with valid tcamManagerHandlerPtr,
                   iteratorPtr [0],
                   hsuBlockMemSizePtr[CPSS_HSU_SINGLE_ITERATION_CNS],
                   non NULL hsuBlockMemPtr,
                   non NULL exportCompletePtr.
    Expected: GT_OK.
    1.5. Call with valid tcamManagerHandlerPtr,
                   iteratorPtr [0],
                   hsuBlockMemSizePtr[1024],
                   non NULL hsuBlockMemPtr,
                   non NULL exportCompletePtr.
    Expected: GT_OK.
    1.6. Call with hsuBlockMemSize[CPSS_HSU_SINGLE_ITERATION_CNS]
                   iterator[100] (not relevant)
                   and other valid parameters
    Expected: GT_OK.
    1.7. Call with null tcamManagerHandlerPtr [NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.8. Call with wrong iteratorPtr[NULL].
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.9. Call with wrong hsuBlockMemSize[1023] (out of range)
                   and other valid parameters
    Expected: NOT GT_OK.
    1.10. Call with wrong hsuBlockMemSizePtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.11. Call with wrong hsuBlockMemPtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
    1.12. Call with wrong exportCompletePtr[NULL]
                   and other valid parameters
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_VOID   *tcamManagerHandlerPtr;
    GT_U8     dev = 0;
    GT_UINTPTR iterator = 0;
    GT_U32    hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;
    GT_U8     *hsuBlockMemPtr;
    GT_BOOL   exportComplete = GT_FALSE;
    GT_U32    size = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
            Expected: GT_OK.
        */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call cpssDxChTcamManagerHsuSizeGet to get HSU data size
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerHsuSizeGet(tcamManagerHandlerPtr, &size);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        hsuBlockMemPtr = cpssOsMalloc(size);
        st = NULL == hsuBlockMemPtr ? GT_NO_RESOURCE : GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "hsuBlockMemPtr = NULL");
        if (NULL == hsuBlockMemPtr)
        {
            return;
        }

        /*
            1.3. Call cpssDxChTcamManagerHsuExport to export HSU data
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerHsuExport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, exportComplete);

        /*
            1.4. Call with valid tcamManagerHandlerPtr,
                           iteratorPtr [0],
                           hsuBlockMemSizePtr[CPSS_HSU_SINGLE_ITERATION_CNS],
                           non NULL hsuBlockMemPtr,
                           non NULL exportCompletePtr.
            Expected: GT_OK.
        */
        iterator = 0;
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.5. Call with valid tcamManagerHandlerPtr,
                           iteratorPtr [0],
                           hsuBlockMemSizePtr[1024],
                           non NULL hsuBlockMemPtr,
                           non NULL exportCompletePtr.
            Expected: GT_OK.
        */
        iterator = 0;
        hsuBlockMemSize = 1024;

        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.6. Call with hsuBlockMemSize[CPSS_HSU_SINGLE_ITERATION_CNS]
                   iterator[100] (not relevant)
                   and other valid parameters
            Expected: GT_OK.
        */
        iterator = 100;
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore valid values */
        iterator = 0;

        /*
            1.7. Call with null tcamManagerHandlerPtr [NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuImport(NULL, &iterator, &hsuBlockMemSize,
                                          hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "tcamManagerHandlerPtr = NULL");

        /*
            1.8. Call with wrong iteratorPtr[NULL].
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, NULL,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "iteratorPtr = NULL");

        /*
            1.9. Call with hsuBlockMemSize[1023]
                           and other valid parameters
            Expected: GT_OK.
        */
        hsuBlockMemSize = 1023;

        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore valid values */
        hsuBlockMemSize = CPSS_HSU_SINGLE_ITERATION_CNS;

        /*
            1.10. Call with wrong hsuBlockMemSizePtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          NULL, hsuBlockMemPtr, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "hsuBlockMemSizePtr = NULL");

        /*
            1.11. Call with wrong hsuBlockMemPtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, NULL, &exportComplete);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "hsuBlockMemPtr = NULL");

        /*
            1.12. Call with wrong exportCompletePtr[NULL]
                           and other valid parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuImport(tcamManagerHandlerPtr, &iterator,
                                          &hsuBlockMemSize, hsuBlockMemPtr, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "exportCompletePtr = NULL");

        if (NULL != hsuBlockMemPtr)
        {
            cpssOsFree(hsuBlockMemPtr);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerHsuInactiveClientDelete
(
    IN  GT_VOID     *tcamManagerHandlerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerHsuInactiveClientDelete)
{
/*
    1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
            Expected: GT_OK.
    1.2. Call with not null tcamManagerHandlerPtr.
    Expected: GT_OK.
    1.3. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_VOID   *tcamManagerHandlerPtr;
    GT_U8     dev = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCreateDefaultTcamManager to create default TCAM manager
            Expected: GT_OK.
        */
        st = prvCreateDefaultTcamManager(&tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call with not null tcamManagerHandlerPtr.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerHsuInactiveClientDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.3. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerHsuInactiveClientDelete(NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "tcamManagerHandlerPtr = NULL");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamManagerTokenInfoGet
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      entryToken,
    OUT CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT       *entryTypePtr,
    OUT GT_VOID                                     **clientCookiePtrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamManagerTokenInfoGet)
{
/*
    ITERATE_DEVICES (xCat, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3)
    1. Call cpssDxChTcamManagerEntryAllocate for registered client
        with the same clientId and following params:
            entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            lowerBoundToken = 0;
            upperBoundToken = 0;
            allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
            defragEnable = GT_FALSE;
    Expected: GT_OK.
    2. Call cpssDxChTcamManagerTokenInfoGet for registered client
        with the same clientId and created entryToken.
    Expected: GT_OK.
    3. Call entryToken allocated to other client (wrong client entryToken).
    Expected: NOT GT_OK.
    4. Call with wrong tcamManagerHandlerPtr[NULL].
    Expected: GT_BAD_PTR.
    5. Call with wrong clientId (not existent).
    Expected: NOT GT_OK.
    6. Call with wrong clientId [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
    Expected: NOT GT_OK (out of range).
    7. Call with wrong entryToken (not existent).
    Expected: NOT GT_OK.
    8. Call with wrong entryTypePtr[NULL].
    Expected: GT_BAD_PTR.
    9. Call with wrong clientCookiePtrPtr[NULL].
    Expected: GT_BAD_PTR.
    10. Call cpssDxChTcamManagerDelete to restore configuration.
    Expected: GT_OK.
*/
    GT_STATUS                               st                      = GT_OK;
    GT_U8                                   dev                     = 0;
    GT_VOID                                 *tcamManagerHandlerPtr  = 0;
    GT_U32                                  clientId                = 0;
    GT_U32                                  entryToken              = 0;
    GT_VOID                                 *clientCookiePtr        = 0;

    GT_U32                                  clientId2               = 0;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    GT_U32                                  lowerBoundToken = 0;
    GT_U32                                  upperBoundToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_BOOL                                 defragEnable = GT_FALSE;

    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryTypeRet            = 0;
    GT_VOID                                 *clientCookiePtrRet     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* create default tcam manager handler with clients */
        st = prvCreateDefaultTcamManagerWithClients(&clientId, &tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1. Call cpssDxChTcamManagerEntryAllocate for registered client
            with the same clientId and following params:
                    entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
                    lowerBoundToken = 0;
                    upperBoundToken = 0;
                    allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
                    defragEnable = GT_FALSE;
            Expected: GT_OK.
        */
        entryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
        lowerBoundToken = 0;
        upperBoundToken = 0;
        allocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        defragEnable = GT_FALSE;

        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2. Call cpssDxChTcamManagerTokenInfoGet for registered client
            with the same clientId and created entryToken.
            Expected: GT_OK.
        */
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId, entryToken,
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            3. Call entryToken allocated to other client (wrong client entryToken).
            Expected: NOT GT_OK.
        */
        /*register clients*/
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client2", &clientFunc, &clientId2);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*allocate entry token*/
        st = cpssDxChTcamManagerEntryAllocate(tcamManagerHandlerPtr, clientId2,
                    entryType, lowerBoundToken, upperBoundToken, allocMethod,
                    defragEnable, clientCookiePtr, &entryToken);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*call with incorrect entryToken (for client2, but we call to client1)*/
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId, entryToken,
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            4. Call with wrong tcamManagerHandlerPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenInfoGet(NULL, clientId2, entryToken,
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, tcamManagerHandlerPtr = NULL");

        /*
            5. Call with wrong clientId2 (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr,
                                             ((clientId2 != 0) ? 0 : 1), entryToken,
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent clientId2");

        /*
            6. Call with wrong clientId2 [CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS].
            Expected: NOT GT_OK (out of range).
        */
        clientId2 = CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS;

        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId2, entryToken,
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, clientId2);

        clientId2 = 0;

        /*
            7. Call with wrong entryToken (not existent).
            Expected: NOT GT_OK.
        */
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId2,
                                             ((entryToken != 0) ? 0 : 1),
                                             &entryTypeRet, &clientCookiePtrRet);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "%d, not existent entryToken");

        /*
            8. Call with wrong entryTypePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId2, entryToken,
                                             NULL, &clientCookiePtrRet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, entryTypePtr = NULL");

        /*
            9. Call with wrong clientCookiePtrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTcamManagerTokenInfoGet(tcamManagerHandlerPtr, clientId2, entryToken,
                                             &entryTypeRet, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%d, clientCookiePtrPtr = NULL");

        /*
            10. Call cpssDxChTcamManagerDelete to restore configuration.
            Expected: GT_OK.
        */
        /*restore configuration*/
        st = cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTcamManagerDelete failed");
    }
}

/*----------------------------------------------------------------------------*/


/**
* @internal prvCreateDefaultTcamManager function
* @endinternal
*
* @brief   Creates dafault TCAM manager and returns handler for the created TCAM manager.
*
* @param[out] tcamManagerPtrPtr        - points to pointer of the created TCAM manager object
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCreateDefaultTcamManager
(
    OUT GT_VOID     **tcamManagerPtrPtr
)
{
    GT_STATUS  st = GT_OK;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType =
                            CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        range;
    GT_VOID                                *tcamManagerHandlerPtr;

    CPSS_NULL_PTR_CHECK_MAC(tcamManagerPtrPtr);

    range.firstLine = 0;
    range.lastLine  = 10;
    tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E;

    /*create tcam manager*/
    st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);

    *tcamManagerPtrPtr = (GT_OK == st) ? tcamManagerHandlerPtr : NULL;

    return st;
}

/**
* @internal prvCreateDefaultTcamManagerWithClients function
* @endinternal
*
* @brief   Creates dafault TCAM manager and register client.
*
* @param[out] clientIdPtr              - points to registered client Id
* @param[out] tcamManagerPtrPtr        - points to pointer of the created TCAM manager object
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCreateDefaultTcamManagerWithClients
(
    OUT GT_U32       *clientIdPtr,
    OUT GT_VOID     **tcamManagerPtrPtr
)
{
    GT_STATUS  st = GT_OK;
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType =
                            CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        range;
    GT_VOID                                *tcamManagerHandlerPtr;

    GT_U32                                  clientId = 0;
    CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  clientFunc;

    CPSS_NULL_PTR_CHECK_MAC(tcamManagerPtrPtr);

    range.firstLine = 0;
    range.lastLine  = 10;
    tcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E;

    /*create tcam manager*/
    st = cpssDxChTcamManagerCreate(tcamType, &range, &tcamManagerHandlerPtr);
    if(GT_OK == st)
    {
        *tcamManagerPtrPtr = tcamManagerHandlerPtr;

        /*register clients*/
        clientFunc.moveToLocationFuncPtr = prvClientMoveToLocation;
        clientFunc.moveToAnywhereFuncPtr = prvClientMoveToAnywhere;
        clientFunc.checkIfDefaultLocationFuncPtr = prvClientCheckIfDefaultLocation;

        st = cpssDxChTcamManagerClientRegister(tcamManagerHandlerPtr,
                                                "Client1", &clientFunc, &clientId);
        *clientIdPtr = clientId;
    }

    return st;
}

/*additional function to register clients*/
static GT_STATUS prvClientMoveToLocation
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *fromPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *toPtr,
    IN  GT_VOID                                     *clientCookiePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fromPtr);
    CPSS_NULL_PTR_CHECK_MAC(toPtr);

    clientCookiePtr = clientCookiePtr;

    return GT_OK;
}

/*additional function to register clients*/
static GT_STATUS prvClientMoveToAnywhere
(
    IN  GT_VOID         *clientCookiePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(clientCookiePtr);
    return GT_OK;
}

static GT_STATUS prvClientCheckIfDefaultLocation(
    IN GT_VOID  *clientCookiePtr,
    OUT GT_BOOL *isDefaultEntryLocationPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(clientCookiePtr);
    CPSS_NULL_PTR_CHECK_MAC(isDefaultEntryLocationPtr);

    isDefaultEntryLocationPtr=isDefaultEntryLocationPtr;

    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTcamManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTcamManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerClientRegister)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerClientUnregister)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerAvailableEntriesCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerEntriesReservationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerEntriesReservationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerEntryAllocate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerEntryClientCookieUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerEntryFree)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerRangeUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerTokenCompare)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerTokenToTcamLocation)
/*    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerUsageGet)  -- API not implemented */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerHsuSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerClientFuncUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerHsuExport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerHsuImport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerHsuInactiveClientDelete)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamManagerTokenInfoGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTcamManager)

