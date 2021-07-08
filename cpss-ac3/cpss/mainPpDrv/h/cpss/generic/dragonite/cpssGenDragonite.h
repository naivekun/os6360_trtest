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
* @file cpssGenDragonite.h
*
* @brief CPSS DRAGONITE APIs.
*
*
* @version   12
********************************************************************************
*/
#ifndef __cpssGenDragoniteh
#define __cpssGenDragoniteh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpssCommon/cpssPresteraDefs.h>


#define CPSS_GEN_DRAGONITE_PORTS_NUM_CNS 96
#define CPSS_GEN_DRAGONITE_CHIPS_NUM_CNS 12
#define CPSS_GEN_DRAGONITE_POWER_BUDGETS_NUM_CNS 16
#define CPSS_GEN_DRAGONITE_BLD_TIME_STRING_LEN_CNS 20 /* in bytes */
/* major version of firmware; if isn't equal to what appears in System structure 
 * error occurs; must be changed appropriately every time new firmware with new 
 * major version arrives 
 */
#define CPSS_GEN_DRAGONITE_MAJOR_VERSION_CNS 1


/**
* @enum CPSS_GEN_DRAGONITE_PRIORITY_ENT
 *
 * @brief This enum defines the powered device/power source priority level list
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PRIORITY_UNKNOWN_E,

    CPSS_GEN_DRAGONITE_PRIORITY_CRITICAL_E,

    CPSS_GEN_DRAGONITE_PRIORITY_HIGH_E,

    CPSS_GEN_DRAGONITE_PRIORITY_LOW_E

} CPSS_GEN_DRAGONITE_PRIORITY_ENT;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_LAYER2_PD_STC
 *
 * @brief Dragonite ports layer 2 powered device data structure
*/
typedef struct{

    /** powered device requested power (LSB */
    GT_U32 pdRequestedPower;

    /** powered device priority; */
    CPSS_GEN_DRAGONITE_PRIORITY_ENT pdPriority;

    /** port cable length LSb = 1meter, reset value 0x64 */
    GT_U32 portCableLen;

} CPSS_GEN_DRAGONITE_PORT_LAYER2_PD_STC;

/**
* @enum CPSS_GEN_DRAGONITE_POWER_SOURCE_TYPE_ENT
 *
 * @brief This enum defines Power source type according to LLDP definitions
*/
typedef enum{

    CPSS_GEN_DRAGONITE_POWER_SOURCE_UNKNOWN_E,

    CPSS_GEN_DRAGONITE_POWER_SOURCE_PRIMARY_E,

    CPSS_GEN_DRAGONITE_POWER_SOURCE_BACKUP_E,

    CPSS_GEN_DRAGONITE_POWER_SOURCE_RESERVED_E

} CPSS_GEN_DRAGONITE_POWER_SOURCE_TYPE_ENT;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_LAYER2_PSE_STC
 *
 * @brief Dragonite ports layer 2 power source data structure
*/
typedef struct{

    /** Port PSE allocated power (LSB */
    GT_U32 pseAllocPower;

    /** port power source priority; */
    CPSS_GEN_DRAGONITE_PRIORITY_ENT psePriority;

    /** port power source type; */
    CPSS_GEN_DRAGONITE_POWER_SOURCE_TYPE_ENT pseType;

} CPSS_GEN_DRAGONITE_PORT_LAYER2_PSE_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_LAYER2_STC
 *
 * @brief Dragonite ports layer 2 data structure
*/
typedef struct{

    /** powered device layer 2 info */
    CPSS_GEN_DRAGONITE_PORT_LAYER2_PD_STC layer2PdInfo;

    /** power source layer 2 info */
    CPSS_GEN_DRAGONITE_PORT_LAYER2_PSE_STC layer2PseInfo;

} CPSS_GEN_DRAGONITE_PORT_LAYER2_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_MATRIX_STC
 *
 * @brief Mapping of application logical ports to Dragonite's
 * physical ports
*/
typedef struct{

    /** @brief Physical port number related to logical port.
     *  The user can define its own logical order of
     *  the ports by matching a logical port number
     *  to a physical port location.
     */
    GT_U8 physicalPortNumber;

    /** The chip number in which the physical port is. */
    GT_U8 chipNumber;

} CPSS_GEN_DRAGONITE_PORT_MATRIX_STC;

