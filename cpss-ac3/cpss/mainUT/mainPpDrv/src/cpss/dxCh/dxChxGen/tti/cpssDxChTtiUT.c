/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChTtiUT.c
*
* @brief Unit tests for cpssDxChTti, that provides
* CPSS tunnel termination.
*
* @version   67
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpssCommon/cpssPresteraDefs.h>

/* defines */

/* Default valid value for port/trunk id */
#define TTI_VALID_PORT_TRUNK_CNS 20

/* Default valid value for vlan id */
#define TTI_VALID_VLAN_ID_CNS           100

/* Default valid value for port id */
#define TTI_VALID_PHY_PORT_CNS          0

/* minimum value for cpu code base field */
#define CPU_CODE_BASE_MIN_CNS           192

/* maximum value for cpu code base field */
#define CPU_CODE_BASE_MAX_CNS           244

/* the size of tti Adjacency table entry in words */
#define TTI_ADJACENCY_SIZE_CNS          8

/* maximum value for Adjacency index */
#define ADJACENCY_MAX_CNS              127


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiMacToMeSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with entryIndex [0 / 3 / max],
                        value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                            vlanId [TTI_VALID_VLAN_ID_CNS, 1000, max]
                              },
                         mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                            vlanId [0x0FFF]
                              },
                interfaceInfo {
                               includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E /
                                                    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                         srcDevice [devNum],
                                        srcIsTrunk [GT_FALSE / GT_TRUE],
                                      srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                              }.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiMacToMeGet.
    Expected: GT_OK and the same value, mask and interfaceInfo.
    1.3. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.
    1.4. Call with out of range value.vlanId, mask.vlanId [0] and
                   other valid params.
    Expected: NOT GT_OK.
    1.5. Call with out of range value.vlanId, mask.vlanId [0x0FFF] and
                   other valid params.
    Expected: NOT GT_OK.
    1.6. Call with wrong interfaceInfo.includeSrcInterface enum values and
                   other valid params.
    Expected: GT_BAD_PARAM.
    1.7. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                      CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                   out of range interfaceInfo.srcDevice (not relevant) and
                   other valid params.
    Expected: GT_OK.
    1.8. Call with out of range interfaceInfo.srcDevice and other valid params.
    Expected: NOT GT_OK.
    1.9. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],
                   out of range interfaceInfo.srcPortTrunk (not relevant) and
                   other valid params.
    Expected: GT_OK.
    1.10. Call with out of range interfaceInfo.srcPortTrunk and other valid params.
    Expected: NOT GT_OK.
    1.11. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.12. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.13. Call with NULL interfaceInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum;

    GT_U32                                          entryIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      value;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      valueGet;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      mask;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      maskGet;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfoGet;
    GT_BOOL                                         isEqual = GT_FALSE;

    cpssOsBzero((GT_VOID*) &value,                  sizeof(value));
    cpssOsBzero((GT_VOID*) &valueGet,               sizeof(valueGet));
    cpssOsBzero((GT_VOID*) &mask,                   sizeof(mask));
    cpssOsBzero((GT_VOID*) &maskGet,                sizeof(maskGet));
    cpssOsBzero((GT_VOID*) &interfaceInfo,          sizeof(interfaceInfo));
    cpssOsBzero((GT_VOID*) &interfaceInfoGet,       sizeof(interfaceInfoGet));


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex [0 / 3 / max],
                                value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                    vlanId [TTI_VALID_VLAN_ID_CNS, 1000, max]
                                      },
                                 mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                    vlanId [0x0FFF]
                                      },
                        interfaceInfo {
                                       includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                            CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E /
                                                            CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                                 srcDevice [devNum]
                                                srcIsTrunk [GT_FALSE / GT_TRUE],
                                              srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                                      }.
            Expected: GT_OK.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call cpssDxChTtiMacToMeGet.
            Expected: GT_OK and the same value, mask and interfaceInfo.
        */
        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                   &interfaceInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                     (GT_VOID*) &valueGet.mac,
                                     sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                   "got another value.mac then was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
            "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                     (GT_VOID*) &maskGet.mac,
                                     sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another mask.mac then was set: %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
            "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

        UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
            "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);

        /* Call with entryIndex [3],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [1000]
                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                           srcDevice [0],
                                          srcIsTrunk [GT_FALSE],
                                        srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                }.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 1000;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = 0;
        interfaceInfo.srcIsTrunk            = GT_FALSE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                    &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                "get another interfaceInfo.srcHwDevice than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Call with entryIndex [max],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [max],

                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                           srcDevice [0],
                                          srcIsTrunk [GT_TRUE],
                                        srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                }.
        */
        entryIndex                          = 7;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = 0;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                       &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                "get another interfaceInfo.srcHwDevice than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Call with entryIndex [0],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [100]
                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                           srcDevice [0],
                                          srcIsTrunk [GT_FALSE],
                                        srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                }.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = 0;
        interfaceInfo.srcIsTrunk            = GT_FALSE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                       &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                "get another interfaceInfo.srcHwDevice than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Call with entryIndex [3],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [1000]
                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                           srcDevice [0],
                                          srcIsTrunk [GT_TRUE],
                                        srcPortTrunk [max/2]
                                }
        */
        entryIndex                          = 3;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 1000;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = 0;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1) / 2;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                       &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                "get another interfaceInfo.srcHwDevice than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Call with entryIndex [max],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [max]
                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                          srcIsTrunk [GT_FALSE],
                                        srcPortTrunk [max]
                                }
        */
        entryIndex                          = 7;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
        interfaceInfo.srcIsTrunk            = GT_FALSE;
        interfaceInfo.srcPortTrunk          = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK_MAC(15);

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                       &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                        (GT_VOID*) &valueGet.mac,
                                        sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Call with entryIndex [0],
                          value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [100]
                                },
                           mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]
                                },
                  interfaceInfo {
                                 includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                          srcIsTrunk [GT_TRUE],
                                        srcPortTrunk [max]
                                }
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

            /*
                1.2. Call cpssDxChTtiMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &valueGet, &maskGet,
                                       &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacToMeGet: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another value.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d", devNum, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d", devNum, entryIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                "get another interfaceInfo.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                "get another interfaceInfo.srcPortTrunk than was set: %d, %d", devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /*
            1.3. Call with out of range entryIndex and other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            entryIndex = 128;
        }
        else
        {
            entryIndex = 8;
        }

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params. */
        entryIndex = 0;

        /*
            1.4. Call with out of range value.vlanId, mask.vlanId [0] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            value.vlanId                        = BIT_13;
        }
        else
        {
            value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS;
        }


        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore correct values. */
        value.vlanId                        = 100;
        mask.vlanId                         = 0x0FFF;

        /*
            1.5. Call with out of range value.vlanId, mask.vlanid [0x0FFF] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            value.vlanId                        = BIT_13;
        }
        else
        {
            value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS;
        }

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params. */
        value.vlanId                        = 100;

        /*
            1.6. Call with wrong interfaceInfo.includeSrcInterface enum values and
                           other valid params.
            Expected: GT_BAD_PARAM.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.srcHwDevice             = 0;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiMacToMeSet
                                (devNum, entryIndex, &value, &mask, &interfaceInfo),
                                 interfaceInfo.includeSrcInterface);
        }

        /* Restore valid params. */
        interfaceInfo.includeSrcInterface   = 0;

        /*
            1.7. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                              CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                           out of range interfaceInfo.srcDevice (not relevant) and
                           other valid params.
            Expected: GT_OK.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],   */
        /*           out of range interfaceInfo.srcDevice (not relevant) and                                            */
        /*           other valid params.                                                                                */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
        interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /*
            1.8. Call with out of range interfaceInfo.srcDevice and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                          = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Restore valid params. */
        interfaceInfo.srcHwDevice             = devNum;

        /*
            1.9. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],
                           out of range interfaceInfo.srcPortTrunk (not relevant) and
                           other valid params.
            Expected: GT_OK.
        */
        entryIndex = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = devNum;
        interfaceInfo.srcIsTrunk            = GT_FALSE;
        interfaceInfo.srcPortTrunk          = (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15)) + 1;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Calling with value.srcIsTrunk [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],   */
        /*              value.srcPortTrunk [max],                                           */
        /*              out of range interfaceInfo.srcPortTrunk (not relevant) and          */
        /*              others valid params.                                                */
        entryIndex = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = devNum;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.10. Call with out of range value.srcPortTrunk and other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = devNum;
        interfaceInfo.srcIsTrunk            = GT_FALSE;
        interfaceInfo.srcPortTrunk          = (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15)) + 1;

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Calling with value.srcIsTrunk [1], value.srcPortTrunk [max], and     */
        /* others valid params.                                                 */
        entryIndex = 0;

        value.mac.arEther[0]                = 0xAA;
        value.mac.arEther[1]                = 0xAA;
        value.mac.arEther[2]                = 0xAA;
        value.mac.arEther[3]                = 0xAA;
        value.mac.arEther[4]                = 0xAA;
        value.mac.arEther[5]                = 0xAA;
        value.vlanId                        = 100;

        cpssOsMemSet(&mask, 0xFF, sizeof(mask));
        mask.vlanId                         = 0x0FFF;

        interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
        interfaceInfo.srcHwDevice             = devNum;
        interfaceInfo.srcIsTrunk            = GT_TRUE;
        interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        /* Restore valid params. */
        interfaceInfo.srcPortTrunk            = TTI_VALID_PORT_TRUNK_CNS;

        /*
            1.11. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, NULL, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                     devNum, entryIndex);

        /*
            1.12. Call with NULL maskPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, NULL, &interfaceInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, maskPtr = NULL",
                                     devNum, entryIndex);

        /*
            1.13. Call with NULL interfaceInfoPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask, NULL);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, interfaceInfoPtr = NULL",
                                         devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    entryIndex = 0;

    value.mac.arEther[0]        = 0xAA;
    value.mac.arEther[1]        = 0xAA;
    value.mac.arEther[2]        = 0xAA;
    value.mac.arEther[3]        = 0xAA;
    value.mac.arEther[4]        = 0xAA;
    value.mac.arEther[5]        = 0xAA;
    value.vlanId                = 100;

    cpssOsMemSet(&mask, 0xFF, sizeof(mask));
    mask.vlanId                         = 0x0FFF;

    interfaceInfo.includeSrcInterface   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMacToMeSet(devNum, entryIndex, &value, &mask, &interfaceInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiMacToMeGet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with entryIndex [0 / 7] and other valid params.
    Expected: GT_OK.
    1.2. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.
    1.3. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL interfaceInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_U32                                              entryIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC                          value;
    CPSS_DXCH_TTI_MAC_VLAN_STC                          mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC      interfaceInfo;

    cpssOsBzero((GT_VOID*) &value, sizeof(value));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*) &interfaceInfo, sizeof(interfaceInfo));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex [0 / 7] and other valid params.
            Expected: GT_OK.
        */
        /* iterate with entryIndex = 0 */
        entryIndex = 0;

        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* iterate with entryIndex = 7 */
        entryIndex = 7;

        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call with out of range entryIndex and other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            entryIndex = 128;
        }
        else
        {
            entryIndex = 8;
        }

        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, &interfaceInfo);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params. */
        entryIndex = 0;

        /*
            1.3. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, NULL, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                     devNum, entryIndex);

        /*
            1.4. Call with NULL maskPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, NULL, &interfaceInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, maskPtr = NULL",
                                     devNum, entryIndex);

        /*
            1.5. Call with NULL interfaceInfoPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, NULL);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, &interfaceInfo = NULL",
                                         devNum, entryIndex);
        }
        else
        {
            /* this device ignore the interfaceInfoPtr parameters */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    entryIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMacToMeGet(devNum, entryIndex, &value, &mask, &interfaceInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortLookupEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E,
                              CPSS_DXCH_TTI_KEY_ETH_E,
                              CPSS_DXCH_TTI_KEY_MIM_E(xCat Lion xCat2 Lion2 Bobcat2; Caelum; Bobcat3 only),
                              CPSS_DXCH_TTI_KEY_TRILL_E(SIP5 only),
                              CPSS_DXCH_TTI_KEY_IPV6_E(SIP5 only),
                              CPSS_DXCH_TTI_KEY_IPV4_UDP_E(SIP5 only)] and
                      enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortLookupEnableGet.
    Expected: GT_OK and the same enable.
    1.1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                    CPSS_DXCH_TTI_KEY_TRILL_E].
    Expected: NOT GT_OK.
    1.1.4. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   devNum      = 0;
    GT_PORT_NUM                             portNum     = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType     = CPSS_DXCH_TTI_KEY_IPV4_E;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableGet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with
                     keyType [CPSS_DXCH_TTI_KEY_IPV4_E,
                              CPSS_DXCH_TTI_KEY_ETH_E,
                              CPSS_DXCH_TTI_KEY_MIM_E(xCat Lion xCat2 Lion2 Bobcat2; Caelum; Bobcat3 only),
                              CPSS_DXCH_TTI_KEY_TRILL_E(SIP5 only),
                              CPSS_DXCH_TTI_KEY_IPV6_E(SIP5 only),
                              CPSS_DXCH_TTI_KEY_IPV4_UDP_E(SIP5 only)] and
                     enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK.
            */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
            enable  = GT_TRUE;

            st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                keyType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                        keyType, enable);

            /*
                1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d, %d", devNum, portNum);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_ETH_E]. */
            keyType = CPSS_DXCH_TTI_KEY_ETH_E;
            enable  = GT_FALSE;

            st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                keyType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                        keyType, enable);

            /*
                1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d, %d", devNum, portNum);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            keyType, enable);
            }

            /*
                1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d, %d", devNum, portNum);
/* TBD TRILL */
#if 0

            /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            keyType, enable);
            }

            /*
                1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d, %d", devNum, portNum);


            /* Call with keyType [CPSS_DXCH_TTI_KEY_IPV6_E(SIP5 only)]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_IPV6_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            keyType, enable);

                /*
                    1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                    Expected: GT_OK and the same enable.
                */
                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                    keyType, &enableGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable then was set: %d, %d", devNum, portNum);
            }

            /* Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_UDP_E(SIP5 only)]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_IPV4_UDP_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            keyType, enable);

                /*
                    1.1.2. Call cpssDxChTtiPortLookupEnableGet.
                    Expected: GT_OK and the same enable.
                */
                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                    keyType, &enableGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortLookupEnableGet: %d, %d, %d", devNum, portNum, keyType);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "got another enable then was set: %d, %d", devNum, portNum);
            }
#endif
            /*
                1.1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                                CPSS_DXCH_TTI_KEY_TRILL_E].
                Expected: NOT GT_OK.
            */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                                keyType, enable);
            }
/* TBD TRILL */
#if 0
            /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E] . */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                    keyType, enable);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                                keyType, enable);
            }

            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            /*
                1.1.4. Call with wrong keyType enum values and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortLookupEnableSet
                                (devNum, portNum, keyType, enable),
                                keyType);
#endif
        }

        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        enable  = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                                keyType, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                            keyType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortLookupEnableSet(devNum, portNum,
                                            keyType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;
    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    enable  = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortLookupEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                              CPSS_DXCH_TTI_KEY_ETH_E /
                              CPSS_DXCH_TTI_KEY_MIM_E /
                              CPSS_DXCH_TTI_KEY_TRILL_E,
                              CPSS_DXCH_TTI_KEY_IPV6_E(SIP5 only),
                              CPSS_DXCH_TTI_KEY_IPV4_UDP_E(SIP5 only)] and
                             other valid params.
    Expected: GT_OK.
    1.1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                    CPSS_DXCH_TTI_KEY_TRILL_E].
    Expected: NOT GT_OK.
    1.1.3. Call with wrong keyType enum values and
                     other valid params.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with NULL enablePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   devNum      = 0;
    GT_PORT_NUM                             portNum     = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType     = CPSS_DXCH_TTI_KEY_IPV4_E;
    GT_BOOL                                 enable      = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                          CPSS_DXCH_TTI_KEY_ETH_E /
                                          CPSS_DXCH_TTI_KEY_MIM_E /
                                          CPSS_DXCH_TTI_KEY_TRILL_E] and
                                 other valid params.
                Expected: GT_OK.
            */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                                        portNum, keyType);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_ETH_E]. */
            keyType = CPSS_DXCH_TTI_KEY_ETH_E;

            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, keyType);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||

               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                    keyType, &enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, keyType);
            }
