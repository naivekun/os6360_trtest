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
* @file prvCpssDxChInfo.h
*
* @brief Includes structures definition for the use of Dx Cheetah Prestera SW.
*
*
* @version   88
********************************************************************************
*/

#ifndef __prvCpssDxChInfoh
#define __prvCpssDxChInfoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get private types */
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
/* get registers structure */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
/* get module config structure */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChModuleConfig.h>
/* get errata structure */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>
/* get CPSS definitions for port configurations */
#include <cpss/generic/port/cpssPortCtrl.h>
/* get tables structure */
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
/* get DXCH registers access APIs */
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
/* get DXCH registers addresses constants */
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
/* get the fine tuning parameters structure */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
/*needed for CPSS_DXCH_PA_UNIT_ENT*/
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
/* get CPSS definitions for private port configurations */
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>

/* get CPSS definitions for private port manager configurations */
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>

#include <cpss/common/port/private/prvCpssPortTypes.h>


/* get CPSS definitions for NotworkIf configurations */
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
/* FDB hash config */
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHash.h>
/* get CPSS definitions for policer */
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
/* generic bridge security breach types */
#include <cpss/generic/bridge/cpssGenBrgSecurityBreachTypes.h>
/* bridge Egress Filtering types */
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>

/* DxCh port mapping */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

/* multi-port-groups debug info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChMultiPortGroupsDebug.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCombo.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>

/* get CPSS definitions for private Exact Match configurations */
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatch.h>

/* log related defines and prototypes */
#include <cpss/generic/log/prvCpssLog.h>

/* CPU PORT NUMBER Definition */
#define PRV_CPSS_DXCH_CPU_PORT_NUM_CNS            63

/* NULL Port */
#define PRV_CPSS_DXCH_NULL_PORT_NUM_CNS           62

/* Some features like ingress bridge rate limit and egress shaper take care of
   ports 0..23 as ports those cannot work in XG modes.
   Only ports 24..27 are XG modes capable ports for such features. */
#define PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS        24

/* macro to get a pointer on the DxCh device
    devNum - the device id of the DxCh device

    NOTE : the macro do NO validly checks !!!!
           (caller responsible for checking with other means/macro)
*/
#ifndef CPSS_USE_MUTEX_PROFILER
#define PRV_CPSS_DXCH_PP_MAC(devNum) \
    ((PRV_CPSS_DXCH_PP_CONFIG_STC*)prvCpssPpConfig[devNum])
#else
#ifdef  WIN32
#define PRV_CPSS_DXCH_PP_MAC(devNum)    ((PRV_CPSS_DXCH_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfig(devNum,__FUNCTION__))
#else
#define PRV_CPSS_DXCH_PP_MAC(devNum)   ((PRV_CPSS_DXCH_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfig(devNum,__func__))
#endif
#endif /*CPSS_USE_MUTEX_PROFILER  */

/* check that the devFamily is one of Dx Cheetah */
#define PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum)              \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &        \
    PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)


/* check that the devFamily is one of Dx Cheetah 2 and above */
#define PRV_CPSS_DXCH2_FAMILY_CHECK_MAC(devNum)              \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH2_FUNCTIONS_SUPPORT_CNS)

/* check that the devFamily is one of Dx Cheetah 3 and above */
#define PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum)              \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH3_FUNCTIONS_SUPPORT_CNS)


/* check that the devFamily is one of DxCh xCat and above */
#define PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)        \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH_XCAT_FUNCTIONS_SUPPORT_CNS)


/* check that the devFamily is one of DxCh Lion and above */
#define PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)       \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH_LION_FUNCTIONS_SUPPORT_CNS)

/* check that the devFamily is one of DxCh Lion2 and above */
#define PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(devNum)       \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH_LION2_FUNCTIONS_SUPPORT_CNS)

/* check if the DxCh Lion2 device is DxCh Lion2_B0 device.
   RevisionID (bits 0-3) should be >= 1 for Lion2_B0 and above  */
#define PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)      \
       &&                                                                      \
    (PRV_CPSS_PP_MAC(_devNum)->revision > 0))

/* check if the DxCh Lion2 device is DxCh Lion2_B1 device.
   RevisionID (bits 0-3) should be >= 2 for Lion2_B1 and above  */
#define PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)      \
       &&                                                                      \
    (PRV_CPSS_PP_MAC(_devNum)->revision > 1))


#define PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(_devNum)                                   \
    (                                                                              \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)     \
           &&                                                                      \
        (PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)      \
    )

/* check if the DxCh Bobcat2 device is DxCh Bobcat2 device.
   RevisionID (bits 0-3) should be == 0 for Bobcat2_A0  */
#define PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(_devNum)                                \
    (                                                                              \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)     \
           &&                                                                      \
        (PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)      \
           &&                                                                      \
        (PRV_CPSS_PP_MAC(_devNum)->revision == 0)                                  \
    )

/* check if the DxCh Bobcat2 device is DxCh Bobcat2 device.
   RevisionID (bits 0-3) should be == 1 for Bobcat2_B0  */
#define PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(_devNum)                                \
    (                                                                              \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)     \
           &&                                                                      \
        (PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)      \
           &&                                                                      \
        (PRV_CPSS_PP_MAC(_devNum)->revision == 1)                                  \
    )


#define PRV_CPSS_DXCH_CETUS_CHECK_MAC(_devNum)                                                \
    (                                                                                         \
         (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)               \
           &&                                                                                 \
         (PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)        \
           &&                                                                                 \
         (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts == 0)\
    )

#define PRV_CPSS_DXCH_CAELUM_CHECK_MAC(_devNum)                                               \
    (                                                                                         \
         (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)               \
           &&                                                                                 \
         (PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)        \
           &&                                                                                 \
         (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts != 0)\
    )

/* check if device is Aldrin */
#define PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(_devNum)                                               \
    (                                                                                         \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)  ||            \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)                \
    )

