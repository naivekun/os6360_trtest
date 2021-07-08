/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssCommonDiag.h
*
* @brief Internal CPSS functions.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssCommonDiagh__
#define __prvCpssCommonDiagh__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>

/* Broadcast DFX client */
#define PRV_CPSS_DFX_CLIENT_BC_CNS  0x1C

/* Multicast DFX client */
#define PRV_CPSS_DFX_CLIENT_MC_CNS  0x1D

/* Broadcast DFX Data logging client */
#define PRV_CPSS_DFX_CLIENT_LOG_CNS 0x1E

/* maximal number of DFX pipes */
#define PRV_CPSS_DFX_MAX_PIPES_CNS  8

/* maximal number of DFX clients per pipe - 0..27  */
#define PRV_CPSS_DFX_MAX_CLIENTS_CNS  28

#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_BASE_ADDR_CNS                       0x0   /* base address */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_CAUSE_REG_CNS            0x1C  /* memory interrupt cause register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS             0x18  /* memory interrupt mask register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS              0x14  /* memory control register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_ADDR_REG_CNS       0x8   /* memory erroneous address register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_SEGMENT_REG_CNS    0x10  /* memory erroneous segment register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ECC_SYNDROME_REG_CNS         0xC   /* memory ECC syndrome register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRORS_COUNTER_REG_CNS       0x4   /* memory error counter register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS          0x50  /* client status 0 register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS1_REG_CNS          0x54  /* client status 1 register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS2_REG_CNS          0x58  /* client status 2 register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS3_REG_CNS          0x5C  /* client status 3 register */

#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_CAUSE_CNS                0x88  /* client interrupt cause register */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_MASK_CNS                 0x8C  /* client interrupt mask register */



/* MACROs to extract members from map key */
#define PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(_key) ((_key) >> 12)
#define PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(_key) (((_key) >> 7) & 0x1F)
#define PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(_key) ((_key) & 0x7F)

/* Macro for checking if a test passed or failed:
   status holds the return code status.
   testStatus holds the result of the test (GT_TRUE - pass, GT_FALSE - fail).
   If the test failed, the function should exit with return code GT_OK. */
#define PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status,testStatus)         \
            if ((status) != GT_OK)                                     \
            {                                                          \
                return (status);                                       \
            }                                                          \
            if ((testStatus) == GT_FALSE)                              \
            {                                                          \
                return GT_OK;                                          \
            }


/*
 * typedef: PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC
 *
 * Description:
 *      Check register existance in particular port group.
 *
 * INPUTS:
 *       devNum          - device number
 *       portGroupId     - the port group Id, to support multi-port-group
 *                         devices that need to access specific port group
 *       regAddr         - register address
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       GT_TRUE    - register exists in specific port group
 *       GT_FALSE   - register doesn't exist in specific port group
 *
 * Comment:
 */
typedef GT_BOOL (*PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 regAddr
);

/**
* @enum PRV_CPSS_DIAG_REGS_SEARCH_TYPE_ENT
 *
 * @brief Test profile.
*/
typedef enum{

    /** @brief look for registers in use number */
    PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E,

    /** @brief look for registers addresses */
    PRV_CPSS_DIAG_REGS_ADDR_SEARCH_E,

    /** @brief look for registers data */
    PRV_CPSS_DIAG_REGS_DATA_SEARCH_E,

    /** @brief look for
     *  Reset and Init Controller registers data
     */
    PRV_CPSS_DIAG_REGS_DATA_RESET_AND_INIT_CTRL_SEARCH_E

} PRV_CPSS_DIAG_REGS_SEARCH_TYPE_ENT;

/**
* @struct PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC
*
* @brief Compare memory BIST data
*/
typedef struct{

    /** @brief bits [19:15]
     *  0-27 - DFX Clients 0 to 27
     *  28 - DFX Clients Broadcast
     *  29 - DFX Clients Multicast
     *  30 - DFX Link Data Logging
     */
    GT_U32 dfxUnitClientIdx;

    /** @brief bits [14:0]
     *  Relevant only if dfxUnitClientIdx = 31
     */
    GT_U32 dfxServerRegisterOffset;

    /** @brief bit [14]
     *  XSB Select
     *  1 - XSB
     *  0 - Memories and Client Registers
     *  Relevant only if dfxUnitClientIdx = 0-29
     */
    GT_U32 dfxXsbSelect;

    /** @brief bits [13:9]
     *  XSB Number
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 1
     */
    GT_U32 dfxXsbNumber;

    /** @brief bits [8:0]
     *  XSB Register Offset
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 1
     */
    GT_U32 dfxXsbRegOffset;

    /** @brief bit [13]
     *  DFX Client Select
     *  0 - Memory
     *  1 - Client Registers
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 0
     */
    GT_U32 dfxClientSelect;

    /** @brief bits [12:0]
     *  DFX Client Register Offset
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 0 and
     *  DFX Client Select = 1
     */
    GT_U32 dfxClientRegisterOffset;

    /** @brief bits [12:6]
     *  DFX Client Memory Number
     *  0-124  - memories from 0 to 124
     *  125-126 - memories Multicast
     *  127   - memories Broadcast
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 0 and
     *  DFX Client Select = 0
     *  dfxClientMemoryRegisterOffset- bits [5:0]
     *  DFX Client Memory Register Offset
     *  Relevant only if dfxUnitClientIdx = 0-29 and
     *  XSB Select = 0 and
     *  DFX Client Select = 0
     */
    GT_U32 dfxClientMemoryNumber;

    GT_U32 dfxClientMemoryRegisterOffset;

} PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC;

/**
* @struct
*         PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC
*
* @brief First stage map entry.
*/
typedef struct{

    /** @brief DFX pipe index */
    GT_U32 dfxPipeIndex;

    /** @brief DXF client index */
    GT_U32 dfxClientIndex;

} PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC;

/**
* @struct PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC
*
* @brief Data integrity memory mapping info
*        ****************************************
*        formula for calculating the entry index of the 'HW table' is:
*          formula = startAddress +
*              (U / logicalBlockSize) * logicalBlockAddressSteps +
*              (U % logicalBlockSize) * addressIncrement
*          where
*              U = firstTableLine + S,
*              S is Memory entry index

*/
typedef struct{

    /** @brief DB key
     *  key coding is next - (dfxPipeIndex << 12) | (dfxClientIndex << 7) | (dfxMemoryIndex):
     *  dfxMemoryIndex - bits 0..6
     *  dfxClientIndex - bits 7..11
     *  dfxPipeIndex  - bits 12..31
     */
    GT_U32 key;

    /** @brief memory type */
    GT_U32 memType;

    /** @brief memory DFX-protection type */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectionType;

    /** @brief memory external protection type (non-DFX) */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT externalProtectionType;

    /** @brief port group ID of event has happened */
    GT_U32 causePortGroupId;

    /** @brief first table data bit */
    GT_U32 firstTableDataBit;

    /** @brief last table data bit */
    GT_U32 lastTableDataBit;

    /** @brief HW table name (HW table may be build of several DFX memories) */
    GT_U32 hwTableName;

    /** @brief The first functional address of this table.
     * See Example in Multi-Bank-Table sheet" */
    GT_U32 startAddress;

    /** @brief In case the table is splitted to logical blocks.
     *  For example: Each action table is divided to logical blocks of 256 entries,
     *  see example in logical Table Sheet
     *  When unused set to table depth.
     *  See formula in general description of this struct.
     */
    GT_U32 logicalBlockSize;

    /** @brief The step of the first address inside the logical block.
     *  see example in logical Table Sheet
     *  When unused set to 0
     */
    GT_U32 logicalBlockAddressSteps;

    /** @brief The functional address difference between two subsequent entries.
     *  By defualt should be set to 1.
     *  See formula in general description of this struct.
     */
    GT_U32 addressIncrement;

    /** @brief The first logical table entry represented by this physical memory.
     *  See formula in general description of this struct.
     */
    GT_U32 firstTableLine;

    /** @brief The last logical table entry represented by this physical memory.

     */
    GT_U32 lastTableLine;

    /** @brief the usage of the memory : config/data/state */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT memUsageType;

    /** @brief what recommended to do when error detected.
     *  (none/scrub/hw fix/sw fix/reboot/packet loss)
     *  formula for calculating the entry index of the 'HW table' that got error
     *  formula required:
     *  formula = G + INT(U/H)I + MOD(U,H)J
     *  U = N+S , where: S is "Physical Entry Error" from the Register)
     *  N is FirstTableline
     *  ==========
     */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT correctionMethod;

} PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC;

/**
* @enum PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT
*
* @brief This enum defines BIST results status
*/
typedef enum{

    /** @brief result status is not ready
     *  (since the BIST is still
     *  running).
     */
    PRV_CPSS_DFX_DIAG_BIST_STATUS_NOT_READY_E,

    /** BIST passed successfully. */
    PRV_CPSS_DFX_DIAG_BIST_STATUS_PASS_E,

    /** BIST failed. */
    PRV_CPSS_DFX_DIAG_BIST_STATUS_FAIL_E

} PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT;

/**
* @internal PRV_CPSS_DFX_MEMORY_PIPE_ID_SET_FUNC
* @endinternal
*
* @brief   Function sets DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon, Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
typedef GT_STATUS (*PRV_CPSS_DFX_MEMORY_PIPE_ID_SET_FUNC)
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId
);

/**
* @internal PRV_CPSS_DFX_PIPES_BMP_GET_FUNC
* @endinternal
*
* @brief   Return bitmap of DFX pipes relevant for the device .
*         If bit #N is raised pipeId #N exists in the device.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @param[out] pipesBmpPtr              - (pointer to) pipes bitmap
*
* @retval GT_OK                    - on success
*/
typedef GT_STATUS (*PRV_CPSS_DFX_PIPES_BMP_GET_FUNC)
(
    IN  GT_U8  devNum,
    OUT GT_U32 *pipesBmpPtr
);

/**
* @internal PRV_CPSS_DFX_MEMORY_REG_WRITE_FUNC
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
typedef GT_STATUS (*PRV_CPSS_DFX_MEMORY_REG_WRITE_FUNC)
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 regData
);

/**
* @internal PRV_CPSS_DFX_MEMORY_REG_READ_FUNC
* @endinternal
*
* @brief   Function reads DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
typedef GT_STATUS (*PRV_CPSS_DFX_MEMORY_REG_READ_FUNC)
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    OUT GT_U32 *regDataPtr
);

/**
* @internal PRV_CPSS_DFX_CLIENT_REG_READ_FUNC
* @endinternal
*
* @brief   Function reads DFX client registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] clientReg                - DFX client register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
typedef GT_STATUS (*PRV_CPSS_DFX_CLIENT_REG_READ_FUNC)
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 clientReg,
    OUT GT_U32 *regDataPtr
);

/**
* @struct PRV_CPSS_DIAG_STC
 *
 * @brief The structure defines diag params
*/
typedef struct{

    /** @brief the prvCpssDfxClientSetRegField function use these
     *  two parameters to get value of register for the
     *  following update of required fields. These fields are
     *  used for MC/BC access when read is may be done only
     *  from some client that is member of the MC group.
     */
    GT_U32 dfxMcAccessGetPipe;

    /** @brief see description above */
    GT_U32 dfxMcAccessGetClient;

    /** @brief function sets DFX pipe select register
    */
    PRV_CPSS_DFX_MEMORY_PIPE_ID_SET_FUNC   dfxMemoryPipeIdSetFunc;
    /** @brief function writes DFX memory registers.
    */
    PRV_CPSS_DFX_MEMORY_REG_WRITE_FUNC     dfxMemoryWriteFunc;
    /** @brief function reads DFX memory registers.
    */
    PRV_CPSS_DFX_MEMORY_REG_READ_FUNC      dfxMemoryRegReadFunc;
    /** @brief function reads DFX client registers.
    */
    PRV_CPSS_DFX_CLIENT_REG_READ_FUNC      dfxClientReadRegFunc;
    /** @brief function returns bitmap of DFX pipes relevant for the device
    */
    PRV_CPSS_DFX_PIPES_BMP_GET_FUNC        dfxPipesBmpGetFunc;

} PRV_CPSS_DIAG_STC;

/**
* @internal prvCpssDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal prvCpssDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal prvCpssDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers in use for the PP.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id, to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
*
* @param[out] regsNumPtr               - number of registers in use
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegsNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              portGroupId,
    IN  GT_U32                              *regsListPtr,
    IN  GT_U32                              regsListSize,
    IN  GT_U32                              offset,
    IN  PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC  checkRegExistFuncPtr,
    OUT GT_U32                              *regsNumPtr
);

/**
* @internal prvCpssDiagRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr               - number of registers to get their addresses and data
* @param[in,out] regsNumPtr               - number of registers that their data were successfully
*                                      retrieved
*
* @param[out] regAddrPtr               - list of registers addresses
* @param[out] regDataPtr               - list of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                portGroupId,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
);

/**
* @internal prvCpssDiagResetAndInitControllerRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use for Reset and Init
*         Controller.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr               - number of registers to get their addresses and data
* @param[in,out] regsNumPtr               - number of registers that their data were successfully
*                                      retrieved
*
* @param[out] regAddrPtr               - list of registers addresses
* @param[out] regDataPtr               - list of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagResetAndInitControllerRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
);

/**
* @internal prvCpssDiagUnitRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - Pointer to the regsiters list
* @param[in] regsListSize             - Size of registers list
* @param[in] offset                   - Only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] unitIdAddrPattern        - pattern to search registers os specific unit. Only registers
*                                      with address that satisfies the formula
*                                      (register_address & unitIdAddrPattern) == unitIdAddrPattern
*                                      will be counted/dumped.
* @param[in] unitIdAddrMask           - mask to search registers of specific unit.
* @param[in] checkRegExistFuncPtr     - Pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr               - Number of registers to get their addresses and data
* @param[in,out] regsNumPtr               - Number of registers that their data were successfully
*                                      retrieved
*
* @param[out] regAddrPtr               - List of registers addresses
* @param[out] regDataPtr               - List of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagUnitRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                portGroupId,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    GT_U32                                unitIdAddrPattern,
    IN    GT_U32                                unitIdAddrMask,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
);

/**
* @internal prvCpssDiagUnitRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers in use for the the given unit in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The device number
* @param[in] regsListPtr              - Pointer to the regsiters list
* @param[in] regsListSize             - Size of registers list
* @param[in] offset                   - Only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] unitIdAddrPattern        - pattern to search registers os specific unit. Only registers
*                                      with address that satisfies the formula
*                                      (register_address & unitIdAddrPattern) == unitIdAddrPattern
*                                      will be counted/dumped.
* @param[in] unitIdAddrMask           - mask to search registers of specific unit.
*
* @param[out] regsNumPtr               - Number of registers in use
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagUnitRegsNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *regsListPtr,
    IN  GT_U32                              regsListSize,
    IN  GT_U32                              offset,
    IN  GT_U32                              unitIdAddrPattern,
    IN  GT_U32                              unitIdAddrMask,
    OUT GT_U32                              *regsNumPtr
);

/**
* @internal prvCpssDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
GT_STATUS prvCpssDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
);

/**
* @internal prvCpssDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - The device number
* @param[in] regsPtr          - The array of register addresses to test
* @param[in] regMasksPtr      - The array of register masks to test
*                              Bits out of masks wriitten as read and not checked.
*                              This pointer can be NULL - so 0xFFFFFFFF masks will be used
* @param[in] regsArraySize    - Number of registers in regsArray
*
* @param[out] testStatusPtr   - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] badRegPtr       - Address of the register which caused the failure if
*                               testStatusPtr is GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr      - The value read from the register which caused the failure
*                               if testStatusPtr is GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE
* @param[out] writeValPtr     - The value written to the register which caused the failure
*                               if testStatusPtr is GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking prvCpssDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagAllRegTest
(
    IN  GT_U8   devNum,
    IN  GT_U32  *regsPtr,
    IN  GT_U32  *regMasksPtr,
    IN  GT_U32  regsArraySize,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
);

/**
* @internal prvCpssDfxClientSetRegField function
* @endinternal
*
* @brief   Function set a field in a DFX client register.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] pipe                     - DFX pipe the client belongs to.
*                                      (not relevant for BC, MC or Data logging clients).
* @param[in] client                   - DFX client to write to.
* @param[in] clientRegAddr            - The client register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note Since reading BC, MC or Data logging DFX clients registers is prohibited
*       client 0 value is read to emulate the default or already existing value.
*
*/
GT_STATUS prvCpssDfxClientSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   pipe,
    IN GT_U32   client,
    IN GT_U32   clientRegAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
);

/**
* @internal prvCpssDfxMemoryRegRead function
* @endinternal
*
* @brief   Function reads DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    OUT GT_U32 *regDataPtr
);

/**
* @internal prvCpssDfxMemoryRegWrite function
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegWrite
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 regData
);

/**
* @internal prvCpssDfxMemoryErrorInjectionSet function
* @endinternal
*
* @brief   Inject error to a selected DFX client RAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] mode                     - error injection mode
* @param[in] enable                   - enable/disable error injection
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryErrorInjectionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           pipeId,
    IN GT_U32                                           clientId,
    IN GT_U32                                           memNumber,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT   mode,
    IN GT_BOOL                                          enable
);

/**
* @internal prvCpssDfxMemoryPipeIdSet function
* @endinternal
*
* @brief   Function sets DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryPipeIdSet
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId
);

/**
* @internal prvCpssDfxMemoryAllPipesSet function
* @endinternal
*
* @brief   Function sets all pipes in DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryAllPipesSet
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDfxClientRegAddressGet function
* @endinternal
*
* @brief   Get DFX client register address
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] client                   - DFX client number.
* @param[in] regAddr                  - Relative register address in client memory space.
*
* @param[out] clientRegAddrPtr         - (pointer to) register addrress in specific client
*                                      memory space.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong client number
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxClientRegAddressGet
(
    IN  GT_U32   client,
    IN  GT_U32   regAddr,
    OUT GT_U32   *clientRegAddrPtr
);

/**
* @internal prvCpssDfxMemoryRegAddressGet function
* @endinternal
*
* @brief   Get DFX memory register address
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] client                   - DFX client number.
* @param[in] memory                   - DFX client RAM number.
* @param[in] regAddr                  - Relative register address in RAM space.
*
* @param[out] memoryRegAddrPtr         - (pointer to) register addrress in for specific client
*                                        memory
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong client or RAM number
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegAddressGet
(
    IN  GT_U32   client,
    IN  GT_U32   memory,
    IN  GT_U32   regAddr,
    OUT GT_U32   *memoryRegAddrPtr
);

/**
* @internal prvCpssDfxClientRegRead function
* @endinternal
*
* @brief   Function reads DFX client registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] clientReg                - DFX client register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxClientRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 clientReg,
    OUT GT_U32 *regDataPtr
);

/**
* @internal prvCpssDfxMemoryGetRegField function
* @endinternal
*
* @brief   Function reads DFX memory register's field
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
*
* @param[out] fieldDataPtr             - (pointer to) register's field data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryGetRegField
(
    IN GT_U8   devNum,
    IN GT_U32  pipeId,
    IN GT_U32  clientId,
    IN GT_U32  memNumber,
    IN GT_U32  memReg,
    IN GT_U32  fieldOffset,
    IN GT_U32  fieldLength,
    OUT GT_U32 *fieldDataPtr
);

/**
* @internal prvCpssDfxMemorySetRegField function
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemorySetRegField
(
    IN GT_U8  devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
);

/**
* @internal prvCpssDfxBuildAddr function
* @endinternal
*
* @brief   Function calculates DFX client/memory address.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - device number
* @param[in] addrStrPtr               - (pointer to) DFX address structure
*
* @param[out] addrPtr                  - (pointer to) calculated address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong addrStrPtr members
*/
GT_STATUS prvCpssDfxBuildAddr
(
    IN  GT_U8                                           devNum,
    IN  PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC       *addrStrPtr,
    OUT GT_U32                                          *addrPtr
);

/**
* @internal prvCpssDfxDiagBistStartSet function
* @endinternal
*
* @brief   Start Stop MC BIST on specific DFX clients registers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] enable                   - GT_TRUE:  Enable and Start.
*                                      GT_FALSE: Disable and Stop.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxDiagBistStartSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal prvCpssDfxDiagBistStatusCheck function
* @endinternal
*
* @brief  Check BIST status of all relevant clients.
*         Configuration done according to memory map DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                - PP device number
* @param[in] dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*
* @param[out] resultsStatusPtr        - (pointer to) BIST results status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDfxDiagBistStatusCheck
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              dfxClientsBmpArr[],
    OUT PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT   *resultsStatusPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonDiagh__ */
