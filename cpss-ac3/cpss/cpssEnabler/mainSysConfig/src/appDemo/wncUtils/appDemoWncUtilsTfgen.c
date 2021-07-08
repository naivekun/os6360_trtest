#include <appDemo/wncUtils/appDemoWncUtilsTrtest.h>
#include <appDemo/wncUtils/appDemoWncUtilsTfgen.h>

/* The task Tid */
static GT_TASK tidGetLinkStatusTask=0;

static GT_U32 getLink_halt = 1;
static GT_U32 delGetLinkStatusTask = 0;
static GT_U32 sendFix_halt = 1;
static GT_U32 clearIntrCounter[MAX_UNIT_CNS] = {0};
static GT_U32 clearIntrCounterPause[MAX_UNIT_CNS] = {0};

extern unsigned int sleep(unsigned int);
extern int usleep(unsigned int);
extern int nanosleep(const struct timespec *, struct timespec *);

/* Add to get WNC SKU ID */
#define AC3BXH_WNC_OS6360P48	7
extern GT_STATUS xcat3BoardTypeGet(OUT GT_U32 *xcat3BoardTypePtr);

/**
* @ wncUtilsTfgenSetClearIntrPktCounterFlag
*
* @brief   Set flag to counter thread to pause per unit.
*
* @param[in] value      - input value
*
* @retval GT_OK         - on success.
*/
GT_STATUS wncUtilsTfgenSetClearIntrPktCounterFlag
(
    IN GT_U8    value
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      wait_pause_cnt = 0;
    struct timespec wait_pause = {0, 100000000}; /* 100ms */

    /* Set flag to counter thread to pause per unit.
     * clearIntrCounter[] will be set here, and be unset only in:
     *  (1) Call wncUtilsTfgenSetClearIntrPktCounterFlag() with value not 1;
     *  (2) Call wncUtilsTfgenClearInternalPktCounter() to unset.
     */

    for(i = 0; i < MAX_UNIT_CNS; i++)
    {
        clearIntrCounter[i] = value;
    }

    if(1 == value)
    {
        for(i = 0; i < MAX_UNIT_CNS; i++)
        {
            /* Wait for response, to make sure counter thread is paused per unit.
             * And then we can make sure during we call CLI command "do clear mac counters" and
             * wncUtilsTfgenClearInternalPktCounter(), the counter thread won't update temporarily.
             */
            while(1 != clearIntrCounterPause[i])
            {
                wait_pause_cnt++;
                nanosleep(&wait_pause, NULL);

                if(100 < wait_pause_cnt)   /* over 10 sec, timeout */
                {
                    clearIntrCounter[i] = 0;
                    break;
                }
            }
        }
    }

    return rc;
}

/**
* @ wncUtilsTfgenClearInternalPktCounter
*
* @brief   Clear internal packet counter for traffic generator
*
* @param[in] plist                 - input port list
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS wncUtilsTfgenClearInternalPktCounter
(
    IN GT_CHAR  plist[128]
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U8       devNum;
    GT_U32      portNum;
    GT_CHAR     tmp_str[32] = {0};
    GT_CHAR_PTR sptr;
    GT_CHAR_PTR sptr_s;

    if(cpssOsStrCmp(plist, "") == 0)
    {
        cpssOsPrintf("Got NULL port list, please check!\n");
        rc = GT_FAIL;
    }
    else if(cpssOsStrNCmp(plist, "all", 3) == 0)
    {
        for(i = 0; i < portmapCount; i++)
        {
            /* devNum and portNum */
            devNum = portMapInfo[i].devNum;
            portNum = portMapInfo[i].portNum;
            /* memset to clear */
            cpssOsMemSet(&hr_tpok_bc[devNum][portNum], 0, sizeof(hr_tpok_bc[devNum][portNum]));
            cpssOsMemSet(&hr_tpok_mc[devNum][portNum], 0, sizeof(hr_tpok_mc[devNum][portNum]));
            cpssOsMemSet(&hr_tpok_uc[devNum][portNum], 0, sizeof(hr_tpok_uc[devNum][portNum]));
            cpssOsMemSet(&hr_rpok_bc[devNum][portNum], 0, sizeof(hr_rpok_bc[devNum][portNum]));
            cpssOsMemSet(&hr_rpok_mc[devNum][portNum], 0, sizeof(hr_rpok_mc[devNum][portNum]));
            cpssOsMemSet(&hr_rpok_uc[devNum][portNum], 0, sizeof(hr_rpok_uc[devNum][portNum]));
        }
    }
    else
    {
        /* ex. 0/0,0/1 */
        sptr = strtok(plist, ",");
        if(sptr == NULL)
        {
            /* May be one port only, ex. 0/0 */
            sptr = plist;
        }

        while(sptr != NULL)
        {
            sptr_s = cpssOsStrChr(sptr, '/');
            if(sptr_s != NULL)
            {
                /* devNum */
                cpssOsMemSet(tmp_str, 0, sizeof(tmp_str));
                cpssOsStrNCpy(tmp_str, sptr, (GT_U32)(sptr_s - sptr));
                tmp_str[sptr_s - sptr] = 0;
                devNum = (GT_U8)atoi(tmp_str);
                /* portNum */
                cpssOsMemSet(tmp_str, 0, sizeof(tmp_str));
                cpssOsStrNCpy(tmp_str, (sptr_s + 1), (GT_U32)(strlen(sptr) - (sptr_s - sptr + 1)));
                tmp_str[strlen(sptr) - (sptr_s - sptr + 1)] = 0;
                portNum = (GT_U32)atoi(tmp_str);
                /* memset to clear */
                cpssOsMemSet(&hr_tpok_bc[devNum][portNum], 0, sizeof(hr_tpok_bc[devNum][portNum]));
                cpssOsMemSet(&hr_tpok_mc[devNum][portNum], 0, sizeof(hr_tpok_mc[devNum][portNum]));
                cpssOsMemSet(&hr_tpok_uc[devNum][portNum], 0, sizeof(hr_tpok_uc[devNum][portNum]));
                cpssOsMemSet(&hr_rpok_bc[devNum][portNum], 0, sizeof(hr_rpok_bc[devNum][portNum]));
                cpssOsMemSet(&hr_rpok_mc[devNum][portNum], 0, sizeof(hr_rpok_mc[devNum][portNum]));
                cpssOsMemSet(&hr_rpok_uc[devNum][portNum], 0, sizeof(hr_rpok_uc[devNum][portNum]));
            }
            else
            {
                cpssOsPrintf("Port list format got wrong, please check!\n");
            }

            sptr = strtok(NULL, ",");
        }
    }

    for(i = 0; i < MAX_UNIT_CNS; i++)
    {
        if(clearIntrCounter[i])
        {
            clearIntrCounter[i] = 0;
        }
    }

    return rc;
}

/**
* @ wncUtilsTfgenHighResCounterForTfGen
*
* @brief   Dump port counter informations with high-resolution mode from counter array to file for traffic generator
*
* @param[in] pid                 - use to distinguish different files
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
GT_STATUS wncUtilsTfgenHighResCounterForTfGen
(
    IN GT_U32  pid
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U8       devNum;
    GT_U32      portNum;
    GT_CHAR     fpath[32];
    FILE        *fp = NULL;

    cpssOsSnprintf(fpath, sizeof(fpath), "/tmp/tfgen_hr_counters_%d", pid);
    if ((fp = fopen(fpath, "w+")) != NULL)
    {
        /* Show by lport order, each line a lport */
        for(i = 0; i < portmapCount; i++)
        {
            devNum  = portMapInfo[i].devNum;
            portNum = portMapInfo[i].portNum;

            if(cpssOsStrNCmp(portMapInfo[i].lport, "i", 1) == 0)
            {
                /* Without information of internal ports */
                continue;
            }
            fprintf(fp, "%lu %lu %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64
                               " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
                         hr_rpps[devNum][portNum],
                         hr_tpps[devNum][portNum],
                         hr_rpok[devNum][portNum],
                         hr_tpok[devNum][portNum],
                         hr_rxbyte[devNum][portNum],
                         hr_txbyte[devNum][portNum],
                         hr_rxerr[devNum][portNum],
                         hr_txerr[devNum][portNum],
                         hr_rxdisc[devNum][portNum],
                         hr_txdisc[devNum][portNum],
                         hr_rxpaus[devNum][portNum],
                         hr_txpaus[devNum][portNum]);
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

static unsigned __TASKCONV taskGetLinkStatusForTfGen
(
    GT_VOID *param
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      interval;
    GT_BOOL     isLinkUp[MAX_PORTS_CNS];
    GT_CHAR     mesg[16] = {0};
    int         fd;
    GT_U32      portSpeed[MAX_PORTS_CNS];
    CPSS_PORT_SPEED_ENT speed;

    /* Get update interval */
    interval = *(GT_U32 *)param;

    getLink_halt = 0;
    while(!getLink_halt)
    {
        for(i = 0; i < portmapCount; i++)
        {
            if(cpssOsStrNCmp(portMapInfo[i].lport, "i", 1) == 0)
            {
                /* Skip internal ports */
                continue;
            }
            /* Get Link */
            rc = cpssDxChPortLinkStatusGet(portMapInfo[i].devNum, portMapInfo[i].portNum, &isLinkUp[i]);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLinkStatusGet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Get Speed */
            rc = cpssDxChPortSpeedGet(portMapInfo[i].devNum, portMapInfo[i].portNum, &speed);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSpeedGet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
            else
            {
                switch(speed)
                {
                    case CPSS_PORT_SPEED_10_E:      /* 0 */
                        portSpeed[i] = 10;
                        break;
                    case CPSS_PORT_SPEED_100_E:     /* 1 */
                        portSpeed[i] = 100;
                        break;
                    case CPSS_PORT_SPEED_1000_E:    /* 2 */
                        portSpeed[i] = 1000;
                        break;
                    case CPSS_PORT_SPEED_10000_E:   /* 3 */
                        portSpeed[i] = 10000;
                        break;
                    case CPSS_PORT_SPEED_2500_E:    /* 5 */
                        portSpeed[i] = 2500;
                        break;
                    case CPSS_PORT_SPEED_5000_E:    /* 6 */
                        portSpeed[i] = 5000;
                        break;
                    case CPSS_PORT_SPEED_20000_E:   /* 8 */
                        portSpeed[i] = 20000;
                        break;
                    case CPSS_PORT_SPEED_40000_E:   /* 9 */
                        portSpeed[i] = 40000;
                        break;
                    default:
                        /* default show speed "0", if not got the info */
                        portSpeed[i] = 0;
                        break;
                }
            }
        }

        if((fd = open(TFGEN_LINK_STAT, O_WRONLY|O_CREAT)) != -1)
        {
            if(flock(fd, LOCK_EX) == 0)
            {
                for(i = 0; i < portmapCount; i++)
                {
                    if(cpssOsStrNCmp(portMapInfo[i].lport, "i", 1) == 0)
                    {
                        /* Skip internal ports */
                        continue;
                    }

                    cpssOsMemSet(mesg, 0, sizeof(mesg));
                    cpssOsSnprintf(mesg, sizeof(mesg), (GT_TRUE == isLinkUp[i]) ? "1 %d\n" : "0 %d\n", portSpeed[i]);
                    if((write(fd, mesg, cpssOsStrlen(mesg))) == -1)
                    {
                        cpssOsPrintf("write failed!\n");
                    }

                }

                if(flock(fd, LOCK_UN) != 0)
                {
                    cpssOsPrintf("unlock %s failed!\n", TFGEN_LINK_STAT);
                }
            }
            close(fd);
        }
        else
        {
            cpssOsPrintf("open() %s failed!\n", TFGEN_LINK_STAT);
            rc = GT_FAIL;
        }

        sleep(interval);
    }

    delGetLinkStatusTask = 1;

    return rc;
}

/**
* @ wncUtilsTfgenHighResCounterForTfGen
*
* @brief   Start the counter watcher and enable high resolution mode to monitor the traffic rate for traffic generator
*
* @param[in] invl                - 
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
GT_STATUS wncUtilsTfgenGetLinkStatusForTfGenStart
(
    IN GT_U32  invl
)
{
    GT_STATUS   rc = GT_OK;
    GT_CHAR     taskName[32];

    if(tidGetLinkStatusTask != 0)
    {
        cpssOsPrintf("Get link status thread exists!\n");
        return GT_ALREADY_EXIST;
    }

    cpssOsPrintf("Start get link status task with interval %d sec!\n", invl);
    cpssOsSnprintf(taskName, sizeof(taskName), "getLinkStat");

    rc = cpssOsTaskCreate ( taskName,                       /* Task Name      */
                            200,                            /* Task Priority  */
                            0x20000,                        /* Stack Size     */
                            taskGetLinkStatusForTfGen,      /* Starting Point */
                            (GT_VOID*)&invl,                /* Arguments list */
                            &tidGetLinkStatusTask);         /* task ID        */
    if(rc != GT_OK)
    {
        cpssOsPrintf("%s create failed!\n", taskName);
    }

    return rc;
}

/**
* @ wncUtilsTfgenGetLinkStatusForTfGenStop
*
* @brief   Stop the counter watcher with high resolution mode for traffic generator
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
GT_STATUS wncUtilsTfgenGetLinkStatusForTfGenStop
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_32       timeout = 5; /* 5 second */

    if(tidGetLinkStatusTask == 0)
    {
        cpssOsPrintf("Get link status thread not exist!\n");
        return GT_NO_SUCH;
    }

    getLink_halt = 1;    /* signal thread to terminate itself */

    while(delGetLinkStatusTask != 1)
    {
        if (timeout <= 0)
        {
            cpssOsPrintf("Wait get link status task terminated timeout!\n");
            tidGetLinkStatusTask = 0;
            return GT_TIMEOUT;
        }
        timeout -= 1;
        sleep(1);  /* 1 ses */
    }

    cpssOsTaskDelete(tidGetLinkStatusTask);
    tidGetLinkStatusTask = 0;
    delGetLinkStatusTask = 0;

    /* To prevent front-end getting file error, we don't remove the generated file! */
    /* Remove generated file
    if(access(TFGEN_LINK_STAT, F_OK) == 0)
    {
        if(remove(TFGEN_LINK_STAT) != 0)
        {
            cpssOsPrintf("remove() %s failed!\n", TFGEN_LINK_STAT);
        }
    }
    */

    return rc;
}

/**
* @internal wncUtilsTfgenSetTgfWsMode
* @endinternal
*
* @brief   Set ws mode
*
* @param[in] devNum            - send_fixed_num
* @param[in] portNum           - send_fixed_num
* @param[in] action            - send_fixed_num
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
GT_STATUS wncUtilsTfgenSetTgfWsMode
(
    IN GT_U8    devNum,
    IN GT_U32   portNum,
    IN GT_U8    action
)
{
    GT_STATUS               rc = GT_OK;

    if(action == 1)
    {
        rc = tgfTrafficGeneratorWsModeOnPortSet(devNum,portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("tgfTrafficGeneratorWsModeOnPortSet() failed!\n");
        }
    }
    else
    {
        rc = tgfTrafficGeneratorStopWsOnPort(devNum,portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("tgfTrafficGeneratorStopWsOnPort() failed!\n");
        }
    }

    return rc;
}

/**
* @internal taskSendFixedNumberPkt
* @endinternal
*
* @brief   Send packet task
*
* @param[in] param            - send_fixed_num
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
static unsigned __TASKCONV taskSendFixedNumberPkt
(
    GT_VOID *param
)
{
    GT_STATUS               rc = GT_OK;
    GT_32                   fd;
    GT_CHAR                 readStr[18001] = {0}; /* 9k bytes + '\0' */
    GT_CHAR                 tmp[2] = {0};
    GT_U8                   data[9000] = {0};
    GT_U32                  i, len;
    GT_U32                  tx_done_cnt = 0, numVfd = 0;
    TGF_VFD_INFO_STC        *vfdArray = NULL;
    GT_BOOL                 withLoopBack = GT_FALSE;
    TGF_PACKET_PAYLOAD_STC  payload;
    TGF_PACKET_PART_STC     packetPart;
    TGF_PACKET_STC          packetInfo;
    CPSS_INTERFACE_INFO_STC portInterface;
    WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC send_fixed_num;

    if(param != NULL)
    {
        cpssOsMemCpy(&send_fixed_num, param, sizeof(WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC));
    }
    else
    {
        cpssOsPrintf("Got param NULL!\n");
        return GT_FAIL;
    }

    cmdOsMemSet(&packetInfo, 0,sizeof(packetInfo));
    cmdOsMemSet(&portInterface, 0, sizeof(portInterface));

    if((fd = open(send_fixed_num.file_path, O_RDONLY)) < 0)
    {
        cpssOsPrintf("open() %s failed!\n", send_fixed_num.file_path);
        return GT_FAIL;
    }
    else
    {
        /* It should be one line data! */
        if((len = read(fd, readStr, sizeof(readStr)-1)) < 0)
        {
            cpssOsPrintf("read() %s failed!\n", send_fixed_num.file_path);
            rc = GT_FAIL;
        }
        else
        {
            readStr[len] = '\0';
        }
        close(fd);
        if(rc != GT_OK) return rc;
    }

    len = strlen(readStr);
    for(i = 0; i < len; i+=2)
    {
        cpssOsMemCpy(tmp, readStr+i, 2);
        data[i/2] = (GT_U8)cpssOsStrTol(tmp, NULL, 16);     /* Hex value stored in one byte */
    }

    payload.dataPtr = data;
    payload.dataLength = len/2;
    packetPart.type = TGF_PACKET_PART_PAYLOAD_E;
    packetPart.partPtr = &payload;

    packetInfo.partsArray = &packetPart ;
    packetInfo.totalLen = len/2;
    packetInfo.numOfParts = 1;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = send_fixed_num.dev_num;
    portInterface.devPort.portNum = send_fixed_num.port_num;

    /* set the CPU device to send traffic */
    rc = tgfTrafficGeneratorCpuDeviceSet(send_fixed_num.dev_num);
    if(rc != GT_OK)
    {
        cpssOsPrintf("tgfTrafficGeneratorCpuDeviceSet(%d) failed!\n", send_fixed_num.dev_num);
        return rc;
    }

    /* state the TGF to not recognize the 2 bytes after the macSa as 'vlan tag' ...
       unless 'by accident' will have next value .
       NOTE: no need to restore the value since 'we always need it'
    */
    (void)tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /*cpssGlobalMtxUnlock();*/

    /* total Tx counts == (send_fixed_num.send_cnt * send_fixed_num.burst_base) */
    while(!sendFix_halt && (tx_done_cnt < send_fixed_num.send_cnt))
    {
        /* transmit send_fixed_num.burst_base packets a time */
        do
        {
            rc = tgfTrafficGeneratorPortTxEthTransmit(&portInterface,
                                                      &packetInfo,
                                                      send_fixed_num.burst_base,
                                                      numVfd,
                                                      vfdArray,
                                                      0, /* sleepAfterXCount */
                                                      0, /* burstSleepTime */
                                                      16 /*prvTgfBurstTraceCount*/,
                                                      withLoopBack);
            if(rc != GT_OK)
            {
                /* getting buffer from the pool may got rc = GT_OUT_OF_CPU_MEM */
                /*cpssOsPrintf("tgfTrafficGeneratorPortTxEthTransmit() failed, sleep 1 sec and retry again!\n");*/
                sleep(1);
            }
        }while(!sendFix_halt && (rc != GT_OK));

        tx_done_cnt++;
    }
    /*cpssGlobalMtxLock();*/

    return rc;
}

/**
* @internal getFileCountInFolder
* @endinternal
*
* @brief   Get file count in folder
*
* @param[in] folder      - input folder
*
* @retval total          - file count
*/
static GT_U32 getFileCountInFolder
(
    IN GT_CHAR  *folder
)
{
    GT_U32 total = 0;
    struct dirent *ptr;
    DIR *dir;

    dir = opendir(folder);
    if(dir == NULL)
    {
        cpssOsPrintf("Open DIR failed!\n");
        return 0;
    }

    while((ptr = readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0)
        {
            continue;
        }

        if(ptr->d_type == 8)    /* DT_REG == 8 */
        {
            total++;
        }
    }
    closedir(dir);

    return total;
}

