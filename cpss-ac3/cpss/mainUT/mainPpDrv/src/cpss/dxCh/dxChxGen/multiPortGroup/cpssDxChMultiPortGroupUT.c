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
* @file cpssDxChMultiPortGroupUT.c
*
* @brief Unit tests for cpssDxChMultiPortGroup that provides
* APIs for building infrastructure of the connections between
* the port groups of a multi-port group device, to get FDB and TTI
* enhancements.
*
* @version   18
********************************************************************************
*/

/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/multiPortGroup/cpssDxChMultiPortGroup.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Invalid enum */
#define INVALID_ENUM_CNS    0x5AAAAAA5

/*Max size of ringInfoArr. In Enhance mode the ringInfoArr may hold four possible entries*/
#define RING_INFO_ARR_MAX_SIZE      CPSS_MAX_PORT_GROUPS_CNS

/*Max size of uplinkPortsArr. */
#define UPLINK_PORT_ARR_MAX_SIZE    8

/* max 4 ring ports per channel on port group */
#define RING_PORTS_MAX 4

/*Max size of ringPortsArr. */
#define RING_PORT_ARR_MAX_SIZE ((RING_PORTS_MAX * RING_INFO_ARR_MAX_SIZE) * CPSS_MAX_PORT_GROUPS_CNS)

/* max 3 trunks per port group */
#define TRUNKS_MAX  (CPSS_MAX_PORT_GROUPS_CNS - 1)

/*Max size of trunkIdArr. */
#define TRUNK_ID_ARR_MAX_SIZE (TRUNKS_MAX * CPSS_MAX_PORT_GROUPS_CNS)

