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
* @file cpssDxChTcamManager.h
*
* @brief This file contains object to manage TCAM entries.
* The TCAM entries handler is responsible for handling the TCAM entries
* and deciding which entries to allocate to each prefix type.
* The TCAM entries handler has two modes of operation:
* - Partition mode. In this mode the TCAM range allocated for the TCAM
* entries handler in partitioned so each entry type has fixed section
* of the TCAM.
* - No partition mode. In this mode the TCAM range is not partitioned.
* All entry types share the TCAM entries and TCAM entries are
* allocated on demand.
* In partition mode, the TCAM entries handler is used as a glue layer to
* the common tcam pool manager code which implements a partitioned TCAM.
* In no partition mode, the TCAM entries handler implements a dynamic
* method for a non partitioned TCAM.
*
* @version   9
********************************************************************************
*/

#ifndef __cpssDxChTcamManagerh
#define __cpssDxChTcamManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManagerTypes.h>
#include <cpss/generic/systemRecovery/hsu/cpssGenHsu.h>

/* Maximum number of clients that can be registered to the TCAM manager */
#define CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS              16

/* Maximum number of chars in client name (including terminating NULL) */
#define CPSS_DXCH_TCAM_MANAGER_CLIENT_NAME_NAX_LENGTH_CNS   16

/**
* @internal cpssDxChTcamManagerCreate function
* @endinternal
*
* @brief   Creates TCAM manager and returns handler for the created TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamType                 - the TCAM type that the TCAM manager will handle
* @param[in] rangePtr                 - the TCAM range allocated for this TCAM manager
*
* @param[out] tcamManagerHandlerPtrPtr - points to pointer of the created TCAM manager object
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerCreate
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC        *rangePtr,
    OUT GT_VOID                                 **tcamManagerHandlerPtrPtr
);

/**
* @internal cpssDxChTcamManagerConfigGet function
* @endinternal
*
* @brief   Gets the configuration of the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @param[out] tcamTypePtr              - points to TCAM type managed by the TCAM manager
* @param[out] rangePtr                 - points to the TCAM range managed by the TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerConfigGet
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    OUT CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    *tcamTypePtr,
    OUT CPSS_DXCH_TCAM_MANAGER_RANGE_STC        *rangePtr
);

/**
* @internal cpssDxChTcamManagerDelete function
* @endinternal
*
* @brief   Deletes the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
*
* @note All entries allocated by the TCAM manager must be freed prior of calling
*       this API. Clients may be registered upon calling the API.
*
*/
GT_STATUS cpssDxChTcamManagerDelete
(
    IN  GT_VOID         *tcamManagerHandlerPtr
);

/**
* @internal cpssDxChTcamManagerRangeUpdate function
* @endinternal
*
* @brief   Updates the range allocated for the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] rangePtr                 - the new TCAM range allocated for this TCAM manager
* @param[in] rangeUpdateMethod        - update method to use during the range update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
*
* @note 1. Since the ranges for static elients are explicitly stated by the clients,
*       then static entries will never be moved (regardless of the range update
*       method). If any static entry (either allocated or reserved by static
*       client) is outside the new range, the function will fail.
*       2. In case of smaller new range, the new range must be big enough to
*       hold all current allocated and reserved entries.
*       3. In CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E update
*       method, if any of existing entries is outside of the new range,
*       the API will fail.
*       4. In CPSS_DXCH_TCAM_MANAGER_MOVE_TOP_AND_KEEP_OFFSETS_RANGE_UPDATE_METHOD_E
*       update method, if any of the existing entries will not fit in the new
*       range then the API will fail.
*       5. Since TCAM manager keeps ordering between same type of entries, then
*       upon range update, ordering between entries of different types may
*       be changed.
*
*/
GT_STATUS cpssDxChTcamManagerRangeUpdate
(
    IN  GT_VOID                                         *tcamManagerHandlerPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC                *rangePtr,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT  rangeUpdateMethod
);

/**
* @internal cpssDxChTcamManagerClientRegister function
* @endinternal
*
* @brief   Register a client in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientNameArr[CPSS_DXCH_TCAM_MANAGER_CLIENT_NAME_NAX_LENGTH_CNS] - string that represent the client
* @param[in] clientFuncPtr            - points to callback functions supplied by the client
*
* @param[out] clientIdPtr              - points to client Id allocated by the TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FULL                  - when max number of clients already registered to
*                                       the TCAM manager
* @retval GT_FAIL                  - on error
*
* @note 1. Up to CPSS_DXCH_TCAM_MANAGER_MAX_CLIENTS_CNS clients is supported.
*
*/
GT_STATUS cpssDxChTcamManagerClientRegister
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_CHAR                                 clientNameArr[CPSS_DXCH_TCAM_MANAGER_CLIENT_NAME_NAX_LENGTH_CNS],
    IN  CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  *clientFuncPtr,
    OUT GT_U32                                  *clientIdPtr
);

/**
* @internal cpssDxChTcamManagerClientUnregister function
* @endinternal
*
* @brief   Unregister a client in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - the clientId is not registered
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerClientUnregister
(
    IN  GT_VOID     *tcamManagerHandlerPtr,
    IN  GT_U32      clientId
);

/**
* @internal cpssDxChTcamManagerClientFuncUpdate function
* @endinternal
*
* @brief   Updates client callback functions in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] clientFuncPtr            - points to callback functions supplied by the client
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - on not registered clientId
* @retval GT_FAIL                  - on error
*
* @note After TCAM manager import, the callback functions of all clients are set
*       to NULL (to prevent using non valid function pointers). Untill the client
*       updates the callback functions, the client will be considered inactive
*       and will be limited.
*
*/
GT_STATUS cpssDxChTcamManagerClientFuncUpdate
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC  *clientFuncPtr
);

/**
* @internal cpssDxChTcamManagerEntriesReservationSet function
* @endinternal
*
* @brief   Reserves entries for a client in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] reservationType          - type of the reservation
* @param[in] requestedEntriesArray[]  - array of amount of entries requested by client
* @param[in] sizeOfArray              - number of elemenets in the array
* @param[in] reservationRangePtr      - the TCAM range allocated for this reservation;
*                                      relevant only for static reservation type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - on not registered clientId
* @retval GT_ALREADY_EXIST         - on static reservation with range that collides
*                                       with other already existing static reservations
* @retval GT_NO_RESOURCE           - the reservation can't fit in the TCAM
* @retval GT_FAIL                  - on error
*
* @note 1. The TCAM manager will save enough space so that at any given time,
*       the client will have enough space for up to the number of reserved
*       entries.
*       2. In case of static entries reservation, the TCAM range allocated to
*       the static reservation must be within the range allocated to the
*       TCAM manager. The requested range can't contain entries reserved for
*       other static reservation.
*       3. Each client can have only one reservation. New reservation for a
*       client will replace the previous reservation.
*       4. Elements in the requestedEntriesArray must refer to unique entries
*       types. Meaning that no two array elements can have same entry type.
*       In addition the amount of requested entries can't be 0.
*
*/
GT_STATUS cpssDxChTcamManagerEntriesReservationSet
(
    IN  GT_VOID                                             *tcamManagerHandlerPtr,
    IN  GT_U32                                              clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             requestedEntriesArray[],
    IN  GT_U32                                              sizeOfArray,
    IN  CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    *reservationRangePtr
);

/**
* @internal cpssDxChTcamManagerEntriesReservationGet function
* @endinternal
*
* @brief   Gets number of reserved entries for a client in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in,out] sizeOfArrayPtr           - points to the allocated size of the array
*
* @param[out] reservationTypePtr       - points to reservation type
* @param[out] reservedEntriesArray[]   - array of amount of entries reserved to client
* @param[in,out] sizeOfArrayPtr           - points to number of elements filled in the array
* @param[out] reservationRangePtr      - points to the reservation range (relevant only
*                                      for static entries reservation)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - the clientId is not registered
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerEntriesReservationGet
(
    IN      GT_VOID                                             *tcamManagerHandlerPtr,
    IN      GT_U32                                              clientId,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT *reservationTypePtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             reservedEntriesArray[],
    INOUT   GT_U32                                              *sizeOfArrayPtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    *reservationRangePtr
);

/**
* @internal cpssDxChTcamManagerUsageGet function
* @endinternal
*
* @brief   Returns the current number of entries from each type used by all
*         clients in the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] usageOfSpecificClient    - GT_TRUE:  get usage of specific client
*                                      GT_FALSE: get commulative usage of all clients
* @param[in] clientId                 - the client Id
*                                      relevant if usageOfSpecificClient is GT_TRUE
* @param[in,out] sizeOfReservedArrayPtr   - points to the allocated size of the
*                                      reserved entries array
* @param[in,out] sizeofAllocatedArrayPtr  - points to the allocated size of the
*                                      allocated entries array
*
* @param[out] reservedEntriesArray[]   - array of amount of entries reserved
* @param[in,out] sizeOfReservedArrayPtr   - points to the allocated size of the
*                                      reserved entries array
* @param[out] allocatedEntriesArray[]  - array of amount of entries allocated
* @param[in,out] sizeofAllocatedArrayPtr  - points to the allocated size of the
*                                      allocated entries array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - the clientId is not registered
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerUsageGet
(
    IN      GT_VOID                                 *tcamManagerHandlerPtr,
    IN      GT_BOOL                                 usageOfSpecificClient,
    IN      GT_U32                                  clientId,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC reservedEntriesArray[],
    INOUT   GT_U32                                  *sizeOfReservedArrayPtr,
    OUT     CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC allocatedEntriesArray[],
    INOUT   GT_U32                                  *sizeofAllocatedArrayPtr
);

/**
* @internal cpssDxChTcamManagerDump function
* @endinternal
*
* @brief   Dumps the state of the TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] dumpType                 - type of dump
* @param[in] clientId                 - the client Id; relevant only when dump
*                                      type is  CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - the clientId is not registered
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChTcamManagerDump
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT    dumpType,
    IN  GT_U32                                  clientId
);

/**
* @internal cpssDxChTcamManagerEntryAllocate function
* @endinternal
*
* @brief   Allocates TCAM entry for client. The entry is allocated in a range
*         bounded by two other entries (bounded in the meaning of TCAM search
*         order). The entry is allocated according to specific allocation method.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] entryType                - entry type to allocate
* @param[in] lowerBoundToken          - token representing the lower boundary; the allocated
*                                      entry will reside after this token (in the meaning
*                                      of TCAM search order); if 0 then no lower boundary
* @param[in] upperBoundToken          - toekn representing the upper boundary; the allocated
*                                      entry will reside before this token (in the meaning
*                                      of TCAM search order); if 0 then no upper boundary
* @param[in] allocMethod              - the allocation method
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation;
* @param[in] clientCookiePtr          - client cookie; used in client callback functions
*
* @param[out] allocEntryTokenPtr       - token representing the allocated entry
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*/
GT_STATUS cpssDxChTcamManagerEntryAllocate
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT   entryType,
    IN  GT_U32                                  lowerBoundToken,
    IN  GT_U32                                  upperBoundToken,
    IN  CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT allocMethod,
    IN  GT_BOOL                                 defragEnable,
    IN  GT_VOID                                 *clientCookiePtr,
    OUT GT_U32                                  *allocEntryTokenPtr
);

/**
* @internal cpssDxChTcamManagerEntryClientCookieUpdate function
* @endinternal
*
* @brief   Updates client cookie for specific entry token.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] entryToken               - token representing the entry to be updated
* @param[in] clientCookiePtr          - new client cookie; used in client callback functions
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*/
GT_STATUS cpssDxChTcamManagerEntryClientCookieUpdate
(
    IN  GT_VOID             *tcamManagerHandlerPtr,
    IN  GT_U32              clientId,
    IN  GT_U32              entryToken,
    IN  GT_VOID             *clientCookiePtr
);

/**
* @internal cpssDxChTcamManagerEntryFree function
* @endinternal
*
* @brief   Frees entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] entryToken               - token representing the entry
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*
* @note 1. The entry must be previously allocated by this client.
*
*/
GT_STATUS cpssDxChTcamManagerEntryFree
(
    IN  GT_VOID             *tcamManagerHandlerPtr,
    IN  GT_U32              clientId,
    IN  GT_U32              entryToken
);

/**
* @internal cpssDxChTcamManagerAvailableEntriesCheck function
* @endinternal
*
* @brief   Checks whether the TCAM manager has enough available entries for the
*         requested amount of entries.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] requestedEntriesArray[]  - array of amount of entries requested by client
* @param[in] sizeOfArray              - number of elemenets in the array
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_SUCH               - on not registered clientId
* @retval GT_NO_RESOURCE           - if there is not enough space in the TCAM manager
*                                       for the requested entries
* @retval GT_FAIL                  - on error
*
* @note 1. The number of reserved entries of the specific client is also taken
*       into calculations.
*       2. Elements in the requestedEntriesArray must refer to unique entries
*       types. Meaning that no two array elements can have same entry type.
*       In addition the amount of requested entries can't be 0.
*
*/
GT_STATUS cpssDxChTcamManagerAvailableEntriesCheck
(
    IN  GT_VOID                                 *tcamManagerHandlerPtr,
    IN  GT_U32                                  clientId,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC requestedEntriesArray[],
    IN  GT_U32                                  sizeOfArray
);

/**
* @internal cpssDxChTcamManagerTokenToTcamLocation function
* @endinternal
*
* @brief   Converts entry token to hardware TCAM location.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] entryToken               - token representing the entry
*
* @param[out] tcamLocationPtr          - points to the hardware TCAM location
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*
* @note 1. The entry must be previously allocated by the same client.
*
*/
GT_STATUS cpssDxChTcamManagerTokenToTcamLocation
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      entryToken,
    OUT CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *tcamLocationPtr
);

/**
* @internal cpssDxChTcamManagerTokenCompare function
* @endinternal
*
* @brief   Compares two tokens and returns the relative order (according to TCAM
*         order).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] firstToken               - first token to compare
* @param[in] secondToken              - second token to compare
*
* @param[out] resultPtr                - points to the compare result,
*                                      GT_EQUAL    - tokens represent same tcam location
*                                      GT_GREATER  - first token is after second token
*                                      according to TCAM search order
*                                      GT_SMALLER  - first tokem is before second token
*                                      according to TCAM search order
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*
* @note The tokens must be previously allocated by the same client.
*
*/
GT_STATUS cpssDxChTcamManagerTokenCompare
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      firstToken,
    IN  GT_U32                                      secondToken,
    OUT GT_COMP_RES                                 *resultPtr
);

/**
* @internal cpssDxChTcamManagerTokenInfoGet function
* @endinternal
*
* @brief   Returns token information.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in] clientId                 - the client Id
* @param[in] entryToken               - token representing the entry
*
* @param[out] entryTypePtr             - points to type of entry associated with the token
* @param[out] clientCookiePtrPtr       - points to pointer of cookie associated with the token
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*
* @note 1. The entry must be previously allocated by the same client.
*
*/
GT_STATUS cpssDxChTcamManagerTokenInfoGet
(
    IN  GT_VOID                                     *tcamManagerHandlerPtr,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      entryToken,
    OUT CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT       *entryTypePtr,
    OUT GT_VOID                                     **clientCookiePtrPtr
);

/**
* @internal cpssDxChTcamManagerHsuSizeGet function
* @endinternal
*
* @brief   Gets the memory size needed for TCAM manager HSU export information.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @param[out] sizePtr                  - points to memory size needed for export data,
*                                      including HSU data block header (calculated in bytes)
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamManagerHsuSizeGet
(
    IN  GT_VOID     *tcamManagerHandlerPtr,
    OUT GT_U32      *sizePtr
);

/**
* @internal cpssDxChTcamManagerHsuExport function
* @endinternal
*
* @brief   Exports TCAM manager HSU data.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the TCAM manager to track the export
*                                      progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation export is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration export. In such case
*                                      it is assumed that hsuBlockMemPtr is large enough
*                                      for the entire HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block to hold HSU data in
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the TCAM manager to track the export
*                                      progress.
*                                      Relevant only if exportCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration export is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] exportCompletePtr        - GT_TRUE:  HSU data export is completed.
*                                      GT_FALSE: HSU data export is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Several export processes can be performed simultaneously.
*       2. It is assumed that in case of multiple iterations export operation,
*       there are no calls to other TCAM manager APIs until export operation
*       is completed. Such calls may modify the data stored within the TCAM
*       manager and cause unexpected behavior.
*
*/
GT_STATUS cpssDxChTcamManagerHsuExport
(
    IN    GT_VOID       *tcamManagerHandlerPtr,
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *exportCompletePtr
);

/**
* @internal cpssDxChTcamManagerHsuImport function
* @endinternal
*
* @brief   Imports TCAM manager HSU data.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the TCAM manager to track the import
*                                      progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation import is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration import. In such case
*                                      it is assumed that hsuBlockMemPtr contains all HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block that holds HSU data for
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the TCAM manager to track the import
*                                      progress.
*                                      Relevant only if importCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration import is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] importCompletePtr        - GT_TRUE:  HSU data import is completed.
*                                      GT_FALSE: HSU data import is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The attributes of the TCAM manager that were supplied during TCAM
*       manager creation of the TCAM manager object that performs the
*       import (TCAM type and TCAM range) must match the attributes of
*       the TCAM manager object that performed the export.
*       2. The import function will override all the TCAM manager data and set
*       it to the data of the TCAM manager that performed the export at the
*       time of the export. All changes made between the export and the import
*       will be discarded (only change is that during import callback functions
*       for registered clients will be set to NULL).
*       3. It is assumed that in case of multiple iterations import operation,
*       there are no calls to other TCAM manager APIs until import is
*       completed. Such calls may modify the data stored within the TCAM
*       manager and cause unexpected behavior.
*       4. It is assumed that no different import processes are performed
*       simultaneously.
*       5. Marvell recommends importing TCAM manager prior to importing
*       registered TCAM manager clients.
*
*/
GT_STATUS cpssDxChTcamManagerHsuImport
(
    IN    GT_VOID       *tcamManagerHandlerPtr,
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *importCompletePtr
);

/**
* @internal cpssDxChTcamManagerHsuInactiveClientDelete function
* @endinternal
*
* @brief   Delete all inactive clients and free the resources used by them. Inactive
*         client is a client that didn't update its callback functions after HSU
*         import operation.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Only resources in the TCAM manager are freed. The TCAM hardware will not
*       be changed.
*
*/
GT_STATUS cpssDxChTcamManagerHsuInactiveClientDelete
(
    IN  GT_VOID     *tcamManagerHandlerPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTcamManagerh */

