
/*******************************************************************************
 *              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file tgfCommon.h
*
* @brief Common helper API, types, variables for enhanced UTs
*
*
* @version   1
********************************************************************************
*/
#ifndef __tgfCommon
#define __tgfCommon

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* copy field from source structure into destination structure  */
#define PRV_TGF_STC_S2D_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        (dstStcPtr)->field = (srcStcPtr)->field

/* copy field from destination structure into source structure  */
#define PRV_TGF_STC_D2S_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        (srcStcPtr)->field = (dstStcPtr)->field

/* copy field from source structure into destination structure  - for union field */
#define PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        cpssOsMemCpy((&(dstStcPtr)->field),&((srcStcPtr)->field),sizeof((dstStcPtr)->field))

/* copy field from destination structure into source structure  - for union field */
#define PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(srcStcPtr,dstStcPtr,field)

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_OUTLIF_TYPE_ENT
 *
 * @brief enumerator for outlif type
*/
typedef enum{

    /** outlif is a regular ethernet interface */
    PRV_TGF_OUTLIF_TYPE_LL_E,

    /** outlif is a DIT entry */
    PRV_TGF_OUTLIF_TYPE_DIT_E,

    /** outlif is a tunnel */
    PRV_TGF_OUTLIF_TYPE_TUNNEL_E

} PRV_TGF_OUTLIF_TYPE_ENT;

/**
* @enum PRV_TGF_TS_PASSENGER_PACKET_TYPE_ENT
 *
 * @brief Tunnel Start passenger protocol types
*/
typedef enum{

    /** ethernet */
    PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E,

    /** other (IP or MPLS) */
    PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E

} PRV_TGF_TS_PASSENGER_PACKET_TYPE_ENT;

/**
* @struct PRV_TGF_OUTLIF_INFO_STC
 *
 * @brief Defines the interface info
*/
typedef struct
{
    PRV_TGF_OUTLIF_TYPE_ENT   outlifType;

    CPSS_INTERFACE_INFO_STC   interfaceInfo;

    union {
        GT_U32  arpPtr;
        GT_U32  ditPtr;

        struct {
            PRV_TGF_TS_PASSENGER_PACKET_TYPE_ENT passengerPacketType;
            GT_U32                               ptr;
        } tunnelStartPtr;

    } outlifPointer;

} PRV_TGF_OUTLIF_INFO_STC;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCommon */

