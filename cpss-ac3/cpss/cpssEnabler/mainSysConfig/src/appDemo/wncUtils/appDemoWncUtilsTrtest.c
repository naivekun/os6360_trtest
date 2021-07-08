#include <appDemo/wncUtils/appDemoWncUtilsTrtest.h>

/* The task Tid */
static GT_TASK tidCwTask[MAX_UNIT_CNS] = {0};;
static GT_TASK tidCwHighResTask[MAX_UNIT_CNS] = {0};
/* Tasks semaphores */
static CPSS_OS_SIG_SEM counterWatcherSigSem[MAX_UNIT_CNS] = {0};

static GT_U32 delCwTask[MAX_UNIT_CNS] = {0};
static GT_U32 delCwHighResTask[MAX_UNIT_CNS] = {0};
static GT_U32 hr_halt[MAX_UNIT_CNS] = {1};
static WNC_CW_INR_ETH_PORTS_STC cw_para[MAX_UNIT_CNS];
static WNC_CW_INR_ETH_PORTS_STC cw_hr_para[MAX_UNIT_CNS];
static GT_U32 clearIntrCounter[MAX_UNIT_CNS] = {0};
static GT_U32 clearIntrCounterPause[MAX_UNIT_CNS] = {0};

extern unsigned int sleep(unsigned int);
extern int usleep(unsigned int);
extern int nanosleep(const struct timespec *, struct timespec *);
extern GT_STATUS prvTgfPortMacCounterGet(IN  GT_U8, IN  GT_U32, IN  CPSS_PORT_MAC_COUNTERS_ENT, OUT GT_U64 *);
extern GT_STATUS cpssDxChTrunkCascadeTrunkPortsSet(IN GT_U8, IN GT_TRUNK_ID, IN CPSS_PORTS_BMP_STC*);
extern GT_STATUS cpssDxChPhyAutonegSmiSet(IN GT_U8, IN GT_PHYSICAL_PORT_NUM, IN GT_BOOL);
extern GT_STATUS cpssDxChPhyPortSmiRegisterWrite(IN GT_U8, IN GT_PHYSICAL_PORT_NUM, IN GT_U8, IN GT_U16);

/**
* @ osCmd function
*
* @brief   Execute system command.
*
* @param[in] cmdPtr                - input command
*
* @retval GT_OK                    - on success
*/
GT_STATUS osCmd
(
    IN  GT_CHAR *cmdPtr
)
{
	GT_STATUS	rc = GT_OK;

	cpssOsPrintf("Input command:%s\n",cmdPtr);
	
    system(cmdPtr);

	return rc;
}

/**
* @ wncUtilsTrtestClearCscdTrunkBmp function
*
* @brief   Clear internal port trunking
*
* @param[in] unit                     - device number
* @param[in] trunkId                  - trunk id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS wncUtilsTrtestClearCscdTrunkBmp
(
    IN GT_U8        unit,
    IN GT_TRUNK_ID  trunkId
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChTrunkCascadeTrunkPortsSet(unit, trunkId, NULL);

    return rc;
}

/**
* @ wncUtilsTrtestClearCscdTrunkBmp function
*
* @brief   Restore internal port trunking
*
* @param[in] unit                     - device number
* @param[in] trunkId                  - trunk id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS wncUtilsTrtestRestoreCscdTrunkBmp
(
    IN GT_U8        unit,
    IN GT_TRUNK_ID  trunkId
)
{
    GT_STATUS   rc = GT_BAD_PARAM;
    GT_U32      i;                  /* Loop index */
    GT_U8       portNum;
#if 0
    CPSS_CSCD_PORT_TYPE_ENT     cscdPortType;       /* Enum of cascade port or network port */
#endif
    GT_TRUNK_ID                 trunkId_from_cfg;   /* the cascade trunkId */
    CPSS_PORTS_BMP_STC          cscdTrunkBmp;       /* bmp of ports members in the cascade trunk */

    if(appDemoPpConfigList[unit].numberOfCscdTrunks)
    {
        /* clear the local ports bmp for the new cascade trunk */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cscdTrunkBmp);

        for(i = 0; i < appDemoPpConfigList[unit].numberOfCscdPorts; i++)
        {
            portNum = appDemoPpConfigList[unit].cscdPortsArr[i].portNum;
#if 0
            cscdPortType = appDemoPpConfigList[unit].cscdPortsArr[i].cscdPortType;
#endif
            trunkId_from_cfg = appDemoPpConfigList[unit].cscdPortsArr[i].trunkId;

            if(trunkId_from_cfg != trunkId)
            {
                continue;
            }

            CPSS_PORTS_BMP_PORT_SET_MAC(&cscdTrunkBmp, portNum);
            rc = cpssDxChTrunkCascadeTrunkPortsSet(unit, trunkId, &cscdTrunkBmp);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTrunkCascadeTrunkPortsSet", rc);
        }
    }
    return rc;
}

/**
* @internal time_diff function
* @endinternal
*
* @brief   Calculate the difference between start and end time
*
* @param[in] start_sec              - input start seconds
* @param[in] start_nsec             - input start nanoseconds
* @param[in] end_sec                - input end seconds
* @param[in] end_nsec               - input end nanoseconds
*
* @retval rtn                    - difference between start and end time in second
*/
static GT_FLOAT32 time_diff
(
    GT_U32 start_sec,
    GT_U32 start_nsec,
    GT_U32 end_sec,
    GT_U32 end_nsec
)
{
    GT_FLOAT32 rtn;
    GT_U32  diff_sec  = 0;
    GT_U32  diff_nsec = 0;

    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec -= 1;
    }

    diff_sec = end_sec - start_sec;
    diff_nsec = end_nsec - start_nsec;

    /* return in second */
    rtn = diff_sec + (diff_nsec / (GT_FLOAT32)1000000000);

    return rtn;
}

/**
* @internal getPortNumber
* @endinternal
*
* @brief   Switch port number from string to digital
*
* @param[in] lportStr              - input front panel port number string
* @param[in] portNumStr            - input physical port number string
*
* @retval portNum                  - physical port number with digital format
*/
static GT_U32 getPortNumber
(
	IN GT_CHAR  lportStr[16],
    IN GT_CHAR  portNumStr[16]
)
{
	GT_CHAR		portNumSkipIStr[16];
	GT_U8		p = 0, c = 0;
	GT_U32		ascii_c = 0;
	GT_U32		portNum = 0;

	/* internal port case, LPort format ex: i11 */
	if(lportStr[0]=="i")
	{
		for(p=0;portNumStr[p]!='\0';p++)
		{
			/* portNumStr, internal PPort format ex: 26i */
			/* check characters is number or not by ascii */
			ascii_c = portNumStr[p];
			if( (ascii_c < 48) || (ascii_c > 57) )
			{
				break;
			}
			else
			{
				portNumSkipIStr[c] = portNumStr[p];
				c++;
			}
		}
		
		portNum = atoi(portNumSkipIStr);
	}
	else /* normal port case, ex 20 */
	{
		portNum = atoi(portNumStr);
	}

	return portNum;
}

/**
* @internal getVlanMemberType
* @endinternal
*
* @brief   Switch vlan tagging tpye from string to id
*
* @param[in] vlanMemberTypeStr      - input vlan tagging tpye string
*
* @retval vlanMemberType            - vlan tagging type id
*/
static GT_U8 getVlanMemberType
(
    IN GT_CHAR  vlanMemberTypeStr[16]
)
{
	GT_U8 vlanMemberType = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

	if (!cpssOsStrNCmp(vlanMemberTypeStr,"Untagged", sizeof("Untagged")))
	{
		vlanMemberType = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
	}
	else if (!cpssOsStrNCmp(vlanMemberTypeStr,"Tag0", sizeof("Tag0")))
	{
		vlanMemberType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
	}
	else if (!cpssOsStrNCmp(vlanMemberTypeStr,"Tag1", sizeof("Tag1")))
	{
		vlanMemberType = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
	}

	return vlanMemberType;
}

/**
* @internal getLoopBackMode
* @endinternal
*
* @brief   Switch loopBack mode tpye from string to id
*
* @param[in] vlanMemberTypeStr      - input loopBack mode tpye string
*
* @retval loopBackMode            - loopBack mode tpye id
*/
static GT_U8 getLoopBackMode
(
    IN GT_CHAR  loopBackModeStr[16]
)
{
	GT_U8 loopBackMode = APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_NONE;

	if (!strncmp(loopBackModeStr,"ExtPhy", sizeof("ExtPhy")))
	{
		loopBackMode = APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_PHYEXT;
	}

	return loopBackMode;
}

/**
* @internal getPhyType
* @endinternal
*
* @brief   Switch phy tpye from string to id
*
* @param[in] phyTypeStr      - input phy tpye string
*
* @retval phyType            - phy tpye id
*/
static GT_U32 getPhyType
(
    IN GT_CHAR  phyTypeStr[16]
)
{
	GT_U32 phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_NONE;

	if (!cpssOsStrNCmp(phyTypeStr,"88E1680", sizeof("88E1680")))
	{
		phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_88E1680;
	}
	else if (!cpssOsStrNCmp(phyTypeStr,"88E1543", sizeof("88E1543")))
	{
		phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_88E1543;
	}
	else if (!cpssOsStrNCmp(phyTypeStr,"88E2010", sizeof("88E2010")))
	{
		phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_88E2010;
	}
	else if (!cpssOsStrNCmp(phyTypeStr,"IntPHY", sizeof("IntPHY")))
	{
		phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_INTPHY;
	}
	else
	{
		phyType = APPDEMO_WNC_UTILS_TRTEST_PHY_NONE;
	}

	return phyType;
}

/**
* @internal getMfgModePortInfo
* @endinternal
*
* @brief   Get port mapping and vlan related info from file for MFG mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS getMfgModePortInfo
(
    GT_VOID
)
{
	GT_STATUS   rc = GT_OK;
	FILE        *fp = NULL;
	GT_CHAR		lineBuff[256];
	GT_U8		parsingFlag = GT_FALSE;
	GT_CHAR		lportStr[8];
	GT_U32		vlan;
	GT_U8		devNum;
	GT_CHAR		portNumStr[16],vlanMemberTypeStr[16],loopBackModeStr[16],phyTypeStr[16];
	GT_U32		portCount = 0;
	GT_U8		devMax = 0;
#if 0
	int i;
#endif
	/* open port info file for MFG mode */
    if((fp = fopen(TF_PORTINFO_MFG_CONFIG, "r")) != NULL)
    {
    	while ( fgets( lineBuff, sizeof(lineBuff), fp ) != NULL )
		{
			if(parsingFlag) /* start parsing port info */
			{
				if ( sscanf(lineBuff,"%s %d/%s %d %s %s %s", lportStr, &devNum, &portNumStr, &vlan, vlanMemberTypeStr, loopBackModeStr, phyTypeStr)==7)
				{
					strncpy(mfgPortInfo[portCount].lport, lportStr, sizeof(lportStr));
					mfgPortInfo[portCount].devNum = devNum;
					mfgPortInfo[portCount].portNum = getPortNumber(lportStr, portNumStr);
					mfgPortInfo[portCount].vlan = vlan;
					mfgPortInfo[portCount].vlanMemberType = getVlanMemberType(vlanMemberTypeStr);
					mfgPortInfo[portCount].loopBackMode = getLoopBackMode(loopBackModeStr);
					mfgPortInfo[portCount].phyType = getPhyType(phyTypeStr);
					portCount++;

					/* record the maximum MAC device unit number */
					if(devNum > devMax)
					{
						devMax = devNum;
					}
				}
			}
			else /* read line form file util get ---- */
			{
				if (!cpssOsStrNCmp(lineBuff,"----",4))
				{
					parsingFlag = GT_TRUE;
				}
			}
	    }
		fclose(fp);

		portmapCount = portCount;
		devMacNum = devMax+1;
    }
	else
	{
		cpssOsPrintf("fopen() %s failed!\n", "/etc/opt/asic/portinfo_mfg");
		/*cpssOsPrintf("Load default!");*/
		rc = GT_FAIL;
	}
	
#if 0
	cpssOsPrintf("WNC_GetMfgModePortInfo: portmapCount=%d\n",portmapCount);

	for(i = 0; i < portmapCount; i++)
	{
		cpssOsPrintf("WNC_GetMfgModePortInfo: lport=%s,devNum=%d,portNum=%d\n",mfgPortInfo[i].lport,
			mfgPortInfo[i].devNum,mfgPortInfo[i].portNum);
		cpssOsPrintf("WNC_GetMfgModePortInfo: vlan=%d,loopBackMode=%d,phyType=%d\n",mfgPortInfo[i].vlan,
			mfgPortInfo[i].loopBackMode,mfgPortInfo[i].phyType);
	}
#endif
	return rc;
}

/**
* @internal getPortMappingTable
* @endinternal
*
* @brief   Get port mapping info from file
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS getPortMappingTable
(
    GT_VOID
)
{
	FILE        *fp = NULL;
	GT_STATUS   rc = GT_OK;
	GT_CHAR		lineBuff[256];
	GT_U8		parsingFlag = GT_FALSE;
	GT_U8		devNum;
	GT_U32		portNum;
	GT_U32		portCount=0;
	GT_U8		devMax=0;
	GT_CHAR		lportStr[8];

#if 0
	int i;
#endif
	/* open system info file for get port mapping  */
    if((fp = fopen(TF_SYSINFO_CONFIG, "r")) != NULL)
    {
    	while ( fgets( lineBuff, sizeof(lineBuff), fp ) != NULL )
		{
			if(parsingFlag) /* start parsing port mapping info */
			{
				if ( sscanf(lineBuff,"%s %d/%d", lportStr, &devNum, &portNum)==3)
				{
					strncpy(portMapInfo[portCount].lport, lportStr, sizeof(lportStr));
					portMapInfo[portCount].devNum = devNum;
					portMapInfo[portCount].portNum = portNum;
					portCount++;

					if(devNum > devMax)
					{
						devMax = devNum;
					}
				}
			}
			else /* read line form file util get ---- */
			{
				if (!cpssOsStrNCmp(lineBuff,"----",4))
				{
					parsingFlag = GT_TRUE;
				}
			}
	    }
		fclose(fp);

		portmapCount = portCount;
		devMacNum = devMax+1;
    }
	else
	{
		cpssOsPrintf("fopen() %s failed!\n", "/etc/opt/asic/sysinfo");
		/*cpssOsPrintf("Load default!");*/
		rc = GT_FAIL;
	}
#if 0
	cpssOsPrintf("WNC_GetPortMappingTable: portmapCount=%d\n",portmapCount);

	for(i = 0; i < portmapCount; i++)
	{
		cpssOsPrintf("WNC_GetPortMappingTable: lport=%s,devNum=%d,portNum=%d\n",portMapInfo[i].lport,
			portMapInfo[i].devNum,portMapInfo[i].portNum);
	}
#endif

	return rc;
}

/**
* @ wncUtilsTrtestMacCounterClearOnRead
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*
* @param[in] devNum                   - device number.
* @param[in] plist                   - physical port list
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable devic
*/
GT_STATUS wncUtilsTrtestMacCounterClearOnRead
(
    IN GT_U8    unit,
    IN GT_CHAR  plist[256],
    IN GT_U8    enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      portNum[MAX_PORTS_CNS] = {0};
    GT_U32      portCount = 0;
    GT_CHAR_PTR sptr;

    /* Get port index array */
    if(cpssOsStrCmp(plist, "all") == 0)
    {
        portCount = portmapCount;

        for(i = 0; i < portCount ; i++)
        {
            if(portMapInfo[i].devNum == unit)
            {
                portNum[i] = portMapInfo[i].portNum;
            }
        }
    }
    else
    {
        i = 0;
        sptr = strtok(plist, ",");
        while(sptr != NULL)
        {
            portNum[i] = osStrTo32(sptr);
            i++;
            sptr = strtok(NULL, ",");
        }
        portCount = i;
    }

    /* Enable/disable read and clean */
    for (i = 0; i < portCount; i++)
    {
        rc = cpssDxChPortMacCountersClearOnReadSet(unit, portNum[i], (enable == 1)?GT_TRUE:GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortMacCountersClearOnReadSet() set false failed on etherport %d/%d!\n",
                          unit, portNum[i]);
        }
    }

    return rc;
}

/**
* @internal taskCounterWatcherStart
* @endinternal
*
* @brief   Get port mapping info from file
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static unsigned __TASKCONV taskCounterWatcherStart
(
    GT_VOID *param
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_FLOAT32  tsDelta;
    GT_U8       devNum = 0;
    GT_U32      portNum;
    GT_U32      cntrNameIdx;
    GT_U64      tmp;
    WNC_CW_INR_ETH_PORTS_STC    *cw_para_ptr = (WNC_CW_INR_ETH_PORTS_STC *)param;
    struct sched_param para = {.sched_priority = sched_get_priority_max(SCHED_FIFO)};
    /*cpu_set_t cpuset;*/

    /* Tx */
    GT_U64  cw_tpok_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  cw_tpng_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  cw_tpok_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    /* Rx */
    GT_U64  cw_rpok_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  cw_rpng_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  cw_rpok_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};

    GT_U32  numOfCntrsTxGood = 3;
    GT_U32  numOfCntrsRxGood = 3;
    GT_U32  numOfCntrsTxNg = 1;
    GT_U32  numOfCntrsRxNg = 6;
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxGood[numOfCntrsTxGood];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxGood[numOfCntrsRxGood];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxNg[numOfCntrsTxNg];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxNg[numOfCntrsRxNg];

    GT_U64  txPktDelta[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U64  rxPktDelta[MAX_UNIT_CNS][MAX_PORTS_CNS];

    /* start and end time seconds part */
    GT_U32  startSec_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  startSec_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endSec_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endSec_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    /* start and end time nanoseconds part */
    GT_U32  startNano_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  startNano_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endNano_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endNano_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];

    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &para) != 0)
    {
        cpssOsPrintf("fail to configure scheduler!\n");
    }

    if(mlockall(MCL_CURRENT) != 0)
    {
        cpssOsPrintf("fail to lock virtual memory!\n");
    }

    /*CPU_ZERO(&cpuset); CPU_SET(0, &cpuset);
    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
    {
        cpssOsPrintf("fail to configure CPU affinity\n");
    }*/

    cntrNameArrRxGood[0] = CPSS_BRDC_PKTS_RCV_E;
    cntrNameArrRxGood[1] = CPSS_MC_PKTS_RCV_E;
    cntrNameArrRxGood[2] = CPSS_GOOD_UC_PKTS_RCV_E;

    cntrNameArrTxGood[0] = CPSS_BRDC_PKTS_SENT_E;
    cntrNameArrTxGood[1] = CPSS_MC_PKTS_SENT_E;
    cntrNameArrTxGood[2] = CPSS_GOOD_UC_PKTS_SENT_E;

    cntrNameArrRxNg[0] = CPSS_UNDERSIZE_PKTS_E;
    cntrNameArrRxNg[1] = CPSS_OVERSIZE_PKTS_E;
    cntrNameArrRxNg[2] = CPSS_JABBER_PKTS_E;
    cntrNameArrRxNg[3] = CPSS_MAC_RCV_ERROR_E;
    cntrNameArrRxNg[4] = CPSS_BAD_CRC_E;
    cntrNameArrRxNg[5] = CPSS_FRAGMENTS_PKTS_E;

    cntrNameArrTxNg[0] = CPSS_MAC_TRANSMIT_ERR_E;

    for (i = 0; i < cw_para_ptr->numCount; i++)
    {
        devNum = cw_para_ptr->devNumArr[i];
        portNum = cw_para_ptr->portNumArr[i];

        memset(&cw_tpps[devNum][portNum], 0, sizeof(cw_tpps[devNum][portNum]));
        memset(&cw_rpps[devNum][portNum], 0, sizeof(cw_rpps[devNum][portNum]));
        memset(&cw_tpok[devNum][portNum], 0, sizeof(cw_tpok[devNum][portNum]));
        memset(&cw_tpng[devNum][portNum], 0, sizeof(cw_tpng[devNum][portNum]));
        memset(&cw_rpok[devNum][portNum], 0, sizeof(cw_rpok[devNum][portNum]));
        memset(&cw_rpng[devNum][portNum], 0, sizeof(cw_rpng[devNum][portNum]));
    }

    /* get counter 1st time */
    for (i = 0; i < cw_para_ptr->numCount; i++)
    {
        devNum = cw_para_ptr->devNumArr[i];
        portNum = cw_para_ptr->portNumArr[i];

        /* Rx pkts ok */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxGood; cntrNameIdx++)
        {
            rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxGood[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_rpok_prev[devNum][portNum] = prvCpssMathAdd64(cw_rpok_prev[devNum][portNum], tmp);
        }
        /* store 1st Rx pkts ok timestamp */
        cpssOsTimeRT(&startSec_rpok[devNum][portNum], &startNano_rpok[devNum][portNum]);

        /* Tx pkts ok */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxGood; cntrNameIdx++)
        {
            rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxGood[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_tpok_prev[devNum][portNum] = prvCpssMathAdd64(cw_tpok_prev[devNum][portNum], tmp);
        }
        /* store 1st Tx pkts ok timestamp */
        cpssOsTimeRT(&startSec_tpok[devNum][portNum], &startNano_tpok[devNum][portNum]);

        /* 
           Since there might have time lag when the counter watcher started: like
           MAC got NG packet at the very beginning, said 0.1 sec,
           but the thread started to get counters at 0.2 sec, then the difference later will be zero.
           No need to get NG counters at the beginning, it should be zero.
        */
    }

    cpssOsSigSemWait(counterWatcherSigSem[devNum], CPSS_OS_SEM_WAIT_FOREVER_CNS);

    /* get counter 2nd time */
    for (i = 0; i < cw_para_ptr->numCount; i++)
    {
        devNum = cw_para_ptr->devNumArr[i];
        portNum = cw_para_ptr->portNumArr[i];

        /* Rx pkts ok */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxGood; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxGood[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_rpok_curr[devNum][portNum] = prvCpssMathAdd64(cw_rpok_curr[devNum][portNum], tmp);
        }
        /* store 2nd Rx pkts ok timestamp */
        cpssOsTimeRT(&endSec_rpok[devNum][portNum], &endNano_rpok[devNum][portNum]);

        /* Tx pkts ok */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxGood; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxGood[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_tpok_curr[devNum][portNum] = prvCpssMathAdd64(cw_tpok_curr[devNum][portNum], tmp);
        }
        /* store 2nd Tx pkts ok timestamp */
        cpssOsTimeRT(&endSec_tpok[devNum][portNum], &endNano_tpok[devNum][portNum]);

        /* Rx pkts NG */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxNg; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxNg[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_rpng_curr[devNum][portNum] = prvCpssMathAdd64(cw_rpng_curr[devNum][portNum], tmp);
        }

        /* Tx pkts NG */
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxNg; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxNg[cntrNameIdx], &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cw_tpng_curr[devNum][portNum] = prvCpssMathAdd64(cw_tpng_curr[devNum][portNum], tmp);
        }
    }

    for (i = 0; i < cw_para_ptr->numCount; i++)
    {
        devNum = cw_para_ptr->devNumArr[i];
        portNum = cw_para_ptr->portNumArr[i];

        /* get delta result */
        rxPktDelta[devNum][portNum] = prvCpssMathSub64(cw_rpok_curr[devNum][portNum], cw_rpok_prev[devNum][portNum]);
        txPktDelta[devNum][portNum] = prvCpssMathSub64(cw_tpok_curr[devNum][portNum], cw_tpok_prev[devNum][portNum]);

        /* Rx rate calculation */
        tsDelta = time_diff(startSec_rpok[devNum][portNum], startNano_rpok[devNum][portNum],
                            endSec_rpok[devNum][portNum], endNano_rpok[devNum][portNum]);
        cw_rpps[devNum][portNum] = (((uint64_t)rxPktDelta[devNum][portNum].l[1] << 32) + rxPktDelta[devNum][portNum].l[0]) / tsDelta;

        /* Tx rate calculation */
        tsDelta = time_diff(startSec_tpok[devNum][portNum], startNano_tpok[devNum][portNum],
                            endSec_tpok[devNum][portNum], endNano_tpok[devNum][portNum]);
        cw_tpps[devNum][portNum] = (((uint64_t)txPktDelta[devNum][portNum].l[1] << 32) + txPktDelta[devNum][portNum].l[0]) / tsDelta;

        /* update TX & RX packet number */
        cw_rpok[devNum][portNum] = cw_rpok_curr[devNum][portNum];
        cw_tpok[devNum][portNum] = cw_tpok_curr[devNum][portNum];
        cw_rpng[devNum][portNum] = cw_rpng_curr[devNum][portNum];
        cw_tpng[devNum][portNum] = cw_tpng_curr[devNum][portNum];
    }

    if(munlockall() != 0)
    {
        cpssOsPrintf("fail to unlock virtual memory!\n");
    }

    delCwTask[devNum] = 1;

    return rc;
}

/**
* @ wncUtilsTrtestCounterWatcherStart
*
* @brief   Start the counter watcher task
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestCounterWatcherStart
(
    IN GT_U8    unit
)
{
    GT_U32      i;
    GT_U32      j;      /* Port list index to thread, according to input unit */
    GT_STATUS   rc = GT_OK;	
    GT_CHAR     semName[32], taskName[32];

    if(tidCwTask[unit] != 0)
    {
        cpssOsPrintf("Couter watcher thread %d exists!\n", unit);
        return GT_ALREADY_EXIST;
    }

    cpssOsSnprintf(semName, sizeof(semName), "counterWatcherSigSem_%d", unit);
    cpssOsSnprintf(taskName, sizeof(taskName), "cwTask_%d", unit);

    if(cpssOsSigSemBinCreate(semName, CPSS_OS_SEMB_EMPTY_E, &counterWatcherSigSem[unit]) != GT_OK)
    {
        cpssOsPrintf("Couter watcher %d sem create failed!\n", unit);
        return GT_NO_RESOURCE;
    }

    /* Map portmapping array to devNumArr and portNumArr */
    memset(&cw_para[unit], 0, sizeof(WNC_CW_INR_ETH_PORTS_STC));
    j = 0;
    for(i = 0; i < portmapCount; i++)
    {
        if(portMapInfo[i].devNum == unit)
        {
            cw_para[unit].devNumArr[j] = portMapInfo[i].devNum;
            cw_para[unit].portNumArr[j] = portMapInfo[i].portNum;
            j++;
        }
    }
    if(0 == j)
    {
        cpssOsPrintf("Unit [%d] is not supported!\n", unit);
        return GT_BAD_PARAM;
    }
    else
    {
        cw_para[unit].numCount = j;
    }

    rc = cpssOsTaskCreate ( taskName,                   /* Task Name      */
                            0,                          /* Task Priority  */
                            0x20000,                    /* Stack Size     */
                            taskCounterWatcherStart,    /* Starting Point */
                            (GT_VOID*)(&cw_para[unit]), /* Arguments list */
                            &tidCwTask[unit]);          /* task ID        */
    if(rc != GT_OK)
    {
        cpssOsPrintf("cwTask_%d create failed!\n", unit);
    }

    return rc;
}

/**
* @ wncUtilsTrtestCounterWatcherStop
*
* @brief   Stop the counter watcher task and dump counter result to file
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestCounterWatcherStop
(
    IN GT_U8    unit
)
{
    GT_STATUS   rc = GT_OK;	
    GT_U32      i;
    GT_32       timeout = 5000000; /* 5 microsecond */
    /* output result */
    GT_U8       devNum;
    GT_U32      portNum;
    GT_CHAR     rxCounter[23];
    GT_CHAR     txCounter[23];
    GT_CHAR     fpath[32];
    FILE        *fp = NULL;

    if(tidCwTask[unit] == 0)
    {
        cpssOsPrintf("Couter watcher thread %d not exist!\n", unit);
        return GT_NO_SUCH;
    }

    rc = cpssOsSigSemSignal(counterWatcherSigSem[unit]);
    if(rc != GT_OK)
    {
        cpssOsPrintf("Counter watcher %d stop cpssOsDigSemSignal failed!\n", unit);
        return rc;
    }

    while(delCwTask[unit] != 1)
    {
        if (timeout <= 0)
        {
            cpssOsPrintf("Wait couter watcher %d terminated timeout!\n", unit);
            tidCwTask[unit] = 0;
            return GT_TIMEOUT;
        }
        timeout -= 10000;
        usleep(10000);  /* 0.01ms */
    }

    cpssOsSigSemDelete(counterWatcherSigSem[unit]);
    cpssOsTaskDelete(tidCwTask[unit]);
    tidCwTask[unit] = 0;
    delCwTask[unit] = 0;

    /* display result */
    cpssOsSnprintf(fpath, sizeof(fpath), "/tmp/cw_result_%d", unit);
    if((fp = fopen(fpath, "w+")) != NULL)
    {
        /* Dump counter result to file first */
        fprintf(fp, "Port  Rx PPS    Tx PPS    Rx Pkts (Good/NG)      Tx Pkts (Good/NG)\n");
        fprintf(fp, "----- --------- --------- ---------------------- ----------------------\n");

        /* Show by lport order */
        for(i = 0; i < portmapCount; i++)
        {
            devNum  = portMapInfo[i].devNum;
            if(devNum != unit)
                continue;

            portNum = portMapInfo[i].portNum;

            /* GT_U64: 32-bit CPU use %llu, 64-bit use %lu */
            cpssOsSnprintf(rxCounter, sizeof(rxCounter), "%llu/%llu",
                           ((uint64_t)cw_rpok[devNum][portNum].l[1] << 32) + cw_rpok[devNum][portNum].l[0],
                           ((uint64_t)cw_rpng[devNum][portNum].l[1] << 32) + cw_rpng[devNum][portNum].l[0]);
            cpssOsSnprintf(txCounter, sizeof(txCounter), "%llu/%llu",
                           ((uint64_t)cw_tpok[devNum][portNum].l[1] << 32) + cw_tpok[devNum][portNum].l[0],
                           ((uint64_t)cw_tpng[devNum][portNum].l[1] << 32) + cw_tpng[devNum][portNum].l[0]);

            fprintf(fp, "%-5s|%9lu|%9lu|%22s|%22s\n",
                         portMapInfo[i].lport,
                         cw_rpps[devNum][portNum],
                         cw_tpps[devNum][portNum],
                         rxCounter, txCounter);
        }
        fclose(fp);
    }
    else
    {
        cpssOsPrintf("fopen() %s failed!\n", fpath);
        rc = GT_FAIL;
    }

    return rc;
}

/**
* @ update64bitUpperValueWithPrev
*
* @brief   update 64bit upper value with previus value
*
* @param[in] prevValue                   - previus value
* @param[out] currValue                  - current value
*
*/
static GT_VOID update64bitUpperValueWithPrev
(
    IN GT_U64   *prevValue,
    IN GT_U64   *currValue
)
{
    /*
       For API that getting counter result only update lower 32 bit value.
       We first recall prevValue->l[1], and carry one manually if needed.
     */
    currValue->l[1] = prevValue->l[1];
    if(currValue->l[0] < prevValue->l[0])
    {
        /* Current lower 32 bit value should be always >= previous one, if
           we update in sec level. If not, it should be carried one to upper
           32 bit.
         */
        currValue->l[1]++;
    }

    /* Sync value in the end */
    *prevValue = *currValue;
}

