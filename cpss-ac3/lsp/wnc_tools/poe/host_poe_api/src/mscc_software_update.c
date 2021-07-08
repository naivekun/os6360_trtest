

/*========================================================================
 INCLUDES
  ======================================================================== */



/*#include <fstream> */
/*#include <sstream>*/
/*#include <string>*/
/*#include <cerrno>*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "mscc_poe_global_internal.h"
#include "host_poe_api.h"
#include "host_poe_communication.h"
#include "host_poe_comm_device.h"
#include "host_poe_comm_os.h"


/****************************************************************************
                         DEFINITIONS
****************************************************************************/
#define MSCC_PRINTF_INFO(fmt, args...)   mscc_printf(fmt, ##args)
#define MSCC_PRINTF_ERROR(fmt, args...)  mscc_printf(__FILE__ " %s, %d: " fmt "\n", __func__, __LINE__, ##args)

void mscc_printf(const char *format, ...)
{
	/*if(ePOE_BOOL_Printf_Enable == ePOE_TRUE)*/ /* Brand */
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format, args);
		fflush(stdout);
		va_end(args);
	}
}


UINT8 pTxParams[POE_MESSAGE_SIZE];
UINT8 pTxParams_SWDownload[55];
UINT8 pRxBuf[64];

/****************************************************************************
                         GLOBAL VARS
****************************************************************************/

static U8 byArr_GET_SYSTEM_STATUS[] = { 0x02, 0x02, 0x07, 0xD0, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x03, 0x99 };
static U8 byArr_DOWNLOAD[]          = { 0x01, 0x01, 0xFF, 0x99, 0x15, 0x16, 0x16, 0x99, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x03, 0xFA };
static U8 byArr_RST[]               = { 'R', 'S', 'T' };




/****************************************************************************
                       L O C A L      F U N C T I O N S
****************************************************************************/


/*---------------------------------------------------------------------
 *    description: Clears Leading Zeroes From Data
 *
 *    input :   byArrBuffer           - data array
 *                      usRplySize            - data length
 *                      iMaxBufferSize        - input array size
 *    output:   none
 *    return:   number of data elements after removing the zeros.
 *---------------------------------------------------------------------*/
U16 ClearLeadingZeroesFromData(U8 byArrBuffer[], U16 usRplySize , int iMaxBufferSize)
{
    U16 wStr_index = 0;
    S32 iBuf_index = 0;

    if(usRplySize > iMaxBufferSize)
        usRplySize = (U16)(iMaxBufferSize -1);

    for(iBuf_index=0; iBuf_index<usRplySize ; iBuf_index++)
    {
        if(byArrBuffer[iBuf_index] != 0)
        {
                byArrBuffer[wStr_index] = byArrBuffer[iBuf_index];
                wStr_index++;
        }
    }

    return wStr_index;
}

/*---------------------------------------------------------------------
 *    description: convert bytes to char This function check if communication with PoE Device is OK (it send GET_SYSTEM_STATUS telemetry and expect proper response)
 *
 *    input :   pcTitle               - title (string) to print before data
 *    			byArrBuffer           - data array
 *    			iDataLen              - data length
 *    			iMaxBufferSize        - input array size
 *    output:   none
 *    return:   none
 *---------------------------------------------------------------------*/
void PrintBytesBuffer(const char* pcTitle, U8 byArrBuffer[], int iDataLen, int iMaxBufferSize)
{
	int i=0;

	if(iDataLen > iMaxBufferSize)
		iDataLen = iMaxBufferSize -1;

	MSCC_PRINTF_INFO(" Datalen : %d , ",iDataLen)  ;

	if(iDataLen > 0) /* check if there is data to print */
	{
		MSCC_PRINTF_INFO(" %s ",pcTitle)  ;

		for(i=0;i<iDataLen;i++)
			MSCC_PRINTF_INFO("%02x ",byArrBuffer[i])  ;  /* display reply rec from PoE MCU */

		MSCC_PRINTF_INFO("\n\r");
	}
}



/*---------------------------------------------------------------------
 *    description: convert bytes to char This function check if communication with PoE Device is OK (it send GET_SYSTEM_STATUS telemetry and expect proper response)
 *
 *    input :   byArrBuffer           - data array
 *    			iDataLen              - data length
 *    			iMaxBufferSize        - input array size
 *    output:   char cStr[]           - char array with converted chars
 *    return:   number of converted chars
 *---------------------------------------------------------------------*/
int BytesToString(U8 byArrBuffer[], _OUT char cStr[], int iDataLen, int iMaxBufferSize)
{
	int i=0;
	cStr[0] = (char) 0;

	if(iDataLen > iMaxBufferSize)
		iDataLen = iMaxBufferSize -1;

	memset(cStr,0, iMaxBufferSize * sizeof(char));

	while (i < iDataLen)
	{
		cStr[i] = (char)byArrBuffer[i];
		if(byArrBuffer[i] == 0)
			return i;

		i++;
	}

	return 0;
}



/*---------------------------------------------------------------------
 *    description: This function check if communication with PoE Device is OK (it send GET_SYSTEM_STATUS telemetry and expect proper response)
 *
 *    input :   byI2C_Address           - device I2C address
 *    output:   *pePOE_BOOL_Is_system_status - indicate if system_status telemetry detected or not
 *    return:   PoE_RC_SUCSESS          - operation succeed
 *              < PoE_RC_SUCSESS        - operation failed
 *---------------------------------------------------------------------*/
POE_BOOL_e iSerComm_verifyComm_with_poe()
{
	PoE_Status_e eResult;

	memcpy(pTxParams, byArr_GET_SYSTEM_STATUS, POE_MESSAGE_SIZE);
	poeCommDevWriteI2C(15);
	poeCommOsSleep(50);
	eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE);

	if (eResult != PoE_RC_SUCSESS)
	{
		 /* ETIMEDOUT - our time is up or just an error */
		 MSCC_PRINTF_INFO("No responde for GetSystemStatus telemetry \n\r"); /* error */
		 return ePOE_FALSE ;
	}

	/* verify that subject is telemetry (checksum and echo already verified)  */
	if((eResult == PoE_RC_SUCSESS) && (pRxBuf[0] == B_Telemetry))
	{
		return ePOE_TRUE;
	}
	else
	{
		 /* ETIMEDOUT - our time is up or just an error */
		 MSCC_PRINTF_INFO("GetSystemStatus telemetry answered with reply error. Fail !!\n\r"); /* error */
		 return ePOE_FALSE ;
	}
}




/*---------------------------------------------------------------------
 *    description: This function program new PoE firmware into the PoE Device
 *
 *    input :   bI2C_Address            - device I2C address
 *              S19FileName             - S19 file name (include absolute path)
 *    output:   none
 *    return:   PoE_RC_SUCSESS          - operation succeed
 *              < PoE_RC_SUCSESS        - operation failed
 *---------------------------------------------------------------------*/
PoE_RC_e eCmnd_do_poe_firmware_update(char S19FileName[])
{
    U8                  bDotCntr=0 ;
    PoE_RC_e        eResult    ;

    U8    bRxMsg[RX_BUFF_SIZE];
    char    cStr[RX_BUFF_SIZE];

	#define LINE_SIZE 120
    char line [LINE_SIZE];


	MSCC_PRINTF_INFO("Check S19 file... ");

    /* check for S19 file validation */
	FILE *file = fopen ( S19FileName, "r" );
	if (file == NULL)
	{
		MSCC_PRINTF_INFO("ERROR: FAILED TO OPEN FILE \n\r");
	    return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_FAILED_TO_OPEN_FILE; /*print the error message on stderr.*/
	}

	if(fgets(line,sizeof line,file) == NULL) /* read a line from a file */
	{
		MSCC_PRINTF_INFO("ERROR: FAILED TO READ FIRST LINE FROM S19 FILE \n\r");
		return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_S19_FILE_CONTENT_ERROR;
	}

	if(line[0] != 0x53) /* looking for char 'S' */
	{
		MSCC_PRINTF_INFO("ERROR: S19 FILE CONTENT ERROR - FIRST CHAR MUST BE S \n\r");
		return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_S19_FILE_CONTENT_ERROR;
	}


	MSCC_PRINTF_INFO("[OK]\n\r");
    fclose(file);

    poeCommOsSleep(1000);

    /* remove get system status messages if exist ... */
    int iCount = 0;
    POE_BOOL_e ePOE_BOOL;
    do
    {
        iCount++;
        ePOE_BOOL = iSerComm_verifyComm_with_poe();
    }
    while((ePOE_BOOL != ePOE_TRUE) && (iCount < 3));

    /* if comm with PoE MCU is OK, send DOWNLOAD command */
    if ( ePOE_BOOL == ePOE_TRUE )
    {
        MSCC_PRINTF_INFO("POE has an operational firmware, Lets erase it \n\r");
        MSCC_PRINTF_INFO("Send DOWNLOAD command. Rec:");

        memcpy(pTxParams, byArr_DOWNLOAD, POE_MESSAGE_SIZE);
        eResult = poeCommDevWriteI2C(15);

        poeCommOsSleep(100);

        /* Read 15bytes response + 5 bytes TPE\n\r from PoE Device */
        /* in case of UART - it will send ack+TPE at once */
        /* in case of I2C - it will send ack and then the TPE */
        eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE);

		if (eResult != PoE_RC_SUCSESS)
		{
			tCountersInternal.tErrorCounters.ulCommunicationTimeout++; /* increment time out counter */

  	         /* ETIMEDOUT - our time is up or just an error */
  	         MSCC_PRINTF_INFO("No responde for DOWNLOAD command. Fail - exit update !!\n\r"); /* error */

 			/* if checksum failed return as error */
 			if (eResult == ePOE_STATUS_ERR_RX_MSG_CHECKSUM_ERR)
 			{
				memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
				poeCommDevWriteI2C(3);
 				return ePOE_STATUS_ERR_RX_MSG_CHECKSUM_ERR;
 			}

			memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
			poeCommDevWriteI2C(3);
  	        return eResult ;
		}

		 /* in case of I2C - now we can obtain the TPE */
		poeCommOsSleep(200);
		eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE);
		if (eResult != PoE_RC_SUCSESS)
		{
			 /* ETIMEDOUT - our time is up or just an error */
			 MSCC_PRINTF_INFO("No responde for DOWNLOAD command. Fail - exit update !!\n\r"); /* error */
			 PrintBytesBuffer("Rec:", bRxMsg, POE_MESSAGE_SIZE, POE_MESSAGE_SIZE );   /* display reply rec from PoE MCU */
			 memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
			 poeCommDevWriteI2C(3);
			 return eResult ;
		}

		/* convert databytes to string */
                memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
                BytesToString( bRxMsg, _OUT cStr, 5, POE_MESSAGE_SIZE);

	    MSCC_PRINTF_INFO(" Download Report OK. Expect to rec TPE. Rec:");

	    if ((!strstr(cStr,"TPE") ) && (!strstr(cStr,"TOE") ))
	    {
	    	MSCC_PRINTF_INFO(" [FAIL] - No TPE received - Exit update\n\r"); /* error */
   	        PrintBytesBuffer("Rec:", bRxMsg, POE_MESSAGE_SIZE, POE_MESSAGE_SIZE );     /* display reply rec from PoE MCU */
		memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
		poeCommDevWriteI2C(3);
	        return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_NO_TPE;
	    }

	    MSCC_PRINTF_INFO(" TPE [OK]\n\r")  ;
    }
    else  /* PoE MCU seem not to have valid firmware */
    {
    	MSCC_PRINTF_INFO("PoE Unit seem not to have valid firmware!!\n\r");
        
        MSCC_PRINTF_INFO("If fails to operate download - please Reset Unit is needed prior to next try...\n\r");
                      
        poeCommOsSleep(5000);
        MSCC_PRINTF_INFO("Send ENTR. Expect to rec TPE. Rec:");
        
	memset(pTxParams, 0, POE_MESSAGE_SIZE);
        poeCommOsSleep(10);

	pTxParams[0] = 'E';
	poeCommDevWriteI2C(1);
	poeCommOsSleep(50);

	pTxParams[0] = 'N';
	poeCommDevWriteI2C(1);
	poeCommOsSleep(50);

	pTxParams[0] = 'T';
	poeCommDevWriteI2C(1);
	poeCommOsSleep(50);

	pTxParams[0] = 'R';
	poeCommDevWriteI2C(1);
	poeCommOsSleep(50);

        /* --- As a result of DOWNLOAD command or ENTR string, we expect to read TPE from PoE MCU ------- */
	poeCommOsSleep(400);      ; /* wait 400ms before trying to read "TPE\r\n\r" from PoE device */

        /* expect to read TPE\n\r */
	eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE);
	memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
        if ((eResult != PoE_RC_SUCSESS))
        {
        	MSCC_PRINTF_INFO("\n\rDEVICE_COMMUNICATION_Read() failed to read read TPE\r\n\r"); /* error */
        	PrintBytesBuffer("Rec:", bRxMsg, POE_MESSAGE_SIZE, POE_MESSAGE_SIZE);   /* display reply rec from PoE MCU */
		memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
		poeCommDevWriteI2C(3);
            return eResult;
        }

	ClearLeadingZeroesFromData(bRxMsg, 5, RX_BUFF_SIZE);
        BytesToString( bRxMsg, _OUT cStr, 5, POE_MESSAGE_SIZE);
	PrintBytesBuffer("Rec:", bRxMsg, 5, POE_MESSAGE_SIZE);   /* display reply rec from PoE MCU */
        if ((!strstr(cStr,"TPE") ) && (!strstr(cStr,"TOE") ))
        {
        	MSCC_PRINTF_INFO(" [FAIL] - No TPE received - Exit update\n\r"); /* error */
        	PrintBytesBuffer("Rec:", bRxMsg, 3, POE_MESSAGE_SIZE);     /* display reply rec from PoE MCU */
		memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
		poeCommDevWriteI2C(3);
            return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_NO_TPE;
        }

        MSCC_PRINTF_INFO(" [OK]\n\r")  ;
    }


    /* --- Send E , and expect to read TOE from PoE MCU ------- */
    MSCC_PRINTF_INFO("Send E, Expect to rec TOE. Rec:")  ;
    
    pTxParams[0] = 'E';
    poeCommDevWriteI2C(1);

    poeCommOsSleep(100);

    eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE); /* expect to read TOE\r\n or TE or TPE   */
	
    memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
    ClearLeadingZeroesFromData(bRxMsg, 5, RX_BUFF_SIZE);
    BytesToString( bRxMsg, _OUT cStr, 5, POE_MESSAGE_SIZE);

    MSCC_PRINTF_INFO("%s",cStr)         ;
    if (!strstr(cStr,"TOE"))  /* rx recieved TOE  */
    {
    	MSCC_PRINTF_INFO(" [FAIL] - No TOE received - Exit update\n\r"); /* error */
    	PrintBytesBuffer("Rec:", bRxMsg, 3, POE_MESSAGE_SIZE);     /* display reply rec from PoE MCU */
	memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
	poeCommDevWriteI2C(3);
        return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_NO_TE_TPE ;
    }

    MSCC_PRINTF_INFO("wait 5Sec. Expect to rec TE TPE. Rec:")  ;
    poeCommOsSleep(5000);

	eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE); /* expect to read TOE\r\n or TE or TPE */

    memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
    ClearLeadingZeroesFromData(bRxMsg, 2, RX_BUFF_SIZE);
    BytesToString( bRxMsg, _OUT cStr, 2, POE_MESSAGE_SIZE);

    MSCC_PRINTF_INFO("%s",cStr)         ;
    if ((!strstr(cStr,"TE")) && (!strstr(cStr,"TPE")) )   /* all 3 must be recieved TE TPE */
    {
    	MSCC_PRINTF_INFO(" [FAIL] - No TOE TE TPE received - Exit update\n\r"); /* error */
    	PrintBytesBuffer("Rec:", bRxMsg, 3, POE_MESSAGE_SIZE);    /* display reply rec from PoE MCU */
	memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
	poeCommDevWriteI2C(3);
        return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_NO_TE_TPE ;
    }

    MSCC_PRINTF_INFO(" [OK]\n\r")         ;

    /* --- Send P , and expect to read TOP from PoE MCU ------- */
    pTxParams[0] = 'P';
    poeCommDevWriteI2C(1);

    poeCommOsSleep(500);

    MSCC_PRINTF_INFO("Send P. Expect to rec TOP. Rec:")         ;

    eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE); /* expect to read TOP\r\n */
    memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
    ClearLeadingZeroesFromData(bRxMsg, 3, RX_BUFF_SIZE);
    BytesToString( bRxMsg, _OUT cStr, 3, POE_MESSAGE_SIZE);

    MSCC_PRINTF_INFO("%s",cStr)         ;
    
    if ( !strstr(cStr,"TOP") )
    {
    	MSCC_PRINTF_INFO(" [FAIL] - No TOP received - Exit update\n\r"); /* error */
    	PrintBytesBuffer("Rec:", bRxMsg, 3, POE_MESSAGE_SIZE);     /* display reply rec from PoE MCU */
		memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
		poeCommDevWriteI2C(3);
        return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_TOP ;
    }
    
    MSCC_PRINTF_INFO(" [OK]\n\r")                                  ;
    MSCC_PRINTF_INFO("Start S19 transfer (a few minutes duration)\n\r")      ;



    S32    iNumber_of_bytes_to_write;

	file = fopen ( S19FileName, "r" );
	if (file == NULL)
	{
		memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
		poeCommDevWriteI2C(3);
	    return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_FAILED_TO_OPEN_FILE; /* print the error message on stderr.*/
	}

	POE_BOOL_e ePOE_BOOL_write_S19_lines_succeed = ePOE_TRUE; /* flag to sign if writing S19 lines operation succeed  */

    while(fgets(line,sizeof line,file)!= NULL) /* read a line from a file */
    {
       if  (line[1] == 0x31 /* characters 5- 8 in the line specify a 16-bit address.  */
         || line[1] == 0x32 /* characters 5-10 in the line specify a 24-bit address.  */
         || line[1] == 0x33 /* characters 5-12 in the line specify a 32-bit address.  */
         || line[1] == 0x37 /* end of file. A checksum follows but if generally ignored. */
         || line[1] == 0x38 /* end of file. A checksum follows but if generally ignored. */
         || line[1] == 0x39 /* end of file. A checksum follows but if generally ignored. */
         )
        {      
    	    iNumber_of_bytes_to_write = strlen(line) - 1;

	    	memset(pTxParams_SWDownload, 0, iNumber_of_bytes_to_write+2);
		    memcpy(pTxParams_SWDownload, line, iNumber_of_bytes_to_write);
		    pTxParams[iNumber_of_bytes_to_write] = 0x0d;
	    	pTxParams[iNumber_of_bytes_to_write+1] = 0x0a;
	    	eResult = poeCommDevWriteI2C(iNumber_of_bytes_to_write+2);

	    if (eResult != PoE_RC_SUCSESS)
			{
                ePOE_BOOL_write_S19_lines_succeed = ePOE_FALSE;
				MSCC_PRINTF_INFO("\n\rFailed to write byte to PoE device ,status: %d - Exit update\n\r", eResult);
				break;
			}

            memset(line,0, sizeof(line));

            if ((eDevice_Method == eDevice_Method_UART_Sync) || (eDevice_Method == eDevice_Method_UART_Async))
		    	poeCommOsSleep(SOFTWARE_UPDATE_S19_ACK_UART_DELAY_mS);  /* uart sync minimum delay is 45 (as tested on lab)*/

            eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE); /* expect to read TOP\r\n */
            memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
	    ClearLeadingZeroesFromData(bRxMsg, 5, RX_BUFF_SIZE);
	    	if (eResult != PoE_RC_SUCSESS)
            {
                ePOE_BOOL_write_S19_lines_succeed = ePOE_FALSE;
            	MSCC_PRINTF_INFO("\n\rS19 line , status: %d , Reply size error: %d - Exit update\n\r" ,eResult, 5);     /* error */
				PrintBytesBuffer("Rec:", bRxMsg, 5, POE_MESSAGE_SIZE );   /* display reply rec from PoE MCU */
            	break;
            }

            BytesToString( bRxMsg, _OUT cStr, 2, POE_MESSAGE_SIZE);
                    
            if ((!strstr(cStr, "T*")) && (!strstr(cStr, "TP")))
            {
                ePOE_BOOL_write_S19_lines_succeed = ePOE_FALSE;
                MSCC_PRINTF_INFO("\n\rReply for S19 line error, Exit update\n\r", cStr);
                PrintBytesBuffer("Rec:", bRxMsg, 5, POE_MESSAGE_SIZE );   /* display reply rec from PoE MCU */
                break;
            }

            /* show progress by printing '.' for every S19 line. After 74 dots start new line */
            MSCC_PRINTF_INFO(".");
            if (bDotCntr++ > 74)
            {
            	MSCC_PRINTF_INFO("\n\r");
                bDotCntr = 0 ;
            }
        } 
    }

    MSCC_PRINTF_INFO("END\n\r");
    fclose(file);

    poeCommOsSleep(400);
    memcpy(pTxParams, byArr_RST, POE_MESSAGE_SIZE); /* Command PoE MCU to RESET itself */
    PoE_Status_e eResultWrite = poeCommDevWriteI2C(3);

    /* check if writing S19 lines operation succeed  */
	if(ePOE_BOOL_write_S19_lines_succeed == ePOE_FALSE)
	{
		if(eResult == PoE_RC_SUCSESS)
			return ePOE_STATUS_ERR_COMMUNICATION_TIMEOUT; /* in case we read single byte with value '0' from PoE device in I2C mode */
		else
			return eResult;
	}
    
    if(eResultWrite != PoE_RC_SUCSESS)
    	return eResultWrite;


    MSCC_PRINTF_INFO("waiting for Get system status telemetry... ");

    poeCommOsSleep(5000);

    /* Read 15bytes from PoE Device */
    POECommReadDev(BUF_LEN_TO_READ, 0, TRUE); /* Read first for clean message */
    memcpy(pTxParams, byArr_GET_SYSTEM_STATUS, POE_MESSAGE_SIZE);
    eResult = poeCommDevWriteI2C(15);
    poeCommOsSleep(50);
    eResult = POECommReadDev(BUF_LEN_TO_READ, 0, TRUE);
    memcpy(bRxMsg, pRxBuf, POE_MESSAGE_SIZE);
    if (eResult != PoE_RC_SUCSESS)
	{
		 /* ETIMEDOUT - our time is up or just an error */
		 MSCC_PRINTF_INFO("Error: No System status telemetry received. Fail - exit update !!\n\r"); /* error */
		 PrintBytesBuffer("Rec:", bRxMsg, POE_MESSAGE_SIZE, POE_MESSAGE_SIZE);   /* display reply rec from PoE MCU */
		 return eResult ;
	}

	/* verify that telemetry checksum ,subject and echo are correct   */
	int i=0;
	for (i = 0, iCount = 0; i < 13; i++)
		iCount += bRxMsg[i];

    	if((bRxMsg[0] == B_Telemetry) && (bRxMsg[1] == byArr_GET_SYSTEM_STATUS[1]) && \
			(bRxMsg[13] == ((iCount >> 8) & 0xFF)) && (bRxMsg[14] == (iCount & 0xFF)))
	{
		if ((bRxMsg[2] & 1) != 0)
		{
			MSCC_PRINTF_INFO("Error- Get system status telemetry : PoE controller error !!\n\r"); /* error */
			return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_PoE_controller_error ;
		}
		else if ((bRxMsg[2] & 2) != 0)
		{
			MSCC_PRINTF_INFO("Error- Get system status telemetry : firmware download is required !!\n\r"); /* error */
			return ePOE_STATUS_ERR_FIRMWARE_DOWNLOAD_PoE_firmware_download_is_required ;
		}

		MSCC_PRINTF_INFO("[OK] \n\rProgramming finished successfully \n\r");
		return PoE_RC_SUCSESS;
	}

	/* ETIMEDOUT - our time is up or just an error */
	MSCC_PRINTF_INFO("Error: System status telemetry can't received. Fail - exit update !!\n\r"); /* error */
	PrintBytesBuffer("Rec:", bRxMsg, POE_MESSAGE_SIZE, POE_MESSAGE_SIZE );   /* display reply rec from PoE MCU */

	if(ePOE_BOOL == ePOE_FALSE)
		return ePOE_STATUS_ERR_RX_MSG_CHECKSUM_ERR;
	else
		return ePOE_STATUS_ERR_POE_MCU_FIRMWARE_ERROR;
}