/* TBD TRILL */
#if 0
            /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_TRILL_E;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, keyType);
            }

            /* Call with keyType [CPSS_DXCH_TTI_KEY_IPV6_E(SIP5 only)]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_IPV6_E;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, keyType);
            }

            /* Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_UDP_E(SIP5 only)]. */
            if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
               (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_IPV4_UDP_E;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, keyType);
            }

            /*
                1.1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                                CPSS_DXCH_TTI_KEY_TRILL_E].
                Expected: NOT GT_OK.
            */

            /* added a validity check on devNum to avoid coverity warnings */
            if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
            {
                st = utfFailureMsgLog("Out of range devNum.\n", NULL, 0);
                return;
            }

            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                    keyType, &enable);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                                keyType);
            }

            /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E] . */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
                enable  = GT_FALSE;

                st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                    keyType, &enable);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum,
                                                keyType, enable);
            }

            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            /*
                1.1.3. Call with wrong keyType enum values and
                                 other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortLookupEnableGet
                                (devNum, portNum, keyType, &enable),
                                keyType);

            /*
                1.1.4. Call with NULL enablePtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL",
                                         devNum, portNum);
#endif
        }

        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChTtiPortLookupEnableGet(devNum, portNum,
                                                keyType, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpv4OnlyTunneledEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.1.2. Call function cpssDxChTtiPortIpv4OnlyTunneledEnableGet
                with not NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK.
            */
            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortIpv4OnlyTunneledEnableGet
                            with not NULL enablePtr.
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st,
                       "cpssDxChTtiPortIpv4OnlyTunneledEnableGet: %d, %d", devNum, portNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL3_PARAM_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);

            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortIpv4OnlyTunneledEnableGet
                            with not NULL enablePtr.
                Expected: GT_OK and enableGet the same as was set.
            */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st,
                       "cpssDxChTtiPortIpv4OnlyTunneledEnableGet: %d, %d", devNum, portNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL3_PARAM_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU portNum number.                                          */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpv4OnlyTunneledEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;

    GT_BOOL     enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call function with enablePtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", devNum, portNum);
        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum,
                                                          &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                 */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpv4OnlyMacToMeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.1.2. Call function cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
                         with not NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
                                     with not NULL enablePtr.
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChTtiPortIpv4OnlyMacToMeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);

            /* Call with enable [GT_FALSE]. */
            enable = GT_FALSE;

            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
                                     with not NULL enablePtr.
                Expected: GT_OK and enableGet the same as was set.
            */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChTtiPortIpv4OnlyMacToMeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);
        }

        enable = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;
    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpv4OnlyMacToMeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;


    GT_BOOL     enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum,
                                                         &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call function with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL",
                                         devNum, portNum);
        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum,
                                                         &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiIpv4McEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiIpv4McEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.2. Call function cpssDxChTtiIpv4McEnableGet with not NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* iterate with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChTtiIpv4McEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call function cpssDxChTtiIpv4McEnableGet with not NULL enablePtr.
            Expected: GT_OK and enable the same as was set.
        */
        st = cpssDxChTtiIpv4McEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiIpv4McEnableGet: %d", devNum);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "got another enable then was set: %d", devNum);

        /* iterate with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChTtiIpv4McEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call function cpssDxChTtiIpv4McEnableGet with not NULL enablePtr.
            Expected: GT_OK and enable the same as was set.
        */
        st = cpssDxChTtiIpv4McEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiIpv4McEnableGet: %d", devNum);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "got another enable then was set: %d", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiIpv4McEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIpv4McEnableSet(devNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiIpv4McEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiIpv4McEnableGet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_BOOL     enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTtiIpv4McEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call function with enablePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChTtiIpv4McEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiIpv4McEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIpv4McEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortMplsOnlyMacToMeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call function cpssDxChTtiPortMplsOnlyMacToMeEnableGet
                         with not NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortMplsOnlyMacToMeEnableGet
                                     with not NULL enablePtr.
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum,
                                                         &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortMplsOnlyMacToMeEnableGet: %d, %d", devNum, portNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);

            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call function cpssDxChTtiPortMplsOnlyMacToMeEnableGet
                                     with not NULL enablePtr.
                Expected: GT_OK and enable the same as was set.
            */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum,
                                                         &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortMplsOnlyMacToMeEnableGet: %d, %d", devNum, portNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d, %d", devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortMplsOnlyMacToMeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = TTI_VALID_PHY_PORT_CNS;

    GT_BOOL     enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum,
                                                         &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call function with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, enablePtr = NULL", devNum, portNum);
        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum,
                                                         &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiMacModeSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call function with keyType[CPSS_DXCH_TTI_KEY_IPV4_E /
                                    CPSS_DXCH_TTI_KEY_MPLS_E /
                                     CPSS_DXCH_TTI_KEY_ETH_E /
                                     CPSS_DXCH_TTI_KEY_MIM_E],
                            macMode [CPSS_DXCH_TTI_MAC_MODE_DA_E /
                                     CPSS_DXCH_TTI_MAC_MODE_SA_E]
    Expected: GT_OK.
    1.2. Call cpssDxChTtiMacModeGet.
    Expected: GT_OK and same macMode.
    1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                  CPSS_DXCH_TTI_KEY_TRILL_E] and
                   other valid params.
    Expected: NOT GT_OK.
    1.4. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong macMode enum values and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType    = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT  macMode    = CPSS_DXCH_TTI_MAC_MODE_DA_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT  macModeGet = CPSS_DXCH_TTI_MAC_MODE_DA_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                             CPSS_DXCH_TTI_KEY_MPLS_E /
                                             CPSS_DXCH_TTI_KEY_ETH_E],
                                    macMode [CPSS_DXCH_TTI_MAC_MODE_DA_E /
                                             CPSS_DXCH_TTI_MAC_MODE_SA_E]
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

        st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

        /*
            1.2. Call cpssDxChTtiMacModeGet.
            Expected: GT_OK and same macMode.
        */
        st = cpssDxChTtiMacModeGet(devNum, keyType, &macModeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChTtiMacModeGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                   "got another macMode then was set: %d", devNum);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E]. */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

        st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

        /*
            1.2. Call cpssDxChTtiMacModeGet.
            Expected: GT_OK and same macMode.
        */
        st = cpssDxChTtiMacModeGet(devNum, keyType, &macModeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChTtiMacModeGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                   "got another macMode then was set: %d", devNum);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_ETH_E]. */
        keyType = CPSS_DXCH_TTI_KEY_ETH_E;
        macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

        st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

        /*
            1.2. Call cpssDxChTtiMacModeGet.
            Expected: GT_OK and same macMode.
        */
        st = cpssDxChTtiMacModeGet(devNum, keyType, &macModeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiMacModeGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
            "got another macMode then was set: %d", devNum);

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

            st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

            /*
                1.2. Call cpssDxChTtiMacModeGet.
                Expected: GT_OK and same macMode.
            */
            st = cpssDxChTtiMacModeGet(devNum, keyType, &macModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiMacModeGet: %d, %d", devNum, keyType);
            UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                "got another macMode then was set: %d", devNum);
        }

        /*
            1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                          CPSS_DXCH_TTI_KEY_TRILL_E] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

            st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);
        }
/* TBD TRILL */
#if 0
        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E] . */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

            st = cpssDxChTtiMacModeSet(devNum, keyType, macMode);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        }

        /*
            1.4. Call with wrong keyType enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiMacModeSet
                            (devNum, keyType, macMode),
                            keyType);

        /*
            1.5. Call with wrong macMode enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiMacModeSet
                            (devNum, keyType, macMode),
                            macMode);
#endif
    }

    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMacModeSet( devNum, keyType, macMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiMacModeGet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with keyType[CPSS_DXCH_TTI_KEY_IPV4_E /
                            CPSS_DXCH_TTI_KEY_MPLS_E /
                            CPSS_DXCH_TTI_KEY_MIM_E] and
                    other valid params.
    Expected: GT_OK.
    1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                  CPSS_DXCH_TTI_KEY_TRILL_E] and
                   other valid params.
    Expected: NOT GT_OK.
    1.3. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL macModePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                    CPSS_DXCH_TTI_KEY_MPLS_E /
                                    CPSS_DXCH_TTI_KEY_MIM_E] and
                           other valid params.
            Expected: GT_OK
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        st = cpssDxChTtiMacModeGet(devNum, keyType, &macMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E]. */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;

        st = cpssDxChTtiMacModeGet(devNum, keyType, &macMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;

            st = cpssDxChTtiMacModeGet(devNum, keyType, &macMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);
        }

        /*
            1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                          CPSS_DXCH_TTI_KEY_TRILL_E] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;

            st = cpssDxChTtiMacModeGet( devNum, keyType, &macMode);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &macMode);
        }
/* TBD TRILL */
#if 0
        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;

            st = cpssDxChTtiMacModeGet( devNum, keyType, &macMode);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &macMode);

            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        }

        /*
            1.3. Call with wrong keyType enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiMacModeGet
                            (devNum, keyType, &macMode),
                            keyType);

        /*
            1.4. Call with NULL macModePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiMacModeGet(devNum, keyType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macModePtr = NULL", devNum);
#endif
    }

    keyType = CPSS_DXCH_TTI_KEY_MPLS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiMacModeGet(devNum, keyType, &macMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMacModeGet(devNum, keyType, &macMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiExceptionCmdSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                  CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E /
                                  CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E /
                                  CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E],
                         command [CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                  CPSS_PACKET_CMD_DROP_HARD_E /
                                  CPSS_PACKET_CMD_FORWARD_E /
                                  CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.2. Call function cpssDxChTtiExceptionCmdGet.
    Expected: GT_OK and the same command.
    1.3. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E],
                   wrong command [CPSS_PACKET_CMD_FORWARD_E] and
                   others valid params.
    Expected: NOT GT_OK.
    1.4. Call with wrong exceptionType enum values and others valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong command [CPSS_PACKET_CMD_BRIDGE_E] and
                   others valid params.
    Expected: NOT GT_OK.
    1.6. Call with wrong command enum values and others valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   devNum;

    CPSS_DXCH_TTI_EXCEPTION_ENT             exceptionType;
    CPSS_PACKET_CMD_ENT                     command;
    CPSS_PACKET_CMD_ENT                     commandGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                          CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E /
                                          CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E /
                                          CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E],
                                 command [CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                          CPSS_PACKET_CMD_DROP_HARD_E /
                                          CPSS_PACKET_CMD_FORWARD_E /
                                          CPSS_PACKET_CMD_DROP_SOFT_E].
        */
        exceptionType   = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
        command         = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType, command);

        /*
            1.2. Call function cpssDxChTtiExceptionCmdGet.
            Expected: GT_OK and the same command.
        */
        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiExceptionCmdGet: %d, %d", devNum, exceptionType);
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
            "got another command then was set: %d", devNum);

        /* Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E]       */
        /*                 command [CPSS_PACKET_CMD_DROP_HARD_E].                           */
        exceptionType   = CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E;
        command       = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exceptionType, command);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType, command);
        }

        /*
            1.2. Call function cpssDxChTtiExceptionCmdGet.
            Expected: GT_OK and the same command.
        */
        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &commandGet);
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChTtiExceptionCmdGet: %d, %d", devNum, exceptionType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChTtiExceptionCmdGet: %d, %d", devNum, exceptionType);
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                                         "got another command then was set: %d", devNum);
        }

        /* Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E]  */
        /*                 command [CPSS_PACKET_CMD_FORWARD_E].                             */
        exceptionType   = CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E;
        command         = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType, command);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exceptionType, command);
        }

        /*
            1.2. Call function cpssDxChTtiExceptionCmdGet.
            Expected: GT_OK and the same command.
        */
        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &commandGet);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChTtiExceptionCmdGet: %d, %d", devNum, exceptionType);
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                                         "got another command then was set: %d", devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                         "cpssDxChTtiExceptionCmdGet: %d, %d", devNum, exceptionType);
        }

        /*
            1.3. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E],
                           wrong command [CPSS_PACKET_CMD_FORWARD_E] and
                           others valid params.
            Expected: NOT GT_OK.
        */
        exceptionType   = CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E;
        command         = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
        if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, command = %d", devNum, command);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command = %d", devNum, command);
        }

        /* Restore valid params. */
        command         = CPSS_PACKET_CMD_DROP_SOFT_E;

        /*
            1.6. Call with wrong command enum values and others valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiExceptionCmdSet
                            (devNum, exceptionType, command),
                            command);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    command       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiExceptionCmdGet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                  CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E /
                                  CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E /
                                  CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E /
                                  CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E] and
                   other valid params.
    Expected: GT_OK.
    1.2. Call with wrong exceptionType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL commandPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    CPSS_DXCH_TTI_EXCEPTION_ENT exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    CPSS_PACKET_CMD_ENT         command       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                          CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E /
                                          CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E /
                                          CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E /
                                          CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E] and
                           other valid params.
            Expected: GT_OK.
        */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, exceptionType);

        /* Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E].    */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E;

        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, exceptionType);

        /* Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E].  */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            exceptionType   = CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E;

            st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, exceptionType);
        }

        /* Call with exceptionType [CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E]. */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            exceptionType   = CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E;

            st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, exceptionType);
        }

        /* added a validity check on devNum to avoid coverity warnings */
        if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            st = utfFailureMsgLog("Out of range devNum.\n", NULL, 0);
            return;
        }

        /*
            1.2. Call with wrong exceptionType enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiExceptionCmdGet
                            (devNum, exceptionType, &command),
                            exceptionType);

        /*
            1.3. Call with NULL commandPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, commandPtr = NULL", devNum);
    }

    exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortMimOnlyMacToMeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortMimOnlyMacToMeEnableGet function.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL     enable;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare physical port iterator */
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;
            st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            enable = GT_TRUE;
            st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChTtiPortMimOnlyMacToMeEnableGet function.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", devNum);
        }

        portNum = 0;
        enable = GT_TRUE;

        /* prepare physical port iterator */
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. Go over non configured ports */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non configured port.   */
            /* Expected: GT_BAD_PARAM                               */
            st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, enable);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. Go over not active devices    */
    /* Expected: GT_BAD_PARAM           */
    portNum = 0;
    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id    */
    /* Expected: GT_BAD_PARAM                                   */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortMimOnlyMacToMeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat, Lion, xCat2, Lion2)
    1.1.1. Call function with correct enable pointer.
    Expected: GT_OK.
    1.1.2. Call function with incorrect enable pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare physical port iterator */
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1. Call function with correct enable pointer.
            Expected: GT_OK. */

            st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*1.1.2. Call function with incorrect enable pointer [NULL].
            Expected: GT_BAD_PTR. */

            st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, enable);
        }

        portNum = 0;

        /* prepare physical port iterator */
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3. Go over non configured ports */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.3.1. Call function for each non configured port.   */
            /* Expected: GT_BAD_PARAM                               */
            st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, enable);

        /* 1.4. For active device check that function returns GT_OK         */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. Go over not active devices    */
    /* Expected: GT_BAD_PARAM           */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id    */
    /* Expected: GT_BAD_PARAM                                   */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPclIdSet)
{
/*
    ITERATE_DEVICES (xCat, Lion, xCat2, Lion2)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                     CPSS_DXCH_TTI_KEY_MPLS_E /
                                     CPSS_DXCH_TTI_KEY_ETH_E  /
                                     CPSS_DXCH_TTI_KEY_MIM_E /
                                     CPSS_DXCH_TTI_KEY_TRILL_E],
                          and pclId [0 / BIT_5 / max].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiPclIdGet.
    Expected: GT_OK and the same pclId.
    1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                  CPSS_DXCH_TTI_KEY_TRILL_E] and
                   ther valid params.
    Expected: NOT GT_OK.
    1.4. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range pclId and other valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;

    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    GT_U32                              pclId;
    GT_U32                              pclIdGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                             CPSS_DXCH_TTI_KEY_MPLS_E /
                                             CPSS_DXCH_TTI_KEY_ETH_E  /
                                             CPSS_DXCH_TTI_KEY_MIM_E /
                                             CPSS_DXCH_TTI_KEY_TRILL_E ],
                                  and pclId [0 / BIT_5 / max].
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        pclId   = 0;

        st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);

        /*
            1.2. Call cpssDxChTtiPclIdGet.
            Expected: GT_OK and the same pclId.
        */
        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclIdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPclIdGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(pclId, pclIdGet,
            "got another pclId then was set: %d", devNum);


        /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E] and     */
        /*             pclId [BIT_5].                               */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        pclId   = BIT_5;

        st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);

        /*
            1.2. Call cpssDxChTtiPclIdGet.
            Expected: GT_OK and the same pclId.
        */
        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclIdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPclIdGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(pclId, pclIdGet,
            "got another pclId then was set: %d", devNum);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_ETH_E] and      */
        /*             pclId [max].                      */
        keyType = CPSS_DXCH_TTI_KEY_ETH_E;
        pclId   = BIT_10 - 1;

        st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);

        /*
            1.2. Call cpssDxChTtiPclIdGet.
            Expected: GT_OK and the same pclId.
        */
        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclIdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPclIdGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(pclId, pclIdGet,
            "got another pclId then was set: %d", devNum);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E] and      */
        /*             pclId [0].                      */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;
            pclId   = 0;

            st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);
        }
/* TBD TRILL */
#if 0
        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E] and    */
        /*             pclId [BIT_5].                               */
        if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
           (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
            pclId   = BIT_5;

            st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);
        }

        /*
            1.2. Call cpssDxChTtiPclIdGet.
            Expected: GT_OK and the same pclId.
        */
        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclIdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPclIdGet: %d, %d", devNum, keyType);
        UTF_VERIFY_EQUAL1_STRING_MAC(pclId, pclIdGet,
            "got another pclId then was set: %d", devNum);

        /*
            1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                          CPSS_DXCH_TTI_KEY_TRILL_E] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;
            pclId   = 0;

            st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);
        }

        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
            pclId   = 0;

            st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);
        }
#endif
        /* Restore valid params. */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        /*
            1.4. Call with wrong keyType enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        pclId   = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPclIdSet
                            (devNum, keyType, pclId),
                            keyType);

        /*
            1.5. Call with out of range pclId and other valid params.
            Expected: NOT GT_OK.
        */
        pclId = BIT_10;

        st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, pclId);

        /* Restore valid params. */
        pclId   = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    pclId   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPclIdGet)
{
/*
    ITERATE_DEVICES (xCat, Lion, xCat2, Lion2)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                     CPSS_DXCH_TTI_KEY_MPLS_E /
                                     CPSS_DXCH_TTI_KEY_ETH_E  /
                                     CPSS_DXCH_TTI_KEY_MIM_E /
                                     CPSS_DXCH_TTI_KEY_TRILL_E] and
                            other valid params.
    Expected: GT_OK.
    1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                  CPSS_DXCH_TTI_KEY_TRILL_E] and
                   ther valid params.
    Expected: NOT GT_OK.
    1.3. Call with wrong keyType enum values and other vslid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong NULL pclIdPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType;
    GT_U32                     pclId;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                             CPSS_DXCH_TTI_KEY_MPLS_E /
                                             CPSS_DXCH_TTI_KEY_ETH_E /
                                             CPSS_DXCH_TTI_KEY_MIM_E /
                                             CPSS_DXCH_TTI_KEY_TRILL_E] and
                                    other valid params.
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E].*/
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;

        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_ETH_E]. */
        keyType = CPSS_DXCH_TTI_KEY_ETH_E;

        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);

        /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;

            st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);
        }

        /* added a validity check on devNum to avoid coverity warnings */
        if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            st = utfFailureMsgLog("Out of range devNum.\n", NULL, 0);
            return;
        }
