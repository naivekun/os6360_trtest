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
* @file cpssPxCncUT.c
*
* @brief Unit tests for cpssPxCnc that provides CPSS implementation for CNC.
*
* @version   1
********************************************************************************
*/
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <gtOs/gtOsMem.h>

UTF_TEST_CASE_MAC(cpssPxCncBlockClientEnableSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        1.3. Call with unexpected client
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     updateEnableGet;
    GT_U32      blockIt;
    CPSS_PX_CNC_CLIENT_ENT  clientIt;
    GT_BOOL     boolIt;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            for(clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;
                clientIt <= CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;
                clientIt++)
            {
                for(boolIt = GT_FALSE; boolIt <= GT_TRUE; boolIt++)
                {
                    st = cpssPxCncBlockClientEnableSet(dev, blockIt,
                        clientIt, boolIt);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, blockIt,
                        clientIt, boolIt);

                    st = cpssPxCncBlockClientEnableGet(dev, blockIt,
                    clientIt, &updateEnableGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, blockIt,
                        clientIt);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(boolIt, updateEnableGet);
                }
            }
        }

        blockIt = 0;
        clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;
        boolIt = GT_FALSE;

        /*  1.2. Call with unexpected blockNum
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientEnableSet(dev, 3, clientIt, boolIt);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, 3, clientIt, boolIt);

        /*  1.3. Call with unexpected client
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientEnableSet(dev, blockIt, 4, boolIt);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, 4, boolIt);

    }
    blockIt = 0;
    clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;
    boolIt = GT_FALSE;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockClientEnableSet(dev, blockIt, clientIt, boolIt);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockIt, clientIt, boolIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockClientEnableSet(dev, blockIt, clientIt, boolIt);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, clientIt,
        boolIt);
}

UTF_TEST_CASE_MAC(cpssPxCncBlockClientEnableGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        1.3. Call with unexpected client
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     updateEnableGet;
    GT_U32      blockIt;
    CPSS_PX_CNC_CLIENT_ENT  clientIt;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            for(clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;
                clientIt <= CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;
                clientIt++)
            {
                    st = cpssPxCncBlockClientEnableGet(dev, blockIt,
                        clientIt, &updateEnableGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, blockIt,
                        clientIt);
            }
        }
        blockIt = 0;
        clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;

        /*  1.2. Call with unexpected blockNum
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientEnableGet(dev, 3, clientIt,  &updateEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 3, clientIt);

        /*  1.3. Call with unexpected client
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientEnableGet(dev, blockIt, 4,  &updateEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, 4);

    }
    blockIt = 0;
    clientIt = CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockClientEnableGet(dev, blockIt, clientIt,
            &updateEnableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockIt, clientIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockClientEnableGet(dev, blockIt, clientIt,
        &updateEnableGet);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, clientIt);
}

UTF_TEST_CASE_MAC(cpssPxCncBlockClientRangesSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    GT_U32      bmpIt;
    GT_U64      portRangeBmp[3];
    GT_U64      portRangeBmpGet;

    portRangeBmp[0].l[0] = 0xFFFFFFFF;
    portRangeBmp[0].l[1] = 0xFFFFFFFF;
    portRangeBmp[1].l[0] = 0x00000000;
    portRangeBmp[1].l[1] = 0x80000000;
    portRangeBmp[2].l[0] = 0x00000001;
    portRangeBmp[2].l[1] = 0x00000000;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(bmpIt = 0; bmpIt < 3; bmpIt++)
        {
            for(blockIt = 0; blockIt < 2; blockIt++)
            {
                st = cpssPxCncBlockClientRangesSet(dev, blockIt,
                    portRangeBmp[bmpIt]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
            }

            for(blockIt = 0; blockIt < 2; blockIt++)
            {
                st = cpssPxCncBlockClientRangesGet(dev, blockIt,
                    &portRangeBmpGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
                UTF_VERIFY_EQUAL0_PARAM_MAC(portRangeBmp[bmpIt].l[0],
                    portRangeBmpGet.l[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(portRangeBmp[bmpIt].l[1],
                    portRangeBmpGet.l[1]);
            }
        }
        blockIt = 0;

        /* 1.2. Call with unexpected blockNum
           Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientRangesSet(dev, blockIt, portRangeBmp[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
    }
    blockIt = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockClientRangesSet(dev, blockIt,portRangeBmp[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockClientRangesSet(dev, blockIt, portRangeBmp[0]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt);
}

UTF_TEST_CASE_MAC(cpssPxCncBlockClientRangesGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    GT_U32      bmpIt;
    GT_U64      portRangeBmpGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(bmpIt = 0; bmpIt < 3; bmpIt++)
        {
            for(blockIt = 0; blockIt < 2; blockIt++)
            {
                st = cpssPxCncBlockClientRangesGet(dev, blockIt,
                    &portRangeBmpGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
            }
        }
        blockIt = 0;

        /* 1.2. Call with unexpected blockNum
           Expected: GT_BAD_PARAM. */
        st = cpssPxCncBlockClientRangesGet(dev, blockIt, &portRangeBmpGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
    }
    blockIt = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockClientRangesGet(dev, blockIt, &portRangeBmpGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockClientRangesGet(dev, blockIt, &portRangeBmpGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterFormatSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        1.3. Call with unexpected format
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT formatIt;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT formatGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            for(formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
                formatIt <= CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E;
                formatIt++)
            {
                st = cpssPxCncCounterFormatSet(dev, blockIt, formatIt);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, blockIt, formatIt);

                st = cpssPxCncCounterFormatGet(dev, blockIt, &formatGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);

                UTF_VERIFY_EQUAL0_PARAM_MAC(formatIt, formatGet);
            }
        }

        formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
        blockIt = 0;

        /*  1.2. Call with unexpected blockNum
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncCounterFormatSet(dev, 3, formatIt);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 3, formatIt);

        /*  1.3. Call with unexpected format
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncCounterFormatSet(dev, blockIt, 5);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, 5);
    }
    blockIt = 0;
    formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterFormatSet(dev, blockIt, formatIt);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockIt, formatIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterFormatSet(dev, blockIt, formatIt);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,
        blockIt, formatIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterFormatGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected blockNum
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT      formatGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            st = cpssPxCncCounterFormatGet(dev, blockIt, &formatGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockIt);
        }
        blockIt = 0;

        /*  1.2. Call with unexpected blockNum
            Expected: GT_BAD_PARAM. */
        st = cpssPxCncCounterFormatGet(dev, 3, &formatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 3);
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    blockIt = 0;

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterFormatGet(dev, blockIt, &formatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, blockIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterFormatGet(dev, blockIt, &formatGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterClearByReadEnableSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     enableIt;
    GT_BOOL     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enableIt = GT_FALSE; enableIt <= GT_TRUE; enableIt++)
        {
            st = cpssPxCncCounterClearByReadEnableSet(dev, enableIt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableIt);

            st = cpssPxCncCounterClearByReadEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }
    enableIt = GT_FALSE;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterClearByReadEnableSet(dev, enableIt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enableIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterClearByReadEnableSet(dev, enableIt);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, enableIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterClearByReadEnableGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssPxCncCounterClearByReadEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterClearByReadEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterClearByReadEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterClearByReadValueSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with NULL counterPtr
        Expected: GT_BAD_PTR.
        1.3. Call with bad format
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  formatIt;
    CPSS_PX_CNC_COUNTER_STC         counter;
    CPSS_PX_CNC_COUNTER_STC         counterGet;

    counter.byteCount.l[1] = 0;
    counter.byteCount.l[0] = 178;
    counter.packetCount.l[1] = 0;
    counter.packetCount.l[0] = 34;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
            formatIt <= CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E; formatIt++)
        {
            st = cpssPxCncCounterClearByReadValueSet(dev, formatIt,
                &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, formatIt);

            st = cpssPxCncCounterClearByReadValueGet(dev, formatIt,
                &counterGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, formatIt);

            if(CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E == formatIt)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, counterGet.byteCount.l[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, counterGet.byteCount.l[1]);
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(counter.byteCount.l[0],
                    counterGet.byteCount.l[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(counter.byteCount.l[1],
                    counterGet.byteCount.l[1]);
            }

            if(CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E == formatIt)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, counterGet.packetCount.l[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, counterGet.packetCount.l[1]);
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(counter.packetCount.l[0],
                    counterGet.packetCount.l[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(counter.packetCount.l[1],
                    counterGet.packetCount.l[1]);
            }
        }
        formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

        /*  1.2. Call with NULL counterPtr
            Expected: GT_BAD_PTR. */
        st = cpssPxCncCounterClearByReadValueSet(dev, formatIt, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, formatIt);

        /*  1.3. Call with bad format
            Expected: GT_BAD_PARAM */
        st = cpssPxCncCounterClearByReadValueSet(dev, 5, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 5);
    }
    formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterClearByReadValueSet(dev, formatIt, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, formatIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterClearByReadValueSet(dev, formatIt, &counter);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, formatIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterClearByReadValueGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with NULL counterPtr
        Expected: GT_BAD_PTR.
        1.3. Call with bad format
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  formatIt;
    CPSS_PX_CNC_COUNTER_STC         counterGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
            formatIt <= CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E; formatIt++)
        {
            st = cpssPxCncCounterClearByReadValueGet(dev, formatIt,
                &counterGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, formatIt);
        }
        formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

        /*  1.2. Call with NULL counterPtr
            Expected: GT_BAD_PTR. */
        st = cpssPxCncCounterClearByReadValueGet(dev, formatIt, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, formatIt);

        /*  1.3. Call with bad format
            Expected: GT_BAD_PARAM */
        st = cpssPxCncCounterClearByReadValueGet(dev, 5, &counterGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 5);
    }
    formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterClearByReadValueGet(dev, formatIt, &counterGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            formatIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterClearByReadValueGet(dev, formatIt, &counterGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, formatIt);
}

static CPSS_PX_CNC_COUNTER_STC ut_counters[1030];

UTF_TEST_CASE_MAC(cpssPxCncBlockUploadTrigger)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters with init
        Expected: GT_OK
        1.2. Call with bad block number
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockNum;
    GT_U32      inProcess;
    GT_U32      count;
    GT_U32      timeout;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters with init
            Expected: GT_OK */
        blockNum = 0;
        st = cpssPxCncBlockUploadTrigger(dev, blockNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, blockNum);
        timeout = 10;
        while(1)
        {
            st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if ((inProcess & 0x1) == 0)
            {
                break;
            }
            cpssOsTimerWkAfter(100);
            if(!(timeout--))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the UPLOAD !!!");
                break;/* avoid endless loop */
            }
        }

        count = 1024;
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(1024, count);

        /*  1.2. Call with bad block number
            Expected: GT_BAD_PARAM */
        blockNum = 2;
        st = cpssPxCncBlockUploadTrigger(dev, blockNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, blockNum);
    }
    blockNum = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockUploadTrigger(dev, blockNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            blockNum);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockUploadTrigger(dev, blockNum);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, blockNum);
}

