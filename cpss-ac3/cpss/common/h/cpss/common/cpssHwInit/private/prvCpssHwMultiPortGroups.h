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
* @file prvCpssHwMultiPortGroups.h
*
* @brief Private API definition for multi-port-groups devices utilities.
*
* @version   15
********************************************************************************
*/

#ifndef __prvCpssHwMultiPortGroupsh
#define __prvCpssHwMultiPortGroupsh

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Bool function tables for prvCpssPortGroupRegistersStateBusyWait */
#define PRV_CPSS_HW_BOOL_FUNCTION_TABLE_AND_CNS 0x8
#define PRV_CPSS_HW_BOOL_FUNCTION_TABLE_OR_CNS 0xE

/**
* @internal prvCpssPortGroupBusyWait function
* @endinternal
*
* @brief   function do 'Busy wait' on a bit ,until cleared.
*         function wait on the register in the specified port group.
*         if portGroupId = 'unaware' the function loop on all active
*         port groups and wait for all of them to finish
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - the register address to query
* @param[in] selfClearBit             - the bit to query
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the bit until cleared , on all port-groups
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the bit was not cleared
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPortGroupBusyWait
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   selfClearBit,
    IN GT_BOOL  onlyCheck
);

/**
* @internal prvCpssPortGroupBusyWaitByMask function
* @endinternal
*
* @brief   function do 'Busy wait' on specific mask of the register.
*         function wait on the register in the specified port group.
*         if portGroupId = 'unaware' the function loop on all active
*         port groups and wait for all of them to finish
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - the register address to query
* @param[in] regMask                  - the mask of the register that we wait for to clear
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached , on all port-groups
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPortGroupBusyWaitByMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   regMask,
    IN GT_BOOL  onlyCheck
);

/**
* @internal prvCpssPortGroupRegistersStateBusyWait function
* @endinternal
*
* @brief   function do 'Busy wait' on specified set of registers for the specified state.
*         function wait on the register in the specified port group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port group devices that need to access
*                                      specific port group
* @param[in] regNum                   - amount of registers to check
* @param[in] regAddrArr[]             - array of the register addresses to query
* @param[in] regMaskArr[]             - array of the masks of the registers that
*                                      we wait to get values specified in pattern
* @param[in] regPatternArr[]          - array of the pattern values of the registers
* @param[in] booleanOpArr[]           -  array of logical operations
*                                      Boolean operation is table of function F(a,b)
*                                      where a and b in the set of {0,1} and result also 0 or 1.
*                                      It is 4-bit value.
*                                      F(0,0)=bit0, F(0,1)=bit1, F(1,0)=bit2, F(1,1)=bit3,
*                                      i.e. F(a,b) = ((F >> ((2  a) + b)) & 1);
*                                      For example OR is 0xE, AND is 0x8
*                                      The state check algorithm is follows:
*                                      The result initialized as 1 (i.e. GT_TRUE)
*                                      and in loop by index from 0 to (regNum - 1)
*                                      result recalculated:
*                                      result = booleanOp[index](
*                                      result,
*                                      ((value & regMaskArr[index]) == regPatternArr[index]))
*                                      where value retrieved from register regAddrArr[index].
*                                      Note0: there are cases when for result recalculation
*                                      not needed to read register: when F(result, 0) == F(result, 1)
*                                      in these cases register should not be read.
*                                      Note1: Application should typically specify
*                                      booleanOp[0]==0x8 (i.e. AND)
* @param[in] onlyCheck                - do we want only to check the current status,
*                                      or to wait until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPortGroupRegistersStateBusyWait
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regNum,
    IN GT_U32   regAddrArr[],
    IN GT_U32   regMaskArr[],
    IN GT_U32   regPatternArr[],
    IN GT_U32   booleanOpArr[],
    IN GT_BOOL  onlyCheck
);

/**
* @internal prvCpssPortGroupsBmpCounterSummary function
* @endinternal
*
* @brief   summary the counters from all the port groups in the bitmap,
*         from the input register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] regAddr                  - The register's address to read the counter value from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read from the register.
*
* @param[out] counter32bitValuePtr     - (pointer to) the summary counter from the port groups, in 32 bits
*                                      when NULL --> ignored
* @param[out] counter64bitValuePtr     - (pointer to) the summary counter from the port groups, in 64 bits
*                                      when NULL --> ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsBmpCounterSummary
(
    IN GT_U8                 devNum,
    IN GT_PORT_GROUPS_BMP    portGroupsBmp,
    IN GT_U32                regAddr,
    IN GT_U32                fieldOffset,
    IN GT_U32                fieldLength,
    OUT GT_U32               *counter32bitValuePtr,
    OUT GT_U64               *counter64bitValuePtr
);

/**
* @internal prvCpssPortGroupsCounterSummary function
* @endinternal
*
* @brief   summary the counters from all the port groups , from the input register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to read the counter value from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read from the register.
*
* @param[out] counter32bitValuePtr     - (pointer to) the summary counter from the port groups, in 32 bits
*                                      when NULL --> ignored
* @param[out] counter64bitValuePtr     - (pointer to) the summary counter from the port groups, in 64 bits
*                                      when NULL --> ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsCounterSummary
(
    IN GT_U8   devNum,
    IN GT_U32  regAddr,
    IN GT_U32  fieldOffset,
    IN GT_U32  fieldLength,
    OUT GT_U32 *counter32bitValuePtr,
    OUT GT_U64 *counter64bitValuePtr
);


/**
* @internal prvCpssPortGroupsBmpMultiCounterSummary function
* @endinternal
*
* @brief   The function make summ of the counters for port group bitmap from the
*         input register. The register holds several (2 or 3) counters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] regAddr                  - The register's address to read the counter value from.
* @param[in] fieldOffset1             - The start bit number in the register - counter 1
* @param[in] fieldLength1             - The number of bits to be read from the register - counter 1
* @param[in] fieldOffset2             - The start bit number in the register - counter 2
*                                      when 0 --> counter ignored
* @param[in] fieldLength2             - The number of bits to be read from the register - counter 2
*                                      when 0 --> counter ignored
* @param[in] fieldOffset3             - The start bit number in the register - counter 3
*                                      when 0 --> counter ignored
* @param[in] fieldLength3             - The number of bits to be read from the register - counter 3
*                                      when 0 --> counter ignored
*
* @param[out] counterValue1Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 1
* @param[out] counterValue2Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 2
* @param[out] counterValue3Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsBmpMultiCounterSummary
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              regAddr,
    IN  GT_U32              fieldOffset1,
    IN  GT_U32              fieldLength1,
    OUT GT_U32              *counterValue1Ptr,
    IN  GT_U32              fieldOffset2,
    IN  GT_U32              fieldLength2,
    OUT GT_U32              *counterValue2Ptr,
    IN  GT_U32              fieldOffset3,
    IN  GT_U32              fieldLength3,
    OUT GT_U32              *counterValue3Ptr
);

/**
* @internal prvCpssPortGroupsMultiCounterSummary function
* @endinternal
*
* @brief   summary the counters from all the port groups , from the input register.
*         The register hold several (2 or 3) counters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to read the counter value from.
* @param[in] fieldOffset1             - The start bit number in the register - counter 1
* @param[in] fieldLength1             - The number of bits to be read from the register - counter 1.
* @param[in] fieldOffset2             - The start bit number in the register - counter 2
*                                      when 0 --> counter ignored
* @param[in] fieldLength2             - The number of bits to be read from the register - counter 2.
*                                      when 0 --> counter ignored
* @param[in] fieldOffset3             - The start bit number in the register - counter 3
*                                      when 0 --> counter ignored
* @param[in] fieldLength3             - The number of bits to be read from the register - counter 3.
*                                      when 0 --> counter ignored
*
* @param[out] counterValue1Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 1
* @param[out] counterValue2Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 2
* @param[out] counterValue3Ptr         - (pointer to) the summary counter from the
*                                      port groups , in 32 bits - counter 3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsMultiCounterSummary
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset1,
    IN GT_U32 fieldLength1,
    OUT GT_U32 *counterValue1Ptr,
    IN GT_U32 fieldOffset2,
    IN GT_U32 fieldLength2,
    OUT GT_U32 *counterValue2Ptr,
    IN GT_U32 fieldOffset3,
    IN GT_U32 fieldLength3,
    OUT GT_U32 *counterValue3Ptr
);

/**
* @internal prvCpssPortGroupCounterAverageValueGet function
* @endinternal
*
* @brief   The function gets average value of counter for current port group.
*         The cntr defines input value of counter that will be distributed between
*         portGroupNum port groups. The distribution is fair between port groups.
*         In best case port groups get same value of counter = cntr / portGroupNum.
*         The reminder from division above is distributed between first port groups.
*         E.g. cntr = 7, 4 port groups
*         port group index  output counter value
*         0        2
*         1        2
*         2        2
*         3        1
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portGroupNum             - number of port groups
* @param[in] portGroupIndex           - port group index
* @param[in] counterValue             - input value of counter
*
* @param[out] averagePtr               - (pointer to) average value of counter for portGroupIndex
*/
GT_VOID prvCpssPortGroupCounterAverageValueGet
(
    IN  GT_U16                            portGroupNum,
    IN  GT_U16                            portGroupIndex,
    IN  GT_U64                            counterValue,
    OUT GT_U64                           *averagePtr
);