/**
* @internal taskCounterWatcherHighResStart
* @endinternal
*
* @brief   the start task for counter watcher with high resolution mode
*
* @param[in] vlanMemberTypeStr      - input loopBack mode tpye string
*
* @retval loopBackMode            - loopBack mode tpye id
*/
static unsigned __TASKCONV taskCounterWatcherHighResStart
(
    GT_VOID *param
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  i;
    GT_FLOAT32  tsDelta;
    GT_U8       devNum = 0;
    GT_U32      portNum;
    GT_U32      cntrNameIdx;
    GT_U64      tmp;
    GT_U8       skipFirstRun = 1;
    GT_U32      wait_clear_cnt = 0;
    WNC_CW_INR_ETH_PORTS_STC    *cw_hr_para_ptr = (WNC_CW_INR_ETH_PORTS_STC *)param;
    struct sched_param para = {.sched_priority = sched_get_priority_max(SCHED_FIFO)};
    /*cpu_set_t cpuset;*/
    struct timespec ts_pause = {1, 0};
    struct timespec wait_pause = {0, 100000000}; /* 100ms */

    /* Tx */
    GT_U64  hr_tpok_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_tpng_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_tpok_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_tpng_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_txbyte_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_txerr_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_txdisc_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_txpaus_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    /* Rx */
    GT_U64  hr_rpok_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rpng_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rpok_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rpng_prev[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rxbyte_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rxerr_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rxdisc_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
    GT_U64  hr_rxpaus_curr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};

    GT_U32  numOfCntrsTxGood = 3;
    GT_U32  numOfCntrsRxGood = 3;
    GT_U32  numOfCntrsTxNg = 1;
    GT_U32  numOfCntrsRxNg = 6;
    GT_U32  numOfCntrsTxByte = 1;
    GT_U32  numOfCntrsRxByte = 1;
    GT_U32  numOfCntrsTxErr = 1;
    GT_U32  numOfCntrsRxErr = 1;
    GT_U32  numOfCntrsTxDisc = 1;
    GT_U32  numOfCntrsRxDisc = 1;
    GT_U32  numOfCntrsTxPaus = 1;
    GT_U32  numOfCntrsRxPaus = 1;

    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxGood[numOfCntrsTxGood];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxGood[numOfCntrsRxGood];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxNg[numOfCntrsTxNg];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxNg[numOfCntrsRxNg];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxByte[numOfCntrsTxByte];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxByte[numOfCntrsRxByte];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxErr[numOfCntrsTxErr];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxErr[numOfCntrsRxErr];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxDisc[numOfCntrsTxDisc];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxDisc[numOfCntrsRxDisc];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrTxPaus[numOfCntrsTxPaus];
    CPSS_PORT_MAC_COUNTERS_ENT  cntrNameArrRxPaus[numOfCntrsRxPaus];

    GT_U64  txPktDelta[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U64  rxPktDelta[MAX_UNIT_CNS][MAX_PORTS_CNS];

    /* start and end time seconds part */
    GT_U32  startSec_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  startSec_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endSec_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endSec_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    /* start and end time nanoseconds part */
    GT_U32  startNano_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  startNano_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endNano_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS];
    GT_U32  endNano_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS];

    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &para) != 0)
    {
        cpssOsPrintf("fail to configure scheduler!\n");
    }

    if(mlockall(MCL_CURRENT) != 0)
    {
        cpssOsPrintf("fail to lock virtual memory!\n");
    }

    /*CPU_ZERO(&cpuset); CPU_SET(0, &cpuset);
    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
    {
        cpssOsPrintf("fail to configure CPU affinity\n");
    }*/

    cntrNameArrRxGood[0] = CPSS_BRDC_PKTS_RCV_E;
    cntrNameArrRxGood[1] = CPSS_MC_PKTS_RCV_E;
    cntrNameArrRxGood[2] = CPSS_GOOD_UC_PKTS_RCV_E;

    cntrNameArrTxGood[0] = CPSS_BRDC_PKTS_SENT_E;
    cntrNameArrTxGood[1] = CPSS_MC_PKTS_SENT_E;
    cntrNameArrTxGood[2] = CPSS_GOOD_UC_PKTS_SENT_E;

    cntrNameArrRxNg[0] = CPSS_UNDERSIZE_PKTS_E;
    cntrNameArrRxNg[1] = CPSS_OVERSIZE_PKTS_E;
    cntrNameArrRxNg[2] = CPSS_JABBER_PKTS_E;
    cntrNameArrRxNg[3] = CPSS_MAC_RCV_ERROR_E;
    cntrNameArrRxNg[4] = CPSS_BAD_CRC_E;
    cntrNameArrRxNg[5] = CPSS_FRAGMENTS_PKTS_E;

    cntrNameArrTxNg[0] = CPSS_MAC_TRANSMIT_ERR_E;

    cntrNameArrRxByte[0] = CPSS_GOOD_OCTETS_RCV_E;
    cntrNameArrTxByte[0] = CPSS_GOOD_OCTETS_SENT_E;
    
    cntrNameArrRxErr[0] = CPSS_MAC_RCV_ERROR_E;
    cntrNameArrTxErr[0] = CPSS_MAC_TRANSMIT_ERR_E;

    cntrNameArrRxDisc[0] = CPSS_DROP_EVENTS_E;
    cntrNameArrTxDisc[0] = CPSS_MAC_TRANSMIT_ERR_E;

    cntrNameArrRxPaus[0] = CPSS_GOOD_FC_RCV_E;
    cntrNameArrTxPaus[0] = CPSS_FC_SENT_E;

    for (i = 0; i < cw_hr_para_ptr->numCount; i++)
    {
        devNum = cw_hr_para_ptr->devNumArr[i];
        portNum = cw_hr_para_ptr->portNumArr[i];

        memset(&hr_tpps[devNum][portNum], 0, sizeof(hr_tpps[devNum][portNum]));
        memset(&hr_rpps[devNum][portNum], 0, sizeof(hr_rpps[devNum][portNum]));
        memset(&hr_tpok[devNum][portNum], 0, sizeof(hr_tpok[devNum][portNum]));
        memset(&hr_tpng[devNum][portNum], 0, sizeof(hr_tpng[devNum][portNum]));
        memset(&hr_rpok[devNum][portNum], 0, sizeof(hr_rpok[devNum][portNum]));
        memset(&hr_rpng[devNum][portNum], 0, sizeof(hr_rpng[devNum][portNum]));
        memset(&hr_txbyte[devNum][portNum], 0, sizeof(hr_txbyte[devNum][portNum]));
        memset(&hr_rxbyte[devNum][portNum], 0, sizeof(hr_rxbyte[devNum][portNum]));
        memset(&hr_txerr[devNum][portNum], 0, sizeof(hr_txerr[devNum][portNum]));
        memset(&hr_rxerr[devNum][portNum], 0, sizeof(hr_rxerr[devNum][portNum]));
        memset(&hr_txdisc[devNum][portNum], 0, sizeof(hr_txdisc[devNum][portNum]));
        memset(&hr_rxdisc[devNum][portNum], 0, sizeof(hr_rxdisc[devNum][portNum]));
        memset(&hr_txpaus[devNum][portNum], 0, sizeof(hr_txpaus[devNum][portNum]));
        memset(&hr_rxpaus[devNum][portNum], 0, sizeof(hr_rxpaus[devNum][portNum]));

        memset(&hr_tpok_bc[devNum][portNum], 0, sizeof(hr_tpok_bc[devNum][portNum]));
        memset(&hr_tpok_mc[devNum][portNum], 0, sizeof(hr_tpok_mc[devNum][portNum]));
        memset(&hr_tpok_uc[devNum][portNum], 0, sizeof(hr_tpok_uc[devNum][portNum]));
        memset(&hr_rpok_bc[devNum][portNum], 0, sizeof(hr_rpok_bc[devNum][portNum]));
        memset(&hr_rpok_mc[devNum][portNum], 0, sizeof(hr_rpok_mc[devNum][portNum]));
        memset(&hr_rpok_uc[devNum][portNum], 0, sizeof(hr_rpok_uc[devNum][portNum]));
    }

    /* start routine, wait for halt signal */
    devNum = cw_hr_para_ptr->devNumArr[0];
    hr_halt[devNum] = 0;
    while (!hr_halt[devNum])
    {
        if(1 == clearIntrCounter[devNum])
        {
            clearIntrCounterPause[devNum] = 1;

            wait_clear_cnt = 0;
            while(clearIntrCounter[devNum])
            {
                wait_clear_cnt++;
                nanosleep(&wait_pause, NULL);

                if(100 < wait_clear_cnt)   /* over 10 sec, timeout */
                {
                    clearIntrCounter[devNum] = 0;
                    break;
                }
            }

            clearIntrCounterPause[devNum] = 0;
        }

        /* get latest TX & RX packet number */
        for (i = 0; i < cw_hr_para_ptr->numCount; i++)
        {
            devNum = cw_hr_para_ptr->devNumArr[i];
            portNum = cw_hr_para_ptr->portNumArr[i];

            /* Rx pkts ok */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxGood; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxGood[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* The API only gets 32bit value, we update upper 32bit manually */
                switch(cntrNameArrRxGood[cntrNameIdx])
                {
                    case CPSS_BRDC_PKTS_RCV_E:
                        update64bitUpperValueWithPrev(&hr_rpok_bc[devNum][portNum], &tmp);
                        break;
                    case CPSS_MC_PKTS_RCV_E:
                        update64bitUpperValueWithPrev(&hr_rpok_mc[devNum][portNum], &tmp);
                        break;
                    case CPSS_GOOD_UC_PKTS_RCV_E:
                        update64bitUpperValueWithPrev(&hr_rpok_uc[devNum][portNum], &tmp);
                        break;
                    default:
                        cpssOsPrintf("Unrecognized cntr name!\n");
                        break;
                }
                hr_rpok_curr[devNum][portNum] = prvCpssMathAdd64(hr_rpok_curr[devNum][portNum], tmp);
            }
            cpssOsTimeRT(&endSec_rpok[devNum][portNum], &endNano_rpok[devNum][portNum]);

            /* Tx pkts ok */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxGood; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxGood[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* The API only gets 32bit value, we update upper 32bit manually */
                switch(cntrNameArrTxGood[cntrNameIdx])
                {
                    case CPSS_BRDC_PKTS_SENT_E:
                        update64bitUpperValueWithPrev(&hr_tpok_bc[devNum][portNum], &tmp);
                        break;
                    case CPSS_MC_PKTS_SENT_E:
                        update64bitUpperValueWithPrev(&hr_tpok_mc[devNum][portNum], &tmp);
                        break;
                    case CPSS_GOOD_UC_PKTS_SENT_E:
                        update64bitUpperValueWithPrev(&hr_tpok_uc[devNum][portNum], &tmp);
                        break;
                    default:
                        cpssOsPrintf("Unrecognized cntr name!\n");
                        break;
                }
                hr_tpok_curr[devNum][portNum] = prvCpssMathAdd64(hr_tpok_curr[devNum][portNum], tmp);
            }
            cpssOsTimeRT(&endSec_tpok[devNum][portNum], &endNano_tpok[devNum][portNum]);
    
            /* Rx pkts NG */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxNg; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxNg[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_rpng_curr[devNum][portNum] = prvCpssMathAdd64(hr_rpng_curr[devNum][portNum], tmp);
            }
    
            /* Tx pkts NG */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxNg; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxNg[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_tpng_curr[devNum][portNum] = prvCpssMathAdd64(hr_tpng_curr[devNum][portNum], tmp);
            }

            /* Rx bytes */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxByte; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxByte[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_rxbyte_curr[devNum][portNum] = prvCpssMathAdd64(hr_rxbyte_curr[devNum][portNum], tmp);
            }

            /* Tx bytes */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxByte; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxByte[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_txbyte_curr[devNum][portNum] = prvCpssMathAdd64(hr_txbyte_curr[devNum][portNum], tmp);
            }

            /* Rx errors */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxErr; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxErr[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_rxerr_curr[devNum][portNum] = prvCpssMathAdd64(hr_rxerr_curr[devNum][portNum], tmp);
            }

            /* Tx errors */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxErr; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxErr[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_txerr_curr[devNum][portNum]= prvCpssMathAdd64(hr_txerr_curr[devNum][portNum], tmp);
            }

            /* Rx discards */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxDisc; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxDisc[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_rxdisc_curr[devNum][portNum] = prvCpssMathAdd64(hr_rxdisc_curr[devNum][portNum], tmp);
            }

            /* Tx discards */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxDisc; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxDisc[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_txdisc_curr[devNum][portNum] = prvCpssMathAdd64(hr_txdisc_curr[devNum][portNum], tmp);
            }

            /* Rx pause */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsRxPaus; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrRxPaus[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_rxpaus_curr[devNum][portNum] = prvCpssMathAdd64(hr_rxpaus_curr[devNum][portNum], tmp);
            }

            /* Tx pause */
            for (cntrNameIdx = 0; cntrNameIdx < numOfCntrsTxPaus; cntrNameIdx++)
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrNameArrTxPaus[cntrNameIdx], &tmp);
                if(rc != GT_OK)
                {
                    return rc;
                }
                hr_txpaus_curr[devNum][portNum] = prvCpssMathAdd64(hr_txpaus_curr[devNum][portNum], tmp);
            }

            /* curr should be always >= prev, if less than prev, it should be counters have been cleared.
             * We set prev to zero to prevent later the result of sub becomes negative */
            if(prvCpssMathCompare64(hr_rpok_curr[devNum][portNum], hr_rpok_prev[devNum][portNum]) == -1)
            {
                memset(&hr_rpok_prev[devNum][portNum], 0, sizeof(hr_rpok_prev[devNum][portNum]));
            }
            if(prvCpssMathCompare64(hr_tpok_curr[devNum][portNum], hr_tpok_prev[devNum][portNum]) == -1)
            {
                memset(&hr_tpok_prev[devNum][portNum], 0, sizeof(hr_tpok_prev[devNum][portNum]));
            }

            /* calculate delta of TX & RX packet number */
            rxPktDelta[devNum][portNum] = prvCpssMathSub64(hr_rpok_curr[devNum][portNum], hr_rpok_prev[devNum][portNum]);
            txPktDelta[devNum][portNum] = prvCpssMathSub64(hr_tpok_curr[devNum][portNum], hr_tpok_prev[devNum][portNum]);

            /* update previous TX & RX packet number */
            hr_rpok_prev[devNum][portNum] = hr_rpok_curr[devNum][portNum];
            hr_tpok_prev[devNum][portNum] = hr_tpok_curr[devNum][portNum];
            hr_rpng_prev[devNum][portNum] = hr_rpng_curr[devNum][portNum];
            hr_tpng_prev[devNum][portNum] = hr_tpng_curr[devNum][portNum];

            /* update detailed counters */
            hr_rxbyte[devNum][portNum] = hr_rxbyte_curr[devNum][portNum];
            hr_txbyte[devNum][portNum] = hr_txbyte_curr[devNum][portNum];
            hr_rxerr[devNum][portNum] =  hr_rxerr_curr[devNum][portNum];
            hr_txerr[devNum][portNum] =  hr_txerr_curr[devNum][portNum];
            hr_rxdisc[devNum][portNum] = hr_rxdisc_curr[devNum][portNum];
            hr_txdisc[devNum][portNum] = hr_txdisc_curr[devNum][portNum];
            hr_rxpaus[devNum][portNum] = hr_rxpaus_curr[devNum][portNum];
            hr_txpaus[devNum][portNum] = hr_txpaus_curr[devNum][portNum];

            /* clear counters */
            memset(&hr_rpok_curr[devNum][portNum], 0, sizeof(hr_rpok_curr[devNum][portNum]));
            memset(&hr_tpok_curr[devNum][portNum], 0, sizeof(hr_tpok_curr[devNum][portNum]));
            memset(&hr_rpng_curr[devNum][portNum], 0, sizeof(hr_rpng_curr[devNum][portNum]));
            memset(&hr_tpng_curr[devNum][portNum], 0, sizeof(hr_tpng_curr[devNum][portNum]));
            memset(&hr_rxbyte_curr[devNum][portNum], 0, sizeof(hr_rxbyte_curr[devNum][portNum]));
            memset(&hr_txbyte_curr[devNum][portNum], 0, sizeof(hr_txbyte_curr[devNum][portNum]));
            memset(&hr_rxerr_curr[devNum][portNum], 0, sizeof(hr_rxerr_curr[devNum][portNum]));
            memset(&hr_txerr_curr[devNum][portNum], 0, sizeof(hr_txerr_curr[devNum][portNum]));
            memset(&hr_rxdisc_curr[devNum][portNum], 0, sizeof(hr_rxdisc_curr[devNum][portNum]));
            memset(&hr_txdisc_curr[devNum][portNum], 0, sizeof(hr_txdisc_curr[devNum][portNum]));
            memset(&hr_rxpaus_curr[devNum][portNum], 0, sizeof(hr_rxpaus_curr[devNum][portNum]));
            memset(&hr_txpaus_curr[devNum][portNum], 0, sizeof(hr_txpaus_curr[devNum][portNum]));
        }

        if(skipFirstRun)
        {
            skipFirstRun = 0;
            goto next_round;
        }

        for (i = 0; i < cw_hr_para_ptr->numCount; i++)
        {
            devNum = cw_hr_para_ptr->devNumArr[i];
            portNum = cw_hr_para_ptr->portNumArr[i];

            /* Rx rate calculation */
            tsDelta = time_diff(startSec_rpok[devNum][portNum], startNano_rpok[devNum][portNum],
                                endSec_rpok[devNum][portNum], endNano_rpok[devNum][portNum]);
            hr_rpps[devNum][portNum] = (((uint64_t)rxPktDelta[devNum][portNum].l[1] << 32) + rxPktDelta[devNum][portNum].l[0]) / tsDelta;

            /* Tx rate calculation */
            tsDelta = time_diff(startSec_tpok[devNum][portNum], startNano_tpok[devNum][portNum],
                                endSec_tpok[devNum][portNum], endNano_tpok[devNum][portNum]);
            hr_tpps[devNum][portNum] = (((uint64_t)txPktDelta[devNum][portNum].l[1] << 32) + txPktDelta[devNum][portNum].l[0]) / tsDelta;

            hr_rpok[devNum][portNum] = hr_rpok_prev[devNum][portNum];
            hr_tpok[devNum][portNum] = hr_tpok_prev[devNum][portNum];
            hr_rpng[devNum][portNum] = hr_rpng_prev[devNum][portNum];
            hr_tpng[devNum][portNum] = hr_tpng_prev[devNum][portNum];
        }