UTF_TEST_CASE_MAC(cpssPxCncBlockUploadInProcessGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK
        1.2. Call with bat pointer to bitmap
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      bmpGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK */
        st = cpssPxCncBlockUploadInProcessGet(dev, &bmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. Call with bat pointer to bitmap
            Expected: GT_BAD_PTR */
        st = cpssPxCncBlockUploadInProcessGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncBlockUploadInProcessGet(dev, &bmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncBlockUploadInProcessGet(dev, &bmpGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCncUploadedBlockGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.1.1 Check that API does not allow read before dump finished
        Expected: GT_NOT_READY.
        1.2. Call with bad format
        Expected: GT_BAD_PARAM.
        1.3. Call with bad parameter to counterValuesPtr
        Expected: GT_BAD_PTR
        1.4. Call with empty message queue
        Expected: GT_NO_MORE
        1.5. Bad pointer to number of  counters
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      inProcess;
    GT_U32      count;
    GT_U32      timeout;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        timeout = 10;
        while(1)
        {
            st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if ((inProcess & 0x1) == 0)
            {
                break;
            }
            cpssOsTimerWkAfter(100);
            if(!(timeout--))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the UPLOAD !!!");
                break;/* avoid endless loop */
            }
        }

        st = cpssPxCncBlockUploadTrigger(dev, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 0);

        /*  1.1.1 Check that API does not allow read before dump finished
            Expected: GT_BAD_STATE. */
        count = 1024;
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_READY, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

        timeout = 10;
        while(timeout)
        {
            st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if((inProcess & 0x1) == 0)
            {
                break;
            }
            cpssOsTimerWkAfter(100);
            if(!(timeout--))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the UPLOAD !!!");
                break;/* avoid endless loop */
            }
        }

        count = 1024;
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

        /*   1.2. Call with bad format
             Expected: GT_BAD_PARAM. */
        timeout = 10;
        while(1)
        {
            st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if((inProcess & 0x1) == 0)
            {
                break;
            }
            cpssOsTimerWkAfter(100);
            if(!(timeout--))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the UPLOAD !!!");
                break;/* avoid endless loop */
            }
        }

        st = cpssPxCncBlockUploadTrigger(dev, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 0);

        timeout = 10;
        while(1)
        {
            st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if((inProcess & 0x1) == 0)
            {
                break;
            }
            cpssOsTimerWkAfter(100);
            if(!(timeout--))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the UPLOAD !!!");
                break;/* avoid endless loop */
            }
        }

        count = 1024;
        st = cpssPxCncUploadedBlockGet(dev, &count, 5, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 5);

        /*  1.3. Call with bad parameter to counterValuesPtr
            Expected: GT_BAD_PTR */
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

        /*  1.4. Call with empty message queue
            Expected: GT_NO_MORE */
        count = 1030;
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NO_MORE, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

        /*  1.5. Bad pointer to number of  counters
            Expected: GT_BAD_PTR */
        count = 1024;
        st = cpssPxCncUploadedBlockGet(dev, NULL,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncUploadedBlockGet(dev, &count,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncUploadedBlockGet(dev, &count,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, ut_counters);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterWraparoundEnableSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     enableIt;
    GT_BOOL     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enableIt = GT_FALSE; enableIt <= GT_TRUE; enableIt++)
        {
            st = cpssPxCncCounterWraparoundEnableSet(dev, enableIt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableIt);

            st = cpssPxCncCounterWraparoundEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }
    enableIt = GT_FALSE;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterWraparoundEnableSet(dev, enableIt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enableIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterWraparoundEnableSet(dev, enableIt);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, enableIt);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterWraparoundEnableGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssPxCncCounterWraparoundEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterWraparoundEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterWraparoundEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterWraparoundIndexesGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad pointer to indexNum
        Expected: GT_BAD_PTR
        1.3. Call with bad value of indexNum
        Expected: GT_BAD_PARAM
        1.4. Call with bad pointer indexesArr
        Expected: GT_BAD_PTR
        1.5. Call with bad blockNum
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    GT_U32      indexIt;
    GT_U32      indexes[2] = {1, 8};
    GT_U32      index;
    GT_U32      indexArr;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            for(indexIt = 0; indexIt < 2; indexIt++)
            {
                index = indexes[indexIt];
                st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, &index,
                    &indexArr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, blockIt, index);
            }

        }
        blockIt = 0;

        /*  1.2. Call with bad pointer to indexNum
            Expected: GT_BAD_PTR */
        st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, NULL,
            &indexArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, blockIt);

        /*  1.3. Call with bad value of indexNum
            Expected: GT_BAD_PARAM */
        index = 0;
        st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, &index,
            &indexArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, index);

        /*  1.4. Call with bad pointer indexesArr
            Expected: GT_BAD_PTR */
        index = indexes[0];
        st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, &index, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, blockIt, index);

        /*  1.5. Call with bad blockNum
            Expected: GT_BAD_PARAM */
        index = indexes[0];
        st = cpssPxCncCounterWraparoundIndexesGet(dev, 2, &index, &indexArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 2, index);
    }
    blockIt = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = indexes[0];
        st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, &index,
            &indexArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, blockIt,
            index);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    index = indexes[0];
    st = cpssPxCncCounterWraparoundIndexesGet(dev, blockIt, &index, &indexArr);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, index);
}

UTF_TEST_CASE_MAC(cpssPxCncCounterGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad blockNum
        Expected: GT_BAD_PARAM
        1.3. Call with bad index
        Expected: GT_BAD_PARAM
        1.4. Call with unexpected format
        Expected: GT_BAD_PARAM
        1.5. Call with bad pointer to counter
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      blockIt;
    GT_U32      formatIt;
    GT_U32      indexes[2] = {0, 1023};
    GT_U32      indexIt;
    CPSS_PX_CNC_COUNTER_STC counterGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(blockIt = 0; blockIt < 2; blockIt++)
        {
            for(indexIt = 0; indexIt < 2; indexIt++)
            {
                for(formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;
                    formatIt <= CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E;
                    formatIt++)
                {
                    st = cpssPxCncCounterGet(dev, blockIt, indexes[indexIt],
                        formatIt, &counterGet);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, blockIt,
                        indexes[indexIt], formatIt);
                }
            }
        }
        blockIt = 0;
        formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

        /*  1.2. Call with bad blockNum
            Expected: GT_BAD_PARAM */
        st = cpssPxCncCounterGet(dev, 2, indexes[0], formatIt, &counterGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, 2, indexes[0],
            formatIt);

        /*  1.3. Call with bad index
            Expected: GT_BAD_PARAM */
        st = cpssPxCncCounterGet(dev, blockIt, 1024, formatIt, &counterGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, 1024,
            formatIt);

        /*  1.4. Call with unexpected format
            Expected: GT_BAD_PARAM */
        st = cpssPxCncCounterGet(dev, blockIt, indexes[0], 5, &counterGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, indexes[0],
            5);

        /*  1.5. Call with bad pointer to counter
            Expected: GT_BAD_PTR */
        st = cpssPxCncCounterGet(dev, blockIt, indexes[0], formatIt, NULL);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, blockIt,
            indexes[0], formatIt);
    }
    blockIt = 0;
    formatIt = CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncCounterGet(dev, blockIt, indexes[0], formatIt,
            &counterGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, blockIt,
            indexes[0], formatIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncCounterGet(dev, blockIt, indexes[0], formatIt,
            &counterGet);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, blockIt, indexes[0],
        formatIt);
}