/**
* @internal prvCpssPortGroupsBmpCounterSet function
* @endinternal
*
* @brief   Set the counter on all the port groups in the bitmap,
*         from the input register.
*         The function set an average value of counter
*         to get the given value summing over
*         all port groups according to bitmap.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] regAddr                  - The register's address to set the counter value to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] counterValue             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsBmpCounterSet
(
    IN GT_U8                 devNum,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32                regAddr,
    IN GT_U32                fieldOffset,
    IN GT_U32                fieldLength,
    IN GT_U32                counterValue
);

/**
* @internal prvCpssPortGroupsCounterSet function
* @endinternal
*
* @brief   set the counter on all the port groups , from the input register
*         the function set first active port group with the needed value , and set other
*         port groups with value 0 --> so the summary will not count multi-values
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to set the counter value to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] counterValue             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsCounterSet
(
    IN GT_U8  devNum,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 counterValue
);

/**
* @internal prvCpssPortGroupsBmpCounterArraySet function
* @endinternal
*
* @brief   set the counters on all the port groups in the bitmap, from the input register
*         the function set first active port group with the needed values , and set other
*         port groups with value 0 --> so the summary will not count multi-values
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
* @param[in] startAddr                - The register's start address to set the counters values to.
* @param[in] numberOfWords            - number of words to set (registers)
* @param[in] counterValueArr[]        - (array of)the counters values to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when numberOfWords = 0
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsBmpCounterArraySet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               startAddr,
    IN GT_U32               numberOfWords,
    IN GT_U32               counterValueArr[]
);

/**
* @internal prvCpssPortGroupsCounterArraySet function
* @endinternal
*
* @brief   set the counters on all the port groups , from the input register
*         the function set first active port group with the needed values , and set other
*         port groups with value 0 --> so the summary will not count multi-values
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] startAddr                - The register's start address to set the counters values to.
* @param[in] numberOfWords            - number of words to set (registers)
* @param[in] counterValueArr[]        - (array of)the counters values to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when numberOfWords = 0
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssPortGroupsCounterArraySet
(
    IN GT_U8 devNum,
    IN GT_U32 startAddr,
    IN GT_U32 numberOfWords,
    IN GT_U32 counterValueArr[]
);

/**
* @internal prvCpssPortGroupsNextPortGroupGet function
* @endinternal
*
* @brief   get the next active port group.
*         NOTE: the function 'wraparound' the port group meaning that when
*         currentPortGroupId == 3 --> the nextPortGroupPtr will be 0
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Puma2.
*
* @param[in] devNum                   - The PP device number
* @param[in] currentPortGroupId       - the current port group id
*
* @param[out] nextPortGroupIdPtr       - (pointer to) the next port group id(may 'wraparound')
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - currentPortGroupId >= CPSS_MAX_PORT_GROUPS_CNS
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - non multi port groups device
*/
GT_STATUS prvCpssPortGroupsNextPortGroupGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 currentPortGroupId,
    OUT GT_U32 *nextPortGroupIdPtr
);

/**
* @internal prvCpssPortGroupsPrevPortGroupGet function
* @endinternal
*
* @brief   get the portGroupId of the previous active port group.
*         NOTE: the function 'wraparound' the portGroupIds meaning that when
*         currentPortGroupId == 0 --> the prevPortGroupIdPtr will be 3
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Puma2.
*
* @param[in] devNum                   - The PP device number
* @param[in] currentPortGroupId       - the current port group Id
*
* @param[out] prevPortGroupIdPtr       - (pointer to) the previous portGroupId (may 'wraparound')
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - currentPortGroupId >= CPSS_MAX_PORT_GROUPS_CNS
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - non multi port groups device
*/
GT_STATUS prvCpssPortGroupsPrevPortGroupGet
(
    IN GT_U8   devNum,
    IN GT_U32  currentPortGroupId,
    OUT GT_U32 *prevPortGroupIdPtr
);

/**
* @internal prvCpssPortGroupsNumActivePortGroupsInBmpGet function
* @endinternal
*
* @brief   Get the number of active port groups in the given bitmap , for specific unit.
*         NOTE: unitId == PRV_CPSS_DXCH_UNIT_LAST_E means 'generic' (unaware to unit)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] unitId                   - unit index on which to get the logic.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon)
*                                      value PRV_CPSS_DXCH_UNIT_LAST_E means 'generic' (unaware to unit)
*
* @param[out] numActivePortGroupsPtr   - (pointer to)number of active port groups in the BMP.
*                                      for non multi-port groups device : value 1 is returned.
*                                      NOTE: param is GT_U16 because of prvCpssPortGroupCounterAverageValueGet(GT_U16)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPortGroupsNumActivePortGroupsInBmpGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN    PRV_CPSS_DXCH_UNIT_ENT    unitId,
    OUT GT_U16                 *numActivePortGroupsPtr
);

/**
* @internal prvCpssPortGroupsBitmapFirstActiveBitGet function
* @endinternal
*
* @brief   Get the first active bit from the given bitmap.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] bitmap                   - source bitmap.
*                                       the index of the found bit
*/
GT_U32 prvCpssPortGroupsBitmapFirstActiveBitGet
(
    IN  GT_U32     bitmap
);

/*******************************************************************************
* prvCpssGlobalPortToPortGroupIdConvert
*
* DESCRIPTION:
*       convert 'global' port number to 'portGroupId'.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number.
*       global_macPortNum - the global port MAC number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the portGroupId
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*portGroupId*/ prvCpssGlobalPortToPortGroupIdConvert
(
    IN GT_U8 devNum ,
    IN GT_U32 global_macPortNum
);

/*******************************************************************************
* prvCpssGlobalPortToLocalPortConvert
*
* DESCRIPTION:
*       convert 'global' port number to 'local' port number.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number.
*       global_macPortNum - the global port MAC number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the 'local' port number
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*local_macPortNum*/ prvCpssGlobalPortToLocalPortConvert
(
    IN GT_U8 devNum ,
    IN GT_U32 global_macPortNum
);

/*******************************************************************************
* prvCpssPhysicalGlobalPortToPortGroupIdConvert
*
* DESCRIPTION:
*       like prvCpssGlobalPortToPortGroupIdConvert but with
*       'CPU port' convert according to 'cpuPortPortGroupId'
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number.
*       global_macPortNum - the global port MAC number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the portGroupId
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*portGroupId*/ prvCpssPhysicalGlobalPortToPortGroupIdConvert(
    IN GT_U8 devNum ,
    IN GT_U32 global_macPortNum
);

/*******************************************************************************
* prvCpssLocalPortToGlobalPortConvert
*
* DESCRIPTION:
*       convert 'local' port number to 'global' port number.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number.
*       portGroupId     - the port group id.
*       local_macPortNum - the global port MAC number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the 'global' port number
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*global_macPortNum*/ prvCpssLocalPortToGlobalPortConvert
(
    IN GT_U8 devNum ,
    IN GT_U32 portGroupId ,
    IN GT_U32 local_macPortNum
);

/**
* @internal prvCpssMultiPortGroupsBmpCheckSpecificUnit function
* @endinternal
*
* @brief   Check port group bmp awareness for specific unit in the device.
*         check and return 'bad param' when device is multi-port group device ,
*         but portGroupsBmp indicate non active port groups
*         for non-multi port groups device --> function updates the portGroupsBmp
*         to CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*         for example:
*         the Bobcat3 is multi-port group device , but the FDB unit is single one, so
*         the portGroupsBmp from the application should be ignored (like in BC2 / xcat3)
*         NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
*         PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] portGroupsBmpPtr         - (pointer to) the port groups bitmap.
* @param[in] unitId                   - is one of PRV_CPSS_DXCH_UNIT_ENT
* @param[in,out] portGroupsBmpPtr         - (pointer to) the updated port groups bitmap.
*
* @retval GT_OK                    - the device supports the IN portGroupsBmpPtr
*                                       and updated the OUT portGroupsBmpPtr
* @retval GT_BAD_PARAM             - the IN portGroupsBmpPtr is illegal for the specific unit
*                                       in the device
*/
GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificUnit(
    IN    GT_U8                     devNum ,
    INOUT GT_PORT_GROUPS_BMP       *portGroupsBmpPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT    unitId
);

/**
* @internal prvCpssMultiPortGroupsBmpCheckSpecificTable function
* @endinternal
*
* @brief   Check port group bmp awareness for specific table in the device.
*         check and return 'bad param' when device is multi-port group device ,
*         but portGroupsBmp indicate non active port groups
*         for non-multi port groups device --> function updates the portGroupsBmp
*         to CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*         for example:
*         the Bobcat3 is multi-port group device , but the ARP/TS table is single one,
*         although the HA unit is 'per pipe'. So the portGroupsBmp from the application
*         should be ignored (like in BC2 / xcat3)
*         NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
*         PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] portGroupsBmpPtr         - (pointer to) the port groups bitmap.
*                                      tableId - is one of CPSS_DXCH_TABLE_ENT
* @param[in,out] portGroupsBmpPtr         - (pointer to) the updated port groups bitmap.
*
* @retval GT_OK                    - the device supports the IN portGroupsBmpPtr
*                                       and updated the OUT portGroupsBmpPtr
* @retval GT_BAD_PARAM             - the IN portGroupsBmpPtr is illegal for the specific table
*                                       in the device
*/
GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificTable(
    IN    GT_U8                     devNum ,
    INOUT GT_PORT_GROUPS_BMP       *portGroupsBmpPtr,
    IN    GT_U32 /*CPSS_DXCH_TABLE_ENT*/   tableId
);


/*******************************************************************************
* prvCpssSip5GlobalPhysicalPortToLocalMacConvert
*
* DESCRIPTION:
*       sip5 : convert 'global' physical port number to 'local' MAC port number.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon;
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       global_physicalPortNum - the global physical port number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the 'local' MAC port number
*       NOTE: value 0xFFFFFFFF means error
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*local_macPortNum*/ prvCpssSip5GlobalPhysicalPortToLocalMacConvert(
    IN GT_U8 devNum ,
    IN GT_U32 global_physicalPortNum
);

/*******************************************************************************
* prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert
*
* DESCRIPTION:
*       sip5 : convert 'global' physical port number to port group number.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon;
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       global_physicalPortNum - the global physical port number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the portGroupId
*       NOTE: value 0xFFFFFFFF means error
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*portGroupId*/ prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert(
    IN GT_U8 devNum ,
    IN GT_U32 global_physicalPortNum
);

/**
* @internal prvCpssMultiPortGroupsResourceBusyWaitByMask function
* @endinternal
*
* @brief   function do 'Busy wait' on specific mask of the register.
*         function wait on the register in the specified resource.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - the resource type
* @param[in] regAddr                  - the register address to query
* @param[in] regMask                  - the mask of the register that we wait for to clear
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMultiPortGroupsResourceBusyWaitByMask
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           regMask,
    IN GT_BOOL                          onlyCheck
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwMultiPortGroupsh */

