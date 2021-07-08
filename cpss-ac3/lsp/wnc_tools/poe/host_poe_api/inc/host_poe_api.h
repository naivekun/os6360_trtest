/*************************************************************************
*
* Copyright (c) Microchip Technology Inc. All rights reserved.
* Licensed under the MIT License. See LICENSE in the project root for license information.
*
*************************************************************************
*
*  File Revision: 1.0
*
*************************************************************************
*
*  Description: contain the structs for comm protocol.
*
*************************************************************************/

#ifndef _POE_API_INC_
#define _POE_API_INC_

#include <stddef.h>

#include "host_poe_comm_global.h"


#define HOST_POE_API_VER 0x0108
#define DATE "15/04/2019"
#define TIME "17:40"


/***********************Enums***************************************/

typedef enum{
PoE_RC_SUCSESS,
PoE_RC_ERR_RESET_DETECT,
PoE_RC_ERR_COMM_DEVICE,
PoE_RC_ERR_COMM_TIMEOUT,
PoE_RC_ERR_WRONG_CS,
PoE_RC_ERR_SUBJECT_BYTES,
PoE_RC_ERR_DATA_BYTES,
PoE_RC_ERR_KEY_VALUE,
PoE_RC_ERR_OUT_OF_RANGE,
PoE_RC_ERR_ECHO_BYTE,
PoE_RC_ERR_MSG_TYPE,
PoE_RC_ERR_MUTEX_INIT,
PoE_RC_ERR_MUTEX_LOCK_UNLOCK,
PoE_RC_ERR_MUTEX_DESTROY,
PoE_RC_ERR_SLEEP_FUNC,
PoE_RC_ERR_INIT,
PoE_RC_ERR_DRIVER,
PoE_RC_ERR_BASE_MSG_MISMATCH,
PoE_RC_ERR_FW_ERR,
PoE_RC_ERR_COMM_DEVICE_NAME
}PoE_RC_e;


typedef struct _POE_BASE_MESSAGE {
	UINT16 code; /*POE message code*/
	UINT8 length;/*message variable length*/
	char data[1];/*place holder of additional data*/
} POE_BASE_MESSAGE, *P_POE_BASE_MESSAGE;

/* POE protocol codes*/


/**************************************/
/*              SYSTEM                 */
/**************************************/

#define POE_MESSAGE_SIZE 15

#define BASE_MESSAGE_SIZE  (offsetof(POE_BASE_MESSAGE, data))

#define INIT_BASE_MSG(_pm, _code,  _no_hdr_length) {\
	(_pm)->code = (unsigned short)(_code);\
	(_pm)->length = (unsigned short)(_no_hdr_length)+BASE_MESSAGE_SIZE;\
}
	



typedef struct _POE_DEBUG_LEVEL_SET_MSG {
	UINT32 state;
} POE_DEBUG_LEVEL_SET_MSG, *P_POE_DEBUG_LEVEL_SET_MSG;



#define  API_INFRASTRCTURE_COMMAND_BASE    0
#define  API_SYSTEM_COMMAND_BASE           10
#define  API_DEVICE_COMMAND_BASE           40
#define  API_PORT_COMMAND_BASE             50
#define  API_POWER_MNGT_COMMAND_BASE       80
#define  API_POWER_DER_COMMAND_BASE        90
#define  API_SOFTWARE_DOWNLOAD     	   	   100
#define  API_ERR_COMMAND_BASE              150



#define POEAPIReqCodeToRepCode(POEReqCode) ((POEReqCode) +1)





/***************************************/
/*              Enum                  */
/****************************************/

#define MAX_DEVICE_NAME_SIZE  16


typedef enum _PoE_Comm_Dev_e {
	POE_COMM_UART_ASYNC,
	POE_COMM_UART_SYNC,
	POE_COMM_I2C,
	POE_COMM_SHARED_MEM,
	POE_COMM_DEV_MAX,
	POE_COMM_SERVER,
	POE_COMM_QUEUE
} PoE_Comm_Dev_e;



typedef struct _POE_SET_I2C_PARAMS {
           UINT8   I2CAddress;
		   UINT16  I2CBitRate;
		   
}POE_SET_I2C_PARAMS, *P_POE_SET_I2C_PARAMS;

typedef struct _POE_HOST_COMM_DEVICE{ 

char deviceName[MAX_DEVICE_NAME_SIZE];
PoE_Comm_Dev_e devType;
UINT16 devDelay;
void *params;

}POE_HOST_COMM_DEVICE,*P_POE_HOST_COMM_DEVICE;



/**************************************/
/*              API Functions          */
/**************************************/


PoE_RC_e POEAPITranslateMessage(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POEAPIOpenSession(IN const char *devName, PoE_Comm_Dev_e devType, UINT16 devCommDelay,UINT8 *params,IN unsigned int apiVersion);
PoE_RC_e POEAPICloseSession(void);



/**********************************************/
/*              Enums of commands/requests	  */
/**********************************************/

typedef enum _POE_CPU_STATUS2 {
	POE_CPU_STATUS2_NO_ERR= 0,
	POE_CPU_STATUS2_SPI_ERR = 2
} POE_CPU_STATUS2;


typedef enum _POE_FAC_DEFAULT {
	POE_FAC_DEFAULT_UNSET= 0,
	POE_FAC_DEFAULT_SET= 1
} POE_FAC_DEFAULT;


typedef enum _POE_INT_REGISTER_EVENT {
	POE_EVENT_PORT_ON =   0x0001,
	POE_EVENT_PORT_OFF =  0x0002,	
	POE_EVENT_DET_UNSUCCESS = 0x0004,
	POE_EVENT_PORT_FAIL = 0x0008,
	POE_EVENT_PORT_UNDERLOAD = 0x0010,
	POE_EVENT_PORT_OVERLOAD = 0x0020,
	POE_EVENT_PORT_PM = 0x0040,
	POE_EVENT_PORT_PWRUP_DEN =0x0080,
	POE_EVENT_DISCONNECT_HIGH_TEMP =0x0100,
	POE_EVENT_EXCEED_USER_DEF_TEMP =0x0200,
	POE_EVENT_DEVICE_FAIL =0x0400,
	POE_EVENT_VMAIN_FAULT=0x2000
}POE_INT_REGISTER_EVENT;

typedef enum _POE_SYSTEM_EVENTS_MASK_REGISTER {
	POE_SYSTEM_EVENT_RESET_REGISTER = 0x0001,
	POE_SYSTEM_EVENT_VMAIN_FAULT = 0x0002,
}POE_SYSTEM_EVENTS_MASK_REGISTER;

typedef enum _POE_DEVICE_EVENTS_MASK_REGISTER {
	POE_DEVICE_EVENT_DISCONNECTION_TEMP = 0x0001,
	POE_DEVICE_EVENT_USER_DEFINED_TEMP = 0x0002,
	POE_DEVICE_EVENT_DEVICE_FAULT = 0x0004,
	POE_DEVICE_EVENT_DEVICE_REFRESHED = 0x0008,
}POE_DEVICE_EVENTS_MASK_REGISTER;

typedef enum _POE_PORT_EVENTS_MASK_REGISTER {
	POE_PORT_EVENT_TURNED_ON = 0x0001,
	POE_PORT_EVENT_TURNED_OFF_USER = 0x0002,
	POE_PORT_EVENT_COUNTERS_RELATED = 0x0004,
	POE_PORT_EVENT_OPEN = 0x0008,
	POE_PORT_EVENT_FAULT = 0x0010
}POE_PORT_EVENTS_MASK_REGISTER;

typedef enum _POE_SYSTEM_BT_MASK_REGISTERS_LIST {
	POE_SYSTEM_BT_MASK_IGNORE_HIGHER_PRIORITY = 0x00,
	POE_SYSTEM_BT_MASK_RECOVERY_SUPPORT = 0x09,
	POE_SYSTEM_BT_MASK_SUPPORT_HIGH_RES_DETECTION = 0x10,
	POE_SYSTEM_BT_MASK_HARDWARE_RESET_ON_ASIC_ERROR = 0x14,
	POE_SYSTEM_BT_MASK_LED_STREAM_NEGATIVE = 0x19,
	POE_SYSTEM_BT_MASK_I2C_RESTART_ENABLE = 0x1B,
	POE_SYSTEM_BT_MASK_PSE_POWERING_OR_CHECKING = 0x1F,
	POE_SYSTEM_BT_MASK_LED_STREAM_TYPE = 0x20,
	POE_SYSTEM_BT_MASK_ENABLE_ASIC_REFRESH = 0x2A,
	POE_SYSTEM_BT_MASK_TEMPERATURE_DERATING_ENABLE_MISDSPAN_BT = 0x32,
	POE_SYSTEM_BT_MASK_TEMPERATURE_DERATING_NEGATIVE_OR_POSITIVE_DELTA = 0x33,
	POE_SYSTEM_BT_MASK_X_SYSTEX_OK_PIN_BEHAVIOR = 0x40,
	POE_SYSTEM_BT_MASK_SINGLE_DETECTION_FAIL_EVENT = 0x46,
	POE_SYSTEM_BT_MASK_AUTO_ZONE2_PORT_ACTIVATION = 0x49,
	POE_SYSTEM_BT_MASK_HOCPP = 0x50
}POE_SYSTEM_BT_MASK_REGISTERS_LIST;

typedef enum _POE_ERROR_CODE {
	POE_ERROR_CODE_NEED_DOWNLOAD =0x4E,
	POE_ERROR_CODE_HW_ERR=0x02,
	POE_ERROR_CODE_SYS_TYPE=0x03
}POE_ERROR_CODE;


typedef enum _POE_SYSTEM_EVENT {
	POE_SYSTEM_EVENT_NOT_OCCURRED = 0,
	POE_SYSTEM_EVENT_RESET_OR_RESTORE
}POE_SYSTEM_EVENT;

typedef enum _POE_SYSTEM_EVENT_EXIST {
	POE_SYSTEM_EVENT_NOT_EXIST = 0,
	POE_SYSTEM_EVENT_IS_EXIST = 1
}POE_SYSTEM_EVENT_EXIST;


typedef enum _POE_SYSOK_REG_VMAIN {
	POE_SYSOK_REG_VMAIN_OUT_RANGE = 0,
	POE_SYSOK_REG_VMAIN_IN_RANGE 
}POE_SYSTEM_OK_REG_VMAIN;

typedef enum _POE_SYSOK_VMAIN_FAULT {
	POE_SYSOK_VMAIN_FAULT_IN_RANGE,
	POE_SYSOK_VMAIN_FAULT_OUT_RANGE
}POE_SYSOK_VMAIN_FAULT;

typedef enum _POE_SYS_OK_REG_PWR_PREC {
	POE_SYSOK_REG_PWR_BELLOW_IND_OFF_RREC =0,
	POE_SYSOK_REG_PWR_ABOVE_IND_ON_RREC 
}POE_SYS_OK_REG_PWR_PREC;


typedef enum _POE_SYS_OK_REG_PWR_VAL {
	POE_SYSOK_REG_PWR_BELLOW_IND_OFF_VAL =0,
	POE_SYSOK_REG_PWR_ABOVE_IND_ON_VAL 
}POE_SYS_OK_REG_PWR_VAL;


typedef enum _POE_DEVICE_EVENT {
	POE_DEVICE_EVENT_0  = 0x0001,
	POE_DEVICE_EVENT_1  = 0x0002,
	POE_DEVICE_EVENT_2  = 0x0004,
	POE_DEVICE_EVENT_3  = 0x0008,
	POE_DEVICE_EVENT_4  = 0x0010,
	POE_DEVICE_EVENT_5  = 0x0020,
	POE_DEVICE_EVENT_6  = 0x0040,
	POE_DEVICE_EVENT_7  = 0x0080,
	POE_DEVICE_EVENT_8  = 0x0100,
	POE_DEVICE_EVENT_9  = 0x0200,
	POE_DEVICE_EVENT_10  = 0x0400,
	POE_DEVICE_EVENT_11  = 0x0800

}POE_DEVICE_EVENT;


typedef enum _POE_SRS_LVD {
	POE_SRS_LVD_RESET_NOT_CAUSE=0,
	POE_SRS_LVD_RESET_CAUSE 
}POE_SRS_LVD;


typedef enum _POE_SRS_ILOP {
	POE_SRS_ILOP_RESET_NOT_CAUSE=0,
	POE_SRS_ILOP_RESET_CAUSE 
}POE_SRS_ILOP;


typedef enum _POE_SRS_COP {
	POE_SRS_COP_RESET_NOT_CAUSE=0,
	POE_SRS_COP_RESET_CAUSE 
}POE_SRS_COP;


typedef enum _POE_SRS_PIN {
	POE_SRS_PIN_RESET_NOT_CAUSE=0,
	POE_SRS_PIN_RESET_CAUSE 
}POE_SRS_PIN;

typedef enum _POE_SRS_POR {
	POE_SRS_POR_RESET_NOT_CAUSE=0,
	POE_SRS_POR_RESET_CAUSE 
}POE_SRS_POR;

typedef enum _POE_GIE1 {
	POE_GIE1_CLK_LOSS_IRQ  = 0x01,
	POE_GIE1_UART_ON_I2C_IRQ  = 0x02,
	POE_GIE1_UNAUTHO_IRQ   = 0x04,
	POE_GIE1_I2C_ARB_LOSS  = 0x08,
	POE_GIE1_EXT_CLK_REC_FAIL  = 0x10,
	POE_GIE1_VMAIN_OUT_RANGE  = 0x20,
	POE_GIE1_CPU_VOL_WARNNING = 0x40,
	POE_GIE1_UART_ERROR_IRQ  = 0x80

}POE_GIE1;



typedef enum _POE_RESET_INFO {
	POE_RESET_INFO_COMM_RESET_COMMAND  = 0x01,
	POE_RESET_INFO_CLK_RECOVERY_FAIL  = 0x02,
	POE_RESET_INFO_DEVICE_FAIL   = 0x04,
	POE_RESET_INFO_I2C_RESTART  = 0x08,
	POE_RESET_INFO_SELF_RESET  = 0x10
	
}POE_RESET_INFO;


typedef enum _POE_DB_RECOVERY {
	POE_DB_RECOVERY_FIELD_CLEAR  = 0,
	POE_DB_RECOVERY_NO_REC_ACT,
	POE_DB_RECOVERY_SYS_REC_VALID_STRUCT,
	POE_DB_RECOVERY_REC_STRUCT_CRC_ERR,
	POE_DB_RECOVERY_REC_STRUCT_VER_ERR,
	POE_DB_RECOVERY_RSEREVED,
	POE_DB_RECOVERY_RESET_DUE_TO_CRASH_EVT,
	POE_DB_RECOVERY_MODE_WAIT_RESET,
	POE_DB_RECOVERY_STRUCT_CORRECT_TIMESTAP,
	POE_DB_RECOVERY_DIFFERENT_ICS_BEFORE_CRASH = 10,
	POE_DB_RECOVERY_MASK_DISABLED,
	POE_DB_RECOVERY_CRC_REVERESED,
	POE_DB_RECOVERY_MISMATCH_AGE_STAMP
	
}POE_DB_RECOVERY;


typedef enum _POE_CLASS_TYPE {
	POE_CLASS_TYPE_1 = 1,
	POE_CLASS_TYPE_2,
	POE_CLASS_TYPE_3,
	POE_CLASS_TYPE_4,
	POE_CLASS_TYPE_5,
	POE_CLASS_TYPE_6,
	POE_CLASS_TYPE_7,
	POE_CLASS_TYPE_8,
	POE_CLASS_TYPE_MAX
}POE_CLASS_TYPE;


typedef enum _POE_SW_VER_PARAMS_CODE {
	POE_SW_VER_PARAMS_RES_LEGACY,
	POE_SW_VER_PARAMS_OLD_PRD1,
	POE_SW_VER_PARAMS_OLD_PRD2,
	POE_SW_VER_PARAMS_RES_NO_LEGACY,
	POE_SW_VER_PARAMS_OLD_PRD3,
	POE_SW_VER_PARAMS_CUST
}POE_SW_VER_PARAMS_CODE;



typedef enum _POE_DEVICE_STATUS {
	POE_DEVICE_NO_FOUND,
	POE_DEVICE_ZONE1_FOUND_OK,
	POE_DEVICE_REFRESH,
	POE_DEVICE_LOST = 4,
	POE_DEVICE_ZONE2_ERR_1,
	POE_DEVICE_ZONE2_ERR_2,
	POE_DEVICE_ZONE2_ERR_3,
	POE_DEVICE_VMAIN_ERR,
	POE_DEVICE_VMAIN_LESS_SYS_AVG,
	POE_DEVICE_ZONE3_FOUND_OK
}POE_DEVICE_STATUS;

typedef enum _POE_DEVICE_EVENT_IND {
	POE_DEVICE_EVENT_SAFE_TEMPERTURE =1,
	POE_DEVICE_EVENT_USER_DEF_TEMPERTURE,
	POE_DEVICE_EVENT_FAULT = 4
}POE_DEVICE_EVENT_IND;


typedef enum _POE_DEVICE_FOUND {
	POE_DEVICE_FOUND_INVALID, 
	POE_DEVICE_FOUND_4_PORT = 4,
	POE_DEVICE_FOUND_8_PORT = 8,
	POE_DEVICE_FOUND_M = 16
}POE_DEVICE_FOUND;

typedef enum _POE_DEVICE_SEVERITY_ERROR {
	POE_DEVICE_SEVERITY_NOT_LOGGED,
	POE_DEVICE_SEVERITY_LOGGED,
	POE_DEVICE_SEVERITY_LOGGED_DATA_NOT_VALID = 4
}POE_DEVICE_SEVERITY_ERROR;

typedef enum _POE_DEVICE_STRUCT_ERROR {
	POE_DEVICE_STRUCTURE_OK,
	POE_DEVICE_STRUCTURE_SIGNATURE_ERROR,
	POE_DEVICE_STRUCTURE_ERASE_COUNTER_AT_LIMIT,
	POE_DEVICE_STRUCTURE_DATA_ERROR
}POE_DEVICE_STRUCT_ERROR;

typedef enum _POE_DEVICE_SECTOR_ERROR {
	POE_DEVICE_SECTOR_OK,
	POE_DEVICE_SECTOR_SIGNATURE_ERROR,
	POE_DEVICE_SECTOR_ERASE_COUNTER_AT_LIMIT,
	POE_DEVICE_SECTOR_DATA_ERROR
}POE_DEVICE_SECTOR_ERROR;

typedef enum _POE_PORT_MODE {
	POE_PORT_MODE_DISABLE,
	POE_PORT_MODE_ENABLE,
	POE_PORT_MODE_FORCE_POWER = 3,
	POE_PORT_MODE_DO_NOT_CHANGE_SETTINGS = 0xF
}POE_PORT_MODE;

typedef enum _POE_PORT_STATUS {
	POE_PORT_STATUS_OFF_MAIN_VOLTAGE_HIGH = 0x06,
	POE_PORT_STATUS_OFF_MAIN_VOLTAGE_LOW = 0x07,
	POE_PORT_STATUS_OFF_DISABLE_ALL_PORTS = 0x08,
	POE_PORT_STATUS_OFF_NON_EXISTING_PORT_NUMBER = 0x0C,
	POE_PORT_STATUS_IS_UNDEFINED = 0x11,
	POE_PORT_STATUS_OFF_INTERNAL_HARDWARE_FAULT = 0x12,
	POE_PORT_STATUS_OFF_USER_SETTING = 0x1A,
	POE_PORT_STATUS_OFF_DETECTION_IN_PROCESS = 0x1B,
	POE_PORT_STATUS_OFF_NON_8023AF_AT_POWERED_DEVICE = 0x1C,
	POE_PORT_STATUS_OFF_UNDERLOAD_STATE = 0x1E,
	POE_PORT_STATUS_OFF_OVERLOAD_STATE = 0x1F,
	POE_PORT_STATUS_OFF_POWER_BUDGES_EXCEEDED= 0x20,
	POE_PORT_STATUS_OFF_CONFIGURATION_CHANGE = 0x22,
	POE_PORT_STATUS_OFF_VOLTAGE_INJECTION_INTO_PORT = 0x24,
	POE_PORT_STATUS_OFF_IMPROPER_CAPACITOR_DETECTION_RESULTS_OR_VALUES_INDICATING_SHORT = 0x25,
	POE_PORT_STATUS_OFF_DISCHARGED_LOAD = 0x26,
	POE_PORT_STATUS_OFF_SHORT_CONDITION = 0x34,
	POE_PORT_STATUS_OFF_OVER_TEMPERATURE = 0x35,
	POE_PORT_STATUS_OFF_DEVICE_TOO_HOT = 0x36,
	POE_PORT_STATUS_OFF_UNKNOWN_DEVICE = 0x37,
	POE_PORT_STATUS_OFF_POWER_MANAGEMENT_STATIC = 0x3C,
	POE_PORT_STATUS_OFF_POWER_MANAGEMENT_STATIC_OVL = 0x3D,
	POE_PORT_STATUS_OFF_POWER_DENIED = 0x41,
	POE_PORT_STATUS_OFF_CLASS_ERROR = 0x43,
	POE_PORT_STATUS_OFF_PORT_TURN_OFF_DURING_HOST_CRASH = 0x44,
	POE_PORT_STATUS_OFF_DELIVERED_POWER_PORT_WAS_FORCED_SHUT_DOWN = 0x45,
	POE_PORT_STATUS_OFF_ENABLED_PORT_WAS_FORCED_SHUT_DOWN = 0x46,
	POE_PORT_STATUS_OFF_FORCE_POWER_CRASH_ERROR = 0x47,
	POE_PORT_STATUS_OFF_RECOVERY_UDL = 0x48,
	POE_PORT_STATUS_OFF_RECOVERY_PG_EVENT = 0x49,
	POE_PORT_STATUS_OFF_RECOVERY_OVL = 0x4A,
	POE_PORT_STATUS_OFF_RECOVERY_SC = 0x4B,
	POE_PORT_STATUS_OFF_RECOVERY_VOLTAGE_INJECTION = 0x4C,
	POE_PORT_STATUS_2P_NON_IEEE = 0x80,
	POE_PORT_STATUS_2P_IEEE = 0x81,
	POE_PORT_STATUS_4P_PORT_DELIVER_2PAIR_NON_IEEE = 0x82,
	POE_PORT_STATUS_4P_PORT_DELIVER_2P_NON_IEEE = 0x83,
	POE_PORT_STATUS_4P_PORT_DELIVER_4P_NON_IEEE = 0x84,
	POE_PORT_STATUS_4P_PORT_DELIVER_2P_IEEE_SSPD = 0x85,
	POE_PORT_STATUS_4P_PORT_DELIVER_4P_IEEE_SSPD = 0x86,
	POE_PORT_STATUS_4P_PORT_DELIVER_4P_IEEE_DSPD_IN_1ST = 0x87,
	POE_PORT_STATUS_4P_PORT_DELIVER_2P_IEEE_DSPD= 0x88,
	POE_PORT_STATUS_4P_PORT_DELIVER_4P_IEEE_DSPD = 0x89,
	POE_PORT_STATUS_FORCE_POWER_BT_2P = 0x90,
	POE_PORT_STATUS_FORCE_POWER_BT_4P = 0x91,
	POE_PORT_STATUS_FORCE_POWER_BT_ERROR = 0xA0,
	POE_PORT_STATUS_CONNECTION_CHECK_ERROR = 0xA7,
	POE_PORT_STATUS_OPEN = 0xA8
}POE_PORT_STATUS;

typedef enum _POE_PORT_PM_MODE {
	POE_PORT_PM_MODE_DYNAMIC,
	POE_PORT_PM_MODE_TPPL_BT,
	POE_PORT_PM_MODE_DYNAMIC_FOR_NON_LLDP_TPPL_BT_FOR_LLDP,
	POE_PORT_PM_MODE_DO_NOT_CHANGE_SETTINGS = 0xF
}POE_PORT_PM_MODE;

typedef enum _POE_CLASS_ERROR_OPER_SELECT {
	POE_CLASS_ERROR_DISABLED = 0x00,
	POE_CLASS_ERROR_SSPD_3_DSPD_3 = 0x01,
	POE_CLASS_ERROR_SSPD_4_DSPD_3 = 0x02,
	POE_CLASS_ERROR_SSPD_6_DSPD_4 = 0x03,
	POE_CLASS_ERROR_SSPD_8_DSPD_5 = 0x04,
	POE_CLASS_ERROR_DO_NOT_CHANGE = 0xF
}POE_CLASS_ERROR_OPER_SELECT;

typedef enum _POE_PORT_OPER_MODE {
	POE_PORT_PM_MODE_4P_90_2P_30_NO_LEGACY  = 0x00,
	POE_PORT_PM_MODE_4P_60_2P_30_NO_LEGACY  = 0x01,
	POE_PORT_PM_MODE_4P_30_2P_30_NO_LEGACY  = 0x02,
	POE_PORT_PM_MODE_4P_15_2P_15_NO_LEGACY  = 0x03,

	POE_PORT_PM_MODE_4P_90_2P_30_WITH_LEGACY  = 0x10,
	POE_PORT_PM_MODE_4P_60_2P_30_WITH_LEGACY  = 0x11,
	POE_PORT_PM_MODE_4P_30_2P_30_WITH_LEGACY  = 0x12,
	POE_PORT_PM_MODE_4P_15_2P_15_WITH_LEGACY  = 0x13,
	POE_PORT_PM_MODE_4P_90_2P_30_WITH_LEGACY_CLASS0_CLASS4 = 0x14,
	POE_PORT_PM_MODE_4P_60_2P_30_WITH_LEGACY_CLASS0_CLASS4 = 0x15,

	POE_PORT_PM_MODE_4P_POH_2P_POH       = 0x20,
	POE_PORT_PM_MODE_4P_60_MSCC_2P_30    = 0x21,
	POE_PORT_PM_MODE_4P_60_CDP_2P_30_CDP = 0x22,
	POE_PORT_PM_MODE_4P_60_2P_30_PRE_BT  = 0x23,
	POE_PORT_PM_MODE_4P_60_4FINGER_2P_30_3FINGER = 0x24,
	POE_PORT_PM_MODE_4P_90_2P_30_UP_TO_CLASS_5_WITH_LEGACY  = 0x30,
	POE_PORT_PM_MODE_NO_CHANGE           = 0xFF
}POE_PORT_OPER_MODE;

typedef enum _POE_PORT_PHY_INFO_CC_RESULT {
	POE_PORT_CC_RESULT_NOT_PERFORMED,
	POE_PORT_CC_RESULT_OPEN,
	POE_PORT_CC_RESULT_OTHER,
	POE_PORT_CC_RESULT_SINGLE_SIGNATURE,
	POE_PORT_CC_RESULT_DUAL_SIGNATURE,
	POE_PORT_CC_RESULT_UNEXPECTED,
}POE_PORT_PHY_INFO_CC_RESULT;

typedef enum _POE_PORT_PHY_INFO_DECIDED_PD_STRUCTURE {
	POE_PORT_DECIDED_PD_STRUCTURE_NOT_PERFORMED,
	POE_PORT_DECIDED_PD_STRUCTURE_OPEN,
	POE_PORT_DECIDED_PD_STRUCTURE_INVALID_SGINATURE,
	POE_PORT_DECIDED_PD_STRUCTURE_4P_SSPD_IEEE,
	POE_PORT_DECIDED_PD_STRUCTURE_4P_SSPD_LEGACY,
	POE_PORT_DECIDED_PD_STRUCTURE_4P_DSPD_IEEE,
	POE_PORT_DECIDED_PD_STRUCTURE_2P_DSPS_4P_CANDIDATE_FALSE,
	POE_PORT_DECIDED_PD_STRUCTURE_2P_IEEE,
	POE_PORT_DECIDED_PD_STRUCTURE_2P_LEGACY
}POE_PORT_PHY_INFO_DECIDED_PD_STRUCTURE;

typedef enum _POE_PORT_AUTO_CLASS_SUPPORT {
	POE_PORT_AUTO_CLASS_SUPPORT_NOT_PERFORMED,
	POE_PORT_AUTO_CLASS_NOT_SUPPORT,
	POE_PORT_AUTO_CLASS_SUPPORT_AUTO_CLASS
}POE_PORT_AUTO_CLASS_SUPPORT;

typedef enum _POE_PORT_PRIORITY {
	POE_PORT_PRIORITY_CRITICAL = 1,
	POE_PORT_PRIORITY_HIGH,
	POE_PORT_PRIORITY_LOW,
	POE_PORT_PRIORITY_DO_NOT_CHANGE_SETTINGS = 0xFF
}POE_PORT_PRIORITY;

typedef enum _POE_PORT_OTHER {
	POE_PORT_OTHER_T4_DEVICE = 0,
	POE_PORT_OTHER_M_DEVICE
}POE_PORT_OTHER;

typedef enum _POE_PORT_AUTO_CLASS_OPERATION {
	POE_PORT_NEGOTIATION_BY_REQUESTED_PWR_FIELD,
	POE_PORT_NEGOTIATION_BY_ATO_CLASS_REQUESTED
}POE_PORT_AUTO_CLASS_OPERATION;

typedef enum _POE_PORT_LAYER2_EXECUTION {
	POE_PORT_LAYER2_IDLE = 0,
	POE_PORT_LAYER2_REQUEST_PENDING,
	POE_PORT_LAYER2_REQUEST_WAS_EXECUTED,
	POE_PORT_LAYER2_LAST_REQUEST_WAS_REJECTED_LACK_POWER,
	POE_PORT_LAYER2_LAST_REQUEST_WAS_REJECTED_ERROR,
	POE_PORT_LAYER2_LAST_REQUEST_WAS_REJECTED_MISMATCH,
	POE_PORT_LAYER2_AUTO_CLASS_REQUEST_PENDING,
	POE_PORT_LAYER2_AUTO_CLASS_MEASUREMENT_COMPLETED,
	POE_PORT_LAYER2_AUTO_CLASS_ERROR
}POE_PORT_LAYER2_EXECUTION;

typedef enum _POE_PORT_LAYER2_USAGE {
	POE_PORT_LAYER2_PORT_NOT_DELIVER_POWER = 0,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_USING_LAYER1,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_USING_LAYER1_AUTO_CALSS,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_USING_LLDP,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_USING_LLDP_AUTO_CLASS,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_WITH_CDP_2P,
	POE_PORT_LAYER2_PORT_DELIVER_POWER_WITH_CDP_4P
}POE_PORT_LAYER2_USAGE;

typedef enum _POE_PORT_IEEE_BT_POWER_BITS_15_14 {
	POE_PORT_IEEE_BT_POWER_BITS_15_14_PORT_NOT_OPERTIONAL = 0,
	POE_PORT_IEEE_BT_POWER_BITS_15_14_2PAIR,
	POE_PORT_IEEE_BT_POWER_BITS_15_14_4PAIR_SINGLE_SIGNATURE,
	POE_PORT_IEEE_BT_POWER_BITS_15_14_4PAIR_DUAL_SIGNATURE
}POE_PORT_IEEE_BT_POWER_BITS_15_14;

typedef enum _POE_PORT_IEEE_BT_POWER_BITS_11_10 {
	POE_PORT_IEEE_BT_POWER_BITS_11_10_PORT_NOT_OPERTIONAL = 0,
	POE_PORT_IEEE_BT_POWER_BITS_11_10_ALTERNATIVE_A,
	POE_PORT_IEEE_BT_POWER_BITS_11_10_ALTERNATIVE_B,
	POE_PORT_IEEE_BT_POWER_BITS_11_10_BOTH_ALTERNATIVES
}POE_PORT_IEEE_BT_POWER_BITS_11_10;


/**********************************************
//              Structures
**********************************************/
typedef struct _POE_SET_USER_BYTE_TO_SAVE     {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
           UINT8  NVMUserByte;
 }POE_SET_USER_BYTE_TO_SAVE, *P_POE_SET_USER_BYTE_TO_SAVE;


typedef struct _POE_SET_PRIVATE_LABEL     {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
           UINT8  RAMPrivateLabel;
 }POE_SET_PRIVATE_LABEL, *P_POE_SET_PRIVATE_LABEL;


typedef struct _POE_GET_BT_SYS_STATUS_REP     {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
           POE_CPU_STATUS2  cpuStatus2;
           POE_FAC_DEFAULT  factoryDefault;
           UINT8  ramPrivateLabel;
           UINT8  nvmUser;
           UINT8  foundActiveDevices;
		   UINT8  foundDevicesAfterBootUp;
		   POE_SYSTEM_EVENT_EXIST  eventExist;
} POE_GET_BT_SYS_STATUS_REP, *P_POE_GET_BT_SYS_STATUS_REP;

typedef struct _POE_GET_BOOT_UP_ERR_REP     {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
		   UINT8   cpuStatus1Err;
		   POE_ERROR_CODE  errorCode;
           UINT8  errorInfo1;
           UINT8  errorInfo2;
           UINT8  downloadType;
} POE_GET_BOOT_UP_ERR_REP, *P_POE_GET_BOOT_UP_ERR_REP;

typedef struct _POE_GET_BT_EVENT_CAUSE_REP     {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
		   UINT8  portCauseEvent0_7;
		   UINT8  portCauseEvent8_15;
		   UINT8  portCauseEvent16_23;
		   UINT8  portCauseEvent24_31;
		   UINT8  portCauseEvent32_39;
		   UINT8  portCauseEvent40_47;
		   POE_SYSTEM_EVENT  systemEventReset;
		   POE_SYSOK_VMAIN_FAULT systemEventVmain;
		   POE_SYSTEM_OK_REG_VMAIN  systemOKRegVmain;
		   POE_SYS_OK_REG_PWR_PREC  systemOKRegPwrPrec;
		   POE_SYS_OK_REG_PWR_VAL   systemOKRegPwrVal;
		   UINT16                   deviceEvent;
} POE_GET_BT_EVENT_CAUSE_REP, *P_POE_GET_BT_EVENT_CAUSE_REP;


typedef struct _POE_GET_SYSTEM_STATUS2_REP {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
           POE_SRS_LVD srsLVD;
           BOOL unrecoverableException;
		   POE_SRS_ILOP srsILOP;
		   POE_SRS_COP  srsCOP;
		   POE_SRS_PIN  srsPIN;
		   POE_SRS_POR  srsPOR;
		   UINT8     gie1;
		   UINT8   resetInfo;
		   POE_DB_RECOVERY dbRecovery;
		   UINT16 savedCommandCount;
}POE_GET_SYSTEM_STATUS2_REP, *P_POE_GET_SYSTEM_STATUS2_REP;

typedef struct _POE_SET_BT_EVENTS_INTERRUPT_MASK {
		/* See Comm. Protocol */
		POE_SYSTEM_EVENTS_MASK_REGISTER	 systemEventMaskRegister;
		POE_DEVICE_EVENTS_MASK_REGISTER	 deviceEventMaskRegister;
		POE_PORT_EVENTS_MASK_REGISTER	 portEventMaskRegister;
}POE_SET_BT_EVENTS_INTERRUPT_MASK, *P_POE_SET_BT_EVENTS_INTERRUPT_MASK;

typedef struct _POE_GET_BT_EVENTS_INTERRUPT_MASK_REP{
		/* See Comm. Protocol */
		POE_SYSTEM_EVENTS_MASK_REGISTER	 systemEventMaskRegister;
		POE_DEVICE_EVENTS_MASK_REGISTER	 deviceEventMaskRegister;
		POE_PORT_EVENTS_MASK_REGISTER	 portEventMaskRegister;
}POE_GET_BT_EVENTS_INTERRUPT_MASK_REP, *P_POE_GET_BT_EVENTS_INTERRUPT_MASK_REP;


typedef struct _POE_SET_INDIVIDUAL_MASK {
		  POE_SYSTEM_BT_MASK_REGISTERS_LIST	 maskNumber;			/* See Comm. Protocol */
		  BOOL       enable;
}POE_SET_INDIVIDUAL_MASK, *P_POE_SET_INDIVIDUAL_MASK;


typedef struct _POE_GET_INDIVIDUAL_MASK{
	POE_SYSTEM_BT_MASK_REGISTERS_LIST   maskNumber;			/* See Comm. Protocol */
}POE_GET_INDIVIDUAL_MASK, *P_POE_GET_INDIVIDUAL_MASK;


typedef struct _POE_GET_INDIVIDUAL_MASK_REP{
	BOOL       enable;
}POE_GET_INDIVIDUAL_MASK_REP, *P_POE_GET_INDIVIDUAL_MASK_REP;



typedef struct _POE_SET_SYSOK_LED_MASK_REG {
		  UINT16	 blinkRegister;			/* See Comm. Protocol */
		  UINT16     lightRegister;
}POE_SET_SYSOK_LED_MASK_REG, *P_POE_SET_SYSOK_LED_MASK_REG;



typedef struct _POE_GET_SYSOK_LED_MASK_REG_REP{
		  UINT16	 blinkRegister;			/* See Comm. Protocol */
		  UINT16     lightRegister;
}POE_GET_SYSOK_LED_MASK_REG_REP, *P_POE_GET_SYSOK_LED_MASK_REG_REP;


typedef struct _POE_SET_BT_POWER_IND_LED {
		  BOOL  indicationType;
		  UINT8  indicationOnPercOrVal;		  /* See Comm. Protocol */
		  UINT8  indicationOffPercOrVal;
}POE_SET_BT_POWER_IND_LED, *P_POE_SET_BT_POWER_IND_LED;


typedef struct _POE_GET_BT_POWER_IND_LED_REP {
		  UINT8	 indicationOnPerc;			/* See Comm. Protocol */
		  UINT8  indicationOffPerc;
		  UINT8  indicationOnVal;		  /* See Comm. Protocol */
		  UINT8  indicationOffVal;
}POE_GET_BT_POWER_IND_LED_REP, *P_POE_GET_BT_POWER_IND_LED_REP;


typedef struct _POE_SET_ADD_CLASS_PWR {
		  POE_CLASS_TYPE	 classType;	/* See Comm. Protocol */
		  UINT8 addedClassPwrval;
}POE_SET_ADD_CLASS_PWR, *P_POE_SET_ADD_CLASS_PWR;

typedef struct _POE_GET_BT_CLASS_PWR {
		  POE_CLASS_TYPE	 classType;	/* See Comm. Protocol */
}POE_GET_BT_CLASS_PWR, *P_POE_GET_BT_CLASS_PWR;

typedef struct _POE_GET_BT_CLASS_PWR_REP {
		  UINT16 classPwr;
		  UINT8 addedClassPwrval;		/* See Comm. Protocol */
		  UINT8 maxAddedClassPwr;		/* See Comm. Protocol */
}POE_GET_BT_CLASS_PWR_REP, *P_POE_GET_BT_CLASS_PWR_REP;



typedef struct _POE_GET_SW_VERSION_REP {
		  UINT8 hwVersion;
		  UINT8 producNum;		/* See Comm. Protocol */
		  UINT8 swPatchVerNum;		/* See Comm. Protocol */
		  UINT8 swMinorVerNum;	  /* See Comm. Protocol */
		  UINT8 swMajorNum;	/* See Comm. Protocol */
		  POE_SW_VER_PARAMS_CODE paramsCodeNum;
		  UINT8 buildNum;
		  UINT16 internalSWNum;
}POE_GET_SW_VERSION_REP, *P_POE_GET_SW_VERSION_REP;


typedef struct _POE_SET_DEV_PARAMETERS {
		  UINT8	chipSetNum;	/* See Comm. Protocol */
		  UINT8 tshVal;
}POE_SET_DEV_PARAMETERS, *P_POE_SET_DEV_PARAMETERS;




typedef struct _POE_GET_BT_DEV_STATUS {
		  UINT8	chipSetNum;	/* See Comm. Protocol */
}POE_GET_BT_DEV_STATUS, *P_POE_GET_BT_DEV_STATUS;


typedef struct _POE_GET_BT_DEV_STATUS_REP {
		  UINT8	chipSetNum;	/* See Comm. Protocol */
		  UINT16 devVersion;
		  POE_DEVICE_STATUS devStatus;
		  UINT8 devEvent;
		  POE_DEVICE_FOUND devFound;
		  BOOL  mDev;
		  UINT8 internalVcalCount;
		  UINT8 temperature;
		  BOOL isNegateTemp;
		  UINT8 tshVal;
		  UINT8 resetCount; 
		  UINT8 porCount; 
		  
}POE_GET_BT_DEV_STATUS_REP, *P_POE_GET_BT_DEV_STATUS_REP;


typedef struct _POE_SET_DEV_REGISTER {
	UINT16 regAddress;	/* See Comm. Protocol */
	UINT16 regData;	/* See Comm. Protocol */
	UINT8 devNum;
}POE_SET_DEV_REGISTER, *P_POE_SET_DEV_REGISTER;

typedef struct _POE_GET_DEV_REGISTER {
	UINT16 regAddress;	/* See Comm. Protocol */
	UINT8 devNum;
}POE_GET_DEV_REGISTER, *P_POE_GET_DEV_REGISTER;

typedef struct _POE_GET_DEV_REGISTER_REP {
	UINT16 regData;	/* See Comm. Protocol */
}POE_GET_DEV_REGISTER_REP, *P_POE_GET_DEV_REGISTER_REP;

typedef struct _POE_GET_DEV_ERROR_LOG {
	UINT8 csNum;	/* See Comm. Protocol */
}POE_GET_DEV_ERROR_LOG, *P_POE_GET_DEV_ERROR_LOG;

typedef struct _POE_GET_DEV_ERROR_LOG_REP {
	POE_DEVICE_SEVERITY_ERROR vopSeverity1;
	POE_DEVICE_SEVERITY_ERROR vopSeverity2;
	POE_DEVICE_SEVERITY_ERROR vopSeverity3;
	POE_DEVICE_STRUCT_ERROR structError;
}POE_GET_DEV_ERROR_LOG_REP, *P_POE_GET_DEV_ERROR_LOG_REP;

typedef struct _POE_GET_DEV_ERROR_LOG_SECTOR_STATUS_REP {
	POE_DEVICE_SECTOR_ERROR sectorError;
	UINT16 eraseSectorCounter;
}POE_GET_DEV_ERROR_LOG_SECTOR_STATUS_REP, *P_POE_GET_DEV_ERROR_LOG_SECTOR_STATUS_REP;




typedef struct _POE_SET_TEMP_MAT {
           UINT8 CHNum;
		   UINT8  physNumA;
		   UINT8 physNumB;
}POE_SET_TEMP_MAT, *P_POE_SET_TEMP_MAT;


typedef struct _POE_GET_PHY_PORT_TMP_MATRIX {
		   UINT8 CHNum;
}POE_GET_PHY_PORT_TMP_MATRIX , *P_POE_GET_PHY_PORT_TMP_MATRIX;


typedef struct _POE_GET_PHY_PORT_TMP_MATRIX_REP {
		   UINT8 physNumA;
		   UINT8 physNumB;
}POE_GET_PHY_PORT_TMP_MATRIX_REP , *P_POE_GET_PHY_PORT_TMP_MATRIX_REP;


typedef struct _POE_GET_PHY_PORT_ACT_MATRIX {
		   UINT8 CHNum;
}POE_GET_PHY_PORT_ACT_MATRIX , *P_POE_GET_PHY_PORT_ACT_MATRIX;


typedef struct _POE_GET_PHY_PORT_ACT_MATRIX_REP {
		   UINT8 physNumA;
		   UINT8 physNumB;
}POE_GET_PHY_PORT_ACT_MATRIX_REP , *P_POE_GET_PHY_PORT_ACT_MATRIX_REP;


typedef struct _POE_GET_PORTS_DEL_POWER_STATE_REP {
		   UINT8  ports0_7;
		   UINT8  ports8_15;
		   UINT8  ports16_23;
		   UINT8  ports24_31;
		   UINT8  ports32_39;
		   UINT8  ports40_47;
}POE_GET_PORTS_DEL_POWER_STATE_REP , *P_POE_GET_PORTS_DEL_POWER_STATE_REP;

typedef struct _POE_SET_BT_PORT_PARAMETERS {
		   UINT8  portNum;
		   POE_PORT_MODE  portModeCFG1;
		   POE_PORT_PM_MODE  portModeCFG2PmMode;
		   POE_CLASS_ERROR_OPER_SELECT portModeCFG2ClassErr;
		   POE_PORT_OPER_MODE  portOperMode;
		   UINT8  addPwrForPortMode;
		   POE_PORT_PRIORITY  priority;
}POE_SET_BT_PORT_PARAMETERS , *P_POE_SET_BT_PORT_PARAMETERS;


typedef struct _POE_GET_BT_PORT_PARAMETERS {
		   UINT8  portNum;
}POE_GET_BT_PORT_PARAMETERS , *P_POE_GET_BT_PORT_PARAMETERS;


typedef struct _POE_GET_BT_PORT_PARAMETERS_REP {
	       POE_PORT_STATUS  portStatus;
		   UINT8  portModeCFG1;
		   POE_PORT_PM_MODE  portModeCFG2PmMode;
		   POE_CLASS_ERROR_OPER_SELECT portModeCFG2ClassErr;
		   POE_PORT_OPER_MODE  portOperMode;
		   UINT8  addPwrForPortMode;
		   POE_PORT_PRIORITY  priority;
		   POE_PORT_OTHER  other;
		   UINT8  MSCCUse12;
}POE_GET_BT_PORT_PARAMETERS_REP , *P_POE_GET_BT_PORT_PARAMETERS_REP;


typedef struct _POE_GET_BT_PORT_STATUS {
		   UINT8  portNum;
}POE_GET_BT_PORT_STATUS , *P_POE_GET_BT_PORT_STATUS;


typedef struct _POE_GET_BT_PORT_STATUS_REP {
	       POE_PORT_STATUS  portStatus;
		   POE_PORT_MODE  portModeCFG1;
		   UINT8  assignedClassPrimary;
		   UINT8  assignedClassSecondary;
		   UINT16  measuredPortPower;
		   UINT8  lastShutdownErrorStatus;
		   POE_PORT_EVENTS_MASK_REGISTER  portEvent;
		   UINT8  MSCCUse12;
}POE_GET_BT_PORT_STATUS_REP , *P_POE_GET_BT_PORT_STATUS_REP;


typedef struct _POE_GET_BT_PORT_COUNTERS {
		   UINT8  portNum;
}POE_GET_BT_PORT_COUNTERS , *P_POE_GET_BT_PORT_COUNTERS;



typedef struct _POE_GET_BT_PORT_COUNTERS_REP {
	       POE_PORT_STATUS  portStatus;
		   UINT8  UDLCount;
		   UINT8  OVLCount;
		   UINT16  SCCount;
		   UINT8  invalidSignatureCount;
		   UINT8  powerDeniedCount;
		   UINT8  MSCCUse11;
		   UINT8  MSCCUse12;
}POE_GET_BT_PORT_COUNTERS_REP , *P_POE_GET_BT_PORT_COUNTERS_REP;


typedef struct _POE_GET_BT_PORT_CLASS {
		   UINT8  portNum;
}POE_GET_BT_PORT_CLASS , *P_POE_GET_BT_PORT_CLASS;


typedef struct _POE_GET_BT_PORT_CLASS_REP {
		   POE_PORT_STATUS  portStatus;
		   POE_PORT_PHY_INFO_CC_RESULT  CCResult;
		   POE_PORT_PHY_INFO_DECIDED_PD_STRUCTURE  decidedPDStructure;
		   UINT8  measuredClassPrimary;
		   UINT8  measuredClassSecondary;
		   UINT8  requestedClassPrimary;
		   UINT8  requestedClassSecondary;
		   UINT16  requestedPower;
		   UINT8  assignedClassPrimary;
		   UINT8  assignedClassSecondary;
		   UINT16  assignedPower;
		   UINT16  autoClassMeasurement;
		   POE_PORT_AUTO_CLASS_SUPPORT  autoClassSupport;
}POE_GET_BT_PORT_CLASS_REP , *P_POE_GET_BT_PORT_CLASS_REP;

typedef struct _POE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST {
		   UINT8  portNum;
	   	   UINT16  pdRequestPowerSingle;
	   	   UINT16  pdRequestPowerDualA;
	   	   UINT16  pdRequestPowerDualB;
	   	   POE_PORT_AUTO_CLASS_OPERATION  autoClassOperation;
	   	   UINT8  cableLength;
}POE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST , *P_POE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST;

typedef struct _POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE {
		   UINT8  portNum;
}POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE , *P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE;


typedef struct _POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP {
		   UINT16  pseAllocatedPwrSingleOrAltA;
	   	   UINT16  pseAllocatedPwrAltB;
	   	   UINT16  pseMaxPwr;
	   	   UINT8  assignedClassPrimary;
	   	   UINT8  assignedClassSecondary;
	   	   POE_PORT_LAYER2_EXECUTION  layer2Execution;
	   	   POE_PORT_LAYER2_USAGE  layer2Usage;
	   	   POE_PORT_IEEE_BT_POWER_BITS_15_14  ieeeBTPwrBitsExt1514;
	   	   POE_PORT_IEEE_BT_POWER_BITS_11_10  ieeeBTPwrBitsExt1110;
	   	   UINT8  cableLength;
}POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP , *P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP;

typedef struct _POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST {
		   UINT8  portNum;
}POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST , *P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST;

typedef struct _POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP {
	   	   UINT16  pdRequestPowerSingle;
	   	   UINT16  pdRequestPowerDualA;
	   	   UINT16  pdRequestPowerDualB;
	   	   UINT8  cableLength;
}POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP , *P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP;

typedef struct _POE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB {
	   	   UINT8  portNum;
	   	   UINT8  pupCmd;
}POE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB , *P_POE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB;

typedef struct _POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB {
	   	   UINT8  portNum;
}POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB , *P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB;

typedef struct _POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP {
	   	   UINT8  pupCmd;
}POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP , *P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP;

typedef struct _POE_SET_PORT_LAYER2_CDP_PD_REQUEST {
	   	   UINT8  portNum;
		   UINT8  reserved;
		   UINT8  pdRequestPwr0;
		   UINT8  pdRequestPwr1;
		   UINT8  pdRequestPwr2;
		   UINT8  pdRequestPwr3;
		   UINT8  pdRequestPwr4;
		   UINT8  pw4;
		   UINT8  pw03;
}POE_SET_PORT_LAYER2_CDP_PD_REQUEST , *P_POE_SET_PORT_LAYER2_CDP_PD_REQUEST;

typedef struct _POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE {
		   UINT8 portNum;
}POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE , *P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE;

typedef struct _POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE_REP {
	   	   UINT16  pseAllocatedPwr;
	   	   UINT16  pdRequestedPwr;
	   	   UINT8  assignedClassPrimary;
	   	   UINT8  assignedClassSecondary;
	   	   POE_PORT_LAYER2_EXECUTION  layer2Execution;
	   	   POE_PORT_LAYER2_USAGE  layer2Usage;
}POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE_REP , *P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE_REP;




typedef struct _POE_GET_TOTAL_POWER_REP {
           UINT16 powerConsumption;
		   UINT16  calcPower;
		   UINT16 availablePower;
		   UINT16  powerLim;
		   UINT8  powerBank;
		   UINT16  vmainVoltage;
}POE_GET_TOTAL_POWER_REP, *P_POE_GET_TOTAL_POWER_REP;

typedef struct _POE_SET_POWER_BANKS {
           UINT8 bank;
		   UINT16  powerLim;
		   UINT16 maxShutdownVoltage;
		   UINT16  minShutdownVoltage;
		   UINT8  guardBand;
}POE_SET_POWER_BANKS, *P_POE_SET_POWER_BANKS;

typedef struct _POE_GET_POWER_BANKS {
           UINT8 bank;
}POE_GET_POWER_BANKS, *P_POE_GET_POWER_BANKS;

typedef struct _POE_GET_POWER_BANKS_REP {
		   UINT16  powerLim;
		   UINT16 maxShutdownVoltage;
		   UINT16  minShutdownVoltage;
		   UINT8  guardBand;
		   UINT8 sourceType;
		   UINT8 Rmode;
}POE_GET_POWER_BANKS_REP, *P_POE_GET_POWER_BANKS_REP;

typedef struct _POE_GET_BT_PORT_MEASUREMENTS {
           UINT8 CHNum;
}POE_GET_BT_PORT_MEASUREMENTS, *P_POE_GET_BT_PORT_MEASUREMENTS;

typedef struct _POE_GET_POWER_SUPPLY_PARAMETERS_REP {
		   UINT16  powerConsumption;
		   UINT16 maxShutdownVoltage;
		   UINT16  minShutdownVoltage;
		   UINT8  powerBank;
		   UINT16 powerLim;
}POE_GET_POWER_SUPPLY_PARAMETERS_REP, *P_POE_GET_POWER_SUPPLY_PARAMETERS_REP;

typedef struct _POE_GET_BT_PORT_MEASUREMENTS_REP {
           UINT16 vmainVoltage;
		   UINT16 calculatedCurrent;
		   UINT16 measuredPortPower;
		   UINT16 portVoltage;
}POE_GET_BT_PORT_MEASUREMENTS_REP, *P_POE_GET_BT_PORT_MEASUREMENTS_REP;

typedef struct _POE_GET_FLOWCHART {

	  POE_SYSTEM_OK_REG_VMAIN  systemOKRegVmain;
	  POE_SYS_OK_REG_PWR_PREC  systemOKRegPwrPrec;
	  POE_SYS_OK_REG_PWR_VAL   systemOKRegPwrVal;
 	  BOOL portsStatusListBool[48];/* to know if there is an event in the port */
 	  BOOL portsClassListBool[48];/* to know if we need to call get BT port class */
 	  BOOL portsCountersListBool[48];/* to know if we need to call get BT port counter */
	  BOOL deviceListBool[12];/* to know if there is an event in the device */

	  POE_GET_BT_DEV_STATUS_REP deviceList[12];/* to know if there is an event in the device */

	  POE_GET_BT_PORT_STATUS_REP portsStatusList[48];
	  POE_GET_BT_PORT_CLASS_REP portsClassList[48];
	  POE_GET_BT_PORT_COUNTERS_REP portsCountersList[48];

	  POE_GET_BT_PORT_MEASUREMENTS_REP nePortsStatusList[48];

	  /* Get Total Power*/
      UINT16 powerConsumption;
	  UINT16  calcPower;
	  UINT16 availablePower;
	  UINT16  powerLim;
	  UINT8  powerBank;
	  UINT16  vmainVoltage;


}POE_GET_FLOWCHART, *P_POE_GET_FLOWCHART;






typedef struct _POE_REQUSET_MESSAGE {
	UINT8 key;
	UINT8 echo;
	UINT8 sub;
	UINT8 sub1;
	UINT8 sub2;
	UINT8 data1;
	UINT8 data2;
	UINT8 data3;
	UINT8 data4;
	UINT8 data5;
	UINT8 data6;
	UINT8 data7;
	UINT8 data8;
	UINT8 MBS;
	UINT8 LSB;
} POE_REQUSET_MESSAGE, *P_POE_REQUSET_MESSAGE;


/**********************************************/
/*              Enums	                       */
/**********************************************/


/**********************************************/
/*              Structures                      */
/**********************************************/





/**********************************************/
/*              Requests                        */
/**********************************************/


#define POE_CODE_INIT_POE_API				API_INFRASTRCTURE_COMMAND_BASE	
#define POE_CODE_SET_DOWNLOAD_FILE          API_INFRASTRCTURE_COMMAND_BASE+1


#define POE_CODE_RESET_COMMAND    			API_SYSTEM_COMMAND_BASE
#define POE_CODE_RES_FAC_DEFAULT		 	API_SYSTEM_COMMAND_BASE+1
#define POE_CODE_SAVE_SYS_SETTING		 	API_SYSTEM_COMMAND_BASE+2
#define POE_CODE_SET_USER_BYTE_SAVE         API_SYSTEM_COMMAND_BASE+3
#define POE_CODE_SET_PRIVATE_LABEL	        API_SYSTEM_COMMAND_BASE+4
#define POE_CODE_GET_BT_SYSTEM_STATUS       API_SYSTEM_COMMAND_BASE+5
#define POE_CODE_GET_BT_SYSTEM_STATUS_REP   API_SYSTEM_COMMAND_BASE+6
#define POE_CODE_GET_BT_EVENT_CAUSE         API_SYSTEM_COMMAND_BASE+7
#define POE_CODE_GET_BT_EVENT_CAUSE_REP     API_SYSTEM_COMMAND_BASE+8
#define POE_CODE_GET_SYSTEM_STATUS2         API_SYSTEM_COMMAND_BASE+9
#define POE_CODE_GET_SYSTEM_STATUS2_REP     API_SYSTEM_COMMAND_BASE+10
#define POE_CODE_SET_BT_EVENTS_INTERRUPT_MASK API_SYSTEM_COMMAND_BASE+11
#define POE_CODE_GET_BT_EVENTS_INTERRUPT_MASK API_SYSTEM_COMMAND_BASE+12
#define POE_CODE_GET_BT_EVENTS_INTERRUPT_MASK_REP API_SYSTEM_COMMAND_BASE+13
#define POE_CODE_SET_INDIVIDUAL_MASK         API_SYSTEM_COMMAND_BASE+14
#define POE_CODE_GET_INDIVIDUAL_MASK         API_SYSTEM_COMMAND_BASE+15
#define POE_CODE_GET_INDIVIDUAL_MASK_REP     API_SYSTEM_COMMAND_BASE+16
/*#define POE_CODE_SET_SYSOK_LED_MASK_REG     API_SYSTEM_COMMAND_BASE+17
#define POE_CODE_GET_SYSOK_LED_MASK_REG     API_SYSTEM_COMMAND_BASE+18
#define POE_CODE_GET_SYSOK_LED_MASK_REG_REP API_SYSTEM_COMMAND_BASE+19*/
#define POE_CODE_SET_BT_POWER_IND_LED          API_SYSTEM_COMMAND_BASE+17
#define POE_CODE_GET_BT_POWER_IND_LED          API_SYSTEM_COMMAND_BASE+18
#define POE_CODE_GET_BT_POWER_IND_LED_REP      API_SYSTEM_COMMAND_BASE+19
#define POE_CODE_SET_BT_CLASS_ADD_POWER     API_SYSTEM_COMMAND_BASE+20
#define POE_CODE_GET_BT_CLASS_POWER         API_SYSTEM_COMMAND_BASE+21
#define POE_CODE_GET_BT_CLASS_POWER_REP     API_SYSTEM_COMMAND_BASE+22
#define POE_CODE_GET_SW_VERSION             API_SYSTEM_COMMAND_BASE+23
#define POE_CODE_GET_SW_VERSION_REP         API_SYSTEM_COMMAND_BASE+24
#define POE_CODE_SET_LOG_SEC_CLEAR_STAMP    API_SYSTEM_COMMAND_BASE+25
#define POE_CODE_GET_BOOT_UP_ERROR_REP      API_SYSTEM_COMMAND_BASE+26




#define POE_CODE_SET_DEV_PARAMETERS         API_DEVICE_COMMAND_BASE
#define POE_CODE_GET_BT_DEV_STATUS          API_DEVICE_COMMAND_BASE+1
#define POE_CODE_GET_BT_DEV_STATUS_REP      API_DEVICE_COMMAND_BASE+2
#define POE_CODE_SET_DEV_REGISTER           API_DEVICE_COMMAND_BASE+3
#define POE_CODE_GET_DEV_REGISTER           API_DEVICE_COMMAND_BASE+4
#define POE_CODE_GET_DEV_REGISTER_REP       API_DEVICE_COMMAND_BASE+5
#define POE_CODE_GET_DEV_ERROR_LOG          API_DEVICE_COMMAND_BASE+6
#define POE_CODE_GET_DEV_ERROR_LOG_REP      API_DEVICE_COMMAND_BASE+7
#define POE_CODE_GET_LOG_SEC_STATUS         API_DEVICE_COMMAND_BASE+8
#define POE_CODE_GET_LOG_SEC_STATUS_REP     API_DEVICE_COMMAND_BASE+9

#define POE_CODE_SET_TEMP_MATRIX            	API_PORT_COMMAND_BASE
#define POE_CODE_GET_PHY_PORT_TMP_MATRIX    	API_PORT_COMMAND_BASE+1
#define POE_CODE_GET_PHY_PORT_TMP_MATRIX_REP  	API_PORT_COMMAND_BASE+2
#define POE_CODE_PROGRAM_GLOBAL_MATRIX        API_PORT_COMMAND_BASE+3
#define POE_CODE_GET_PHY_PORT_ACT_MATRIX    	API_PORT_COMMAND_BASE+4
#define POE_CODE_GET_PHY_PORT_ACT_MATRIX_REP  	API_PORT_COMMAND_BASE+5
#define POE_CODE_GET_PORTS_DEL_POWER_STATE     API_PORT_COMMAND_BASE+6
#define POE_CODE_GET_PORTS_DEL_POWER_STATE_REP API_PORT_COMMAND_BASE+7
#define POE_CODE_SET_BT_PORT_PARAMETERS       API_PORT_COMMAND_BASE+8
#define POE_CODE_GET_BT_PORT_PARAMETERS       API_PORT_COMMAND_BASE+9
#define POE_CODE_GET_BT_PORT_PARAMETERS_REP   API_PORT_COMMAND_BASE+10
#define POE_CODE_GET_BT_PORT_STATUS           API_PORT_COMMAND_BASE+11
#define POE_CODE_GET_BT_PORT_STATUS_REP       API_PORT_COMMAND_BASE+12
#define POE_CODE_GET_BT_PORT_COUNTERS         API_PORT_COMMAND_BASE+13
#define POE_CODE_GET_BT_PORT_COUNTERS_REP     API_PORT_COMMAND_BASE+14
#define POE_CODE_GET_BT_PORT_CLASS            API_PORT_COMMAND_BASE+15
#define POE_CODE_GET_BT_PORT_CLASS_REP        API_PORT_COMMAND_BASE+16
#define POE_CODE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST   API_PORT_COMMAND_BASE+17
#define POE_CODE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE   API_PORT_COMMAND_BASE+18
#define POE_CODE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP   API_PORT_COMMAND_BASE+19
#define POE_CODE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST   API_PORT_COMMAND_BASE+20
#define POE_CODE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP   API_PORT_COMMAND_BASE+21
#define POE_CODE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB   API_PORT_COMMAND_BASE+22
#define POE_CODE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB   API_PORT_COMMAND_BASE+23
#define POE_CODE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP   API_PORT_COMMAND_BASE+24
#define POE_CODE_SET_PORT_LAYER2_CDP_PD_REQUEST   API_PORT_COMMAND_BASE+25
#define POE_CODE_GET_BT_PORT_LAYER2_CDP_PSE_DATA   API_PORT_COMMAND_BASE+26
#define POE_CODE_GET_BT_PORT_LAYER2_CDP_PSE_DATA_REP   API_PORT_COMMAND_BASE+27

#define POE_CODE_GET_TOTAL_POWER              API_POWER_MNGT_COMMAND_BASE
#define POE_CODE_GET_TOTAL_POWER_REP          API_POWER_MNGT_COMMAND_BASE+1
#define POE_CODE_SET_POWER_BANKS              API_POWER_MNGT_COMMAND_BASE+2
#define POE_CODE_GET_POWER_BANKS              API_POWER_MNGT_COMMAND_BASE+3
#define POE_CODE_GET_POWER_BANKS_REP          API_POWER_MNGT_COMMAND_BASE+4
#define POE_CODE_GET_POWER_SUPP_PARAMS        API_POWER_MNGT_COMMAND_BASE+5
#define POE_CODE_GET_POWER_SUPP_PARAMS_REP    API_POWER_MNGT_COMMAND_BASE+6
#define POE_CODE_GET_BT_PORT_MEAS            API_POWER_MNGT_COMMAND_BASE+7 
#define POE_CODE_GET_BT_PORT_MEAS_REP        API_POWER_MNGT_COMMAND_BASE+8          
        
#define POE_CODE_SET_DERATING_DATA            API_POWER_DER_COMMAND_BASE
#define POE_CODE_GET_DERATING_DATA            API_POWER_DER_COMMAND_BASE+1
#define POE_CODE_GET_DERATING_DATA_REP        API_POWER_DER_COMMAND_BASE+2
#define POE_CODE_SET_DERATING_USER_TEMP       API_POWER_DER_COMMAND_BASE+3
#define POE_CODE_GET_DERATING_USER_TEMP        API_POWER_DER_COMMAND_BASE+4
#define POE_CODE_GET_DERATING_USER_TEMP_REP    API_POWER_DER_COMMAND_BASE+5
#define POE_CODE_GET_DERATING_SYS_MEAS        API_POWER_DER_COMMAND_BASE+6
#define POE_CODE_GET_DERATING_SYS_MEAS_REP    API_POWER_DER_COMMAND_BASE+7

#define POE_CODE_DOWNLOAD_COMMAND   API_SOFTWARE_DOWNLOAD




#endif  /* _MTIL_API_INC_ */