UTF_TEST_CASE_MAC(cpssPxCncEgressQueueClientModeSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2. Call with unexpected mode
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT modeIt;
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT modeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters and validate
        Expected: GT_OK.*/
        for(modeIt = CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;
            modeIt <= CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E; modeIt++)
        {
            st = cpssPxCncEgressQueueClientModeSet(dev, modeIt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, modeIt);

            st = cpssPxCncEgressQueueClientModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(modeIt, modeGet);
        }

        /*  1.2. Call with unexpected mode
            Expected: GT_BAD_PARAM */
        st = cpssPxCncEgressQueueClientModeSet(dev, 3);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 3);
    }
    modeIt = CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncEgressQueueClientModeSet(dev, modeIt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, modeIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncEgressQueueClientModeSet(dev, modeIt);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, modeIt);
}

UTF_TEST_CASE_MAC(cpssPxCncEgressQueueClientModeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad pointer to mode
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT modeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
        Expected: GT_OK.*/
        st = cpssPxCncEgressQueueClientModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. Call with bad pointer to mode
        Expected: GT_BAD_PTR */
        st = cpssPxCncEgressQueueClientModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncEgressQueueClientModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncEgressQueueClientModeGet(dev, &modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCncClientByteCountModeSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad client
        Expected: GT_BAD_PARAM
        1.3. Call with bad mode
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT      modeIt;
    GT_U32      client;
    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT      modeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;
        for(modeIt = CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E;
            modeIt <= CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E; modeIt++)
        {
            st = cpssPxCncClientByteCountModeSet(dev, client, modeIt);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, client, modeIt);

            st = cpssPxCncClientByteCountModeGet(dev, client, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, client);

            UTF_VERIFY_EQUAL0_PARAM_MAC(modeIt, modeGet);
        }
        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
        modeIt = CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E;

        /*  1.2. Call with bad client
            Expected: GT_BAD_PARAM */
        st = cpssPxCncClientByteCountModeSet(dev, client, modeIt);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, client, modeIt);
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;

        /*  1.3. Call with bad mode
            Expected: GT_BAD_PARAM */
        st = cpssPxCncClientByteCountModeSet(dev, client, 2);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, client, 2);
    }
    client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
    modeIt = CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncClientByteCountModeSet(dev, client, modeIt);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, client,
            modeIt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncClientByteCountModeSet(dev, client, modeIt);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, client, modeIt);
}

