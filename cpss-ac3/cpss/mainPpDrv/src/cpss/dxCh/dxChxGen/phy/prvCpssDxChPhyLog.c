/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhyLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhyLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/phy/private/prvCpssGenPhyLog.h>


/********* enums *********/

PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT);
}


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi0 = {
     "autoPollNumOfPortsSmi0", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi1 = {
     "autoPollNumOfPortsSmi1", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi2 = {
     "autoPollNumOfPortsSmi2", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi3 = {
     "autoPollNumOfPortsSmi3", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_ERRATA_WA_ENT_errataWa = {
     "errataWa", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_ERRATA_WA_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_SMI_INTERFACE_ENT_smiInterface = {
     "smiInterface", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_SMI_INTERFACE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor = {
     "divisionFactor", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_smiInterface = {
     "smiInterface", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_XSMI_INTERFACE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_xsmiInterface = {
     "xsmiInterface", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_XSMI_INTERFACE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VCT_ACTION_ENT_vctAction = {
     "vctAction", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_VCT_ACTION_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VCT_CONFIG_STC_PTR_configPtr = {
     "configPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_VCT_CONFIG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_offset = {
     "offset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableXsmi = {
     "enableXsmi", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invertMDC = {
     "invertMDC", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_useExternalPhy = {
     "useExternalPhy", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_HEX_data = {
     "data", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U16_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_phyReg = {
     "phyReg", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U16)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_autoMediaSelect = {
     "autoMediaSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_smiInterfacePortGroup = {
     "smiInterfacePortGroup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyAddr = {
     "phyAddr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyDev = {
     "phyDev", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyId = {
     "phyId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyReg = {
     "phyReg", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi0Ptr = {
     "autoPollNumOfPortsSmi0Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi1Ptr = {
     "autoPollNumOfPortsSmi1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi2Ptr = {
     "autoPollNumOfPortsSmi2Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi3Ptr = {
     "autoPollNumOfPortsSmi3Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_SMI_INTERFACE_ENT_PTR_smiInterfacePtr = {
     "smiInterfacePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PHY_SMI_INTERFACE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr = {
     "divisionFactorPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_XSMI_INTERFACE_ENT_PTR_smiInterfacePtr = {
     "smiInterfacePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PHY_XSMI_INTERFACE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR_extendedCableStatusPtr = {
     "extendedCableStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_VCT_CABLE_EXTENDED_STATUS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_VCT_CABLE_STATUS_STC_PTR_cableStatusPtr = {
     "cableStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_VCT_CABLE_STATUS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_offsetPtr = {
     "offsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableXsmiPtr = {
     "enableXsmiPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertMDCPtr = {
     "invertMDCPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_HEX_PTR_dataPtr = {
     "dataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_autoMediaSelectPtr = {
     "autoMediaSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_smiInterfacePortGroupPtr = {
     "smiInterfacePortGroupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_phyAddFromDbPtr = {
     "phyAddFromDbPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_phyAddFromHwPtr = {
     "phyAddFromHwPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssVctCableStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_VCT_ACTION_ENT_vctAction,
    &DX_OUT_CPSS_VCT_CABLE_STATUS_STC_PTR_cableStatusPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssVctSetCableDiagConfig_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_VCT_CONFIG_STC_PTR_configPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssVctLengthOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_GT_32_offset
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssVctLengthOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_OUT_GT_32_PTR_offsetPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyAutoPollNumOfPortsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi0,
    &DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi1,
    &DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi2,
    &DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi3
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyXsmiMdcDivisionFactorSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_xsmiInterface,
    &DX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyXsmiMdcDivisionFactorGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_xsmiInterface,
    &DX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyBobcat2SmiXsmiMuxSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_enableXsmi
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyXSmiMDCInvertSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_invertMDC
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssVctCableExtendedStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_OUT_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR_extendedCableStatusPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssPhyErrataWaExecute_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PHY_ERRATA_WA_ENT_errataWa
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PHY_SMI_INTERFACE_ENT_smiInterface
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhySmiMdcDivisionFactorSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortXSmiInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_smiInterface
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhySmiAutoMediaSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_autoMediaSelect
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiInterfacePortGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_smiInterfacePortGroup
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortAddrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_phyAddr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPort10GSmiRegisterWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_phyId,
    &DX_IN_GT_BOOL_useExternalPhy,
    &DX_IN_GT_U16_phyReg,
    &DX_IN_GT_U8_phyDev,
    &DX_IN_GT_U16_HEX_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPort10GSmiRegisterRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_phyId,
    &DX_IN_GT_BOOL_useExternalPhy,
    &DX_IN_GT_U16_phyReg,
    &DX_IN_GT_U8_phyDev,
    &DX_OUT_GT_U16_HEX_PTR_dataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiRegisterWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_phyReg,
    &DX_IN_GT_U16_HEX_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiRegisterRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_phyReg,
    &DX_OUT_GT_U16_HEX_PTR_dataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_PHY_SMI_INTERFACE_ENT_PTR_smiInterfacePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhySmiMdcDivisionFactorGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortXSmiInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_PHY_XSMI_INTERFACE_ENT_PTR_smiInterfacePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyAutonegSmiGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enabledPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhySmiAutoMediaSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_autoMediaSelectPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortSmiInterfacePortGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_smiInterfacePortGroupPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyPortAddrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U8_PTR_phyAddFromHwPtr,
    &DX_OUT_GT_U8_PTR_phyAddFromDbPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyAutoPollNumOfPortsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi0Ptr,
    &DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi1Ptr,
    &DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi2Ptr,
    &DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi3Ptr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyBobcat2SmiXsmiMuxGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_enableXsmiPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhyXSmiMDCInvertGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_invertMDCPtr
};


/********* lib API DB *********/

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPhyLogLibDb[] = {
    {"cpssPhyErrataWaExecute", 3, cpssPhyErrataWaExecute_PARAMS, NULL},
    {"cpssVctCableExtendedStatusGet", 3, cpssVctCableExtendedStatusGet_PARAMS, NULL},
    {"cpssVctCableStatusGet", 4, cpssVctCableStatusGet_PARAMS, NULL},
    {"cpssVctSetCableDiagConfig", 3, cpssVctSetCableDiagConfig_PARAMS, NULL},
    {"cpssVctLengthOffsetSet", 3, cpssVctLengthOffsetSet_PARAMS, NULL},
    {"cpssVctLengthOffsetGet", 3, cpssVctLengthOffsetGet_PARAMS, NULL},
    {"cpssDxChPhyPortSmiInit", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChPhyPortSmiRegisterRead", 4, cpssDxChPhyPortSmiRegisterRead_PARAMS, NULL},
    {"cpssDxChPhyPortSmiRegisterWrite", 4, cpssDxChPhyPortSmiRegisterWrite_PARAMS, NULL},
    {"cpssDxChPhyPort10GSmiRegisterRead", 7, cpssDxChPhyPort10GSmiRegisterRead_PARAMS, NULL},
    {"cpssDxChPhyPort10GSmiRegisterWrite", 7, cpssDxChPhyPort10GSmiRegisterWrite_PARAMS, NULL},
    {"cpssDxChPhyAutonegSmiGet", 3, cpssDxChPhyAutonegSmiGet_PARAMS, NULL},
    {"cpssDxChPhyAutonegSmiSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChPhySmiAutoMediaSelectSet", 3, cpssDxChPhySmiAutoMediaSelectSet_PARAMS, NULL},
    {"cpssDxChPhySmiAutoMediaSelectGet", 3, cpssDxChPhySmiAutoMediaSelectGet_PARAMS, NULL},
    {"cpssDxChPhyXsmiMdcDivisionFactorSet", 3, cpssDxChPhyXsmiMdcDivisionFactorSet_PARAMS, NULL},
    {"cpssDxChPhyXsmiMdcDivisionFactorGet", 3, cpssDxChPhyXsmiMdcDivisionFactorGet_PARAMS, NULL},
    {"cpssDxChPhySmiMdcDivisionFactorSet", 3, cpssDxChPhySmiMdcDivisionFactorSet_PARAMS, NULL},
    {"cpssDxChPhySmiMdcDivisionFactorGet", 3, cpssDxChPhySmiMdcDivisionFactorGet_PARAMS, NULL},
    {"cpssDxChPhyBobcat2SmiXsmiMuxSet", 2, cpssDxChPhyBobcat2SmiXsmiMuxSet_PARAMS, NULL},
    {"cpssDxChPhyBobcat2SmiXsmiMuxGet", 2, cpssDxChPhyBobcat2SmiXsmiMuxGet_PARAMS, NULL},
    {"cpssDxChPhyXSmiMDCInvertSet", 2, cpssDxChPhyXSmiMDCInvertSet_PARAMS, NULL},
    {"cpssDxChPhyXSmiMDCInvertGet", 2, cpssDxChPhyXSmiMDCInvertGet_PARAMS, NULL},
    {"cpssDxChPhyPortAddrSet", 3, cpssDxChPhyPortAddrSet_PARAMS, NULL},
    {"cpssDxChPhyPortAddrGet", 4, cpssDxChPhyPortAddrGet_PARAMS, NULL},
    {"cpssDxChPhyAutoPollNumOfPortsSet", 5, cpssDxChPhyAutoPollNumOfPortsSet_PARAMS, NULL},
    {"cpssDxChPhyAutoPollNumOfPortsGet", 5, cpssDxChPhyAutoPollNumOfPortsGet_PARAMS, NULL},
    {"cpssDxChPhyPortSmiInterfaceSet", 3, cpssDxChPhyPortSmiInterfaceSet_PARAMS, NULL},
    {"cpssDxChPhyPortSmiInterfaceGet", 3, cpssDxChPhyPortSmiInterfaceGet_PARAMS, NULL},
    {"cpssDxChPhyPortSmiInterfacePortGroupSet", 3, cpssDxChPhyPortSmiInterfacePortGroupSet_PARAMS, NULL},
    {"cpssDxChPhyPortSmiInterfacePortGroupGet", 3, cpssDxChPhyPortSmiInterfacePortGroupGet_PARAMS, NULL},
    {"cpssDxChPhyPortXSmiInterfaceSet", 3, cpssDxChPhyPortXSmiInterfaceSet_PARAMS, NULL},
    {"cpssDxChPhyPortXSmiInterfaceGet", 3, cpssDxChPhyPortXSmiInterfaceGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PHY(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPhyLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPhyLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

