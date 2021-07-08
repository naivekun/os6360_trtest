#include <Copyright.h>
/**
********************************************************************************
* @file gtBrgFdb.h
*
* @brief API/Structure definitions for Marvell ATU functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtBrgFdb.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell ATU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtBrgFdb_h
#define __prvCpssDrvGtBrgFdb_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported ATU Types                                                       */
/****************************************************************************/
/**
* @enum GT_CPSS_FLUSH_CMD
 *
 * @brief Enumeration of the address translation unit flush operation.
*/
typedef enum{

    /** flush all entries. */
    GT_CPSS_FLUSH_ALL           = 1,

    /** flush all non-static entries. */
    GT_CPSS_FLUSH_ALL_NONSTATIC = 2,

} GT_CPSS_FLUSH_CMD;

/**
* @enum GT_CPSS_MOVE_CMD
 *
 * @brief Enumeration of the address translation unit move or remove operation.
 * When destination port is set to 0x1F, Remove operation occurs.
*/
typedef enum{

    /** move all entries. */
    GT_CPSS_MOVE_ALL           = 1,

    /** move all non-static entries. */
    GT_CPSS_MOVE_ALL_NONSTATIC = 2,

} GT_CPSS_MOVE_CMD;

/**
* @struct GT_CPSS_ATU_EXT_PRI
 *
 * @brief Extended priority information for the address tarnslaton unit entry.
*/
typedef struct{

    /** MAC frame priority data (0 ~ 7). */
    GT_U8 macFPri;

    /** MAC Queue priority data (0 ~ 7). */
    GT_U8 macQPri;

} GT_CPSS_ATU_EXT_PRI;


/**
* @struct GT_CPSS_ATU_ENTRY
 *
 * @brief address translation unit Entry
*/
typedef struct{

    /** mac address */
    GT_ETHERADDR macAddr;

    /** Link Aggregation Mapped Address */
    GT_BOOL LAG;

    /** @brief port Vector.
     *  If trunkMember field is GT_TRUE, this value represents trunk ID.
     */
    GT_U32 portVec;

    GT_U8 reserved;

    /** @brief ATU MAC Address Database number. If multiple address
     *  databases are not being used, DBNum should be zero.
     *  If multiple address databases are being used, this value
     *  should be set to the desired address database number.
     */
    GT_U16 DBNum;

    /** the entry state. */
    GT_U8 entryState;

    /** @brief extended priority information.
     *  Comment:
     *  The entryState union Type is determine according to the Mac Type.
     */
    GT_CPSS_ATU_EXT_PRI exPrio;

} GT_CPSS_ATU_ENTRY;

/**
* @struct GT_CPSS_ATU_INT_STATUS
 *
 * @brief VLAN tarnslaton unit interrupt status
*/
typedef struct{

    GT_U16 atuIntCause;

    GT_U8 spid;

    GT_U16 dbNum;

    /** MAC Address */
    GT_ETHERADDR macAddr;

} GT_CPSS_ATU_INT_STATUS;

/*
 * Bit Vector Definition for ATU Violation
 */
#define GT_CPSS_ATU_AGE_OUT_VIOLATION        0x8
#define GT_CPSS_ATU_MEMBER_VIOLATION         0x4
#define GT_CPSS_ATU_MISS_VIOLATION           0x2
#define GT_CPSS_ATU_FULL_VIOLATION           0x1

/******************************************************************************/
/* Exported ATU Functions                                                     */
/******************************************************************************/
/**
* @internal prvCpssDrvGfdbFlush function
* @endinternal
*
* @brief   This routine flush all or all non-static addresses from the MAC Address
*         Table.
* @param[in] flushCmd                 - the flush operation type.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none
*
*/
GT_STATUS prvCpssDrvGfdbFlush
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_FLUSH_CMD   flushCmd
);

/**
* @internal prvCpssDrvGfdbFlushInDB function
* @endinternal
*
* @brief   This routine flush all or all non-static addresses from the particular
*         ATU Database (DBNum). If multiple address databases are being used, this
*         API can be used to flush entries in a particular DBNum database.
* @param[in] flushCmd                 - the flush operation type.
* @param[in] DBNum                    - ATU MAC Address Database Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note none
*
*/
GT_STATUS prvCpssDrvGfdbFlushInDB
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_FLUSH_CMD   flushCmd,
    IN  GT_U32              DBNum
);

/**
* @internal prvCpssDrvGfdbMove function
* @endinternal
*
* @brief   This routine moves all or all non-static addresses from a port to
*         another.
* @param[in] moveCmd                  - the move operation type.
* @param[in] moveFrom                 - port where moving from
* @param[in] moveTo                   - port where moving to
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note none
*
*/
GT_STATUS prvCpssDrvGfdbMove
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_MOVE_CMD    moveCmd,
    IN  GT_U32              moveFrom,
    IN  GT_U32              moveTo
);

/**
* @internal prvCpssDrvGfdbMoveInDB function
* @endinternal
*
* @brief   This routine move all or all non-static addresses which are in the
*         particular ATU Database (DBNum) from a port to another.
* @param[in] moveCmd                  - the move operation type.
* @param[in] DBNum                    - ATU MAC Address Database Number.
* @param[in] moveFrom                 - port where moving from
* @param[in] moveTo                   - port where moving to
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGfdbMoveInDB
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_MOVE_CMD    moveCmd,
    IN  GT_U32              DBNum,
    IN  GT_U32              moveFrom,
    IN  GT_U32              moveTo
);

/**
* @internal prvCpssDrvGfdbAddMacEntry function
* @endinternal
*
* @brief   Creates the new entry in MAC address table.
*
* @param[in] macEntry                 - mac address entry to insert to the ATU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note DBNum in macEntry -
*       ATU MAC Address Database number. If multiple address
*       databases are not being used, DBNum should be zero.
*       If multiple address databases are being used, this value
*       should be set to the desired address database number.
*
*/
GT_STATUS prvCpssDrvGfdbAddMacEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_ATU_ENTRY   *macEntry
);

/**
* @internal prvCpssDrvGfdbDelAtuEntry function
* @endinternal
*
* @brief   Deletes ATU entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note DBNum in atuEntry -
*       ATU MAC Address Database number. If multiple address
*       databases are not being used, DBNum should be zero.
*       If multiple address databases are being used, this value
*       should be set to the desired address database number.
*
*/
GT_STATUS prvCpssDrvGfdbDelAtuEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_ATU_ENTRY   *atuEntry
);

/**
* @internal prvCpssDrvGfdbGetAtuEntryNext function
* @endinternal
*
* @brief   Gets next lexicographic MAC address starting from the specified Mac Addr
*         in a particular ATU database (DBNum or FID).
* @param[in,out] atuEntry                 - the Mac Address to start the search.
* @param[in,out] atuEntry                 - match Address translate unit entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note Search starts from atu.macAddr[xx:xx:xx:xx:xx:xx] specified by the
*       user.
*       DBNum in atuEntry -
*       ATU MAC Address Database number. If multiple address
*       databases are not being used, DBNum should be zero.
*       If multiple address databases are being used, this value
*       should be set to the desired address database number.
*
*/
GT_STATUS prvCpssDrvGfdbGetAtuEntryNext
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_CPSS_ATU_ENTRY     *atuEntry
);

/**
* @internal prvCpssDrvGfdbGetViolation function
* @endinternal
*
* @brief   Get ATU Violation data
*
* @param[out] atuIntStatus             - interrupt cause, source portID, dbNum and mac address.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none
*
*/
GT_STATUS prvCpssDrvGfdbGetViolation
(
    IN  GT_CPSS_QD_DEV          *dev,
    OUT GT_CPSS_ATU_INT_STATUS  *atuIntStatus
);

/**
* @internal prvCpssDrvGfdbFindAtuMacEntry function
* @endinternal
*
* @brief   Find FDB entry for specific MAC address from the ATU.
*
* @param[in,out] atuEntry                 - the Mac address to search.
*
* @param[out] found                    - GT_TRUE, if the appropriate entry exists.
* @param[in,out] atuEntry                 - the entry parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error or entry does not exist.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - on bad parameter
*
* @note DBNum in atuEntry -
*       ATU MAC Address Database number. If multiple address
*       databases are not being used, DBNum should be zero.
*       If multiple address databases are being used, this value
*       should be set to the desired address database number.
*
*/
GT_STATUS prvCpssDrvGfdbFindAtuMacEntry
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_CPSS_ATU_ENTRY     *atuEntry,
    OUT   GT_BOOL               *found
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtBrgFdb_h */