/* TBD TRILL */
#if 0
        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
        if ((((PRV_CPSS_SIP_5_CHECK_MAC(devNum))) &&
           (GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;

            st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);
        }

        /*
            1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                          CPSS_DXCH_TTI_KEY_TRILL_E] and
                                 other valid params.
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_MIM_E;

            st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);
        }

        /* Call with keyType [CPSS_DXCH_TTI_KEY_TRILL_E]. */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;

            st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, &pclId);
        }

        /* Restore valid params. */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        /*
            1.3. Call with wrong keyType enum values and other vslid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPclIdGet
                            (devNum, keyType, &pclId),
                            keyType);

        /*
            1.4. Call with wrong NULL pclIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChTtiPclIdGet(devNum, keyType, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType, &pclId);
#endif
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPclIdGet(devNum, keyType, &pclId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill table MacToMe
*/
UTF_TEST_CASE_MAC(cpssDxChTtiFillMacToMeTable)
{
/*
    ITERATE_DEVICE (DxCh3; xCat; xCat3; Lion; xCat2)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in MacToMe table.
         Call cpssDxChTtiMacToMeSet with entryIndex [0... numEntries-1],
                                    valuePtr {mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                              vlanId [100]},
                                    maskPtr {mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                              vlanId [0x0FFF]}
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChTtiMacToMeSet with entryIndex [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in MacToMe table and compare with original.
         Call function cpssDxChTtiMacToMeGet with not-NULL valuePtr,
                                                  not-NULL maskPtr
                                                  and other parameter from 1.1.
    Expected: GT_OK and value, mask the same as was set.
    1.5. Try to read entry with index out of range.
         Call function cpssDxChTtiMacToMeGet with entryIndex [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;
    GT_BOOL     isEqual    = GT_FALSE;

    CPSS_DXCH_TTI_MAC_VLAN_STC  value;
    CPSS_DXCH_TTI_MAC_VLAN_STC  mask;
    CPSS_DXCH_TTI_MAC_VLAN_STC  valueGet;
    CPSS_DXCH_TTI_MAC_VLAN_STC  maskGet;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfoGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &valueGet, sizeof(valueGet));
    cpssOsBzero((GT_VOID*) &maskGet, sizeof(maskGet));
    cpssOsBzero((GT_VOID*) &value, sizeof(value));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*) &interfaceInfo, sizeof(interfaceInfo));
    cpssOsBzero((GT_VOID*) &interfaceInfoGet, sizeof(interfaceInfoGet));

    /* Fill the entry for MacToMe table */
    value.mac.arEther[0] = 0xAA;
    value.mac.arEther[1] = 0xAA;
    value.mac.arEther[2] = 0xAA;
    value.mac.arEther[3] = 0xAA;
    value.mac.arEther[4] = 0xAA;
    value.mac.arEther[5] = 0xAA;

    value.vlanId = 100;

    mask.mac.arEther[0] = 0xFF;
    mask.mac.arEther[1] = 0xFF;
    mask.mac.arEther[2] = 0xFF;
    mask.mac.arEther[3] = 0xFF;
    mask.mac.arEther[4] = 0xFF;
    mask.mac.arEther[5] = 0xFF;

    mask.vlanId = 0x0FFF;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Get MacToMe Size */
        st = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", devNum);

        /* 1.2. Fill all entries in MacToMe table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* make every entry unique */
            value.mac.arEther[2] = (GT_U8)(iTemp % 255);
            value.mac.arEther[3] = (GT_U8)(iTemp % 255);
            value.mac.arEther[4] = (GT_U8)(iTemp % 255);
            value.mac.arEther[5] = (GT_U8)(iTemp % 255);
            value.vlanId         = (GT_U16)(iTemp % 4095);

            interfaceInfo.includeSrcInterface = 0;

            st = cpssDxChTtiMacToMeSet(devNum, iTemp, &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTtiMacToMeSet: %d, %d", devNum, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChTtiMacToMeSet(devNum, numEntries, &value, &mask, &interfaceInfo);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTtiMacToMeSet: %d, %d", devNum, numEntries);

        /* 1.4. Read all entries in MacToMe table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* restore unique entry before compare */
            value.mac.arEther[2] = (GT_U8)(iTemp % 255);
            value.mac.arEther[3] = (GT_U8)(iTemp % 255);
            value.mac.arEther[4] = (GT_U8)(iTemp % 255);
            value.mac.arEther[5] = (GT_U8)(iTemp % 255);
            value.vlanId         = (GT_U16)(iTemp % 4095);

            st = cpssDxChTtiMacToMeGet(devNum, iTemp, &valueGet, &maskGet, &interfaceInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTtiMacToMeGet: %d, %d", devNum, iTemp);

            /* validation values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value,
                                         (GT_VOID*) &valueGet,
                                         sizeof(value))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "got another value then was set: %d", devNum);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask,
                                         (GT_VOID*) &maskGet,
                                         sizeof(mask))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "got another mask then was set: %d", devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(interfaceInfoGet.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfoGet.includeSrcInterface than was set: %d", devNum);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChTtiMacToMeGet(devNum, numEntries, &valueGet, &maskGet, &interfaceInfoGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTtiMacToMeGet: %d, %d", devNum, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupMacToMeSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with entryIndex [0 / 3 / max],
                         value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                             vlanId [TTI_VALID_VLAN_ID_CNS, 1000, max]
                               },
                          mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                             vlanId [0x0FFF]
                               },
                 interfaceInfo {
                                includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                     CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E /
                                                     CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                          srcDevice [devNum],
                                         srcIsTrunk [GT_FALSE / GT_TRUE],
                                       srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                               }.
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
    Expected: GT_OK and the same value, mask and interfaceInfo.
    1.1.3. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.

    1.1.4. Call with out of range value.vlanId, mask.vlanId [0] and
                     other valid params.
    Expected: NOT GT_OK.
    1.1.5. Call with out of range value.vlanId, mask.vlanId [0x0FFF] and
                     other valid params.
    Expected: NOT GT_OK.
    1.1.6. Call with wrong interfaceInfo.includeSrcInterface enum values and
                     other valid params.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                        CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                   out of range interfaceInfo.srcDevice (not relevant) and
                   other valid params.
    Expected: GT_OK.
    1.1.8. Call with out of range interfaceInfo.srcDevice and other valid params.
    Expected: NOT GT_OK.
    1.1.9. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],
                     out of range interfaceInfo.srcPortTrunk (not relevant) and
                     other valid params.
    Expected: GT_OK.
    1.1.10. Call with out of range interfaceInfo.srcPortTrunk and other valid params.
    Expected: NOT GT_OK.
    1.1.11. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.12. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.13. Call with NULL interfaceInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st = GT_OK;

    GT_U8                                           devNum;
    GT_PORT_GROUPS_BMP                              portGroupsBmp;
    GT_U32                                          portGroupId;
    GT_U32                                          entryIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      value;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      valueGet;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      mask;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      maskGet;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfoGet;
    GT_BOOL                                         isEqual = GT_FALSE;

    cpssOsBzero((GT_VOID*) &value,                  sizeof(value));
    cpssOsBzero((GT_VOID*) &valueGet,               sizeof(valueGet));
    cpssOsBzero((GT_VOID*) &mask,                   sizeof(mask));
    cpssOsBzero((GT_VOID*) &maskGet,                sizeof(maskGet));
    cpssOsBzero((GT_VOID*) &interfaceInfo,          sizeof(interfaceInfo));
    cpssOsBzero((GT_VOID*) &interfaceInfoGet,       sizeof(interfaceInfoGet));


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Initialize port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with entryIndex [0 / 3 / max],
                                     value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                         vlanId [TTI_VALID_VLAN_ID_CNS, 1000, max]
                                           },
                                      mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                         vlanId [0x0FFF]
                                           },
                             interfaceInfo {
                                            includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                                 CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E /
                                                                 CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                                      srcDevice [devNum]
                                                     srcIsTrunk [GT_FALSE / GT_TRUE],
                                                   srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                                           }.
                Expected: GT_OK.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /*
                1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                Expected: GT_OK and the same value, mask and interfaceInfo.
            */
            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &valueGet, &maskGet, &interfaceInfoGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                         (GT_VOID*) &valueGet.mac,
                                         sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                       "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                         (GT_VOID*) &maskGet.mac,
                                         sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

            UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                "get another interfaceInfo.includeSrcInterface than was set: %d, %d", devNum, portGroupsBmp, entryIndex);

            /* Call with entryIndex [3],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [1000]
                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                               srcDevice [0],
                                              srcIsTrunk [GT_FALSE],
                                            srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                    }.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = 1000;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = 0;
            interfaceInfo.srcIsTrunk            = GT_FALSE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                             (GT_VOID*) &valueGet.mac,
                                             sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                    "get another interfaceInfo.srcHwDevice than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with entryIndex [max],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [max],

                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                               srcDevice [0],
                                              srcIsTrunk [GT_TRUE],
                                            srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                    }.
            */
            entryIndex                          = 7;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = 0;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                             (GT_VOID*) &valueGet.mac,
                                             sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                    "get another interfaceInfo.srcHwDevice than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with entryIndex [0],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [TTI_VALID_VLAN_ID_CNS]
                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                               srcDevice [0],
                                              srcIsTrunk [GT_FALSE],
                                            srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS]
                                    }
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = 0;
            interfaceInfo.srcIsTrunk            = GT_FALSE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                             (GT_VOID*) &valueGet.mac,
                                             sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                    "get another interfaceInfo.srcHwDevice than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with entryIndex [3],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [1000]
                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E],
                                               srcHwDevice [0],
                                              srcIsTrunk [GT_TRUE],
                                            srcPortTrunk [max/2]
                                    }
            */
            entryIndex                          = 3;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = 1000;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = 0;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1) / 2;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                             (GT_VOID*) &valueGet.mac,
                                             sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcHwDevice, interfaceInfoGet.srcHwDevice,
                    "get another interfaceInfo.srcHwDevice than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with entryIndex [max],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [max]
                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                              srcIsTrunk [GT_FALSE],
                                            srcPortTrunk [max]
                                    }
            */
            entryIndex                          = 7;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
            interfaceInfo.srcIsTrunk            = GT_FALSE;
            interfaceInfo.srcPortTrunk          = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15);

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                            (GT_VOID*) &valueGet.mac,
                                            sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with entryIndex [0],
                              value {                mac [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                                  vlanId [TTI_VALID_VLAN_ID_CNS]
                                    },
                               mask {                mac [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]],
                                                  vlanId [0x0FFF]
                                    },
                      interfaceInfo {
                                     includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                              srcIsTrunk [GT_TRUE],
                                            srcPortTrunk [max]
                                    }
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacToMeGet.
                    Expected: GT_OK and the same value, mask and interfaceInfo.
                */
                st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                    &valueGet, &maskGet, &interfaceInfoGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacToMeGet: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.mac,
                                             (GT_VOID*) &valueGet.mac,
                                             sizeof(value.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(value.vlanId, valueGet.vlanId,
                    "get another value.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask.mac,
                                             (GT_VOID*) &maskGet.mac,
                                             sizeof(mask.mac))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another mask.mac then was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(mask.vlanId, maskGet.vlanId,
                    "get another mask.vlanId than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);

                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.includeSrcInterface, interfaceInfoGet.includeSrcInterface,
                    "get another interfaceInfo.includeSrcInterface than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcIsTrunk, interfaceInfoGet.srcIsTrunk,
                    "get another interfaceInfo.srcIsTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
                UTF_VERIFY_EQUAL3_STRING_MAC(interfaceInfo.srcPortTrunk, interfaceInfoGet.srcPortTrunk,
                    "get another interfaceInfo.srcPortTrunk than was set: %d, %d, %d", devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /*
                1.1.3. Call with out of range entryIndex and other valid params.
                Expected: NOT GT_OK.
            */
            if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                entryIndex = 128;
            }
            else
            {
                entryIndex = 8;
            }

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

            /* Restore valid params. */
            entryIndex = 0;

            /*
                1.1.4. Call with out of range value.vlanId, mask.vlanId [0] and
                                 other valid params.
                Expected: NOT GT_OK.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                value.vlanId                        = BIT_13;
            }
            else
            {
                value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS;
            }

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /* Restore correct values. */
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;
            mask.vlanId                         = 0x0FFF;

            /*
                1.1.5. Call with out of range value.vlanId, mask.vlanId [0x0FFF] and
                                 other valid params.
                Expected: NOT GT_OK.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                value.vlanId                        = BIT_13;
            }
            else
            {
                value.vlanId                        = PRV_CPSS_MAX_NUM_VLANS_CNS;
            }

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);

            /* Restore valid params. */
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;
            mask.vlanId                         = 0x0FFF;

            /*
                1.1.6. Call with wrong interfaceInfo.includeSrcInterface enum values and
                                 other valid params.
                Expected: GT_BAD_PARAM.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.srcHwDevice             = 0;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupMacToMeSet
                                    (devNum, portGroupsBmp, entryIndex, &value, &mask, &interfaceInfo),
                                     interfaceInfo.includeSrcInterface);
            }

            /* Restore valid params. */
            interfaceInfo.includeSrcInterface   = 0;

            /*
                1.1.7. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E /
                                                                    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],
                                 out of range interfaceInfo.srcDevice (not relevant) and
                                 other valid params.
                Expected: GT_OK.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                /* the device ignore the interfaceInfo.srcDevice when
                    interfaceInfo.includeSrcInterface  == CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E],   */
            /*           out of range interfaceInfo.srcDevice (not relevant) and                                            */
            /*           other valid params.                                                                                */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
            interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /*
                1.1.8. Call with out of range interfaceInfo.srcDevice and
                                 other valid params.
                Expected: NOT GT_OK.
            */
            entryIndex                          = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                                entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Restore valid params. */
            interfaceInfo.srcHwDevice             = devNum;

            /*
                1.1.9. Call with interfaceInfo.includeSrcInterface [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],
                                 out of range interfaceInfo.srcPortTrunk (not relevant) and
                                 other valid params.
                Expected: GT_OK.
            */
            entryIndex = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = devNum;
            interfaceInfo.srcIsTrunk            = GT_FALSE;
            interfaceInfo.srcPortTrunk          = (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15)) + 1;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /* Calling with value.srcIsTrunk [CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E],   */
            /*              value.srcPortTrunk [max],                                           */
            /*              out of range interfaceInfo.srcPortTrunk (not relevant) and          */
            /*              others valid params.                                                */
            entryIndex = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = devNum;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /*
                1.1.10. Call with out of range value.srcPortTrunk and other valid params.
                Expected: NOT GT_OK.
            */
            entryIndex = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = devNum;
            interfaceInfo.srcIsTrunk            = GT_FALSE;
            interfaceInfo.srcPortTrunk          = (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15)) + 1;

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Calling with value.srcIsTrunk [1], value.srcPortTrunk [max], and     */
            /* others valid params.                                                 */
            entryIndex = 0;

            value.mac.arEther[0]                = 0xAA;
            value.mac.arEther[1]                = 0xAA;
            value.mac.arEther[2]                = 0xAA;
            value.mac.arEther[3]                = 0xAA;
            value.mac.arEther[4]                = 0xAA;
            value.mac.arEther[5]                = 0xAA;
            value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

            cpssOsMemSet(&mask, 0xFF, sizeof(mask));
            mask.vlanId                         = 0x0FFF;

            interfaceInfo.includeSrcInterface   = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
            interfaceInfo.srcHwDevice             = devNum;
            interfaceInfo.srcIsTrunk            = GT_TRUE;
            interfaceInfo.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                            entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }

            /* Restore valid params. */
            interfaceInfo.srcPortTrunk            = TTI_VALID_PORT_TRUNK_CNS;

            /*
                1.1.11. Call with NULL valuePtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                NULL, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                         devNum, portGroupsBmp, entryIndex);

            /*
                1.1.12. Call with NULL maskPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, NULL, &interfaceInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, maskPtr = NULL",
                                         devNum, portGroupsBmp, entryIndex);

            /*
                1.1.13. Call with NULL interfaceInfoPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, NULL);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, interfaceInfoPtr = NULL",
                                             devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* Restore port group. */
        portGroupId = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next non-active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp, entryIndex);

        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp                       = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        value.vlanId                        = TTI_VALID_VLAN_ID_CNS;
        interfaceInfo.includeSrcInterface   = 0;

        st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portGroupsBmp                       = 1;

    entryIndex                          = 0;

    value.mac.arEther[0]                = 0xAA;
    value.mac.arEther[1]                = 0xAA;
    value.mac.arEther[2]                = 0xAA;
    value.mac.arEther[3]                = 0xAA;
    value.mac.arEther[4]                = 0xAA;
    value.mac.arEther[5]                = 0xAA;
    value.vlanId                        = TTI_VALID_VLAN_ID_CNS;

    cpssOsMemSet(&mask, 0xFF, sizeof(mask));
    mask.vlanId                         = 0x0FFF;

    interfaceInfo.includeSrcInterface   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                            &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                        &value, &mask, &interfaceInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupMacToMeGet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with entryIndex [0 / 7] and other valid params.
    Expected: GT_OK.
    1.1.2. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.
    1.1.3. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.4. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.5. Call with NULL interfaceInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;

    GT_U8                                               devNum;
    GT_U32                                              portGroupId;
    GT_PORT_GROUPS_BMP                                  portGroupsBmp;
    GT_U32                                              entryIndex;
    CPSS_DXCH_TTI_MAC_VLAN_STC                          value;
    CPSS_DXCH_TTI_MAC_VLAN_STC                          mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC      interfaceInfo;

    cpssOsBzero((GT_VOID*) &value, sizeof(value));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*)&interfaceInfo, sizeof(interfaceInfo));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Restore port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next non-active port group. */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with entryIndex [0 / 7] and other valid params.
                Expected: GT_OK.
            */
            /* iterate with entryIndex = 0 */
            entryIndex = 0;

            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /* iterate with entryIndex = 7 */
            entryIndex = 7;

            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                               &value, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                        entryIndex);

            /*
                1.1.2. Call with out of range entryIndex and other valid params.
                Expected: NOT GT_OK.
            */
            if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                entryIndex = 128;
            }
            else
            {
                entryIndex = 8;
            }

            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);

            /* Restore valid params. */
            entryIndex = 0;

            /*
                1.1.3. Call with NULL valuePtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                NULL, &mask, &interfaceInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                         devNum, portGroupsBmp, entryIndex);

            /*
                1.1.4. Call with NULL maskPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &value, NULL, &interfaceInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, maskPtr = NULL",
                                         devNum, portGroupsBmp, entryIndex);

            /*
                1.1.5. Call with NULL interfaceInfoPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, NULL);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, &interfaceInfo = NULL",
                                             devNum, portGroupsBmp, entryIndex);
            }
            else
            {
                /* this device ignore the interfaceInfoPtr parameters */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, entryIndex);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* Restore port group. */
        portGroupId = 1;

        entryIndex = 0;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next non-active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                                &value, &mask, &interfaceInfo);

            if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                            &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    portGroupsBmp = 1;

    entryIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                             &value, &mask, &interfaceInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex,
                                        &value, &mask, &interfaceInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupMacModeSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupMacModeSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                    CPSS_DXCH_TTI_KEY_MPLS_E /
                              CPSS_DXCH_TTI_KEY_ETH_E /
                              CPSS_DXCH_TTI_KEY_MIM_E],
                     macMode [CPSS_DXCH_TTI_MAC_MODE_DA_E /
                              CPSS_DXCH_TTI_MAC_MODE_SA_E] and
                     other valid params.
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortGroupMacModeGet.
    Expected: GT_OK and same macMode.
    1.1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                    CPSS_DXCH_TTI_KEY_TRILL_E] and
                     other valid params.
    Expected: NOT GT_OK.
    1.1.4. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with wrong macMode enum values and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   devNum;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId   = 0;

    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType    = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT  macMode    = CPSS_DXCH_TTI_MAC_MODE_DA_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT  macModeGet = CPSS_DXCH_TTI_MAC_MODE_DA_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                          CPSS_DXCH_TTI_KEY_MPLS_E /
                                          CPSS_DXCH_TTI_KEY_ETH_E /
                                          CPSS_DXCH_TTI_KEY_MIM_E],
                                 macMode [CPSS_DXCH_TTI_MAC_MODE_DA_E /
                                          CPSS_DXCH_TTI_MAC_MODE_SA_E] and
                     other valid params.
                Expected: GT_OK.
            */

            /* iterate with keyType = CPSS_DXCH_TTI_KEY_IPV4_E */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

            st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

            /*
                1.1.2. Call cpssDxChTtiPortGroupMacModeGet.
                Expected: GT_OK and same macMode.
            */
            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortGroupMacModeGet: %d, %d", devNum, keyType);
            UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                "got another macMode then was set: %d", devNum);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E] and */
            /*           macMode [CPSS_DXCH_TTI_MAC_MODE_SA_E]. */
            keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

            st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

            /*
                1.1.2. Call cpssDxChTtiPortGroupMacModeGet.
                Expected: GT_OK and same macMode.
            */
            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortGroupMacModeGet: %d, %d", devNum, keyType);
            UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                "got another macMode then was set: %d", devNum);

            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E] and */
                /*           macMode [CPSS_DXCH_TTI_MAC_MODE_DA_E]. */
                keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
                macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

                st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType,
                                                    macMode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, keyType, macMode);

                /*
                    1.1.2. Call cpssDxChTtiPortGroupMacModeGet.
                    Expected: GT_OK and same macMode.
                */
                st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType,
                                                    &macModeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiPortGroupMacModeGet: %d, %d", devNum, keyType);
                UTF_VERIFY_EQUAL1_STRING_MAC(macMode, macModeGet,
                    "got another macMode then was set: %d", devNum);
            }

            /*
                1.1.3. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                                CPSS_DXCH_TTI_KEY_TRILL_E] and
                                 other valid params.
                Expected: NOT GT_OK.
            */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;
                macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

                st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp,
                                                    keyType, macMode);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp,
                                                keyType, &macMode);
            }
