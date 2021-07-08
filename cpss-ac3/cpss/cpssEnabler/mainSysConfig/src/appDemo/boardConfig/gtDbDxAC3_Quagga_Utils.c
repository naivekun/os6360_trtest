#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/common/port/cpssPortStat.h>
#include <appDemo/boardConfig/gtDbDxAC3_Quagga_Utils.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>


/*For task*/
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/gtOsSharedIPC.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsInit.h>

#include <appDemo/dragonite/appDemoDragonite.h>



static WNC_DEV_TYPE BoardType;
static GT_BOOL printResult=GT_TRUE;
GT_TASK             tidPoeled_demo; /* The task Tid                 */

GT_STATUS WNCCmds()
{
	cpssOsPrintf("WNC_WhiteLedCtrl [status(0 or 1)] : Control White LED\n");
	cpssOsPrintf("WNC_OrangeLedCtrl [status(0 or 1)] : Control Orange LED\n");
	cpssOsPrintf("WNC_FrontPort1GLedCtrl [ledtyp] [status] : Control 1G Front Port LED\n");
	cpssOsPrintf("ledtype 0=Green,1=Orange,2=Both ; status 0=Both off 1=on\n");
	cpssOsPrintf("WNC_FrontPort10GLedCtrl [ledtyp] [status] : Control 10G Front Port LED\n");
	cpssOsPrintf("ledtype 0=Green,1=Orange; status 0=off 1=on\n");
	cpssOsPrintf("WNC_Check1GPortLinkUp : Check if all 1G ports are link up\n");
	cpssOsPrintf("WNC_Check10GPortLinkUp : Check if all 10G ports are link up\n");
	cpssOsPrintf("WNC_Show1GUCCnt : Show unicast frame count of all 1G ports\n");
	cpssOsPrintf("WNC_Show10GUCCnt : Show unicast frame count of all 1G ports\n");
	cpssOsPrintf("WNC_CleanUCCnt : Clean mac counters of all ports\n");
	cpssOsPrintf("WNC_Set10GRxAutoTune : Execture rx auto tune of all 10G ports\n");
	cpssOsPrintf("WNC_1GPktGenTest [count]: 1G Packet test. Test packets = 16M * count\n");
	cpssOsPrintf("WNC_10GPktGenTest [count]: 10G Packet test. Test packets = 16M * count\n");
	return GT_OK;
}


#if 0 
GT_STATUS OsCmd
(
    IN  GT_CHAR *cmdPtr
)
{
	GT_STATUS	rc = GT_OK;

	cpssOsPrintf("Input command:%s\n",cmdPtr);
	
    system(cmdPtr);

	return rc;
}
#endif
GT_STATUS WNC_FrontPort10GLedCtrl
(
	int ledtype,
	int status
)
{
	 GT_U32   portGroupId = 0xFFFFFFFF;
	 GT_U32 mask = 0xFFFFFFFF;

	if ( ledtype < 0 || ledtype > 2)
	{
		cpssOsPrintf("Invaild ledtype parameter! 0~2\n");
		return GT_BAD_PARAM;
	}	

	if ( status != 0 && status != 1)
	{
		cpssOsPrintf("Invaild status parameter! 0~1\n");
		return GT_BAD_PARAM;
	}	

	if ( ledtype == 1 )
		osCmd("switch_sfp_led 1");
	else
		osCmd("switch_sfp_led 0");

	
	if(BoardType == QUAGGA_24_PORT)
	{

		/* Set device #0 port 24 (LED_DATA_1 = 0), port 25 (LED_STB_1 = 0)*/
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x12019090, mask, 0x64fa);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d9030, mask, 0x32c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d9084, mask, 0x212);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005114, mask, 0x1);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x0a005104, mask, 0x55555555);

		/*Config port 0/26/27 LED*/
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x1201B090, mask, 0x6Cfa);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120db030, mask, 0x36c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120db084, mask, 0x210);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x07005114, mask, 0x1);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x08005104, mask, 0x55555555);
	
		if(status == 1)
		{
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005104, mask, 0x00000010);
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x07005104, mask, 0x00000010);
		}
		else
		{
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005104, mask, 0x0000001c);
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x07005104, mask, 0x0000001c);
		}

	}
	else
	{

		/* Set device #0 port 24 (LED_DATA_1 = 0), port 25 (LED_STB_1 = 0)*/
	    cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x12018090, mask, 0x60fa);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x12019090, mask, 0x64fa);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d8030, mask, 0x30c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d9030, mask, 0x32c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d8084, mask, 0x212);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d9084, mask, 0x212);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005114, mask, 0x1);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x0a005104, mask, 0x55555555);

		/*Set device #1 port 25 (LED_STB_1 = 0), port 27 (LED_STB_0 = 0)*/
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x12019090, mask, 0x64fa);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x120d9030, mask, 0x32c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120d9084, mask, 0x212);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x09005114, mask, 0x1);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x0a005104, mask, 0x55555555);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x1201B090, mask, 0x6Cfa);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x120db030, mask, 0x36c);
		cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x120db084, mask, 0x210);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x07005114, mask, 0x1);
		cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x08005104, mask, 0x55555555);

		if(status == 1)
		{
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005104, mask, 0x00000010);
			cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x09005104, mask, 0x00000010);
			cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x07005104, mask, 0x00000010);

		}
		else
		{
			cpssDrvPpHwRegBitMaskWrite(0, portGroupId, 0x09005104, mask, 0x0000001c);
			cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x09005104, mask, 0x00000018);
			cpssDrvPpHwRegBitMaskWrite(1, portGroupId, 0x07005104, mask, 0x00000018);

		}
		
	}
	
}


GT_STATUS WNC_Check1GPortLinkUp()
{
    GT_STATUS	rc;
	int devNum = 0;
	int port;
	GT_PHYSICAL_PORT_NUM realPort;
	GT_PHYSICAL_PORT_NUM portNum; 

	GT_BOOL             isLinkUp;
	int iError=0;

	if(BoardType == QUAGGA_24_PORT)
	{
		port=24;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum-1;
			
			rc = cpssDxChPortLinkStatusGet(devNum, realPort, &isLinkUp);
			 
			if(rc != GT_OK)
			{
				printf("1G Port %02d gets LinkStatus fail!\n",portNum);
				return rc;
			}
			else
			{	
				if(!isLinkUp)
				{
					printf("1G Port%02d is NOT link up\n",portNum);
					iError++;
				}
			}	
		}
	}
	else
	{

		port=48;

		for( portNum=1; portNum < port+1; portNum++)
		{	
			if( portNum < 25)
			{
				devNum=0;
				realPort=portNum-1;
			}	
			else
			{
				devNum=1;
				realPort=portNum-25;
			}	
			
			rc = cpssDxChPortLinkStatusGet(devNum, realPort, &isLinkUp);
			 
			if(rc != GT_OK)
			{
				printf("1G Port %02d gets LinkStatus fail!\n",portNum);
				return rc;
			}
			else
			{	
				if(!isLinkUp)
				{
					printf("1G Port%02d is NOT link up\n",portNum);
					iError++;
				}
			}	
		}
	}

	if(iError != 0)
	{
		printf("Some 1G ports are NOT link up.\n");
		rc = ERR_HW_ERROR;
	}	
	else
	{
		printf("All 1G ports are link up.\n");
		rc = GT_OK;	
	}	

	return rc;

}


GT_STATUS WNC_Check10GPortLinkUp()
{
	GT_STATUS	rc;
	int devNum = 0,port;
	GT_PHYSICAL_PORT_NUM portNum,realPort;
	GT_BOOL             isLinkUp;
	int iError = 0;
	
	if(BoardType == QUAGGA_24_PORT)
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum+23;
			
			rc = cpssDxChPortLinkStatusGet(devNum, realPort, &isLinkUp);
			 
			if(rc != GT_OK)
			{
				printf("10G Port %02d gets LinkStatus fail!\n",portNum);
				return rc;
			}
			else
			{	
				if(!isLinkUp)
				{
					printf("10G Port%02d is NOT link up\n",portNum);
					iError++;
				}
			}	
		}

	}
	else
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=24;}
			if(portNum == 2) { devNum=1;realPort=25;}
			if(portNum == 3) { devNum=0;realPort=25;}
			if(portNum == 4) { devNum=0;realPort=27;}
			
			rc = cpssDxChPortLinkStatusGet(devNum, realPort, &isLinkUp);
			 
			if(rc != GT_OK)
			{
				printf("10G Port %02d gets LinkStatus fail!\n",portNum);
				return rc;
			}
			else
			{	
				if(!isLinkUp)
				{
					printf("10G Port%02d is NOT link up\n",portNum);
					iError++;
				}
			}	
		}
	
		
		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=26;}
			if(portNum == 2) { devNum=1;realPort=27;}
			if(portNum == 3) { devNum=0;realPort=24;}
			if(portNum == 4) { devNum=0;realPort=26;}
			
			rc = cpssDxChPortLinkStatusGet(devNum, realPort, &isLinkUp);
			 
			if(rc != GT_OK)
			{
				printf("Internal 20G dev %d Port%02d gets LinkStatus fail!\n",devNum,realPort);
				return rc;
			}
			else
			{	
				if(!isLinkUp)
				{
					printf("Internal 20G dev %d Port%02d is NOT link up\n",devNum,realPort);
					iError++;
				}
			}	
		}
	}

	if(iError != 0)
	{
		printf("Some 10G ports are NOT link up.\n");
		rc = ERR_HW_ERROR;
	}	
	else
	{
		printf("All 10G ports are link up.\n");
		rc = GT_OK;	
	}	

	return rc;

}

GT_STATUS WNC_Set10GRxAutoTune()
{

	GT_STATUS	rc;
	int devNum = 0,port;
	GT_PHYSICAL_PORT_NUM portNum,realPort;
	CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
	int iError = 0;

	if(BoardType == QUAGGA_24_PORT)
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum+23;
			
			rc = cpssDxChPortSerdesAutoTune(devNum,realPort,CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);

			if(rc != GT_OK)
			{
				printf("Set RX auto tune for 10G port %02d fail!.\n",portNum);
				iError++;
			}
		}

		if(iError != 0)
		{
			printf("Some 10G ports can't set auto tune.\n");
			rc = ERR_HW_ERROR;
		}	
		else
		{
			printf("Set RX auto tune for 10G ports.\n");
		}	

	}
	else
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=24;}
			if(portNum == 2) { devNum=1;realPort=25;}
			if(portNum == 3) { devNum=0;realPort=25;}
			if(portNum == 4) { devNum=0;realPort=27;}
			
			rc = cpssDxChPortSerdesAutoTune(devNum,realPort,CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);

			if(rc != GT_OK)
			{
				printf("Set RX auto tune for 10G port %02d fail!.\n",portNum);
				iError++;
			}
		}

		/*Turning 20G*/		
		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=26;}
			if(portNum == 2) { devNum=1;realPort=27;}
			if(portNum == 3) { devNum=0;realPort=24;}
			if(portNum == 4) { devNum=0;realPort=26;}
			
			rc = cpssDxChPortSerdesAutoTune(devNum,realPort,CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);

			if(rc != GT_OK)
			{
				printf("Set RX auto tune for 20G port dev %d port %02d fail!.\n",devNum,realPort);
				iError++;
			}
		}

		if(iError != 0)
		{
			printf("Some 10G or 20G ports can't set auto tune.\n");
			rc = ERR_HW_ERROR;
		}	
		else
		{
			printf("Set RX auto tune for 10G and 20G ports.\n");
		}	
		
	}

	return GT_OK;

}

GT_STATUS WNC_FrontPort1GLedCtrl
(
	int ledtype,
	int status
)
{
	int i,smiaddr;
	
	printf("Boardtype = %d\n",BoardType);

	if ( ledtype < 0 || ledtype > 2)
	{
		cpssOsPrintf("Invaild ledtype parameter! 0~2\n");
		return GT_BAD_PARAM;
	}	
	
	if ( status != 0 && status != 1)
	{
		cpssOsPrintf("Invaild status parameter! 0~1\n");
		return GT_BAD_PARAM;
	}	

	if(BoardType == QUAGGA_24_PORT)
	{
		/*portnum=24;*/
		for(i=0; i< 16 ; i++)
		{
			smiaddr = i;
			cpssSmiRegisterWriteShort(0,0,0,smiaddr,22,0x0003);
			
			if( status == 1 ) /*Turn on*/
			{
				if( ledtype == 0 ) /*Green*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0089);	
				else if (ledtype == 1) /*Orange*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0098);	
				else /*Both*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0099);	
			}
			else  /*Turn off*/
				cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(0,0,0,smiaddr,22,0x0000);
		}
	
		for(i=16; i< 24 ; i++)
		{
			smiaddr = i-8;
			cpssSmiRegisterWriteShort(0,0,1,smiaddr,22,0x0003);
	
			if( status == 1 ) 
			{
				if( ledtype == 0 ) 
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0089);
				else if (ledtype == 1) 
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0098);
				else
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0099);
			}
			else 
				cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(0,0,1,smiaddr,22,0x0000);
		}
	
	}	
	else
	{
	    /*portnum=48;*/

		for(i=0; i< 16 ; i++)
		{
			smiaddr = i;
			cpssSmiRegisterWriteShort(0,0,0,smiaddr,22,0x0003);
			
			if( status == 1 ) /*Turn on*/
			{
				if( ledtype == 0 ) /*Green*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0089);	
				else if (ledtype == 1) /*Orange*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0098);	
				else /*Both*/
					cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0099);	
			}
			else  /*Turn off*/
				cpssSmiRegisterWriteShort(0,0,0,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(0,0,0,smiaddr,22,0x0000);
		}
		
		for(i=16; i< 24 ; i++)
		{
			smiaddr = i-8;
			cpssSmiRegisterWriteShort(0,0,1,smiaddr,22,0x0003);
	
			if( status == 1 ) 
			{
				if( ledtype == 0 ) 
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0089);
				else if (ledtype == 1) 
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0098);
				else
					cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0099);
			}
			else 
				cpssSmiRegisterWriteShort(0,0,1,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(0,0,1,smiaddr,22,0x0000);
		}
		
		for(i=24; i< 40 ; i++)
		{
			smiaddr = i-24;
			cpssSmiRegisterWriteShort(1,0,0,smiaddr,22,0x0003);
	
			if( status == 1 ) 
			{
				if( ledtype == 0 ) 
					cpssSmiRegisterWriteShort(1,0,0,smiaddr,16,0x0089);
				else if (ledtype == 1) 
					cpssSmiRegisterWriteShort(1,0,0,smiaddr,16,0x0098);
				else
					cpssSmiRegisterWriteShort(1,0,0,smiaddr,16,0x0099);
			}
			else 
				cpssSmiRegisterWriteShort(1,0,0,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(1,0,0,smiaddr,22,0x0000);
			
		}

		for(i=40; i< 48 ; i++)
		{
			smiaddr = i-32;
			cpssSmiRegisterWriteShort(1,0,1,smiaddr,22,0x0003);
	
			if( status == 1 ) 
			{
				if( ledtype == 0 ) 
					cpssSmiRegisterWriteShort(1,0,1,smiaddr,16,0x0089);
				else if (ledtype == 1) 
					cpssSmiRegisterWriteShort(1,0,1,smiaddr,16,0x0098);
				else
					cpssSmiRegisterWriteShort(1,0,1,smiaddr,16,0x0099);
			}
			else 
				cpssSmiRegisterWriteShort(1,0,1,smiaddr,16,0x0088);

			cpssSmiRegisterWriteShort(1,0,1,smiaddr,22,0x0000);
		}
	
	}	
	
}

GT_STATUS WNC_Show1GUCCnt() /*Show unicast frame count*/
{

	GT_STATUS	rc;
	int devNum = 0;
	int port;
	GT_PHYSICAL_PORT_NUM realPort;
	GT_PHYSICAL_PORT_NUM portNum;
	CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;

	if(BoardType == QUAGGA_24_PORT)
	{
		port=24;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum-1;
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				if(printResult)
				{
					printf("1G Port%02d:ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					printf(":ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
				}	
			}   
		}

	}
	else
	{

		port=48;

		for( portNum=1; portNum < port+1; portNum++)
		{	
			if( portNum < 25)
			{
				devNum=0;
				realPort=portNum-1;
			}	
			else
			{
				devNum=1;
				realPort=portNum-25;
			}	
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				if(printResult)
				{
					printf("1G Port%02d:ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					printf(":ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
				}	
			}
		}

	}

	return rc;
	
}


GT_STATUS WNC_Show10GUCCnt() /*Show unicast frame count*/
{

	GT_STATUS	rc;
	int devNum = 0,port;
	GT_PHYSICAL_PORT_NUM portNum,realPort;
	CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;

	if(BoardType == QUAGGA_24_PORT)
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum+23;
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				if(printResult)
				{
					printf("10G Port%02d:ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					printf(":ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
				}	
			}
		}

	}
	else
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=24;}
			if(portNum == 2) { devNum=1;realPort=25;}
			if(portNum == 3) { devNum=0;realPort=25;}
			if(portNum == 4) { devNum=0;realPort=27;}
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{		
				if(printResult)
				{
					printf("10G Port%02d:ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					printf(":ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
				}	
			}
		}

		
		if(printResult)
		{
			printf("\nInternal 20G Port as below:\n");
		}	
		
		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=26;}
			if(portNum == 2) { devNum=1;realPort=27;}
			if(portNum == 3) { devNum=0;realPort=24;}
			if(portNum == 4) { devNum=0;realPort=26;}
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				if(printResult)	
				{
					printf("Dev %d Port%2d:ucPktsSent%12d",devNum,realPort,portMacCounterSetArray.ucPktsSent);
					printf(":ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
				}	
			}
		}
	}

	return rc;
	
}

GT_STATUS WNC_CleanUCCnt
(
	int printR
)
{

	if ( printR != 0 && printR != 1)
	{
		cpssOsPrintf("Invaild status parameter! 0~1\n");
		return GT_BAD_PARAM;
	}	

	if( printR == 1 )
		printResult = GT_TRUE;
	else
		printResult = GT_FALSE;

	WNC_Show1GUCCnt();
	WNC_Show10GUCCnt();
	
	printf("Clean unicast frame count.\n");

	printResult = GT_TRUE;

	return 0;
}

GT_STATUS WNC_1GPktGenTest
(
	IN GT_U32 pktCount	
)
{
	GT_STATUS	rc;
	int devNum;
	GT_BOOL  burstTransmitDone;
	GT_U32 totalPkt = 0;
	GT_PHYSICAL_PORT_NUM realPort;
	GT_PHYSICAL_PORT_NUM portNum;
	GT_U8 failCount = 0;
	CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
	GT_U16 timeOut = 180; /*Timeout 180 secs*/
	int port;
	GT_BOOL testRet;
	

	/*Check link up*/
	rc = WNC_Check1GPortLinkUp();

	if(rc != GT_OK)
	{
		cpssOsPrintf("1G Packet Generator FAIL. NOT all ports are link up.\n");
		return rc;
	}

	/*Clean previous*/
	WNC_CleanUCCnt(0);

	/*Send from 0/0*/
	WNCPktGen(0,0,512,pktCount,7,2,0); 

	if(BoardType == QUAGGA_48_PORT)
	{
		/*Send from 0/1*/	
		WNCPktGen(1,0,512,pktCount,7,2,0);
	}	

    GT_U16 sec =0;

	timeOut =  pktCount*12+5; /*pktCount 1*16777216 is about 12 seconds*/
	cpssOsPrintf("Timeout Time=%dsec.\n",timeOut);

	totalPkt = pktCount *16777216;
	
	while (1)
	{
		/*
		rc = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(devNum, pgPort, &burstTransmitDone);
		if(rc != GT_OK)
		{
			return rc;
		}

		if(burstTransmitDone)
		{
			cpssOsPrintf("\nTotal packet=%d*1048576 Total Time=%dsec\n",pktCount,sec);
			break;
		}
		*/

		if ( sec >= timeOut )
		{
			cpssOsPrintf("\nWait total Time=%dsec.\n",sec);
			break;
		}
				
		if ( (sec%3) == 0)
		{
			cpssOsPrintf(".");
		}	

		sleep(1);		
		sec++;
	}


	if(BoardType == QUAGGA_24_PORT)
	{
		port=24;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum-1;
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				
				if (portMacCounterSetArray.ucPktsRcv.l[0] != totalPkt || portMacCounterSetArray.ucPktsSent.l[0] != totalPkt)
				{
					failCount ++;
					cpssOsPrintf("Port%2d Fail: ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					cpssOsPrintf(": ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
					
				}	
		
			}
		}

	}
	else
	{

		port=48;

		for( portNum=1; portNum < port+1; portNum++)
		{	
			if( portNum < 25)
			{
				devNum=0;
				realPort=portNum-1;
			}	
			else
			{
				devNum=1;
				realPort=portNum-25;
			}	
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
		 	}
			else
			{	
				if (portMacCounterSetArray.ucPktsRcv.l[0] != totalPkt || portMacCounterSetArray.ucPktsSent.l[0] != totalPkt)
				{
					failCount ++;
					cpssOsPrintf("Port%2d Fail: ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					cpssOsPrintf(": ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
					
				}	
			}
		}

	}

	if ( failCount > 0 )
	{
		cpssOsPrintf("\n1G Packet Generator FAIL\n");
		rc = GT_OK;
	}
	else
	{
		cpssOsPrintf("\n1G Packet Generator PASS\n");
		rc = GT_OK;
	}
		
	return rc;

}


GT_STATUS WNC_10GPktGenTest
(
	IN GT_U32 pktCount	
)
{
	GT_STATUS	rc;
	int devNum;
	GT_BOOL  burstTransmitDone;
	GT_U32 totalPkt = 0;
	GT_PHYSICAL_PORT_NUM realPort;
	GT_PHYSICAL_PORT_NUM portNum;
	GT_U8 failCount = 0;
	CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
	GT_U16 timeOut = 180; /*Timeout 180 secs*/
	int port;

	/*Check link up*/
	rc = WNC_Check10GPortLinkUp();

	if(rc != GT_OK)
	{
		cpssOsPrintf("10G Packet Generator FAIL. NOT all ports are link up.\n");
		return rc;
	}
		
	/*Clean previous*/
	WNC_CleanUCCnt(0);

	if(BoardType == QUAGGA_24_PORT)
		/*Send from 0/24*/
		WNCPktGen(0,24,512,pktCount,7,2,0);
	else
		/*Send from 1/24*/
		WNCPktGen(1,24,512,pktCount,7,2,0); 

    GT_U16 sec =0;

	timeOut =  pktCount*8+5; /*pktCount 1*16777216 is about 12 seconds*/
	cpssOsPrintf("Timeout Time=%dsec.\n",timeOut);
	totalPkt = pktCount *16777216;
	
	while (1)
	{
		/*
		rc = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(devNum, pgPort, &burstTransmitDone);
		if(rc != GT_OK)
		{
			return rc;
		}

		if(burstTransmitDone)
		{
			cpssOsPrintf("\nTotal packet=%d*1048576 Total Time=%dsec\n",pktCount,sec);
			break;
		}
		*/

		if ( sec >= timeOut )
		{
			cpssOsPrintf("\nWait total Time=%dsec.\n",sec);
			break;
		}
				
		if ( (sec%3) == 0)
		{
			cpssOsPrintf(".");
		}	

		sleep(1);		
		sec++;
	}


	if(BoardType == QUAGGA_24_PORT)
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			realPort=portNum+23;
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
			}
			else
			{	
				
				if (portMacCounterSetArray.ucPktsRcv.l[0] != totalPkt || portMacCounterSetArray.ucPktsSent.l[0] != totalPkt)
				{
					failCount ++;
					cpssOsPrintf("Port%2d Fail: ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					cpssOsPrintf(": ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
					
				}	
			}
		}

	}
	else
	{
		port=4;

		for( portNum=1; portNum < port+1; portNum++)
		{
			if(portNum == 1) { devNum=1;realPort=24;}
			if(portNum == 2) { devNum=1;realPort=25;}
			if(portNum == 3) { devNum=0;realPort=25;}
			if(portNum == 4) { devNum=0;realPort=27;}
			
			rc = cpssDxChPortMacCountersOnPortGet(devNum, realPort, &portMacCounterSetArray);

			if(rc != GT_OK)
			{
				return rc;
			}
			else
			{		
				if (portMacCounterSetArray.ucPktsRcv.l[0] != totalPkt || portMacCounterSetArray.ucPktsSent.l[0] != totalPkt)
				{
					failCount ++;
					cpssOsPrintf("Port%2d Fail: ucPktsSent%12d",portNum,portMacCounterSetArray.ucPktsSent);
					cpssOsPrintf(": ucPktsRcv%12d\n",portMacCounterSetArray.ucPktsRcv);
					
				}	
			}
		}
	}

	if ( failCount > 0 )
	{
		cpssOsPrintf("\n10G Packet Generator FAIL\n");
		rc = GT_OK;
	}
	else
	{
		cpssOsPrintf("\n10G Packet Generator PASS\n");
		rc = GT_OK;
	}
		
	return rc;

}


GT_STATUS WNC_WhiteLedCtrl
(	
	int status
)
{

	/*GPIO 7 */
	GT_STATUS	rc;
	GT_U32      regValue;

	if ( status != 0 && status != 1)
	{
		cpssOsPrintf("Invaild parameter! 0~1\n");
		return GT_BAD_PARAM;
	}	

	/*Set pin 7 as gpio*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18000,&regValue);
	if(rc != GT_OK){	return rc;}
	
    U32_SET_FIELD_MAC(regValue,28, 4, 0);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18000, regValue);
    if(rc != GT_OK){	return rc;}
    
	 
	/*Set gpio 7 as output*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18104,&regValue);
	if(rc != GT_OK){	return rc;}
	
	/* Set '0' to bit_7 */
    U32_SET_FIELD_MAC(regValue,7, 1, 0);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18104, regValue);
    if(rc != GT_OK){	return rc;}

	/*Set gpio 7 output vlaue as input status*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18100,&regValue);
	if(rc != GT_OK){	return rc;}
	
	/* Set status value to bit_7 */
    U32_SET_FIELD_MAC(regValue,7, 1, status);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18100, regValue);
    if(rc != GT_OK){	return rc;}

	cpssOsPrintf("Turn %s White LED!\n", status ? "on" : "off");
	
	return rc;
}

GT_STATUS WNC_OrangeLedCtrl
(	
	int status
)
{

	/*GPIO 8*/
	GT_STATUS	rc;
	GT_U32      regValue;

	if ( status != 0 && status != 1)
	{
		cpssOsPrintf("Invaild parameter! 0~1\n");
		return GT_BAD_PARAM;
	}	
	
	/*Set pin 8 as gpio*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18004,&regValue);
	if(rc != GT_OK){	return rc;}
	
    U32_SET_FIELD_MAC(regValue,0, 4, 0);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18004, regValue);
    if(rc != GT_OK){	return rc;}
    
	 
	/*Set gpio 8 as output*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18104,&regValue);
	if(rc != GT_OK){	return rc;}
	
	/* Set '0' to bit_8 */
    U32_SET_FIELD_MAC(regValue,8, 1, 0);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18104, regValue);
    if(rc != GT_OK){	return rc;}

	/*Set gpio 8 output vlaue as input status*/
	rc = cpssDrvPpHwInternalPciRegRead(0, 0, 0x18100,&regValue);
	if(rc != GT_OK){	return rc;}
	
	/* Set status value to bit_8 */
	U32_SET_FIELD_MAC(regValue,8, 1, status);
	rc = cpssDrvPpHwInternalPciRegWrite(0, 0, 0x18100, regValue);
	if(rc != GT_OK){	return rc;}

	cpssOsPrintf("Turn %s Orange LED!\n", status ? "on" : "off");
	
	return rc;
}

GT_STATUS WNCPktGen
(
    IN int devNum,
    IN GT_PHYSICAL_PORT_NUM pgPort,
	IN GT_U32 pktLength,
	IN GT_U32 pktCount,
	IN GT_U8 pktCountMultiplier,
	IN GT_U8 pktIfaceSize,
	IN GT_U8 checkDone
)
{
	GT_STATUS	rc;
	/*int devNum = 0;*/
	CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC pgConfig;
	GT_BOOL   burstTransmitDone=GT_FALSE;
	GT_U32 totalPkt = 0;

    /* configure packet generator */
    cpssOsMemSet(&pgConfig, 0, sizeof(pgConfig));
	
	pgConfig.macDa.arEther[5]      = 0x11; /*00:00:00:00:00:11*/
	pgConfig.macDaIncrementEnable  = GT_FALSE;
	pgConfig.macDaIncrementLimit   = 0;
    pgConfig.macSa.arEther[5]      = 0xa0; /*00:00:00:00:00:a0*/
    pgConfig.vlanTagEnable         = GT_FALSE;
	pgConfig.vpt 				   = 0;
	pgConfig.cfi                   = 0;
    pgConfig.vid                   = 0;
	pgConfig.etherType             = 0;  
	pgConfig.payloadType           = CPSS_DIAG_PG_PACKET_PAYLOAD_CONSTANT_E;
	pgConfig.cyclicPatternArr[0]   = 0;
	pgConfig.cyclicPatternArr[1]   = 0;
	pgConfig.cyclicPatternArr[2]   = 0;
	pgConfig.cyclicPatternArr[3]   = 0;
	pgConfig.cyclicPatternArr[4]   = 0;
	pgConfig.cyclicPatternArr[5]   = 0;
	pgConfig.cyclicPatternArr[6]   = 0;
	pgConfig.cyclicPatternArr[7]   = 0;
	pgConfig.packetLengthType      = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E; /*0*/
    pgConfig.packetLength          = pktLength;
	pgConfig.undersizeEnable       = GT_FALSE;
    pgConfig.transmitMode          = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
    pgConfig.packetCount           = pktCount;

	/*packetCountMultiplier 0~8*/
	if( pktCountMultiplier < 0 || pktCountMultiplier > 7)
		pktCountMultiplier = 0;

	pgConfig.packetCountMultiplier = pktCountMultiplier;

	/*
	if ( pgPort > 20 ) 
		pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_8_BYTES_E;
    	else  
		pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_64_BYTES_E;
     */

	switch(pktCountMultiplier)
    {
        case 0:
			totalPkt = pktCount *1;
            break;
		case 1:
			totalPkt = pktCount *256;
            break;
		case 2:
			totalPkt = pktCount *512;
            break;
		case 3:
			totalPkt = pktCount *1024;
            break;
		case 4:
			totalPkt = pktCount *4096;
            break;
		case 5:
			totalPkt = pktCount *65536;
            break;
		case 6:
			totalPkt = pktCount *1048576;
            break;
		case 7:
			totalPkt = pktCount *16777216;
            break;
        default:
            totalPkt = pktCount *1;
            break;
    }
	

    pgConfig.ipg                   = 12;

	switch(pktIfaceSize)
    {
        case 0:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_DEFAULT_E;
            break;
		case 1:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_1_BYTE_E;
            break;
		case 2:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_8_BYTES_E;
            break;
		case 3:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_16_BYTES_E;
            break;
		case 4:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_32_BYTES_E;
            break;
		case 5:
			pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_64_BYTES_E;
            break;
        default:
            pgConfig.interfaceSize=CPSS_DIAG_PG_IF_SIZE_DEFAULT_E;
            break;
    }

	rc = cpssDxChDiagPacketGeneratorConnectSet(devNum, pgPort,GT_TRUE, &pgConfig);
	if(rc != GT_OK)
	{
		return rc;
	}

	/* start transmit*/
	rc = cpssDxChDiagPacketGeneratorTransmitEnable(devNum, pgPort, GT_TRUE);
	if(rc != GT_OK)
	{
		return rc;
	}

	if(checkDone == 1)
	{
	    int sec =0;
		while (1)
		{
			rc = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(devNum, pgPort, &burstTransmitDone);
			if(rc != GT_OK)
			{
				return rc;
			}

			if(burstTransmitDone)
			{
				cpssOsPrintf("\nTotal packet=%d Total Time=%dsec\n",totalPkt,sec);
				break;
			}
					
			if ( (sec%5) == 0)
			{
				cpssOsPrintf(".");
			}	

			sleep(1);		
			sec++;
		}
	}
	else
		cpssOsPrintf("\nStarting to send total packet=%d from dev %d port %d\n",totalPkt,devNum,pgPort);
	
	return rc;

}

GT_STATUS WNCPktGenStop
(
   IN int devNum,	
   IN GT_PHYSICAL_PORT_NUM pgPort
)
{
	GT_STATUS	rc;
	/*int devNum = 0;*/
	
	/* stop transmit*/
	rc = cpssDxChDiagPacketGeneratorTransmitEnable(devNum, pgPort, GT_FALSE);
	if(rc != GT_OK)
	{
		return rc;
	 }
	
	return rc;
}

GT_STATUS WNCLedSetting
(
   IN WNC_DEV_TYPE dev_type
)
{
	GT_STATUS	rc = GT_OK;
	int i;

	if ( dev_type == QUAGGA_24_PORT )
	{

		printf("WNC Configuare LED: Fury 24 ports Switch.\n");

      	        BoardType = QUAGGA_24_PORT;
		
		/*dev 0*/
		cpssDxChLedStreamDirectModeEnableSet(0,0,GT_TRUE);
		cpssDxChLedStreamDirectModeEnableSet(0,1,GT_TRUE);
		

		/*dev 0 position*/
		for(i=24;i<28;i++)
				cpssDxChLedStreamPortPositionSet(0,i,i);
			
		/* LED interface 1	: port 24, 25*/
		/* LED interface 0	: port 26, 27*/

		CPSS_LED_CONF_STC ledConfig;
		cpssOsMemSet(&ledConfig, 0, sizeof(ledConfig));

		ledConfig.ledOrganize=CPSS_LED_ORDER_MODE_BY_PORT_E;
		ledConfig.disableOnLinkDown=GT_FALSE;
		ledConfig.blink0DutyCycle=CPSS_LED_BLINK_DUTY_CYCLE_1_E;
		ledConfig.blink0Duration=CPSS_LED_BLINK_DURATION_5_E;
		ledConfig.blink1DutyCycle=CPSS_LED_BLINK_DUTY_CYCLE_0_E;
		ledConfig.blink1Duration=CPSS_LED_BLINK_DURATION_0_E;
		ledConfig.pulseStretch=CPSS_LED_PULSE_STRETCH_1_E;
		ledConfig.ledStart=0;
		ledConfig.ledEnd=255;
		ledConfig.clkInvert=GT_FALSE;
		ledConfig.class5select=CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
		ledConfig.class13select=CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
		ledConfig.invertEnable=GT_FALSE;
		ledConfig.ledClockFrequency=CPSS_LED_CLOCK_OUT_FREQUENCY_500_E;

		cpssDxChLedStreamConfigSet(0,1,&ledConfig); /*0/24,25*/
		cpssDxChLedStreamConfigSet(0,0,&ledConfig); /*0/26,27*/
		
		/* 0/24,25 set class indication of class 9~10 */
		cpssDxChLedStreamClassIndicationSet(0,1,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(0,1,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);

		 /*0/26,27 set class indication of class 9~10*/
		cpssDxChLedStreamClassIndicationSet(0,0,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(0,0,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);

		CPSS_LED_CLASS_MANIPULATION_STC led_class;
		cpssOsMemSet(&led_class, 0, sizeof(led_class));
  
		led_class.invertEnable=GT_TRUE;
		led_class.blinkEnable=GT_FALSE;
		led_class.blinkSelect=CPSS_LED_BLINK_SELECT_0_E;
		led_class.forceEnable=GT_FALSE;
		led_class.forceData=0;
		led_class.pulseStretchEnable=GT_FALSE;
		led_class.disableOnLinkDown=GT_FALSE;
	
		cpssDxChLedStreamClassManipulationSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*0/24,25*/
		cpssDxChLedStreamClassManipulationSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*0/26,27*/

		cpssOsMemSet(&led_class, 0, sizeof(led_class));
		
		led_class.invertEnable=GT_FALSE;
		led_class.blinkEnable=GT_TRUE;
		led_class.blinkSelect=CPSS_LED_BLINK_SELECT_0_E;
		led_class.forceEnable=GT_FALSE;
		led_class.forceData=0;
		led_class.pulseStretchEnable=GT_FALSE;
		led_class.disableOnLinkDown=GT_FALSE;

	 
		cpssDxChLedStreamClassManipulationSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*0/24,25*/
		cpssDxChLedStreamClassManipulationSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*0/26,27*/
		

		CPSS_LED_GROUP_CONF_STC led_group;
		cpssOsMemSet(&led_group, 0, sizeof(led_group));
		led_group.classA=9;
		led_group.classB=9;
		led_group.classC=10;
		led_group.classD=10;

		cpssDxChLedStreamGroupConfigSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*0/24*/
		cpssDxChLedStreamGroupConfigSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*0/25*/
		cpssDxChLedStreamGroupConfigSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*0/26*/
		cpssDxChLedStreamGroupConfigSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*0/27*/

	}
	else if ( dev_type == QUAGGA_48_PORT )
	{
		printf("WNC Configuare LED: Fury 48 ports Switch.\n");

		BoardType = QUAGGA_48_PORT;

		/*direct mode enable*/		
		/*dev 0*/
		cpssDxChLedStreamDirectModeEnableSet(0,0,GT_TRUE);
		cpssDxChLedStreamDirectModeEnableSet(0,1,GT_TRUE);
		/*dev 1*/
		cpssDxChLedStreamDirectModeEnableSet(1,0,GT_TRUE);
		cpssDxChLedStreamDirectModeEnableSet(1,1,GT_TRUE);
		
		/*dev 0 position*/
		for(i=24;i<28;i++)
			cpssDxChLedStreamPortPositionSet(0,i,i);
		
		/*dev 1 position*/
		for(i=24;i<28;i++)
			cpssDxChLedStreamPortPositionSet(1,i,i);


		CPSS_LED_CONF_STC ledConfig;
		cpssOsMemSet(&ledConfig, 0, sizeof(ledConfig));

		ledConfig.ledOrganize=CPSS_LED_ORDER_MODE_BY_PORT_E;
		ledConfig.disableOnLinkDown=GT_FALSE;
		ledConfig.blink0DutyCycle=CPSS_LED_BLINK_DUTY_CYCLE_1_E;
		ledConfig.blink0Duration=CPSS_LED_BLINK_DURATION_5_E;
		ledConfig.blink1DutyCycle=CPSS_LED_BLINK_DUTY_CYCLE_0_E;
		ledConfig.blink1Duration=CPSS_LED_BLINK_DURATION_0_E;
		ledConfig.pulseStretch=CPSS_LED_PULSE_STRETCH_1_E;
		ledConfig.ledStart=0;
		ledConfig.ledEnd=255;
		ledConfig.clkInvert=GT_FALSE;
		ledConfig.class5select=CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
		ledConfig.class13select=CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
		ledConfig.invertEnable=GT_FALSE;
		ledConfig.ledClockFrequency=CPSS_LED_CLOCK_OUT_FREQUENCY_500_E;

		cpssDxChLedStreamConfigSet(0,1,&ledConfig); /*dev 0*/
		cpssDxChLedStreamConfigSet(0,0,&ledConfig); /*dev 0*/
		cpssDxChLedStreamConfigSet(1,1,&ledConfig); /*dev 1*/
		cpssDxChLedStreamConfigSet(1,0,&ledConfig); /*dev 1*/

		/* 0/25,27 set class indication of class 9~10 */
		cpssDxChLedStreamClassIndicationSet(0,0,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(0,0,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);
		cpssDxChLedStreamClassIndicationSet(0,1,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(0,1,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);

		 /* 1/24,25 set class indication of class 9~10 */
		cpssDxChLedStreamClassIndicationSet(1,1,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(1,1,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);
		cpssDxChLedStreamClassIndicationSet(1,0,9,CPSS_DXCH_LED_INDICATION_LINK_E);
		cpssDxChLedStreamClassIndicationSet(1,0,10,CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);

		CPSS_LED_CLASS_MANIPULATION_STC led_class;
		cpssOsMemSet(&led_class, 0, sizeof(led_class));
  
		led_class.invertEnable=GT_TRUE;
		led_class.blinkEnable=GT_FALSE;
		led_class.blinkSelect=CPSS_LED_BLINK_SELECT_0_E;
		led_class.forceEnable=GT_FALSE;
		led_class.forceData=0;
		led_class.pulseStretchEnable=GT_FALSE;
		led_class.disableOnLinkDown=GT_FALSE;
	
		cpssDxChLedStreamClassManipulationSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*dev 0*/
		cpssDxChLedStreamClassManipulationSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*dev 0*/
		cpssDxChLedStreamClassManipulationSet(1,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*dev 1*/
		cpssDxChLedStreamClassManipulationSet(1,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,9,&led_class); /*dev 1*/
		
		cpssOsMemSet(&led_class, 0, sizeof(led_class));
		
		led_class.invertEnable=GT_FALSE;
		led_class.blinkEnable=GT_TRUE;
		led_class.blinkSelect=CPSS_LED_BLINK_SELECT_0_E;
		led_class.forceEnable=GT_FALSE;
		led_class.forceData=0;
		led_class.pulseStretchEnable=GT_FALSE;
		led_class.disableOnLinkDown=GT_FALSE;
	 
		cpssDxChLedStreamClassManipulationSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*dev 0*/
		cpssDxChLedStreamClassManipulationSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*dev 0*/
		cpssDxChLedStreamClassManipulationSet(1,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*dev 1*/
		cpssDxChLedStreamClassManipulationSet(1,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,10,&led_class); /*dev 1*/

		CPSS_LED_GROUP_CONF_STC led_group;
		cpssOsMemSet(&led_group, 0, sizeof(led_group));
		led_group.classA=9;
		led_group.classB=9;
		led_group.classC=10;
		led_group.classD=10;

		cpssDxChLedStreamGroupConfigSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*dev 0*/
		cpssDxChLedStreamGroupConfigSet(0,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*dev 0*/
		cpssDxChLedStreamGroupConfigSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*dev 0*/
		cpssDxChLedStreamGroupConfigSet(0,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*dev 0*/
		cpssDxChLedStreamGroupConfigSet(1,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*dev 1*/
		cpssDxChLedStreamGroupConfigSet(1,1,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*dev 1*/
		cpssDxChLedStreamGroupConfigSet(1,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,0,&led_group); /*dev 1*/
		cpssDxChLedStreamGroupConfigSet(1,0,CPSS_DXCH_LED_PORT_TYPE_XG_E,1,&led_group); /*dev 1*/

	}
	else
	{
		rc = GT_FAIL;

		printf("Error: Unknown devic type.\n");
	}

	return rc;

}

GT_STATUS WNCFCSetting
(
   IN WNC_DEV_TYPE dev_type
)
{
	GT_STATUS	rc = GT_OK;
	int i;

	if ( dev_type == QUAGGA_24_PORT )
	{

		printf("WNC Configuare FC setting: Fury 24 ports Switch.\n");

		BoardType = QUAGGA_24_PORT;

		/*Set Flow Control or HOL system mode on the specified device.*/
  	    	cpssDxChPortFcHolSysModeSet(0, CPSS_DXCH_PORT_FC_E);	/*dev 0*/

		for(i=24;i<28;i++)
		{
			cpssDxChPortFlowCntrlAutoNegEnableSet(0,i,GT_FALSE,GT_FALSE);
			cpssDxChPortFlowControlEnableSet(0,i,CPSS_PORT_FLOW_CONTROL_RX_TX_E);
		}
		
	}
	else if ( dev_type == QUAGGA_48_PORT )
	{
		printf("WNC Configuare FC setting: Fury 48 ports Switch.\n");

		BoardType = QUAGGA_48_PORT;

		/*Set Flow Control or HOL system mode on the specified device.*/
  	    	cpssDxChPortFcHolSysModeSet(0, CPSS_DXCH_PORT_FC_E);	/*dev 0*/
		cpssDxChPortFcHolSysModeSet(1, CPSS_DXCH_PORT_FC_E);	/*dev 1*/
		
		/*dev 0*/
		for(i=24;i<28;i++)
		{
			cpssDxChPortFlowCntrlAutoNegEnableSet(0,i,GT_FALSE,GT_FALSE);
			cpssDxChPortFlowControlEnableSet(0,i,CPSS_PORT_FLOW_CONTROL_RX_TX_E);
		}	
				
		/*dev 1*/
		for(i=24;i<28;i++)
		{
			cpssDxChPortFlowCntrlAutoNegEnableSet(1,i,GT_FALSE,GT_FALSE);
			cpssDxChPortFlowControlEnableSet(1,i,CPSS_PORT_FLOW_CONTROL_RX_TX_E);
		}	
				
	}
	else
	{
		rc = GT_FAIL;

		printf("Error: Unknown devic type.\n");
	}

	return rc;

}

GT_STATUS WNCSFPTXSetting
(
   IN WNC_DEV_TYPE dev_type
)
{
	GT_STATUS	rc = GT_OK;
	CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfg;
	int i;

	if ( dev_type == QUAGGA_24_PORT )
	{
		printf("WNC Configuare SFP TX  setting: Fury 24 ports Switch.\n");

		serdesCfg.txAmp=0x11; /*17*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x07;
		serdesCfg.txAmpShft=GT_FALSE;
		cpssDxChPortSerdesManualTxConfigSet(0,24,0,&serdesCfg); /*0/24*/
		cpssDxChPortSerdesManualTxConfigSet(0,25,0,&serdesCfg); /*0/25*/	
		
		serdesCfg.txAmp=0x11; /*17*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x06;
		serdesCfg.txAmpShft=GT_FALSE;		
			
		cpssDxChPortSerdesManualTxConfigSet(0,26,0,&serdesCfg); /*0/26*/
		cpssDxChPortSerdesManualTxConfigSet(0,27,0,&serdesCfg); /*0/27*/

		
	}
	else if ( dev_type == QUAGGA_48_PORT )
	{
		printf("WNC Configuare SFP TX setting: Fury 48 ports Switch.\n");

		BoardType = QUAGGA_48_PORT;

		serdesCfg.txAmp=0x11; /*17*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x06;
		serdesCfg.txAmpShft=GT_FALSE;
		cpssDxChPortSerdesManualTxConfigSet(0,24,0,&serdesCfg); /*0/24*/

		serdesCfg.txAmp=0x11; /*17*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x07;
		serdesCfg.txAmpShft=GT_FALSE;		
		cpssDxChPortSerdesManualTxConfigSet(0,25,0,&serdesCfg); /*0/25*/		
		cpssDxChPortSerdesManualTxConfigSet(1,25,0,&serdesCfg); /*1/25*/
		cpssDxChPortSerdesManualTxConfigSet(1,27,0,&serdesCfg); /*1/27*/

	}
	else
	{
		rc = GT_FAIL;

		printf("Error: Unknown devic type.\n");
	}

	return rc;

}

GT_STATUS WNCQSGMIISetting
(
   IN WNC_DEV_TYPE dev_type
)
{
	GT_STATUS	rc = GT_OK;
	CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfg;
	int i;

	if ( dev_type == QUAGGA_24_PORT )
	{
		printf("WNC Configuare QSGMII TX  setting: Fury 24 ports Switch.\n");
		
		serdesCfg.txAmp=0x0A; /*10*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x05;
		serdesCfg.txAmpShft=GT_FALSE;

		for(i=0;i<24;i++) /*port 0/0-23*/
		{
			cpssDxChPortSerdesManualTxConfigSet(0,i,0,&serdesCfg); 
		}	

	}
	else if ( dev_type == QUAGGA_48_PORT )
	{
		printf("WNC Configuare QSGMII TX  setting: Fury 48 ports Switch.\n");

		BoardType = QUAGGA_48_PORT;

		serdesCfg.txAmp=0x13; /*19*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x0A;/*10*/
		serdesCfg.txAmpShft=GT_FALSE;
		
		for(i=0;i<16;i++)  /*port 0/0-15*/
		{
			cpssDxChPortSerdesManualTxConfigSet(0,i,0,&serdesCfg); 
		}	
		
		serdesCfg.txAmp=0x11; /*17*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x09;
		serdesCfg.txAmpShft=GT_FALSE;

		for(i=16;i<24;i++) /*port 0/16-23*/
		{
			cpssDxChPortSerdesManualTxConfigSet(0,i,0,&serdesCfg); 
		}	
		for(i=0;i<8;i++) /*port 1/0-7*/
		{
			cpssDxChPortSerdesManualTxConfigSet(1,i,0,&serdesCfg); 
		}	
		
		
		serdesCfg.txAmp=0xC; /*12*/
		serdesCfg.txAmpAdjEn=GT_TRUE;
		serdesCfg.emph0=0x01;
		serdesCfg.emph1=0x08;
		serdesCfg.txAmpShft=GT_FALSE;

		for(i=8;i<24;i++) /*port 1/8-23*/
		{
			cpssDxChPortSerdesManualTxConfigSet(1,i,0,&serdesCfg); 
		}	

	}
	else
	{
		rc = GT_FAIL;

		printf("Error: Unknown devic type.\n");
	}

	return rc;

}

GT_STATUS WNCVODSetting
(
   IN WNC_DEV_TYPE dev_type
)
{
	GT_STATUS	rc = GT_OK;
	CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfg;
	int i;

	if ( dev_type == QUAGGA_24_PORT )
	{
		printf("WNC Configuare VOD setting: Fury 24 ports Switch.\n");

		for(i=0;i<24;i++)
		{
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x16,0x00FC);  /*register 22 data 0x00FC*/
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x12,0x5555);  /*register 18 data 0x5555*/
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x16,0x0000);  /*register 22 data 0x0000*/
		}
	}
	else if ( dev_type == QUAGGA_48_PORT )
	{
		printf("WNC Configuare VOD setting: Fury 48 ports Switch.\n");

		BoardType = QUAGGA_48_PORT;

		for(i=0;i<24;i++)
		{
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x16,0x00FC);  /*register 22 data 0x00FC*/
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x12,0x5555);  /*register 18 data 0x5555*/
			cpssDxChPhyPortSmiRegisterWrite(0,i,0x16,0x0000);  /*register 22 data 0x0000*/
		}

		for(i=0;i<24;i++)
		{
			cpssDxChPhyPortSmiRegisterWrite(1,i,0x16,0x00FC);  /*register 22 data 0x00FC*/
			cpssDxChPhyPortSmiRegisterWrite(1,i,0x12,0x5555);  /*register 18 data 0x5555*/
			cpssDxChPhyPortSmiRegisterWrite(1,i,0x16,0x0000);  /*register 22 data 0x0000*/
		}

	}
	else
	{
		rc = GT_FAIL;

		printf("Error: Unknown devic type.\n");
	}

	return rc;

}


GT_STATUS WNC_1GLedCtrl
(
	int port,
	int status
)
{
	int i,smiaddr,smiinf,dnum;
	
	/*printf("Boardtype = %d\n",BoardType);*/

	if ( status < 0 || status > 4)
	{
		cpssOsPrintf("Invaild status parameter! 0~3\n");
		return GT_BAD_PARAM;
	}	

	if(BoardType == QUAGGA_24_PORT)
	{
	
		if ( port < 1 || port > 24)
		{
			cpssOsPrintf("Invaild port parameter! 1~24\n");
			return GT_BAD_PARAM;
		}	

		dnum=0;

		if(port<17)
		{
			smiinf=0;
			smiaddr=port-1;
		}
		else
		{
			smiinf=1;
			smiaddr = port-9;
		}
			
		cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,22,0x0003); /*Switch register page*/		 
		
		switch(status)
		{
			case 1:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0089); /*Green*/
				break;
			case 2:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0098);/*Orange*/
				break;
			case 3:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0099); /*both*/
				break;
			default:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0088); /*force off*/
				break;
		}

		cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,22,0x0); /*Switch register page*/
	}	
	else
	{
	         /*portnum=48;*/
		if ( port < 1 || port > 48)
		{
			cpssOsPrintf("Invaild port parameter! 1~48\n");
			return GT_BAD_PARAM;
		}	

		if(port<17) /*1-16*/
		{
			dnum=0;
			smiinf=0;
			smiaddr=port-1;
		}
		else if( port > 16 && port < 25) /*17-24*/
		{
			dnum=0;
			smiinf=1;
			smiaddr = port-9;
		}		
		else if( port > 24 && port < 41) /*25-40*/
		{
			dnum=1;
			smiinf=0;
			smiaddr = port-25;
		}
		else /*41-48*/
		{
			dnum=1;
			smiinf=1;
			smiaddr = port-33;
		}	

		cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,22,0x0003); /*Switch register page*/

		switch(status)
		{
			case 1:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0089); /*Green*/
				break;
			case 2:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0098); /*Orange*/
				break;
			case 3:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0099); /*both*/
				break;
			default:
				cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,16,0x0088); /*force off*/
				break;
		}

		cpssSmiRegisterWriteShort(dnum,0,smiinf,smiaddr,22,0x0); /*Switch register page*/

	}	
	
}


static unsigned __TASKCONV taskPoeLedDemo
(
GT_VOID * param
)
{
	int i=0;
	GT_U16 portLoading;
	int ports;
	int greenLed,orangeLed,status;
	int dev,portNum;
	GT_BOOL isLinkUp;
	GT_STATUS rc;

	poeTest("i");
	poeTest("e");

	if(BoardType == QUAGGA_24_PORT)
		ports=24;
	else	
		ports=48;

	while(1)
	{
		sleep(1);
	
		for(i=1;i<(ports+1);i++)
		{
			/*initial status*/
			portLoading=0;
			greenLed=0;
			orangeLed=0;
			dev=0;
			isLinkUp=GT_FALSE;
			rc=GT_FAIL;
		
			poeGetPortLoading(i,&portLoading);
			
			if(portLoading>0)
			{
				/*cpssOsPrintf("Bernie port %d Loading%d\n",i,portLoading);*/
				greenLed=1;
			}

			if(i>24)
			{
				dev=1;
				portNum=i-25;
			}
			else
			{
				dev=0;
				portNum=i-1;
			}
			
			rc=cpssDxChPortLinkStatusGet(dev, portNum, &isLinkUp);

			if( isLinkUp == GT_TRUE && rc == GT_OK )
			{
				CPSS_PORT_SPEED_ENT   linkRate;
                    		cpssDxChPortSpeedGet(dev,portNum,&linkRate);

				if(linkRate == CPSS_PORT_SPEED_1000_E)
					orangeLed=1;
				/*cpssOsPrintf("linkRate=%d\n",linkRate);*/
				
			}
			/*else
				cpssOsPrintf("isLinkUp false\n");*/
				
			status=orangeLed*2 + greenLed;

			WNC_1GLedCtrl(i,status);	
			
		}
		
		/*cpssOsPrintf("Task i=%d\n",i);*/
		/*i++;*/
		/*cpssOsPrintf("\n");	*/
	}
	
	return 0;
}

GT_STATUS WNC_PoeLed_Demo()
{

GT_STATUS	rc = GT_OK;	

rc = osTaskCreate("poeLedDemoTask",
				  255,
				  0x20000,
				  taskPoeLedDemo,
				  NULL,
				  &tidPoeled_demo);

return rc;

}

