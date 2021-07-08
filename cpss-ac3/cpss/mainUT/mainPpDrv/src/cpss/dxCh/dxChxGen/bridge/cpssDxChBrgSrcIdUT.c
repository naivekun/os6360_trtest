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
* @file cpssDxChBrgSrcIdUT.c
*
* @brief Unit tests for cpssDxChBrgSrcId, that provides
* CPSS DxCh Source Id facility API
*
* @version   33
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* defines */
#define BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS   0

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGroupPortAdd
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U8   portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGroupPortAdd)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1. Call function with sourceId [0 / UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1].
    Expected: GT_OK.
    1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)].
    Expected: NON GT_OK.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          sourceId;
    GT_PHYSICAL_PORT_NUM port;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with sourceId [0/UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1]. Expected: GT_OK. */
            sourceId = 0;
            st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);

            sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1;
            st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);

            /* 1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)]. Expected: NON GT_OK. */
            sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev);

            st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);
        }

        sourceId = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    sourceId = 0;
    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGroupPortAdd(dev, sourceId, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGroupPortDelete
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U8   portNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGroupPortDelete)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1. Call function with sourceId [0 / UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1].
    Expected: GT_OK.
    1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)].
    Expected: NON GT_OK.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          sourceId;
    GT_PHYSICAL_PORT_NUM port;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*1.1. Call function with sourceId [0/UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1]. Expected: GT_OK. */
            sourceId = 0;

            st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);

            sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1;

            st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);

            /* 1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)]. Expected: NON GT_OK. */
            sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev);

            st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, port);
        }

        sourceId = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    sourceId = 0;
    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGroupPortDelete(dev, sourceId, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGroupEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              sourceId,
    IN GT_BOOL             cpuSrcIdMember,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGroupEntrySet)
{
/*
    ITERATE_DEVICES(DxChx)
    1.1. Call function with sourceId [0/ UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
         cpuSrcIdMember[GT_TRUE / GT_FALSE]
         and non-NULL portsMembersPtr [{00..0011}].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdGroupEntryGet.
    Expected: GT_OK and the same cpuSrcIdMember and  portsMembersPtr.
    1.3. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)],
         cpuSrcIdMember[GT_TRUE] and non-NULL portsMembersPtr [{00..0011}].
    Expected: NON GT_OK.
    1.4. Call function with sourceId [0] cpuSrcIdMember[GT_TRUE] and portsMembersPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  sourceId;
    GT_BOOL                 cpuSrcIdMember;
    CPSS_PORTS_BMP_STC      portsMembers;

    GT_BOOL                 retCpuSrcIdMember;
    CPSS_PORTS_BMP_STC      retPortsMembers;
    GT_BOOL                 isEqual;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with sourceId [0/ UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
                 cpuSrcIdMember[GT_TRUE / GT_FALSE]
                 and non-NULL portsMembersPtr [{00..0011}].
            Expected: GT_OK.
        */
        sourceId = 0;
        cpuSrcIdMember = GT_TRUE;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&retPortsMembers);
        portsMembers.ports[0]=3;

        st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, &portsMembers);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, cpuSrcIdMember);

        /*
            1.2. Call cpssDxChBrgSrcIdGroupEntryGet.
            Expected: GT_OK and the same cpuSrcIdMember and  portsMembersPtr.
        */
        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGroupEntryGet: ", dev, sourceId);
        UTF_VERIFY_EQUAL2_STRING_MAC(cpuSrcIdMember, retCpuSrcIdMember,
                "got another cpuSrcIdMember than was set: %d, %d", dev, sourceId);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&portsMembers,
                                     (GT_VOID*)&retPortsMembers,
                                     sizeof(portsMembers)))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another portsMember than was set: %d, %d", dev, sourceId);

        /*
            1.1. Call function with sourceId [0/ UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
                 cpuSrcIdMember[GT_TRUE / GT_FALSE]
                 and non-NULL portsMembersPtr [{00..0011}].
            Expected: GT_OK.
        */
        sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1;
        cpuSrcIdMember = GT_FALSE;

        st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, &portsMembers);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, cpuSrcIdMember);

        /*
            1.2. Call cpssDxChBrgSrcIdGroupEntryGet.
            Expected: GT_OK and the same cpuSrcIdMember and  portsMembersPtr.
        */
        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGroupEntryGet: ", dev, sourceId);
        UTF_VERIFY_EQUAL2_STRING_MAC(cpuSrcIdMember, retCpuSrcIdMember,
             "got another cpuSrcIdMember than was set: %d, %d", dev, sourceId);
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&portsMembers,
                                     (GT_VOID*)&retPortsMembers,
                                     sizeof(portsMembers)))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another portsMember than was set: %d, %d", dev, sourceId);

        /*
            1.3. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)],
                 cpuSrcIdMember[GT_TRUE] and non-NULL portsMembersPtr [{00..0011}].
            Expected: NON GT_OK.
        */
        sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev);
        cpuSrcIdMember = GT_TRUE;

        st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, &portsMembers);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, sourceId, cpuSrcIdMember);

        /*
            1.4. Call function with sourceId [0] cpuSrcIdMember[GT_TRUE] and portsMembersPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        sourceId = 0;
        cpuSrcIdMember = GT_TRUE;

        st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, NULL);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL",
                                     dev, sourceId, cpuSrcIdMember);
    }

    sourceId = 0;
    cpuSrcIdMember = GT_TRUE;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    portsMembers.ports[0] = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, &portsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGroupEntrySet(dev, sourceId, cpuSrcIdMember, &portsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGroupEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              sourceId,
    OUT GT_BOOL             *cpuSrcIdMemberPtr,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGroupEntryGet)
{
/*
    ITERATE_DEVICES(DxChx)
    1.1. Call function with sourceId [0/UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
         non-NULL cpuSrcIdMemberPtr and non-NULL portsMembersPtr.
    Expected: GT_OK.
    1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)] ,
         non-NULL cpuSrcIdMemberPtr and non-NULL portsMembersPtr.
    Expected: NON GT_OK.
    1.3. Call function with sourceId [0], cpuSrcIdMemberPtr[NULL]
         and non-NULL portsMembersPtr.
    Expected: GT_BAD_PTR.
    1.4. Call function with sourceId [0], non-NULL cpuSrcIdMemberPtr
         and portsMembersPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          sourceId;
    GT_BOOL                         retCpuSrcIdMember;
    CPSS_PORTS_BMP_STC              retPortsMembers;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with sourceId [0/UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
                 non-NULL cpuSrcIdMemberPtr and non-NULL portsMembersPtr.
            Expected: GT_OK.
        */
        sourceId = 0;

        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceId);

        /*
            1.1. Call function with sourceId [0/UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1],
                 non-NULL cpuSrcIdMemberPtr and non-NULL portsMembersPtr.
            Expected: GT_OK.
        */
        sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev) - 1;

        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceId);

        /*
            1.2. Call function with sourceId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)] ,
                 non-NULL cpuSrcIdMemberPtr and non-NULL portsMembersPtr.
            Expected: NON GT_OK.
        */
        sourceId = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev);

        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceId);

        /*
            1.3. Call function with sourceId [0], cpuSrcIdMemberPtr[NULL] and non-NULL portsMembersPtr.
            Expected: GT_BAD_PTR.
        */
        sourceId = 0;

        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, NULL, &retPortsMembers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, sourceId);

        /*
            1.4. Call function with sourceId [0], non-NULL cpuSrcIdMemberPtr and portsMembersPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        sourceId = 0;

        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, portsMembersPtr = NULL", dev, sourceId);
    }

    sourceId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGroupEntryGet(dev, sourceId, &retCpuSrcIdMember, &retPortsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8   devNum,
    IN GT_U8   portNum,
    IN GT_U32  defaultSrcId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortDefaultSrcIdSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with defaultSrcId[0/ 31].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdPortDefaultSrcIdGet.
    Expected: GT_OK and the same defaultSrcId.
    1.3. Call function with defaultSrcId [UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev)] .
    Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_NUM             port;
    GT_U32                  defaultSrcId;
    GT_U32                  retDefaultSrcId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call function with defaultSrcId [0 / UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(dev) - 1].
                Expected: GT_OK.
            */
            defaultSrcId = 0;

            st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, defaultSrcId);

            /*
                1.2. Call cpssDxChBrgSrcIdPortDefaultSrcIdGet.
                Expected: GT_OK and the same defaultSrcId.
            */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdPortDefaultSrcIdGet: %d, %d ", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(defaultSrcId, retDefaultSrcId,
                    "got another defaultSrcId than was set: %d, %d", dev, port);

            /*
                1.1. Call function with defaultSrcId [0 / UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(dev) - 1].
                Expected: GT_OK.
            */
            defaultSrcId = UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(dev) - 1;

            st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, defaultSrcId);

            /*
                1.2. Call cpssDxChBrgSrcIdPortDefaultSrcIdGet.
                Expected: GT_OK and the same defaultSrcId.
            */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgSrcIdPortDefaultSrcIdGet: %d, %d ",
                                         dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(defaultSrcId, retDefaultSrcId,
                                         "got another defaultSrcId than was set: %d, %d", dev, port);

            /*
                1.3. Call function with defaultSrcId [UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(dev)].
                Expected: NON GT_OK.
            */
            defaultSrcId = UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(dev);

            st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, defaultSrcId);
        }

        defaultSrcId = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    defaultSrcId = 0;
    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortDefaultSrcIdSet(dev, port, defaultSrcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U32  *defaultSrcIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortDefaultSrcIdGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1. Call function with non-NULL defaultSrcIdPtr.
    Expected: GT_OK.
    1.2. Call function with defaultSrcIdPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_PORT_NUM     port;
    GT_U32          retDefaultSrcId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL defaultSrcIdPtr. Expected: GT_OK. */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call function with defaultSrcIdPtr[NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortDefaultSrcIdGet(dev, port, &retDefaultSrcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGlobalUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGlobalUcastEgressFilterSet)
{
/*
    ITERATE_DEVICES(DxChx)
    1.1. Call function with enable[GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdGlobalUcast EgressFilterGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_BOOL             enable;
    GT_BOOL             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChBrgSrcIdGlobalUcast EgressFilterGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, &retEnable);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgSrcIdGlobalUcastEgressFilterGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, retEnable,
            "got another enable than was set: %d", dev);

        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChBrgSrcIdGlobalUcast EgressFilterGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, &retEnable);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgSrcIdGlobalUcastEgressFilterGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, retEnable,
            "got another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGlobalUcastEgressFilterGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGlobalUcastEgressFilterGet)
{
/*
    ITERATE_DEVICES(DxChx)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_BOOL             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */

        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with enablePtr[NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(dev, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet)
{
/*
    ITERATE_DEVICES(DxCh1 or xCat and above)
    1.1. Call function with mode[CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E
                               / CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet.
    Expected: GT_OK and the same mode.
    1.3. Call function with mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                        st = GT_OK;
    GT_U8                            dev;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  retMode;
    CPSS_PP_FAMILY_TYPE_ENT          devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call function with mode[CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E /
                                        CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
           Expected: GT_OK.
        */
        /*call with mode [CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E]*/
        mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;

        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet.
            Expected: GT_OK and the same mode.
        */
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, &retMode);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, retMode,
                "got another mode than was set: %d", dev);

        /*
            1.1. Call function with mode[CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E /
                                        CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
           Expected: GT_OK.
        */
        /*call with mode [CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E]*/
        mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E;

        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet.
            Expected: GT_OK and the same mode.
        */
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, &retMode);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, retMode,
                           "got another mode than was set: %d", dev);

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet)
{
/*
    ITERATE_DEVICES(DxCh1 or xCat and above)
    1.1. Call function with non-NULL modePtr.
    Expected: GT_OK.
    1.2. Call function with modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                        st = GT_OK;
    GT_U8                            dev;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  retMode;
    CPSS_PP_FAMILY_TYPE_ENT          devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Call function with non-NULL modePtr. Expected: GT_OK. */
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, &retMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with modePtr[NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, &retMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(dev, &retMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortSrcIdAssignModeSet
(
    IN GT_U8                            devNum,
    IN GT_U8                            portNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortSrcIdAssignModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2, DxCh3)
    1.1. Call function with mode[CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E
                               / CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdPortSrcIdAssignModeGet.
    Expected: GT_OK and the same mode.
    1.3. Call function with mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8                           dev;
    GT_PORT_NUM                     port;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT mode;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT retMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call function with mode
                             [CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E
                            / CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
                Expected: GT_OK.
            */
            mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;

            st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.2. Call cpssDxChBrgSrcIdPortSrcIdAssignModeGet.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdPortSrcIdAssignModeGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, retMode,
                    "got another mode than was set: %d, %d", dev, port);

            /*
                1.1. Call function with mode
                             [CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E
                            / CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E].
                Expected: GT_OK.
            */
            mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E;

            st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
                1.2. Call cpssDxChBrgSrcIdPortSrcIdAssignModeGet.
                Expected: GT_OK and the same mode.
            */
            st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdPortSrcIdAssignModeGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, retMode,
                    "got another mode than was set: %d, %d", dev, port);

            /*
                1.3. Call function with wrong enum values mode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgSrcIdPortSrcIdAssignModeSet
                                                        (dev, port, mode),
                                                        mode);
        }

        mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    mode = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;
    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortSrcIdAssignModeGet
(
    IN GT_U8                            devNum,
    IN GT_U8                            portNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortSrcIdAssignModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2, DxCh3)
    1.1. Call function with non-NULL modePtr.
    Expected: GT_OK.
    1.2. Call function with modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_PORT_NUM                             port;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT         retMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL modePtr. Expected: GT_OK. */

            st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call function with modePtr[NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(dev, port, &retMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortUcastEgressFilterSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
    1.1. Call function with enable[GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdPortUcastEgressFilterGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL                 enable;
    GT_BOOL                 retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call function with enable[GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2. Call cpssDxChBrgSrcIdPortUcastEgressFilterGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgSrcIdPortUcastEgressFilterGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                  "got another enable than was set: %d, %d", dev, port);

            /*
                1.1. Call function with enable[GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.2. Call cpssDxChBrgSrcIdPortUcastEgressFilterGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdPortUcastEgressFilterGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "got another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;
    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortUcastEgressFilterSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortUcastEgressFilterGet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortUcastEgressFilterGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh2 and above)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL                 retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call function with enablePtr[NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,"%d, %d, NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortUcastEgressFilterGet(dev, port, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortSrcIdForceEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (xCat and above)
    1.1. Call function with enable[GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdPortSrcIdForceEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_NUM             port;
    GT_BOOL                 enable;
    GT_BOOL                 retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
               1.1. Call function with enable[GT_TRUE / GT_FALSE].
               Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.2. Call cpssDxChBrgSrcIdPortSrcIdForceEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgSrcIdPortSrcIdForceEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                  "got another enable than was set: %d, %d", dev, port);

            /*
               1.1. Call function with enable[GT_TRUE / GT_FALSE].
               Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.2. Call cpssDxChBrgSrcIdPortSrcIdForceEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdPortSrcIdForceEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "got another enable than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;
    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdPortSrcIdForceEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (xCat and above)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_NUM             port;
    GT_BOOL                 retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with non-NULL enablePtr. Expected: GT_OK. */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call function with enablePtr[NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,"%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_SRC_ID_VALID_PHY_PORT_NUM_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(dev, port, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdEtagTypeLocationSet)
{
/*
    1.1. Call function with bitLocation[0 / 15].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgSrcIdEtagTypeLocationGet.
    Expected: GT_OK and the same bitLocation.
    1.3. Call function with bitLocation [16].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  bitLocation;
    GT_U32                  bitLocationGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with bitLocation[0 / 15].
            Expected: GT_OK.
        */
        bitLocation = 0;

        st = cpssDxChBrgSrcIdEtagTypeLocationSet(dev, bitLocation);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bitLocation);

        /*
            1.2. Call cpssDxChBrgSrcIdEtagTypeLocationGet.
            Expected: GT_OK and the same bitLocation.
        */
        st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, &bitLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdEtagTypeLocationGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(bitLocation, bitLocationGet,
                "got another bitLocation than was set: %d, %d", dev);

        /*
            1.1. Call function with bitLocation[0 / 15].
            Expected: GT_OK.
        */
        bitLocation = UTF_CPSS_PP_MAX_SRC_ID_ETAG_TYPE_LOC_BIT_MAC(dev) - 1;

        st = cpssDxChBrgSrcIdEtagTypeLocationSet(dev, bitLocation);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bitLocation);

        /*
            1.2. Call cpssDxChBrgSrcIdEtagTypeLocationGet.
            Expected: GT_OK and the same bitLocation.
        */
        st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, &bitLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdEtagTypeLocationGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(bitLocation, bitLocationGet,
                "got another bitLocation than was set: %d, %d", dev);

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for bitLocation.                         */
        bitLocation = UTF_CPSS_PP_MAX_SRC_ID_ETAG_TYPE_LOC_BIT_MAC(dev);

        st = cpssDxChBrgSrcIdEtagTypeLocationSet(dev, bitLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    bitLocation = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdEtagTypeLocationSet(dev, bitLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdEtagTypeLocationSet(dev, bitLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdEtagTypeLocationGet)
{
/*
    1.1. Call function with non-NULL bitLocationPtr.
    Expected: GT_OK.
    1.2. Call function with bitLocationPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  bitLocation;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. Call function with non-NULL bitLocationPtr. Expected: GT_OK. */
            st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, &bitLocation);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2. Call function with bitLocation[NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, &bitLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgSrcIdEtagTypeLocationGet(dev, &bitLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Source ID Egress Filtering table.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgSrcIdFillSrcIdGroupTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size. numEntries [32].
    1.2. Fill all entries in Source ID Egress Filtering table.
         Call cpssDxChBrgSrcIdGroupEntrySet with sourceId [0..numEntries-1],
                                                 cpuSrcIdMember[GT_TRUE]
                                                 and non-NULL portsMembersPtr [{00..0011}].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgSrcIdGroupEntrySet with sourceId [numEntries],
                                                 cpuSrcIdMember[GT_TRUE]
                                                 and non-NULL portsMembersPtr [{00..0011}].
    Expected: NOT GT_OK.
    1.4. Read all entries in Source ID Egress Filtering table and compare with original.
         Call cpssDxChBrgSrcIdGroupEntryGet with not NULL cpuSrcIdMemberPtr and portsMembersPtr.
    Expected: GT_OK and the same cpuSrcIdMember and  portsMembersPtr.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgSrcIdGroupEntryGet with sourceId [numEntries],
         not NULL cpuSrcIdMemberPtr and portsMembersPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL                 isEqual    = GT_FALSE;
    GT_U32                  numEntries = 0;
    GT_U32                  iTemp      = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    GT_BOOL                 cpuSrcIdMember = GT_FALSE;
    CPSS_PORTS_BMP_STC      portsMembers;

    GT_BOOL                 retCpuSrcIdMember = GT_FALSE;
    CPSS_PORTS_BMP_STC      retPortsMembers;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&retPortsMembers);

    /* Fill the entry for Source ID Egress Filtering table */
    cpuSrcIdMember = GT_TRUE;
    portsMembers.ports[0]=3;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        numEntries = UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in Source ID Egress Filtering table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgSrcIdGroupEntrySet(dev, iTemp, cpuSrcIdMember, &portsMembers);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgSrcIdGroupEntrySet: %d, %d, %d", dev, iTemp, cpuSrcIdMember);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChBrgSrcIdGroupEntrySet(dev, numEntries, cpuSrcIdMember, &portsMembers);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChBrgSrcIdGroupEntrySet: %d, %d, %d",
                                         dev, numEntries, cpuSrcIdMember);

        /* 1.4. Read all entries in Source ID Egress Filtering table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgSrcIdGroupEntryGet(dev, iTemp, &retCpuSrcIdMember, &retPortsMembers);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGroupEntryGet: ", dev, iTemp);

            UTF_VERIFY_EQUAL2_STRING_MAC(cpuSrcIdMember, retCpuSrcIdMember,
                "got another cpuSrcIdMember than was set: %d, %d", dev, iTemp);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&portsMembers,
                                         (GT_VOID*)&retPortsMembers, sizeof(portsMembers)))
                       ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                    "got another portsMember than was set: %d, %d", dev, iTemp);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChBrgSrcIdGroupEntryGet(dev, numEntries, &retCpuSrcIdMember, &retPortsMembers);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgSrcIdGroupEntryGet: ", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgSrcId suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgSrcId)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGroupPortAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGroupPortDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortDefaultSrcIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortDefaultSrcIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGlobalUcastEgressFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGlobalUcastEgressFilterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortSrcIdAssignModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortSrcIdAssignModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortUcastEgressFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortUcastEgressFilterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortSrcIdForceEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdPortSrcIdForceEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdEtagTypeLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdEtagTypeLocationGet)
    /* Test filling Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgSrcIdFillSrcIdGroupTable)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgSrcId)