/* TBD TRILL */
#if 0
            /* Call with [CPSS_DXCH_TTI_KEY_TRILL_E]. */
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;
            macMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;

            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, keyType, &macMode);
#endif
            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            /*
                1.1.4. Call with wrong keyType enum values and
                                other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupMacModeSet
                                (devNum, portGroupsBmp, keyType, macMode),
                                keyType);

            /*
                1.1.5. Call with wrong macMode enum values and
                                 other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupMacModeSet
                                (devNum, portGroupsBmp, keyType, macMode),
                                macMode);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);

            if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    portGroupsBmp = 1;

    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupMacModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupMacModeGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1.1. Call with keyType[CPSS_DXCH_TTI_KEY_IPV4_E /
                              CPSS_DXCH_TTI_KEY_MPLS_E /
                              CPSS_DXCH_TTI_KEY_MIM_E].
    Expected: GT_OK.
    1.1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                    CPSS_DXCH_TTI_KEY_TRILL_E] and
                   other valid params.
    Expected: NOT GT_OK.
    1.1.3. Call with wrong keyType enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with NULL macModePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   devNum;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId   = 0;

    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT  macMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with keyType [CPSS_DXCH_TTI_KEY_IPV4_E /
                                          CPSS_DXCH_TTI_KEY_MPLS_E].
                Expected: GT_OK.
            */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* Call with keyType [CPSS_DXCH_TTI_KEY_MPLS_E]. */
            keyType = CPSS_DXCH_TTI_KEY_MPLS_E;

            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                /* Call with keyType [CPSS_DXCH_TTI_KEY_MIM_E]. */
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;

                st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);
            }

            /*
                1.1.2. Call with wrong keyType [CPSS_DXCH_TTI_KEY_MIM_E /
                                                CPSS_DXCH_TTI_KEY_TRILL_E] and
                                 other valid params.
                Expected: NOT GT_OK.
            */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
               (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                keyType = CPSS_DXCH_TTI_KEY_MIM_E;

                st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, keyType, &macMode);
            }
/* TBD TRILL */
#if 0

            /* Call with [CPSS_DXCH_TTI_KEY_TRILL_E]. */
            keyType = CPSS_DXCH_TTI_KEY_TRILL_E;

            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp, keyType, &macMode);
#endif
            /* Restore valid params. */
            keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

            /*
                1.1.3. Call with wrong keyType enum values and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupMacModeGet
                                (devNum, portGroupsBmp, keyType, &macMode),
                                keyType);

            /*
                1.1.4. Call with NULL macModePtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macModePtr = NULL", devNum);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);

            if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    portGroupsBmp = 1;

    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, &macMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortTrillEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortTrillEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiPortTrillEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL             enable;
    GT_BOOL             enableGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

            /*
                1.1.2. Call cpssDxChTtiPortTrillEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortTrillEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);

            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

            /*
                1.1.2. Call cpssDxChTtiPortTrillEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortTrillEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);
        }

        portNum = 0;
        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortTrillEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortTrillEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_BOOL             enable;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &enable);

            /*
                1.2. Call function with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "%d, enablePtr = NULL", devNum);
        }

        portNum = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortTrillEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortTrillOuterVid0Set
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U16           vlanId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortTrillOuterVid0Set)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with vlanId [TTI_VALID_VLAN_ID_CNS / 1000 / max].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortTrillOuterVid0Get.
    Expected: GT_OK and the same vlanId.
    1.1.3 Call with out of range vlanId and other valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_U16              vlanId;
    GT_U16              vlanIdGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with vlanId [TTI_VALID_VLAN_ID_CNS / 1000 / max].
                Expected: GT_OK.
            */
            vlanId = TTI_VALID_VLAN_ID_CNS;

            st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, vlanId);

            /*
                1.1.2. Call cpssDxChTtiPortTrillOuterVid0Get.
                Expected: GT_OK and the same vlanId.
            */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortTrillOuterVid0Get: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                "get another vlanId than was set: %d", devNum);

            /* Call with vlanId [1000]. */
            vlanId = 1000;

            st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, vlanId);

            /*
                1.1.2. Call cpssDxChTtiPortTrillOuterVid0Get.
                Expected: GT_OK and the same vlanId.
            */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortTrillOuterVid0Get: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                "get another vlanId than was set: %d", devNum);

            /* Call with vlanId [max]. */
            vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

            st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, vlanId);

            /*
                1.1.2. Call cpssDxChTtiPortTrillOuterVid0Get.
                Expected: GT_OK and the same vlanId.
            */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortTrillOuterVid0Get: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                "get another vlanId than was set: %d", devNum);
            /*
                1.1.3 Call with out of range vlanId and other valid params.
                Expected: NOT GT_OK.
            */
            vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

            st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
            /* the device supports 12 bits for this vid */
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, vlanId);
        }

        vlanId = TTI_VALID_VLAN_ID_CNS;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        vlanId);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    vlanId = TTI_VALID_VLAN_ID_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16         *vlanIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortTrillOuterVid0Get)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call function with not NULL vlanIdPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL vlanIdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    GT_U16              vlanId;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1. Call function with not NULL vlanIdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &vlanId);

            /*
                1.2. Call function with NULL vlanIdPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, vlanIdPtr = NULL", devNum, portNum);
        }

        portNum = 0;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        vlanId);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, &vlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with enable[GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS st = GT_OK;

    GT_U8   devNum;
    GT_BOOL enable = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_LION_E |
                                              UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_TRUE / GT_FALSE],
            Expected: GT_OK.
        */
            enable = GT_TRUE;

        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum,
                                                                                 enable);
        if(PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                                 &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d", devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* Call with enable [GT_FALSE]. */
            enable = GT_FALSE;

        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum,
                                                                                 enable);
        if(PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                                 &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d", devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_LION_E |
                                              UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum,
                                                                                 enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

   /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum,
                                                                             enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet)
{
/*
    1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8    devNum;
    GT_BOOL  enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_LION_E |
                                              UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
            /*
            1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                                 &enable);
        if(PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

            /*
            1.2. Call api with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                                 NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "%d, enablePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E | UTF_CH3_E | UTF_LION_E |
                                              UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                                 &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum,
                                                                             &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              ethertype
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiEthernetTypeSet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E] and
                       ethertype [0, max/2, max].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiEthernetTypeGet.
    Expected: GT_OK and the same ethertype.
    1.3. Call with wrong ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E \
                                        CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].
    Expected: NOT GT_OK.
    1.4. Call with wrong ethertypeType enum values.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT          ethertypeType;
    GT_U32                                  ethertype;
    GT_U32                                  ethertypeGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E] and
                               ethertype [0, max/2, max].
            Expected: GT_OK.
        */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
        ethertype       = 0;

        st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);

        /*
            1.2. Call cpssDxChTtiEthernetTypeGet.
            Expected: GT_OK and the same ethertype.
        */
        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertypeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiEthernetTypeGet: %d, %d", devNum, ethertypeType);
        UTF_VERIFY_EQUAL2_STRING_MAC(ethertype, ethertypeGet,
            "get another ethertype than was set: %d, %d", devNum, ethertypeType);

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E] and    */
        /*               ethertype [max/2].                                             */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E;
        ethertype       = (BIT_16 - 1) / 2;

        st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, ethertypeType, ethertype);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);
        }

        /*
            1.2. Call cpssDxChTtiEthernetTypeGet.
            Expected: GT_OK and the same ethertype.
        */
        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertypeGet);
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                         "cpssDxChTtiEthernetTypeGet: %d", devNum, ethertypeType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChTtiEthernetTypeGet: %d", devNum, ethertypeType);
            UTF_VERIFY_EQUAL2_STRING_MAC(ethertype, ethertypeGet,
                                         "get another ethertype than was set: %d", devNum, ethertypeType);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E] and */
        /*               ethertype [max].                                       */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
        ethertype       = BIT_16 - 1;

        st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);

        /*
            1.2. Call cpssDxChTtiEthernetTypeGet.
            Expected: GT_OK and the same ethertype.
        */
        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertypeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiEthernetTypeGet: %d", devNum, ethertypeType);
        UTF_VERIFY_EQUAL2_STRING_MAC(ethertype, ethertypeGet,
            "get another ethertype than was set: %d", devNum, ethertypeType);

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E] and       */
        /*               ethertype [0].                                         */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;
            ethertype       = 0;

            st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);

            /*
                1.2. Call cpssDxChTtiEthernetTypeGet.
                Expected: GT_OK and the same ethertype.
            */
            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiEthernetTypeGet: %d, %d", devNum, ethertypeType);
            UTF_VERIFY_EQUAL2_STRING_MAC(ethertype, ethertypeGet,
                "get another ethertype than was set: %d, %d", devNum, ethertypeType);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E] and     */
        /*               ethertype [max/2].                                     */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;
            ethertype       = (BIT_16 - 1) / 2;

            st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);

            /*
                1.2. Call cpssDxChTtiEthernetTypeGet.
                Expected: GT_OK and the same ethertype.
            */
            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChTtiEthernetTypeGet: %d, %d", devNum, ethertypeType);
            UTF_VERIFY_EQUAL2_STRING_MAC(ethertype, ethertypeGet,
                "get another ethertype than was set: %d, %d", devNum, ethertypeType);
        }

        /*
            1.3. Call with wrong ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E \
                                                CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;
            ethertype       = 0;

            st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].    */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E && !PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;
            ethertype       = 0;

            st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, ethertype);
        }

        /* Restore valid param. */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
        ethertype       = 0;

        /*
            1.4. Call with wrong ethertypeType enum values.
            Expected: GT_BAD_PARAM.
        */
        ethertype       = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChTtiEthernetTypeSet
                            (devNum, ethertypeType, ethertype),
                            ethertypeType);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
    ethertype       = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                               UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound ethertype for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              *ethertypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiEthernetTypeGet)
{
/*
    ITERATE_DEVICES (DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E /
                                  CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E] and
                   other valid params.
    Expected: GT_OK.
    1.2. Call with NULL ethertypePtr.
    Expected: GT_BAD_PTR.
    1.3. Call with wrong ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E \
                                        CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].
    Expected: NOT GT_OK.
    1.4. Call with wrong ethertypeType enum values.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT          ethertypeType;
    GT_U32                                  ethertype;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E /
                                          CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E] and
                           other valid params.
            Expected: GT_OK.
        */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;

        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E] and    */
        /*           other valid params.                                                */
        ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E;

        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, ethertypeType, &ethertype);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E].  */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)  ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;

            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].  */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;

            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);
        }

        /*
            1.2. Call with NULL ethertypePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
            "%d, %d, ethertypePtr = NULL", devNum, ethertypeType);

        /*
            1.3. Call with wrong ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E \
                                                CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].
            Expected: NOT GT_OK.
        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)  &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
           (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;

            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);
        }

        /* Call with ethertypeType [CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E].    */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E && !PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;

            st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ethertypeType, &ethertype);
        }

        /*
            1.4. Call with wrong ethertypeType enum values.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiEthernetTypeGet
                            (devNum, ethertypeType, &ethertype),
                            ethertypeType);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    ethertypeType   = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                              UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound ethertype for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, &ethertype);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

#if 1
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillCpuCodeBaseSet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with cpuCodeBase [min / (min + max)/2 / max].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiTrillCpuCodeBaseGet.
    Expected: GT_OK and the same cpuCodeBase.
    1.3. Call with out of range cpuCodeBase.
    Expected: NOT GT_OK.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_U32              cpuCodeBase;
    GT_U32              cpuCodeBaseGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with cpuCodeBase [min / (min + max)/2 / max].
            Expected: GT_OK.
        */
        cpuCodeBase = CPU_CODE_BASE_MIN_CNS;

        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cpuCodeBase);

        /*
            1.2. Call cpssDxChTtiTrillCpuCodeBaseGet.
            Expected: GT_OK and the same cpuCodeBase.
        */
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillCpuCodeBaseGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
            "get another cpuCodeBase than was set: %d", devNum);

        /* Call with cpuCodeBase [(min + max)/2]. */
        cpuCodeBase = (CPU_CODE_BASE_MIN_CNS + CPU_CODE_BASE_MAX_CNS) / 2;

        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cpuCodeBase);

        /*
            1.2. Call cpssDxChTtiTrillCpuCodeBaseGet.
            Expected: GT_OK and the same cpuCodeBase.
        */
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillCpuCodeBaseGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
            "get another cpuCodeBase than was set: %d", devNum);

        /* Call with cpuCodeBase [max]. */
        cpuCodeBase = CPU_CODE_BASE_MAX_CNS;

        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cpuCodeBase);

        /*
            1.2. Call cpssDxChTtiTrillCpuCodeBaseGet.
            Expected: GT_OK and the same cpuCodeBase.
        */
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillCpuCodeBaseGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
            "get another cpuCodeBase than was set: %d", devNum);

        /*
            1.3. Call with out of range cpuCodeBase.
            Expected: NOT GT_OK.
        */
        cpuCodeBase = CPU_CODE_BASE_MAX_CNS + 1;

        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cpuCodeBase);

        /* Call with cpuCodeBase [CPU_CODE_BASE_MAX_CNS - ]. */
        cpuCodeBase = CPU_CODE_BASE_MIN_CNS - 1 > 0 ? CPU_CODE_BASE_MIN_CNS - 1:
                                                      0;

        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, cpuCodeBase);
    }

    cpuCodeBase = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound cpuCodeBase for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillCpuCodeBaseGet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call function with not NULL cpuCodeBasePtr.
    Expected: GT_OK.
    1.2. Call function with NULL cpuCodeBasePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_U32              cpuCodeBase;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL cpuCodeBasePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &cpuCodeBase);

        /*
            1.2. Call function with NULL cpuCodeBasePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "%d, cpuCodeBasePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound cpuCodeBase for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillCpuCodeBaseGet(devNum, &cpuCodeBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillAdjacencyCheckEntrySet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with entryIndex [0 / BIT_4 / max],
                        value {      outerMacSa [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                      trillMBit [0 / 1],
                                trillEgressRbid [0 / max/2 / max],
                                      srcDevice [devNum],
                                     srcIsTrunk [0 / 1],
                                   srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                              },
                         mask {      outerMacSa [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]]
                                      trillMBit [0x00000001],
                                trillEgressRbid [0x000FFFFF],
                                      srcDevice [0x00000FFF],
                                     srcIsTrunk [0x00000001],
                                   srcPortTrunk [0x00FFFFFF]
                              }.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiTrillAdjacencyCheckEntryGet.
    Expected: GT_OK and the same value.
    1.3. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.
    1.4. Call with out of range value.trillMBit, mask.trillMBit [0] and
                   other valid params.
    Expected: NOT GT_OK.
    1.5. Call with out of range value.trillMBit, mask.trillMBit [0x00000001] and
                   other valid params.
    Expected: NOT GT_OK.
    1.6. Call with out of range value.trillEgressRbid,
                   mask.trillEgressRbid [0] and
                   other valid params.
    Expected: NOT GT_OK.
    1.7. Call with out of range value.trillEgressRbid,
                   mask.trillEgressRbid [0x000FFFFF] and
                   other valid params.
    Expected: NOT GT_OK.
    1.8. Call with out of range value.srcDevice, mask.srcDevice [0] and
                   other valid params.
    Expected: NOT GT_OK.
    1.9. Call with out of range value.srcDevice,
                   mask.srcDevice [0x00000FFF] and
                   other valid params.
    Expected: NOT GT_OK.
    1.10. Call with out of range value.srcIsTrunk, mask.srcIsTrunk [0] and
                    other valid params.
    Expected: NOT GT_OK.
    1.11. Call with out of range value.srcIsTrunk, mask.srcIsTrunk [0x00000001] and
                    other valid params.
    Expected: NOT GT_OK.
    1.12. Call with out of range value.srcPortTrunk, mask.srcPortTrunk [0] and
                    other valid params.
    Expected: NOT GT_OK.
    1.13. Call with out of range value.srcPortTrunk,
                    mask.srcPortTrunk [0x00FFFFFF] and
                    other valid params.
    Expected: NOT GT_OK.
    1.14. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.15. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st = GT_OK;

    GT_U8                                           devNum;
    GT_U32                                          entryIndex;
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC               value;
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC               valueGet;
    GT_BOOL                                         isEqual;
    GT_U32                                          notAppFamilyBmp;

    cpssOsBzero((GT_VOID*) &value,                  sizeof(value));
    cpssOsBzero((GT_VOID*) &valueGet,               sizeof(valueGet));

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex [0 / BIT_4 / max],
                                value {     outerMacSa [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                             trillMBit [0 / 1],
                                       trillEgressRbid [0 / max/2 / max],
                                             srcDevice [devNum],
                                            srcIsTrunk [0 / 1],
                                          srcPortTrunk [TTI_VALID_PORT_TRUNK_CNS / max/2 / max]
                                      },
                                 mask {     outerMacSa [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]]
                                             trillMBit [0x00000001],
                                       trillEgressRbid [0x000FFFFF],
                                             srcDevice [0x00000FFF],
                                            srcIsTrunk [0x00000001],
                                          srcPortTrunk [0x00FFFFFF]
                                      }.
            Expected: GT_OK.
        */
        entryIndex                          = 0;

        value.outerMacSa.arEther[0]         = 0xAA;
        value.outerMacSa.arEther[1]         = 0xAA;
        value.outerMacSa.arEther[2]         = 0xAA;
        value.outerMacSa.arEther[3]         = 0xAA;
        value.outerMacSa.arEther[4]         = 0xAA;
        value.outerMacSa.arEther[5]         = 0xAA;
        value.trillMBit                     = 0;
        value.trillEgressRbid               = 0;
        value.srcHwDevice                     = devNum;
        value.srcIsTrunk                    = 0;
        value.srcPortTrunk                  = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call cpssDxChTtiTrillAdjacencyCheckEntryGet.
            Expected: GT_OK and the same value and mask.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillAdjacencyCheckEntrySet: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.outerMacSa,
                                     (GT_VOID*) &valueGet.outerMacSa,
                                     sizeof(value.outerMacSa))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another value.outerMacSa then was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillMBit, valueGet.trillMBit,
            "get another value.trillMBit than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillEgressRbid, valueGet.trillEgressRbid,
            "get another value.trillEgressRbid than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcHwDevice, valueGet.srcHwDevice,
            "get another value.srcHwDevice than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcIsTrunk, valueGet.srcIsTrunk,
            "get another value.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcPortTrunk, valueGet.srcPortTrunk,
            "get another value.srcPortTrunk than was set: %d, %d", devNum, entryIndex);

        /*
            Call with entryIndex [BIT_4],
                           value {     outerMacSa [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                        trillMBit [1],
                                  trillEgressRbid [max/2],
                                        srcDevice [devNum],
                                       srcIsTrunk [1],
                                     srcPortTrunk [max/2]
                                 },
                            mask {     outerMacSa [[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]]
                                        trillMBit [0x00000001],
                                  trillEgressRbid [0x000FFFFF],
                                        srcDevice [0x00000FFF],
                                       srcIsTrunk [0x00000001],
                                     srcPortTrunk [0x00FFFFFF]
                                 }.
            Expected: GT_OK.
        */
        entryIndex                  = BIT_4;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 1;
        value.trillEgressRbid       = (BIT_16 - 1) / 2;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 1;
        value.srcPortTrunk          = (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1) / 2;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call cpssDxChTtiTrillAdjacencyCheckEntryGet.
            Expected: GT_OK and the same value and mask.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillAdjacencyCheckEntrySet: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.outerMacSa,
                                     (GT_VOID*) &valueGet.outerMacSa,
                                     sizeof(value.outerMacSa))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another value.outerMacSa then was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillMBit, valueGet.trillMBit,
            "get another value.trillMBit than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillEgressRbid, valueGet.trillEgressRbid,
            "get another value.trillEgressRbid than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcHwDevice, valueGet.srcHwDevice,
            "get another value.srcHwDevice than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcIsTrunk, valueGet.srcIsTrunk,
            "get another value.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcPortTrunk, valueGet.srcPortTrunk,
            "get another value.srcPortTrunk than was set: %d, %d", devNum, entryIndex);

        /*
            Call with entryIndex [max],
                           value {     outerMacSa [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                        trillMBit [0],
                                  trillEgressRbid [max],
                                        srcDevice [devNum],
                                       srcIsTrunk [0],
                                     srcPortTrunk [max]
                                 }
            Expected: GT_OK.
        */
        entryIndex                  = ADJACENCY_MAX_CNS;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = BIT_16 - 1;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15);

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call cpssDxChTtiTrillAdjacencyCheckEntryGet.
            Expected: GT_OK and the same value and mask.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillAdjacencyCheckEntrySet: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.outerMacSa,
                                     (GT_VOID*) &valueGet.outerMacSa,
                                     sizeof(value.outerMacSa))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another value.outerMacSa then was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillMBit, valueGet.trillMBit,
            "get another value.trillMBit than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillEgressRbid, valueGet.trillEgressRbid,
            "get another value.trillEgressRbid than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcHwDevice, valueGet.srcHwDevice,
            "get another value.srcHwDevice than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcIsTrunk, valueGet.srcIsTrunk,
            "get another value.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcPortTrunk, valueGet.srcPortTrunk,
            "get another value.srcPortTrunk than was set: %d, %d", devNum, entryIndex);

        /*
            Call with entryIndex [max],
                           value {     outerMacSa [[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]],
                                        trillMBit [1],
                                  trillEgressRbid [max],
                                        srcDevice [devNum],
                                       srcIsTrunk [1],
                                     srcPortTrunk [max]
                                 }
            Expected: GT_OK.
        */
        entryIndex                  = ADJACENCY_MAX_CNS;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 1;
        value.trillEgressRbid       = BIT_16 - 1;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 1;
        value.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call cpssDxChTtiTrillAdjacencyCheckEntryGet.
            Expected: GT_OK and the same value and mask.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillAdjacencyCheckEntrySet: %d, %d", devNum, entryIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value.outerMacSa,
                                     (GT_VOID*) &valueGet.outerMacSa,
                                     sizeof(value.outerMacSa))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
            "got another value.outerMacSa then was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillMBit, valueGet.trillMBit,
            "get another value.trillMBit than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.trillEgressRbid, valueGet.trillEgressRbid,
            "get another value.trillEgressRbid than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcHwDevice, valueGet.srcHwDevice,
            "get another value.srcHwDevice than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcIsTrunk, valueGet.srcIsTrunk,
            "get another value.srcIsTrunk than was set: %d, %d", devNum, entryIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(value.srcPortTrunk, valueGet.srcPortTrunk,
            "get another value.srcPortTrunk than was set: %d, %d", devNum, entryIndex);

        /*
            1.3. Call with out of range entryIndex and other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = ADJACENCY_MAX_CNS + 1;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        entryIndex              = 0;

        /*
            1.4. Call with out of range value.trillMBit, mask.trillMBit [0] and
                            other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 2;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = 0;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.trillMBit             = 0;

        /*
            1.5. Call with out of range value.trillMBit and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 2;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.trillMBit             = 0;

        /*
            1.6. Call with out of range value.trillEgressRbid,
                            mask.trillEgressRbid [0] and
                            other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = BIT_16;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = 0;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.trillEgressRbid       = BIT_16;

        /*
            1.7. Call with out of range value.trillEgressRbid and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = BIT_16;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.trillEgressRbid       = BIT_16;

        /*
            1.8. Call with out of range value.srcDevice, mask.srcDevice [0] and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = 0;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcHwDevice             = devNum;

        /*
            1.9. Call with out of range value.srcDevice and
                           other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcHwDevice             = devNum;

        /*
            1.10. Call with out of range value.srcIsTrunk, mask.srcIsTrunk [0] and
                            other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 3;
        value.srcPortTrunk          = 0;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcIsTrunk            = 0;

        /*
            1.11. Call with out of range value.srcIsTrunk and
                            other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 3;
        value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcIsTrunk            = 0;

       /*
            1.12. Call with out of range value.srcPortTrunk,
                            mask.srcPortTrunk [0] and
                            other valid params.
            Expected: GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK(15);

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Call with out of range value.srcPortTrunk, value.srcIsTrunk [1] and  */
        /*           other valid params.                                        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 1;
        value.srcPortTrunk          = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcPortTrunk          = 0;

        /*
            1.13. Call with out of range value.srcPortTrunk and
                            other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 0;
        value.srcPortTrunk          = BIT_20;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Call with out of range value.srcPortTrunk, value.srcIsTrunk [1] and  */
        /*           other valid params.                                        */
        entryIndex                  = 0;

        value.outerMacSa.arEther[0] = 0xAA;
        value.outerMacSa.arEther[1] = 0xAA;
        value.outerMacSa.arEther[2] = 0xAA;
        value.outerMacSa.arEther[3] = 0xAA;
        value.outerMacSa.arEther[4] = 0xAA;
        value.outerMacSa.arEther[5] = 0xAA;
        value.trillMBit             = 0;
        value.trillEgressRbid       = 0;
        value.srcHwDevice             = devNum;
        value.srcIsTrunk            = 1;
        value.srcPortTrunk          = BIT_12;

        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params.    */
        value.srcPortTrunk          = 0;

        /*
            1.14. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                     devNum, entryIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    entryIndex                  = 0;

    value.outerMacSa.arEther[0] = 0xAA;
    value.outerMacSa.arEther[1] = 0xAA;
    value.outerMacSa.arEther[2] = 0xAA;
    value.outerMacSa.arEther[3] = 0xAA;
    value.outerMacSa.arEther[4] = 0xAA;
    value.outerMacSa.arEther[5] = 0xAA;
    value.trillMBit             = 0;
    value.trillEgressRbid       = 0;
    value.srcHwDevice             = devNum;
    value.srcIsTrunk            = 0;
    value.srcPortTrunk          = TTI_VALID_PORT_TRUNK_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *valuePtr,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillAdjacencyCheckEntryGet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with entryIndex [0 / 255] and other valid params.
    Expected: GT_OK.
    1.2. Call with out of range entryIndex and other valid params.
    Expected: NOT GT_OK.
    1.3. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL maskPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_U32                                  entryIndex;
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC       value;
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC       mask;
    GT_U32                                  notAppFamilyBmp;

    cpssOsBzero((GT_VOID*) &value,          sizeof(value));
    cpssOsBzero((GT_VOID*) &mask,           sizeof(mask));

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex [0 / max] and other valid params.
            Expected: GT_OK.
        */
        entryIndex = 0;

        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Call with entryIndex [max]. */
        entryIndex = ADJACENCY_MAX_CNS;

        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /*
            1.2. Call with out of range entryIndex and other valid params.
            Expected: NOT GT_OK.
        */
        entryIndex = ADJACENCY_MAX_CNS + 1;

        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);

        /* Restore valid params. */
        entryIndex = 0;

        /*
            1.3. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, valuePtr = NULL",
                                     devNum, entryIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    entryIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillMaxVersionSet
(
    IN GT_U8            devNum,
    IN GT_U32           maxTrillVersion
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillMaxVersionSet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with maxTrillVersion [0 / BIT_1 / max].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiTrillMaxVersionGet.
    Expected: GT_OK and the same maxTrillVersion.
    1.3. Call with out of range maxTrillVersion.
    Expected: NOT GT_OK.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_U32              maxTrillVersion;
    GT_U32              maxTrillVersionGet;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with maxTrillVersion [0 / BIT_1 / max].
            Expected: GT_OK.
        */
        maxTrillVersion = 0;

        st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, maxTrillVersion);

        /*
            1.2. Call cpssDxChTtiTrillMaxVersionGet.
            Expected: GT_OK and the same maxTrillVersion.
        */
        st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillMaxVersionGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(maxTrillVersion, maxTrillVersionGet,
            "get another maxTrillVersion than was set: %d", devNum);

        /* Call with maxTrillVersion [BIT_1]. */
        maxTrillVersion = BIT_1;

        st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, maxTrillVersion);

        /*
            1.2. Call cpssDxChTtiTrillMaxVersionGet.
            Expected: GT_OK and the same maxTrillVersion.
        */
        st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillMaxVersionGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(maxTrillVersion, maxTrillVersionGet,
            "get another maxTrillVersion than was set: %d", devNum);

        /* Call with maxTrillVersion [BIT_2]. */
        maxTrillVersion = BIT_2 - 1;

        st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, maxTrillVersion);

        /*
            1.2. Call cpssDxChTtiTrillMaxVersionGet.
            Expected: GT_OK and the same maxTrillVersion.
        */
        st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTrillMaxVersionGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(maxTrillVersion, maxTrillVersionGet,
            "get another maxTrillVersion than was set: %d", devNum);

        /*
            1.3. Call with out of range maxTrillVersion.
            Expected: NOT GT_OK.
        */
        maxTrillVersion = BIT_2;

        st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, maxTrillVersion);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    maxTrillVersion = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound maxTrillVersion for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiTrillMaxVersionGet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call function with not NULL maxTrillVersionPtr.
    Expected: GT_OK.
    1.2. Call function with NULL maxTrillVersionPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               devNum;
    GT_U32              maxTrillVersion;
    GT_U32              notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL maxTrillVersionPtr.
            Expected: GT_OK.
        */
        st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersion);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &maxTrillVersion);

        /*
            1.2. Call function with NULL maxTrillVersionPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiTrillMaxVersionGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "%d, maxTrillVersionPtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersion);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound maxTrillVersion for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTrillMaxVersionGet(devNum, &maxTrillVersion);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}
#endif

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpTotalLengthDeductionEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortIpTotalLengthDeductionEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS           st              = GT_OK;
    GT_U8               devNum          = 0;
    GT_PORT_NUM         portNum         = 0;
    GT_BOOL             enable          = GT_FALSE;
    GT_BOOL             enableGet       = GT_FALSE;
    GT_U32              notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum,
                                                                enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChTtiPortIpTotalLengthDeductionEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                                &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortIpTotalLengthDeductionEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);

            /* Call with enable [GT_FALSE]. */
            enable = GT_FALSE;

            st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum,
                                                                enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChTtiPortIpTotalLengthDeductionEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChTtiPortIpTotalLengthDeductionEnableGet: %d", devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d", devNum);
        }

        portNum = 0;
        enable = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum,
                                                                enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        enable);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum,
                                                            enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum,
                                                            enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortIpTotalLengthDeductionEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st              = GT_OK;
    GT_U8               devNum          = 0;
    GT_PORT_NUM         portNum         = 0;
    GT_BOOL             enable          = GT_FALSE;
    GT_U32              notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                                &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &enable);

            /*
                1.1.2. Call function with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                                NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "%d, enablePtr = NULL", devNum);
        }

        portNum = 0;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                                &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        &enable);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                            &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                             */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                            &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                            &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum,
                                                        &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiIpTotalLengthDeductionValueSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with value[0, 30, max],
                   ipType[CPSS_IP_PROTOCOL_IPV4_E,
                          CPSS_IP_PROTOCOL_IPV6_E,
                          CPSS_IP_PROTOCOL_IPV4_E],
                   and other valid params.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiIpTotalLengthDeductionValueGet.
    Expected: GT_OK and the same value.
    1.3. Call with out of range enum value ipType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range value and other valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       devNum          = 0;
    GT_U32                      value           = 0;
    GT_U32                      valueGet        = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  ipType          = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U32                      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with value[0, 30, max],
                   ipType[CPSS_IP_PROTOCOL_IPV4_E,
                          CPSS_IP_PROTOCOL_IPV6_E,
                          CPSS_IP_PROTOCOL_IPV4_E],
                   and other valid params.
            Expected: GT_OK.
        */
        value = 0;
        ipType = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, value);

        /*
            1.2. Call cpssDxChTtiIpTotalLengthDeductionValueGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiIpTotalLengthDeductionValueGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet,
            "get another value than was set: %d", devNum);

        /* Call with value[30]. */
        value = 30;
        ipType = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, value);

        /*
            1.2. Call cpssDxChTtiIpTotalLengthDeductionValueGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiIpTotalLengthDeductionValueGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet,
            "get another value than was set: %d", devNum);

        /* Call with value[max] and other valid params. */
        value = BIT_6 - 1;
        ipType = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, value);

        /*
            1.2. Call cpssDxChTtiIpTotalLengthDeductionValueGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiIpTotalLengthDeductionValueGet: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet,
            "get another value than was set: %d", devNum);

        /*
            1.3. Call with out of range enum value ipType
                and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiIpTotalLengthDeductionValueSet
                            (devNum, ipType, value),
                            ipType);

        /*
            1.4. Call with out of range value and other valid params.
            Expected: NOT GT_OK.
        */
        value = BIT_6;

        st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, value);
    }

    /* restore values */
    value = 0;
    ipType = CPSS_IP_PROTOCOL_IPV4_E;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiIpTotalLengthDeductionValueGet)
{
/*
    ITERATE_DEVICES(SIP5)
    1.1.1. Call function with not NULL valuePtr and other valid params.
    Expected: GT_OK.
    1.1.2. Call with out of range enum value ipType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.1.3. Call function with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st              = GT_OK;
    GT_U8                       devNum          = 0;
    GT_U32                      value           = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  ipType          = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U32                      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL valuePtr and other valid params.
            Expected: GT_OK.
        */
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &value);

        /*
            1.1.2. Call with out of range enum value ipType
                and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiIpTotalLengthDeductionValueGet
                            (devNum, ipType, &value),
                            ipType);

        /*
            1.1.3. Call function with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "%d, valuePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
);
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortPassengerOuterIsTag0Or1Set)
{
/*
    ITERATE_DEVICES_INGRESS_PORTS(SIP5)
    1.1.1. Call with mode[CPSS_VLAN_ETHERTYPE0_E, CPSS_VLAN_ETHERTYPE0_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortPassengerOuterIsTag0Or1Get.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call API with out of range enum value mode.
    Expected: GT_BAD_PARAM.
*/

    GT_U8               dev     = 0;
    GT_PORT_NUM         port    = 0;
    GT_STATUS           st      = GT_OK;
    CPSS_ETHER_MODE_ENT mode    = CPSS_VLAN_ETHERTYPE0_E;
    CPSS_ETHER_MODE_ENT modeRet = CPSS_VLAN_ETHERTYPE1_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available generic ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with mode[CPSS_VLAN_ETHERTYPE0_E, CPSS_VLAN_ETHERTYPE0_E].
                Expected: GT_OK.
            */

            /* call with mode[CPSS_VLAN_ETHERTYPE0_E] */
            mode = CPSS_VLAN_ETHERTYPE0_E;
            st = cpssDxChTtiPortPassengerOuterIsTag0Or1Set(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChTtiPortPassengerOuterIsTag0Or1Get.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, &modeRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeRet,
                                         "got another value mode "\
                                         "than was set: %d, %d", dev, port);

            /* call with mode[CPSS_VLAN_ETHERTYPE1_E] */
            mode = CPSS_VLAN_ETHERTYPE1_E;
            st = cpssDxChTtiPortPassengerOuterIsTag0Or1Set(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChTtiPortPassengerOuterIsTag0Or1Get.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, &modeRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeRet,
                                         "got another value mode "\
                                         "than was set: %d, %d", dev, port);

            /*
                1.1.3. Call API with out of range enum value mode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortPassengerOuterIsTag0Or1Set
                                (dev, port, mode),
                                mode);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiPortPassengerOuterIsTag0Or1Set(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTtiPortPassengerOuterIsTag0Or1Set(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Get
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT  CPSS_ETHER_MODE_ENT         *modePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortPassengerOuterIsTag0Or1Get)
{
    /*
        ITERATE_DEVICES_INGRESS_PORTS(SIP5)
        1.1.1. Call with valid  modePtr.
        Expected: GT_OK.
        1.1.2. Call with out of range modePtr[NULL].
        Expected: GT_BAD_PTR.
    */

        GT_U8               dev     = 0;
        GT_PORT_NUM         port    = 0;
        GT_STATUS           st      = GT_OK;
        CPSS_ETHER_MODE_ENT mode    = CPSS_VLAN_ETHERTYPE0_E;

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                               UTF_CH2_E | UTF_CH3_E |
                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                               UTF_LION_E | UTF_LION2_E );

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {

            prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);

            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available generic ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
            {
                /*
                    1.1.1. Call with valid  modePtr.
                    Expected: GT_OK.
                */

                st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


                /*
                    1.1.2. Call with out of range modePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }
        }

        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_BAD_PARAM.                        */

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                               UTF_CH2_E | UTF_CH3_E |
                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                               UTF_LION_E | UTF_LION2_E );

        /* go over all non active devices */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssDxChTtiPortPassengerOuterIsTag0Or1Get(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupUserDefinedByteSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                           CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                           CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                   udbIndex[0, 11, 29],
                   offsetType[CPSS_DXCH_TTI_OFFSET_L2_E,
                              CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E,
                              CPSS_DXCH_TTI_OFFSET_INVALID_E],
                   offset[0, 77, 127].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTtiPortGroupUserDefinedByteGet.
    Expected: GT_OK and the same value.
    1.1.3. Call with out of range enum value keyType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range udbIndex[30] and other valid parameters.
    Expected: NOT GT_OK.
    1.1.5. Call with out of range enum value offsetType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with out of range offset[128] and other valid parameters.
    Expected: NOT GT_OK.
*/

    GT_STATUS                       st              = GT_OK;
    GT_U8                           devNum          = 0;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 0;
    GT_U32                          portGroupId     = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType         = 0;
    GT_U32                          udbIndex        = 0;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetType      = 0;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetTypeGet   = 0;
    GT_U8                           offset          = 0;
    GT_U8                           offsetGet       = 0;
    GT_U32                          notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Initialize port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                                       CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                               udbIndex[0, 11, 29],
                               offsetType[CPSS_DXCH_TTI_OFFSET_L2_E,
                                          CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E,
                                          CPSS_DXCH_TTI_OFFSET_INVALID_E],
                               offset[0, 77, 127].
                Expected: GT_OK.
            */

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
            udbIndex = 0;
            offsetType = CPSS_DXCH_TTI_OFFSET_L2_E;
            offset = 0;

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /*
                1.1.2. Call cpssDxChTtiPortGroupUserDefinedByteGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetTypeGet, &offsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                         "got another value than was set: %d",
                                         offsetTypeGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                         "got another value than was set: %d",
                                         offsetGet);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
            udbIndex = 11;
            offsetType = CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E;
            offset = 77;

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /*
                1.1.2. Call cpssDxChTtiPortGroupUserDefinedByteGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetTypeGet, &offsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                         "got another value than was set: %d",
                                         offsetTypeGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                         "got another value than was set: %d",
                                         offsetGet);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
            udbIndex = 29;
            offsetType = CPSS_DXCH_TTI_OFFSET_INVALID_E;
            offset = 127;

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /*
                1.1.2. Call cpssDxChTtiPortGroupUserDefinedByteGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetTypeGet, &offsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                         "got another value than was set: %d",
                                         offsetTypeGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                         "got another value than was set: %d",
                                         offsetGet);

            /*
                1.1.3. Call with out of range enum value keyType and
                    other valid parameters.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupUserDefinedByteSet
                                (devNum,  portGroupsBmp, keyType, udbIndex, offsetType, offset),
                                keyType);

            /*
                1.1.4. Call with out of range udbIndex[30] and other valid parameters.
                Expected: NOT GT_OK.
            */
            udbIndex = 30;

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* restore values */
            udbIndex = 29;

            /*
                1.1.5. Call with out of range enum value offsetType and
                    other valid parameters.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupUserDefinedByteSet
                                (devNum,  portGroupsBmp, keyType, udbIndex, offsetType, offset),
                                offsetType);

            /*
                1.1.6. Call with out of range offset[128] and
                    other valid parameters.
                Expected: NOT GT_OK.
            */
            offset = 128;

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* restore values */
            offset = 127;
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* restore port group. */
        portGroupId = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next non-active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                        keyType, udbIndex,
                                                        offsetType, offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp);

        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum,  portGroupsBmp,
                                                    keyType, udbIndex,
                                                    offsetType, offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portGroupsBmp = 1;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum, portGroupsBmp,
                                                    keyType, udbIndex,
                                                    offsetType, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupUserDefinedByteSet(devNum, portGroupsBmp,
                                                keyType, udbIndex,
                                                offsetType, offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPortGroupUserDefinedByteGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                           CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                           CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                   udbIndex[0, 11, 29],
                   non-null offsetTypePtr and non-null offsetPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range enum value keyType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with out of range udbIndex[30] and other valid parameters.
    Expected: NOT GT_OK.
    1.1.4. Call with out of range offsetTypePtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.1.5. Call with out of range offsetPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                       st              = GT_OK;
    GT_U8                           devNum          = 0;
    GT_PORT_GROUPS_BMP              portGroupsBmp   = 0;
    GT_U32                          portGroupId     = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType         = 0;
    GT_U32                          udbIndex        = 0;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetType      = 0;
    GT_U8                           offset          = 0;
    GT_U32                          notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Initialize port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                                       CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                               udbIndex[0, 11, 29],
                               non-null offsetTypePtr and non-null offsetPtr.
                Expected: GT_OK.
            */

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
            udbIndex = 0;

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
            udbIndex = 11;

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
            udbIndex = 29;

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /*
                1.1.2. Call with out of range enum value keyType and
                    other valid parameters.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiPortGroupUserDefinedByteGet
                                (devNum, portGroupsBmp, keyType, udbIndex, &offsetType, &offset),
                                keyType);

            /*
                1.1.3. Call with out of range udbIndex[30] and other valid parameters.
                Expected: NOT GT_OK.
            */
            udbIndex = 30;

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, &offset);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* restore values */
            udbIndex = 29;

            /*
                1.1.4. Call with out of range offsetTypePtr[NULL] and
                    other valid parameters.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        NULL, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType);

            /*
                1.1.5. Call with out of range offsetPtr[NULL] and
                    other valid parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType);

        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        /* restore port group. */
        portGroupId = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next non-active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                        keyType, udbIndex,
                                                        &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portGroupsBmp);

        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum,  portGroupsBmp,
                                                    keyType, udbIndex,
                                                    &offsetType, &offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portGroupsBmp = 1;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                    keyType, udbIndex,
                                                    &offsetType, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp,
                                                keyType, udbIndex,
                                                &offsetType, &offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiUserDefinedByteSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                           CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                           CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                   udbIndex[0, 11, 29],
                   offsetType[CPSS_DXCH_TTI_OFFSET_L2_E,
                              CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E,
                              CPSS_DXCH_TTI_OFFSET_INVALID_E],
                   offset[0, 77, 127].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiUserDefinedByteGet.
    Expected: GT_OK and the same value.
    1.3. Call with out of range enum value keyType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range udbIndex[30] and other valid parameters.
    Expected: NOT GT_OK.
    1.5. Call with out of range enum value offsetType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range offset[128] and other valid parameters.
    Expected: NOT GT_OK.
*/

    GT_STATUS                       st              = GT_OK;
    GT_U8                           devNum          = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType         = 0;
    GT_U32                          udbIndex        = 0;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetType      = 0;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetTypeGet   = 0;
    GT_U8                           offset          = 0;
    GT_U8                           offsetGet       = 0;
    GT_U32                          notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                                   CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                   CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           udbIndex[0, 11, 29],
                           offsetType[CPSS_DXCH_TTI_OFFSET_L2_E,
                                      CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E,
                                      CPSS_DXCH_TTI_OFFSET_INVALID_E],
                           offset[0, 77, 127].
            Expected: GT_OK.
        */

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        udbIndex = 0;
        offsetType = CPSS_DXCH_TTI_OFFSET_L2_E;
        offset = 0;

        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiUserDefinedByteGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                           &offsetTypeGet, &offsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                     "got another value than was set: %d",
                                     offsetTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                     "got another value than was set: %d",
                                     offsetGet);

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
        udbIndex = 11;
        offsetType = CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E;
        offset = 77;

        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiUserDefinedByteGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                           &offsetTypeGet, &offsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                     "got another value than was set: %d",
                                     offsetTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                     "got another value than was set: %d",
                                     offsetGet);

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        udbIndex = 29;
        offsetType = CPSS_DXCH_TTI_OFFSET_INVALID_E;
        offset = 127;

        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiUserDefinedByteGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                           &offsetTypeGet, &offsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        UTF_VERIFY_EQUAL1_STRING_MAC(offsetType, offsetTypeGet,
                                     "got another value than was set: %d",
                                     offsetTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(offset, offsetGet,
                                     "got another value than was set: %d",
                                     offsetGet);

        /*
            1.3. Call with out of range enum value keyType and
                other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiUserDefinedByteSet
                            (devNum, keyType, udbIndex, offsetType, offset),
                            keyType);

        /*
            1.4. Call with out of range udbIndex[30] and other valid parameters.
            Expected: NOT GT_OK.
        */
        udbIndex = 30;

        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /* restore values */
        udbIndex = 29;

        /*
            1.5. Call with out of range enum value offsetType and
                other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiUserDefinedByteSet
                            (devNum, keyType, udbIndex, offsetType, offset),
                            offsetType);

        /*
            1.6. Call with out of range offset[128] and
                other valid parameters.
            Expected: NOT GT_OK.
        */
        offset = 128;

        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /* restore values */
        offset = 127;
    }

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                           offsetType, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex,
                                       offsetType, offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_U32                              udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiUserDefinedByteGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                           CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                           CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                   udbIndex[0, 11, 29],
                   non-null offsetTypePtr and non-null offsetPtr.
    Expected: GT_OK.
    1.2. Call with out of range enum value keyType and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range udbIndex[30] and other valid parameters.
    Expected: NOT GT_OK.
    1.4. Call with out of range offsetTypePtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.5. Call with out of range offsetPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
*/

        GT_STATUS                       st              = GT_OK;
        GT_U8                           devNum          = 0;
        CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType         = 0;
        GT_U32                          udbIndex        = 0;
        CPSS_DXCH_TTI_OFFSET_TYPE_ENT   offsetType      = 0;
        GT_U8                           offset          = 0;
        GT_U32                          notAppFamilyBmp = 0;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                                       CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                               udbIndex[0, 11, 29],
                               non-null offsetTypePtr and non-null offsetPtr.
                Expected: GT_OK.
            */

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
            udbIndex = 0;

            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
            udbIndex = 11;

            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
            keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
            udbIndex = 29;

            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /*
                1.2. Call with out of range enum value keyType and
                    other valid parameters.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTtiUserDefinedByteGet
                                (devNum, keyType, udbIndex, &offsetType, &offset),
                                keyType);

            /*
                1.3. Call with out of range udbIndex[30] and other valid parameters.
                Expected: NOT GT_OK.
            */
            udbIndex = 30;

            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, &offset);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

            /* restore values */
            udbIndex = 29;

            /*
                1.4. Call with out of range offsetTypePtr[NULL] and
                    other valid parameters.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               NULL, &offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType);

            /*
                1.5. Call with out of range offsetPtr[NULL] and
                    other valid parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType);
        }

        /* 2. For not active devices check that function returns non GT_OK. */

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                               &offsetType, &offset);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex,
                                           &offsetType, &offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN CPSS_DXCH_TTI_KEY_SIZE_ENT           size
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPacketTypeKeySizeSet)
{
    /*
        ITERATE_DEVICES (SIP5)
        1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                               CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                               CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                       keySize[CPSS_DXCH_TTI_KEY_SIZE_10_B_E,
                               CPSS_DXCH_TTI_KEY_SIZE_20_B_E,
                               CPSS_DXCH_TTI_KEY_SIZE_30_B_E],
                       and other valid params.
        Expected: GT_OK.
        1.2. Call cpssDxChTtiPacketTypeKeySizeGet.
        Expected: GT_OK and the same value.
        1.3. Call with out of range enum value keyType and other valid parameters.
        Expected: GT_BAD_PARAM.
        1.4. Call with out of range enum value keySize and other valid parameters.
        Expected: GT_BAD_PARAM.
    */

    GT_STATUS                   st              = GT_OK;
    GT_U8                       devNum          = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType         = 0;
    CPSS_DXCH_TTI_KEY_SIZE_ENT  keySize         = 0;
    CPSS_DXCH_TTI_KEY_SIZE_ENT  keySizeGet      = 0;
    GT_U32                      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with keyType[CPSS_DXCH_TTI_KEY_IPV4_E,
                                   CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                   CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           keySize[CPSS_DXCH_TTI_KEY_SIZE_10_B_E,
                                   CPSS_DXCH_TTI_KEY_SIZE_20_B_E,
                                   CPSS_DXCH_TTI_KEY_SIZE_30_B_E],
                           and other valid params.
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        keySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;

        st = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiPacketTypeKeySizeGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySizeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChTtiPacketTypeKeySizeGet: %d",
                                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(keySize, keySizeGet,
                                     "got another value than was set: %d",
                                     keySizeGet);

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
        keySize = CPSS_DXCH_TTI_KEY_SIZE_20_B_E;

        st = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiPacketTypeKeySizeGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySizeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChTtiPacketTypeKeySizeGet: %d",
                                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(keySize, keySizeGet,
                                     "got another value than was set: %d",
                                     keySizeGet);

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        keySize = CPSS_DXCH_TTI_KEY_SIZE_30_B_E;

        st = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call cpssDxChTtiPacketTypeKeySizeGet.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySizeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChTtiPacketTypeKeySizeGet: %d",
                                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(keySize, keySizeGet,
                                     "got another value than was set: %d",
                                     keySizeGet);

        /*
            1.3. Call with out of range enum value keyType and
                other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPacketTypeKeySizeSet
                            (devNum, keyType, keySize),
                            keyType);

        /*
            1.4. Call with out of range enum value keySize and
                other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPacketTypeKeySizeSet
                            (devNum, keyType, keySize),
                            keySize);
    }

    /* restore values */
    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
    keySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, keySize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, keySize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    OUT CPSS_DXCH_TTI_KEY_SIZE_ENT           *sizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPacketTypeKeySizeGet)
{
    /*
        ITERATE_DEVICES (SIP5)
        1.1. Call with keyType[CPSS_DXCH_TTI_KEY_IPV4_E,
                               CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                               CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                       non-null keySizePtr and other valid params.
        Expected: GT_OK.
        1.2. Call with out of range enum value keyType and other valid parameters.
        Expected: GT_BAD_PARAM.
        1.3. Call with out of range keySizePtr[NULL] and other valid parameters.
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                   st              = GT_OK;
    GT_U8                       devNum          = 0;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType         = 0;
    CPSS_DXCH_TTI_KEY_SIZE_ENT  keySize         = 0;
    GT_U32                      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with keyType[CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E,
                                   CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                   CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           non-null keySizePtr and other valid params.
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);


        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;

        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /* call with keyType[CPSS_DXCH_TTI_KEY_UDB_UDE6_E] */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;

        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.2. Call with out of range enum value keyType and
                other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPacketTypeKeySizeGet
                            (devNum, keyType, &keySize),
                            keyType);

        /*
            1.3. Call with out of range keySizePtr[NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyType);
    }

    /* restore values */
    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &keySize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwSequencingSupportEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Bobcat3)
    1.1. Call function with enable [GT_TRUE / GT_FALSE]
    Expected: GT_OK.
    1.2. Call function cpssDxChTtiPwCwSequencingSupportEnableGet with not NULL enablePtr.
    Expected: GT_OK and enable the same as was set.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* iterate with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call function cpssDxChTtiPwCwSequencingSupportEnableGet with not NULL enablePtr.
            Expected: GT_OK and enable the same as was set.
        */
        st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPwCwSequencingSupportEnableGet: %d", devNum);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "got another enable then was set: %d", devNum);

        /* iterate with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call function cpssDxChTtiPwCwSequencingSupportEnableGet with not NULL enablePtr.
            Expected: GT_OK and enable the same as was set.
        */
        st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiPwCwSequencingSupportEnableGet: %d", devNum);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "got another enable then was set: %d", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwSequencingSupportEnableGet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st = GT_OK;
    GT_U8       devNum;

    GT_BOOL     enable = GT_FALSE;
    GT_U32      notAppFamilyBmp = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call function with enablePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiExceptionCpuCodeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiExceptionCpuCodeSet with relevant value
              exceptionType[CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E]
              code[CPSS_NET_ETH_BRIDGED_LLT_E/
                   CPSS_NET_CONTROL_E/
                   CPSS_NET_CONTROL_BPDU_E/
                   CPSS_NET_INTERVENTION_IGMP_E/
                   CPSS_NET_MIRROR_IPV4_RIPV1_TO_CPU_E/
                   CPSS_NET_RPF_CHECK_FAILED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiExceptionCpuCodeGet.
    Expected: GT_OK and the same code.
    1.3. Call with wrong enum value exceptionType.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum value code.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    CPSS_NET_RX_CPU_CODE_ENT            code = CPSS_NET_CONTROL_E;
    CPSS_NET_RX_CPU_CODE_ENT            codeGet = CPSS_NET_CONTROL_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E;
        code = CPSS_NET_ETH_BRIDGED_LLT_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E;
        code = CPSS_NET_CONTROL_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E;
        code = CPSS_NET_CONTROL_BPDU_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E;
        code = CPSS_NET_INTERVENTION_IGMP_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E;
        code = CPSS_NET_MIRROR_IPV4_RIPV1_TO_CPU_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.1.  */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E;
        code = CPSS_NET_RPF_CHECK_FAILED_E;

        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &codeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiExceptionCpuCodeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(code, codeGet,
                   "get another code than was set: %d", dev);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code),
                            exceptionType);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code),
                            code);
    }

    exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    code = CPSS_NET_CONTROL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiExceptionCpuCodeSet(dev, exceptionType, code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiExceptionCpuCodeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiExceptionCpuCodeGet with relevant value
              exceptionType[CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E/
                            CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E]
              and not NULL codePtr.
    Expected: GT_OK.
    1.2. Call with wrong enum value exceptionType.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL codePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    CPSS_NET_RX_CPU_CODE_ENT            code = CPSS_NET_CONTROL_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        exceptionType = CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E;

        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code),
                            exceptionType);

        /*  1.3. */
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiExceptionCpuCodeGet(dev, exceptionType, &code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiTcamSegmentModeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiTcamSegmentModeSet with relevant value
              keyType[CPSS_DXCH_TTI_KEY_IPV4_E/
                      CPSS_DXCH_TTI_KEY_MPLS_E/
                      CPSS_DXCH_TTI_KEY_ETH_E/
                      CPSS_DXCH_TTI_KEY_MIM_E/
                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E]
              segmentMode
    Expected: GT_OK.
    1.2. Call cpssDxChTtiTcamSegmentModeGet.
    Expected: GT_OK and the same segmentMode or GT_BAD_PARAM according to device revision.
    1.3. Call with wrong enum value keyType.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum value segmentMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentModeGet;


    GT_U32      notAppFamilyBmp;

    notAppFamilyBmp = UTF_ALL_FAMILY_E -
                    (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiTcamSegmentModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                   "get another segment mode than was set: %d", dev);

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTcamSegmentModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                       "get another segment mode than was set: %d", dev);
        }

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTcamSegmentModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                       "get another segment mode than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTcamSegmentModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                       "get another segment mode than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTcamSegmentModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                       "get another segment mode than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MPLS_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChTtiTcamSegmentModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                       "get another segment mode than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_ETH_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiTcamSegmentModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                   "get another segment mode than was set: %d", dev);

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_MIM_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiTcamSegmentModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                   "get another segment mode than was set: %d", dev);

        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE5_E;
        segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;

        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiTcamSegmentModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(segmentMode, segmentModeGet,
                   "get another segment mode than was set: %d", dev);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode),
                            keyType);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode),
                            segmentMode);
    }

    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    segmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTcamSegmentModeSet(dev, keyType, segmentMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiTcamSegmentModeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiPacketTypeKeyTypeGet with relevant value
              keyType[CPSS_DXCH_TTI_KEY_IPV4_E/
                      CPSS_DXCH_TTI_KEY_MPLS_E/
                      CPSS_DXCH_TTI_KEY_ETH_E/
                      CPSS_DXCH_TTI_KEY_MIM_E/
                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E]
              and not NULL segmentModePtr.
    Expected: GT_OK.
    1.2. Call with wrong enum value keyType.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL valuePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode = GT_FALSE;


    GT_U32      notAppFamilyBmp;

    notAppFamilyBmp = UTF_ALL_FAMILY_E -
                    (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentMode),
                            keyType);

        /*  1.3.  */
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    keyType = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiTcamSegmentModeGet(dev, keyType, &segmentMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMcTunnelDuplicationModeSet)
{
    /*
        ITERATE_DEVICES_VIRT_PORTS
        1.1.1. Call with relevant values
               protocol[CPSS_TUNNEL_MULTICAST_IPV4_E/
                        CPSS_TUNNEL_MULTICAST_IPV6_E/
                        CPSS_TUNNEL_MULTICAST_MPLS_E/
                        CPSS_TUNNEL_MULTICAST_TRILL_E/
                        CPSS_TUNNEL_MULTICAST_PBB_E],
               mode[CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E/
                    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E/
                    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_GRE_E/
                    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_UDP_PORT_E/
                    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E]
        Expected: GT_OK.
        1.1.2. Call cpssDxChTtiMcTunnelDuplicationModeGet.
        Expected: GT_OK and the same mode.
        1.1.3. Call with wrong enum values protocol.
        Expected: GT_BAD_PARAM.
        1.1.4. Call with wrong enum values mode.
        Expected: GT_BAD_PARAM.
    */
        GT_STATUS           st              = GT_OK;
        GT_U8               devNum          = 0;
        GT_PORT_NUM         portNum         = 0;

        CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;
        CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;
        CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     modeGet = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;


        GT_U32              notAppFamilyBmp = 0;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            st = prvUtfNextDefaultEPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
            {
                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;
                mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;

                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiMcTunnelDuplicationModeGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", devNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_IPV6_E;
                mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E;

                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiMcTunnelDuplicationModeGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", devNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_TRILL_E;
                mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;

                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiMcTunnelDuplicationModeGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", devNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_MPLS_E;
                mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E;

                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiMcTunnelDuplicationModeGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", devNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_PBB_E;
                mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E;

                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTtiMcTunnelDuplicationModeGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", devNum);

                /*  1.1.3.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTtiMcTunnelDuplicationModeSet
                                    (devNum, portNum, protocol, mode),
                                    protocol);

                /*  1.1.4.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTtiMcTunnelDuplicationModeSet
                                    (devNum, portNum, protocol, mode),
                                    mode);
            }
            portNum = 0;
            protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;
            mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;

            st = prvUtfNextDefaultEPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                           protocol, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

            st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                       protocol, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                             */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                       protocol, mode);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

        }

        /* 2. For not active devices check that function returns non GT_OK. */
        portNum = 0;
        protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;
        mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                       protocol, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                   protocol, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMcTunnelDuplicationModeGet)
{
    /*
        ITERATE_DEVICES_VIRT_PORTS
        1.1.1. Call with relevant values
               protocol[CPSS_TUNNEL_MULTICAST_IPV4_E/
                        CPSS_TUNNEL_MULTICAST_IPV6_E/
                        CPSS_TUNNEL_MULTICAST_MPLS_E/
                        CPSS_TUNNEL_MULTICAST_TRILL_E/
                        CPSS_TUNNEL_MULTICAST_PBB_E],
               not NULL modePtr.
        Expected: GT_OK.
        1.1.2. Call with wrong enum values protocol.
        Expected: GT_BAD_PARAM.
        1.1.3. Call with NULL modePtr.
        Expected: GT_BAD_PTR.
    */
        GT_STATUS           st              = GT_OK;
        GT_U8               devNum          = 0;
        GT_PORT_NUM         portNum         = 0;

        CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;
        CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;


        GT_U32              notAppFamilyBmp = 0;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            st = prvUtfNextDefaultEPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
            {
                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;

                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_IPV6_E;

                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_TRILL_E;

                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_MPLS_E;

                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.1.  */
                protocol = CPSS_TUNNEL_MULTICAST_PBB_E;

                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTtiMcTunnelDuplicationModeGet
                                    (devNum, portNum, protocol, &mode),
                                    protocol);

                /*  1.1.3.  */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);

            }
            portNum = 0;
            protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;

            st = prvUtfNextDefaultEPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                           protocol, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

            st = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum,
                                                       protocol, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                             */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                       protocol, &mode);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }
        }

        /* 2. For not active devices check that function returns non GT_OK. */
        portNum = 0;
        protocol = CPSS_TUNNEL_MULTICAST_IPV4_E;

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                       protocol, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum,
                                                   protocol, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMcTunnelDuplicationUdpDestPortSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiMcTunnelDuplicationUdpDestPortSet with relevant value
              udpPort[0/0xFF/0xFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiMcTunnelDuplicationUdpDestPortGet.
    Expected: GT_OK and the same udpPort.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16      udpPort = 0;
    GT_U16      udpPortGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        udpPort = 0;

        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(dev, udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPortGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMcTunnelDuplicationUdpDestPortGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                   "get another udpPort than was set: %d", dev);

        /*  1.1.  */
        udpPort = 0xFF;

        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(dev, udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPortGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMcTunnelDuplicationUdpDestPortGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                   "get another udpPort than was set: %d", dev);

        /*  1.1.  */
        udpPort = 0xFFFF;

        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(dev, udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPortGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMcTunnelDuplicationUdpDestPortGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                   "get another udpPort than was set: %d", dev);
    }
    udpPort = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(dev, udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(dev, udpPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMcTunnelDuplicationUdpDestPortGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiMcTunnelDuplicationUdpDestPortGet with not NULL udpPortPtr
    Expected: GT_OK.
    1.2. Call with NULL udpPortPtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16      udpPort = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(dev, &udpPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMplsMcTunnelTriggeringMacDaSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiMplsMcTunnelTriggeringMacDaSet with relevant value
              address[{0, 0x12, 0x34, 0xAB, 0xCD, 0xFF}/
                      {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB}],
              mask[{0xFF, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE}/
                   {0xFF, 0xFF, 0x12, 0x34, 0x56, 0x78}].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiMplsMcTunnelTriggeringMacDaGet.
    Expected: GT_OK and the same addressPtr, maskPtr.
    1.3. Call with NULL addressPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL maskPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     isEqual = GT_FALSE;

    GT_ETHERADDR     address = {{0, 0, 0, 0, 0, 0}};
    GT_ETHERADDR     mask = {{0, 0, 0, 0, 0, 0}};
    GT_ETHERADDR     addressGet = {{0, 0, 0, 0, 0, 0}};
    GT_ETHERADDR     maskGet = {{0, 0, 0, 0, 0, 0}};

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        address.arEther[0] = 0x12;
        address.arEther[1] = 0x34;
        address.arEther[2] = 0x56;
        address.arEther[3] = 0x78;
        address.arEther[4] = 0x90;
        address.arEther[5] = 0xAB;

        mask.arEther[0] = 0xFF;
        mask.arEther[1] = 0xAA;
        mask.arEther[2] = 0xBB;
        mask.arEther[3] = 0xCC;
        mask.arEther[4] = 0xDD;
        mask.arEther[5] = 0xEE;

        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        cpssOsMemSet(&addressGet, 0, sizeof(addressGet.arEther[0])*6);
        cpssOsMemSet(&maskGet, 0, sizeof(maskGet.arEther[0])*6);
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &addressGet, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMplsMcTunnelTriggeringMacDaGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&address, (GT_VOID*)&addressGet, sizeof (address.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another address than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&mask, (GT_VOID*)&maskGet, sizeof (mask.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mask than was set: %d", dev);

        /*  1.1.  */
        address.arEther[0] = 0;
        address.arEther[1] = 0x12;
        address.arEther[2] = 0x32;
        address.arEther[3] = 0xAB;
        address.arEther[4] = 0xCD;
        address.arEther[5] = 0xFF;

        mask.arEther[0] = 0xFF;
        mask.arEther[1] = 0xFF;
        mask.arEther[2] = 0x12;
        mask.arEther[3] = 0x34;
        mask.arEther[4] = 0x56;
        mask.arEther[5] = 0x78;

        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        cpssOsMemSet(&addressGet, 0, sizeof(addressGet.arEther[0])*6);
        cpssOsMemSet(&maskGet, 0, sizeof(maskGet.arEther[0])*6);
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &addressGet, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMplsMcTunnelTriggeringMacDaGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&address, (GT_VOID*)&addressGet, sizeof (address.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another address than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&mask, (GT_VOID*)&maskGet, sizeof (mask.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mask than was set: %d", dev);

        /*  1.1.  */
        address.arEther[0] = 0;
        address.arEther[1] = 0;
        address.arEther[2] = 0;
        address.arEther[3] = 0;
        address.arEther[4] = 0;
        address.arEther[5] = 0;

        mask.arEther[0] = 0xFF;
        mask.arEther[1] = 0xFF;
        mask.arEther[2] = 0xFF;
        mask.arEther[3] = 0xFF;
        mask.arEther[4] = 0xFF;
        mask.arEther[5] = 0xFF;

        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        cpssOsMemSet(&addressGet, 0, sizeof(addressGet.arEther[0])*6);
        cpssOsMemSet(&maskGet, 0, sizeof(maskGet.arEther[0])*6);
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &addressGet, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMplsMcTunnelTriggeringMacDaGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&address, (GT_VOID*)&addressGet, sizeof (address.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another address than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&mask, (GT_VOID*)&maskGet, sizeof (mask.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mask than was set: %d", dev);

        /*  1.1.  */
        address.arEther[0] = 0xFF;
        address.arEther[1] = 0xFF;
        address.arEther[2] = 0xFF;
        address.arEther[3] = 0xFF;
        address.arEther[4] = 0xFF;
        address.arEther[5] = 0xFF;

        mask.arEther[0] = 0;
        mask.arEther[1] = 0;
        mask.arEther[2] = 0;
        mask.arEther[3] = 0;
        mask.arEther[4] = 0;
        mask.arEther[5] = 0;

        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        cpssOsMemSet(&addressGet, 0, sizeof(addressGet.arEther[0])*6);
        cpssOsMemSet(&maskGet, 0, sizeof(maskGet.arEther[0])*6);
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &addressGet, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiMplsMcTunnelTriggeringMacDaGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&address, (GT_VOID*)&addressGet, sizeof (address.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another address than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&mask, (GT_VOID*)&maskGet, sizeof (mask.arEther[0])*6))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mask than was set: %d", dev);

        /*  1.3.  */
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, NULL, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.4.  */
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(dev, &address, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiMplsMcTunnelTriggeringMacDaGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiMplsMcTunnelTriggeringMacDaGet with not NULL addressPtr
         and maskPtr.
    Expected: GT_OK.
    1.2. Call with NULL addressPtr
    Expected: GT_BAD_PTR.
    1.3. Call with NULL maskPtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_ETHERADDR     address = {{0, 0, 0, 0, 0, 0}};
    GT_ETHERADDR     mask = {{0, 0, 0, 0, 0, 0}};


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, NULL, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &address, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &address, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(dev, &address, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwExceptionCmdSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiPwCwExceptionCmdSet with relevant values
              exceptionType[CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E],
              command[CPSS_PACKET_CMD_FORWARD_E/
                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E/
                      CPSS_PACKET_CMD_TRAP_TO_CPU_E/
                      CPSS_PACKET_CMD_DROP_HARD_E/
                      CPSS_PACKET_CMD_DROP_SOFT_E/
                      CPSS_PACKET_CMD_NONE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiPwCwExceptionCmdGet.
    Expected: GT_OK and the same command.
    1.3. Call with wrong enum values exceptionType.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values command.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_STATUS   expectedSt;

    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E;
    CPSS_PACKET_CMD_ENT                 command = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT                 commandGet = CPSS_PACKET_CMD_FORWARD_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E;
        }
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedSt, st,
        "cpssDxChTtiPwCwExceptionCmdGet: %d", dev);

        if (st == GT_OK)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                                         "get another command than was set: %d", dev);
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
            command = CPSS_PACKET_CMD_FORWARD_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E;
            command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedSt, st,
        "cpssDxChTtiPwCwExceptionCmdGet: %d", dev);
        if (st == GT_OK)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                       "get another command than was set: %d", dev);
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E;
        }
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedSt, st,
        "cpssDxChTtiPwCwExceptionCmdGet: %d", dev);

        if (st == GT_OK)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                       "get another command than was set: %d", dev);
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E;
        }
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedSt, st,
        "cpssDxChTtiPwCwExceptionCmdGet: %d", dev);

        if (st == GT_OK)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                       "get another command than was set: %d", dev);
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
            command = CPSS_PACKET_CMD_FORWARD_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E;
            command = CPSS_PACKET_CMD_DROP_SOFT_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedSt, st,
        "cpssDxChTtiPwCwExceptionCmdGet: %d", dev);

        if (st == GT_OK)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                       "get another command than was set: %d", dev);
        }

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPwCwExceptionCmdSet
                            (dev, exceptionType, command),
                            exceptionType);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPwCwExceptionCmdSet
                            (dev, exceptionType, command),
                            command);
    }
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwExceptionCmdSet(dev, exceptionType, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwExceptionCmdGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiPwCwExceptionCmdGet with relevant values
              exceptionType[CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E/
                    CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E],
              and not NULL commandPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values exceptionType.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL command.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_STATUS   expectedSt;

    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E;
    CPSS_PACKET_CMD_ENT                 command = CPSS_PACKET_CMD_FORWARD_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.1.  */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E;
        }
        else
        {
            exceptionType = CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E;
        }

        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiPwCwExceptionCmdGet
                            (dev, exceptionType, &command),
                            exceptionType);

        /*  1.3.  */
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwExceptionCmdGet(dev, exceptionType, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwCpuCodeBaseSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiPwCwCpuCodeBaseSet with relevant value
              cpuCodeBase[192/224/255].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiPwCwCpuCodeBaseGet.
    Expected: GT_OK and the same cpuCodeBase.
    1.3. Call with out of range cpuCodeBase[191/256].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      cpuCodeBase = 0;
    GT_U32      cpuCodeBaseGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        cpuCodeBase = 192;

        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiPwCwCpuCodeBaseGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
                   "get another cpuCodeBase than was set: %d", dev);

        /*  1.1.  */
        cpuCodeBase = 224;

        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiPwCwCpuCodeBaseGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
                   "get another cpuCodeBase than was set: %d", dev);

        /*  1.1.  */
        cpuCodeBase = 251;

        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBaseGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiPwCwCpuCodeBaseGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCodeBase, cpuCodeBaseGet,
                   "get another cpuCodeBase than was set: %d", dev);

        /*  1.3.  */
        cpuCodeBase = 252;

        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        cpuCodeBase = 191;

        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        cpuCodeBase = 192;

    }
    cpuCodeBase = 192;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwCpuCodeBaseSet(dev, cpuCodeBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiPwCwCpuCodeBaseGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiPwCwCpuCodeBaseGet with not NULL
              cpuCodeBasePtr.
    Expected: GT_OK.
    1.2. Call with NULL cpuCodeBasePtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      cpuCodeBase = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiPwCwCpuCodeBaseGet(dev, &cpuCodeBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiIPv6ExtensionHeaderSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiIPv6ExtensionHeaderSet with relevant value
              extensionHeaderId[0/1],
              extensionHeaderValue[0/128/255]
    Expected: GT_OK.
    1.2. Call cpssDxChTtiIPv6ExtensionHeaderGet.
    Expected: GT_OK and the same extensionHeaderValue.
    1.3. Call with out of range extensionHeaderId[2].
    Expected: NOT GT_OK.
    1.4. Call with out of range extensionHeaderValue[256].
    Expected: NOT GT_OK.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      extensionHeaderId = 0;
    GT_U32      extensionHeaderValue = 0;
    GT_U32      extensionHeaderValueGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        extensionHeaderId = 0;
        extensionHeaderValue = 0;

        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiIPv6ExtensionHeaderGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(extensionHeaderValue, extensionHeaderValueGet,
                   "get another extensionHeaderValue than was set: %d", dev);

        /*  1.1.  */
        extensionHeaderId = 1;
        extensionHeaderValue = 128;

        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiIPv6ExtensionHeaderGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(extensionHeaderValue, extensionHeaderValueGet,
                   "get another extensionHeaderValue than was set: %d", dev);

        /*  1.1.  */
        extensionHeaderId = 0;
        extensionHeaderValue = 255;

        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiIPv6ExtensionHeaderGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(extensionHeaderValue, extensionHeaderValueGet,
                   "get another extensionHeaderValue than was set: %d", dev);

        /*  1.3.  */
        extensionHeaderId = 2;
        extensionHeaderValue = 0;

        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        extensionHeaderId = 0;
        extensionHeaderValue = 256;

        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    extensionHeaderId = 0;
    extensionHeaderValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIPv6ExtensionHeaderSet(dev, extensionHeaderId, extensionHeaderValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiIPv6ExtensionHeaderGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiIPv6ExtensionHeaderSet with relevant value
            extensionHeaderId[0/1],
            and not NULL extensionHeaderValuePtr.
    Expected: GT_OK.
    1.2. Call with out of range
            extensionHeaderId[2],
            and not NULL extensionHeaderValuePtr.
    Expected: NOT GT_OK.
    1.3. Call with NULL extensionHeaderValuePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      extensionHeaderId = 0;
    GT_U32      extensionHeaderValue = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        extensionHeaderId = 0;

        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        extensionHeaderId = 1;

        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiIPv6ExtensionHeaderGet(dev, extensionHeaderId, &extensionHeaderValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiSourceIdBitsOverrideSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiSourceIdBitsOverrideSet with relevant value
              lookup[0/1],
              overrideBitmap[0/0x3F/0xFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiSourceIdBitsOverrideGet.
    Expected: GT_OK and the same overrideBitmap.
    1.3. Call with out of range lookup[2/3].
    Expected: GT_BAD_PARAM when device is not Bobcat2 B0 and above.
              GT_OK - overwise.
    1.4. Call with out of range lookup[4].
    Expected: NOT GT_OK.
    1.5. Call with out of range overrideBitmap[0x1000].
    Expected: NOT GT_OK.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      lookup = 0;
    GT_U32      overrideBitmap = 0;
    GT_U32      overrideBitmapGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        lookup = 0;
        overrideBitmap = 0;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmapGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiSourceIdBitsOverrideGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(overrideBitmap, overrideBitmapGet,
                   "get another overrideBitmap than was set: %d", dev);

        /*  1.1.  */
        lookup = 1;
        overrideBitmap = 0xF;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmapGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiSourceIdBitsOverrideGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(overrideBitmap, overrideBitmapGet,
                   "get another overrideBitmap than was set: %d", dev);

        /*  1.1.  */
        lookup = 1;
        overrideBitmap = 0xFFF;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmapGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiSourceIdBitsOverrideGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(overrideBitmap, overrideBitmapGet,
                   "get another overrideBitmap than was set: %d", dev);

        /*  1.3.  */
        lookup = 2;
        overrideBitmap = 0;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  1.3.  */
        lookup = 3;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  1.4.  */
        lookup = 4;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.5.  */
        lookup = 0;
        overrideBitmap = 0x1000;

        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    lookup = 0;
    overrideBitmap = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiSourceIdBitsOverrideSet(dev, lookup, overrideBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiSourceIdBitsOverrideGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiSourceIdBitsOverrideGet with relevant value
            lookup[0/1],
            and not NULL overrideBitmapPtr.
    Expected: GT_OK.
    1.2. Call with out of range lookup[2/3].
    Expected: GT_BAD_PARAM when device is not Bobcat2 B0 and above.
              GT_OK - overwise.
    1.3. Call with out of range lookup[4].
            and not NULL overrideBitmapPtr.
    Expected: NOT GT_OK.
    1.4. Call with NULL overrideBitmapPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      lookup = 0;
    GT_U32      overrideBitmap = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        lookup = 0;

        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        lookup = 1;

        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        lookup = 2;

        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  1.2.  */
        lookup = 3;
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  1.3.  */
        lookup = 4;
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        lookup = 0;
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiSourceIdBitsOverrideGet(dev, lookup, &overrideBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiGreExtensionsEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiGreExtensionsEnableSet with relevant value
              enable[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTtiGreExtensionsEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChTtiGreExtensionsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiGreExtensionsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiGreExtensionsEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChTtiGreExtensionsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiGreExtensionsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChTtiGreExtensionsEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiGreExtensionsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiGreExtensionsEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiGreExtensionsEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiGreExtensionsEnableSet with not NULL enablePtr.
    Expected: GT_OK.
    1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChTtiGreExtensionsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiGreExtensionsEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiGreExtensionsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiGreExtensionsEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeForwardingEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiFcoeForwardingEnableSet with enable = GT_TRUE.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiFcoeForwardingEnableGet.
    Expected: GT_OK and enable == GT_TRUE.
    1.3. Call cpssDxChTtiFcoeForwardingEnableSet with enable = GT_FALSE.
    Expected: GT_OK.
    1.4. Call cpssDxChTtiFcoeForwardingEnableGet.
    Expected: GT_OK and enable == GT_FALSE.
    1.5. Call cpssDxChTtiFcoeForwardingEnableGet with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE, saved_enable;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChTtiFcoeForwardingEnableGet(dev, &saved_enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        enable = GT_TRUE;
        st = cpssDxChTtiFcoeForwardingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiFcoeForwardingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

        /*  1.3.  */
        enable = GT_FALSE;
        st = cpssDxChTtiFcoeForwardingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        st = cpssDxChTtiFcoeForwardingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

        /*  1.5.  */
        st = cpssDxChTtiFcoeForwardingEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChTtiFcoeForwardingEnableSet(dev, saved_enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeForwardingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeForwardingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeForwardingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiFcoeForwardingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeEtherTypeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChTtiFcoeEtherTypeSet with etherType = 0x8906.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiFcoeEtherTypeGet.
    Expected: GT_OK and etherType == 0x8906..
    1.3. Call cpssDxChTtiFcoeEtherTypeSet with etherType = 0x8914.
    Expected: GT_OK.
    1.4. Call cpssDxChTtiFcoeEtherTypeGet.
    Expected: GT_OK and etherType == 0x8914..
    1.5. Call cpssDxChTtiFcoeEtherTypeGet with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    IN GT_U16   etherType, saved_etherType;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChTtiFcoeEtherTypeGet(dev, &saved_etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        etherType = 0x8906;
        st = cpssDxChTtiFcoeEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChTtiFcoeEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(0x8906, etherType, etherType);

        /*  1.3.  */
        etherType = 0x8914;
        st = cpssDxChTtiFcoeEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        st = cpssDxChTtiFcoeEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(0x8914, etherType, etherType);

        /* TODO: check for unsupported ethertype */
        /*
        etherType = 0x8989;
        st = cpssDxChTtiFcoeEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        */

        /*  1.5.  */
        st = cpssDxChTtiFcoeEtherTypeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChTtiFcoeEtherTypeSet(dev, saved_etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeEtherTypeSet(dev, etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiFcoeEtherTypeGet(dev, &etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeAssignVfIdEnableSet)
{
/*
    1.1. ITERATE_DEVICES_VIRT_PORT
    1.2.1. Call cpssDxChTtiFcoeAssignVfIdEnable with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChTtiFcoeAssignVfIdEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChTtiFcoeAssignVfIdEnable with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChTtiFcoeAssignVfIdEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChTtiFcoeAssignVfIdEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_PORT_NUM port = TTI_VALID_PHY_PORT_CNS;
    GT_BOOL     enable, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2.2 */
            st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, enable, enable, port);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2.4 */
            st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FALSE, enable, enable, port);

            /* 1.2.5 */
            st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

#if 0
        /* 1.2.6. For all active devices go over all non available ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.7. Call function for each non-active port */
            st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
#endif

        /*
            1.2.8. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);
        st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.2.9. For active device check that function returns GT_OK
            for CPU port number
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeAssignVfIdEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiFcoeAssignVfIdEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeExceptionCountersGet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over exception types.
    1.1.1 Call cpssDxChTtiFcoeExceptionCountersGet with not NULL counterValue pointer.
    Expected: GT_OK.
    1.1.2 Call with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
    1.2. Call with wrong exception type.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_U32                              counterValue;
    CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT    exceptionType = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        for(exceptionType = 0; exceptionType <= CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E; exceptionType++)
        {
            /*  1.1.1  */
            st = cpssDxChTtiFcoeExceptionCountersGet(dev, exceptionType, &counterValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.1.2  */
            st = cpssDxChTtiFcoeExceptionCountersGet(dev, exceptionType, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        /* 1.2 */
        st = cpssDxChTtiFcoeExceptionCountersGet(dev, exceptionType+1, &counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeExceptionCountersGet(dev, exceptionType, &counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeExceptionCountersGet(dev, exceptionType, &counterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeExceptionConfiguratiosEnableSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over exception types.
    1.1.1 Call cpssDxChTtiFcoeExceptionConfiguratiosEnableSet with GT_TRUE.
    Expected: GT_OK.
    1.1.2 Call cpssDxChTtiFcoeExceptionConfiguratiosEnableGet with not NULL enable pointer.
    Expected: GT_OK.
    1.1.3 Call with NULL enale pointer.
    Expected: GT_BAD_PTR.
    1.2.1 Call cpssDxChTtiFcoeExceptionConfiguratiosEnableSet with wrong exception type.
    Expected: GT_BAD_PARAM.
    1.2.1 Call cpssDxChTtiFcoeExceptionConfiguratiosEnableGet with wrong exception type.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_BOOL                             enable, saved_enable;
    CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT    exceptionType = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 */
        for(exceptionType = 0; exceptionType <= CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E; exceptionType++)
        {
            st = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(dev, exceptionType, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.1.1  */
            st = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(dev, exceptionType, GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.1.2  */
            st = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(dev, exceptionType, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, enable);

            /*  1.1.3  */
            st = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(dev, exceptionType, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(dev, exceptionType, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /* 1.2.1 */
        st = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(dev, exceptionType+1, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.2.2 */
        st = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(dev, exceptionType+1, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(dev, exceptionType, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(dev, exceptionType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(dev, exceptionType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeExceptionPacketCommandSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over commands.
    1.1.1 Call cpssDxChTtiFcoeExceptionPacketCommandSet.
    Expected: GT_OK.
    1.1.2 Call cpssDxChTtiFcoeExceptionPacketCommandGet.
    Expected: GT_OK.
    1.1.3 Compare command we get with command we set.
    Expected: GT_OK.
    1.2. Call cpssDxChTtiFcoeExceptionPacketCommandSet with wrong command.
    Expected: GT_BAD_PARAM.
    1.3 Call cpssDxChTtiFcoeExceptionPacketCommandGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  notAppFamilyBmp;
    CPSS_PACKET_CMD_ENT     command, temp_command, saved_command;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChTtiFcoeExceptionPacketCommandGet(dev, &saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(command = 0; command <= CPSS_PACKET_CMD_DROP_SOFT_E; command++)
        {
            /*  1.1.1  */
            st = cpssDxChTtiFcoeExceptionPacketCommandSet(dev, command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, command);

            /*  1.1.2  */
            st = cpssDxChTtiFcoeExceptionPacketCommandGet(dev, &temp_command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(command, temp_command, command);
        }
        /* 1.2 */
        st = cpssDxChTtiFcoeExceptionPacketCommandSet(dev, command+1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 */
        st = cpssDxChTtiFcoeExceptionPacketCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChTtiFcoeExceptionPacketCommandSet(dev, saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeExceptionPacketCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeExceptionPacketCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeExceptionPacketCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiFcoeExceptionPacketCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiFcoeExceptionCpuCodeSet)
{
/*
    ITERATE_DEVICES
    1.1 Call cpssDxChTtiFcoeExceptionCpuCodeSet with acceptable code.
    Expected: GT_OK.
    1.2 Call cpssDxChTtiFcoeExceptionCpuCodeSet.
    Expected: GT_OK.
    1.3 Compare code we get with code we set.
    Expected: GT_OK.
    1.4 Call cpssDxChTtiFcoeExceptionCpuCodeSet with wrong code.
    Expected: GT_BAD_PARAM.
    1.5 Call cpssDxChTtiFcoeExceptionCpuCodeSet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      notAppFamilyBmp;
    CPSS_NET_RX_CPU_CODE_ENT    code, temp_code, saved_code;

    /* to be initialized */
    code = CPSS_NET_CONTROL_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChTtiFcoeExceptionCpuCodeGet(dev, &saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1  */
        st = cpssDxChTtiFcoeExceptionCpuCodeSet(dev, CPSS_NET_FCOE_SIP_NOT_FOUND_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, code);

        /* 1.2  */
        st = cpssDxChTtiFcoeExceptionCpuCodeGet(dev, &temp_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.3  */
        UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_NET_FCOE_SIP_NOT_FOUND_E, temp_code, code);

        /* 1.4 */
        st = cpssDxChTtiFcoeExceptionCpuCodeSet(dev, CPSS_NET_ALL_CPU_OPCODES_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5 */
        st = cpssDxChTtiFcoeExceptionCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChTtiFcoeExceptionCpuCodeSet(dev, saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiFcoeExceptionCpuCodeSet(dev, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChTtiFcoeExceptionCpuCodeGet(dev, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiFcoeExceptionCpuCodeSet(dev, code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiFcoeExceptionCpuCodeGet(dev, &code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiEcnPacketCommandSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  command
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiEcnPacketCommandSet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with command [CPSS_PACKET_CMD_FORWARD_E /
                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                            CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                            CPSS_PACKET_CMD_DROP_HARD_E /
                            CPSS_PACKET_CMD_DROP_SOFT_E]
    Expected: GT_OK.
    1.2. Call cpssDxChTtiEcnPacketCommandSet with not NULL commandPtr.
    Expected: GT_OK and the same commandPtr as was set.
    1.3. Call with command [wrong enum values]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_PACKET_CMD_ENT commandArr[5] = {
                                        CPSS_PACKET_CMD_FORWARD_E,
                                        CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                        CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                        CPSS_PACKET_CMD_DROP_HARD_E,
                                        CPSS_PACKET_CMD_DROP_SOFT_E
                                     };
    GT_U32 commandArrSize = sizeof(commandArr)/sizeof(commandArr[0]);
    CPSS_PACKET_CMD_ENT command;
    GT_U32 i;
    CPSS_PACKET_CMD_ENT commandGet = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < commandArrSize; i ++)
        {

            /*
            1.1. Call with command [CPSS_PACKET_CMD_FORWARD_E /
                                    CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                    CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                    CPSS_PACKET_CMD_DROP_HARD_E /
                                    CPSS_PACKET_CMD_DROP_SOFT_E]
            Expected: GT_OK.
            */

            /* iterate with CPSS_PACKET_CMD_FORWARD_E */
            command = commandArr[i];

            st = cpssDxChTtiEcnPacketCommandSet(dev, command);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, command);

            /*
                1.2. Call cpssDxChTunnelStartEcnModeGet with not NULL commandPtr.
                Expected: GT_OK and the same commandPtr as was set.
            */
            st = cpssDxChTtiEcnPacketCommandGet(dev, &commandGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChTunnelStartEcnModeGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                        "get another ECN mode than was set: %d", dev);
        }
        /*
            1.3. Call with ecnMode [wrong enum values]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTtiEcnPacketCommandSet
                            (dev, command),
                            command);
    }

    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiEcnPacketCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiEcnPacketCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTtiEcnPacketCommandGet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTtiEcnPacketCommandGet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with not NULL commandPtr.
    Expected: GT_OK.
    1.2. Call commandPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_PACKET_CMD_ENT command = CPSS_PACKET_CMD_FORWARD_E;


    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL commandPtr.
            Expected: GT_OK.
        */
        st = cpssDxChTtiEcnPacketCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call commandPtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChTtiEcnPacketCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, commandPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiEcnPacketCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiEcnPacketCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTtiEcnCpuCodeSet)
{
/*
    ITERATE_DEVICES
    1.1 Call cpssDxChTtiEcnCpuCodeSet with acceptable code.
    Expected: GT_OK.
    1.2 Call cpssDxChTtiEcnCpuCodeSet.
    Expected: GT_OK.
    1.3 Compare code we get with code we set.
    Expected: GT_OK.
    1.4 Call cpssDxChTtiEcnCpuCodeSet with wrong code.
    Expected: GT_BAD_PARAM.
    1.5 Call cpssDxChTtiEcnCpuCodeGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      notAppFamilyBmp;
    CPSS_NET_RX_CPU_CODE_ENT    code, temp_code, saved_code;

    /* to be initialized */
    code = CPSS_NET_CONTROL_E;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChTtiEcnCpuCodeGet(dev, &saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1  */
        st = cpssDxChTtiEcnCpuCodeSet(dev, CPSS_NET_FCOE_SIP_NOT_FOUND_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, code);

        /* 1.2  */
        st = cpssDxChTtiEcnCpuCodeGet(dev, &temp_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.3  */
        UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_NET_FCOE_SIP_NOT_FOUND_E, temp_code, code);

        /* 1.4 */
        st = cpssDxChTtiEcnCpuCodeSet(dev, CPSS_NET_ALL_CPU_OPCODES_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5 */
        st = cpssDxChTtiEcnCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChTtiEcnCpuCodeSet(dev, saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTtiEcnCpuCodeSet(dev, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTtiEcnCpuCodeSet(dev, code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChTtiEcnCpuCodeGet(dev, &code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTti suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTti)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMacToMeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMacToMeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortLookupEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortLookupEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpv4OnlyTunneledEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpv4OnlyTunneledEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpv4OnlyMacToMeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpv4OnlyMacToMeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIpv4McEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIpv4McEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortMplsOnlyMacToMeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortMplsOnlyMacToMeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMacModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMacModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiExceptionCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiExceptionCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortMimOnlyMacToMeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortMimOnlyMacToMeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPclIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPclIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupMacToMeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupMacToMeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupMacModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupMacModeGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortTrillEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortTrillEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortTrillOuterVid0Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortTrillOuterVid0Get)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillCpuCodeBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillCpuCodeBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillAdjacencyCheckEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillAdjacencyCheckEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillMaxVersionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTrillMaxVersionGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiEthernetTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiEthernetTypeGet)

    /* Test for Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFillMacToMeTable)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortPassengerOuterIsTag0Or1Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortPassengerOuterIsTag0Or1Get)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpTotalLengthDeductionEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortIpTotalLengthDeductionEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIpTotalLengthDeductionValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIpTotalLengthDeductionValueGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupUserDefinedByteSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPortGroupUserDefinedByteGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiUserDefinedByteSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiUserDefinedByteGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPacketTypeKeySizeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPacketTypeKeySizeGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwSequencingSupportEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwSequencingSupportEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiExceptionCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTcamSegmentModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiTcamSegmentModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMcTunnelDuplicationModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMcTunnelDuplicationModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMcTunnelDuplicationUdpDestPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMcTunnelDuplicationUdpDestPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMplsMcTunnelTriggeringMacDaSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiMplsMcTunnelTriggeringMacDaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwExceptionCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwExceptionCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwCpuCodeBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiPwCwCpuCodeBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIPv6ExtensionHeaderSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiIPv6ExtensionHeaderGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiSourceIdBitsOverrideSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiSourceIdBitsOverrideGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiGreExtensionsEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiGreExtensionsEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeForwardingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeAssignVfIdEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeExceptionCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeExceptionConfiguratiosEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeExceptionPacketCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiFcoeExceptionCpuCodeSet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiEcnPacketCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiEcnPacketCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTtiEcnCpuCodeSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTti)