/**
* @internal getFileNameInFolder
* @endinternal
*
* @brief   Get file name in folder
*
* @param[in] folder            - input folder
* @param[out] fileNameArr      - folder name array
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
static GT_STATUS getFileNameInFolder
(
    IN GT_CHAR  *folder,
    IN GT_CHAR  **fileNameArr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  i = 0;
    DIR *dir;
    struct dirent *ptr;

    dir = opendir(folder);
    if(dir == NULL)
    {
        cpssOsPrintf("Open DIR failed!\n");
        rc = GT_FAIL;
        return rc;
    }

    while((ptr = readdir(dir)) != NULL)
    {
        if(cpssOsStrCmp(ptr->d_name,".") == 0 || cpssOsStrCmp(ptr->d_name,"..") == 0)
        {
            continue;
        }

        if(ptr->d_type == 8)    /* DT_REG == 8 */
        {
            cpssOsSprintf(fileNameArr[i], "%s", ptr->d_name);
            i++;
        }
    }
    closedir(dir);

    return rc;
}

/**
* @internal taskChkSendFixedDone
* @endinternal
*
* @brief   The task that sending all configured packets
*
* @param[in] param            - burstCount
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
static unsigned __TASKCONV taskChkSendFixedDone
(
    GT_VOID *param
)
{
    GT_STATUS   rc = GT_OK;
    GT_32       fd;
    GT_U32      xcat3BoardType;
    GT_U32      i, burstCount;

    GT_U32      quot_1st, remainder_1st, quot_2nd, remainder_2nd;
    GT_U32      base = TFGEN_TX_BASE, thread_avail = TFGEN_THREAD_PER_FILE;
    GT_U32      send_burst_for_one, send_cnt_for_base;

    GT_U32      file_cnt = 0;                               /* File counts under TFGEN_TX_FOLDER */
    GT_U32      index_cnt_for_same_file = 0;                /* Indicate same file content in the array */
    GT_U32      real_thread_per_file;                       /* Actual thread cnts we create to send per file */
    GT_U32      thread_all;                                 /* Total threads we create, for later pthread_join() */
    GT_U32      idx;                                        /* Index of send_fixed_array */
    GT_CHAR     **file_array = NULL, *pfile = NULL;         /* Array to record files' name under TFGEN_TX_FOLDER */
    WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC *send_fixed_array = NULL;
    pthread_t   *t_array = NULL;
    GT_VOID     **t_rtn = NULL;

    /* flock TFGEN_TX_DONE_FILE, to let GUI server know that now is sending packets */
    if((fd = open(TFGEN_TX_DONE_FILE, O_WRONLY|O_CREAT)) != -1)
    {
        if(flock(fd, LOCK_EX) == 0)
        {
            burstCount = (GT_U32)((GT_UINTPTR)param);

            /* get WNC SKU ID */
            xcat3BoardTypeGet(&xcat3BoardType);

            /* Get file counts under TFGEN_TX_FOLDER */
            file_cnt = getFileCountInFolder(TFGEN_TX_FOLDER);
            if(0 == file_cnt)
            {
                cpssOsPrintf("No files under %s, please check!\n", TFGEN_TX_FOLDER);
                return GT_FAIL;
            }

            /* Get file name under TFGEN_TX_FOLDER */
            file_array = (GT_CHAR **)cpssOsMalloc(file_cnt * sizeof(GT_CHAR *));
            if(NULL == file_array)
            {
                cpssOsPrintf("file_array allocation failed!\n");
                rc = GT_FAIL;
                goto exit;
            }
            /* The lengh of file name with path assigns to 64 */
            pfile = (GT_CHAR *)cpssOsMalloc(file_cnt * 64 * sizeof(GT_CHAR));
            if(NULL == pfile)
            {
                cpssOsPrintf("pfile allocation failed!\n");
                rc = GT_FAIL;
                goto exit;
            }
            for(i = 0; i < file_cnt; i++, pfile+=64)
            {
                file_array[i] = pfile;
                cpssOsMemSet(file_array[i], 0, 64);
            }
            rc = getFileNameInFolder(TFGEN_TX_FOLDER, file_array);
            if(GT_OK != rc)
            {
                goto exit;
            }

            /* Allocate threads to send packets, ex. if burstCount is 12345678:
                quot_1st      = 12345678 / 2000 = 6172
                remainder_1st = 12345678 % 2000 = 1678      // Use one thread to send

                quot_2nd      = 6172 / (10-1) = 685         // Use other 9 threads to send
                remainder_2nd = 6172 % (10-1) = 7           // Come along with above first thread
                
                send_burst_for_one  = remainder_2nd * base + remainder_1st = 7 * 2000 + 1678 = 15678
                send_cnt_for_base = quot_2nd = 685

                Therefore, below are how we send:
                1 thread send with send_burst_for_one = 15678, count for loop = 1
                9 thread send with base = 2000, count for loop = send_cnt_for_base = 685
                1 * 15678 * 1 + 9 * 2000 * 685 = 15678 + 12330000 = 12345678.
             */

            /* Divide 1st time */
            quot_1st = burstCount / base;
            remainder_1st = burstCount % base;

            /* Divide 2nd time, reserve one thread for remainder_1st if not zero */
            quot_2nd = quot_1st / ((0 != remainder_1st)?(thread_avail - 1):(thread_avail));
            remainder_2nd = quot_1st % ((0 != remainder_1st)?(thread_avail - 1):(thread_avail));

            send_burst_for_one = remainder_2nd * base + remainder_1st;
            send_cnt_for_base = quot_2nd;

            if(0 != send_burst_for_one) index_cnt_for_same_file++;
            if(0 != send_cnt_for_base)  index_cnt_for_same_file++;


            /* Array of WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC, per file and index_cnt_for_same_file */
            send_fixed_array = (WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC *)
                               cpssOsMalloc(file_cnt * index_cnt_for_same_file * sizeof(WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC));
            if(NULL == send_fixed_array)
            {
                cpssOsPrintf("send_fixed_array allocation failed!\n");
                rc = GT_FAIL;
                goto exit;
            }

            for(i = 0; i < file_cnt * index_cnt_for_same_file; i++)
            {
                /* Ex. if file_cnt == 2, and index_cnt_for_same_file == 2 
                    Array size is 2, and two index a file, remainder part first. That is,
                      send_fixed_array[0].file_path  = file_1
                      send_fixed_array[0].send_cnt   = 1
                      send_fixed_array[0].burst_base = send_burst_for_one

                      send_fixed_array[1].file_path  = file_1
                      send_fixed_array[1].send_cnt   = send_cnt_for_base
                      send_fixed_array[1].burst_base = base

                      send_fixed_array[2].file_path  = file_2
                      send_fixed_array[2].send_cnt   = 1
                      send_fixed_array[2].burst_base = send_burst_for_one
                 
                      send_fixed_array[3].file_path  = file_2
                      send_fixed_array[3].send_cnt   = send_cnt_for_base
                      send_fixed_array[3].burst_base = base
                 */
                cpssOsMemSet(&send_fixed_array[i], 0, sizeof(WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC));
                if(xcat3BoardType == AC3BXH_WNC_OS6360P48)
                {
                    send_fixed_array[i].dev_num  = 1;
                    send_fixed_array[i].port_num = 12;
                }
                else
                {
                    /* FIXME, not define yet for other SKUs, set 0/0 as default */
                    send_fixed_array[i].dev_num  = 0;
                    send_fixed_array[i].port_num = 0;
                }
                cpssOsSnprintf(send_fixed_array[i].file_path, sizeof(send_fixed_array[i].file_path), "%s/%s",
                               TFGEN_TX_FOLDER, file_array[i/index_cnt_for_same_file]);

                if((send_burst_for_one != 0) && (i % index_cnt_for_same_file == 0))
                {
                    /* Remainder part first, if not zero */ 
                    send_fixed_array[i].send_cnt = 1;
                    send_fixed_array[i].burst_base = send_burst_for_one;
                }
                else
                {
                    send_fixed_array[i].send_cnt = send_cnt_for_base;
                    send_fixed_array[i].burst_base = base;
                }
            }

            if(sendFix_halt)
            {
                /* First call, or restore here if ever called stop */
                sendFix_halt = 0;
            }

            (0 != send_cnt_for_base) ? (real_thread_per_file = 10) : (real_thread_per_file = 1);
            thread_all = file_cnt * real_thread_per_file;

            /* For pthread_join() */
            t_array = (pthread_t *)cpssOsMalloc(thread_all * sizeof(pthread_t));
            t_rtn = (GT_VOID **)cpssOsMalloc(thread_all * sizeof(GT_VOID *));

            for(i = 0; i < thread_all; i++)
            {
                if(sendFix_halt)
                {
                    break;
                }

                if((i % real_thread_per_file != 0) && (1 != index_cnt_for_same_file))
                {
                    /* There are two send_fixed_array[] with same file but different cnt we need to send
                       Case of not first thread per file, send index i+1.
                     */

                    idx = (i / real_thread_per_file ) * index_cnt_for_same_file + 1;
                }
                else
                {
                    /* Case of the first thread per file */
                    idx = (i / real_thread_per_file) * index_cnt_for_same_file;
                }

                rc = pthread_create(&t_array[i], NULL, (GT_VOID*)taskSendFixedNumberPkt, (GT_VOID*)(&send_fixed_array[idx]));
                if(rc != GT_OK)
                {
                    /* Set t_array[i] to 0 for later to skip (id will start from 1?) */
                    t_array[i] = 0;
                    cpssOsPrintf("taskSendFixedNumberPkt[%d] create failed!\n", i);
                }
            }

            for(i = 0; i < thread_all; i++)
            {
                if(t_array[i] != 0)
                {
                    pthread_join(t_array[i], &t_rtn[i]);
                    if((GT_U32)t_rtn[i] != GT_OK)
                    {
                        cpssOsPrintf("rtn from taskSendFixedNumberPkt[%d] is [%d]!\n", i, (GT_U32)t_rtn[i]);
                    }
                }
            }

            if(flock(fd, LOCK_UN) != 0)
            {
                cpssOsPrintf("unlock %s failed!\n", TFGEN_TX_DONE_FILE);
            }
        }
        else
        {
            cpssOsPrintf("flock() %s failed!\n", TFGEN_TX_DONE_FILE);
        }
        close(fd);
    }
    else
    {
        cpssOsPrintf("open() %s failed!\n", TFGEN_TX_DONE_FILE);
        rc = GT_FAIL;
    }