UTF_TEST_CASE_MAC(cpssPxCncClientByteCountModeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad client
        Expected: GT_BAD_PARAM
        1.3. Call with bad pointer to mode
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      client;
    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT      modeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;
        st = cpssPxCncClientByteCountModeGet(dev, client, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, client);

        client = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;

        /*  1.2. Call with bad client
            Expected: GT_BAD_PARAM */
        st = cpssPxCncClientByteCountModeGet(dev, client, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, client);
        client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;

        /*  1.3. Call with bad pointer to mode
            Expected: GT_BAD_PTR */
        st = cpssPxCncClientByteCountModeGet(dev, client, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, client);
    }
    client = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncClientByteCountModeGet(dev, client, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, client);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncClientByteCountModeGet(dev, client, &modeGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, client);
}

UTF_TEST_CASE_MAC(cpssPxCncUploadInit)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with bad pointer
        Expected: GT_BAD_PTR
        1.3. Call with bad size
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_DMA_QUEUE_CFG_STC cncCfg;
    GT_UINTPTR restoreDmaPtr;
    GT_U32 restoreDmaSize;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    {
        GT_U32 numDevices = 0;
        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            numDevices++;
        }

        if(numDevices > 1)
        {
            #ifndef ASIC_SIMULATION
            /* need to debug this test ...*/
            PRV_UTF_LOG0_MAC("\n Test CRASH the RD board (only on HW) , so skipped !!!\n\n");
            SKIP_TEST_MAC;/* RD board cause crash */
            #endif /* */
        }

        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    }


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Save */
        restoreDmaSize = PRV_CPSS_PX_PP_MAC(dev)->cncDmaDesc.blockSize;
        restoreDmaPtr = PRV_CPSS_PX_PP_MAC(dev)->cncDmaDesc.block;

        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        cncCfg.dmaDescBlockSize = 8 * 4 * 2048;
        cncCfg.dmaDescBlock = osCacheDmaMalloc(cncCfg.dmaDescBlockSize);
        st = cpssPxCncUploadInit(dev, &cncCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        osCacheDmaFree(cncCfg.dmaDescBlock);

        /*  1.2. Call with bad pointer
            Expected: GT_BAD_PTR */
        cncCfg.dmaDescBlock = NULL;
        cncCfg.dmaDescBlockSize = 8 * 4 * 2048;
        st = cpssPxCncUploadInit(dev, &cncCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3. Call with bad size
            Expected: GT_BAD_PARAM */
        cncCfg.dmaDescBlock = (GT_U8 *)1;
        cncCfg.dmaDescBlockSize = 0;
        st = cpssPxCncUploadInit(dev, &cncCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Restore */
        cncCfg.dmaDescBlock = (GT_U8 *)restoreDmaPtr;
        cncCfg.dmaDescBlockSize = restoreDmaSize * 4/*words*/ * 2/*counters*/;
        st = cpssPxCncUploadInit(dev, &cncCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCncUploadInit(dev, &cncCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxCncUploadInit(dev, &cncCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_SUIT_BEGIN_TESTS_MAC(cpssPxCnc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockClientEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockClientEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockClientRangesSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockClientRangesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterFormatSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterFormatGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterClearByReadEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterClearByReadEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterClearByReadValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterClearByReadValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockUploadTrigger)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncBlockUploadInProcessGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncUploadedBlockGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterWraparoundEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterWraparoundEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterWraparoundIndexesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncEgressQueueClientModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncEgressQueueClientModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncClientByteCountModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncClientByteCountModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCncUploadInit)
UTF_SUIT_END_TESTS_MAC(cpssPxCnc)

