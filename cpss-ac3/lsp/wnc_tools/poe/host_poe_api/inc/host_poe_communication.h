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
*  Description: contain defines for implementation of the communication of our system.
*
*************************************************************************/

#ifndef _POE_COMM_INC_
#define _POE_COMM_INC_


#include "host_poe_comm_global.h"



#define SIZE_OF_COM_READ_BUF  64
#define ECO_SYSTEM_STATUS     255
#define POE_FIRMWARE_ERROR     2




#define BUF_LEN_TO_READ       1
#define NEED_UPDATE_REPLY     1



/* Little endian word variable as 2 bytes */
typedef union
{
    UINT16 word_var;
    struct
    {
        UINT8 lsb;
        UINT8 msb;
    } bytes;
} POE_WORD_VAR_U;



/* static var*/ 




typedef enum{
POE_COMMAND_KEY,
POE_PROGRAM_KEY,	
POE_REQUEST_KEY,
POE_TELEMETRY_KEY,
POE_TEST_KEY,
POE_REPORT_KEY =0x52

}POE_KEY_E;

typedef enum{
POE_GLOBAL_SUB = 7
}POE_SUB_E;


#define ZERO_CODE   0x0
#define CLEAR_LOG  0x01
#define BT_POWER_INDICATION  0x08
#define E2_PARAM 0x06
#define SAVE_CONFIG 0x0F
#define VERSION_Z 0x1E
#define SW_VERSION 0x21
#define RESTORE_FACTORY 0x2D
#define USER_BYTE 0x41
#define POE_NA_CODE 0x4E
#define RESET_CODE 0x55
#define INDVIDUAL_MASK  0x56
#define BT_IRQ_MASK  0x64
#define SYSTEM_STATUS2  0x84
#define DEVICE_PARAMS   0x87
#define SYS_OK_MASK  0xA1
#define BT_SYSTEM_STATUS 0xD0
#define SYSTEM_STATUS 0x3D
#define BT_EVENT  0xD1
#define BT_CLASS_PWR  0xD2
#define BT_DEV_INFO  0xD3

#define SUPPLY  0x0B
#define TOTAL_POWER  0x60
#define POWER_BUDGET 0x57
#define POE_MAX_POWER_BANK 0x0F
#define CHANNEL 0x05
#define BT_PORT_MEASE 0xC5
#define MAIN  0x17


#define READ_ASIC_REG  0x52
#define WRITE_ASIC_REG  0x53
#define DEVICE_LOG  0xC3
#define LOG_SECTOR_STATUS  0xC4


#define TMP_MAT  0x43
#define CHANNEL_MAT  0x44
#define ALL_PORTS_DELIVERING  0xC0
#define BT_PORT_CONFIG1  0xC0
#define BT_PORT_STATUS 0xC1
#define BT_PORT_COUNTERS 0xC2
#define BT_PORT_CLASS 0xC4

#define LAYER2_BT_LLDP_PD 0x50
#define LAYER2_BT_LLDP_PSE 0x51
#define LAYER2_CDP_PWR_UP_ALTB 0x52
#define LAYER2_CDP_PD_REQUEST 0x53
#define LAYER2_CDP_PSE 0x54


#define POE_MAX_NVM_USER_BYTE  0xFE
#define POE_MAX_PERC_BT_POWER_INDICATION  0x64
#define POE_MIN_PERC_BT_POWER_INDICATION   0x0A
#define POE_MIN_RAM_PRIVATE_LABEL  0x01
#define POE_MAX_RAM_PRIVATE_LABEL  0xFF
#define POE_MAX_POWER_LIMIT 0x1770
#define POE_MAX_VOLTAGE 0x249
#define POE_MIN_VOLTAGE 0x1F4

#define POE_MESSAGE_NOT_RANGE "is not in the right range \n"
#define POE_MESSAGE_WRONG_PARAMETERS_FROM_USER "the user gave wrong parameters \n"


#define FILL_COMMAND_GLOBAL \
	pTxParams[varIndex++]= POE_COMMAND_KEY; \
	pTxParams[varIndex++]= echoNum; \
	pTxParams[varIndex++]= POE_GLOBAL_SUB;

#define FILL_COMMAND_CHANNEL \
		pTxParams[varIndex++]= POE_COMMAND_KEY; \
		pTxParams[varIndex++]= echoNum; \
		pTxParams[varIndex++]= CHANNEL;

#define FILL_TEST \
	pTxParams[varIndex++]= POE_TEST_KEY; \
	pTxParams[varIndex++]= echoNum; \
	pTxParams[varIndex++]= POE_TEST_KEY;

#define FILL_REQUEST_TEST \
	pTxParams[varIndex++]= POE_REQUEST_KEY; \
	pTxParams[varIndex++]= echoNum; \
	pTxParams[varIndex++]= POE_TEST_KEY;


#define FILL_PROGRAMM \
	pTxParams[varIndex++]= POE_PROGRAM_KEY; \
	pTxParams[varIndex++]= echoNum;
	
#define FILL_REQUEST_GLOBAL \
	pTxParams[varIndex++]= POE_REQUEST_KEY; \
	pTxParams[varIndex++]= echoNum; \
	pTxParams[varIndex++]= POE_GLOBAL_SUB;

#define FILL_REQUEST_CHANNEL \
				pTxParams[varIndex++]= POE_REQUEST_KEY; \
				pTxParams[varIndex++]= echoNum; \
				pTxParams[varIndex++]= CHANNEL;


#define CALCULATE_CHECK_SUM(_result) \
	  		int i;                   \
			for (i = 0; i < 13; i++) \
 			_result += pTxParams[i];

#define CALCULATE_PORTS_EVENT(_ports, _start) \
		for (i=0; i<8; i++){   \
			pGetFlowchart->portsStatusListBool[i+_start] = (_ports & count) >> i; \
			count = 2 * count;   \
		}                  \
		count = 1; \

#define CALCULATE_DEVICES_EVENT(_devices) \
		for (i=0; i<12; i++){   \
			pGetFlowchart->deviceListBool[i] = (_devices & count) >> i; \
			count = 2 * count;   \
		}                  \
		count = 1; \


#define RETURN_READ_RC(_result,_len,_rec) \
			if(_len != _rec)  \
				return PoE_RC_ERR_COMM_TIMEOUT; \
			if (_result != PoE_RC_SUCSESS)  \
				return rc;

#define LOW_BYTE(x) ( (UINT8)(  (x) & 0x00FF) )
#define HIGH_BYTE(x) ( (UINT8)( ((x)>> 8) & 0x00FF) )



#endif  /* _MTIL_API_INC_ */