/**
* @struct CPSS_GEN_DRAGONITE_CONFIG_STC
 *
 * @brief Dragonite configuration data structure
*/
typedef struct{

    /** select AC or DC disconnect method: GT_FALSE */
    GT_BOOL dcDisconnectEn;

    /** @brief disable external sync operations: GT_FALSE
     *  GT_TRUE - external sync disabled
     */
    GT_BOOL externalSyncDis;

    /** disable Cap detection: GT_FALSE */
    GT_BOOL capDis;

    /** @brief override disable ports line disconnection: 0
     *  1 - disable ports line override
     */
    GT_BOOL disPortsOverride;

    /** disable the RPR protection: 0 */
    GT_BOOL rprDisable;

    /** @brief enable the disconnection of an AT port on Vmain AT low condition: 0
     *  disabled, 1 - Vmain AT Disco enabled
     */
    GT_BOOL vmainAtPolicyEn;

    /** @brief when class 0 is detected then decide AF: 0
     *  1 - class 0 is detected as AF;
     */
    GT_BOOL class0EqAf;

    /** @brief when port is AT then decide class 123 as AF: 0
     *  1 - decide class 123 as AF;
     */
    GT_BOOL class123EqAf;

    /** @brief if the result of the second class event is not as the first then detection fails: 0
     *  different results are invalid, 1 - two different results are valid;
     */
    GT_BOOL classBypass2ndError;

    /** @brief decide class out when class current was above class 4 limit: 0
     *  to ClassErrorEq4 flag settings, 1 - class error decided as 0
     */
    GT_BOOL classErrorEq0;

    /** @brief decide class out when class current was above class 4 limit: 0
     *  1 - class error decided as 4
     */
    GT_BOOL classErrorEq4;

    /** Enables Layer2 power management operation: 0 */
    GT_BOOL layer2En;

    /** @brief Enable port priority settings according to PD LLDP advertisement:
     *  0 - Layer2 Priority Setting Disable,
     *  1 - Layer2 Priority Setting Enable
     */
    GT_BOOL portPrioritySetByPD;

    /** @brief This bit is equal to 0 after reset. The host should set this bit to '1' during config.
     *  By monitoring this bit the host can detect self reset events:
     *  0 - reset occurred.
     *  1 - Bit was set by the Host
     */
    GT_BOOL privateLableBit;

    /** @brief when there is a request for startup power when the edge of the budget is reached,
     *  and there is consumption in a lower priority level, if this bit is set then the
     *  highest priority port which is requesting power will be given additional budget
     *  temporarily to enable startup and then the budget will be returned to the previous
     *  level so that low priority ports will be disconnected:
     *  0 - feature disabled
     *  1 - feature enabled
     */
    GT_BOOL lowPriDiscoForHiPriStartupEn;

    /** Port Power limit calculation method for power allocation; */
    GT_U32 portMethodLimitMode;

    /** port power consumption calculation method for power management; */
    GT_U32 portMethodCalcMode;

    /** @brief Guard band value power for startup will be used up to current budget;
     *  (LSB -0.1W, Range - 0 - 25.5W)
     */
    GT_U32 guardBandValue;

    /** @brief when startup power is not enough for the current port, check
     *  if the following ports can be started up or mark them all as PM:
     *  0 - don't try to start the following ports
     *  1 - check if startup power is enough for lower priority ports
     */
    GT_BOOL startupHiPrOverride;

    /** max Icut value for AT ports: Def */
    GT_U32 IcutMaxAt;

    /** temperature level for alarm: Temp = ((reg_value */
    GT_U32 tempAlarmTh;

    /** max Vmain level; */
    GT_U32 vmainHighTh;

    /** AT min Vmain level; */
    GT_U32 vmainAtLowTh;

    /** @brief AF min Vmain level;
     *  matrixPort[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS] - Mapping of application logical ports to Dragonite's
     *  physical ports
     */
    GT_U32 vmainAfLowTh;

    CPSS_GEN_DRAGONITE_PORT_MATRIX_STC matrixPort[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    /** communication polling time; */
    GT_U32 commPollingTime;

    /** irq generation time; */
    GT_U32 irqGenTime;

} CPSS_GEN_DRAGONITE_CONFIG_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_MEASUREMENTS_STC
 *
 * @brief Dragonite ports measurements data structure
*/
typedef struct{

    /** Real port power consumption; */
    GT_U32 powerConsumption;

    /** port current; */
    GT_U32 current;

    /** port voltage; */
    GT_U32 volt;

    /** Static port power consumption; */
    GT_U32 calcPowerConsumption;

} CPSS_GEN_DRAGONITE_PORT_MEASUREMENTS_STC;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STATUS_ENT
 *
 * @brief This enum defines Implementation specific Status for the port
*/
typedef enum{

    /** PortOn - Port was turned on due to a valid signature (res \ cap) */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_ON_E,

    /** PortOnTM - Port was turned on due to Force Power */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_ON_TM_E,

    /** Startup - Port is in startup */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_E,

    /** StartupTM - Port is in startup as force power */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_TM_E,

    /** Searching - Port needs detection or during detection */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SEARCHING_E,

    /** InvalidSig - Invalid signature has been detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_INVALID_SIGNATURE_E,

    /** ClassError - Error in classification has been detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_CLASS_ERROR_E,

    /** TestMode - Port needs to be turned on as Test Mode - Force Power */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_TEST_MODE_E,

    /** ValidSig - A valid signature has been detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_VALID_SIGNATURE_E,

    /** Disabled - Port is disabled */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_DISABLED_E,

    /** StartupOVL - Overload during startup */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_OVERLOAD_E,

    /** StartupUDL - Underload during startup */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_UNDERLOAD_E,

    /** StartupShort - Short during startup */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_SHORT_E,

    /** DvDtFail - Failure in the Dv/Dt algorithm */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_DVDT_FAIL_E,

    /** TestError - Port was turned on as Test Mode and has error */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_TEST_ERROR_E,

    /** OVL - Overload detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_OVERLOAD_E,

    /** UDL - Underload detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNDERLOAD_E,

    /** ShortCircuit - Short circuit detected */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SHORT_CIRCUIT_E,

    /** PM - port was turned off due to PM */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_POWER_MANAGE_E,

    /** SysDisabled - Chip level error */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SYSTEM_DISABLED_E,

    /** Unknown - General chip error */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNKNOWN_E

} CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STATUS_ENT;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_STATUS_ENT
 *
 * @brief This enum defines possible 802.3_af/at_PortStatus
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_DISABLED_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_SEARCHING_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_DELIVERING_POWER_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_TEST_MODE_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_TEST_ERROR_E,

    /** Implementation Specific */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_IMPLEMENTATION_SPECIFIC_E

} CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_STATUS_ENT;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_ENT
 *
 * @brief This enum defines possible port class
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_0_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_1_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_2_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_3_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_4_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_ERROR_E,

    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_RESERVED_E,

    /** Class Not Defined */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_UNDEFINED_E

} CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_ENT;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STC
 *
 * @brief Dragonite port status data