/* check if device is Aldrin2 */
#define PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(_devNum)                                               \
    (                                                                                         \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)            \
    )

/* check if device is Bobcat3 */
#define PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(_devNum)                                              \
    (                                                                                         \
         (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)               \
    )

/* check if device is Falcon */
#define PRV_CPSS_DXCH_FALCON_CHECK_MAC(_devNum)                                             \
    (                                                                                       \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)               \
    )

extern GT_STATUS prvCpssDxChHwIsSerdesExists(
    IN GT_U8    devNum,
    IN GT_U32   serdesId,
    OUT GT_BOOL *isExistsPtr);
/* MACRO to allow to skip SERDESes that are not valid in loop on SERDESes */
#define PRV_CPSS_DXCH_SKIP_NON_VALID_SERDES_IN_LOOP_MAC(_devNum,_serdesId)                    \
    {                                                                                         \
        GT_BOOL isExist;                                                                      \
        GT_STATUS rc = prvCpssDxChHwIsSerdesExists(_devNum,_serdesId,&isExist);               \
        if(rc != GT_OK || isExist == GT_FALSE)                                                \
        {                                                                                     \
            /* SERDES not valid */                                                            \
            continue;                                                                         \
        }                                                                                     \
    }


/* NOTE:
    the MACRO : PRV_CPSS_DXCH_BOBCAT2_B0_AND_ABOVE_CHECK_MAC(_devNum)
    was removed.

    replaced by use the macros of next style :
        PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)
*/

/* check that the device is on of EXISTING DxCh
   return GT_BAD_PARAM or GT_NOT_APPLICABLE_DEVICE on error
*/
#define PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum)                   \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))              {\
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(devNum)); \
    }                                                         \
    if(0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum))          {\
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(devNum)); \
    }

/* check that the device is on of EXISTING DxCh
   return GT_BAD_PARAM or GT_NOT_APPLICABLE_DEVICE on error
   Obtain the functionality lock on success.
*/
#define PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(_devNum,_functionality)                   \
    CPSS_API_LOCK_MAC(_devNum,_functionality)                                           \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(_devNum))              {\
        CPSS_API_UNLOCK_MAC(_devNum,_functionality)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_devNum)); \
    }                                                         \
    if(0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(_devNum))          {\
        CPSS_API_UNLOCK_MAC(_devNum,_functionality)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_devNum)); \
    }

/* check if the DxCh XCAT device is DxCh xCat_C0 device */
#define PRV_CPSS_DXCH_XCAT_C0_ONLY_CHECK_MAC(_devNum)                           \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)        \
       &&                                                                       \
    ((PRV_CPSS_DXCH_PP_HW_INFO_TTI_MAC(_devNum).metalFix & 0x80000000) != 0))

/* check if the DxCh XCAT device is DxCh xCat_B1(A3B) device or xCat3 */
#define PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(_devNum)                       \
    (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)   \
       &&                                                                   \
     ((PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(_devNum).metalFix & 8) != 0)) ||     \
     (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))

/* check if the DxCh XCAT device is DxCh xCat_A3 device or xCat3 */
#define PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(_devNum)                       \
    (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)   \
       &&                                                                   \
     ((PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(_devNum).metalFix & 4) != 0)) ||     \
     (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))

/* check that device is DxCh xCat_A3, otherwize return GT_BAD_PARAM or
   GT_NOT_APPLICABLE_DEVICE on error */
#define PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(_devNum)    \
    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                 \
    if (! PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(_devNum))     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);


/* check that the devFamily is one of DxCh xCat2 and above */
#define PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum)         \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_DXCH_XCAT2_FUNCTIONS_SUPPORT_CNS)

/* check if the DxCh xCat3 device is DxCh xCat3_A0 device.
   RevisionID (bits 0-3) should be == 0 for xCat3_A0  */
#define PRV_CPSS_DXCH_XCAT3_A0_CHECK_MAC(_devNum)                           \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)   \
       &&                                                                   \
    (PRV_CPSS_PP_MAC(_devNum)->revision == 3))

/* check if the DxCh xCat3 device is DxCh xCat3_A1 device.
   RevisionID (bits 0-3) should be >= 4 for xCat3_A1 and above  */
#define PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(_devNum)                 \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)   \
       &&                                                                   \
    (PRV_CPSS_PP_MAC(_devNum)->revision > 3))



/* get the port type */
#define PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) \
    (((((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE))) ? \
         PRV_CPSS_PORT_GE_E:PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType)



/* get the port interface mode */
#define PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portIfMode

/* get the port speed */
#define PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portSpeed

/* get port's options i.e. which ifModes it supports */
#define PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portTypeOptions

/* get isFlexLink */
#define PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].isFlexLink

/* port serdes optimisation algorithms bitmap */
#define PRV_CPSS_DXCH_PORT_SD_OPT_ALG_BMP_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].serdesOptAlgBmp

/* check physical port -- use the generic macro */
#define PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC \
        PRV_CPSS_PHY_PORT_CHECK_MAC

/* check physical port or CPU port -- use the generic macro */
#define PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC \
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC

/* check physical port - enhanced macro for eArch support (CPU port is invalid) */
#define PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(_devNum, _portNum)            \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum))                    \
    {                                                                           \
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(_devNum,_portNum);                 \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum, _portNum);                    \
    }

/* check physical port - enhanced macro for eArch support (CPU port is valid) */
#define PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum, _portNum)            \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum))                    \
    {                                                                           \
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(_devNum,_portNum);                 \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum, _portNum);                    \
    }

/* access to the registers addresses of the DxCh device */
#define PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)  \
    (&(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddr))

/* access to the registers addresses of the 'eArch' device , registers replacing regsAddr for 'eArch' devices*/
#define PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)  \
    (&(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1))

/* access to the config module of the DxCh device */
#define PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)  \
    (&(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg))

/* access to the device object of the DxCh device */
#define PRV_CPSS_DXCH_DEV_OBJ_MAC(devNum)  \
    (&(PRV_CPSS_DXCH_PP_MAC(devNum)->devObj))

/* number of QoS Profiles */
#define PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS    72

/* number of QoS Profiles for Cheetah2 */
#define PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS    128

/* DxCh support 256 STG entries */
#define PRV_CPSS_DXCH_STG_MAX_NUM_CNS        256

/* check stp range */
#define PRV_CPSS_DXCH_STG_RANGE_CHECK_MAC(_devNum,_stpId)    \
    if((_stpId) >= PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.stgNum)     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* Maximal number of QoS Profiles  for a device */
#define PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(_devNum)                           \
    (((PRV_CPSS_PP_MAC(_devNum)->devFamily) != CPSS_PP_FAMILY_CHEETAH_E) ?   \
     (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum) == GT_TRUE) ? BIT_10 : \
     PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS : PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS)

/* check CH QoS Profile Index .
(EArch - for aligned 10 bits tables) */
#define PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(_devNum, _profileId)     \
        if((_profileId) >= (GT_U32)(PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(_devNum)))   \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* macro to check if sdma interface used for cpu traffic by current device */
#define  PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(__devNum)    \
    if(PRV_CPSS_PP_MAC(__devNum)->cpuPortMode != CPSS_NET_CPU_PORT_MODE_SDMA_E)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG)

/*  this macro used to check that the user did not give over size value of vid
    into 12 or 16 bits of HW according to the configuration in flexFieldNumBitsSupport.

    This macro replaces the usage of PRV_CPSS_VLAN_VALUE_CHECK_MAC
*/
#define PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(_devNum,_vid)    \
    if(_vid > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(_devNum))   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


/*  this macro used to check that the user did not give index out of range into vlan table. */
#define PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(_devNum,_vid)    \
    if(_vid > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(_devNum))   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


#define PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum) \
    PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum)

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_CHECK_MAC(hwDevNum) \
    PRV_CPSS_DUAL_HW_DEVICE_CHECK_MAC(hwDevNum)

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(hwDevNum,portNumber) \
        if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum) && (((hwDevNum) %2) || (portNumber) > 127)) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(hwDevNum,portNumber) \
        ((PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum) && ((portNumber) > 63)) ? \
        ((portNumber) & 0x3F) : (portNumber))

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(hwDevNum,portNumber) \
        ((PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum) && ((portNumber) > 63)) ? \
        ((hwDevNum) + 1) : (hwDevNum))

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(interfacePtr) \
        if((PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC((interfacePtr)->devPort.hwDevNum)) && \
           (((interfacePtr)->devPort.hwDevNum)%2))              \
        {                                                   \
           ((interfacePtr)->devPort.hwDevNum) &= (~BIT_0);      \
           ((interfacePtr)->devPort.portNum) |=  BIT_6;       \
        }

#define PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_PORT_MAC(hwDevNum,portNumber) \
        if((PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC((hwDevNum)) && \
           ((hwDevNum)%2)))              \
        {                                                   \
           (hwDevNum) &= (~BIT_0);       \
           (portNumber) |=  BIT_6;       \
        }

/* return number of LPM lines in ram according to number of prefixes supported.
   We use the rule of average 10Bytes for Ipv4 prefix and 20Bytes for Ipv6 prefix.
   each line holds 4 byte so one prefix hold 10 Bytes = 2.5 Lines. */
#define PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(_numOfPrefixes)        \
    ((GT_U32)(_numOfPrefixes*2.5))

/* the factor defined to make 12800 to be 16K */
#define PRV_CPSS_DXCH_SIP6_LPM_LINES_TO_PREFIX_RATIO_MAC    ((double)_16K/(20*640))

/* sip6 LPM : convert num of prefixes to lines */
#define PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_LINES_MAC(_numOfPrefixes)        \
    ((GT_U32)((_numOfPrefixes + (PRV_CPSS_DXCH_SIP6_LPM_LINES_TO_PREFIX_RATIO_MAC/2))/PRV_CPSS_DXCH_SIP6_LPM_LINES_TO_PREFIX_RATIO_MAC))

/* sip6 LPM : convert num of lines to prefixes */
#define PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(_numOfLines)        \
    ((GT_U32)(_numOfLines * PRV_CPSS_DXCH_SIP6_LPM_LINES_TO_PREFIX_RATIO_MAC))


/* macro to set where the Lion sim will have different configuration */
#define PRV_CPSS_DXCH_LION_BOOKMARK  CPSS_TBD_BOOKMARK

#define PRV_CPSS_DXCH_PORT_WITH_CPU_NUM_IN_HEMISPHERE_CNS 64

#define PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS \
    ((PRV_CPSS_DXCH_PORT_WITH_CPU_NUM_IN_HEMISPHERE_CNS + 31) / 32)

#define PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS 4

#define PRV_CPSS_LION_PORT_GROUPS_NUM_CNS  4
#define PRV_CPSS_LION2_PORT_GROUPS_NUM_CNS 8


/**
* @struct PRV_CPSS_DXCH_BRIDGE_INFO_STC
 *
 * @brief A structure to hold all PP data needed on bridge DxCh layer
*/
typedef struct{

    /** @brief bmp of HW device Ids that the FDB age daemon in PP
     *  should know about needed as part of Erratum WA.
     */
    GT_U32 devTable;

    /** FDB hash parameters. */
    PRV_CPSS_DXCH_MAC_HASH_STC fdbHashParams;

    /** @brief GT_FALSE: The SrcID field in FDB table is 9b.
     *  SrcID[11:9] are used for extending the
     *  user defined bits
     *  GT_TRUE: The SrcID filed in FDB is 12b
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  NOTE: sip5 only and not sip6. (replaced by sip6FdbMacEntryMuxingMode , sip6FdbIpmcEntryMuxingMode)
     */
    GT_BOOL maxLengthSrcIdInFdbEn;

    /** @brief GT_FALSE: <OrigVID1> is not written in the FDB and is not read from the FDB.
     *  <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
     *  and <DA Security Level> are written/read from the FDB.
     *  GT_TRUE: <OrigVID1> is written in the FDB and read from the FDB
     *  as described in Mac Based VLAN FS section.
     *  <SrcID>[8:6], <SA Security Level> and <DA Security Level>
     *  are read as 0 from the FDB entry.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  NOTE: sip5 only and not sip6. (replaced by sip6FdbMacEntryMuxingMode , sip6FdbIpmcEntryMuxingMode)
     */
    GT_BOOL tag1VidFdbEn;

    /** @brief Defines the muxing mode in case of FDB MAC entry.
     *  see enum of CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT sip6FdbMacEntryMuxingMode;

    /** @brief Defines the muxing mode in case of FDB IPMC (IPv4/Ipv6 MC) entry.
     *  see enum of CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT sip6FdbIpmcEntryMuxingMode;

    /** @brief Defines the 'drop' mode of the 'MAC SA' command in the FDB entry.
     *  the relevant options are 'HARD DROP' else 'SOFT DROP'
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_MAC_TABLE_CMD_ENT sip6FdbSaDropCommand;

    /** @brief Defines the 'drop' mode of the 'MAC DA' command in the FDB entry.
     *  the relevant options are 'HARD DROP' else 'SOFT DROP'
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_MAC_TABLE_CMD_ENT sip6FdbDaDropCommand;

    /** @brief alignment in words of the AU message
     *  (for legacy devices : 4 , for Bobcat2, Caelum, Bobcat3, Aldrin, AC3X is 8 )
     */
    GT_U32 auMessageNumOfWords;

    /** the FDB action Hw device number. */
    GT_U32 actionHwDevNum;

    /** the FDB action Hw device number mask. */
    GT_U32 actionHwDevNumMask;

    /** @brief indication that although the device is 'multi port groups'
     *  the 'FDB unit' is 'single instance' and NOT per 'port group'
     *  Bobcat3 - multi port groups but single FDB instance.
     *  Lion; Lion2 - considered FDB 'per port group'
     *  Falcon - considered FDB 'per tile' (meaning per 2 port groups).
     *  all 'non multi port groups'(ch1,2,3,xcat,xcat2 ; Bobcat2; Caelum; Aldrin ; Aldrin2)
     *      also considered 'single instance'
     */
    GT_BOOL supportSingleFdbInstance;

    /** port VLAN egress filtering table access mode. */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT portVlanfltTabAccessMode;

    /** @brief FDB aging : granularity for '1 second'
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    GT_U32 fdbAging_oneSecGranularity;

    /** @brief FDB aging : max value of seconds that can be achieved
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    GT_U32 fdbAging_maxSec;

    /** @brief Total number of entries in the "L2ECMP Table" (the host table for L2Ecmp members and trunk members)
     *  NOTE: currently the CPSS 'split' the hosting table :
     *  lower part for 'l2ecmp members' and higher part for 'trunk members'
     *  NOTE: this value is set by 'fine tunning' mechanism.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    GT_U32 l2EcmpNumMembers;

    /** @brief Total number of entries in the "L2ECMP Table" that 'reserved' for trunk members.
     *  NOTE: currently the CPSS 'split' the hosting table :
     *  lower part for 'l2ecmp members' and higher part for 'trunk members'
     *  NOTE: this value is set by 'fine tunning' mechanism.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    GT_U32 l2EcmpNumMembers_usedByTrunk;

    /** @brief the index that the trunk LIB used as 'reserved'
     *  index for 'NULL port' that needed for the 'empty trunks'
     *  NOTE: this index is included in the <l2EcmpNumMembers_usedByTrunk>
     *  of number of indexes that used by trunk LIB.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     *  keyModePerFidBmpPtr - a pointer to a bitmap storing an FDB Lookup
     *  Key Mode value per FIDs (0 - single tag mode,
     *  1 - double tag mode). When some VLAN's field
     *  <FDB Lookup Key Mode> is changed a bit with index
     *  VLAN<FID> will be assigned by an appropriate value
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    GT_U32 l2EcmpIndex_NULL_port;

    GT_U32 *keyModePerFidBmpPtr;

} PRV_CPSS_DXCH_BRIDGE_INFO_STC;

 /* get an FDB Lookup key mode (stored as a bit flag in the bitmap)for specified FID.
  * if FDB loookup key mode is double tag return 1. Otherwise 0. */
#define PRV_CPSS_DXCH_FDB_LOOKUP_KEY_MODE_PER_FID_IS_DOUBLE_MAC(devNum, fid)  \
    ((PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.keyModePerFidBmpPtr[(fid)>>5] >> ((fid) & 0x1f)) & 1 ? \
     CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E                                                          \
     : CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E)                                                            \

/* set an FDB Lookup key mode for specified FID. It is stored as a bit flag in the bitmap */
#define PRV_CPSS_DXCH_FDB_LOOKUP_KEY_MODE_PER_FID_SET_MAC(devNum, fid, value)                                  \
    if (value)                                                                                                 \
    {                                                                                                          \
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.keyModePerFidBmpPtr[(fid)>>5] |= (1 << ((fid) & 0x1f));  \
    }                                                                                                          \
    else                                                                                                       \
    {                                                                                                          \
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.keyModePerFidBmpPtr[(fid)>>5] &= ~(1 << ((fid) & 0x1f)); \
    }


/**
* @struct PRV_CPSS_DXCH_POLICER_INFO_STC
 *
 * @brief A structure to hold all PP data needed on policer DxCh layer
*/
typedef struct{

    GT_U32 memSize[3];

    /** @brief selecting calculation of HW metering
     *  parameters between CIR\PIR or CBS\PBS
     *  orientation.
     */
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT meteringCalcMethod;

    /** @brief the allowed deviation in percentage from
     *  the requested CBS\PBS. Relevant only for
     *  CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
     */
    GT_U32 cirPirAllowedDeviation;

    /** @brief GT_TRUE: If CBS\PBS constraints cannot be
     *  matched return to CIR\PIR oriented
     *  calculation.
     *  GT_FALSE: If CBS\PBS constraints cannot
     *  be matched return error.
     *  Relevant only for
     *  CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
     */
    GT_BOOL cbsPbsCalcOnFail;

    /** @brief GT_TRUE: The Metering Configuration Table of this
     *  unit is shared with at least one more PLR unit .
     *  GT_FALSE: The Metering Configuration Table not shared with
     *  other PLR units
     *  relevant for sip5_15 and above
     */
    GT_BOOL isMeterConfigTableShared[3];

    /** @brief GT_TRUE: The entry index to Metering Configuration
     *  Table of this unit should include the unit base
     *  address.
     *  GT_FALSE: The entry index to Metering Configuration Table of
     *  this unit should not include the unit base address.
     *  relevant for sip5_15 and above
     *  if isMeterConfigTableShared = GT_TRUE,
     *  meterConfigTableAddBaseAddr have to be GT_TRUE also
     */
    GT_BOOL meterConfigTableAddBaseAddr[3];


} PRV_CPSS_DXCH_POLICER_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC
 *
 * @brief A structure to hold for XLG MIB counters support DxCh layer
*/
typedef struct{

    GT_U32 mibShadow[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];

    GT_U32 captureMibShadow[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];

    /** clear on read enable */
    GT_BOOL clearOnReadEnable;

} PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC;

/**
* @struct PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC
 *
 * @brief A Structure to hold per remote MAC port 'private' info.
*/
typedef struct{

    /** whether "Clear on Read" is enabled at this remote port. */
    GT_BOOL clearOnReadEnable;

    /** Copy of last counter value read, when "Clear on Read" is enabled. */
    CPSS_PORT_MAC_COUNTER_SET_STC mibShadow;

    /** whether captureMibShadow contains valid data. */
    GT_BOOL captureMibShadowValid;

    /** Copy of last counter value read by MIB counter by packet (i.e. capture). */
    CPSS_PORT_MAC_COUNTER_SET_STC captureMibShadow;

    /** @brief offset value used for fine tuning of VCT length
     *  calculations.
     */
    GT_32 vctLengthOffset;

} PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC
 *
 * @brief A Structure to hold 'management' info about connection between
 * remote device and local device.
 * this info allow the SW on top of local device to configure a remote
 * device (for example over SMI bus).
*/

typedef struct{
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   connectedPhyMacInfo;

    GT_VOID*        drvInfoPtr;
    GT_U8           dxDevNum;

    /* ---- ISR info ----- */
    struct{
        GT_VOID*        intScanRoot; /*type (PRV_CPSS_DRV_INTERRUPT_SCAN_STC*) */
        GT_VOID*        intNodesPool;/*type (PRV_CPSS_DRV_EV_REQ_NODE_STC*)    */
        GT_U32 *        intMaskShadow;
        GT_U32          numOfIntBits;
    }interrupts;

    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoArr;
}PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC;


/**
* @struct PRV_CPSS_DXCH_PORT_INFO_STC
 *
 * @brief A structure to hold all PP data needed on port DxCh layer
*/
typedef struct{

    /** SERDES reference clock type. */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT serdesRefClock;

    /** @brief number of bits from the source port
     *  that are used to index the port isolation table.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 portIsolationLookupPortBits;

    /** @brief number of bits from the source device
     *  that are used to index the port isolation table.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 portIsolationLookupDeviceBits;

    /** @brief number of bits from the trunk ID
     *  that are used to index the port isolation table.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 portIsolationLookupTrunkBits;

    /** @brief the first index of the trunk ID
     *  based lookup. The default value is 2048 (0x800)
     *  for backward compatibility.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  portsMibShadowArr - array of per port MAC MIB counters shadows.
     *  Used to simulate support for single counter read,
     *  clear on read disable and capture for XLG/MSM MIBs.
     *  NOTE: indexed by 'mac portNum'
  *      physicalPortToAnode - mapping between physical port number  to scheduler A nodes.(APPLICABLE DEVICES: Falcon)
     */
    GT_U32 portIsolationLookupTrunkIndexBase;

    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *portsMibShadowArr       [PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

    CPSS_DXCH_DETAILED_PORT_MAP_STC portsMapInfoShadowArr   [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    /** @brief shadow of txq port mapping to primary scheduler (PDQ)  and bandwith.
   *  Contain also queue to port mapping.
   *  (APPLICABLE DEVICES: Falcon)
   */
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION                                                     tileConfigs[4];

    /** @brief Mapping between DMA and scheduler P nodes.
   *  (APPLICABLE DEVICES: Falcon)
   */
    PRV_CPSS_DXCH_TXQ_SIP_6_DMA_TO_PNODE_MAP_STC                                dmaToPnode[PRV_CPSS_MAX_DMA_NUM_CNS  ];

    /** @brief Mapping between physical port numbers and tile/A node
   *   (APPLICABLE DEVICES: Falcon)
   */
    PRV_CPSS_DXCH_TXQ_SIP_6_PHYSICAL_PORT_TO_ANODE_MAP_STC       physicalPortToAnode[CPSS_MAX_PORTS_NUM_CNS];

    /** @brief Shadow of txq scheduler profiles
   *   (APPLICABLE DEVICES: Falcon)
   */
    PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE                                                     schedProfiles[CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM];

    GT_PHYSICAL_PORT_NUM portsMac2PhyscalConvArr [PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

    CPSS_DXCH_PORT_COMBO_PARAMS_STC comboPortsInfoArray     [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT prbsMode                [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    GT_BOOL prbsSerdesCountersClearOnReadEnable[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS][PRV_CPSS_MAX_PORT_LANES_CNS];

    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoArr[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

    /** @brief indication that all MIB counters of the port are '64 bits'.
     *  Bobcat3.
     *  gtMacCounterOffset   - MIB counters offsets for none CPU GMII interface
     *  reservedPortsPtr    - (pointer to) reserved ports array
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X)
     */
    GT_BOOL portMibCounters64Bits;

    GT_U8 *gtMacCounterOffset;

    GT_PHYSICAL_PORT_NUM *reservedPortsPtr;

    PRV_CPSS_PORT_MNG_DB_STC portManagerDb;

} PRV_CPSS_DXCH_PORT_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT
 *
 * @brief Enumeration of the FDB modes
*/
typedef enum{

    /** @brief value for 'non-multi port groups device'
     */
    PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_NON_MULTI_PORT_GROUP_DEVICE_E = 0,

    /** @brief all port groups have the
     *  same FDB entries. (unified tables) (except SP entries)
     */
    PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E,

    /** @brief the port groups of
     *  the device are split to 2 groups .
     *  Each port group in those groups may hold different FDB entries.
     *  But the 2 groups hold the same entries. (except SP entries)
     *  (APPLICABLE DEVICES: Lion; Lion2)
     */
    PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E,

    /** @brief each port group of
     *  the device may hold different FDB entries.
     *  (APPLICABLE DEVICES: Lion; Lion2)
     */
    PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_LINKED_E

} PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT;

/**
* @struct PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC
 *
 * @brief A structure to hold info about the port-group resources : credits/headers/payloads/BW
 * the info used ALSO by the non multi-port-groups device (core == DP == data path)
*/
typedef struct{

    GT_U32 usedDescCredits[MAX_DP_CNS];

    GT_U32 maxDescCredits[MAX_DP_CNS];

    GT_U32 usedPayloadCredits[MAX_DP_CNS];

    GT_U32 maxPayloadCredits[MAX_DP_CNS];

    GT_U32 usedHeaderCredits[MAX_DP_CNS];

    GT_U32 maxHeaderCredits[MAX_DP_CNS];

    /** BW accumulated at TM */
    GT_U32 trafficManagerCumBWMbps;

    GT_U32 coreOverallSpeedSummary[MAX_DP_CNS];

    GT_U32 coreOverallSpeedSummaryTemp[MAX_DP_CNS];

} PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC;


/**
* @struct PRV_CPSS_DXCH_PP_PORT_GROUPS_INFO_STC
 *
 * @brief A structure to hold info about the port-group of a device
 * the info used ALSO by the non multi-port-groups device
*/
typedef struct
{
    PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_STC   debugInfo;

    PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_ENT fdbMode;

    struct
    {
        CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT    counterMode;
        GT_U32      portGroupId;
    } securBreachDropCounterInfo;

    struct
    {
        GT_U32      portGroupId;
    } portEgressCntrModeInfo[2];

    struct
    {
        GT_U32      portGroupId;
    } bridgeIngressCntrMode[2];

    struct
    {
        GT_U32      portGroupId;
    } cfgIngressDropCntrMode;

    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC groupResorcesStatus;

} PRV_CPSS_DXCH_PP_PORT_GROUPS_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_DEV_OBJ_STC
 *
 * @brief The structure defines the device object
*/
typedef struct{

    /** Port configuration functions structure pointer */
    PRV_CPSS_DXCH_PORT_OBJ_PTR portPtr;

} PRV_CPSS_DXCH_DEV_OBJ_STC;

/**
* @struct PRV_CPSS_DXCH_DIAG_STC
 *
 * @brief The structure defines diag params
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - disable TCAM parity calculations
     */
    GT_BOOL tcamParityCalcEnable;

} PRV_CPSS_DXCH_DIAG_STC;

/**
* @struct PRV_CPSS_DXCH_EXT_MEMORY_STC
 *
 * @brief The structure defines External Memory params
*/
typedef struct{

    /** @brief External Memory Init Flags
     *  mvHwsDevTopologyMapPtr - (pointer to)device Topology Map for external memories
     */
    GT_U32 externalMemoryInitFlags;

    void *mvHwsDevTopologyMapPtr;

} PRV_CPSS_DXCH_EXT_MEMORY_STC;

/**
* @struct PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC
 *
 * @brief Port SERDES TX/RX Polarity configuration parameters.
*/
typedef struct{

    /** Tx Polarity */
    GT_BOOL txPolarity;

    /** Rx Polarity */
    GT_BOOL rxPolarity;

} PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC;


/*
* @struct PRV_CPSS_DXCH_HW_TABLES_SHADOW_STC

 *
 * @brief This struct defines HW table shadow DB info (per HW table)
*/
typedef struct{

    GT_U32 *hwTableMemPtr;

    /** type of the shadow */
    CPSS_DXCH_SHADOW_TYPE_ENT shadowType;

    /** @brief size of the memory in hwTableMemPtr (in bytes)
     *  NOTE: relevant only when shadow type is CPSS_DXCH_SHADOW_TYPE_CPSS_E
     */
    GT_U32 hwTableMemSize;

} PRV_CPSS_DXCH_HW_TABLES_SHADOW_STC;



#define PRV_CPSS_DXCH_DYNAMIC_PA_SLICE_NUM_CNS 2000

/*----------------------------------------------------*/
/*   Port Data Base                                   */
/*----------------------------------------------------*/
typedef struct
{
    GT_PHYSICAL_PORT_NUM  physicalPortNum;
    GT_BOOL               isInitilized;
    GT_U32                portSpeedInMBit;
    GT_U32                ilknChannelSpeedInMBit;
}PRV_CPSS_DXCH_PORT_PA_SINGLE_PORT_DB_STC;

/*----------------------------------------------------------------------------*
 *   PA special TXQ client group (used for extender-cascade-ports clients)    *
 *          Init (groupId)                                                    *
 *          Add                                                               *
 *          Remove                                                            *
 *  example :                                                                 *
 *       type    phys Txq  BWMbps                                             *
 *      -------                                                               *
 *       ccfc    xxx  70   1000                                               *
 *       remote  101  51   1000                                               *
 *       remote  102  51   1000                                               *
 *----------------------------------------------------------------------------*/
#define PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_SIZE_CNS 20
#define PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_NUM_CNS  36

typedef enum
{
     PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_UNDEFINED_E   = 0
    ,PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_CCFC_E        = 1
    ,PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_REMOTE_PORT_E = 2
}PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT;

typedef struct
{
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT clientType;
    GT_PHYSICAL_PORT_NUM                      physicalPortNum;
    GT_U32                                    txqPort;
    GT_U32                                    speedMbps;
}PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC;

typedef struct
{
    GT_U32  groupId;
    GT_BOOL isExtendedCascadeExist;
    GT_U32  clientListSize;
    GT_U32  clientListLen;
    GT_U32  groupSpeedMbps;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC clientIdList[PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_SIZE_CNS];
    GT_U32  tmp_groupTxqClient;
}PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC;

typedef struct
{
    GT_U32 groupId; /* TXQ is used for group id */
    GT_U32 place;
}PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_ID_PLACE_STC;

typedef struct
{
    GT_U32 listSize;
    GT_U32 listLen;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_ID_PLACE_STC groupIdPlaceList[PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_NUM_CNS];
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC          groupDB         [PRV_CPSS_DXCH_PA_SPECIAL_TXQ_CLIENT_GROUP_NUM_CNS];  /* group id is used as index in groupDB */
}PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC;


/*---------------------------------------------------------------------*
 *   PA client BW database for clinets having fixed BW (provisioning)  *
 *      operations:                                                    *
 *          Init                                                       *
 *          Add/Update                                                 *
 *          Remove                                                     *
 *  example (on base of TM):                                           *
 *       TxQ    64    25000                                            *
 *       TxDMA  73    25000                                            *
 *       TxFIFO 73    25000                                            *
 *---------------------------------------------------------------------*/

typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT unitType;
    GT_U32                clinetId;
    GT_U32                bwMbps;
}PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC;

typedef struct
{
    GT_U32                                 len;
    GT_U32                                 size;
    PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC  arr[20];
}PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC;

/* Pizza DB is used for SIP_5 device and not used for SIP_6
   BC3 supports 512 ports*/
#define PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS 512

typedef struct
{
    GT_U32                                        numInitPorts;
    PRV_CPSS_DXCH_PORT_PA_SINGLE_PORT_DB_STC      prv_portDB[PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS];
    /* Speed factor in percent. Index in array is global MAC number */
    GT_U16                                        txFifoSpeedFactorArr[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];
    GT_U32                                        ilknSpeedResolutionMbps;
    GT_U32                                        ilknIFnaxBWMbps;
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC           paClientBWList;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC paSpecialTxQClientGroupDB;
}PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC;

typedef struct
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC             paPortDB;
    GT_U32                                        paUnitPipeBWInGBitArr[CPSS_DXCH_PA_UNIT_MAX_E];
    GT_U32                                        paMinSliceResolutionMbps;
    struct PRV_CPSS_DXCH_PA_WORKSPACE_STC        *paWsPtr;
    struct PRV_CPSS_DXCH_BC2_PA_UNIT_STCT        *paUnitDrvList;
}PRV_CPSS_DXCH_PA_DATA_STC;

/* macro to check that the PHA library was initialized properly */
/* as all PHA related APIs are not allowed if not initialized first */
#define PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(_devNum)                 \
    if(GT_FALSE == PRV_CPSS_DXCH_PP_MAC(_devNum)->phaInfo.phaInitDone) \
    {                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,             \
        "the PHA library was NOT initialized");                       \
    }

/* the max value that the phaThreadId parameter support */
#define PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC         255
/* the min value that the phaThreadId parameter support */
#define PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC         1

/* macro to check that the PHA threadId is valid value */
#define PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(_devNum,_phaThreadId)   \
    if(_phaThreadId < PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC ||         \
       _phaThreadId > PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC)            \
    {                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                   \
        "the PHA library was NOT initialized");                       \
    }


/**
* @struct PRV_CPSS_DXCH_PHA_INFO_STC
 *
 * @brief A structure to hold for PHA internal info needed by the CPSS
 *  NOTE: relevant to sip6 devices
*/
typedef struct{
    /** @brief indication that the library was initialized
     */
    GT_BOOL  phaInitDone;
    /** @brief indication to use '0' (ZERO) or 'actual value' for <Skip Counter>
        GT_TRUE  - use 'actual value' for <Skip Counter>
        GT_FALSE - use '0' (ZERO) for <Skip Counter>
     */
    GT_BOOL  packetOrderChangeEnable;
    /** @brief hold array of the thread types . used by 'Get' API when a specific
        'instruction pointer' is represented by more than single 'SW thread type'.
     */
    GT_U32  threadTypeArr[PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC+1];
    /** @brief hold array of the source physical port type . used by 'Get' API to
        know how to parse the HW info.

        actual type is :CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT
     */
    GT_U32  sourcePortEntryTypeArr [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    /** @brief hold array of the target physical port type . used by 'Get' API to
        know how to parse the HW info.
        actual type is :CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT
     */
    GT_U32  targetPortEntryTypeArr [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

}PRV_CPSS_DXCH_PHA_INFO_STC;


/**
* @struct PRV_CPSS_DXCH_PP_CONFIG_STC
 *
 * @brief A structure to hold all PP data needed in CPSS DxCh
*/
typedef struct{

    /** generic info structure */
    PRV_CPSS_GEN_PP_CONFIG_STC genInfo;

    /** @brief Module configuration parameters such as tables addresses ,
     *  num entries , entries size , entries offset
     */
    PRV_CPSS_DXCH_MODULE_CONFIG_STC moduleCfg;

    /** @brief registers addresses of the device
     *  (relevant for this device type).
     */
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC regsAddr;

    /** registers replacing regsAddr for 'eArch' devices */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC regsAddrVer1;

    /** @brief info about what Errata should be fixed in the CPSS for the
     *  device (depend on : revision , devType)
     */
    PRV_CPSS_DXCH_ERRATA_STC errata;

    /** @brief tables sizes and parameters that define the "fine tuning"
     *  of the specific device.
     */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC fineTuning;

    /** @brief needed bridge info on the device
     *  accessTableInfoPtr - pointer direct access table entry info.
     */
    PRV_CPSS_DXCH_BRIDGE_INFO_STC bridge;

    PRV_CPSS_DXCH_TABLES_INFO_STC *accessTableInfoPtr;

    /** number of entries in 'accessTableInfoPtr'. */
    GT_U32 accessTableInfoSize;

    /** info needed by the network interface. */
    PRV_CPSS_DXCH_NET_INFO_STC netIf;

    /** info needed by the policer */
    PRV_CPSS_DXCH_POLICER_INFO_STC policer;

    /** needed port info on the device */
    PRV_CPSS_DXCH_PORT_INFO_STC port;

    /** @brief extra info needed by several libraries but concern
     *  'multi-port-groups' device support
     */
    PRV_CPSS_DXCH_PP_PORT_GROUPS_INFO_STC portGroupsExtraInfo;

    /** info about PP (HW parameters) */
    PRV_CPSS_DXCH_PP_HW_INFO_STC hwInfo;

    /** @brief device object with pointers to device/family
     *  specific functions
     *  serdesCfgDbArrPtr - DB of serdes tuning values per lane per serdes
     *  frequency
     */
    PRV_CPSS_DXCH_DEV_OBJ_STC devObj;

    CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR *serdesCfgDbArrPtr;

    /** DB for diagnostic features */
    PRV_CPSS_DXCH_DIAG_STC diagInfo;

    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC tableFormatInfo[PRV_CPSS_DXCH_TABLE_LAST_FORMAT_E];

    /** External Memory configuration */
    PRV_CPSS_DXCH_EXT_MEMORY_STC extMemory;

    PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC *serdesPolarityPtr;

    PRV_CPSS_DXCH_HW_TABLES_SHADOW_STC shadowInfoArr[CPSS_DXCH_TABLE_LAST_E];

    /** @brief PA related data base
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    PRV_CPSS_DXCH_PA_DATA_STC paData;

    /** @brief cut Through Enable used to optimize packet buffers using configuration
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL cutThroughEnable;

    PRV_CPSS_DXCH_SIP6_PDQ_SCHED_DB_STC pdqInfo;

    /** @brief Exact Match parameters needed for Expander operation
     *  (APPLICABLE DEVICES: Falcon)
     */
    PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_STC       exactMatchInfo;

    GT_U32                              tc4PfcToEgfIndex[8][128];


    PRV_CPSS_DXCH_PHA_INFO_STC      phaInfo;/* sip6 PHA info */
} PRV_CPSS_DXCH_PP_CONFIG_STC;

/* array of pointers to the valid devices */
extern void* prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

#if defined  CPSS_USE_MUTEX_PROFILER
extern void * prvCpssGetPrvCpssPpConfig(    IN GT_U32      devNum,IN const char *               functionName);
#endif


#define PRV_CPSS_DXCH_LION2_REV_A0 0
#define PRV_CPSS_DXCH_LION2_REV_B0 1
#define PRV_CPSS_DXCH_LION2_REV_B1 2

/* macro to validate the value of SDMA queue parameter (DxCh format)
    queue - queue number

*/
#define PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,queue)                                 \
{                                                                               \
    GT_STATUS _rc;                                                              \
    GT_U32    _numOfNetIfs;                                                     \
    _rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&_numOfNetIfs);             \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, LOG_ERROR_NO_MSG);                   \
    }                                                                           \
    if((queue) >= (_numOfNetIfs * 8) || queue >= NUM_OF_SDMA_QUEUES )           \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);          \
    }                                                                           \
}