next_round:
        for (i = 0; i < cw_hr_para_ptr->numCount; i++)
        {
            devNum = cw_hr_para_ptr->devNumArr[i];
            portNum = cw_hr_para_ptr->portNumArr[i];

            /* Update previous time stamp */
            startSec_rpok[devNum][portNum]  = endSec_rpok[devNum][portNum];
            startNano_rpok[devNum][portNum] = endNano_rpok[devNum][portNum];
            startSec_tpok[devNum][portNum]  = endSec_tpok[devNum][portNum];
            startNano_tpok[devNum][portNum] = endNano_tpok[devNum][portNum];
        }

        nanosleep(&ts_pause, NULL); /* nanosleep() is high-resolutional */
    }

    if(munlockall() != 0)
    {
        cpssOsPrintf("fail to unlock virtual memory!\n");
    }

    delCwHighResTask[devNum] = 1;

    return rc;
}

/**
* @ wncUtilsTrtestCounterWatcherHighResStart
*
* @brief   Start the counter watcher and enable high resolution mode to monitor the traffic rate
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestCounterWatcherHighResStart
(
    IN GT_U8    unit
)
{
    GT_U32      i;
    GT_U32      j;      /* Port list index to thread, according to input unit */
    GT_STATUS   rc = GT_OK;	
    GT_CHAR     taskName[32];

    if(tidCwHighResTask[unit] != 0)
    {
        cpssOsPrintf("Couter watcher high-res thread %d exists!\n", unit);
        return GT_ALREADY_EXIST;
    }

    cpssOsSnprintf(taskName, sizeof(taskName), "cwHighResTask_%d", unit);

    /* map portmapping array to devNumArr and portNumArr */
    memset(&cw_hr_para[unit], 0, sizeof(WNC_CW_INR_ETH_PORTS_STC));
    j = 0;
    for(i = 0; i < portmapCount; i++)
    {
        if(portMapInfo[i].devNum== unit)
        {
            cw_hr_para[unit].devNumArr[j] = portMapInfo[i].devNum;
            cw_hr_para[unit].portNumArr[j] = portMapInfo[i].portNum;
            j++;
        }
    }
    if(0 == j)
    {
        cpssOsPrintf("Unit [%d] is not supported!\n", unit);
        return GT_BAD_PARAM;
    }
    else
    {
        cw_hr_para[unit].numCount = j;
    }

    rc = cpssOsTaskCreate ( taskName,                       /* Task Name      */
                            0,                              /* Task Priority  */
                            0x20000,                        /* Stack Size     */
                            taskCounterWatcherHighResStart, /* Starting Point */
                            (GT_VOID*)(&cw_hr_para[unit]),  /* Arguments list */
                            &tidCwHighResTask[unit]);       /* task ID        */
    if(rc != GT_OK)
    {
        cpssOsPrintf("cwHighResTask_%d create failed!\n", unit);
    }

    return rc;
}

