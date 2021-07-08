/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChBrgE2Phy.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChBrgE2PhyUT, that provides
*       Eport to Physical port tables facility CPSS DxCh implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*******************************************************************************/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with physicalInfo {   type [CPSS_INTERFACE_PORT_E /
                                            CPSS_INTERFACE_TRUNK_E /
                                            CPSS_INTERFACE_VIDX_E],
                                   devPort {devNum  [devNum],
                                            portNum [0 / max/2 / max]},
                                   trunkId [1 / max/2 / max],
                                      vidx [0 / max/2 / max]}.
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
    Expected: GT_OK and the same physicalInfo.
    1.1.3. Call with physicalInfo.type [CPSS_INTERFACE_TRUNK_E],
                     out of range physicalInfo.devPort.portNum (not relevant) and
                     others valid params.
    Expected: GT_OK.
    1.1.4. Call with physicalInfo.type [CPSS_INTERFACE_PORT_E],
                     out of range physicalInfo.trunkId (not relevant) and
                     others valid params.
    Expected: GT_OK.
    1.1.5. Call with physicalInfo.type [CPSS_INTERFACE_PORT_E],
                     out of range physicalInfo.vidx (not relevant) and
                     others valid params.
    Expected: GT_OK.
    1.1.6. Call with wrong physicalInfo.type [CPSS_INTERFACE_VID_E /
                                            CPSS_INTERFACE_INDEX_E] and
                     others valid params.
    Expected: NOT GT_OK.
    1.1.7. Call with wrong physicalInfo.type enum values and
                     other valid params.
    Expected: GT_BAD_PARAM.
    1.1.8. Call with out of range physicalInfo.devPort.hwDevNum and
                     others valid params.
    Expected: NOT GT_OK.
    1.1.9. Call with out of range physicalInfo.devPort.portNum and
                     others valid params.
    Expected: NOT GT_OK.
    1.1.10. Call with out of range physicalInfo.trunkId and others valid params.
    Expected: NOT GT_OK.
    1.1.11. Call with out of range physicalInfo.vidx and others valid params.
    Expected: NOT GT_OK.
    1.1.12. Call with NULL physicalInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;

    GT_U8                               devNum;
    GT_PORT_NUM                         portNum = 0;
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    CPSS_INTERFACE_INFO_STC             physicalInfoGet;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&physicalInfo, sizeof(physicalInfo));
    cpssOsBzero((GT_VOID*)&physicalInfoGet, sizeof(physicalInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with physicalInfo {   type [CPSS_INTERFACE_PORT_E /
                                                        CPSS_INTERFACE_TRUNK_E /
                                                        CPSS_INTERFACE_VIDX_E],
                                               devPort {devNum  [devNum],
                                                        portNum [0]}}.
                Expected: GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = 0;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.hwDevNum, physicalInfoGet.devPort.hwDevNum,
                "get another physicalInfo.devPort.hwDevNum than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.portNum, physicalInfoGet.devPort.portNum,
                "get another physicalInfo.devPort.portNum than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_PORT_E],     */
            /*                           vPort {devNum  [devNum],           */
            /*                                  portNum [max/2]}}.          */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum) / 2;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.hwDevNum, physicalInfoGet.devPort.hwDevNum,
                "get another physicalInfo.devPort.hwDevNum than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.portNum, physicalInfoGet.devPort.portNum,
                "get another physicalInfo.devPort.portNum than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_PORT_E],     */
            /*                           vPort {devNum  [devNum],           */
            /*                                  portNum [max]}}.            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum) - 1;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.hwDevNum, physicalInfoGet.devPort.hwDevNum,
                "get another physicalInfo.devPort.hwDevNum than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.devPort.portNum, physicalInfoGet.devPort.portNum,
                "get another physicalInfo.devPort.portNum than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_TRUNK_E],    */
            /*                         trunkId [1]}.                        */
            physicalInfo.type               = CPSS_INTERFACE_TRUNK_E;
            physicalInfo.trunkId            = 1;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.trunkId, physicalInfoGet.trunkId,
                "get another physicalInfo.trunkId than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_TRUNK_E],    */
            /*                         trunkId [max/2]}.                    */
            physicalInfo.type    = CPSS_INTERFACE_TRUNK_E;
            physicalInfo.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) / 2);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.trunkId, physicalInfoGet.trunkId,
                "get another physicalInfo.trunkId than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_TRUNK_E],    */
            /*                         trunkId [max]}.                      */
            physicalInfo.type               = CPSS_INTERFACE_TRUNK_E;
            physicalInfo.trunkId            = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.trunkId, physicalInfoGet.trunkId,
                "get another physicalInfo.trunkId than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_VIDX_E],     */
            /*                            vidx [0]}.                        */
            physicalInfo.type               = CPSS_INTERFACE_VIDX_E;
            physicalInfo.vidx               = 0;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.vidx, physicalInfoGet.vidx,
                "get another physicalInfo.vidx than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_VIDX_E],     */
            /*                            vidx [max/2]}.                    */
            physicalInfo.type = CPSS_INTERFACE_VIDX_E;
            physicalInfo.vidx = (GT_U16) (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum) / 2);
            if(physicalInfo.vidx == 0xFFF)
            {
                physicalInfo.type = CPSS_INTERFACE_VID_E;
                physicalInfoGet.vidx = physicalInfo.vidx;/*dummy for the compare results*/
            }

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.vidx, physicalInfoGet.vidx,
                "get another physicalInfo.vidx than was set: %d, %d", devNum, portNum);

            /* Call with physicalInfo {   type [CPSS_INTERFACE_VIDX_E],     */
            /*                            vidx [max]}.                      */
            physicalInfo.type = CPSS_INTERFACE_VIDX_E;
            physicalInfo.vidx = (GT_U16) PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);
            if(physicalInfo.vidx == 0xFFF)
            {
                physicalInfo.type = CPSS_INTERFACE_VID_E;
                physicalInfoGet.vidx = physicalInfo.vidx;/*dummy for the compare results*/
            }

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortTargetMappingTableGet.
                Expected: GT_OK and the same physicalInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                     portNum,
                                                                     &physicalInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.type, physicalInfoGet.type,
                "get another physicalInfo.type than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalInfo.vidx, physicalInfoGet.vidx,
                "get another physicalInfo.vidx than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call with physicalInfo.type [CPSS_INTERFACE_TRUNK_E],
                                 out of range physicalInfo.devPort.portNum (not relevant) and
                                 others valid params.
                Expected: GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_TRUNK_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);
            physicalInfo.trunkId            = 1;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.4. Call with physicalInfo.type [CPSS_INTERFACE_PORT_E],
                                 out of range physicalInfo.trunkId (not relevant) and
                                 others valid params.
                Expected: GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = 0;
            physicalInfo.trunkId            = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum) - 1);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.5. Call with physicalInfo.type [CPSS_INTERFACE_PORT_E],
                                 out of range physicalInfo.vidx (not relevant) and
                                 others valid params.
                Expected: GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = 0;
            physicalInfo.vidx               = (GT_U16) (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum) + 1);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /*
                1.1.6. Call with wrong physicalInfo.type [CPSS_INTERFACE_VID_E /
                                                          CPSS_INTERFACE_INDEX_E] and
                                 others valid params.
                Expected: NOT GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_VID_E;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /* Calling with wrong physicalInfo.type [CPSS_INTERFACE_INDEX_E] and    */
            /*              others valid params.                                    */
            physicalInfo.type               = CPSS_INTERFACE_INDEX_E;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalInfo.type);

            /* Restore correct values. */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    = 0;

            /*
                1.1.7. Call with wrong physicalInfo.type enum values and
                                 other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
                                (devNum, portNum, &physicalInfo),
                                physicalInfo.type);

            /*
                1.1.8. Call with out of range physicalInfo.devPort.hwDevNum and
                                 others valid params.
                Expected: NOT GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(devNum);
            physicalInfo.devPort.portNum    = 0;

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            physicalInfo.type);

            /* Restore correct values. */
            physicalInfo.devPort.hwDevNum     = devNum;

            /*
                1.1.9. Call with out of range physicalInfo.devPort.portNum and
                                 others valid params.
                Expected: NOT GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum     = devNum;
            physicalInfo.devPort.portNum    =   1 << (PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 10 :
                                                      PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ?  9 :
                                                      8);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            physicalInfo.type);

            /* Restore correct values. */
            physicalInfo.devPort.portNum    = 0;

            /*
                1.1.10. Call with out of range physicalInfo.trunkId and others valid params.
                Expected: NOT GT_OK.
            */
            physicalInfo.type               = CPSS_INTERFACE_TRUNK_E;
            physicalInfo.trunkId            = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(devNum);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            physicalInfo.type);

            /* Restore correct values. */
            physicalInfo.trunkId            = 1;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(physicalInfo.trunkId);

            /*
                1.1.11. Call with out of range physicalInfo.vidx and others valid params.
                Expected: NOT GT_OK.
            */
            physicalInfo.type = CPSS_INTERFACE_VIDX_E;
            physicalInfo.vidx = (GT_U16) (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum) + 1);

            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum,
                                            physicalInfo.type);

            /* Restore correct values. */
            physicalInfo.vidx               = 0;

            /*
                1.1.12. Call with NULL physicalInfoPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, physicalInfoPtr = NULL", devNum, portNum);
        }

        physicalInfo.type               = CPSS_INTERFACE_PORT_E;
        physicalInfo.devPort.hwDevNum     = devNum;
        physicalInfo.devPort.portNum    = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                 portNum,
                                                                 &physicalInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                 portNum,
                                                                 &physicalInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    physicalInfo.type               = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum     = devNum;
    physicalInfo.devPort.portNum    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                 portNum,
                                                                 &physicalInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                             portNum,
                                                             &physicalInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_INTERFACE_INFO_STC             *physicalInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEportToPhysicalPortTargetMappingTableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call function with not NULL physicalInfoPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL physicalInfoPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = 0;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&physicalInfo, sizeof(physicalInfo));

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL physicalInfoPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &physicalInfo);

            /*
                1.1.2. Call function with NULL physicalInfoPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, physicalInfoPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum) ;

        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, &physicalInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   *egressInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with egressInfo {tunnelStart [GT_TRUE / GT_FALSE],
                                         tunnelStartPtr [0, max/2, max],
                                  tsPassengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E /
                                                         CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E],
                                                 arpPtr [0, max/2, max],
                                            modifyMacSa [GT_FALSE / GT_TRUE]
                                            modifyMacDa [GT_FALSE / GT_TRUE]}.
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
    Expected: GT_OK and the same egressInfo.
    1.1.3. Call with tunnelStartPtr [GT_TRUE],
                     out of range egressInfo.arpPtr (not relevant) and
                     others valid params.
    Expected: GT_OK.
    1.1.4. Call with tunnelStartPtr [GT_FALSE],
                     out of range egressInfo.tunnelStartPtr (not relevant) and
                     others valid params.
    Expected: GT_OK.
    1.1.5. Call with out of range egressInfo.tunnelStartPtr and
                     others valid params.
    Expected: NOT GT_OK.
    1.1.6. Call with wrong egressInfo.tsPassengerPacketType enum values and
                     other valid params.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with out of range egressInfo.arpPtr and
                     others valid params.
    Expected: NOT GT_OK.
    1.1.8. Call with NULL egressInfoPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = 0;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressInfo;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressInfoGet;
    GT_U32                                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*) &egressInfo, sizeof(egressInfo));
    cpssOsBzero((GT_VOID*) &egressInfoGet, sizeof(egressInfoGet));

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with egressInfo {tunnelStart [GT_TRUE / GT_FALSE],
                                                     tunnelStartPtr [0, max/2, max],
                                              tsPassengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E /
                                                                     CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E],
                                                             arpPtr [0, max/2, max],
                                                        modifyMacSa [GT_FALSE / GT_TRUE]
                                                        modifyMacDa [GT_FALSE / GT_TRUE]}.
                Expected: GT_OK.
            */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = 0;
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStartPtr, egressInfoGet.tunnelStartPtr,
                "get another egressInfo.tunnelStartPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tsPassengerPacketType, egressInfoGet.tsPassengerPacketType,
                "get another egressInfo.tsPassengerPacketType than was set: %d, %d", devNum, portNum);

            /* Call with egressInfo {       tunnelStart [GT_TRUE],                              */
            /*                               tunnelStartPtr [max/2],                                */
            /*                        tsPassengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E]}.  */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TS_PTR_MAC(devNum) - 1;
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStartPtr, egressInfoGet.tunnelStartPtr,
                "get another egressInfo.tunnelStartPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tsPassengerPacketType, egressInfoGet.tsPassengerPacketType,
                "get another egressInfo.tsPassengerPacketType than was set: %d, %d", devNum, portNum);

            /* Call with egressInfo {        tunnelStart [GT_TRUE],                              */
            /*                               tunnelStartPtr [max],                                  */
            /*                        tsPassengerPacketType [CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E]}.  */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TS_PTR_MAC(devNum);
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStartPtr, egressInfoGet.tunnelStartPtr,
                "get another egressInfo.tunnelStartPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tsPassengerPacketType, egressInfoGet.tsPassengerPacketType,
                "get another egressInfo.tsPassengerPacketType than was set: %d, %d", devNum, portNum);

            /* Call with egressInfo {           tunnelStart [GT_FALSE],     */
            /*                                  arpPtr [0],                 */
            /*                                  modifyMacSa [GT_FALSE],     */
            /*                                  modifyMacDa [GT_FALSE]}.    */
            egressInfo.tunnelStart              = GT_FALSE;
            egressInfo.arpPtr                   = 0;
            egressInfo.modifyMacSa              = GT_FALSE;
            egressInfo.modifyMacDa              = GT_FALSE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.arpPtr, egressInfoGet.arpPtr,
                "get another egressInfo.arpPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacSa, egressInfoGet.modifyMacSa,
                "get another egressInfo.modifyMacSa than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacDa, egressInfoGet.modifyMacDa,
                "get another egressInfo.modifyMacDa than was set: %d, %d", devNum, portNum);

            /* Call with egressInfo {           tunnelStart [GT_FALSE],     */
            /*                                  arpPtr [max/2],             */
            /*                                  modifyMacSa [GT_TRUE],      */
            /*                                  modifyMacDa [GT_FALSE]}.    */
            egressInfo.tunnelStart              = GT_FALSE;
            egressInfo.arpPtr                   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum) / 2;
            egressInfo.modifyMacSa              = GT_TRUE;
            egressInfo.modifyMacDa              = GT_FALSE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.arpPtr, egressInfoGet.arpPtr,
                "get another egressInfo.arpPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacSa, egressInfoGet.modifyMacSa,
                "get another egressInfo.modifyMacSa than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacDa, egressInfoGet.modifyMacDa,
                "get another egressInfo.modifyMacDa than was set: %d, %d", devNum, portNum);
            /* Call with egressInfo {           tunnelStart [GT_FALSE],     */
            /*                                  arpPtr [max],          */
            /*                                  modifyMacSa [GT_TRUE],      */
            /*                                  modifyMacDa [GT_TRUE]}.     */
            egressInfo.tunnelStart              = GT_FALSE;
            egressInfo.arpPtr                   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum);
            egressInfo.modifyMacSa              = GT_TRUE;
            egressInfo.modifyMacDa              = GT_TRUE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChBrgEportToPhysicalPortEgressPortInfoGet.
                Expected: GT_OK and the same egressInfo.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum,
                                                                 &egressInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.tunnelStart, egressInfoGet.tunnelStart,
                "get another egressInfo.tunnelStart than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.arpPtr, egressInfoGet.arpPtr,
                "get another egressInfo.arpPtr than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacSa, egressInfoGet.modifyMacSa,
                "get another egressInfo.modifyMacSa than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(egressInfo.modifyMacDa, egressInfoGet.modifyMacDa,
                "get another egressInfo.modifyMacDa than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call with tunnelStartPtr [GT_TRUE],
                                 out of range egressInfo.arpPtr (not relevant) and
                                 others valid params.
                Expected: GT_OK.
            */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = 0;
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            egressInfo.arpPtr                   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum) + 1;
            egressInfo.modifyMacSa              = GT_TRUE;
            egressInfo.modifyMacDa              = GT_TRUE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.4. Call with tunnelStartPtr [GT_FALSE],
                                 out of range egressInfo.tunnelStartPtr (not relevant) and
                                 others valid params.
                Expected: GT_OK.
            */
            egressInfo.tunnelStart              = GT_FALSE;
            egressInfo.tunnelStartPtr           = 0;
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            egressInfo.arpPtr                   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum) / 2;
            egressInfo.modifyMacSa              = GT_FALSE;
            egressInfo.modifyMacDa              = GT_FALSE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.5. Call with out of range egressInfo.tunnelStartPtr and
                                 others valid params.
                Expected: NOT GT_OK.
            */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TS_PTR_MAC(devNum) + 1;
            egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* Restore correct params. */
            egressInfo.tunnelStartPtr = 0;

            /*
                1.1.6. Call with wrong egressInfo.tsPassengerPacketType enum values and
                                 other valid params.
                Expected: GT_BAD_PARAM.
            */
            egressInfo.tunnelStart              = GT_TRUE;
            egressInfo.tunnelStartPtr           = 0;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet
                                (devNum, portNum, &egressInfo),
                                egressInfo.tsPassengerPacketType);

            /*
                1.1.7. Call with out of range egressInfo.arpPtr and
                                 others valid params.
                Expected: NOT GT_OK.
            */
            egressInfo.tunnelStart              = GT_FALSE;
            egressInfo.arpPtr                   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum) + 1;
            egressInfo.modifyMacSa              = GT_TRUE;
            egressInfo.modifyMacDa              = GT_TRUE;

            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* Restore correct params. */
            egressInfo.arpPtr = 0;

            /*
                1.1.8. Call with NULL egressInfoPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, egressInfoPtr = NULL", devNum, portNum);
        }

        egressInfo.tunnelStart              = GT_TRUE;
        egressInfo.tunnelStartPtr           = 0;
        egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                                 &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                              &egressInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                             &egressInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    egressInfo.tunnelStart              = GT_TRUE;
    egressInfo.tunnelStartPtr           = 0;
    egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                             &egressInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum,
                                                         &egressInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  *egressInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEportToPhysicalPortEgressPortInfoGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call function with not NULL egressInfoPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL egressInfoPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = 0;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressInfo;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*) &egressInfo, sizeof(egressInfo));

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL egressInfoPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &egressInfo);

            /*
                1.1.2. Call function with NULL egressInfoPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, egressInfoPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum) ;

        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, &egressInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgE2Phy suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgE2Phy)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEportToPhysicalPortTargetMappingTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEportToPhysicalPortEgressPortInfoGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgE2Phy)