/* check that the port number is valid physical port (CPU port is valid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum)     \
{                                                            \
    GT_BOOL   _isCpu;                                        \
    GT_STATUS _rc;                                           \
    _rc = prvCpssDxChPortPhysicalPortMapIsCpuGet(devNum,portNum,&_isCpu);\
    if(_rc != GT_OK)                                         \
    {                                                        \
        return _rc;                                          \
    }                                                        \
    if(_isCpu == GT_FALSE)                                   \
        PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,portNum)          \
}

/* macro to check TXQ of SIP5 devices supported */
/* SIP6 devices not support the SIP5 TXQ ... so all APIs that try to access those
   registers will fail (until updated to SIP6 TXQ or other unit replacement) */
#define PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(_devNum) \
    if(GT_TRUE == PRV_CPSS_SIP_6_CHECK_MAC(_devNum))        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "SIP6 device not supports SIP5 TXQ")

/* bookmark to allow SIP6 device to return GT_OK on functions that need implementation
    but not implemented yet.
   (function mandatory for initialization sequence)
*/
#define CPSS_BOOKMARK_TBD_FALCON_TXQ


/* macro to check 'Packet generator' of SIP5 devices supported */
/* SIP6 devices not support the SIP5 'Packet generator' ... so all APIs that try to access those
   registers will fail (until updated to SIP6 'Packet generator' or other unit replacement) */
#define PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(_devNum) \
    if(GT_TRUE == PRV_CPSS_SIP_6_CHECK_MAC(_devNum))        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "SIP6 device not supports SIP5 'Packet generator'")


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChInfoh */

