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
* @file cpssDxChPortStatUT.c
*
* @brief Unit tests for cpssDxChPortStat, that provides
* CPSS implementation for Port configuration and control facility.
*
* @version   44
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define PORT_STAT_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacCounterGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with ctrlName [CPSS_BRDC_PKTS_RCV_E /
                               CPSS_GOOD_OCTETS_RCV_E /
                               CPSS_OVERSIZE_PKTS_E /
                               CPSS_JABBER_PKTS_E /
                               CPSS_BAD_OCTETS_RCV_E /
                               CPSS_MAC_TRANSMIT_ERR_E]
                     and non-null cntrValuePtr.
    Expected: GT_OK.
    1.1.2. Call with ctrlName [CPSS_BAD_PKTS_RCV_E /
                               CPSS_UNRECOG_MAC_CNTR_RCV_E]
                     and non-null cntrValuePtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with ctrlName [CPSS_BadFC_RCV_E] (not supported by Cheetah/Cheetah2)
                     and non-null cntrValuePtr.
    Expected: NON GT_OK.
    1.1.4. Call with ctrlName [CPSS_DROP_EVENTS_E]
                     and non-null cntrValuePtr.
    Expected: GT_OK.
    1.1.5. Call with collision ctrlName [CPSS_COLLISIONS_E /
                                         CPSS_LATE_COLLISIONS_E /
                                         CPSS_EXCESSIVE_COLLISIONS_E] (applicable for Half-Duplex mode only, what is not possible for XG ports)
                     and non-null cntrValuePtr.
    Expected: GT_OK for non XG ports and GT_BAD_PARAM for XG ports.
    1.1.6. Call with out of range ctrlName [CPSS_LAST_MAC_COUNTER_NUM_E /
                                            wrong enum values]
                     and non-null cntrValuePtr.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with ctrlName [ CPSS_BAD_PKTS_RCV_E /
                                CPSS_UNRECOG_MAC_CNTR_RCV_E /
                                CPSS_BadFC_RCV_E /
                                CPSS_GOOD_PKTS_RCV_E /
                                CPSS_GOOD_PKTS_SENT_E] not supported.
                     and non-null cntrValuePtr.
    Expected: NOT GT_OK.
    1.1.8. Call with ctrlName [CPSS_GOOD_OCTETS_RCV_E]
                     and cntrValuePtr [NULL]
    Expected: GT_BAD_PTR.
    1.2. Call for CPU port [CPSS_CPU_PORT_NUM_CNS] with cntrName [CPSS_MAC_TRANSMIT_ERR_E/ CPSS_DROP_EVENTS_E / CPSS_BAD_OCTETS_RCV_E] and non NULL cntrValuePtr.
    Expected: GT_OK.
    1.3. Call for CPU port [CPSS_CPU_PORT_NUM_CNS] with non-supported cntrName [CPSS_BRDC_PKTS_RCV_E/ CPSS_OVERSIZE_PKTS_E/ CPSS_COLLISIONS_E/ CPSS_BadFC_RCV_E/ CPSS_LAST_MAC_COUNTER_NUM_E] and non NULL cntrValuePtr.
    Expected: NON GT_OK.