exit:
    /* Free allocated memory */
    if(NULL != pfile)               cpssOsFree(pfile);
    if(NULL != file_array)          cpssOsFree(file_array);
    if(NULL != send_fixed_array)    cpssOsFree(send_fixed_array);
    if(NULL != t_array)             cpssOsFree(t_array);
    if(NULL != t_rtn)               cpssOsFree(t_rtn);
    
    return rc;
}

/**
* @ wncUtilsTfgenSendFixedNumberPktForTfGenStart
*
* @brief   Start the task that send fixed packet number for traffic generator.
*
* @param[in] burstCount          - number of burst packets
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - on error.
*/
GT_STATUS wncUtilsTfgenSendFixedNumberPktForTfGenStart
(
    IN GT_U32   burstCount
)
{
    GT_STATUS   rc = GT_OK;
    GT_CHAR     taskName[32];

    if(burstCount == 0)
    {
        cpssOsPrintf("Got burstCount = [0], please check the input!\n");
        return GT_FAIL;
    }

    cpssOsSnprintf(taskName, sizeof(taskName), "chkSendDone");
    rc = cpssOsTaskCreate ( taskName,                           /* Task Name      */
                            10,                                 /* Task Priority  */
                            0x10000,                            /* Stack Size     */
                            taskChkSendFixedDone,               /* Starting Point */
                            (GT_VOID*)((GT_UINTPTR)burstCount), /* Arguments list */
                            NULL);                              /* task ID        */
    if(rc != GT_OK)
    {
        cpssOsPrintf("%s create failed!\n", taskName);
    }

    return rc;
}

/**
* @ wncUtilsTfgenSendFixedNumberPktForTfGenStop
*
* @brief   Stop the task that send fixed packet number for traffic generator.
*
* @retval GT_OK                  - on success.
*/
GT_STATUS wncUtilsTfgenSendFixedNumberPktForTfGenStop
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* It will stop all taskSendFixedNumberPkt */
    sendFix_halt = 1;

    return rc;
}