/**
* @ wncUtilsTrtestCounterWatcherHighResStop
*
* @brief   Stop the counter watcher with high resolution mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestCounterWatcherHighResStop
(
    IN GT_U8    unit
)
{
    GT_STATUS   rc = GT_OK;	
    GT_32       timeout = 5; /* 5 second */
    
    if(tidCwHighResTask[unit] == 0)
    {
        cpssOsPrintf("Couter watcher high-res thread %d not exist!\n", unit);
        return GT_NO_SUCH;
    }

    hr_halt[unit] = 1;    /* signal thread to terminate itself */

    while(delCwHighResTask[unit] != 1)
    {
        if(timeout <= 0)
        {
            cpssOsPrintf("Wait couter watcher high-res %d terminated timeout!\n", unit);
            tidCwHighResTask[unit] = 0;
            return GT_TIMEOUT;
        }
        timeout -= 1;
        sleep(1);  /* 1 sec */
    }

    cpssOsTaskDelete(tidCwHighResTask[unit]);
    tidCwHighResTask[unit] = 0;
    delCwHighResTask[unit] = 0;

    return rc;
}

/**
* @ wncUtilsTrtestCounterWatcherHighResShowAll
*
* @brief   Dump port counter informations with high-resolution mode from counter array to file
*
* @param[in] pid           - use to distinguish different files
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestCounterWatcherHighResShowAll
(
    IN GT_U32   pid
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U8       devNum;
    GT_U32      portNum;
    GT_CHAR     rxCounter[23];
    GT_CHAR     txCounter[23];
    GT_CHAR     fpath[32];
    FILE        *fp = NULL;

#if 0
    /* Need to check the thread exists? */
    if(tidCwHighResTask[unit] == 0)
    {
        cpssOsPrintf("Couter watcher high-res thread %d not exist!\n", unit);
        return GT_NO_SUCH;
    }
#endif

    cpssOsSnprintf(fpath, sizeof(fpath), "/tmp/hr_counters_%d", pid);
    if ((fp = fopen(fpath, "w+")) != NULL)
    {
        /* Dump counter result to file first */
        fprintf(fp, "Port  Rx PPS    Tx PPS    Rx Pkts (Good/NG)      Tx Pkts (Good/NG)\n");
        fprintf(fp, "----- --------- --------- ---------------------- ----------------------\n");

        /* Show by lport order */
        for(i = 0; i < portmapCount; i++)
        {
            devNum  = portMapInfo[i].devNum;
            portNum = portMapInfo[i].portNum;

            /* GT_U64: 32-bit CPU use %llu, 64-bit use %lu */
            cpssOsSnprintf(rxCounter, sizeof(rxCounter), "%llu/%llu",
                           ((uint64_t)hr_rpok[devNum][portNum].l[1] << 32) + hr_rpok[devNum][portNum].l[0],
                           ((uint64_t)hr_rpng[devNum][portNum].l[1] << 32) + hr_rpng[devNum][portNum].l[0]);
            cpssOsSnprintf(txCounter, sizeof(txCounter), "%llu/%llu",
                           ((uint64_t)hr_tpok[devNum][portNum].l[1] << 32) + hr_tpok[devNum][portNum].l[0],
                           ((uint64_t)hr_tpng[devNum][portNum].l[1] << 32) + hr_tpng[devNum][portNum].l[0]);

            fprintf(fp, "%-5s|%9lu|%9lu|%22s|%22s\n",
                         portMapInfo[i].lport,
                         hr_rpps[devNum][portNum],
                         hr_tpps[devNum][portNum],
                         rxCounter, txCounter);
        }
        fclose(fp);

#if 0
        GT_CHAR cmd[128];

        /* Output to /tmp/appDemo_stdout */
        cpssOsSnprintf(cmd, sizeof(cmd), "cat %s; rm -f %s", fpath, fpath);
        system(cmd);
#endif
    }
    else
    {
        cpssOsPrintf("fopen() %s failed!\n", fpath);
        rc = GT_FAIL;
    }

    return rc;
}

/**
* @ wncUtilsTrtestCheckCpssAlive
*
* @brief   Check CPSS is alive or not
*
* @param[in] show           - show the CPSS version
*
* @retval GT_OK             - on success
*/
GT_STATUS wncUtilsTrtestCheckCpssAlive
(
    GT_U8   show
)
{
    /*
       Refer to appDemoShowCpssSwVersion(), to get CPSS version via IPC to make sure the CPSS alive.
       Client will get return 0 if alive, otherwise get return 1 due to connect() failed or read() failed.
    */
    GT_STATUS   rc = GT_OK;

    /* Not always dump to /tmp/appDemo_stdout, to prevent messy log */
    if(show)
    {
        cpssOsPrintf("\nCPSS Version Stream: %s\n", CPSS_STREAM_NAME_CNS);
    }

    return rc;
}