*/

    GT_STATUS                   st     = GT_OK;

    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port     = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PORT_MAC_COUNTERS_ENT  cntrName = CPSS_GOOD_OCTETS_RCV_E;
    GT_U64                      cntrVal;
    PRV_CPSS_PORT_TYPE_ENT      portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_BOOL                     expectFail;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with cntrName [CPSS_BRDC_PKTS_RCV_E /
                                          CPSS_GOOD_OCTETS_RCV_E /
                                          CPSS_OVERSIZE_PKTS_E /
                                          CPSS_JABBER_PKTS_E /
                                          CPSS_BAD_OCTETS_RCV_E /
                                          CPSS_MAC_TRANSMIT_ERR_E]
                                and non-null cntrValuePtr.
               Expected: GT_OK.
            */
            /* Call with ctrlName [CPSS_BRDC_PKTS_RCV_E] */
            cntrName = CPSS_BRDC_PKTS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_GOOD_OCTETS_RCV_E] */
            cntrName = CPSS_GOOD_OCTETS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_OVERSIZE_PKTS_E] */
            cntrName = CPSS_OVERSIZE_PKTS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_JABBER_PKTS_E] */
            cntrName = CPSS_JABBER_PKTS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_BAD_OCTETS_RCV_E] */
            cntrName = CPSS_BAD_OCTETS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_MAC_TRANSMIT_ERR_E] */
            cntrName = CPSS_MAC_TRANSMIT_ERR_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /*
                1.1.2. Call with ctrlName [CPSS_BAD_PKTS_RCV_E /
                                           CPSS_UNRECOG_MAC_CNTR_RCV_E]
                                 and non-null cntrValuePtr.
                Expected: GT_BAD_PARAM.
            */

            /* Call with ctrlName [CPSS_BAD_PKTS_RCV_E] */
            cntrName = CPSS_BAD_PKTS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);

            /* Call with ctrlName [CPSS_UNRECOG_MAC_CNTR_RCV_E] */
            cntrName = CPSS_UNRECOG_MAC_CNTR_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);

            /*
                1.1.3. Call with ctrlName [CPSS_BadFC_RCV_E] (not supported by Cheetah/Cheetah2)
                            and non-null cntrValuePtr.
                Expected: NON GT_OK.
            */
            cntrName = CPSS_BadFC_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            /*
                1.1.4. Call with ctrlName [CPSS_DROP_EVENTS_E]
                       and non-null cntrValuePtr.
                Expected: GT_OK
            */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            cntrName = CPSS_DROP_EVENTS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);


            /*
                1.1.5. Call with collision ctrlName [CPSS_COLLISIONS_E /
                                                     CPSS_LATE_COLLISIONS_E /
                                                     CPSS_EXCESSIVE_COLLISIONS_E]
                                 (applicable for Half-Duplex mode only, what is not possible for XG ports)
                                 and non-null cntrValuePtr.
                Expected: GT_OK for non XG ports and GT_NOT_SUPPORTED for XG ports.
            */

            /* Call with ctrlName [CPSS_COLLISIONS_E] */
            cntrName = CPSS_COLLISIONS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            if ((PRV_CPSS_PORT_XG_E <= portType) && (!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /* Call with ctrlName [CPSS_LATE_COLLISIONS_E] */
            cntrName = CPSS_LATE_COLLISIONS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            if ((PRV_CPSS_PORT_XG_E <= portType) && (!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /* Call with ctrlName [CPSS_EXCESSIVE_COLLISIONS_E] */
            cntrName = CPSS_EXCESSIVE_COLLISIONS_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            if (PRV_CPSS_PORT_XG_E <= portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /*
               1.1.6. Call with out of range ctrlName [CPSS_LAST_MAC_COUNTER_NUM_E /
                                                       wrong enum values]
                                and non-null cntrValuePtr.
               Expected: GT_BAD_PARAM.
            */

            /* Call with ctrlName [CPSS_LAST_MAC_COUNTER_NUM_E] */
            cntrName = CPSS_LAST_MAC_COUNTER_NUM_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, cntrName);

            /* Call with ctrlName [wrong enum values] */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacCounterGet
                                (dev, port, cntrName, &cntrVal),
                                cntrName);

            /*
                1.1.7. Call with ctrlName [CPSS_BAD_PKTS_RCV_E /
                                           CPSS_UNRECOG_MAC_CNTR_RCV_E /
                                           CPSS_BadFC_RCV_E /
                                           CPSS_GOOD_PKTS_RCV_E /
                                           CPSS_GOOD_PKTS_SENT_E]
                                 and non-null cntrValuePtr.
                Expected: NOT GT_OK.
            */
            cntrName = CPSS_BAD_PKTS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_UNRECOG_MAC_CNTR_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_BadFC_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_GOOD_PKTS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_GOOD_PKTS_SENT_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_BRDC_PKTS_RCV_E;

            /*
                1.1.8. Call with ctrlName [CPSS_GOOD_OCTETS_RCV_E]
                                 and cntrValuePtr [NULL]
                Expected: GT_BAD_PTR.
            */
            cntrName = CPSS_GOOD_OCTETS_RCV_E;

            st = cpssDxChMacCounterGet(dev, port, cntrName, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        cntrName = CPSS_GOOD_OCTETS_RCV_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Call for CPU port [CPSS_CPU_PORT_NUM_CNS] with
        cntrName [CPSS_MAC_TRANSMIT_ERR_E/
        CPSS_DROP_EVENTS_E / CPSS_BAD_OCTETS_RCV_E]
        and non NULL cntrValuePtr.
        Expected: GT_OK. */
        port = CPSS_CPU_PORT_NUM_CNS;
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_FALSE;
        }

        cntrName = CPSS_MAC_TRANSMIT_ERR_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_DROP_EVENTS_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_BAD_OCTETS_RCV_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port, cntrName);

        /* 1.5. Call for CPU port [CPSS_CPU_PORT_NUM_CNS] with non-supported
        cntrName [CPSS_BRDC_PKTS_RCV_E/ CPSS_OVERSIZE_PKTS_E/ CPSS_COLLISIONS_E/
        CPSS_BadFC_RCV_E/ CPSS_LAST_MAC_COUNTER_NUM_E] and non NULL cntrValuePtr.
        Expected: NON GT_OK. */
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
        }
        else
        {
            expectFail = GT_TRUE;
        }

        cntrName = CPSS_BRDC_PKTS_RCV_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_OVERSIZE_PKTS_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_COLLISIONS_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_BadFC_RCV_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port, cntrName);

        cntrName = CPSS_LAST_MAC_COUNTER_NUM_E;
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL3_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port, cntrName);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    cntrName = CPSS_GOOD_OCTETS_RCV_E;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, cntrName == CPSS_GOOD_OCTETS_RCV_E */

    st = cpssDxChMacCounterGet(dev, port, cntrName, &cntrVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS   cpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersOnPortGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with non-null portMacCounterSetArray.
    Expected: GT_OK.
    1.1.2. Call with portMacCounterSetArray [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS                     st   = GT_OK;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          port = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PORT_MAC_COUNTER_SET_STC counter;
    GT_BOOL                     expectFail;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null portMacCounterSetArray.
               Expected: GT_OK.
            */
            st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with portMacCounterSetArray [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacCountersOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, portMacCounterSetArray = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_FALSE;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCountersOnPortGet(dev, port, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCounterCaptureGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCounterCaptureGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with cntrName[CPSS_GOOD_OCTETS_RCV_E / CPSS_MC_PKTS_RCV_E],
                     non-null cntrValuePtr.
    Expected: GT_OK.
    1.1.2. Call with cntrValuePtr [NULL] and other params from 1.1.1.
    Expected: GT_BAD_PTR
    1.1.3. Call with cntrName[wrong enum values] and other params from 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with ctrlName [ CPSS_BAD_PKTS_RCV_E /
                                CPSS_UNRECOG_MAC_CNTR_RCV_E /
                                CPSS_BadFC_RCV_E /
                                CPSS_GOOD_PKTS_RCV_E /
                                CPSS_GOOD_PKTS_SENT_E] not supported.
                     and non-null cntrValuePtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    CPSS_PORT_MAC_COUNTERS_ENT  cntrName = CPSS_GOOD_PKTS_RCV_E;
    GT_U64                      cntrValue;
    GT_BOOL                     expectFail;
    GT_BOOL                     isRemotePort;

    cpssOsBzero((GT_VOID*) &cntrValue, sizeof(cntrValue));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

            /*
                1.1.1. Call with cntrName[CPSS_GOOD_OCTETS_RCV_E / CPSS_MC_PKTS_RCV_E],
                                 non-null cntrValuePtr.
                Expected: GT_OK.
            */
            /* iterate with cntrName = CPSS_GOOD_OCTETS_RCV_E */
            cntrName = CPSS_GOOD_OCTETS_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                /* Don't verify return code if operation is not supported on a remote port */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /* iterate with cntrName = CPSS_MC_PKTS_RCV_E */
            cntrName = CPSS_MC_PKTS_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                /* Don't verify return code if operation is not supported on a remote port */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
            }

            /*
                1.1.2. Call with cntrValuePtr [NULL] and other params from 1.1.1.
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, cntrValuePtr = NULL", dev, port);

            /*
                1.1.3. Call with cntrName[wrong enum values] and other params from 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortMacCounterCaptureGet
                                (dev, port, cntrName, &cntrValue),
                                cntrName);

            /*
                1.1.4. Call with ctrlName [CPSS_BAD_PKTS_RCV_E /
                                           CPSS_UNRECOG_MAC_CNTR_RCV_E /
                                           CPSS_BadFC_RCV_E /
                                           CPSS_GOOD_PKTS_RCV_E /
                                           CPSS_GOOD_PKTS_SENT_E]
                                 and non-null cntrValuePtr.
                Expected: NOT GT_OK.
            */
            cntrName = CPSS_BAD_PKTS_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_UNRECOG_MAC_CNTR_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_BadFC_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_GOOD_PKTS_RCV_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);

            cntrName = CPSS_GOOD_PKTS_SENT_E;

            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cntrName);
        }

        cntrName = CPSS_GOOD_OCTETS_RCV_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }

        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    cntrName = CPSS_GOOD_PKTS_RCV_E;
    port     = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCounterCaptureGet(dev, port, cntrName, &cntrValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersCaptureOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersCaptureOnPortGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null portMacCounterSetArray.
    Expected: GT_OK.
    1.1.2. Call with portMacCounterSetArray [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = PORT_STAT_VALID_PHY_PORT_CNS;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
    GT_BOOL                     expectFail;
    GT_BOOL                     isRemotePort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

            /*
                1.1.1. Call with non-null portMacCounterSetArray.
                Expected: GT_OK.
            */
            st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                /* Don't verify return code if operation is not supported on a remote port */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with portMacCounterSetArray [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, portMacCounterSetArray = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }

        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCountersCaptureOnPortGet(dev, port, &portMacCounterSetArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersCaptureTriggerSet
(
    IN  GT_U8  devNum,
    IN  GT_U8  portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersCaptureTriggerSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call cpssDxChPortMacCountersCaptureTriggerGet with non NULL captureIsDonePtr.
    Expected: GT_OK and captureIsDone = GT_TRUE.
*/
    GT_STATUS   st   = GT_OK;
    GT_STATUS   expectedSt   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = PORT_STAT_VALID_PHY_PORT_CNS;

    GT_BOOL     captureIsDone = GT_FALSE;
    GT_BOOL                     expectFail;
    GT_BOOL                     isRemotePort;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

            st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortMacCountersCaptureTriggerGet: %d, %d",
                                         dev, port);

            if (GT_TRUE == captureIsDone)
            {
                if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(dev, port))
                {
                    expectedSt = GT_NOT_APPLICABLE_DEVICE;
                }
                else
                {
                    expectedSt = GT_OK;
                }
                st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
                if ((isRemotePort == GT_FALSE) ||
                    (st != GT_NOT_SUPPORTED))
                {
                    /* Don't verify return code if operation is not supported on a remote port */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }

        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCountersCaptureTriggerSet(dev, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersCaptureTriggerGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_BOOL  *captureIsDonePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersCaptureTriggerGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not-NULL captureIsDonePtr.
    Expected: GT_OK.
    1.1.2. Call with captureIsDonePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = PORT_STAT_VALID_PHY_PORT_CNS;

    GT_BOOL     captureIsDone = GT_FALSE;
    GT_BOOL                     expectFail;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not-NULL captureIsDonePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with captureIsDonePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, captureIsDonePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }

        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCountersCaptureTriggerGet(dev, port, &captureIsDone);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_U8                           portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEgressCntrModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with cntrSetNum[0/ 0/ 1/ 1],
                     setModeBmp [CPSS_EGRESS_CNT_PORT_E /
                                 CPSS_EGRESS_CNT_VLAN_E /
                                 CPSS_EGRESS_CNT_TC_E /
                                 CPSS_EGRESS_CNT_DP_E],
                     vlanId[5 / 127 / 1024 / 4000],
                     tc[1, 2, 5, 7],
                     and dpLevel [245 /
                                  CPSS_DP_GREEN_E /
                                  CPSS_DP_RED_E ]/
    Expected: GT_OK
    1.1.2. Call cpssDxChPortEgressCntrModeGet with no NULL pointers and cntrSetNum as in 1.1.
    Expected: GT_OK and the same params as was set
    1.1.3. Call with cntrSetNum [1]
                     and wrong enum values setModeBmp ,
                     vlanId [7],
                     tc [1]
                     and dpLevel [CPSS_DP_RED_E].
    Expected: NOT GT_OK.
    1.1.4. Call with cntrSetNum [0],
                     setModeBmp [CPSS_EGRESS_CNT_VLAN_E]
                     and out of range  vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096],
                     tc [0]
                     and dpLevel [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.1.5. Call with cntrSetNum [1],
                     setModeBmp [CPSS_EGRESS_CNT_TC_E],
                     vlanId [1025]
                     out of range tc [CPSS_TC_RANGE_CNS=8]
                     and dpLevel [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.1.6. Call with cntrSetNum [0],
                     setModeBmp CPSS_EGRESS_CNT_DP_E],
                     vlanId [2085],
                     tc [5]
                     and wrong enum values dpLevel.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with cntrSetNum [0],
                     setModeBmp [CPSS_EGRESS_CNT_DP_E],
                     vlanId [2085],
                     tc [5]
                     and out of range dpLevel [CPSS_DP_YELLOW_E] (check yellow dpLevel).
    Expected: NOT GT_OK.
    1.1.8. Call with cntrSetNum[200] (check big value for counter set),
                     setModeBmp [CPSS_EGRESS_CNT_DP_E],
                     vlanId[100],
                     tc[1],
                     and dpLevel [CPSS_DP_GREEN_E]
    Expected: NOT GT_OK
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st       = GT_OK;

    GT_U8                           dev;
    GT_U8                           cntrSetNum = 0;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp = CPSS_EGRESS_CNT_PORT_E;
    GT_PHYSICAL_PORT_NUM            port       = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_U16                          vlanId     = 0;
    GT_U8                           tc         = 0;
    CPSS_DP_LEVEL_ENT               dpLevel    = CPSS_DP_GREEN_E;

    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmpGet = CPSS_EGRESS_CNT_PORT_E;
    GT_PHYSICAL_PORT_NUM            portGet       = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_U16                          vlanIdGet     = 0;
    GT_U8                           tcGet         = 0;
    CPSS_DP_LEVEL_ENT               dpLevelGet    = CPSS_DP_GREEN_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(   UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) ?
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E :
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with cntrSetNum[0/ 0/ 1/ 1],
                                setModeBmp [CPSS_EGRESS_CNT_PORT_E /
                                            CPSS_EGRESS_CNT_VLAN_E /
                                            CPSS_EGRESS_CNT_TC_E /
                                            CPSS_EGRESS_CNT_DP_E],
                                 vlanId[5/ 127/ 1024/ 5000],
                                 tc[10, 2, 5, 7],
                                 and dpLevel [245 /
                                              CPSS_DP_GREEN_E /
                                              CPSS_DP_YELLOW_E /
                                              CPSS_DP_RED_E ]
               Expected: GT_OK
            */

            /* Call with cntrSetNum[0], setModeBmp [CPSS_EGRESS_CNT_PORT_E], */
            /* vlanId[5], tc[1], dpLevel [CPSS_DP_GREEN_E]                  */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_PORT_E;
            vlanId = 5;
            tc = 1;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);

            /*
                1.1.2. Call cpssDxChPortEgressCntrModeGet with no NULL pointers and cntrSetNum as in 1.1.
                Expected: GT_OK and the same params as was set
            */
            st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmpGet, &portGet, &vlanIdGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet, "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet, "got another port than was set: %d", dev);

            /* Call with cntrSetNum[0], setModeBmp [CPSS_EGRESS_CNT_VLAN_E], */
            /* vlanId[127], tc[2], dpLevel [CPSS_DP_YELLOW_E]                  */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_VLAN_E;
            vlanId = 127;
            tc = 2;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);

            /*
                1.1.2. Call cpssDxChPortEgressCntrModeGet with no NULL pointers and cntrSetNum as in 1.1.
                Expected: GT_OK and the same params as was set
            */
            st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmpGet, &portGet, &vlanIdGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet, "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet, "got another vlanId than was set: %d", dev);

            /* Call with cntrSetNum[1], setModeBmp [CPSS_EGRESS_CNT_TC_E], */
            /* vlanId[1024], tc[5], dpLevel [CPSS_DP_RED_E]                  */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_TC_E;
            vlanId = 1024;
            tc = 5;
            dpLevel = CPSS_DP_RED_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);

            /*
                1.1.2. Call cpssDxChPortEgressCntrModeGet with no NULL pointers and cntrSetNum as in 1.1.
                Expected: GT_OK and the same params as was set
            */
            st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmpGet, &portGet, &vlanIdGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet, "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tc, tcGet, "got another tc than was set: %d", dev);

            /* Call with cntrSetNum[1], setModeBmp [CPSS_EGRESS_CNT_DP_E], */
            /* vlanId[4000], tc[7], dpLevel [CPSS_DP_GREEN_E]                  */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            vlanId = 4000;
            tc = 7;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);

            /*
                1.1.2. Call cpssDxChPortEgressCntrModeGet with no NULL pointers and cntrSetNum as in 1.1.
                Expected: GT_OK and the same params as was set
            */
            st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmpGet, &portGet, &vlanIdGet, &tcGet, &dpLevelGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(setModeBmp, setModeBmpGet, "got another setModeBmp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet, "got another dpLevel than was set: %d", dev);

            /*
                1.1.3. Call with cntrSetNum [1]
                                 wrong enum values setModeBmp ,
                                 vlanId [7],
                                 tc [1]
                                 and dpLevel [CPSS_DP_RED_E].
                Expected: NOT GT_OK.
            */
            cntrSetNum = 1;
            vlanId = 7;
            tc = 1;
            dpLevel = CPSS_DP_RED_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChPortEgressCntrModeSet
                                (dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel),
                                setModeBmp);

            /*
                1.1.4. Call with cntrSetNum [0],
                                 setModeBmp [CPSS_EGRESS_CNT_VLAN_E]
                                 out of range  vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS= 4096],
                                 tc [0]
                                 and dpLevel [CPSS_DP_GREEN_E].
                Expected: GT_BAD_PARAM.
            */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_VLAN_E;
            vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
            tc = 0;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, vlanId);

            /*
                1.1.5. Call with cntrSetNum [1],
                                 setModeBmp [CPSS_EGRESS_CNT_TC_E],
                                 vlanId [1025]
                                 out of range tc [CPSS_TC_RANGE_CNS=8]
                                 and dpLevel [CPSS_DP_GREEN_E].
                Expected: GT_BAD_PARAM.
            */
            cntrSetNum = 1;
            setModeBmp = CPSS_EGRESS_CNT_TC_E;
            vlanId = 1025;
            tc = CPSS_TC_RANGE_CNS;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tc);

            /*
                1.1.6. Call with cntrSetNum [0],
                                 setModeBmp CPSS_EGRESS_CNT_DP_E],
                                 vlanId [2085],
                                 tc [5]
                                 and wrong enum values dpLevel.
                Expected: GT_BAD_PARAM.
            */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            vlanId = 2085;
            tc = 5;

            UTF_ENUMS_CHECK_MAC(cpssDxChPortEgressCntrModeSet
                                (dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel),
                                dpLevel);

            /*
                1.1.7. Call with cntrSetNum [0],
                                 setModeBmp [CPSS_EGRESS_CNT_DP_E],
                                 vlanId [2085],
                                 tc [5]
                                 out of range dpLevel [CPSS_DP_YELLOW_E] (check yellow dpLevel).
                Expected: NOT GT_OK for non DxCh3.
            */
            cntrSetNum = 0;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            vlanId = 2085;
            tc = 5;
            dpLevel = CPSS_DP_YELLOW_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);

            if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, dpLevel);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, dpLevel);
            }

            if (st == GT_OK)
            {
                st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmpGet, &portGet, &vlanIdGet, &tcGet, &dpLevelGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortEgressCntrModeGet: %d, %d", dev, cntrSetNum);

                UTF_VERIFY_EQUAL1_STRING_MAC(dpLevel, dpLevelGet, "got another dpLevel than was set: %d", dev);
            }

            dpLevel = CPSS_DP_GREEN_E;

            /*
                1.1.8. Call with cntrSetNum[2] (check big value for counter set),
                                 setModeBmp [CPSS_EGRESS_CNT_DP_E],
                                 vlanId[100],
                                 tc[1],
                                 and dpLevel [CPSS_DP_GREEN_E]
                Expected: NOT GT_OK
            */
            cntrSetNum = 2;
            setModeBmp = CPSS_EGRESS_CNT_DP_E;
            vlanId = 100;
            tc = 1;
            dpLevel = CPSS_DP_GREEN_E;

            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, dpLevel);
        }

        cntrSetNum = 0;
        setModeBmp = CPSS_EGRESS_CNT_PORT_E;
        vlanId     = 5;
        tc         = 10;
        dpLevel    = CPSS_DP_GREEN_E;

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);
        st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    cntrSetNum = 0;
    setModeBmp = CPSS_EGRESS_CNT_PORT_E;
    vlanId     = 5;
    tc         = 10;
    dpLevel    = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEgressCntrModeSet(dev, cntrSetNum, setModeBmp, port, vlanId, tc, dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_U8                           *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEgressCntrModeGet)
{
/*
    1.1. Call with cntrSetNum[0 / 1]
                   and not NULL pointers setModeBmpPtr,
                                         portNumPtr,
                                         vlanIdPtr,
                                         tcPtr,
                                         dpLevelPtr.
    Expected: GT_OK.
    1.2. Call with cntrSetNum [2] and other params as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call with setModeBmpPtr [NULL] and other params as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with portNumPtr [NULL] and other params as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with vlanIdPtr [NULL] and other params as in 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with tcPtr [NULL] and other params as in 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with dpLevelPtr [NULL] and other params as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                           cntrSetNum = 0;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp = CPSS_EGRESS_CNT_PORT_E;
    GT_PHYSICAL_PORT_NUM            port       = PORT_STAT_VALID_PHY_PORT_CNS;
    GT_U16                          vlanId     = 0;
    GT_U8                           tc         = 0;
    CPSS_DP_LEVEL_ENT               dpLevel    = CPSS_DP_GREEN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with cntrSetNum[0 / 1]
                      and not NULL pointers setModeBmpPtr,
                                            portNumPtr,
                                            vlanIdPtr,
                                            tcPtr,
                                            dpLevelPtr.
            Expected: GT_OK.
        */
        /* iterate with cntrSetNum = 0 */
        cntrSetNum = 0;

        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        /* iterate with cntrSetNum = 1 */
        cntrSetNum = 1;

        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        /*
            1.2. Call with cntrSetNum [2] and other params as in 1.1.
            Expected: NOT GT_OK.
        */
        cntrSetNum = 2;

        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetNum);

        cntrSetNum = 0;

        /*
            1.3. Call with setModeBmpPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, NULL, &port, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, setModeBmpPtr = NULL", dev);

        /*
            1.4. Call with portNumPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, NULL, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portPtr = NULL", dev);

        /*
            1.5. Call with vlanIdPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, NULL, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanIdPtr = NULL", dev);

        /*
            1.6. Call with tcPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, NULL, &dpLevel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcPtr = NULL", dev);

        /*
            1.7. Call with dpLevelPtr [NULL] and other params as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dpLevelPtr = NULL", dev);
    }

    cntrSetNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, &dpLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEgressCntrModeGet(dev, cntrSetNum, &setModeBmp, &port, &vlanId, &tc, &dpLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortEgressCntrsGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortEgressCntrsGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with cntrSetNum [1] and non-null egrCntrPtr.
    Expected: GT_OK.
    1.2. Call with with cntrSetNum [0] and egrCntrPtr [NULL].
    Expected: GT_BAD_PTR
    1.3. Call with cntrSetNum [254] and non-null egrCntrPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS                   st     = GT_OK;

    GT_U8                       dev;
    GT_U8                       setNum = 0;
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with cntrSetNum [1]
                     and non-null egrCntrPtr.
           Expected: GT_OK.
        */
        setNum = 1;

        st = cpssDxChPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, setNum);

        /*
            1.2. Call with cntrSetNum [0]
                           and egrCntrPtr [NULL].
            Expected: GT_BAD_PTR
        */
        setNum = 0;

        st = cpssDxChPortEgressCntrsGet(dev, setNum, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, setNum);

        /*
            1.3. Call with cntrSetNum [254]
                           and non-null egrCntrPtr.
            Expected: NOT GT_OK.
        */
        setNum = 254;

        st = cpssDxChPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, setNum);
    }

    setNum = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortEgressCntrsGet(dev, setNum, &egrCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortEgressCntrsGet(dev, setNum, &egrCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersEnable(
    IN GT_U8 devNum,
    IN GT_U8 portNum,
    IN GT_BOOL enable
)
*/

UTF_TEST_CASE_MAC(cpssDxChPortMacCountersEnable)
{
/*
ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
1.1. Call with enable [GT_FALSE/ GT_TRUE].
Expected: GT_OK.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL     enable;
    GT_BOOL     enableGet;
    GT_BOOL     isRemotePort;
    PRV_CPSS_PORT_TYPE_ENT      portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            if (portType == PRV_CPSS_PORT_CG_E)
            {
                continue;
            }
            /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
            /* Expected: GT_OK.                                             */
            enable = GT_FALSE;

            st = cpssDxChPortMacCountersEnable(dev, port, enable);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                /* Don't verify return code if disable operation is not supported on a remote port */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                /* If disable operation is not supported on a remote port, expect to get TRUE result */
                enable = GT_TRUE;
            }


            st = cpssDxChPortMacCountersEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* the GM not supports this register and returns 0xBADAD on read */
            if (GT_FALSE == prvUtfIsGmCompilation()) {
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                "got another enable than was set: %d, %d", dev, port);
            }

            enable = GT_TRUE;

            st = cpssDxChPortMacCountersEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChPortMacCountersEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersEnable(dev, port, enable);
        if(prvUtfIsCpuPortMacSupported(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);

        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
        }
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0            */
    /* enable == GT_TRUE    */

    st = cpssDxChPortMacCountersEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersClearOnReadSet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersClearOnReadSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2 Call cpssDxChPortMacCountersClearOnReadGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL                     expectFail;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1. Call function for with enable = GT_FALSE and GT_TRUE.
               Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2 Call cpssDxChPortMacCountersClearOnReadGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d", dev);

            /*
               1.1. Call function for with enable = GT_FALSE and GT_TRUE.
               Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2 Call cpssDxChPortMacCountersClearOnReadGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d", dev);
        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_FALSE;
        }
        UTF_VERIFY_EQUAL3_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port , enable);
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortMacCountersClearOnReadSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersClearOnReadGet(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersClearOnReadGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1. Call with  not null enablePtr.
    Expected: GT_OK.
    1.2 Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL                     expectFail;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call with  not null enablePtr.
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.2 Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacCountersClearOnReadGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_FALSE;
        }
        UTF_VERIFY_EQUAL3_PARAM_MAC((st == GT_OK), (expectFail == GT_FALSE), dev, port , enable);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacCountersClearOnReadGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersRxHistogramEnable(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersRxHistogramEnable)
{
/*
ITERATE_DEVICES_PHY_PORTS (DxChx)
1.1. Call with enable [GT_FALSE/ GT_TRUE].
Expected: GT_OK.
*/

    GT_STATUS            st = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL              enable;
    GT_BOOL              enableGet;
    GT_BOOL              expectFail;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
            /* Expected: GT_OK.                                             */
            enable = GT_FALSE;

            st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);

            enable = GT_TRUE;

            st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);

        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM  */
        /* for CPU port number.                                             */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
        if (st == GT_OK && expectFail ==  GT_FALSE)
        {
            st = cpssDxChPortMacCountersRxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);
        }
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0            */
    /* enable == GT_TRUE    */

    st = cpssDxChPortMacCountersRxHistogramEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersTxHistogramEnable(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersTxHistogramEnable)
{
/*
ITERATE_DEVICES_PHY_PORTS (DxChx)
1.1. Call with enable [GT_FALSE/ GT_TRUE].
Expected: GT_OK.
*/

    GT_STATUS            st = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL              enable;
    GT_BOOL              enableGet;
    GT_BOOL              expectFail;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function for with enable = GT_FALSE and GT_TRUE.   */
            /* Expected: GT_OK.                                             */
            enable = GT_FALSE;

            st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);

            enable = GT_TRUE;

            st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssDxChPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);

        }

        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM  */
        /* for CPU port number.                                             */
        /* enable == GT_TRUE */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);

        if (st == GT_OK &&  expectFail == GT_FALSE)
        {
            st = cpssDxChPortMacCountersTxHistogramEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                            "got another enable than was set: %d, %d", dev, port);
        }
    }

    enable = GT_TRUE;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0            */
    /* enable == GT_TRUE    */

    st = cpssDxChPortMacCountersTxHistogramEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortStatInit(
    IN GT_U8    devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortStatInit)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1. Call with not null dev.
    Expected: GT_OK.
