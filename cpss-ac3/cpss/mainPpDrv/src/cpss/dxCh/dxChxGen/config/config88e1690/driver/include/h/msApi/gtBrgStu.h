#include <Copyright.h>
/**
********************************************************************************
* @file gtBrgStu.h
*
* @brief API/Structure definitions for Marvell STU functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtBrgStu.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell STU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtBrgStu_h
#define __prvCpssDrvGtBrgStu_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported STU Types                                                            */
/****************************************************************************/
/**
* @enum GT_CPSS_PORT_STP_STATE
 *
 * @brief Enumeration of the port spanning tree state.
*/
typedef enum{

    /** port is disabled. */
    GT_CPSS_PORT_DISABLE = 0,

    /** port is in blocking/listening state. */
    GT_CPSS_PORT_BLOCKING,

    /** port is in learning state. */
    GT_CPSS_PORT_LEARNING,

    /** port is in forwarding state. */
    GT_CPSS_PORT_FORWARDING

} GT_CPSS_PORT_STP_STATE;

/**
* @struct GT_CPSS_STU_ENTRY
 *
 * @brief 802.1s Port State Information Database (STU) Entry
*/
typedef struct{

    /** STU ID */
    GT_U16 sid;

    GT_CPSS_PORT_STP_STATE portState[CPSS_MAX_SWITCH_PORTS];

} GT_CPSS_STU_ENTRY;

/******************************************************************************/
/* Exported STU Functions                                                     */
/******************************************************************************/
/**
* @internal prvCpssDrvGstuGetEntryNext function
* @endinternal
*
* @brief   Gets next valid STU entry from the specified SID.
*
* @param[in,out] stuEntry                 - the SID to start the search.
* @param[in,out] stuEntry                 - next STU entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstuGetEntryNext
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_CPSS_STU_ENTRY     *stuEntry
);

/**
* @internal prvCpssDrvGstuAddEntry function
* @endinternal
*
* @brief   Creates or update the entry in STU table based on SID.
*
* @param[in] stuEntry                 - stu entry to insert to the STU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstuAddEntry
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_STU_ENTRY    *stuEntry
);

/**
* @internal prvCpssDrvGstuDelEntry function
* @endinternal
*
* @brief   Deletes STU entry specified by SID.
*
* @param[in] stuEntry                 - the STU entry to be deleted
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstuDelEntry
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_STU_ENTRY    *stuEntry
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtBrgStu_h */