*/
typedef struct{

    /** Implementation specific Status for the port; */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STATUS_ENT status;

    /** 802.3_af/at_PortStatus; */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_AF_AT_STATUS_ENT extStatus;

    /** Port Class; */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_CLASS_ENT portClass;

    /** @brief after classification this bit indicates if the port is
     *  decided to be AF or AT:
     *  GT_FALSE - port decided to be AF
     *  GT_TRUE - port decided to be AT
     */
    GT_BOOL portAtBehavior;

} CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_INDICATIONS_STC
 *
 * @brief Indications of different port's problems
*/
typedef struct{

    /** Underload Detected */
    GT_BOOL underload;

    /** Overload Detected */
    GT_BOOL overload;

    /** Short Circuit detected */
    GT_BOOL shortCircuit;

    /** Invalid PD Resistor Signature Detected */
    GT_BOOL invalidSignature;

    /** Valid PD Resistor Signature Detected */
    GT_BOOL validSignature;

    /** Power Was Denied */
    GT_BOOL powerDenied;

    /** Valid Capacitor signature Detected */
    GT_BOOL validCapacitor;

    /** Backoff state has occurred */
    GT_BOOL backoff;

    /** Class Error has occurred */
    GT_BOOL classError;

} CPSS_GEN_DRAGONITE_PORT_INDICATIONS_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_STATUSES_STC
 *
 * @brief Dragonite ports status structure