*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null dev.
            Expected: GT_OK.
        */
        st = cpssDxChPortStatInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortStatInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortStatInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacOversizedPacketsCounterModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2) or ITERATE_DEVICES_PHY_PORTS(xCat3)
    1.1.1 Call with  not null counterModePtr.
    Expected: GT_OK.
    1.1.2 Call with wrong counterModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode =
                      CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
    GT_BOOL                     expectFail;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1 Call with  not null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2 Call with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacOversizedPacketsCounterModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat2) or ITERATE_DEVICES_PHY_PORTS(xCat3)
    1.1.1. Call with counterMode [CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E
           / CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E
           / CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortMacOversizedPacketsCounterModeGet.
    Expected: GT_OK and the same counterMode value as was set.
    1.1.3. Call with wrong enum values counterMode and other parameters form 1.1.1
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st     = GT_OK;
    GT_STATUS   expectedSt = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_STAT_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode =
            CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterModeGet =
            CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
    GT_BOOL                     expectFail;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with counterMode [CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E
                       / CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E
                       / CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E].
                Expected: GT_OK.
            */
            /* Call function with counterMode [CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E] */
            counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
            if (PRV_CPSS_DXCH_XCAT2_STACK_MAC_COUNT_NO_CLEAR_ON_READ_WA_MAC(dev, port))
            {
                expectedSt = GT_NOT_APPLICABLE_DEVICE;
            }
            else
            {
                expectedSt = GT_OK;
            }

            st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortMacOversizedPacketsCounterModeGet.
                Expected: GT_OK and the same enable value as was set.
            */
#ifndef GM_USED /* don't test in GM */
            st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortMacOversizedPacketsCounterModeGet: %d", dev);
            if (expectedSt == GT_OK)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                        "get another enable than was set: %d, %d", dev, port);
            }
#endif

            /* Call function with counterMode [CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E] */
            counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
            st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, port);

            /*
                1.1.2. Call cpssDxChPortMacOversizedPacketsCounterModeGet.
                Expected: GT_OK and the same enable value as was set.
            */
#ifndef GM_USED /* don't test in GM */
            st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChPortMacOversizedPacketsCounterModeGet: %d", dev);
            if (expectedSt == GT_OK)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                        "get another enable than was set: %d, %d", dev, port);
            }
#endif

            if(PRV_CPSS_PP_MAC(dev)->appDevFamily == CPSS_XCAT2_E)
            {

                /* Call function with counterMode [CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E] */
                counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
                st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, port);

                /*
                    1.1.2. Call cpssDxChPortMacOversizedPacketsCounterModeGet.
                    Expected: GT_OK and the same enable value as was set.
                */
                st = cpssDxChPortMacOversizedPacketsCounterModeGet(dev, port, &counterModeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChPortMacOversizedPacketsCounterModeGet: %d", dev);
                if (expectedSt == GT_OK)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(counterMode, counterModeGet,
                            "get another enable than was set: %d, %d", dev, port);
                }
            }

            /*
                1.1.3. Call with wrong enum values counterMode and other parameters form 1.1.1
                Expected: GT_BAD_PARAM.
            */
            if (expectedSt == GT_OK)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChPortMacOversizedPacketsCounterModeSet
                                    (dev, port, counterMode),
                                    counterMode);
            }

        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no special MAC for the CPU ...*/
            /* but check if there is valid port by this number */
            port = CPSS_CPU_PORT_NUM_CNS - 1;
            prvUtfNextMacPortGet(&port, GT_TRUE);
            if(port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* the port hold MAC ... and was bound to it */
                expectFail = GT_FALSE;
            }
            else
            {
                /* the port hold NO MAC ... (or was bound to it) */
                expectFail = GT_TRUE;
            }
            port = CPSS_CPU_PORT_NUM_CNS;/*restore value*/
        }
        else
        {
            expectFail = GT_TRUE;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC((st != GT_OK), (expectFail != GT_FALSE), dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    counterMode = CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
    port = PORT_STAT_VALID_PHY_PORT_CNS;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortMacOversizedPacketsCounterModeSet(dev, port, counterMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortStatTxDebugCountersGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortStatTxDebugCountersGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null dropCntrStcPtr.
    Expected: GT_OK.
    1.2. Call with egrCntrPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS                   st     = GT_OK;

    GT_U8                       dev;
    CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC   dropCntrStc;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null dropCntrStc.
           Expected: GT_OK.
        */
        st = cpssDxChPortStatTxDebugCountersGet(dev, &dropCntrStc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with dropCntrStc [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPortStatTxDebugCountersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortStatTxDebugCountersGet(dev, &dropCntrStc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortStatTxDebugCountersGet(dev, &dropCntrStc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacCounterOnPhySideGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacCounterOnPhySideGet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    CPSS_PORT_MAC_COUNTERS_ENT          cntrName;
    GT_U64                              cntrValueGet;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            for (cntrName = (CPSS_PORT_MAC_COUNTERS_ENT) 0; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E; cntrName++)
            {
                st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
                switch (cntrName)
                {
                    case CPSS_GOOD_PKTS_RCV_E:
                    case CPSS_BAD_PKTS_RCV_E:
                    case CPSS_GOOD_PKTS_SENT_E:
                    case CPSS_UNRECOG_MAC_CNTR_RCV_E:
                    case CPSS_BadFC_RCV_E:
                    case CPSS_PKTS_1024TO1518_OCTETS_E:
                    case CPSS_PKTS_1519TOMAX_OCTETS_E:

                        /* Unsupported counter names */
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, cntrName);
                        break;

                    default:
                        /* Supported counter names */
                        UTF_VERIFY_EQUAL3_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum, cntrName);
                        break;
                }
            }
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        cntrName = CPSS_GOOD_OCTETS_RCV_E;

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    cntrName = CPSS_GOOD_OCTETS_RCV_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChMacCounterOnPhySideGet(dev, portNum, cntrName, &cntrValueGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortMacCountersOnPhySidePortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortMacCountersOnPhySidePortGet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    CPSS_PORT_MAC_COUNTER_SET_STC       portMacCounterSetArrayGet;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortMacCountersOnPhySidePortGet(dev, portNum, &portMacCounterSetArrayGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortStat suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortStat)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCounterCaptureGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersCaptureOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersCaptureTriggerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersCaptureTriggerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEgressCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEgressCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortEgressCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersClearOnReadSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersClearOnReadGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersRxHistogramEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersTxHistogramEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortStatInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacOversizedPacketsCounterModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacOversizedPacketsCounterModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortStatTxDebugCountersGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacCounterOnPhySideGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortMacCountersOnPhySidePortGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChPortStat)

