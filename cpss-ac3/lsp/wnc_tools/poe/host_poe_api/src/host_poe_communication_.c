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
*  Description: contain the implementation of the communication of our system.
*
*************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <time.h>
#include <fcntl.h>
#include <pthread.h>

#include "../inc/host_poe_api.h"
#include "../inc/host_poe_communication.h"
#include "../../host_poe_hal/inc/host_poe_comm_device.h"
#include "../../host_poe_hal/inc/host_poe_comm_os.h"


#define POE_INFRA_ID		0
#define POE_SYS_ID			1
#define POE_DEV_ID			2
#define POE_PORT_ID			3
#define POE_POWER_MNG_ID	4
#define POE_POWER_DER_ID	5
#define POE_SOFTWARE_DOWNLOAD_ID	6
#define POE_NUMIDS          7

#define SIZE_OF_COM_READ_BUF  64

typedef PoE_RC_e (*ReqTranslate) (IN OUT P_POE_BASE_MESSAGE pmsg);



PoE_RC_e POECommValidateCheckSum(void);


PoE_RC_e POECommTranslateInfraMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslateSysSMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslateDevMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslatePortMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslatePowerMngMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslateDownloadSwrMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommTranslatePowerDerMsg(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e getFlow(IN P_POE_GET_FLOWCHART pGetFlowchart);
PoE_RC_e POECommOpenDev(void);
PoE_RC_e POECommCloseDev(void);
PoE_RC_e POECommWriteDev(void);
PoE_RC_e POECommReadDev(UINT8 bufLenToRead, UINT8* bufLenRead,BOOL readTimeout);
PoE_RC_e POECommClearDev(void);
PoE_RC_e POECommWaitReadReply (void);
PoE_RC_e POECommCheckReport(void);
PoE_RC_e POECommCheckReadReply(int * updateReply);
PoE_RC_e POECommUpdateInfraReply(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdateSysReply(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdateDevReply(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdatePortReply(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdatePowerMngReply(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdateFlowchart(IN OUT P_POE_BASE_MESSAGE pmsg);
PoE_RC_e POECommUpdatePowerDerReply(IN OUT P_POE_BASE_MESSAGE pmsg);


PoE_RC_e POECommUpdateAndSend(void);
INT8 POEAPIReqCodeToSGId(IN unsigned short poe_req_code);



static ReqTranslate ReqTranslators[POE_NUMIDS] = {
	POECommTranslateInfraMsg,
	POECommTranslateSysSMsg,
	POECommTranslateDevMsg,
	POECommTranslatePortMsg,
	POECommTranslatePowerMngMsg,
	POECommTranslatePowerDerMsg,
	POECommTranslateDownloadSwrMsg

};

static ReqTranslate ReplyTranslators[POE_NUMIDS] = {
	POECommUpdateInfraReply,
	POECommUpdateSysReply,
	POECommUpdateDevReply,
	POECommUpdatePortReply,
	POECommUpdatePowerMngReply,
	POECommUpdatePowerDerReply

};
/***************static variables *****************************/
static UINT8 echoNum = 0;
static UINT8 varIndex = 0;


/***************global variables *****************************/
UINT8 pTxParams[POE_MESSAGE_SIZE];
UINT8 pRxParams[POE_MESSAGE_SIZE];
UINT8 pRxParamsServer[POE_MESSAGE_SIZE];

UINT8 pRxBuf[SIZE_OF_COM_READ_BUF];

POE_HOST_COMM_DEVICE commDevice;


/***************************Macro defintions *******************************************/





/******************************************************** Functions ************************************/

void serverReply(){
	pRxParamsServer[0] = 0x03;
	pRxParamsServer[1] = echoNum;
	pRxParamsServer[2] = 0x03;
	pRxParamsServer[3] = 0x02;
	pRxParamsServer[4] = 0x02;
	pRxParamsServer[5] = 0x49;
	pRxParamsServer[6] = 0x02;
	pRxParamsServer[7] = 0x0A;
	pRxParamsServer[8] = 0x4E;
	pRxParamsServer[9] = 0x70;
	pRxParamsServer[10] = 0x00;
	pRxParamsServer[11] = 0x11;
	pRxParamsServer[12] = 0x4E;
	int result = 0;
	CALCULATE_CHECK_SUM(result);
	pRxParamsServer[13] = (result >> 8) & 0xFF; /* update checksum MBS */
	pRxParamsServer[14] = result & 0xFF; /* update checksum LSB */
}

void POECommSetVarArg(char *msg, va_list ap)
{

   
    int i = 0;
    UINT16 tmp;
    POE_WORD_VAR_U word_data;

    
    for (i = 0; ( (msg[i] != '\0') && (i < POE_MESSAGE_SIZE) ) ; i++)
    {
        if (msg[i] != '%')
        {
            pTxParams[varIndex] = msg[i];
            varIndex++;
            continue;
        }
        switch (msg[i + 1])
        {
            case 'c':
                pTxParams[varIndex] = va_arg(ap, int);
                varIndex++;

                break;


            case 'w' :

                tmp = va_arg(ap, int);
                word_data.word_var = tmp;

                pTxParams[varIndex] = word_data.bytes.msb;
                pTxParams[varIndex + 1] = word_data.bytes.lsb;

                varIndex += sizeof(UINT16);

                break;


            default:
                 pTxParams[varIndex] = msg[i];

                break;
        }
        i++;
    }

   
}



void POECommSetVarMsg(char *msg, ...)
{				


    va_list ap;

    va_start(ap, msg);
    POECommSetVarArg( msg, ap);
    va_end(ap);


}

PoE_RC_e POECommValidateCheckSum(void)
{
	UINT16 csCalc = 0,cs =0;
	UINT8 i = 0;

	for (i = 0; i < (POE_MESSAGE_SIZE - 2); i++)
		csCalc += pRxParams[i];

	cs = (UINT16)((pRxParams[POE_MESSAGE_SIZE - 2] << 8) + pRxParams[POE_MESSAGE_SIZE - 1]);

	if (cs != csCalc)
		return PoE_RC_ERR_WRONG_CS;

	return PoE_RC_SUCSESS;
}

PoE_RC_e POECommOpenDev(void)
{

	switch (commDevice.devType)
		{

			case POE_COMM_UART_ASYNC:
				break;
			case POE_COMM_UART_SYNC:
				return poeCommDevOpenUART();
				break;

			case POE_COMM_I2C:
				return poeCommDevOpenI2C();
				break;
			case POE_COMM_SHARED_MEM:
				break;
#ifdef _MSCC_HAL_IPC_QUEUE_C_
			case POE_COMM_QUEUE:
				return poeCommDevOpenQueue();
				break;
#endif
			case POE_COMM_SERVER:
				serverReply();
				int i;
				for (i=0; i<15; i++)
					pRxParams[i] = pRxParamsServer[i];
				return PoE_RC_SUCSESS;
				break;

			default:
				printf("Did not open the comm device.\n\r");
				return PoE_RC_ERR_COMM_DEVICE_NAME;

		}
		return PoE_RC_SUCSESS;


}

PoE_RC_e POECommCloseDev(void)
{

	switch (commDevice.devType)
		{

			case POE_COMM_UART_ASYNC:
				break;
			case POE_COMM_UART_SYNC:
				return poeCommDevCloseUART();
				break;

			case POE_COMM_I2C:
				return poeCommDevCloseI2C();
				break;
			case POE_COMM_SHARED_MEM:
				break;
#ifdef _MSCC_HAL_IPC_QUEUE_C_
			case POE_COMM_QUEUE:
				return poeCommDevCloseQueue();
				break;
#endif
			case POE_COMM_SERVER:
				serverReply();
				int i;
				for (i=0; i<15; i++)
					pRxParams[i] = pRxParamsServer[i];
				return PoE_RC_SUCSESS;
				break;
			default:
				break;

		}
		return PoE_RC_SUCSESS;



}

PoE_RC_e POECommWriteDev(void)
{
	
	
	switch (commDevice.devType)
		{

			case POE_COMM_UART_ASYNC:
				break;
			case POE_COMM_UART_SYNC:
				return poeCommDevWriteUART();
				break;

			case POE_COMM_I2C:
				return poeCommDevWriteI2C(POE_MESSAGE_SIZE);
				break;
			case POE_COMM_SHARED_MEM:
				break;
#ifdef _MSCC_HAL_IPC_QUEUE_C_
			case POE_COMM_QUEUE:
				return poeCommDevWriteQueue();
				break;
#endif
			case POE_COMM_SERVER:
				serverReply();
				int i;
				for (i=0; i<15; i++)
					pRxParams[i] = pRxParamsServer[i];
				return PoE_RC_SUCSESS;
				break;
			default:
				break;

		}
		return PoE_RC_SUCSESS;
}


PoE_RC_e POECommReadDev(UINT8 bufLenToRead, UINT8* bufLenRead,BOOL readTimeout)
{
	
	memset(pRxBuf,0,SIZE_OF_COM_READ_BUF);
	switch (commDevice.devType)
		{

			case POE_COMM_UART_ASYNC:
				break;
			case POE_COMM_UART_SYNC:
				return poeCommDevReadUART(bufLenToRead,bufLenRead,readTimeout);
				break;

			case POE_COMM_I2C:
				poeCommDevReadI2C((UINT16)bufLenToRead,(UINT16*)bufLenRead,readTimeout);
				break;
			case POE_COMM_SHARED_MEM:
				break;
#ifdef _MSCC_HAL_IPC_QUEUE_C_
			case POE_COMM_QUEUE:
				return poeCommDevReadQueue(bufLenToRead,bufLenRead,readTimeout);
				break;
#endif
			case POE_COMM_SERVER:
				serverReply();
				int i;
				for (i=0; i<15; i++)
					pRxParams[i] = pRxParamsServer[i];
				return PoE_RC_SUCSESS;
				break;
			default:
				break;

		}
		return PoE_RC_SUCSESS;
}

PoE_RC_e POECommClearDev(void)
{

	memset(pRxBuf,0,SIZE_OF_COM_READ_BUF);

	switch (commDevice.devType)
		{

			case POE_COMM_UART_ASYNC:
				break;
			case POE_COMM_UART_SYNC:
				return poeCommDevClearUART();
				break;

			case POE_COMM_I2C:
				break;
			case POE_COMM_SHARED_MEM:
				break;
#ifdef _MSCC_HAL_IPC_QUEUE_C_
			case POE_COMM_QUEUE:
				return poeCommDevClearQueue();
				break;
#endif
			case POE_COMM_SERVER:
				serverReply();
				int i;
				for (i=0; i<15; i++)
					pRxParams[i] = pRxParamsServer[i];
				return PoE_RC_SUCSESS;
				break;
			default:
				break;

		}
		return PoE_RC_SUCSESS;


}

PoE_RC_e POECommWaitReadReply (void)
{

	#define RECEIVE_RETRY_NUMBER 5

	PoE_RC_e rc;

	int nonValidByteRec=0;
	UINT8 bufLenRead =0;
	int recRetry;


	for(recRetry=0;recRetry<RECEIVE_RETRY_NUMBER;recRetry++)
	{

		if((commDevice.devType == POE_COMM_UART_SYNC) || (commDevice.devType == POE_COMM_I2C))
		{
					/* Wait for reply from PoE device */

			OS_SLEEP(commDevice.devDelay);

		}


		else if ((commDevice.devType == POE_COMM_SERVER)){
			int i;
			for (i=0; i<15; i++)
				pRxParams[i] = pRxParamsServer[i];
			return PoE_RC_SUCSESS;
		}

		{
			/* Read from PoE Device */

			rc=  POECommReadDev(BUF_LEN_TO_READ, &bufLenRead,TRUE);
			if(rc != PoE_RC_SUCSESS)
				return rc; /*exit from while */

			if(pRxBuf[0] == 0)
				nonValidByteRec++;
			else
				nonValidByteRec = 0;

			if (pRxBuf[0] == POE_TELEMETRY_KEY || pRxBuf[0] == POE_REPORT_KEY) /* first msg byte - telemetry or response */
			{
				memcpy(pRxParams, pRxBuf, POE_MESSAGE_SIZE);

				rc = POECommValidateCheckSum();
				if(rc != PoE_RC_SUCSESS)
					return rc;

				if(pRxParams[1] == ECO_SYSTEM_STATUS)
				{


				}
				return rc;	/*success*/

			}
		}

	}		
	return rc;
}


PoE_RC_e POECommCheckReport(void)
{
	
	UINT16  sub0Sub1;;
	
		if ((pRxParams[2] == 0) && (pRxParams[3] == 0))
			return PoE_RC_SUCSESS; /* valid report message was received */
	
		if ((pRxParams[2] == 0xFF) && (pRxParams[3] == 0xFF) && (pRxParams[4] == 0xFF) && (pRxParams[5] == 0xFF))
		{
			return PoE_RC_ERR_WRONG_CS; /* message send to PoE Device contained CRC error */
		}
	
		sub0Sub1 = ((UINT16) pRxParams[2] << 8) | ((UINT16) pRxParams[3]);
	
		if ((sub0Sub1 >= 0x0001) && (sub0Sub1 <= 0x7FFF))
		{
			return PoE_RC_ERR_SUBJECT_BYTES; /* SUBJECT field unknown */
		}
	
		if ((sub0Sub1 >= 0x8000) && (sub0Sub1 <= 0x8FFF))
		{
			return PoE_RC_ERR_DATA_BYTES; /* leagal command, but with illegal value (example: set port power to 100W) */
		}
	
		if (sub0Sub1 == 0xFFFF)
		{
			return PoE_RC_ERR_KEY_VALUE; /* leagal key command) */
		}
	
		
		return PoE_RC_ERR_OUT_OF_RANGE; /* unknown error type */


}



PoE_RC_e POECommCheckReadReply(int * updateReply)
{

	PoE_RC_e rc;

	
	if(rc == PoE_RC_SUCSESS)
	{


		if ((pTxParams[1] != pRxParams[1])&&(pRxParams[1] =! ECO_SYSTEM_STATUS)) /*error in Echo - but not Boot up Error*/
		{
			/* clear device buffer - Read 15bytes reply from PoE Device - Empty all data from RX buffer */


			MUTEX_LOCK;
			rc = POECommClearDev();
			MUTEX_UNLOCK;

			if (rc != PoE_RC_SUCSESS)
				return rc;
			
			return PoE_RC_ERR_ECHO_BYTE;
		}

	/* check if we rec REPORT RX message  ( result of SET message, or error for 'REQUEST' message ) */
		if (pRxParams[0] == POE_REPORT_KEY)
		{
			if((pTxParams[0] ==POE_COMMAND_KEY) ||(pTxParams[0] ==POE_PROGRAM_KEY) )

			if(pTxParams[3]== RESET_CODE)
			{
				OS_SLEEP(4000);

			}
			return( POECommCheckReport());
		}

	
		if (pRxParams[0] == POE_TELEMETRY_KEY) /* Correct Reply */
		{
			if (pTxParams[0] == POE_REQUEST_KEY)
			{

				*updateReply = NEED_UPDATE_REPLY;
				return PoE_RC_SUCSESS; /* We rec valid telemetry, exit retry loop */
			}
			else
				return PoE_RC_ERR_KEY_VALUE;
		}	
		return PoE_RC_ERR_KEY_VALUE;

	}
	
	return rc;

}



PoE_RC_e POECommUpdateInfraReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	return PoE_RC_SUCSESS;
}


PoE_RC_e POECommUpdateSysReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	PoE_RC_e rc = PoE_RC_SUCSESS;
	
	unsigned short codeSend = pmsg->code; 
	memset(pmsg,0,POE_MESSAGE_SIZE);/* reply structure size is up to 15 byte*/

	switch (codeSend)
		{

			case POE_CODE_RESET_COMMAND:
			case POE_CODE_GET_BT_SYSTEM_STATUS:
			{


				if(echoNum == ECO_SYSTEM_STATUS)/* Boot up error or Reset command*/

				{

					if(pRxParams[2]&POE_FIRMWARE_ERROR)
					{
						P_POE_GET_BOOT_UP_ERR_REP pGetBootupErr = (P_POE_GET_BOOT_UP_ERR_REP)pmsg->data;
				
						pGetBootupErr->errorCode = pRxParams[5];
						pGetBootupErr->errorInfo1 = pRxParams[6];
						pGetBootupErr->errorInfo2 = pRxParams[7];
						pGetBootupErr->downloadType = pRxParams[8];
						INIT_BASE_MSG(pmsg,POE_CODE_GET_BOOT_UP_ERROR_REP,sizeof(*pGetBootupErr));
						rc = PoE_RC_ERR_FW_ERR;
					}
					else
						rc = PoE_RC_ERR_RESET_DETECT;

				}

				else
				{
					if(pRxParams[2] != 0)
					{
						P_POE_GET_BOOT_UP_ERR_REP pGetBootupErr = (P_POE_GET_BOOT_UP_ERR_REP)pmsg->data;
						pGetBootupErr->cpuStatus1Err = pRxParams[2];
						pGetBootupErr->errorCode = pRxParams[5];
						pGetBootupErr->errorInfo1 = pRxParams[6];
						pGetBootupErr->errorInfo2 = pRxParams[7];
						pGetBootupErr->downloadType = pRxParams[8];
						INIT_BASE_MSG(pmsg,POE_CODE_GET_BOOT_UP_ERROR_REP,sizeof(*pGetBootupErr));
						rc = PoE_RC_ERR_FW_ERR;
						printf("Boot Up Error \n");
					}
				
					else if(pRxParams[2] == 0 )/* no Boot Up Error*/
					{
						P_POE_GET_BT_SYS_STATUS_REP pGetBTStatusRep = (P_POE_GET_BT_SYS_STATUS_REP)pmsg->data;

						pGetBTStatusRep->cpuStatus2 = (POE_CPU_STATUS2)(pRxParams[3]&3);
						pGetBTStatusRep->factoryDefault = ((pRxParams[4]&1) ? POE_FAC_DEFAULT_SET:POE_FAC_DEFAULT_UNSET);
						pGetBTStatusRep->ramPrivateLabel = pRxParams[6];
						pGetBTStatusRep->nvmUser = pRxParams[7];
						pGetBTStatusRep->foundActiveDevices = (pRxParams[8]&0xF0)>>4;
						pGetBTStatusRep->foundDevicesAfterBootUp = pRxParams[8]&0x0F;
						pGetBTStatusRep->eventExist = ((pRxParams[12]&1) ? POE_SYSTEM_EVENT_IS_EXIST : POE_SYSTEM_EVENT_NOT_EXIST);
						if(pGetBTStatusRep->foundActiveDevices > 0xB)
						{
							printf("foundActiveDevices %s", POE_MESSAGE_NOT_RANGE);
							return PoE_RC_ERR_DATA_BYTES;
						}

						if(pGetBTStatusRep->foundDevicesAfterBootUp > 0xB)
						{
							printf("foundDevicesAfterBootUp %s", POE_MESSAGE_NOT_RANGE);
							return PoE_RC_ERR_DATA_BYTES;
						}
			        	INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_SYSTEM_STATUS_REP,sizeof(*pGetBTStatusRep));
					}
					else/* mismatch error*/
						rc= PoE_RC_ERR_MSG_TYPE;
				}
		
			}
			break;


			case POE_CODE_GET_BT_EVENT_CAUSE:
			{
				P_POE_GET_BT_EVENT_CAUSE_REP pGetBTEventCause = (P_POE_GET_BT_EVENT_CAUSE_REP)pmsg->data;
				UINT16 deviceEvent;

				pGetBTEventCause->portCauseEvent0_7 = pRxParams[2];
				pGetBTEventCause->portCauseEvent8_15 = pRxParams[3];
				pGetBTEventCause->portCauseEvent16_23 = pRxParams[4];
				pGetBTEventCause->portCauseEvent24_31 = pRxParams[5];
				pGetBTEventCause->portCauseEvent32_39 = pRxParams[6];
				pGetBTEventCause->portCauseEvent40_47 = pRxParams[7];

				pGetBTEventCause->systemEventReset = (pRxParams[8]&1) ? POE_SYSTEM_EVENT_RESET_OR_RESTORE : POE_SYSTEM_EVENT_NOT_OCCURRED;
				pGetBTEventCause->systemEventVmain = (pRxParams[8]&2) ? POE_SYSOK_VMAIN_FAULT_OUT_RANGE : POE_SYSOK_VMAIN_FAULT_IN_RANGE;
				pGetBTEventCause->systemOKRegVmain = ((pRxParams[10]&1) ? POE_SYSOK_REG_VMAIN_IN_RANGE :POE_SYSOK_REG_VMAIN_OUT_RANGE);
				pGetBTEventCause->systemOKRegPwrPrec = ((pRxParams[10]&2) ? POE_SYSOK_REG_PWR_ABOVE_IND_ON_RREC :POE_SYSOK_REG_PWR_BELLOW_IND_OFF_RREC);
			    pGetBTEventCause->systemOKRegPwrVal = ((pRxParams[10]&4) ? POE_SYSOK_REG_PWR_ABOVE_IND_ON_VAL :POE_SYSOK_REG_PWR_BELLOW_IND_OFF_VAL);

				deviceEvent = (UINT16) (((pRxParams[11] & 0xFFFF)<<8)|(pRxParams[12] & 0xFFFF));

				pGetBTEventCause->deviceEvent = (deviceEvent&POE_DEVICE_EVENT_0) | ( deviceEvent&POE_DEVICE_EVENT_1) |(deviceEvent&POE_DEVICE_EVENT_2) | (deviceEvent&POE_DEVICE_EVENT_3) |\
												(deviceEvent&POE_DEVICE_EVENT_4) | ( deviceEvent&POE_DEVICE_EVENT_5) |(deviceEvent&POE_DEVICE_EVENT_6) |  (deviceEvent&POE_DEVICE_EVENT_7) |\
												(deviceEvent&POE_DEVICE_EVENT_8) | ( deviceEvent&POE_DEVICE_EVENT_9) |(deviceEvent&POE_DEVICE_EVENT_10) |  (deviceEvent&POE_DEVICE_EVENT_11); 




				INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_EVENT_CAUSE_REP,sizeof(*pGetBTEventCause));

			}

			break;



			case POE_CODE_GET_SYSTEM_STATUS2:

			{

				P_POE_GET_SYSTEM_STATUS2_REP pGetSysStatus2  = (P_POE_GET_SYSTEM_STATUS2_REP)pmsg->data;

				pGetSysStatus2->srsLVD = ((pRxParams[2]&2) ? POE_SRS_LVD_RESET_CAUSE :POE_SRS_LVD_RESET_NOT_CAUSE);
				pGetSysStatus2->unrecoverableException = ((pRxParams[2]&8) ? 1 : 0);
				pGetSysStatus2->srsILOP = ((pRxParams[2]&16) ? POE_SRS_ILOP_RESET_CAUSE :POE_SRS_ILOP_RESET_NOT_CAUSE);
				pGetSysStatus2->srsCOP = ((pRxParams[2]&32) ? POE_SRS_COP_RESET_CAUSE :POE_SRS_COP_RESET_NOT_CAUSE);
				pGetSysStatus2->srsPIN = ((pRxParams[2]&64) ? POE_SRS_PIN_RESET_CAUSE :POE_SRS_PIN_RESET_NOT_CAUSE);
				pGetSysStatus2->srsPOR = ((pRxParams[2]&128) ? POE_SRS_POR_RESET_CAUSE :POE_SRS_POR_RESET_NOT_CAUSE);
				pGetSysStatus2->gie1 = (pRxParams[3]&POE_GIE1_CLK_LOSS_IRQ) | ( pRxParams[3]&POE_GIE1_UART_ON_I2C_IRQ) |(pRxParams[3]&POE_GIE1_UNAUTHO_IRQ) | \
											(pRxParams[3]&POE_GIE1_I2C_ARB_LOSS) |  (pRxParams[3]&POE_GIE1_EXT_CLK_REC_FAIL) |(pRxParams[3]&POE_GIE1_VMAIN_OUT_RANGE) | \
											(pRxParams[3]&POE_GIE1_CPU_VOL_WARNNING) | ( pRxParams[3]&POE_GIE1_UART_ERROR_IRQ); 
				pGetSysStatus2->resetInfo = (pRxParams[4]&POE_RESET_INFO_COMM_RESET_COMMAND) | ( pRxParams[4]&POE_RESET_INFO_CLK_RECOVERY_FAIL) |(pRxParams[4]&POE_RESET_INFO_DEVICE_FAIL) | \
											(pRxParams[4]&POE_RESET_INFO_I2C_RESTART) |	(pRxParams[4]&POE_RESET_INFO_SELF_RESET);

				pGetSysStatus2->dbRecovery = (POE_DB_RECOVERY)pRxParams[6];	
				pGetSysStatus2->savedCommandCount = pRxParams[10];
				INIT_BASE_MSG(pmsg,POE_CODE_GET_SYSTEM_STATUS2_REP,sizeof(*pGetSysStatus2));

				

			}
			break;

			case POE_CODE_GET_BT_EVENTS_INTERRUPT_MASK:
			{
				P_POE_GET_BT_EVENTS_INTERRUPT_MASK_REP pGetIntMask = (P_POE_GET_BT_EVENTS_INTERRUPT_MASK_REP)pmsg->data;
				pGetIntMask->systemEventMaskRegister = (pRxParams[2]&POE_SYSTEM_EVENT_RESET_REGISTER) | (pRxParams[2]&POE_SYSTEM_EVENT_VMAIN_FAULT);
				pGetIntMask->deviceEventMaskRegister = (pRxParams[3]&POE_DEVICE_EVENT_DISCONNECTION_TEMP) | (pRxParams[3]&POE_DEVICE_EVENT_USER_DEFINED_TEMP) | (pRxParams[3]&POE_DEVICE_EVENT_DEVICE_FAULT) | (pRxParams[3]&POE_DEVICE_EVENT_DEVICE_REFRESHED);
				pGetIntMask->portEventMaskRegister = (pRxParams[4]&POE_PORT_EVENT_TURNED_ON) | (pRxParams[4]&POE_PORT_EVENT_TURNED_OFF_USER) | (pRxParams[4]&POE_PORT_EVENT_COUNTERS_RELATED) | (pRxParams[4]&POE_PORT_EVENT_OPEN) | (pRxParams[4]&POE_PORT_EVENT_FAULT);
				INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_EVENTS_INTERRUPT_MASK_REP,sizeof(*pGetIntMask));


			}
			break;


 			case POE_CODE_GET_INDIVIDUAL_MASK:
 			{

				P_POE_GET_INDIVIDUAL_MASK_REP pGetIndMask = (P_POE_GET_INDIVIDUAL_MASK_REP)pmsg->data;
				pGetIndMask->enable =(pRxParams[2]&1 ? TRUE :FALSE);
				INIT_BASE_MSG(pmsg,POE_CODE_GET_INDIVIDUAL_MASK_REP,sizeof(*pGetIndMask));


			}
			break;

		/*	case POE_CODE_GET_SYSOK_LED_MASK_REG:
			{
				P_POE_GET_SYSOK_LED_MASK_REG_REP pGetSysOKMask =  (P_POE_GET_SYSOK_LED_MASK_REG_REP)pmsg->data;
				pGetSysOKMask->blinkRegister = ((pRxParams[2]&0x00FF)<<8)|(pRxParams[3]&0x00FF);
				pGetSysOKMask->lightRegister = ((pRxParams[4]&0x00FF)<<8)|(pRxParams[5]&0x00FF);
				INIT_BASE_MSG(pmsg,POE_CODE_GET_SYSOK_LED_MASK_REG_REP,sizeof(*pGetSysOKMask));

			}
			break;*/

			case POE_CODE_GET_BT_POWER_IND_LED:
			{
				P_POE_GET_BT_POWER_IND_LED_REP pGetPowerIndLed = (P_POE_GET_BT_POWER_IND_LED_REP)pmsg->data;
				pGetPowerIndLed->indicationOnPerc = pRxParams[2];
				pGetPowerIndLed->indicationOffPerc = pRxParams[3];
				pGetPowerIndLed->indicationOnVal = pRxParams[4];
				pGetPowerIndLed->indicationOffVal = pRxParams[5];
				if(pGetPowerIndLed->indicationOnPerc > POE_MAX_PERC_BT_POWER_INDICATION)
					pGetPowerIndLed->indicationOnPerc = POE_MAX_PERC_BT_POWER_INDICATION;

				if(pGetPowerIndLed->indicationOnPerc < POE_MIN_PERC_BT_POWER_INDICATION)
					pGetPowerIndLed->indicationOnPerc = POE_MIN_PERC_BT_POWER_INDICATION;

				if(pGetPowerIndLed->indicationOffPerc < POE_MIN_PERC_BT_POWER_INDICATION)
					pGetPowerIndLed->indicationOffPerc = POE_MIN_PERC_BT_POWER_INDICATION;

				if(pGetPowerIndLed->indicationOffPerc > POE_MAX_PERC_BT_POWER_INDICATION)
					pGetPowerIndLed->indicationOffPerc = POE_MAX_PERC_BT_POWER_INDICATION;
				if(pGetPowerIndLed->indicationOnPerc < pGetPowerIndLed->indicationOffPerc)
				{
					printf("indicationOnPercentage is bigger than indicationOffPercentage \n");
					return PoE_RC_ERR_BASE_MSG_MISMATCH;
				}
				if(pGetPowerIndLed->indicationOnVal > pGetPowerIndLed->indicationOffVal)
				{
					printf("indicationOnVal is bigger than indicationOffVal \n");
					return PoE_RC_ERR_BASE_MSG_MISMATCH;
				}


				INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_POWER_IND_LED_REP,sizeof(*pGetPowerIndLed));

			}
			break;


			case POE_CODE_GET_BT_CLASS_POWER:
			{

				P_POE_GET_BT_CLASS_PWR_REP pGetClassPwr = (P_POE_GET_BT_CLASS_PWR_REP)pmsg->data;

				pGetClassPwr->classPwr = ((pRxParams[2]&0x00FF)<<8)|(pRxParams[3]&0x00FF);
				pGetClassPwr->addedClassPwrval = pRxParams[4];
				pGetClassPwr->maxAddedClassPwr = pRxParams[5];

				INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_CLASS_POWER_REP,sizeof(*pGetClassPwr));

			}
			break;

			case  POE_CODE_GET_SW_VERSION:
			{

				P_POE_GET_SW_VERSION_REP pGetSWVer = (P_POE_GET_SW_VERSION_REP)pmsg->data;
				UINT16 versionNum;

				pGetSWVer->hwVersion = pRxParams[2];
				pGetSWVer->producNum = pRxParams[4];
				versionNum = ((pRxParams[5]&0x00FF)<<8)|(pRxParams[6]&0x00FF);
				pGetSWVer->swPatchVerNum = (UINT8)(versionNum % 10);
				pGetSWVer->swMinorVerNum = (UINT8) ((versionNum /10)%10);
				pGetSWVer->swMajorNum = (UINT8)  ((versionNum /100)%100);
				if(pRxParams[7] >= POE_SW_VER_PARAMS_CUST)
					pGetSWVer->paramsCodeNum = POE_SW_VER_PARAMS_CUST;
				else
					pGetSWVer->paramsCodeNum = (POE_SW_VER_PARAMS_CODE)pRxParams[7];

				pGetSWVer->buildNum = pRxParams[8];

				pGetSWVer->internalSWNum = ((pRxParams[9]&0x00FF)<<8)|(pRxParams[10]&0x00FF);

				INIT_BASE_MSG(pmsg,POE_CODE_GET_SW_VERSION_REP,sizeof(*pGetSWVer));

			}
			break;
			
			default:
				rc = PoE_RC_ERR_SUBJECT_BYTES; 
				break;
				
				
		}
		return rc;
}



PoE_RC_e POECommUpdateDevReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{


	PoE_RC_e rc = PoE_RC_SUCSESS;
		
	unsigned short codeSend = pmsg->code; 

	memset(pmsg,0,POE_MESSAGE_SIZE);/* reply structure size is up to 15 byte*/
	
	switch (codeSend) 
	{

		case POE_CODE_GET_BT_DEV_STATUS:
		{

			P_POE_GET_BT_DEV_STATUS_REP pGetDevSt = (P_POE_GET_BT_DEV_STATUS_REP)pmsg->data;

			if(pRxParams[2] >0x0B)
			{
				printf("chipSetNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			pGetDevSt->chipSetNum = pRxParams[2];

			pGetDevSt->isNegateTemp = FALSE;
			pGetDevSt->devVersion = ((pRxParams[3]&0x00FF)<<8)|(pRxParams[4]&0x00FF);
			pGetDevSt->devStatus = (POE_DEVICE_STATUS)pRxParams[5];
			pGetDevSt->devEvent = (pRxParams[6]&POE_DEVICE_EVENT_SAFE_TEMPERTURE)|(pRxParams[6]&POE_DEVICE_EVENT_USER_DEF_TEMPERTURE)|(pRxParams[6]&POE_DEVICE_EVENT_FAULT);
			pGetDevSt->devFound = (POE_DEVICE_FOUND)pRxParams[7]&0x0F;
			pGetDevSt->mDev = (pRxParams[7]& POE_DEVICE_FOUND_M) ? TRUE : FALSE;
			pGetDevSt->internalVcalCount = pRxParams[8];
			pGetDevSt->temperature =  pRxParams[9];
			pGetDevSt->tshVal = pRxParams[10];
			pGetDevSt->resetCount = (pRxParams[11]&0xF0)>>4;
			pGetDevSt->porCount = pRxParams[11]&0x0F;

			if((pRxParams[3] == 0xFF)&&(pRxParams[4] == 0xFF))
			{
				printf("device does not respond! \n");
				return PoE_RC_ERR_FW_ERR;
			}
			if (pGetDevSt->temperature > 0xCD){ /* if temperature is bigger than 205*/
				pGetDevSt->temperature = 256 - pGetDevSt->temperature;
				pGetDevSt->isNegateTemp = TRUE;
				printf("the temperature is negetive! \n");
			}
			if((pGetDevSt->devFound != POE_DEVICE_FOUND_INVALID)&&(pGetDevSt->devFound != POE_DEVICE_FOUND_4_PORT)&&(pGetDevSt->devFound != POE_DEVICE_FOUND_8_PORT))
			{
				printf("devFound %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_DEV_STATUS_REP,sizeof(*pGetDevSt));

		}
		break;

		case POE_CODE_GET_DEV_REGISTER:
		{

			P_POE_GET_DEV_REGISTER_REP pGetDevReg = (P_POE_GET_DEV_REGISTER_REP)pmsg->data;
			pGetDevReg->regData = ((pRxParams[2]&0x00FF)<<8)|(pRxParams[3]&0x00FF);
			INIT_BASE_MSG(pmsg,POE_CODE_GET_DEV_REGISTER_REP,sizeof(*pGetDevReg));

		}
		break;

		case POE_CODE_GET_DEV_ERROR_LOG:
		{

			P_POE_GET_DEV_ERROR_LOG_REP pGetDevErrorLog = (P_POE_GET_DEV_ERROR_LOG_REP)pmsg->data;
			pGetDevErrorLog->vopSeverity1 = (POE_DEVICE_SEVERITY_ERROR) (pRxParams[2] & 1) |  (pRxParams[2] & 2);
			pGetDevErrorLog->vopSeverity2 = (POE_DEVICE_SEVERITY_ERROR)((pRxParams[2]&0x0C)>>2);
			pGetDevErrorLog->vopSeverity3 = (POE_DEVICE_SEVERITY_ERROR)((pRxParams[2]&0x30)>>4);
			pGetDevErrorLog->structError = (POE_DEVICE_STRUCT_ERROR)pRxParams[3];
			if(pGetDevErrorLog->structError >0x03)
			{
				printf("structError %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			INIT_BASE_MSG(pmsg,POE_CODE_GET_DEV_ERROR_LOG_REP,sizeof(*pGetDevErrorLog));

		}
		break;

		case POE_CODE_GET_LOG_SEC_STATUS:
		{

			P_POE_GET_DEV_ERROR_LOG_SECTOR_STATUS_REP pGetDevLogSectorStatus = (P_POE_GET_DEV_ERROR_LOG_SECTOR_STATUS_REP)pmsg->data;
			pGetDevLogSectorStatus->sectorError = (POE_DEVICE_SECTOR_ERROR)pRxParams[2];
			if(pGetDevLogSectorStatus->sectorError >0x03)
			{
				printf("structError %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			pGetDevLogSectorStatus->eraseSectorCounter = ((pRxParams[3]&0x00FF)<<8)|(pRxParams[4]&0x00FF);
			INIT_BASE_MSG(pmsg,POE_CODE_GET_LOG_SEC_STATUS_REP,sizeof(*pGetDevLogSectorStatus));

		}
		break;


		default:
			rc = PoE_RC_ERR_SUBJECT_BYTES;
			break;


		
	}

	return rc;
}


PoE_RC_e POECommUpdatePortReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	PoE_RC_e rc = PoE_RC_SUCSESS;

	unsigned short codeSend = pmsg->code;
	memset(pmsg,0,POE_MESSAGE_SIZE);/* reply structure size is up to 15 byte*/

	switch (codeSend)
	{
		case POE_CODE_GET_PHY_PORT_TMP_MATRIX:
		{
			P_POE_GET_PHY_PORT_TMP_MATRIX_REP pGetPhysPortTmpMat = (P_POE_GET_PHY_PORT_TMP_MATRIX_REP)pmsg->data;
			pGetPhysPortTmpMat->physNumA = pRxParams[2];
			pGetPhysPortTmpMat->physNumB = pRxParams[3];
			if(pGetPhysPortTmpMat->physNumA > 0x5F && pGetPhysPortTmpMat->physNumA != 0xFF)
			{
				printf("physNumA %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetPhysPortTmpMat->physNumB > 0x5F && pGetPhysPortTmpMat->physNumB != 0xFF)
			{
				printf("physNumB %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_PHY_PORT_TMP_MATRIX_REP,sizeof(*pGetPhysPortTmpMat));
		}
		break;

		case POE_CODE_GET_PHY_PORT_ACT_MATRIX:
		{
			P_POE_GET_PHY_PORT_ACT_MATRIX_REP pGetPhysPortActMat = (P_POE_GET_PHY_PORT_ACT_MATRIX_REP)pmsg->data;
			pGetPhysPortActMat->physNumA = pRxParams[2];
			pGetPhysPortActMat->physNumB = pRxParams[3];
			if(pGetPhysPortActMat->physNumA > 0x5F && pGetPhysPortActMat->physNumA != 0xFF)
			{
				printf("physNumA %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pGetPhysPortActMat->physNumB > 0x5F && pGetPhysPortActMat->physNumB != 0xFF)
			{
				printf("physNumB %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_PHY_PORT_ACT_MATRIX_REP,sizeof(*pGetPhysPortActMat));
		}
		break;

		case POE_CODE_GET_PORTS_DEL_POWER_STATE:
		{
			P_POE_GET_PORTS_DEL_POWER_STATE_REP pGetPortsDelPwrState = (P_POE_GET_PORTS_DEL_POWER_STATE_REP)pmsg->data;

			pGetPortsDelPwrState->ports0_7 = pRxParams[2];
			pGetPortsDelPwrState->ports8_15 = pRxParams[3];
			pGetPortsDelPwrState->ports16_23 = pRxParams[4];
			pGetPortsDelPwrState->ports24_31 = pRxParams[5];
			pGetPortsDelPwrState->ports32_39 = pRxParams[6];
			pGetPortsDelPwrState->ports40_47 = pRxParams[7];

			INIT_BASE_MSG(pmsg,POE_CODE_GET_PORTS_DEL_POWER_STATE_REP,sizeof(*pGetPortsDelPwrState));
		}
		break;

		case POE_CODE_GET_BT_PORT_PARAMETERS:
		{
			P_POE_GET_BT_PORT_PARAMETERS_REP pGetBTPortParameters = (P_POE_GET_BT_PORT_PARAMETERS_REP)pmsg->data;
			pGetBTPortParameters->portStatus = (POE_PORT_STATUS)pRxParams[2];
			pGetBTPortParameters->portModeCFG1 = pRxParams[3];
			pGetBTPortParameters->portModeCFG2PmMode = (POE_PORT_PM_MODE)(pRxParams[4] & 0x0F);
			pGetBTPortParameters->portModeCFG2ClassErr = (POE_CLASS_ERROR_OPER_SELECT)((pRxParams[4]&0xF0)>>4);
			pGetBTPortParameters->portOperMode = (POE_PORT_OPER_MODE)pRxParams[5];
			pGetBTPortParameters->addPwrForPortMode = pRxParams[6];
			pGetBTPortParameters->priority = pRxParams[7];
			pGetBTPortParameters->other = ((pRxParams[11]&1) ?  POE_PORT_OTHER_M_DEVICE : POE_PORT_OTHER_T4_DEVICE);
			pGetBTPortParameters->MSCCUse12 = pRxParams[12];

			if(pGetBTPortParameters->portModeCFG1 > 3 && pGetBTPortParameters->portModeCFG1 != 0xF)
			{
				printf("portModeCFG1 %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if((pGetBTPortParameters->portOperMode > 3 && pGetBTPortParameters->portOperMode < 10) || (pGetBTPortParameters->portOperMode > 13 && pGetBTPortParameters->portOperMode < 20) || (pGetBTPortParameters->portOperMode > 21 && pGetBTPortParameters->portOperMode < 30) || (pGetBTPortParameters->portOperMode > 30 && pGetBTPortParameters->portOperMode != 0xFF))
			{
				printf("portOperMode %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortParameters->portModeCFG2PmMode > 2 && pGetBTPortParameters->portModeCFG2PmMode != 0xF)
			{
				printf("portModeCFG2PmMode %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortParameters->portModeCFG2ClassErr > 4 && pGetBTPortParameters->portModeCFG2ClassErr != 0xF)
			{
				printf("portModeCFG2ClassErr %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortParameters->priority > 3 && pGetBTPortParameters->priority != 0xFF)
			{
				printf("priority %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_PARAMETERS_REP,sizeof(*pGetBTPortParameters));
		}
		break;

		case POE_CODE_GET_BT_PORT_STATUS:
		{
			P_POE_GET_BT_PORT_STATUS_REP pGetBTPortStatus = (P_POE_GET_BT_PORT_STATUS_REP)pmsg->data;
			pGetBTPortStatus->portStatus = (POE_PORT_STATUS)pRxParams[2];
			pGetBTPortStatus->portModeCFG1 = (POE_PORT_MODE)(pRxParams[3]&0x0F);
			pGetBTPortStatus->assignedClassPrimary = (pRxParams[4]&0xF0)>>4;
			pGetBTPortStatus->assignedClassSecondary = pRxParams[4]&0x0F;
			pGetBTPortStatus->measuredPortPower = ((pRxParams[5]&0x00FF)<<8)|(pRxParams[6]&0x00FF);
			pGetBTPortStatus->lastShutdownErrorStatus = pRxParams[9];
			pGetBTPortStatus->portEvent = (pRxParams[10]&POE_PORT_EVENT_TURNED_ON) | (pRxParams[10]&POE_PORT_EVENT_TURNED_OFF_USER) | (pRxParams[10]&POE_PORT_EVENT_COUNTERS_RELATED) | (pRxParams[10]&POE_PORT_EVENT_OPEN) | (pRxParams[10]&POE_PORT_EVENT_FAULT);;
			pGetBTPortStatus->MSCCUse12 = pRxParams[12];


			if((pGetBTPortStatus->assignedClassPrimary > 8 && pGetBTPortStatus->assignedClassPrimary != 0xC) || pGetBTPortStatus->assignedClassPrimary == 0)
			{
				printf("assignedClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if((pGetBTPortStatus->assignedClassSecondary > 8 && pGetBTPortStatus->assignedClassSecondary != 0xC) || pGetBTPortStatus->assignedClassSecondary == 0)
			{
				printf("assignedClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortStatus->portEvent > 31)
			{
				printf("portEvent %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_STATUS_REP,sizeof(*pGetBTPortStatus));
		}
		break;

		case POE_CODE_GET_BT_PORT_COUNTERS:
		{
			P_POE_GET_BT_PORT_COUNTERS_REP pGetBTPortCounters = (P_POE_GET_BT_PORT_COUNTERS_REP)pmsg->data;
			pGetBTPortCounters->portStatus = (POE_PORT_STATUS)pRxParams[2];
			pGetBTPortCounters->UDLCount = pRxParams[3];
			pGetBTPortCounters->OVLCount = pRxParams[4];
			pGetBTPortCounters->SCCount = pRxParams[5];
			pGetBTPortCounters->invalidSignatureCount = pRxParams[6];
			pGetBTPortCounters->powerDeniedCount = pRxParams[7];
			pGetBTPortCounters->MSCCUse11 = pRxParams[11];
			pGetBTPortCounters->MSCCUse12 = pRxParams[12];

			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_COUNTERS_REP,sizeof(*pGetBTPortCounters));
		}
		break;

		case POE_CODE_GET_BT_PORT_CLASS:
		{
			P_POE_GET_BT_PORT_CLASS_REP pGetBTPortClass = (P_POE_GET_BT_PORT_CLASS_REP)pmsg->data;
			pGetBTPortClass->portStatus = (POE_PORT_STATUS)pRxParams[2];
			pGetBTPortClass->CCResult = (POE_PORT_PHY_INFO_CC_RESULT)(pRxParams[3]&0xF0)>>4;
			pGetBTPortClass->decidedPDStructure = (POE_PORT_PHY_INFO_DECIDED_PD_STRUCTURE)pRxParams[3]&0x0F;
			pGetBTPortClass->measuredClassPrimary = (pRxParams[4]&0xF0)>>4;
			pGetBTPortClass->measuredClassSecondary = pRxParams[4]&0x0F;
			pGetBTPortClass->requestedClassPrimary = (pRxParams[5]&0xF0)>>4;
			pGetBTPortClass->requestedClassSecondary = pRxParams[5]&0x0F;
			pGetBTPortClass->requestedPower = ((pRxParams[6]&0x00FF)<<8)|(pRxParams[7]&0x00FF);
			pGetBTPortClass->assignedClassPrimary = (pRxParams[8]&0xF0)>>4;
			pGetBTPortClass->assignedClassSecondary = pRxParams[8]&0x0F;
			pGetBTPortClass->assignedPower = ((pRxParams[9]&0x00FF)<<8)|(pRxParams[10]&0x00FF);
			pGetBTPortClass->autoClassMeasurement = (((pRxParams[11]&0x0F) << 8) | (pRxParams[12]&0x00FF));
			pGetBTPortClass->autoClassSupport = (POE_PORT_AUTO_CLASS_SUPPORT)((pRxParams[11]&0xF0)>>4);

			if(pGetBTPortClass->CCResult > 5)
			{
				printf("CCResult %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortClass->decidedPDStructure > 8)
			{
				printf("decidedPDStructure %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if((pGetBTPortClass->measuredClassPrimary > 8 && pGetBTPortClass->measuredClassPrimary != 0xC))
			{
				printf("measuredClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				if (pGetBTPortClass->measuredClassPrimary == 0x9)
					printf("error : class overcurrent \n");
				else if (pGetBTPortClass->measuredClassPrimary == 0xA)
					printf("error : 2fingers mismatch \n");
				else if (pGetBTPortClass->measuredClassPrimary == 0xB)
					printf("error : invalid class code \n");
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->measuredClassSecondary > 5 && pGetBTPortClass->measuredClassSecondary != 0xC))
			{
				printf("measuredClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->assignedClassPrimary > 8 && pGetBTPortClass->assignedClassPrimary != 0xC) || pGetBTPortClass->assignedClassPrimary == 0)
			{
				printf("assignedClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->assignedClassSecondary > 5 && pGetBTPortClass->assignedClassSecondary != 0xC) || pGetBTPortClass->assignedClassSecondary == 0)
			{
				printf("assignedClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->requestedClassPrimary > 8 && pGetBTPortClass->requestedClassPrimary != 0xC))
			{
				printf("requestedClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->requestedClassSecondary > 8 && pGetBTPortClass->requestedClassSecondary != 0xC))
			{
				printf("requestedClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortClass->autoClassSupport > 2))
			{
				printf("autoClassSupport %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_CLASS_REP,sizeof(*pGetBTPortClass));
		}
		break;

		case POE_CODE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE:
		{
			P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP pGetBTPortLayer2LLDPData = (P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP)pmsg->data;
			pGetBTPortLayer2LLDPData->pseAllocatedPwrSingleOrAltA = ((pRxParams[2])<<8)|(pRxParams[3]);
			pGetBTPortLayer2LLDPData->pseAllocatedPwrAltB = ((pRxParams[4])<<8)|(pRxParams[5]);
			pGetBTPortLayer2LLDPData->pseMaxPwr = ((pRxParams[6])<<8)|(pRxParams[7]);
			pGetBTPortLayer2LLDPData->assignedClassPrimary = (pRxParams[8]&0xF0)>>4;
			pGetBTPortLayer2LLDPData->assignedClassSecondary = pRxParams[8]&0x0F;
			pGetBTPortLayer2LLDPData->layer2Execution = (POE_PORT_LAYER2_EXECUTION) (pRxParams[9]&0xF0)>>4;
			pGetBTPortLayer2LLDPData->layer2Usage = (POE_PORT_LAYER2_USAGE) pRxParams[9]&0x0F;
			pGetBTPortLayer2LLDPData->ieeeBTPwrBitsExt1514 = (POE_PORT_IEEE_BT_POWER_BITS_15_14)((pRxParams[10]&0x0C)>>2);
			pGetBTPortLayer2LLDPData->ieeeBTPwrBitsExt1110 = (POE_PORT_IEEE_BT_POWER_BITS_11_10) (pRxParams[10]&0x3);
			pGetBTPortLayer2LLDPData->cableLength  = pRxParams[11]&0x0F;

			if(pGetBTPortLayer2LLDPData->pseAllocatedPwrSingleOrAltA > 0x3E7)
			{
				printf("pseAllocatedPwrSingleOrAltA %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pGetBTPortLayer2LLDPData->pseAllocatedPwrAltB > 0x1F3)
			{
				printf("pseAllocatedPwrAltB %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if((pGetBTPortLayer2LLDPData->assignedClassPrimary > 8 && pGetBTPortLayer2LLDPData->assignedClassPrimary != 0xC) || pGetBTPortLayer2LLDPData->assignedClassPrimary == 0)
			{
				printf("assignedClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortLayer2LLDPData->assignedClassSecondary > 5 && pGetBTPortLayer2LLDPData->assignedClassSecondary != 0xC) || pGetBTPortLayer2LLDPData->assignedClassSecondary == 0)
			{
				printf("assignedClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if (pRxParams[11] != 0x4E){
				if(pGetBTPortLayer2LLDPData->cableLength > 0xA)
				{
					printf("cableLength %s", POE_MESSAGE_NOT_RANGE);
					return PoE_RC_ERR_DATA_BYTES;
				}
			}
			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE_REP,sizeof(*pGetBTPortLayer2LLDPData));
		}
		break;

		case POE_CODE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST:
		{
			P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP pGetBTPortLayer2LLDPPdRequest = (P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP)pmsg->data;
			pGetBTPortLayer2LLDPPdRequest->pdRequestPowerSingle = ((pRxParams[2])<<8)|(pRxParams[3]);
			pGetBTPortLayer2LLDPPdRequest->pdRequestPowerDualA = ((pRxParams[4])<<8)|(pRxParams[5]);
			pGetBTPortLayer2LLDPPdRequest->pdRequestPowerDualB = ((pRxParams[6])<<8)|(pRxParams[7]);
			pGetBTPortLayer2LLDPPdRequest->cableLength = pRxParams[8]&0x0F;

			if(pGetBTPortLayer2LLDPPdRequest->pdRequestPowerSingle > 0xACAC)
			{
				printf("pdRequestPowerSingle %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pGetBTPortLayer2LLDPPdRequest->cableLength > 0xA)
			{
				printf("cableLength %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST_REP,sizeof(*pGetBTPortLayer2LLDPPdRequest));
			break;
		}

		case POE_CODE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB:
		{
			P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP pGetPortLayer2CDPPwrUpAltB = (P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP)pmsg->data;
			pGetPortLayer2CDPPwrUpAltB->pupCmd = pRxParams[2];

			INIT_BASE_MSG(pmsg,POE_CODE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB_REP,sizeof(*pGetPortLayer2CDPPwrUpAltB));
			break;
		}

		case POE_CODE_GET_BT_PORT_LAYER2_CDP_PSE_DATA:
		{
			P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE_REP pGetBTPortLayer2CDPPseData = (P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE_REP)pmsg->data;
			pGetBTPortLayer2CDPPseData->pseAllocatedPwr = ((pRxParams[2])<<8)|(pRxParams[3]);
			pGetBTPortLayer2CDPPseData->pdRequestedPwr = ((pRxParams[4])<<8)|(pRxParams[5]);
			pGetBTPortLayer2CDPPseData->assignedClassPrimary = (pRxParams[6]&0xF0)>>4;
			pGetBTPortLayer2CDPPseData->assignedClassSecondary = pRxParams[6]&0x0F;
			pGetBTPortLayer2CDPPseData->layer2Execution = (POE_PORT_LAYER2_EXECUTION) (pRxParams[7]&0xF0)>>4;
			pGetBTPortLayer2CDPPseData->layer2Usage = (POE_PORT_LAYER2_USAGE) pRxParams[7]&0x0F;

			if(pGetBTPortLayer2CDPPseData->pseAllocatedPwr > 0x3E7)
			{
				printf("pseAllocatedPwr %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pGetBTPortLayer2CDPPseData->pdRequestedPwr > 0x3E7)
			{
				printf("pdRequestedPwr %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if((pGetBTPortLayer2CDPPseData->assignedClassPrimary > 8 && pGetBTPortLayer2CDPPseData->assignedClassPrimary != 0xC) || pGetBTPortLayer2CDPPseData->assignedClassPrimary == 0)
			{
				printf("assignedClassPrimary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pGetBTPortLayer2CDPPseData->assignedClassSecondary > 5 && pGetBTPortLayer2CDPPseData->assignedClassSecondary != 0xC && pGetBTPortLayer2CDPPseData->assignedClassSecondary != 0xD ) || pGetBTPortLayer2CDPPseData->assignedClassSecondary == 0)
			{
				printf("assignedClassSecondary %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}


			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_LAYER2_CDP_PSE_DATA_REP,sizeof(*pGetBTPortLayer2CDPPseData));
			break;
		}

		default:
			rc = PoE_RC_ERR_SUBJECT_BYTES;
			break;

	}
	return rc;
}


PoE_RC_e POECommUpdatePowerMngReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	PoE_RC_e rc = PoE_RC_SUCSESS;

	unsigned short codeSend = pmsg->code;
	memset(pmsg,0,POE_MESSAGE_SIZE);/* reply structure size is up to 15 byte*/

	switch (codeSend)
	{
		case POE_CODE_GET_TOTAL_POWER:
		{
			P_POE_GET_TOTAL_POWER_REP pGetTotalPower = (P_POE_GET_TOTAL_POWER_REP)pmsg->data;
			pGetTotalPower->powerConsumption = (pRxParams[2]<<8)|(pRxParams[3]);
			pGetTotalPower->calcPower = (pRxParams[4]<<8)|(pRxParams[5]);
			pGetTotalPower->availablePower = (pRxParams[6]<<8)|(pRxParams[7]);
			pGetTotalPower->powerLim = ((pRxParams[8]&0x00FF)<<8)|(pRxParams[9]&0x00FF);
			pGetTotalPower->powerBank = pRxParams[10];
			pGetTotalPower->vmainVoltage = ((pRxParams[11])<<8)|(pRxParams[12]);
			if(pGetTotalPower->powerLim > POE_MAX_POWER_LIMIT)
			{
				printf("powerLim %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_TOTAL_POWER_REP,sizeof(*pGetTotalPower));
		}
		break;

		case POE_CODE_GET_POWER_BANKS:
		{
			P_POE_GET_POWER_BANKS_REP pGetPowerBanks = (P_POE_GET_POWER_BANKS_REP)pmsg->data;
			pGetPowerBanks->powerLim = (pRxParams[2]<<8)|(pRxParams[3]);
			pGetPowerBanks->maxShutdownVoltage = (pRxParams[4]<<8)|(pRxParams[5]);
			pGetPowerBanks->minShutdownVoltage = (pRxParams[6]<<8)|(pRxParams[7]);
			pGetPowerBanks->guardBand = pRxParams[8];
			pGetPowerBanks->sourceType = pRxParams[9];
			pGetPowerBanks->Rmode = pRxParams[10];


			if(pGetPowerBanks->powerLim > POE_MAX_POWER_LIMIT)
			{
				printf("powerLim %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pGetPowerBanks->maxShutdownVoltage > POE_MAX_VOLTAGE || pGetPowerBanks->minShutdownVoltage < POE_MIN_VOLTAGE)
			{
				printf("maxShutdownVoltage or minShutdownVoltage %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pGetPowerBanks->maxShutdownVoltage <= pGetPowerBanks->minShutdownVoltage + 0x1E)
			{
				printf("maxShutdownVoltage is smaller than minShutdownVoltage + 0x1E  \n");
				return PoE_RC_ERR_DATA_BYTES;
			}


			INIT_BASE_MSG(pmsg,POE_CODE_GET_POWER_BANKS_REP,sizeof(*pGetPowerBanks));
		}
		break;

		case POE_CODE_GET_POWER_SUPP_PARAMS:
		{
			P_POE_GET_POWER_SUPPLY_PARAMETERS_REP pGetPowerSuppParams = (P_POE_GET_POWER_SUPPLY_PARAMETERS_REP)pmsg->data;
			pGetPowerSuppParams->powerConsumption = (pRxParams[2]<<8)|(pRxParams[3]);
			pGetPowerSuppParams->maxShutdownVoltage = (pRxParams[4]<<8)|(pRxParams[5]);
			pGetPowerSuppParams->minShutdownVoltage = (pRxParams[6]<<8)|(pRxParams[7]);
			pGetPowerSuppParams->powerBank = pRxParams[9];
			pGetPowerSuppParams->powerLim = (pRxParams[10]<<8)|(pRxParams[11]);

			if(pGetPowerSuppParams->powerLim > POE_MAX_POWER_LIMIT)
			{
				printf("powerLim %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			if(pGetPowerSuppParams->maxShutdownVoltage <= pGetPowerSuppParams->minShutdownVoltage + 0x1E)
			{
				printf("maxShutdownVoltage is smaller than minShutdownVoltage + 0x1E  \n");
				return PoE_RC_ERR_DATA_BYTES;
			}

			INIT_BASE_MSG(pmsg,POE_CODE_GET_POWER_SUPP_PARAMS_REP,sizeof(*pGetPowerSuppParams));
		}
		break;

		case POE_CODE_GET_BT_PORT_MEAS:
		{
			P_POE_GET_BT_PORT_MEASUREMENTS_REP pGetPortMeasurments = (P_POE_GET_BT_PORT_MEASUREMENTS_REP)pmsg->data;
			pGetPortMeasurments->vmainVoltage = (pRxParams[2]<<8)|(pRxParams[3]);
			pGetPortMeasurments->calculatedCurrent = (pRxParams[4]<<8)|(pRxParams[5]);
			pGetPortMeasurments->measuredPortPower = (pRxParams[6]<<8)|(pRxParams[7]);
			pGetPortMeasurments->portVoltage = (pRxParams[9]<<8)|(pRxParams[10]);
			INIT_BASE_MSG(pmsg,POE_CODE_GET_BT_PORT_MEAS,sizeof(*pGetPortMeasurments));
		}
		break;
		default:
			rc = PoE_RC_ERR_SUBJECT_BYTES;
			break;

	}

	return rc;
}

PoE_RC_e POECommUpdateFlowchart(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	return PoE_RC_SUCSESS;
}

PoE_RC_e POECommUpdatePowerDerReply(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	return PoE_RC_SUCSESS;
}











PoE_RC_e POECommUpdateAndSend(void)
{

	int cs =0;

		CALCULATE_CHECK_SUM(cs);
		pTxParams[13] = (cs >> 8) & 0xFF; /* update checksum MBS */
		pTxParams[14] = cs & 0xFF; /* update checksum LSB */
		
		if(++echoNum == 255)
		  echoNum = 0;
	
		
		/* send message to communication device  */
		return (POECommWriteDev());


}



PoE_RC_e POECommTranslateInfraMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	switch (pmsg->code)
		{

			case POE_CODE_INIT_POE_API:
				{




				}
				break;

			case POE_CODE_SET_DOWNLOAD_FILE:
				{


				}
				break;


			default:
				return -1;


		}
		
		return 0;

}





PoE_RC_e POECommTranslateSysSMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	varIndex =0;

	

	switch (pmsg->code)
	{

		case POE_CODE_RESET_COMMAND:
			
		{


			FILL_COMMAND_GLOBAL;
			
			POECommSetVarMsg("%c%c%c%c%c",RESET_CODE,ZERO_CODE,RESET_CODE,ZERO_CODE,RESET_CODE);

			break; 

		}

		case POE_CODE_RES_FAC_DEFAULT:

		{

			FILL_PROGRAMM;

			pTxParams[varIndex] = RESTORE_FACTORY;

			break; 


		}

		 case POE_CODE_SAVE_SYS_SETTING:
		{
			FILL_PROGRAMM;
			pTxParams[varIndex++] = E2_PARAM;
			pTxParams[varIndex] = SAVE_CONFIG;

			break; 


		}

		case POE_CODE_SET_USER_BYTE_SAVE:
			
		{

			P_POE_SET_USER_BYTE_TO_SAVE pSetUserByte = (P_POE_SET_USER_BYTE_TO_SAVE) pmsg->data;


			if((pmsg->length == 0)||( pSetUserByte == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}


			if(pSetUserByte->NVMUserByte > POE_MAX_NVM_USER_BYTE)
			{
				printf("NVMUserByte %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			FILL_PROGRAMM;

			POECommSetVarMsg("%c%c",USER_BYTE,pSetUserByte->NVMUserByte);

			
			break; 


		}

		case POE_CODE_SET_PRIVATE_LABEL:
		{

			P_POE_SET_PRIVATE_LABEL pSetPrivateLabel = (P_POE_SET_PRIVATE_LABEL) pmsg->data;


			if((pmsg->length == 0)||( pSetPrivateLabel == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if((pSetPrivateLabel->RAMPrivateLabel< POE_MIN_RAM_PRIVATE_LABEL)||(pSetPrivateLabel->RAMPrivateLabel> POE_MAX_RAM_PRIVATE_LABEL))
			{
				printf("RAMPrivateLabel %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_COMMAND_GLOBAL;
			POECommSetVarMsg("%c%c",SYSTEM_STATUS,pSetPrivateLabel->RAMPrivateLabel);


			break; 


		}



		case POE_CODE_GET_BT_SYSTEM_STATUS:
		{

			FILL_REQUEST_GLOBAL;
			pTxParams[varIndex] = BT_SYSTEM_STATUS;

			break; 

		}



		case POE_CODE_GET_BT_EVENT_CAUSE:
		{

			FILL_REQUEST_GLOBAL;
			pTxParams[varIndex] = BT_EVENT;
			break; 



		}


    case POE_CODE_GET_SYSTEM_STATUS2 :
    	{

			FILL_REQUEST_GLOBAL;
			pTxParams[varIndex] = SYSTEM_STATUS2;
			break; 

			break; 



		}


    case POE_CODE_SET_BT_EVENTS_INTERRUPT_MASK:
    	{

    		P_POE_SET_BT_EVENTS_INTERRUPT_MASK pint_mask = (P_POE_SET_BT_EVENTS_INTERRUPT_MASK)pmsg->data;


			if((pmsg->length == 0)||( pint_mask == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}


			FILL_COMMAND_GLOBAL;

			POECommSetVarMsg("%c%c%c%c",BT_IRQ_MASK,pint_mask->systemEventMaskRegister, pint_mask->deviceEventMaskRegister, pint_mask->portEventMaskRegister);

    		break;
    	}

    case POE_CODE_GET_BT_EVENTS_INTERRUPT_MASK:
       	{


				FILL_REQUEST_GLOBAL;
				pTxParams[varIndex] = BT_IRQ_MASK;
				break; 
       	}


	case POE_CODE_SET_INDIVIDUAL_MASK:
			{
	
				P_POE_SET_INDIVIDUAL_MASK pind_mask = (P_POE_SET_INDIVIDUAL_MASK)pmsg->data;

				if((pmsg->length == 0)||( pind_mask == NULL))
				{
					printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
					return PoE_RC_ERR_BASE_MSG_MISMATCH;
				}


				FILL_COMMAND_GLOBAL;


				POECommSetVarMsg("%c%c%c",INDVIDUAL_MASK,pind_mask->maskNumber,pind_mask->enable);


				break;
			}
	
	case POE_CODE_GET_INDIVIDUAL_MASK:
			{
				P_POE_GET_INDIVIDUAL_MASK pind_mask = (P_POE_GET_INDIVIDUAL_MASK)pmsg->data;
	
				if((pmsg->length == 0)||( pind_mask == NULL))
				{
					printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
					return PoE_RC_ERR_BASE_MSG_MISMATCH;
				}

				FILL_REQUEST_GLOBAL;


				POECommSetVarMsg("%c%c",INDVIDUAL_MASK,pind_mask->maskNumber);

				break;
			}

/*	case POE_CODE_SET_SYSOK_LED_MASK_REG:
			{
	
				P_POE_SET_SYSOK_LED_MASK_REG psysOK_mask = (P_POE_SET_SYSOK_LED_MASK_REG)pmsg->data;
		
				if((pmsg->length == 0)||( psysOK_mask == NULL))
					return PoE_RC_ERR_BASE_MSG_MISMATCH;

				FILL_COMMAND_GLOBAL;
					
				POECommSetVarMsg("%c%w%w",SYS_OK_MASK,psysOK_mask->blinkRegister,psysOK_mask->lightRegister);

				break;
			}
		
	case POE_CODE_GET_SYSOK_LED_MASK_REG:
			{
				
	
				FILL_REQUEST_GLOBAL;
				pTxParams[varIndex] = SYS_OK_MASK;
				break;
			}*/


	case POE_CODE_SET_BT_POWER_IND_LED:

	{
	
		P_POE_SET_BT_POWER_IND_LED ppowerInd_led = (P_POE_SET_BT_POWER_IND_LED)pmsg->data;
	

		if((pmsg->length == 0)||( ppowerInd_led == NULL))
		{
			printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
			return PoE_RC_ERR_BASE_MSG_MISMATCH;
		}
		if(ppowerInd_led->indicationOnPercOrVal < POE_MIN_PERC_BT_POWER_INDICATION)
			ppowerInd_led->indicationOnPercOrVal = POE_MIN_PERC_BT_POWER_INDICATION;

		if(ppowerInd_led->indicationOnPercOrVal > POE_MAX_PERC_BT_POWER_INDICATION)
			ppowerInd_led->indicationOnPercOrVal = POE_MAX_PERC_BT_POWER_INDICATION;

		if(ppowerInd_led->indicationOffPercOrVal < POE_MIN_PERC_BT_POWER_INDICATION)
			ppowerInd_led->indicationOffPercOrVal = POE_MIN_PERC_BT_POWER_INDICATION;

		if(ppowerInd_led->indicationOffPercOrVal > POE_MAX_PERC_BT_POWER_INDICATION)
			ppowerInd_led->indicationOffPercOrVal = POE_MAX_PERC_BT_POWER_INDICATION;

		if (ppowerInd_led->indicationType == 0){ /* Percentage use */

			if(ppowerInd_led->indicationOnPercOrVal < ppowerInd_led->indicationOffPercOrVal)
			{
				printf("indicationOnPercentage is bigger than indicationOffPercentage \n");
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}
		}

		else if (ppowerInd_led->indicationType == 1){   /* Watts use*/
			if (ppowerInd_led->indicationOnPercOrVal > ppowerInd_led->indicationOffPercOrVal)
			{
				printf("indicationOnVal is bigger than indicationOffVal \n");
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}
		}
		else
		{
			printf("indicationType %s", POE_MESSAGE_NOT_RANGE);
			return PoE_RC_ERR_SUBJECT_BYTES;
		}

		FILL_COMMAND_GLOBAL;

		POECommSetVarMsg("%c%c%c%c",BT_POWER_INDICATION,ppowerInd_led->indicationType,ppowerInd_led->indicationOnPercOrVal,ppowerInd_led->indicationOffPercOrVal);

		break;
	}


	case POE_CODE_GET_BT_POWER_IND_LED:
	{

						
		FILL_REQUEST_GLOBAL;
		pTxParams[varIndex] = BT_POWER_INDICATION;
		break;
			
	}


	case POE_CODE_SET_BT_CLASS_ADD_POWER:
	{

		P_POE_SET_ADD_CLASS_PWR pclass_add_pwr = (P_POE_SET_ADD_CLASS_PWR)pmsg->data;

		if((pmsg->length == 0)||( pclass_add_pwr == NULL))
		{
			printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
			return PoE_RC_ERR_BASE_MSG_MISMATCH;
		}
		if ((pclass_add_pwr->classType >= POE_CLASS_TYPE_MAX)||(pclass_add_pwr->classType == 0))
		{
			printf("classType %s", POE_MESSAGE_NOT_RANGE);
			return PoE_RC_ERR_SUBJECT_BYTES;
		}
		FILL_COMMAND_GLOBAL;


		POECommSetVarMsg("%c%c%c",BT_CLASS_PWR,pclass_add_pwr->classType,pclass_add_pwr->addedClassPwrval);

	}



	case POE_CODE_GET_BT_CLASS_POWER:
	{
		P_POE_GET_BT_CLASS_PWR pclass_pwr = (P_POE_GET_BT_CLASS_PWR)pmsg->data;

		if((pmsg->length == 0)||( pclass_pwr == NULL))
		{
			printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
			return PoE_RC_ERR_BASE_MSG_MISMATCH;
		}


		FILL_REQUEST_GLOBAL;

		if ((pclass_pwr->classType >= POE_CLASS_TYPE_MAX)||(pclass_pwr->classType == 0))
		{
			printf("classType %s", POE_MESSAGE_NOT_RANGE);
			return PoE_RC_ERR_SUBJECT_BYTES;
		}
		
		POECommSetVarMsg("%c%c",BT_CLASS_PWR,pclass_pwr->classType);

		break;


	}


	case POE_CODE_GET_SW_VERSION:
	{
		FILL_REQUEST_GLOBAL;
		pTxParams[varIndex++] = VERSION_Z;
		pTxParams[varIndex] = SW_VERSION;

		break;
	
	
	}


	case POE_CODE_SET_LOG_SEC_CLEAR_STAMP:

	{
		
		FILL_PROGRAMM;
		pTxParams[varIndex++] = POE_GLOBAL_SUB;
		pTxParams[varIndex] = CLEAR_LOG;

		break;
	

	}



	default:
	 	return PoE_RC_ERR_BASE_MSG_MISMATCH;



	}

	return PoE_RC_SUCSESS;


}



PoE_RC_e POECommTranslateDevMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	varIndex =0;
	
	
	switch (pmsg->code)
	
	{
		case POE_CODE_SET_DEV_PARAMETERS:
		{
			P_POE_SET_DEV_PARAMETERS pDevParams = (P_POE_SET_DEV_PARAMETERS)pmsg->data;

			
			if((pmsg->length == 0)||( pDevParams == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pDevParams->chipSetNum > 0x0B)
			{
				printf("chipSetNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pDevParams->tshVal > 150)
			{
				printf("chipSetNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			FILL_COMMAND_GLOBAL;

			POECommSetVarMsg("%c%c%c%c",DEVICE_PARAMS,pDevParams->chipSetNum, 0x4E, pDevParams->tshVal);

			break;

		}

		case POE_CODE_GET_BT_DEV_STATUS:
		{
			P_POE_GET_BT_DEV_STATUS pDevStatus = (P_POE_GET_BT_DEV_STATUS)pmsg->data;

			if((pmsg->length == 0)||( pDevStatus == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}


			if(pDevStatus->chipSetNum > 0x0B)
			{
				printf("chipSetNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}


			FILL_REQUEST_GLOBAL;

			POECommSetVarMsg("%c%c",BT_DEV_INFO,pDevStatus->chipSetNum);


			break;

		}	
		case POE_CODE_SET_DEV_REGISTER:
		{
			P_POE_SET_DEV_REGISTER pDevRegister = (P_POE_SET_DEV_REGISTER)pmsg->data;

			if((pmsg->length == 0)||( pDevRegister == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}


			if(pDevRegister->devNum > 0x0B)
			{
				printf("devNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}


			FILL_TEST;

			POECommSetVarMsg("%c%w%w%c",WRITE_ASIC_REG,pDevRegister->regAddress,pDevRegister->regData,pDevRegister->devNum);


			break;

		}
		case POE_CODE_GET_DEV_REGISTER:
		{
			P_POE_GET_DEV_REGISTER pDevRegister = (P_POE_GET_DEV_REGISTER)pmsg->data;

			if((pmsg->length == 0)||( pDevRegister == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;

			}


			if(pDevRegister->devNum > 0x0B)
			{
				printf("devNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_TEST;

			POECommSetVarMsg("%c%w%c",READ_ASIC_REG,pDevRegister->regAddress,pDevRegister->devNum);


			break;
		}
		case POE_CODE_GET_DEV_ERROR_LOG:
		{
			P_POE_GET_DEV_ERROR_LOG pDevErrorLog = (P_POE_GET_DEV_ERROR_LOG)pmsg->data;

			if((pmsg->length == 0)||( pDevErrorLog == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}



			FILL_REQUEST_GLOBAL;

			POECommSetVarMsg("%c%c",DEVICE_LOG, pDevErrorLog->csNum);


			break;
		}

		case POE_CODE_GET_LOG_SEC_STATUS:
		{

			FILL_REQUEST_GLOBAL;

			POECommSetVarMsg("%c",LOG_SECTOR_STATUS);


			break;
		}

		default:
			return PoE_RC_ERR_BASE_MSG_MISMATCH;

	}
	return PoE_RC_SUCSESS;

}


PoE_RC_e POECommTranslatePortMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	varIndex =0;


	switch (pmsg->code)

	{
		case POE_CODE_SET_TEMP_MATRIX:
		{
			P_POE_SET_TEMP_MAT pSetTempMat = (P_POE_SET_TEMP_MAT)pmsg->data;


			if((pmsg->length == 0)||( pSetTempMat == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetTempMat->physNumA > 0x5F && pSetTempMat->physNumA != 0xFF)
			{
				printf("physNumA %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pSetTempMat->physNumB > 0x5F && pSetTempMat->physNumB != 0xFF)
			{
				printf("physNumB %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			FILL_COMMAND_CHANNEL;

			POECommSetVarMsg("%c%c%c%c", TMP_MAT, pSetTempMat->CHNum, pSetTempMat->physNumA, pSetTempMat->physNumB);

			break;

		}

		case POE_CODE_GET_PHY_PORT_TMP_MATRIX:
		{
			P_POE_GET_PHY_PORT_TMP_MATRIX pGetPhysPortTmpMat = (P_POE_GET_PHY_PORT_TMP_MATRIX)pmsg->data;


			if((pmsg->length == 0)||( pGetPhysPortTmpMat == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", TMP_MAT, pGetPhysPortTmpMat->CHNum);

			break;

		}

		case POE_CODE_PROGRAM_GLOBAL_MATRIX:
		{

			FILL_COMMAND_GLOBAL;

			POECommSetVarMsg("%c", TMP_MAT);

			break;

		}

		case POE_CODE_GET_PHY_PORT_ACT_MATRIX:
		{
			P_POE_GET_PHY_PORT_ACT_MATRIX pGetPhysPortActMat = (P_POE_GET_PHY_PORT_ACT_MATRIX)pmsg->data;


			if((pmsg->length == 0)||( pGetPhysPortActMat == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if( pGetPhysPortActMat->CHNum > 0x2F)
			{
				printf("CHNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", CHANNEL_MAT, pGetPhysPortActMat->CHNum);

			break;

		}

		case POE_CODE_GET_PORTS_DEL_POWER_STATE:
		{

			FILL_REQUEST_GLOBAL;

			POECommSetVarMsg("%c", ALL_PORTS_DELIVERING);

			break;

		}

		case POE_CODE_SET_BT_PORT_PARAMETERS:
		{
			P_POE_SET_BT_PORT_PARAMETERS pSetBTPortParmeters = (P_POE_SET_BT_PORT_PARAMETERS)pmsg->data;

			if((pmsg->length == 0)||( pSetBTPortParmeters == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetBTPortParmeters->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			if(pSetBTPortParmeters->portModeCFG1 > 3 && pSetBTPortParmeters->portModeCFG1 != 0xF)
			{
				printf("portModeCFG1 %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pSetBTPortParmeters->portModeCFG2PmMode > 2 && pSetBTPortParmeters->portModeCFG2PmMode != 0xF)
			{
				printf("portModeCFG2PmMode %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pSetBTPortParmeters->portModeCFG2ClassErr > 4 && pSetBTPortParmeters->portModeCFG2ClassErr != 0xF)
			{
				printf("portModeCFG2ClassErr %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if((pSetBTPortParmeters->portOperMode > 0x3 && pSetBTPortParmeters->portOperMode < 0x10) || (pSetBTPortParmeters->portOperMode > 0x15 && pSetBTPortParmeters->portOperMode < 0x20) || (pSetBTPortParmeters->portOperMode > 0x24 && pSetBTPortParmeters->portOperMode < 0x30) || (pSetBTPortParmeters->portOperMode > 0x30 && pSetBTPortParmeters->portOperMode != 0xFF))
			{
				printf("portOperMode %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pSetBTPortParmeters->priority > 3 && pSetBTPortParmeters->priority != 0xFF)
			{
				printf("priority %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			FILL_COMMAND_CHANNEL;
			POECommSetVarMsg("%c%c%c%c%c%c%c", BT_PORT_CONFIG1, pSetBTPortParmeters->portNum, pSetBTPortParmeters->portModeCFG1, (pSetBTPortParmeters->portModeCFG2PmMode | (pSetBTPortParmeters->portModeCFG2ClassErr<<4)),pSetBTPortParmeters->portOperMode, pSetBTPortParmeters->addPwrForPortMode, pSetBTPortParmeters->priority);

			break;

		}

		case POE_CODE_GET_BT_PORT_PARAMETERS:
		{
			P_POE_GET_BT_PORT_PARAMETERS pGetBTPortParmeters = (P_POE_GET_BT_PORT_PARAMETERS)pmsg->data;


			if((pmsg->length == 0)||( pGetBTPortParmeters == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortParmeters->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", BT_PORT_CONFIG1, pGetBTPortParmeters->portNum);

			break;

		}

		case POE_CODE_GET_BT_PORT_STATUS:
		{
			P_POE_GET_BT_PORT_STATUS pGetBTPortStatus = (P_POE_GET_BT_PORT_STATUS)pmsg->data;


			if((pmsg->length == 0)||( pGetBTPortStatus == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortStatus->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}
			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", BT_PORT_STATUS, pGetBTPortStatus->portNum);

			break;

		}

		case POE_CODE_GET_BT_PORT_COUNTERS:
		{
			P_POE_GET_BT_PORT_COUNTERS pGetBTPortCounters = (P_POE_GET_BT_PORT_COUNTERS)pmsg->data;


			if((pmsg->length == 0)||(pGetBTPortCounters == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}
			if(pGetBTPortCounters->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", BT_PORT_COUNTERS, pGetBTPortCounters->portNum);

			break;

		}

		case POE_CODE_GET_BT_PORT_CLASS:
		{
			P_POE_GET_BT_PORT_CLASS pGetBTPortClass = (P_POE_GET_BT_PORT_CLASS)pmsg->data;


			if((pmsg->length == 0)||(pGetBTPortClass == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortClass->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}


			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", BT_PORT_CLASS, pGetBTPortClass->portNum);

			break;

		}

		case POE_CODE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST:
		{
			P_POE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST pSetBTPortLayer2LLDPPdRequest = (P_POE_SET_BT_PORT_LAYER2_LLDP_PD_REQUEST)pmsg->data;


			if((pmsg->length == 0)||( pSetBTPortLayer2LLDPPdRequest == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetBTPortLayer2LLDPPdRequest->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			if(pSetBTPortLayer2LLDPPdRequest->cableLength > 0xA)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			FILL_COMMAND_CHANNEL;

			POECommSetVarMsg("%c%c%w%w%w%c", LAYER2_BT_LLDP_PD, pSetBTPortLayer2LLDPPdRequest->portNum, pSetBTPortLayer2LLDPPdRequest->pdRequestPowerSingle,  pSetBTPortLayer2LLDPPdRequest->pdRequestPowerDualA,  pSetBTPortLayer2LLDPPdRequest->pdRequestPowerDualB, pSetBTPortLayer2LLDPPdRequest->autoClassOperation, pSetBTPortLayer2LLDPPdRequest->cableLength);

			break;
		}

		case POE_CODE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE:
		{
			P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE pGetBTPortLayer2LLDPData = (P_POE_GET_BT_PORT_LAYER2_LLDP_PSE_DATE)pmsg->data;


			if((pmsg->length == 0)||( pGetBTPortLayer2LLDPData == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortLayer2LLDPData->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", LAYER2_BT_LLDP_PSE, pGetBTPortLayer2LLDPData->portNum);

			break;
		}

		case POE_CODE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST:
		{
			P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST pGetBTPortLayer2LLDPPdRequest = (P_POE_GET_BT_PORT_LAYER2_LLDP_PD_REQUEST)pmsg->data;


			if((pmsg->length == 0)||( pGetBTPortLayer2LLDPPdRequest == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortLayer2LLDPPdRequest->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", LAYER2_BT_LLDP_PD, pGetBTPortLayer2LLDPPdRequest->portNum);

			break;
		}

		case POE_CODE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB:
		{
			P_POE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB pSetPortLayer2CPdPwrUpAltB = (P_POE_SET_PORT_LAYER2_CDP_POWER_UP_ALTB)pmsg->data;


			if((pmsg->length == 0)||( pSetPortLayer2CPdPwrUpAltB == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetPortLayer2CPdPwrUpAltB->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_COMMAND_CHANNEL;

			POECommSetVarMsg("%c%c%c", LAYER2_CDP_PWR_UP_ALTB, pSetPortLayer2CPdPwrUpAltB->portNum, pSetPortLayer2CPdPwrUpAltB->pupCmd);

			break;
		}

		case POE_CODE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB:
		{
			P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB pGetPortLayer2CdpPwrUpAltB = (P_POE_GET_PORT_LAYER2_CDP_POWER_UP_ALTB)pmsg->data;


			if((pmsg->length == 0)||( pGetPortLayer2CdpPwrUpAltB == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetPortLayer2CdpPwrUpAltB->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", LAYER2_CDP_PWR_UP_ALTB, pGetPortLayer2CdpPwrUpAltB->portNum);

			break;
		}
		case POE_CODE_SET_PORT_LAYER2_CDP_PD_REQUEST:
		{
			P_POE_SET_PORT_LAYER2_CDP_PD_REQUEST pSetPortLayer2CdpPdRequest = (P_POE_SET_PORT_LAYER2_CDP_PD_REQUEST)pmsg->data;


			if((pmsg->length == 0)||( pSetPortLayer2CdpPdRequest == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetPortLayer2CdpPdRequest->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			pSetPortLayer2CdpPdRequest->pw4 = (pSetPortLayer2CdpPdRequest->pw4&1) | (pSetPortLayer2CdpPdRequest->pw4&2);
			FILL_COMMAND_CHANNEL;

			POECommSetVarMsg("%c%c%c%c%c%c%c%c%c%c", LAYER2_CDP_PD_REQUEST, pSetPortLayer2CdpPdRequest->portNum, pSetPortLayer2CdpPdRequest->reserved, pSetPortLayer2CdpPdRequest->pdRequestPwr0, pSetPortLayer2CdpPdRequest->pdRequestPwr1, pSetPortLayer2CdpPdRequest->pdRequestPwr2, pSetPortLayer2CdpPdRequest->pdRequestPwr3, pSetPortLayer2CdpPdRequest->pdRequestPwr4, pSetPortLayer2CdpPdRequest->pw4, pSetPortLayer2CdpPdRequest->pw03);
			break;
		}

		case POE_CODE_GET_BT_PORT_LAYER2_CDP_PSE_DATA:
		{
			P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE pGetBTPortLayer2CdpPseData = (P_POE_GET_BT_PORT_LAYER2_CDP_PSE_DATE)pmsg->data;


			if((pmsg->length == 0)||( pGetBTPortLayer2CdpPseData == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetBTPortLayer2CdpPseData->portNum > 0x2F)
			{
				printf("portNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			FILL_REQUEST_CHANNEL;

			POECommSetVarMsg("%c%c", LAYER2_CDP_PSE, pGetBTPortLayer2CdpPseData->portNum);

			break;
		}

		default:
			return PoE_RC_ERR_BASE_MSG_MISMATCH;

	}
	return PoE_RC_SUCSESS;
}

PoE_RC_e POECommTranslatePowerMngMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	varIndex =0;


	switch (pmsg->code)

	{
		case POE_CODE_GET_TOTAL_POWER:
		{
			FILL_REQUEST_GLOBAL;
			pTxParams[varIndex++] = SUPPLY;
			pTxParams[varIndex] = TOTAL_POWER;

			break;
		}

		case POE_CODE_SET_POWER_BANKS:
		{
			P_POE_SET_POWER_BANKS pSetPowerBanks = (P_POE_SET_POWER_BANKS)pmsg->data;

			if((pmsg->length == 0)||( pSetPowerBanks == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pSetPowerBanks->bank > POE_MAX_POWER_BANK)
			{
				printf("bank %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			if(pSetPowerBanks->powerLim > POE_MAX_POWER_LIMIT)
			{
				printf("powerLim %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			if(pSetPowerBanks->maxShutdownVoltage > POE_MAX_VOLTAGE || pSetPowerBanks->minShutdownVoltage < POE_MIN_VOLTAGE)
			{
				printf("maxShutdownVoltage or minShutdownVoltage %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}
			if(pSetPowerBanks->maxShutdownVoltage <= pSetPowerBanks->minShutdownVoltage + 0x1E)
			{
				printf("maxShutdownVoltage is smaller than minShutdownVoltage +0x1E  \n");
				return PoE_RC_ERR_DATA_BYTES;
			}


			FILL_COMMAND_GLOBAL;

			POECommSetVarMsg("%c%c%c%w%w%w%c", SUPPLY, POWER_BUDGET, pSetPowerBanks->bank, pSetPowerBanks->powerLim, pSetPowerBanks->maxShutdownVoltage, pSetPowerBanks->minShutdownVoltage, pSetPowerBanks->guardBand);

			break;
		}

		case POE_CODE_GET_POWER_BANKS:
		{
			P_POE_GET_POWER_BANKS pGetPowerBanks = (P_POE_GET_POWER_BANKS)pmsg->data;

			if((pmsg->length == 0)||( pGetPowerBanks == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if(pGetPowerBanks->bank > POE_MAX_POWER_BANK)
			{
				printf("bank %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_DATA_BYTES;
			}

			FILL_REQUEST_GLOBAL;

			POECommSetVarMsg("%c%c%c", SUPPLY, POWER_BUDGET, pGetPowerBanks->bank);

			break;
		}

		case POE_CODE_GET_POWER_SUPP_PARAMS:
		{
			FILL_REQUEST_GLOBAL;
			pTxParams[varIndex++] = SUPPLY;
			pTxParams[varIndex] = MAIN;

			break;
		}

		case POE_CODE_GET_BT_PORT_MEAS:
		{
			P_POE_GET_BT_PORT_MEASUREMENTS pGetBTPortMeas = (P_POE_GET_BT_PORT_MEASUREMENTS)pmsg->data;

			if((pmsg->length == 0)||( pGetBTPortMeas == NULL))
			{
				printf(POE_MESSAGE_WRONG_PARAMETERS_FROM_USER);
				return PoE_RC_ERR_BASE_MSG_MISMATCH;
			}

			if( pGetBTPortMeas->CHNum > 0x2F)
			{
				printf("CHNum %s", POE_MESSAGE_NOT_RANGE);
				return PoE_RC_ERR_SUBJECT_BYTES;
			}

			POECommSetVarMsg("%c%c%c%c%c",POE_REQUEST_KEY, echoNum, CHANNEL, BT_PORT_MEASE, pGetBTPortMeas->CHNum);

			break;
		}

		default:
			return PoE_RC_ERR_BASE_MSG_MISMATCH;

	}
	return PoE_RC_SUCSESS;

}

PoE_RC_e getFlow(P_POE_GET_FLOWCHART pGetFlowchart)
{
	P_POE_BASE_MESSAGE newPbmsg;
	PoE_RC_e rc;
	char buffer[POE_MESSAGE_SIZE*4];
	int i,count = 1;

	newPbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* Get BT Event Cause*/
	INIT_BASE_MSG(newPbmsg,POE_CODE_GET_BT_EVENT_CAUSE,0);
	rc = POEAPITranslateMessage(newPbmsg);
	P_POE_GET_BT_EVENT_CAUSE_REP getBTEventCause = (P_POE_GET_BT_EVENT_CAUSE_REP)newPbmsg->data;

	/* to know if there is an event in the port*/
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent0_7, 0);
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent8_15, 8);
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent16_23, 16);
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent24_31, 24);
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent32_39, 32);
	CALCULATE_PORTS_EVENT(getBTEventCause->portCauseEvent40_47, 40);

	/* to know if there is an event in the device*/
	CALCULATE_DEVICES_EVENT(getBTEventCause->deviceEvent);

	pGetFlowchart->systemOKRegVmain = getBTEventCause->systemOKRegVmain;
	pGetFlowchart->systemOKRegPwrPrec = getBTEventCause->systemOKRegPwrPrec;
	pGetFlowchart->systemOKRegPwrVal = getBTEventCause->systemOKRegPwrVal;

	/* device actions*/
	if (getBTEventCause->deviceEvent & 0xFFF){
		for (i=0; i<12; i++){
			if (pGetFlowchart->deviceListBool[i]){

				/* Get Device Status*/
				P_POE_GET_BT_DEV_STATUS getDevStatus = (P_POE_GET_BT_DEV_STATUS) newPbmsg->data;
				getDevStatus->chipSetNum = i;
				INIT_BASE_MSG(newPbmsg, POE_CODE_GET_BT_DEV_STATUS,sizeof(*getDevStatus));
				rc = POEAPITranslateMessage(newPbmsg);

				P_POE_GET_BT_DEV_STATUS_REP getDevStatusRep = (P_POE_GET_BT_DEV_STATUS_REP)newPbmsg->data;


				pGetFlowchart->deviceList[i].devStatus = getDevStatusRep->devStatus;
				pGetFlowchart->deviceList[i].devEvent = getDevStatusRep->devEvent;
				pGetFlowchart->deviceList[i].temperature = getDevStatusRep->temperature;
				pGetFlowchart->deviceList[i].isNegateTemp = getDevStatusRep->isNegateTemp;
				pGetFlowchart->deviceList[i].resetCount = getDevStatusRep->resetCount;
				pGetFlowchart->deviceList[i].porCount = getDevStatusRep->porCount;
			}
		}

	}

	/* actions of ports with event*/
	for (i=0; i<48; i++){
		if (pGetFlowchart->portsStatusListBool[i]){
			/* Get BT Port Status*/
			P_POE_GET_BT_PORT_STATUS getBTPortStatus = (P_POE_GET_BT_PORT_STATUS) newPbmsg->data;
			getBTPortStatus->portNum = i;
			INIT_BASE_MSG(newPbmsg, POE_CODE_GET_BT_PORT_STATUS,sizeof(*getBTPortStatus));
			rc = POEAPITranslateMessage(newPbmsg);
			P_POE_GET_BT_PORT_STATUS_REP getBTPortStatusRep = (P_POE_GET_BT_PORT_STATUS_REP)newPbmsg->data;

			pGetFlowchart->portsStatusList[i].portStatus = getBTPortStatusRep->portStatus;
			pGetFlowchart->portsStatusList[i].assignedClassPrimary = getBTPortStatusRep->assignedClassPrimary;
			pGetFlowchart->portsStatusList[i].assignedClassSecondary = getBTPortStatusRep->assignedClassSecondary;
			pGetFlowchart->portsStatusList[i].measuredPortPower = getBTPortStatusRep->measuredPortPower;
			pGetFlowchart->portsStatusList[i].portEvent = getBTPortStatusRep->portEvent;


			if ((getBTPortStatusRep->portStatus >= 0x80 || getBTPortStatusRep->portStatus < 0x92) && (getBTPortStatusRep->portEvent&1)){

				pGetFlowchart->portsClassListBool[i] = TRUE;

				/* Get BT Port Class*/
				P_POE_GET_BT_PORT_CLASS getBTPortClass = (P_POE_GET_BT_PORT_CLASS) newPbmsg->data;
				getBTPortClass->portNum = i;
				INIT_BASE_MSG(newPbmsg, POE_CODE_GET_BT_PORT_CLASS,sizeof(*getBTPortClass));
				rc = POEAPITranslateMessage(newPbmsg);
				P_POE_GET_BT_PORT_CLASS_REP getBTPortClassRep = (P_POE_GET_BT_PORT_CLASS_REP)newPbmsg->data;

				pGetFlowchart->portsClassList[i].CCResult = getBTPortClassRep->CCResult;
				pGetFlowchart->portsClassList[i].decidedPDStructure = getBTPortClassRep->decidedPDStructure;
				pGetFlowchart->portsClassList[i].measuredClassPrimary = getBTPortClassRep->measuredClassPrimary;
				pGetFlowchart->portsClassList[i].measuredClassSecondary = getBTPortClassRep->measuredClassSecondary;
				pGetFlowchart->portsClassList[i].requestedClassPrimary = getBTPortClassRep->requestedClassPrimary;
				pGetFlowchart->portsClassList[i].requestedClassSecondary = getBTPortClassRep->requestedClassSecondary;
				pGetFlowchart->portsClassList[i].requestedPower = getBTPortClassRep->requestedPower;
				pGetFlowchart->portsClassList[i].assignedPower = getBTPortClassRep->assignedPower;
				pGetFlowchart->portsClassList[i].autoClassMeasurement = getBTPortClassRep->autoClassMeasurement;
				pGetFlowchart->portsClassList[i].autoClassSupport = getBTPortClassRep->autoClassSupport;

			}
			else{
				pGetFlowchart->portsClassListBool[i] = FALSE;
			}
			if ((getBTPortStatusRep->portStatus == 0xA0 || getBTPortStatusRep->portStatus == 0xA7 || getBTPortStatusRep->portStatus == 0x47 || getBTPortStatusRep->portStatus == 0x43) && (getBTPortStatusRep->portEvent&2)){

				pGetFlowchart->portsCountersListBool[i] = TRUE;

				/* Get BT Port Counters*/
				P_POE_GET_BT_PORT_COUNTERS getBTPortCounters = (P_POE_GET_BT_PORT_COUNTERS) newPbmsg->data;
				getBTPortCounters->portNum = i;
				INIT_BASE_MSG(newPbmsg, POE_CODE_GET_BT_PORT_COUNTERS,sizeof(*getBTPortCounters));
				rc = POEAPITranslateMessage(newPbmsg);
				P_POE_GET_BT_PORT_COUNTERS_REP getBTPortCountersRep = (P_POE_GET_BT_PORT_COUNTERS_REP)newPbmsg->data;

				pGetFlowchart->portsCountersList[i].UDLCount = getBTPortCountersRep->UDLCount;
				pGetFlowchart->portsCountersList[i].OVLCount = getBTPortCountersRep->OVLCount;
				pGetFlowchart->portsCountersList[i].SCCount = getBTPortCountersRep->SCCount;
				pGetFlowchart->portsCountersList[i].invalidSignatureCount = getBTPortCountersRep->invalidSignatureCount;
				pGetFlowchart->portsCountersList[i].powerDeniedCount = getBTPortCountersRep->powerDeniedCount;
				pGetFlowchart->portsCountersList[i].MSCCUse11 = getBTPortCountersRep->MSCCUse11;
				pGetFlowchart->portsCountersList[i].MSCCUse12 = getBTPortCountersRep->MSCCUse12;

			}
			else{
				pGetFlowchart->portsCountersListBool[i] = FALSE;
			}
		}
	}
	/* actions of ports with no event*/
	for (i=0; i<48; i++){
		if (!pGetFlowchart->portsStatusListBool[i]){

			/* Get BT Port Measurement*/
			P_POE_GET_BT_PORT_MEASUREMENTS getBTNePortMeasurement = (P_POE_GET_BT_PORT_MEASUREMENTS) newPbmsg->data;
			getBTNePortMeasurement->CHNum = i;
			INIT_BASE_MSG(newPbmsg, POE_CODE_GET_BT_PORT_MEAS,sizeof(*getBTNePortMeasurement));
			rc = POEAPITranslateMessage(newPbmsg);

			P_POE_GET_BT_PORT_MEASUREMENTS_REP getBTNePortStatusRep = (P_POE_GET_BT_PORT_MEASUREMENTS_REP)newPbmsg->data;

			pGetFlowchart->nePortsStatusList[i].measuredPortPower = getBTNePortStatusRep->measuredPortPower;
		}
	}

	/* Get Total Power*/
	INIT_BASE_MSG(newPbmsg, POE_CODE_GET_TOTAL_POWER,0);
	rc = POEAPITranslateMessage(newPbmsg);
	P_POE_GET_TOTAL_POWER_REP getTotalPower = (P_POE_GET_TOTAL_POWER_REP)newPbmsg->data;

	pGetFlowchart->powerConsumption = getTotalPower->powerConsumption;
	pGetFlowchart->calcPower = getTotalPower->calcPower;
	pGetFlowchart->availablePower = getTotalPower->availablePower;
	pGetFlowchart->powerLim = getTotalPower->powerLim;
	pGetFlowchart->powerBank = getTotalPower->powerBank;
	pGetFlowchart->vmainVoltage = getTotalPower->vmainVoltage;

	return PoE_RC_SUCSESS;
}




PoE_RC_e POECommTranslateDownloadSwrMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	varIndex =0;


	switch (pmsg->code)

	{
		case POE_CODE_DOWNLOAD_COMMAND:
		{
			FILL_PROGRAMM;
			POECommSetVarMsg("%c%c%c%c%c%c",0xFF, 0x99, 0x15, 0x16, 0x16, 0x99);

			break;
		}
		default:
			return PoE_RC_ERR_BASE_MSG_MISMATCH;

	}
	return PoE_RC_SUCSESS;

}


PoE_RC_e POECommTranslatePowerDerMsg(IN OUT P_POE_BASE_MESSAGE pmsg)
{
	return	PoE_RC_SUCSESS;
}








PoE_RC_e POEAPITranslateMessage(IN OUT P_POE_BASE_MESSAGE pmsg)
{

	PoE_RC_e rc;
	int txRetry;
	int recUpdateRep =0;


	INT8  sg_id = POEAPIReqCodeToSGId(pmsg->code);
	
	if (sg_id != -1)
	{

		for(txRetry =0;txRetry<RECEIVE_RETRY_NUMBER;txRetry++)
		{


			MUTEX_LOCK;

			memset(pTxParams,POE_NA_CODE,sizeof(pTxParams));

			rc = ReqTranslators[sg_id](pmsg);
			if(rc == PoE_RC_SUCSESS)
			{
			
				rc = POECommUpdateAndSend();

				if( rc == PoE_RC_SUCSESS)
				/*now wait for reply*/
				{

					rc = POECommWaitReadReply();
					rc = POECommCheckReadReply(&recUpdateRep);
					if( rc == PoE_RC_SUCSESS)
					{
						if(recUpdateRep == NEED_UPDATE_REPLY) /*TELEMENTRY received - need to update reply structure*/
							rc = ReplyTranslators[sg_id](pmsg);
					}
				}
			}
		
			MUTEX_UNLOCK;

			if(rc == PoE_RC_ERR_COMM_TIMEOUT)
			{
				OS_SLEEP(2000);

			}
			else if (rc == PoE_RC_ERR_COMM_DEVICE)
			{
	
				OS_SLEEP(3000);

			}
			else if((rc == PoE_RC_ERR_WRONG_CS)||(rc == PoE_RC_ERR_SUBJECT_BYTES)||(rc == PoE_RC_ERR_DATA_BYTES)||(rc == PoE_RC_ERR_KEY_VALUE)||(rc == PoE_RC_ERR_ECHO_BYTE))
				txRetry++;
			else
				return rc;
			
		}

	}

	return PoE_RC_ERR_MSG_TYPE; 


}





PoE_RC_e POEAPIOpenSession(IN const char *devName, PoE_Comm_Dev_e devType, UINT16 devCommDelay,UINT8 *params,IN unsigned int apiVersion)
{

	PoE_RC_e rc;



	if(	apiVersion > HOST_POE_API_VER)
		return PoE_RC_ERR_INIT;

	rc = poeCommOsMutexInit();

	if(rc != PoE_RC_SUCSESS)
		return rc;



	if(strlen(devName) >= MAX_DEVICE_NAME_SIZE)
		return PoE_RC_ERR_INIT;


	MUTEX_LOCK;
	strcpy(commDevice.deviceName,devName);
	commDevice.deviceName[MAX_DEVICE_NAME_SIZE] ='\0';

	if(devType == POE_COMM_I2C)
		{
			P_POE_SET_I2C_PARAMS i2cParams = (P_POE_SET_I2C_PARAMS)params;
			commDevice.params =(void *)i2cParams;

		}



	commDevice.devType= devType;
	commDevice.devDelay = devCommDelay;


	rc =  POECommOpenDev();
	MUTEX_UNLOCK;

	return rc;
}




PoE_RC_e POEAPICloseSession(void)
{

	PoE_RC_e rc;


	rc = poeCommOsMutexDetroy();
	if(rc != PoE_RC_SUCSESS)
		return rc;


	if(strlen(commDevice.deviceName) == 0) /* No any Device open */
			return PoE_RC_ERR_DRIVER;


	
	return POECommCloseDev();

}










INT8 POEAPIReqCodeToSGId(IN unsigned short poe_req_code) 
{
	if (poe_req_code >= API_INFRASTRCTURE_COMMAND_BASE) {
			if (poe_req_code < API_SYSTEM_COMMAND_BASE) {
				return POE_INFRA_ID;
			}
			if (poe_req_code < API_DEVICE_COMMAND_BASE) {
				return POE_SYS_ID;
			}
			if (poe_req_code < API_PORT_COMMAND_BASE) {
				return POE_DEV_ID;
			}
			if (poe_req_code < API_POWER_MNGT_COMMAND_BASE) {
				return POE_PORT_ID;
			}
			if (poe_req_code < API_POWER_DER_COMMAND_BASE) {
				return POE_POWER_MNG_ID;
			}

			if (poe_req_code < API_SOFTWARE_DOWNLOAD) {
				return POE_POWER_DER_ID;
			}
			if (poe_req_code < API_ERR_COMMAND_BASE) {
				return POE_SOFTWARE_DOWNLOAD_ID;
			}
			
   }
   return -1;	
}






