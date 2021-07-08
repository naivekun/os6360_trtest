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
* @file cpssDxChPha.h
*
* @brief CPSS declarations relate to PHA (programmable header alteration) in the
*   egress processing , that allows enhanced key technologies , such as:
*   In-band Network Telemetry header (INT), NSH metadata, Geneve TLVs, and any
*   new tunnel/shim header that may emerge.
*
*   APPLICABLE DEVICES:      Falcon.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPhah
#define __cpssDxChPhah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/**
 * @enum CPSS_DXCH_PHA_THREAD_TYPE_ENT
 *
 * @brief This enumeration lists all the PHA extended info types.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef enum{
    /** the threadId is not used */
    CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E,

    /** type of thread to handle IOAM Ipv4 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E,
    /** type of thread to handle IOAM Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E,
    /** type of thread to handle IOAM Ipv4 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E,
    /** type of thread to handle IOAM Ipv6 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E,

    CPSS_DXCH_PHA_THREAD_TYPE___LAST___E  /* not to be used */
}CPSS_DXCH_PHA_THREAD_TYPE_ENT;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** next are fields in the IOAM packet format */
    GT_U32  IOAM_Trace_Type;/* 16 bits value */
    GT_U32  Maximum_Length; /*  8 bits value */
    GT_U32  Flags;          /*  8 bits value */
    GT_U32  Hop_Lim;        /*  8 bits value */
    GT_U32  node_id;        /* 24 bits value */
    GT_U32  Type1;          /*  8 bits value , 'IOAM' */
    GT_U32  IOAM_HDR_len1;  /*  8 bits value */
    GT_U32  Reserved1;      /*  8 bits value */
    GT_U32  Next_Protocol1; /*  8 bits value */
    /* @brief next fields relevant only to 'e2e' option */
    GT_U32  Type2;          /*  8 bits value , 'IOAM_E2E' */
    GT_U32  IOAM_HDR_len2;  /*  8 bits value */
    GT_U32  Reserved2;      /*  8 bits value */
    GT_U32  Next_Protocol2; /*  8 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv6 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *  NOTE: the 'Hop_Lim' (8 bits value) is set according to the 'ttl' in the descriptor.
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** next are fields in the IOAM packet format */
    GT_U32  node_id;        /* 24 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4 packets in the transit switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv6 packets in the transit switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_UNT
 *
 * @brief This union if for all the PHA extended info types.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef union{
    /** dummy field for type that no extra info needed. */
    GT_U32 notNeeded;

    /** template for thread to handle IOAM Ipv4 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC  ioamIngressSwitchIpv4;
    /** template for thread to handle IOAM Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC  ioamIngressSwitchIpv6;
    /** template for thread to handle IOAM Ipv4 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC  ioamTransitSwitchIpv4;
    /** template for thread to handle IOAM Ipv6 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC  ioamTransitSwitchIpv6;

}CPSS_DXCH_PHA_THREAD_INFO_UNT;

/**
 * @enumeration CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT
 *
 * @brief This enumeration lists all the PHA Busy Stall Modes.
 * The action taken when the PHA is busy
 *  (APPLICABLE DEVICES: Falcon)
 *
*/
typedef enum{
    /** Back-pressure to Control Pipe */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E,
    /** Send to Bypass path and Drop the packet */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E,
    /** Send to PHA. PHA will not modify the packet */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E
}CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT;


/**
 * @enumeration CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT
 *
 * @brief This enumeration for the Header Modification Violation Reason -
 *      Type describes which exact header size violation occurred.
 *
*/
typedef enum{
    /** brief FW tries to increase header above the allowed expansion -
        accessible window size dependent. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E,
    /** brief Total PHA output header exceeds 255B. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E,
    /** brief FW tries to decrease accessible header to a zero or negative length-
        accessible window size dependent. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E,
    /** brief Total PHA output header is below 2B. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E
}CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT;


/**
 * @struct CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC
 *
 * @brief This structure if for the PHA common info per thread-Id.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief Statistical processing decision threshold. If 0, packet is always processed by the PHA.
     *   If 255 the packet has a 1 in 256 chance of being processed
    */
    GT_U32                                          statisticalProcessingFactor;
    /** @brief The action taken when the PHA is busy.
    */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT               busyStallMode;
    /** @brief Drop Code to set when the packet is dropped due to PHA overload */
    CPSS_NET_RX_CPU_CODE_ENT                        stallDropCode;
}CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC;


/**
* @internal cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained

*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8   devNum,
    IN GT_BOOL  packetOrderChangeEnable
);


/**
* @internal cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);

/**
* @internal cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);

/**
* @internal cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
);


/**
* @internal cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
);

typedef enum{
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_A_E,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_B_E,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_C_E,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_Z_E
}CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT;

typedef enum{
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_A_E,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_B_E,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_C_E,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_Z_E
}CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT;

typedef GT_U32  CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT;
typedef GT_U32  CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT;

/**
* @internal cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

/**
* @internal cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);



/**
* @internal cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);


/**
* @internal cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
);

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
);

/**
* @internal cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
);



/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
);

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
);

/**
* @internal cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *counterPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPhah */


