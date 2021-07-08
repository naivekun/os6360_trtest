#include <Copyright.h>
/**
********************************************************************************
* @file gtBrgVtu.h
*
* @brief API/Structure definitions for Marvell VTU functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtBrgVtu.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell VTU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtBrgVtu_h
#define __prvCpssDrvGtBrgVtu_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Exported VTU Types                                                         */
/******************************************************************************/
/**
* @struct GT_CPSS_VTU_EXT_INFO
 *
 * @brief Extanded VTU Entry information for Priority Override and Non Rate Limit.
 * Frame Priority is used to as the tag's PRI bits if the frame egresses
 * the switch tagged. The egresss queue the frame is switch into is not
 * modified by the Frame Priority Override.
 * Queue Priority is used to determine the egress queue the frame is
 * switched into. If the frame egresses tagged, the priority in the frame
 * will not be modified by a Queue Priority Override.
 * NonRateLimit for VID is used to indicate any frames associated with this
 * VID are to bypass ingress and egress rate limiting, if the ingress
 * port's VID NRateLimit is also enabled.
*/
typedef struct{

    /** @brief Use VID frame priority override. When this is GT_TRUE and
     *  VIDFPriOverride of the ingress port of the frame is enabled,
     */
    GT_BOOL useVIDFPri;

    /** @brief data is used to override the frame priority on
     *  any frame associated with this VID.
     *  vidFPri  - VID frame priority data (0 ~ 7).
     */
    GT_U8 vidFPri;

    /** @brief Use VID queue priority override. When this is GT_TRUE and
     *  VIDQPriOverride of the ingress port of the frame is enabled,
     */
    GT_BOOL useVIDQPri;

    /** @brief data is used to override the frame priority on
     *  any frame associated with this VID.
     *  vidQPri  - VID queue priority data (0 ~ 7).
     */
    GT_U8 vidQPri;

    /** @brief Don's learn. When this bit is set to one and this VTU entry is
     *  accessed by a frame, then learning will not take place on this frame
     */
    GT_BOOL dontLearn;

    /** Filter Unicast frames. */
    GT_BOOL filterUC;

    /** Filter Broadcast frames. */
    GT_BOOL filterBC;

    /** Filter Multicast frames. */
    GT_BOOL filterMC;

    /** VTU Page */
    GT_U8 vtuPage;

} GT_CPSS_VTU_EXT_INFO;

/*
 *  definition for MEMBER_TAG
 */
 typedef enum
 {
     CPSS_MEMBER_EGRESS_UNMODIFIED=0,
     CPSS_MEMBER_EGRESS_UNTAGGED,
     CPSS_MEMBER_EGRESS_TAGGED,
     CPSS_MEMTAGP_NOT_A_MEMBER
 }GT_CPSS_MEMTAGP;

/**
* @struct GT_CPSS_VTU_ENTRY
 *
 * @brief VLAN tarnslaton unit Entry
 * Each field in the structure is device specific, i.e., some fields may not
 * be supported by the used switch device. In such case, those fields are
 * ignored by the MSD driver. Please refer to the datasheet for the list of
 * supported fields.
*/
typedef struct{

    /** database number or FID (forwarding information database) */
    GT_U16 DBNum;

    /** VLAN ID */
    GT_U16 vid;

    /** 802.1s Port State Database ID */
    GT_U8 sid;

    GT_CPSS_MEMTAGP memberTagP[CPSS_MAX_SWITCH_PORTS];

    /** @brief indicate that the frame with this VID uses VID Policy
     *  (see gprtSetPolicy API).
     */
    GT_BOOL vidPolicy;

    /** @brief extanded information for VTU entry. If the device supports extanded
     *  information, vidPriorityOverride and vidPriority values are
     *  ignored.
     */
    GT_CPSS_VTU_EXT_INFO vidExInfo;

} GT_CPSS_VTU_ENTRY;

/**
* @struct GT_CPSS_VTU_INT_STATUS
 *
 * @brief VLAN tarnslaton unit interrupt status
*/
typedef struct{

    GT_U16 vtuIntCause;

    GT_U8 spid;

    /** @brief VLAN ID
     *  if intCause is GT_CPSS_VTU_FULL_VIOLATION, it means nothing
     *  vid[12] is the vtuPage, vid[11:0] is the original vid
     */
    GT_U16 vid;

} GT_CPSS_VTU_INT_STATUS;

/**
* @enum GT_CPSS_VTU_MODE
 *
 * @brief VLAN tarnslaton unit mode
*/
typedef enum{

    /** @brief 4K vtu, page should be 0 unless overridden by TCAM
     *  CPSS_VTU_MODE_1,  8K vtu with vtu page being 0 if a frame is a unicast frame
     *  CPSS_VTU_MODE_2, - use 8K vtu with vtu page being = to the source
     *  port's VTU page bit(port offset 0x05)
     *  CPSS_VTU_MODE_3, - use 8K vtu with vtu page being a one if frame is not
     *  properly Provider tagged entering a Provider port
     */
    CPSS_VTU_MODE_0 =0,

    CPSS_VTU_MODE_1,

    CPSS_VTU_MODE_2,

    CPSS_VTU_MODE_3,

} GT_CPSS_VTU_MODE;


/*
 * Bit Vector Definition for VTU Violation
 */
#define GT_CPSS_VTU_MEMBER_VIOLATION                0x4
#define GT_CPSS_VTU_MISS_VIOLATION                  0x2
#define GT_CPSS_VTU_FULL_VIOLATION                  0x1

/*
 *  typedef: struct GT_CPSS_STU_ENTRY
 *
 *  Description: 802.1s Port State Information Database (STU) Entry
 *
 *  Fields:
 *      sid       - STU ID
 *      portState - Per VLAN Port States for each port.
 */

/******************************************************************************/
/* Exported VTU Functions                                                     */
/******************************************************************************/

/**
* @internal prvCpssDrvGvtuGetEntryNext function
* @endinternal
*
* @brief   Gets next valid VTU entry from the specified VID.
*
* @param[in,out] vtuEntry                 - the VID and VTUPage to start the search.
* @param[in,out] vtuEntry                 - match VTU  entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note Search starts from the VID and VTU Page (13th vid) in the vtuEntry
*       specified by the user.
*
*/
GT_STATUS prvCpssDrvGvtuGetEntryNext
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_CPSS_VTU_ENTRY     *vtuEntry
);

/**
* @internal prvCpssDrvGvtuFindVidEntry function
* @endinternal
*
* @brief   Find VTU entry for a specific VID and VTU Page, it will return the entry,
*         if found, along with its associated data
* @param[in,out] vtuEntry                 - contains the VID and VTU Page to search for.
*
* @param[out] found                    - GT_TRUE, if the appropriate entry exists.
* @param[in,out] vtuEntry                 - the entry parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error or entry does not exist.
* @retval GT_NO_SUCH               - no more entries.
*/
GT_STATUS prvCpssDrvGvtuFindVidEntry
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_CPSS_VTU_ENTRY     *vtuEntry,
    OUT   GT_BOOL               *found
);

/**
* @internal prvCpssDrvGvtuFlush function
* @endinternal
*
* @brief   This routine removes all entries from VTU Table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuFlush
(
    IN  GT_CPSS_QD_DEV  *dev
);

/**
* @internal prvCpssDrvGvtuAddEntry function
* @endinternal
*
* @brief   Creates the new entry in VTU table based on user input.
*
* @param[in] vtuEntry                 - vtu entry to insert to the VTU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvtuAddEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_VTU_ENTRY   *vtuEntry
);

/**
* @internal prvCpssDrvGvtuDelEntry function
* @endinternal
*
* @brief   Deletes VTU entry specified by user.
*
* @param[in] vtuEntry                 - the VTU entry to be deleted
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvtuDelEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_VTU_ENTRY   *vtuEntry
);

/**
* @internal prvCpssDrvGvtuSetMode function
* @endinternal
*
* @brief   Set VTU Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuSetMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_VTU_MODE    mode
);

/**
* @internal prvCpssDrvGvtuGetMode function
* @endinternal
*
* @brief   Get VTU Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuGetMode
(
    IN  GT_CPSS_QD_DEV     *dev,
    OUT GT_CPSS_VTU_MODE   *mode
);

/**
* @internal prvCpssDrvGvtuGetViolation function
* @endinternal
*
* @brief   Get VTU Violation data
*
* @param[out] vtuIntStatus             - interrupt cause, source portID, and vid.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuGetViolation
(
    IN  GT_CPSS_QD_DEV          *dev,
    OUT GT_CPSS_VTU_INT_STATUS  *vtuIntStatus
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtBrgVtu_h */