/**
* @internal autoNegotiationSet function
* @endinternal
*
* @brief   Disable auto negotiation on port and set port with input speed
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*                                       (For DxCh3, not XG ports only.)
*/
static GT_STATUS autoNegotiationSet
(
	IN	GT_U8 devNum,
	IN	GT_PHYSICAL_PORT_NUM  portNum,
	IN	CPSS_PORT_SPEED_ENT speed
)
{
	GT_STATUS   rc = GT_OK;

	rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
	if (rc != GT_OK) return rc;
	rc = cpssDxChPortSpeedSet(devNum, portNum, speed);
	if (rc != GT_OK) return rc;
	rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
	if (rc != GT_OK) return rc;

	return rc;
}

/**
* @internal setPhyExtLoopbackStub function
* @endinternal
*
* @brief   Set external phy loopback on input devie/port according to phy type 
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] phyType                  - phy type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS setPhyExtLoopbackStub
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 phyType
)
{
    GT_STATUS               rc = GT_OK;

	switch(phyType)
	{
		case APPDEMO_WNC_UTILS_TRTEST_PHY_88E1680:
			rc = cpssDxChPhyAutonegSmiSet(devNum, portNum, GT_FALSE);
			CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyAutonegSmiSet", rc);
			if (rc != GT_OK) return rc;

	        /* Set loopback stub */
	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x6);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 18, 0x8);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x0);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

			rc = cpssDxChPhyAutonegSmiSet(devNum, portNum, GT_TRUE);
			CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyAutonegSmiSet", rc);
			if (rc != GT_OK) return rc;

			/* fix port speed */
			rc = autoNegotiationSet(devNum, portNum, CPSS_PORT_SPEED_1000_E);
			CPSS_ENABLER_DBG_TRACE_RC_MAC("autoNegotiationSet", rc);
			if (rc != GT_OK) return rc;
			break;
		case APPDEMO_WNC_UTILS_TRTEST_PHY_88E1543:
			/* Set loopback stub */
	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x6);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 18, 0x8);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

	        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x0);
	        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
	        if(rc != GT_OK)  return rc;

			/* fix port speed */
			rc = autoNegotiationSet(devNum, portNum, CPSS_PORT_SPEED_1000_E);
			CPSS_ENABLER_DBG_TRACE_RC_MAC("autoNegotiationSet", rc);
			if (rc != GT_OK) return rc;
			break;
		default:
			break;
	}
    
    return rc;
}


/**
* @ wncUtilsTrtestMfgModeSetLoopback
*
* @brief   Set loopback mode on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS wncUtilsTrtestMfgModeSetLoopback
(
    GT_VOID
)
{
	GT_U32      i;
	GT_STATUS	rc = GT_OK;

	for(i = 0; i < portmapCount; i++)
	{
		switch(mfgPortInfo[i].loopBackMode)
		{
			case APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_PHYEXT:
				rc = setPhyExtLoopbackStub(mfgPortInfo[i].devNum, mfgPortInfo[i].portNum, mfgPortInfo[i].phyType);
				if(rc != GT_OK)  return rc;
				break;
			default:
				break;
		}
	}

    return rc;
}

/**
* @internal addVlanMember function
* @endinternal
*
* @brief   Add VLAN Members on input VLAN id.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] tagged                   - VLAN tagged format
* @param[in] portsAdditionalMembers   - additional member
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
*/
static GT_STATUS addVlanMember
(
    IN  GT_U8       devNum,
    IN  GT_U32      vlanId,
    IN  GT_U32      tagged,
    IN  CPSS_PORTS_BMP_STC	portsAdditionalMembers
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     isValid;    /* is Valid flag */
    GT_U32      i, port;
	GT_U32      portsAdditionalMembersPorts;	/* Local VLAN additional members for parsing */
    CPSS_PORTS_BMP_STC  portsMembers;           /* VLAN members */
    CPSS_PORTS_BMP_STC  portsTagging;           /* VLAN tagging */
    CPSS_DXCH_BRG_VLAN_INFO_STC             cpssVlanInfo;       /* cpss vlan info format */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;    /* ports tagging command */

    if(vlanId >= 4096)
    {
        cpssOsPrintf("addVlanMember() with vlanId %d exceeding the max value!\n", vlanId);
        return GT_FAIL;
    }

    /* Read VLAN entry */
    rc = cpssDxChBrgVlanEntryRead(devNum, vlanId, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryRead", rc);
    if (rc != GT_OK) return rc;

    if(isValid == GT_FALSE)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
        osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

        /* default IP MC VIDX */
        cpssVlanInfo.unregIpmEVidx      = 0xFFF;
        cpssVlanInfo.naMsgToCpuEn       = GT_TRUE;
        cpssVlanInfo.floodVidx          = 0xFFF;
        cpssVlanInfo.fidValue           = vlanId;

        cpssVlanInfo.autoLearnDisable   = GT_TRUE;
        cpssVlanInfo.naMsgToCpuEn       = GT_TRUE;
        cpssVlanInfo.portIsolationMode  = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
    }

    /* Add port to the VLAN if got plist */
	portsAdditionalMembersPorts=portsAdditionalMembers.ports[0];
	if(portsAdditionalMembersPorts!=0)
	{
		port=0;
    	while(portsAdditionalMembersPorts)
		{
			if(portsAdditionalMembersPorts&0x1)
			{
				cpssOsPrintf("addVlanMember port=%d\n",port);
				if(tagged)
					portsTaggingCmd.portsCmd[port] = tagged;
			}
			portsAdditionalMembersPorts >>= 1;
			port++;
		}

        CPSS_PORTS_BMP_BITWISE_OR_MAC(&portsMembers, &portsMembers, &portsAdditionalMembers);
	}

    /* Write VLAN entry */
    rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryWrite", rc);

    return rc;
}

/**
* @ wncUtilsTrtestMfgModeSetVlan function
*
* @brief   Config MFG mode VLAN setting
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestMfgModeSetVlan
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
	GT_U8       devNum;
	GT_U32      i;
	CPSS_PORTS_BMP_STC	portsAdditionalMembers; /* VLAN members to add */

	/* Set all ports vlan to default vlan 100 first */
	for(devNum = 0; devNum < devMacNum; devNum++)
	{
		CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);
	    rc = addVlanMember(devNum, DEFAULT_VLAN, 0, portsAdditionalMembers);
	    CPSS_ENABLER_DBG_TRACE_RC_MAC("addVlanMember", rc);
		if(rc != GT_OK)  return rc;
	}

	/* Set all ports pvid to default vlan 100 first */
	for(i = 0; i < portmapCount; i++)
	{		
		/* Set port pvid 100 */
		rc = cpssDxChBrgVlanPortVidSet(mfgPortInfo[i].devNum, mfgPortInfo[i].portNum, CPSS_DIRECTION_INGRESS_E, DEFAULT_VLAN);
		CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
		if (rc != GT_OK) return rc;
		/*cpssOsPrintf("cpssDxChBrgVlanPortVidSet pvid=%d\n",DEFAULT_VLAN);*/
	}

	/* Set VLAN for TrTest in MFG mode */
	for(i = 0; i < portmapCount; i++)
	{
		CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, mfgPortInfo[i].portNum);

		rc = addVlanMember(mfgPortInfo[i].devNum, mfgPortInfo[i].vlan ,mfgPortInfo[i].vlanMemberType, portsAdditionalMembers);
	    CPSS_ENABLER_DBG_TRACE_RC_MAC("addVlanMember", rc);
		if(rc != GT_OK)  return rc;

		/* Set port pvid */
        rc = cpssDxChBrgVlanPortVidSet(mfgPortInfo[i].devNum, mfgPortInfo[i].portNum, CPSS_DIRECTION_INGRESS_E, mfgPortInfo[i].vlan);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
        if (rc != GT_OK) return rc;
		/*cpssOsPrintf("cpssDxChBrgVlanPortVidSet pvid=%d\n",mfgPortInfo[i].vlan);*/
	}

    return rc;
}

/**
* @ wncUtilsTrtestMfgModeInit function
* @endinternal
*
* @brief   MFG mode initialization
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestMfgModeInit
(
	GT_VOID
)
{
	GT_STATUS   rc = GT_OK;

	rc = getMfgModePortInfo();
	if(rc != GT_OK)  return rc;

	return rc;
}

/**
* @ wncUtilsTrtestInit function
* @endinternal
*
* @brief   traffic test initialization
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTrtestInit
(
	GT_VOID
)
{
	GT_STATUS   rc = GT_OK;

	rc = getPortMappingTable();
	if(rc != GT_OK)  return rc;

	return rc;
}
