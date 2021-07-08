/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file appDemoFdbIpv6UcLinkedList.h
*
* @brief the implementation of functions to manipulate of FDB IPv6 UC Route
* entries linked list, that holds AddressIndex to DataIndex mapping.
*
*
* @version   3
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>

#ifdef CHX_FAMILY

/**
* @internal appDemoBrgFdbIpv6UcFreeAddrDataLinkedList function
* @endinternal
*
* @brief   Free FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
* @param[in] devNum                   - the device number from which AU are taken
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Only elements that were deleted in flush process will be deleted
*       from the Linked List. If only part of the IPV6 UC entry was deleted
*       (address entry or data entry) then the Linked list will be updated
*       with value PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS indicating the
*       entry is not valid
*
*/
GT_STATUS appDemoBrgFdbIpv6UcFreeAddrDataLinkedList
(
    IN GT_U8  devNum
);

/**
* @internal appDemoBrgFdbIpv6UcUpdateAddrDataLinkedList function
* @endinternal
*
* @brief   Update FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
* @param[in] devNum                   - the device number
* @param[in] addrIndex                - FDB IPv6 UC address index bind to dataIndex
* @param[in] dataIndex                - FDB IPv6 UC data index bind to addrIndex
* @param[in] deleteElement            - GT_TRUE: delete the element if found
*                                      GT_FALSE: update the element if found or add a new one
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - on allocation fail
* @retval GT_FAIL                  - otherwise.
*
* @note adding a mapping of addrIndex and dataIndex to the Linked List must be
*       of valid entries that were added to HW.
*       if the address entry or the data entry is not added to HW this function will fail.
*       the Linked List should reflect the HW state
*
*/
GT_STATUS appDemoBrgFdbIpv6UcUpdateAddrDataLinkedList
(
    IN  GT_U8           devNum,
    IN  GT_U32          addrIndex,
    IN  GT_U32          dataIndex,
    IN  GT_BOOL         deleteElement
);

/**
* @internal prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete function
* @endinternal
*
* @brief   This function get an FDB IPv6 UC index that can be a dataIndex or an
*         addressIndex and return the related index bound to it from the linked list.
* @param[in] devNum                   - the device number
* @param[in] entryType                - the type of the index input parameter.
* @param[in] index                    - according to the entryType this can be a dataIndex or a addressIndex
* @param[in] deleteElement            - GT_TRUE: delete the element if found
*                                      GT_FALSE: just return the index of the elemnt if found
*
* @param[out] pairIndexPtr             - (pointer to) the pair index if found, according to the
* @param[out] entryType                this can be an addressIndex or a dataIndex
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - incase there is no matching pair for this index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete
(
    IN  GT_U8                           devNum,
    IN  CPSS_MAC_ENTRY_EXT_TYPE_ENT     entryType,
    IN  GT_U32                          index,
    IN  GT_BOOL                         deleteElement,
    OUT  GT_U32                         *pairIndexPtr
);

#endif  /* CHX_FAMILY */

