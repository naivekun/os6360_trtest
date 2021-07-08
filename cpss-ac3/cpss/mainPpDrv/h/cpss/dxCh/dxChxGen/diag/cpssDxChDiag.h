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
* @file cpssDxChDiag.h
*
* @brief Diag APIs for CPSS DxCh.
*
* PRBS sequence for tri-speed ports:
* 1. Enable PRBS checker on receiver port (cpssDxChDiagPrbsPortCheckEnableSet)
* 2. Check that checker initialization is done (cpssDxChDiagPrbsPortCheckReadyGet)
* 3. Set CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E transmit mode on transmiting port
* (cpssDxChDiagPrbsPortTransmitModeSet)
* 4. Enable PRBS generator on transmiting port (cpssDxChDiagPrbsPortGenerateEnableSet)
* 5. Check results on receiving port (cpssDxChDiagPrbsPortStatusGet)
*
* PRBS sequence for XG ports:
* 1. Set CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E or CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E
* transmit mode on both ports(cpssDxChDiagPrbsPortTransmitModeSet)
* 2. Enable PRBS checker on receiver port (cpssDxChDiagPrbsPortCheckEnableSet)
* 3. Enable PRBS generator on transmiting port (cpssDxChDiagPrbsPortGenerateEnableSet)
* 4. Check results on receiving port (cpssDxChDiagPrbsPortStatusGet)
*
* @version   34
********************************************************************************
*/
#ifndef __cpssDxChDiagh
#define __cpssDxChDiagh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/diag/cpssDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>

/**
* @enum CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT
*
* @brief Transmit modes.
*/
typedef enum{

    /** Regular Mode: Input is from the MAC PCS Tx block. */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E,

    /** PRBS Mode: 1.25 Gbps input is from the PRBS Generator. */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E,

    /** Zeros Constant. */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ZEROS_E,

    /** Ones Constant. */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ONES_E,

    /** Cyclic Data; The data in Cyclic Date Register 0 3 is transmitted. */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_CYCLIC_E,

    /** @brief PRBS7.
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E,

    /** @brief PRBS9
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E,

    /** @brief PRBS15
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E,

    /** @brief PRBS23
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E,

    /** @brief PRBS31
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E,

    /** @brief _1T
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E,

    /** @brief _2T
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E,

    /** @brief _5T
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E,

    /** @brief _10T
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E,

    /** @brief DFETraining
     *  (Applicable devices: xCat3, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_DFETraining,

    /** for validity checks */
    CPSS_DXCH_DIAG_TRANSMIT_MODE_MAX_E

} CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT;

/**
* @enum CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT
*
* @brief External Memory BIST Pattern.
*/
typedef enum{

    /** PBS1 pattern (basic) */
    CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_BASIC_E,

    /** DQ0 pattern (long) */
    CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_STRESS_E,

    /** SSO0 pattern (long) */
    CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_STRESS_1_E

} CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT;

/**
* @struct CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC
*
* @brief External Memory BIST result
*/
typedef struct{

    /** counter of errors */
    GT_U32 errCounter;

    /** @brief last address (offset) in memory that caused an error
     *  Comments:
     *  None
     */
    GT_U32 lastFailedAddr;

} CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC;

/* maximal amount of external memories (for BIST) */
#define CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS 5

/**
* @enum CPSS_DIAG_PP_MEM_BIST_TYPE_ENT
*
* @brief Packet Process memory type for BIST.
*/
typedef enum{

    /** The PCL TCAM. */
    CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E,

    /** The ROUTER TCAM. */
    CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E

} CPSS_DIAG_PP_MEM_BIST_TYPE_ENT;

/**
* @enum CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT
*
* @brief Packet Process memory type for BIST.
*/
typedef enum{

    /** @brief The MBIST (Memory BIST)
     *  is a pure memory check that treats the TCAM memory as regular
     *  memory and performs standard read and write operations.
     */
    CPSS_DIAG_PP_MEM_BIST_PURE_MEMORY_TEST_E,

    /** @brief The COMP_BIST is a check of
     *  the memory behavior under a compare BIST action. It can configure
     *  different algorithms based on configurable flexible commands in
     *  dedicated opcode registers. These registers are configured for
     *  every compare test algorithm, and changed for a new test when
     *  the BIST_DONE indication is received.
     */
    CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E

} CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT;

/* total number of memory BIST tests of compare type */
#define CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS 25

/**
* @struct CPSS_DIAG_PP_MEM_BIST_COMPARE_MEM_DATA_STC
*
* @brief Compare memory BIST data
*/
typedef struct{

    /** value to set in PCL TCAM BIST opcode command register */
    GT_U32 pclOpCode;

    /** value to set in Router TCAM BIST opcode command register */
    GT_U32 rtrOpCode;

    /** @brief value to set in Expected Hit TCAM BIST register
     *  Comments:
     *  None
     */
    GT_U32 expectedHitVal;

} CPSS_DIAG_PP_MEM_BIST_COMPARE_MEM_DATA_STC;

/* use CPSS default delay for serdes optimization algorithm */
#define CPSS_DXCH_DIAG_SERDES_TUNE_PRBS_TIME_DEFAULT_CNS 0

/**
* @struct CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC
*
* @brief Coordinates of lane where to run optimization algorithm
*/
typedef struct{

    /** number of port */
    GT_PHYSICAL_PORT_NUM portNum;

    /** number of one of serdes lanes occupied by port */
    GT_U32 laneNum;

} CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC;

/**
* @struct CPSS_DXCH_DIAG_BIST_RESULT_STC
*
* @brief BIST result structure
*/
typedef struct{

    /** memory type */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType;

    /** @brief memory location indexes
     *  Comments:
     *  None
     */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC location;

} CPSS_DXCH_DIAG_BIST_RESULT_STC;

/**
* @enum CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT
*
* @brief Defines optimization algorithm modes
*/
typedef enum{

    /** Accuracy aware */
    CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E

} CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT;


/**
* @enum CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT
*
* @brief Defines Temperature Sensors.
*/
typedef enum{

    /** Temperature Sensor 0. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E,

    /** Temperature Sensor 1. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E,

    /** Temperature Sensor 2. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E,

    /** Temperature Sensor 3. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E,

    /** Temperature Sensor 4. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E,

    /** Average temperature of all sensors. */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E,

    /** @brief Temperature of hottest sensor.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E

} CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT;

/**
* @enum CPSS_DXCH_DIAG_BIST_STATUS_ENT
*
* @brief This enum defines BIST results status
*/
typedef enum{

    /** @brief result status is not ready
     *  (since the BIST is still
     *  running).
     */
    CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E,

    /** BIST passed successfully. */
    CPSS_DXCH_DIAG_BIST_STATUS_PASS_E,

    /** BIST failed. */
    CPSS_DXCH_DIAG_BIST_STATUS_FAIL_E

} CPSS_DXCH_DIAG_BIST_STATUS_ENT;

/**
* @internal cpssDxChDiagMemTest function
* @endinternal
*
* @brief   Performs memory test on a specified memory location and size for a
*         specified memory type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
* @param[in] memType                  - The packet processor memory type to verify.
* @param[in] startOffset              - The offset address to start the test from.
* @param[in] size                     - The memory size in byte to test (start from offset).
* @param[in] profile                  - The test profile.
*
* @param[out] testStatusPtr            - (pointer to) test status. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - (pointer to) address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong memory type
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by writing and reading a test pattern.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*       Supported memories:
*       - Buffer DRAM
*       - MAC table memory
*       - VLAN table memory
*       For buffer DRAM:
*       startOffset must be aligned to 64 Bytes and size must be in 64 bytes
*       resolution.
*       For MAC table:
*       startOffset must be aligned to 16 Bytes and size must be in 16 bytes
*       resolution.
*       For VLAN table:
*       DX CH devices: startOffset must be aligned to 12 Bytes and size must
*       be in 12 bytes resolution.
*       DX CH2 and above devices: startOffset must be aligned to 16 Bytes
*       and size must be in 16 bytes resolution.
*/
GT_STATUS cpssDxChDiagMemTest
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
);

/**
* @internal cpssDxChDiagAllMemTest function
* @endinternal
*
* @brief   Performs memory test for all the internal and external memories.
*         Tested memories:
*         - Buffer DRAM
*         - MAC table memory
*         - VLAN table memory
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagMemTest in loop for all the
*       memory types and for AA-55, random and incremental patterns.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*/
GT_STATUS cpssDxChDiagAllMemTest
(
    IN GT_U8                     devNum,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *addrPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
);

/**
* @internal cpssDxChDiagMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*/
GT_STATUS cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
);

/**
* @internal cpssDxChDiagMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagMemRead
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
);

/**
* @internal cpssDxChDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssDxChDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssDxChDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssDxChDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssDxChDiagPhyRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] data                     - data to write
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
GT_STATUS cpssDxChDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssDxChDiagPhyRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - (pointer to) read data
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
GT_STATUS cpssDxChDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssDxChDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
);

/**
* @internal cpssDxChDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssDxChDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
);

/**
* @internal cpssDxChDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
GT_STATUS cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
);

/**
* @internal cpssDxChDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssDxChDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*/
GT_STATUS cpssDxChDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
);

/**
* @internal cpssDxChDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
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
GT_STATUS cpssDxChDiagRegTest
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
* @internal cpssDxChDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*/
GT_STATUS cpssDxChDiagAllRegTest
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
);

/**
* @internal cpssDxChDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssDxChDiagPrbsPortTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
);

/**
* @internal cpssDxChDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssDxChDiagPrbsPortTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
);

/**
* @internal cpssDxChDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
);

/**
* @internal cpssDxChDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssDxChDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
);

/**
* @internal cpssDxChDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
);

/**
* @internal cpssDxChDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckReadyGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL        *isReadyPtr
);

/**
* @internal cpssDxChDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortStatusGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *checkerLockedPtr,
    OUT  GT_U32         *errorCntrPtr
);

/**
* @internal cpssDxChDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssDxChDiagPrbsCyclicDataSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_U32         cyclicDataArr[4]
);

/**
* @internal cpssDxChDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssDxChDiagPrbsCyclicDataGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         cyclicDataArr[4]
);

/**
* @internal cpssDxChDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssDxChDiagPrbsSerdesTransmitModeSet.
*       2. Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
);

/**
* @internal cpssDxChDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL       *enablePtr
);

/**
* @internal cpssDxChDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
(
    IN   GT_U8                            devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode
);

/**
* @internal cpssDxChDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeGet
(
    IN   GT_U8                             devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT *modePtr
);

/**
* @internal cpssDxChDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*                                      NOT APPLICABLE:Caelum,
*                                      Aldrin, AC3X, Bobcat3,
*                                      Aldrin2, Falcon.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32    laneNum,
    OUT  GT_BOOL  *lockedPtr,
    OUT  GT_U32   *errorCntrPtr,
    OUT  GT_U64   *patternCntrPtr
);

/**
* @internal cpssDxChDiagMemoryBistsRun function
* @endinternal
*
* @brief   Runs BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      DxCh3; Lion; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] bistTestType             - BIST test type (pure memory test/compare memory test)
* @param[in] timeOut                  - maximal time in milisecond to wait for BIST finish.
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after test.
*                                      GT_FALSE - Don't clear memory after test.
* @param[in] testsToRunBmpPtr         - bitmap of required test related for
*                                      CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E
*                                      (from 0 to CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS,
*                                      so minimum bitmap size 32 bits, maximum depends on future
*                                      number of tests); 1 - run test, 0 - don't run test.
*                                      Bit 0 set in bitmap means run test 0.0 from "Table 1: TCAM Compare BIST:
*                                      Parameters and Expected Result Configurations"
*                                      (see full tests list and definitions in Application Notes)
*
* @param[out] testsResultBmpPtr        - pointer to bitmap with results of compare memory test,
*                                      !!! pay attention: 0 - test pass, 1 - test failed;
*                                      if NULL, just final result of test will be returned;
*                                      regarding size see testsToRunBmpPtr above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
GT_STATUS cpssDxChDiagMemoryBistsRun
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT  bistTestType,
    IN  GT_U32                               timeOut,
    IN  GT_BOOL                              clearMemoryAfterTest,
    IN  GT_U32                               *testsToRunBmpPtr,
    OUT GT_U32                               *testsResultBmpPtr
);

/**
* @internal cpssDxChDiagMemoryBistBlockStatusGet function
* @endinternal
*
* @brief   Gets redundancy block status.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] blockIndex               - block index
*                                      CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
*                                      valid blockIndex: 0 to 13.
*                                      CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
*                                      valid blockIndex: 0 to 19.
*
* @param[out] blockFixedPtr            - (pointer to) block status
*                                      GT_TRUE - row in the block was fixed
*                                      GT_FALSE - row in the block wasn't fixed
* @param[out] replacedIndexPtr         - (pointer to) replaced row index.
*                                      Only valid if  blockFixedPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note Each redundancy block contains 256 rows. Block 0: rows 0 to 255,
*       block 1: raws 256 to 511 ...
*       One raw replacement is possible though the full redundancy block
*       of 256 rows.
*
*/
GT_STATUS cpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  GT_U32                               blockIndex,
    OUT GT_BOOL                              *blockFixedPtr,
    OUT GT_U32                               *replacedIndexPtr
);

/**
* @internal cpssDxChDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
);

/**
* @internal cpssDxChDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
);

/**
* @internal cpssDxChDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      (APPLICABLE RANGES: Lion2 -142..228)
*                                      (APPLICABLE RANGES: xCat3, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X -277..198)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureThresholdSet
(
    IN  GT_U8  devNum,
    IN  GT_32  thresholdValue
);

/**
* @internal cpssDxChDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureThresholdGet
(
    IN  GT_U8  devNum,
    OUT GT_32  *thresholdValuePtr
);

/**
* @internal cpssDxChDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureGet
(
    IN  GT_U8    devNum,
    OUT GT_32    *temperaturePtr
);

/**
* @internal cpssDxChDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS cpssDxChDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
);

/**
* @internal cpssDxChDiagPortGroupMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the specific port group in
*         PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupMemWrite
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
);


/**
* @internal cpssDxChDiagPortGroupMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the specific port group
*         in the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupMemRead
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
);

/**
* @internal cpssDxChDiagPortGroupRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the specific port group in the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupRegsNumGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *regsNumPtr
);

/**
* @internal cpssDxChDiagPortGroupRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values for the specific port
*         group in the PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*/
GT_STATUS cpssDxChDiagPortGroupRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
);

/**
* @internal cpssDxChDiagSerdesTuningTracePrintEnable function
* @endinternal
*
* @brief   Enable/disable intermidiate reports print.
*         Status set per system not per device; devNum used just for applicable
*         device check
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] enable                   - enable/disable intermidiate reports print
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChDiagSerdesTuningTracePrintEnable
(
    IN    GT_U8  devNum,
    IN    GT_BOOL enable
);

/**
* @internal cpssDxChDiagSerdesTuningRxTune function
* @endinternal
*
* @brief   Run the SerDes optimization algorithm and save its results in CPSS
*         internal SW DB for further usage in SerDes power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] portLaneArrPtr           - array of pairs (port;lane) where to run optimization
*                                      algorithm
* @param[in] portLaneArrLength        - number of pairs in portLaneArrPtr
* @param[in] prbsType                 - Type of PRBS used for test.
* @param[in] prbsTime                 - duration[ms] of the PRBS test during suggested setup
*                                      verification.
*                                      If (CPSS_DXCH_PORT_SERDES_TUNE_PRBS_TIME_DEFAULT_CNS
*                                      == prbsTime) then default
*                                      PRV_CPSS_DXCH_PORT_SERDES_TUNE_PRBS_DEFAULT_DELAY_CNS
*                                      will be used
* @param[in] optMode                  - optimization algorithm mode
*
* @param[out] optResultArrPtr          - Array of algorithm results, must be of length enough
*                                      to keep results for all tested lanes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error, signal detect fail
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
* @retval GT_BAD_PARAM             - if one of parameters is wrong
*
* @note If algorithm fails for some lane as result of HW problem appropriate
*       entry in optResultArrPtr will be set to 0xffffffff
*       Marvell recommends running the following PRBS type per interface type:
*       - SERDES using 8/10 bit encoding (DHX,QSGMII,HGS4,HGS,XAUI,QX,
*       SGMII 2.5,SGMII,100FX): PRBS7
*       - RXAUI: PRBS15
*       - SERDES using 64/66 bit encoding (XLG): PRBS31
*
*/
GT_STATUS cpssDxChDiagSerdesTuningRxTune
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    *portLaneArrPtr,
    IN  GT_U32                                      portLaneArrLength,
    IN  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType,
    IN  GT_U32                                      prbsTime,
    IN  CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC              *optResultArrPtr
);

/**
* @internal cpssDxChDiagSerdesTuningSystemInit function
* @endinternal
*
* @brief   Initialize Serdes optimization system. Allocates resources for algorithm
*         which will be released by cpssDxChDiagSerdesTuningSystemClose
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
GT_STATUS cpssDxChDiagSerdesTuningSystemInit
(
    IN    GT_U8  devNum
);

/**
* @internal cpssDxChDiagSerdesTuningSystemClose function
* @endinternal
*
* @brief   Stop Tx if it was engaged and free all resources allocated by tuning
*         algorithm.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
GT_STATUS cpssDxChDiagSerdesTuningSystemClose
(
    IN    GT_U8  devNum
);

/**
* @internal cpssDxChDiagRegDefaultsEnableSet function
* @endinternal
*
* @brief   Set the initRegDefaults flag to allow system init with no HW writes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with register defaults.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChDiagRegDefaultsEnableSet
(
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet function
* @endinternal
*
* @brief   Set the fastBootSkipOwnDeviceInit flag to allow system init with no HW write
*         to the device Device_ID within a Prestera chipset.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with no HW write to the device Device_ID.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet
(
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS cpssDxChDiagBistTriggerAllSet
(
    IN  GT_U8                                       devNum
);

/**
* @internal cpssDxChDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagBistResultsGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_DIAG_BIST_STATUS_ENT              *resultsStatusPtr,
    OUT CPSS_DXCH_DIAG_BIST_RESULT_STC              resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
);

/**
* @internal cpssDxChDiagExternalMemoriesBistRun function
* @endinternal
*
* @brief   This function runs BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extMemoBitmap            - bitmap of External DRAM units.
*                                      Bobcat2 devices support 5 External DRAM units.
* @param[in] testWholeMemory          - GT_TRUE - test all memory,
*                                      GT_FALSE - test area specified by
* @param[in] testedAreaOffset         and testedAreaLength only.
* @param[in] testedAreaOffset         - Tested Area Offset in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] testedAreaLength         - Tested Area Length in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] pattern                  - pattern for writing to the memory.
*
* @param[out] testStatusPtr            - (pointer to)GT_TRUE - no errors, GT_FALSE - errors
* @param[out] errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS] - array of Error Info structures for external DRAMs.
*                                      An array should contain 5 structures even not all
*                                      5 memories tested. Counters for bypassed memories
*                                      will contain zeros.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function should be called when External DRAM was already initialized.
*       The cpssDxChTmGlueDramInit is used for External DRAM initialization.
*
*/
GT_STATUS    cpssDxChDiagExternalMemoriesBistRun
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       extMemoBitmap,
    IN  GT_BOOL                                      testWholeMemory,
    IN  GT_U32                                       testedAreaOffset,
    IN  GT_U32                                       testedAreaLength,
    IN  CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern,
    OUT GT_BOOL                                      *testStatusPtr,
    OUT CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS]
);

/**
* @internal cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssDxChPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagh */
