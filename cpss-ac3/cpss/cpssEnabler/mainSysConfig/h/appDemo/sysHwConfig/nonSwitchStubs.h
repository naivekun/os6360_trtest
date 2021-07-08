/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file nonSwitchStubs.h
*
* @brief App demo STUBs for 'non-switch' devices. (like PX-pipe)
*
* @version   1
********************************************************************************
*/
#ifndef __nonSwitchStubs_h
#define __nonSwitchStubs_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

#include <cpss/generic/version/gtVersion.h>

/* create stub for enum type */
#define STUB_ENUM(my_enum) \
    typedef enum {my_enum##_E} my_enum;

/* create stub for enum type */
#define STUB_STC(my_stc) \
    typedef struct{GT_U32 dummy;}  my_stc


STUB_ENUM(CPSS_SRAM_SIZE_ENT);
STUB_ENUM(CPSS_DRAM_SIZE_ENT);

STUB_STC(CPSS_DRAM_CFG_STC);

STUB_STC(CPSS_BUFF_MGMT_CFG_STC);
STUB_ENUM(CPSS_PP_UPLINK_CONFIG_ENT);
STUB_ENUM(CPSS_BUFFER_DRAM_PARAM_ENT);
STUB_ENUM(CPSS_FLOW_DRAM_PARAM_ENT);
STUB_ENUM(CPSS_NARROW_SRAM_CFG_ENT);

STUB_STC(CPSS_SRAM_CFG_STC);
STUB_ENUM(CPSS_BUF_MODE_ENT);

#define CPSS_MAX_NUMBER_OF_EXTERNAL_SRAMS_CNS   1

STUB_STC(CPSS_RLDRAM_CFG_STC);


STUB_ENUM(CPSS_AU_MESSAGE_LENGTH_ENT);

#define CPSS_MAX_NUMBER_LPM_INTERNAL_SRAMS_CNS 10

STUB_STC(CPSS_DXCH_LPM_RAM_CONFIG_STC);
STUB_STC(CPSS_REG_VALUE_INFO_STC);

#ifndef CPSS_CSCD_PORT_TYPE_ENT_DEFINED
 #define CPSS_CSCD_PORT_TYPE_ENT_DEFINED
 STUB_ENUM(CPSS_CSCD_PORT_TYPE_ENT);
#endif

STUB_STC(CPSS_CSCD_LINK_TYPE_STC);

#define CPSS_CSCD_MAX_DEV_IN_CSCD_CNS 1

typedef enum
{
    CPSS_NA_E = 0,
    CPSS_QA_E,
    CPSS_QR_E,
    CPSS_AA_E,
    CPSS_TA_E,
    CPSS_SA_E,
    CPSS_QI_E,
    CPSS_FU_E,
    CPSS_HR_E

} CPSS_UPD_MSG_TYPE_ENT;

STUB_STC(CPSS_MAC_UPDATE_MSG_EXT_STC);
STUB_STC(CPSS_MAC_ENTRY_EXT_STC);
STUB_STC(CPSS_MAC_ENTRY_EXT_KEY_STC);
STUB_ENUM(CPSS_MAC_ENTRY_EXT_TYPE_ENT);

#define AU_FDB_UPDATE_LOCK()        /*empty*/
#define AU_FDB_UPDATE_UNLOCK()      /*empty*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __nonSwitchStubs_h */