#define UTF_CHECK_LION_SUPPORT_MAC(devNum)  \
    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)) \
        continue

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_MULTI_PORT_GROUP_LINKED_MODE_ENT    mode,
    IN  GT_U32  numOfUplinkPorts,
    IN  GT_U8   uplinkPortsArr[],
    IN  GT_U32  ringInfoNum,
    IN  CPSS_DXCH_MULTI_PORT_GROUP_RING_INFO_STC   ringInfoArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupConfigSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.2. Call function with all correct params.
         mode [CPSS_DXCH_MULTI_PORT_GROUP_LINKED_MODE_ENHANCED_E].
    Expected: GT_OK.
    1.3. Call function with wrong  mode [INVALID_ENUM_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong uplinkPortsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call function with wrong ringInfoPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_U32      numOfUplinkPorts = 0;
    GT_PORT_NUM       uplinkPortsArr[UPLINK_PORT_ARR_MAX_SIZE];
    GT_U32      ringInfoNum = 0;
    CPSS_DXCH_MULTI_PORT_GROUP_RING_INFO_STC ringInfoArr[RING_INFO_ARR_MAX_SIZE];

    GT_U32      counter = 0;
    GT_PHYSICAL_PORT_NUM       ringPortsArr[RING_PORT_ARR_MAX_SIZE] = {0};
    GT_U32      ringPortsIndex;/* current index in ringPortsArr[] */
    GT_TRUNK_ID trunkIdArr[TRUNK_ID_ARR_MAX_SIZE] = {0};
    GT_U32      ii,jj;
    GT_U32      numRingPortsPerPortGroupPerChannel;/* number of ring ports per port group per channel*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)ringInfoArr, sizeof(ringInfoArr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfUplinkPorts = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

        for(counter = 0; counter < numOfUplinkPorts; counter++)
        {
            /* uplink on each port group */
            uplinkPortsArr[counter] = ( (16 * counter) + 5);
        }

        ringInfoNum = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;
        numRingPortsPerPortGroupPerChannel = 1;

        ringPortsIndex = 0;
        for(counter = 0; counter < ringInfoNum; counter++)
        {
            /* allow checking of srcPortGroupId != 0 */
            ringInfoArr[counter].srcPortGroupId =
                (counter + 1) % PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

            ringInfoArr[counter].numOfRingPorts = numRingPortsPerPortGroupPerChannel *
                    (PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups - 1); /* NO support RU ports needed */

            ringInfoArr[counter].ringPortsPtr   = &ringPortsArr[ringPortsIndex];
            ringInfoArr[counter].trunkIdNum     = 0;

            for(jj = 0 ; jj < PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups ;jj ++)
            {
                for(ii = 0 ; ii <  numRingPortsPerPortGroupPerChannel ; ii++)
                {
                    if (jj == ringInfoArr[counter].srcPortGroupId)
                    {
                        /* jump to next jj */
                        break;
                    }

                    if(ringPortsIndex >= RING_PORT_ARR_MAX_SIZE)
                    {
                        st = GT_BAD_STATE;
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                        break;
                    }

                    ringPortsArr[ringPortsIndex++]  = (
                        (16 * jj) + /* port in 'current' port group */
                        (ii + 6));/* local port number (after the uplinks) */
                }/*ii*/
            }/*jj*/
        }/*counter*/

        UTF_CHECK_LION_SUPPORT_MAC(dev);

        st = cpssDxChMultiPortGroupConfigSet(dev, numOfUplinkPorts,
                                    uplinkPortsArr, ringInfoNum, ringInfoArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for(jj = 0 ; jj < CPSS_MAX_PORT_GROUPS_CNS ;jj ++)
        {
            if(trunkIdArr[jj] == 0)
            {
                continue;
            }

            st = cpssDxChTrunkCascadeTrunkPortsSet(dev,trunkIdArr[jj],NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        /*
            1.4. Call function with wrong uplinkPortsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupConfigSet(dev, numOfUplinkPorts,
                                                    NULL, ringInfoNum, ringInfoArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call function with wrong ringInfoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupConfigSet(dev, numOfUplinkPorts,
                                                  uplinkPortsArr, ringInfoNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupConfigSet(dev, numOfUplinkPorts,
                                    uplinkPortsArr, ringInfoNum, ringInfoArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupConfigSet(dev, numOfUplinkPorts,
                                uplinkPortsArr, ringInfoNum, ringInfoArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet
(
    IN GT_U8             devNum,
    IN GT_TRUNK_ID       trunkId,
    IN GT_U32            numOfPorts,
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[],
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call function with wrong portsArrPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call function with wrong  mode [INVALID_ENUM_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_TRUNK_ID trunkId = 5;
    GT_U32      numOfPorts = 0;
    CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[3];
    CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode = 0;
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* trunk member */

    GT_U32      counter = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)portsArr, sizeof(portsArr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */

        trunkId = 5;
        numOfPorts = sizeof(portsArr) / sizeof(portsArr[0]);
        for(counter = 0; counter < numOfPorts; counter++)
        {
            portsArr[counter].srcPort   = (counter);
            /* use trunk ports from next port group last ports */
            portsArr[counter].trunkPort = (16 + 11 - counter);

            if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
            {
                /* for this device the trunk should exists when calling cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet
                   because the CPSS must check that all ports of the trunk are members in the same hemisphere.
                */
                trunkMember.port = portsArr[counter].trunkPort;
                trunkMember.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(dev);
                st = cpssDxChTrunkMemberAdd(dev,trunkId,&trunkMember);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        for(mode = 0; mode < 2; mode++)
        {
            /* suitable for 'enhanced' mode */
            st = cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet(dev,
                                        trunkId, numOfPorts, portsArr, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(dev))
        {
            /* destroy the trunk */
            st = cpssDxChTrunkMembersSet(dev,trunkId,0,NULL,0,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /*
            1.2. Call function with wrong portsArrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet(dev,
                                        trunkId, numOfPorts, NULL, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call function with wrong  mode [INVALID_ENUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        mode = INVALID_ENUM_CNS;

        st = cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet(dev,
                                    trunkId, numOfPorts, portsArr, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        mode = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet(dev,
                                    trunkId, numOfPorts, portsArr, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet(dev,
                                trunkId, numOfPorts, portsArr, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupPortLookupEnableSet
(
    IN GT_U8      dev,
    IN GT_PHYSICAL_PORT_NUM      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupPortLookupEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK.
        1.1.2. Call cpssDxChMultiPortGroupPortLookupEnableGet.
        Expected: GT_OK and the same value.
    */
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port  = 0;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChMultiPortGroupPortLookupEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
              "cpssDxChMultiPortGroupPortLookupEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);

            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChMultiPortGroupPortLookupEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
              "cpssDxChMultiPortGroupPortLookupEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupPortLookupEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupPortLookupEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    OUT GT_BOOL      *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupPortLookupEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
        1.1.1. Call with valid statusPtr.
        Expected: GT_OK.
        1.1.2. Call wrong statusPtr [NULL].
        Expected: GT_BAD_PTR.
    */
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port = 0;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid statusPtr.
                Expected: GT_OK.
            */
            st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call wrong statusPtr [NULL].
                Expected: GT_OK.
            */
            st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupPortLookupEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupPortRingEnableSet
(
    IN GT_U8      dev,
    IN GT_PHYSICAL_PORT_NUM      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupPortRingEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK.
        1.1.2. Call cpssDxChMultiPortGroupPortRingEnableGet.
        Expected: GT_OK and the same value.
    */
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port  = 0;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChMultiPortGroupPortRingEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
              "cpssDxChMultiPortGroupPortRingEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);

            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChMultiPortGroupPortRingEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
              "cpssDxChMultiPortGroupPortRingEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupPortRingEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupPortRingEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    OUT GT_BOOL      *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupPortRingEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
        1.1.1. Call with valid statusPtr.
        Expected: GT_OK.
        1.1.2. Call wrong statusPtr [NULL].
        Expected: GT_BAD_PTR.
    */
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port = 0;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid statusPtr.
                Expected: GT_OK.
            */
            st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call wrong statusPtr [NULL].
                Expected: GT_OK.
            */
            st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupPortRingEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupLookupNotFoundTableSet
(
    IN GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32  sourcePortGroupId,
    IN  CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC   *infoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupLookupNotFoundTableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChMultiPortGroupLookupNotFoundTableGet
         function with the same params.
    Expected: GT_OK and the same info values.
    1.3. Call function with wrong sourcePortGroupId [4].
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong infoPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call function with wrong info.unknownDaCommand [INVALID_ENUM_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32  sourcePortGroupId = 0;
    CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC   info;

    CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC   infoGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&info, sizeof(info));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        for(sourcePortGroupId = 0; sourcePortGroupId < PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups; sourcePortGroupId++)
        {
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            info.unknownDaCommand = CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_NO_REDIRECT_E;
            info.nextRingInterface.type = CPSS_INTERFACE_PORT_E;
            info.nextRingInterface.devPort.hwDevNum = 0;
            info.nextRingInterface.devPort.portNum = 0;

            st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                         portGroupsBmp, sourcePortGroupId, &info);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourcePortGroupId);

            if(GT_OK == st)
            {
                /*
                    1.2. Call cpssDxChMultiPortGroupLookupNotFoundTableGet
                         function with the same params.
                    Expected: GT_OK and the same info values.
                */
                st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                             portGroupsBmp, sourcePortGroupId, &infoGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(info.unknownDaCommand, infoGet.unknownDaCommand,
                      "get another info.unknownDaCommand than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(info.nextRingInterface.type,
                                             infoGet.nextRingInterface.type,
                      "get another info.nextRingInterface.type than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(info.nextRingInterface.devPort.hwDevNum,
                                             infoGet.nextRingInterface.devPort.hwDevNum,
                      "get another info.nextRingInterface.devPort.hwDevNum than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(info.nextRingInterface.devPort.portNum,
                                             infoGet.nextRingInterface.devPort.portNum,
                      "get another info.nextRingInterface.devPort.PortNum than was set: %d", dev);
            }
        }

        /*
            1.3. Call function with wrong sourcePortGroupId [4].
            Expected: GT_BAD_PARAM.
        */
        sourcePortGroupId = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

        st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        sourcePortGroupId = 0;

        /*
            1.4. Call function with wrong infoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call function with wrong info.unknownDaCommand [INVALID_ENUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        info.unknownDaCommand = INVALID_ENUM_CNS;

        st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        info.unknownDaCommand = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupLookupNotFoundTableSet(dev,
                                 portGroupsBmp, sourcePortGroupId, &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupLookupNotFoundTableGet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32  sourcePortGroupId,
    OUT CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC   *infoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupLookupNotFoundTableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call function with wrong sourcePortGroupId [4].
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong infoPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32  sourcePortGroupId = 0;
    CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC   info;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        for(sourcePortGroupId = 0; sourcePortGroupId < PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups; sourcePortGroupId++)
        {
            st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                         portGroupsBmp, sourcePortGroupId, &info);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourcePortGroupId);
        }

        /*
            1.2. Call function with wrong sourcePortGroupId [4].
            Expected: GT_BAD_PARAM.
        */
        sourcePortGroupId = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

        st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        sourcePortGroupId = 0;

        /*
            1.3. Call function with wrong infoPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupLookupNotFoundTableGet(dev,
                                 portGroupsBmp, sourcePortGroupId, &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet
(
    IN GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32  sourcePortGroupId,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet
         function with the same params.
    Expected: GT_OK and the same enable value.
    1.3. Call function with wrong sourcePortGroupId [4].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32  sourcePortGroupId = 0;
    GT_BOOL enable = GT_TRUE;
    GT_BOOL enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        for(sourcePortGroupId = 0; sourcePortGroupId < PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups; sourcePortGroupId++)
        {
            st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet(dev,
                                         portGroupsBmp, sourcePortGroupId, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourcePortGroupId);

            if(GT_OK == st)
            {
                /*
                    1.2. Call cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet
                         function with the same params.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                             portGroupsBmp, sourcePortGroupId, &enableGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);
            }
        }

        /*
            1.3. Call function with wrong sourcePortGroupId [4].
            Expected: GT_BAD_PARAM.
        */
        sourcePortGroupId = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

        st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        sourcePortGroupId = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet(dev,
                                     portGroupsBmp, sourcePortGroupId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet(dev,
                                 portGroupsBmp, sourcePortGroupId, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32  sourcePortGroupId,
    OUT  GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call function with wrong sourcePortGroupId [4].
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32  sourcePortGroupId = 0;
    GT_BOOL enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        for(sourcePortGroupId = 0; sourcePortGroupId < PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups; sourcePortGroupId++)
        {
            st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                         portGroupsBmp, sourcePortGroupId, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourcePortGroupId);
        }

        /*
            1.2. Call function with wrong sourcePortGroupId [4].
            Expected: GT_BAD_PARAM.
        */
        sourcePortGroupId = PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;

        st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        sourcePortGroupId = 0;

        /*
            1.3. Call function with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                     portGroupsBmp, sourcePortGroupId, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet(dev,
                                 portGroupsBmp, sourcePortGroupId, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupFdbModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT fdbMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupFdbModeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChMultiPortGroupFdbModeGet
         function with the same params.
    Expected: GT_OK and the same value.
    1.3. Call function with wrong fdbMode [INVALID_ENUM_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT fdbMode = 0;
    CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT fdbModeGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        for(fdbMode = CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E;
            fdbMode <= CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_LINKED_E;
            fdbMode++)
        {
            st = cpssDxChMultiPortGroupFdbModeSet(dev, fdbMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fdbMode);

            if(GT_OK == st)
            {
                /*
                    1.2. Call cpssDxChMultiPortGroupFdbModeGet
                         function with the same params.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChMultiPortGroupFdbModeGet(dev, &fdbModeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(fdbMode, fdbModeGet,
                                     "get another fdbMode than was set: %d", dev);
            }
        }

        /*
            1.3. Call function with wrong fdbMode [INVALID_ENUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        fdbMode = INVALID_ENUM_CNS;

        st = cpssDxChMultiPortGroupFdbModeSet(dev, fdbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        fdbMode = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupFdbModeSet(dev, fdbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupFdbModeSet(dev, fdbMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMultiPortGroupFdbModeGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT *fdbModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMultiPortGroupFdbModeGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call function with all correct parameters.
    Expected: GT_OK.
    1.2. Call function with wrong fdbModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT fdbMode = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_CHECK_LION_SUPPORT_MAC(dev);

        /*
            1.1. Call function with correct params.
            Expected: GT_OK.
        */
        st = cpssDxChMultiPortGroupFdbModeGet(dev, &fdbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with wrong fdbModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChMultiPortGroupFdbModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMultiPortGroupFdbModeGet(dev, &fdbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMultiPortGroupFdbModeGet(dev, &fdbMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChMultiPortGroup suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChMultiPortGroup)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupTrunkSrcPortHashMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupPortLookupEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupPortLookupEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupPortRingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupPortRingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupLookupNotFoundTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupLookupNotFoundTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupBridgeUnknownDaMaskEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupFdbModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMultiPortGroupFdbModeGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChMultiPortGroup)