*/
typedef struct{

    /** port status; */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STC portSr;

    /** @brief Reason for ports last disconnection:
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_DISABLED_E      - Disabled - Port is disabled
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_OVERLOAD_E  - StartupOVL - Overload during startup
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_UNDERLOAD_E - StartupUDL - Underload during startup
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_SHORT_E   - StartupShort - Short during startup
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_DVDT_FAIL_E - DvDtFail - Failure in the Dv/Dt algorithm
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_TEST_ERROR_E - TestError - Port was turned on as Test Mode and has error
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_OVERLOAD_E      - OVL - Overload detected
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNDERLOAD_E     - UDL - Underload detected
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SHORT_CIRCUIT_E   - ShortCircuit - Short circuit detected
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_POWER_MANAGE_E    - PM - port was turned off due to PM
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SYSTEM_DISABLED_E  - SysDisabled - Chip level error
     *  CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNKNOWN_E      - Unknown - General chip error
     */
    CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STATUS_ENT lastDisconnect;

    /** Indications of different port's problems */
    CPSS_GEN_DRAGONITE_PORT_INDICATIONS_STC indications;

} CPSS_GEN_DRAGONITE_PORT_STATUSES_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_COUNTERS_STC
 *
 * @brief Dragonite ports counters data structure
*/
typedef struct{

    GT_U8 invalidSignatureCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    GT_U8 powerDeniedCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    GT_U8 overloadCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    GT_U8 shortCyrcuitCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    GT_U8 underloadCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

    GT_U8 classErrorCounter[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];

} CPSS_GEN_DRAGONITE_PORT_COUNTERS_STC;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_ENT
 *
 * @brief This enum defines possible port DRAGONITE control status
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_DISABLED_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_ENABLED_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_FORCE_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_RESERVED_E

} CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_ENT;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_ENT
 *
 * @brief This enum defines port DRAGONITE pair status
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_RESERVED_0_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_ALTER_A_E,

    /** backoff enable */
    CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_ALTER_B_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_RESERVED_3_E

} CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_ENT;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_ENT
 *
 * @brief This enum defines possible port DRAGONITE AT/AF mode
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_AF_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_AT_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_AT4PAIR_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_RESERVED_E

} CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_ENT;

/**
* @enum CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_ENT
 *
 * @brief This enum defines possible port DRAGONITE priority
*/
typedef enum{

    CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_CRITICAL_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_HIGH_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_LOW_E,

    CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_RESERVED_E

} CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_ENT;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_CTRL_STC
 *
 * @brief Dragonite port control command data
*/
typedef struct{

    /** Port Enable Status; */
    CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_ENT pseEnable;

    /** Port Pair Control; */
    CPSS_GEN_DRAGONITE_PORT_CTRL_PAIRCTRL_ENT pairControl;

    /** Port Type Definition; */
    CPSS_GEN_DRAGONITE_PORT_CTRL_MODE_ENT portMode;

    /** Port Priority Level; */
    CPSS_GEN_DRAGONITE_PORT_CTRL_PRIORITY_ENT portPriority;

} CPSS_GEN_DRAGONITE_PORT_CTRL_STC;

/**
* @struct CPSS_GEN_DRAGONITE_PORT_COMMAND_STC
 *
 * @brief Dragonite ports configuration data structure
*/
typedef struct{

    /** port control values; */
    CPSS_GEN_DRAGONITE_PORT_CTRL_STC portCtrl;

    /** initialization port power allocation limit */
    GT_U32 portPpl;

    /** on going port power allocation limit */
    GT_U32 portTppl;

} CPSS_GEN_DRAGONITE_PORT_COMMAND_STC;

/**
* @struct CPSS_GEN_DRAGONITE_CHIP_STC
 *
 * @brief Dragonite chips status data structure
*/
typedef struct{

    /** chip info */
    GT_U32 chipInfo;

    /** measured temperature; */
    GT_U32 measuredTemp;

    /** maximum measured temperature; */
    GT_U32 maxMeasuredTemp;

    /** number of ports connected to this DRAGONITE chip */
    GT_U32 numOfPorts;

    /** Vmain is over the upper threshold */
    GT_BOOL vmainHighError;

    /** The temperature is over the threshold */
    GT_BOOL overTempError;

    /** Disable ports line is active */
    GT_BOOL disablePortsActiveError;

    /** Vmain is under AF low threshold */
    GT_BOOL vmainLowAfError;

    /** Vmain is under AT low threshold */
    GT_BOOL vmainLowAtError;

    /** The temperature is over the alarm threshold */
    GT_BOOL tempAlarm;

    /** chip device Id */
    GT_U32 chipDevId;

} CPSS_GEN_DRAGONITE_CHIP_STC;

/**
* @struct CPSS_GEN_DRAGONITE_POWER_BUDGET_STC
 *
 * @brief Power budgets data
*/
typedef struct{

    /** power budget for state 000 of the power good lines (LSB */
    GT_U32 availablePower;

    /** Power source type according to LLDP definitions */
    CPSS_GEN_DRAGONITE_POWER_SOURCE_TYPE_ENT powerSourceType;

} CPSS_GEN_DRAGONITE_POWER_BUDGET_STC;

/**
* @enum CPSS_GEN_DRAGONITE_CHIP_STATE_ENT
 *
 * @brief This enum defines the type of info in DRAGONITE data structure
*/
typedef enum{

    /** DRAGONITE chip doesn't exists; */
    CPSS_GEN_DRAGONITE_CHIP_STATE_NOT_EXISTS_E,

    /** DRAGONITE chip exists; */
    CPSS_GEN_DRAGONITE_CHIP_STATE_EXISTS_E,

    /** state not applicable; */
    CPSS_GEN_DRAGONITE_CHIP_STATE_NA_E,

    /** DRAGONITE chip error; */
    CPSS_GEN_DRAGONITE_CHIP_STATE_ERROR_E

} CPSS_GEN_DRAGONITE_CHIP_STATE_ENT;

/**
* @struct CPSS_GEN_DRAGONITE_SYSTEM_STC
 *
 * @brief Dragonite system data structure
*/
typedef struct{
    GT_U32     systemMask0; /* reserved*/
    GT_U32     activeBudget;
    GT_U32     vmain;
    CPSS_GEN_DRAGONITE_POWER_BUDGET_STC powerBudgetArr[CPSS_GEN_DRAGONITE_POWER_BUDGETS_NUM_CNS];
    GT_BOOL     vmainHigh;
    GT_BOOL     vmainLowAT;
    GT_BOOL     vmainLowAF;
    GT_BOOL     tempAlarm;
    GT_BOOL     overTemp;
    GT_BOOL     disablePortsActive;
    GT_U32      osStatus; /* TBD in reg spec*/
    CPSS_GEN_DRAGONITE_CHIP_STATE_ENT chipStatusArr[CPSS_GEN_DRAGONITE_CHIPS_NUM_CNS];
    GT_U32      sysTotalCriticalCons;
    GT_U32      sysTotalHighCons;
    GT_U32      sysTotalLowCons;
    GT_U32      sysTotalCriticalReq;
    GT_U32      sysTotalHighReq;
    GT_U32      sysTotalLowReq;
    GT_U32      sysTotalCalcPowerCons;
    GT_U32      sysTotalPowerRequest;
    GT_U32      sysTotalDeltaPower;
    GT_U32      sysTotalRealPowerCons;
    GT_U32      minorVersion;
    GT_U32      majorVersion;
    GT_U8       bldDateTime[CPSS_GEN_DRAGONITE_BLD_TIME_STRING_LEN_CNS];
    GT_U32      checkSumErrorCounter;
    GT_U32      lengthErrorCounter;
    GT_U32      structVersionErrorCounter;
    GT_U32      typeErrorCounter;
    GT_U32      configErrorCounter;
    GT_U32      irqErrorCounter;
} CPSS_GEN_DRAGONITE_SYSTEM_STC;


/**
* @enum CPSS_GEN_DRAGONITE_DATA_TYPE_ENT
 *
 * @brief This enum defines the type of info in DRAGONITE data structure
*/
typedef enum{

    /** DRAGONITE data structure holds MCU configuration data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_CONFIG_E = 0,

    /** DRAGONITE data structure holds MCU system data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_SYSTEM_E,

    /** DRAGONITE data structure holds driver/device data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_CHIP_E,

    /** DRAGONITE data structure holds port commands data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_COMMANDS_E,

    /** DRAGONITE data structure holds port counters data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_CNTRS_E,

    /** DRAGONITE data structure holds port status data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_STATUS_E,

    /** DRAGONITE data structure holds port measurements data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_MEASURE_E,

    /** DRAGONITE data structure holds port layer2 data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_LAYER2_E,

    /** DRAGONITE data structure holds MCU debug data; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_DEBUG_E,

    /** reserved; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_OTHER_E,

    /** for generic implementation purposes; */
    CPSS_GEN_DRAGONITE_DATA_TYPE_MAX_E = (CPSS_GEN_DRAGONITE_DATA_TYPE_OTHER_E)

} CPSS_GEN_DRAGONITE_DATA_TYPE_ENT;

/*
 * typedef: struct CPSS_GEN_DRAGONITE_DATA_STC
 *
 * Description: Dragonite data structure
 *
 * Fields:
 *   dataType        - type of data that structure holds at the moment;
 *   dragoniteData         - union holding data accordingly to dataType;
 */
typedef struct CPSS_GEN_DRAGONITE_DATA_STCT
{
    CPSS_GEN_DRAGONITE_DATA_TYPE_ENT         dataType;

    union {
        CPSS_GEN_DRAGONITE_SYSTEM_STC            system;
        CPSS_GEN_DRAGONITE_CHIP_STC              chip[CPSS_GEN_DRAGONITE_CHIPS_NUM_CNS];
        CPSS_GEN_DRAGONITE_PORT_COMMAND_STC      portCmd[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];
        CPSS_GEN_DRAGONITE_PORT_COUNTERS_STC     portCntrs;
        CPSS_GEN_DRAGONITE_PORT_STATUSES_STC     portStat[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];
        CPSS_GEN_DRAGONITE_PORT_MEASUREMENTS_STC portMeasure[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];
        CPSS_GEN_DRAGONITE_CONFIG_STC            config;
        CPSS_GEN_DRAGONITE_PORT_LAYER2_STC       layer2[CPSS_GEN_DRAGONITE_PORTS_NUM_CNS];
        GT_U32                             *debugDataPtr;
        GT_U32                             *otherDataPtr;
    } dragoniteData; /* union */

} CPSS_GEN_DRAGONITE_DATA_STC;

/**
* @internal cpssDragoniteInit function
* @endinternal
*
* @brief   Initialize DRAGONITE management library:
*         get shared memory base pointer from BSP;
*         get system structure and check version compatibility;
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; ExMx; Puma2; Puma3.
*
* @param[in] intVectNum               - DRAGONITE interrupt vector number
* @param[in] intMask                  - DRAGONITE interrupt mask
*
* @retval GT_OK                    - on success
* @retval GT_INIT_ERROR            - if major version of firmware isn't
*                                       equal to CPSS_GEN_DRAGONITE_MAJOR_VERSION_CNS
* @retval GT_NOT_READY             - DRAGONITE controller not finished yet
*                                       copy system structure to shared memory
* @retval GT_BAD_STATE             - data received, but not of System type
* @retval GT_BAD_PTR               - if wrong shared mem base addr
*
* @note This call may return NOT_READY if DRAGONITE manager still not initialized
*       application must set some timeout for this case and try again,
*       it can't do nothing DRAGONITE related till this call not succeeded.
*
*/
GT_STATUS cpssDragoniteInit
(
    IN GT_U32 intVectNum,
    IN GT_U32 intMask
);

/**
* @internal cpssDragoniteReadTrigger function
* @endinternal
*
* @brief   Trigger DRAGONITE controller to put in shared memory data of requested type
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; ExMx; Puma2; Puma3.
*
* @param[in] type                     -  of data structure application wants to read
*                                      from DRAGONITE MCU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown type
* @retval GT_NOT_READY             - DRAGONITE controller not finished yet
*                                       previous operation
* @retval GT_NOT_INITIALIZED       - DragoniteInit not executed
*/
GT_STATUS cpssDragoniteReadTrigger
(
    IN CPSS_GEN_DRAGONITE_DATA_TYPE_ENT type
);

/**
* @internal cpssDragoniteReadData function
* @endinternal
*
* @brief   Read DRAGONITE data structure
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; ExMx; Puma2; Puma3.
*
*
* @param[out] dataPtr                  - pointer to structure, that is actually union,
*                                      where to put data read from shared memory.
*                                      Pay attention dataPtr is just memory pointer - any setting in it
*                                      will be ignorred - this function just returns to application
*                                      what it has read from shared memory, it doesn't check if it's
*                                      what application asked in ReadTrigger or Write called before
*                                       --------------------
*                                       Return codes generated by CPSS API itself:
*                                       --------------------
*
* @retval GT_OK                    - on success
* @retval GT_NOT_READY             - DRAGONITE controller not finished yet
*                                       copy data to shared memory
* @retval GT_BAD_PTR               - Illegal pointer value
*                                       ------------------------
*                                       Following errors returned by DRAGONITE controller communication
*                                       protocol:
*                                       ------------------------
* @retval GT_CHECKSUM_ERROR        - checksum error.
* @retval GT_BAD_SIZE              - Structure length error.
* @retval GT_NOT_SUPPORTED         - Structure version mismatch.
* @retval GT_BAD_PARAM             - Unknown Type. When this bit is set,
*                                       there will be no data structure returned, only the
*                                       header.
* @retval GT_OUT_OF_RANGE          - Out of range value in the structure
*                                       data. The DRAGONITE manager will fill the data in the structure
*                                       with zeros as long as the received data was in range. In
*                                       case of data out of range, the received out of range
*                                       data will be returned.
* @retval GT_BAD_STATE             - Config is not allowed - Application can
*                                       send config structure to DRAGONITE controller only once after
*                                       reset.
* @retval GT_ERROR                 - could happen only if there is something in protocol
*                                       that cpss is not aware of
* @retval GT_NOT_INITIALIZED       - DragoniteInit not executed
*                                       COMMENT:
* @retval Just one error code returned at once - error priority list:
* @retval GT_CHECKSUM_ERROR        - highest priority
*                                       GT_BAD_PARAM
*                                       GT_NOT_SUPPORTED
*                                       GT_BAD_SIZE
*                                       GT_BAD_STATE (specific for Config structure)
*                                       GT_OUT_OF_RANGE (fields with wrong size returned in
*                                       dataPtr)
*/
GT_STATUS cpssDragoniteReadData
(
    OUT CPSS_GEN_DRAGONITE_DATA_STC *dataPtr
);

/**
* @internal cpssDragoniteWrite function
* @endinternal
*
* @brief   Write DRAGONITE data structure of requested type
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; ExMx; Puma2; Puma3.
*
* @param[in] dataPtr                  - pointer to structure that is actually union,
*                                      contain data to send to DRAGONITE controller
*
* @retval GT_OK                    - on success
* @retval GT_NOT_READY             - if shared memory isn't free yet
* @retval GT_BAD_VALUE             - if one of fields exceeds number of bits for field
* @retval GT_BAD_PARAM             - wrong structure type
* @retval GT_NOT_SUPPORTED         - on try to write to Read Only structure type
* @retval GT_NOT_INITIALIZED       - DragoniteInit not executed
*                                       COMMENT:
*                                       after cpssDxChDragoniteWrite application should
*                                       call cpssDxChDragoniteReadData to ensure there was no
* @retval communication error      - it could be done by polling after
*                                       delay or upon write_done interrupt.
*/
GT_STATUS cpssDragoniteWrite
(
    IN CPSS_GEN_DRAGONITE_DATA_STC *dataPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenDragoniteh */


