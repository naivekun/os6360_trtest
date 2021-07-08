/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDragonite.c
*
* @brief Dragonite appDemo code.
*
* @version   1
********************************************************************************
*/
/*#include <appDemo/boardConfig/appDemoBoardConfig.h>*/
#include <appDemo/dragonite/appDemoDragonite.h>

#if defined(_linux) && !defined(ASIC_SIMULATION)

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#endif

#define APP_DEMO_DRAGONITE_FW_FILE_SIZE     65536
#define APP_DEMO_DRAGONITE_FW_FILE_PATH     "/usr/bin/dragonite.bin"

static GT_U8 dragoniteDevNum = 0;

typedef struct POE_CUSTOMER_INFO_DATA_STC
{
  GT_U32 version_number; /* struct version */
  GT_U32 struct_size; /* struct sizeof */
  GT_U8  *sign_date; /* date of generate the version  */
  GT_U8  *sign_time; /* time of generate the version  */
  GT_U8  *chr_array_ptr; /* TBD string */
  GT_U16 *software_pn; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U8  *software_build; /* this value is the same as in the 15 BYTE communication protocol doc */
  GT_U8  *product_number; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U16 *kernel_version; /* this value is the same as in the 15 BYTE communication protocol doc  */
  GT_U8  *salad_param_code; /* this value is the same as in the 15 BYTE communication protocol doc  */

}POE_CUSTOMER_INFO_DATA_STC;


GT_STATUS appDemoDragoniteMessageCheckSumCalc
(
    IN  GT_U8   *bufferPtr,
    IN  GT_U32  msgLen,
    OUT GT_U16  *checkSumPtr
)
{
    GT_U16  checkSum = 0;

    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);
    CPSS_NULL_PTR_CHECK_MAC(checkSumPtr);

    while (msgLen > 0)
    {
        msgLen--;
        checkSum += bufferPtr[msgLen];
    }

    *checkSumPtr = checkSum;

    return GT_OK;
}

GT_STATUS poeMessageCheckSumCalc
(
    IN  GT_U8   *bufferPtr,
    IN  GT_U32  msgLen,
    OUT GT_U16  *checkSumPtr
)
{
    GT_U16  checkSum = 0;

    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);
    CPSS_NULL_PTR_CHECK_MAC(checkSumPtr);

    while (msgLen > 0) {
        msgLen--;
        checkSum += bufferPtr[msgLen];
    }

    bufferPtr[13] = U32_GET_FIELD_MAC(checkSum, 8, 8);
    bufferPtr[14] = U32_GET_FIELD_MAC(checkSum, 0, 8);

    *checkSumPtr = checkSum;

    return GT_OK;
}



static GT_STATUS prvDrvCpssWriteMasked(
    IN  void *drvData,
    IN  GT_U32 addrSpace,
    IN  GT_U32 regAddr,
    IN  GT_U32 data,
    IN  GT_U32 mask)
{
    GT_U8 dev = (GT_U8)((GT_UINTPTR)drvData);
    GT_STATUS rc = GT_FAIL;
    if (addrSpace == 0)
        return cpssDrvPpHwRegBitMaskWrite(dev, 0, regAddr, mask, data);
    if (mask != 0xffffffff)
    {
        GT_U32 oldVal;
        if (addrSpace == 1)
            rc = cpssDrvPpHwInternalPciRegRead(dev, 0, regAddr, &oldVal);
        if (addrSpace == 2)
            rc = cpssDrvHwPpResetAndInitControllerReadReg(dev, regAddr, &oldVal);
        if (rc != GT_OK)
            return rc;
        data &= mask;
        data |= oldVal & (~mask);
    }
    if (addrSpace == 1)
        return cpssDrvPpHwInternalPciRegWrite(dev, 0, regAddr, data);
    if (addrSpace == 2)
        return cpssDrvHwPpResetAndInitControllerWriteReg(dev, regAddr, data);
    return GT_NOT_SUPPORTED;
}

/**
* @internal appDemoDragoniteDevSet function
* @endinternal
*
* @brief   Select device with dragonite unit
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - device not exists/not initialized
*/
GT_STATUS appDemoDragoniteDevSet(
    IN  GT_U8   devNum
)
{
    if (cpssDrvHwPpHwInfoStcPtrGet(devNum, 0) == NULL)
    {
        cpssOsPrintf("device %d not exists/not initialized\n", devNum);
        return GT_BAD_PARAM;
    }
    dragoniteDevNum = devNum;
    return GT_OK;
}






static GT_BOOL poeInitializationDone = GT_FALSE;



/**
* @internal appDemoDragoniteSoftwareDownload function
* @endinternal
*
* @brief   Example to show Dragonite FW download.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteSoftwareDownload()
{

    GT_STATUS rc = GT_OK;
    char *buf = NULL;
    GT_U32 nr;
    GT_BOOL msg;
    GT_32 rfd = 0, i;
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    CPSS_HW_INFO_STC    *hwInfoPtr;

    hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dragoniteDevNum, 0);
    if (hwInfoPtr == NULL)
    {
        cpssOsPrintf("device %d is not initialized or doesn't exists\n", dragoniteDevNum);
        cpssOsPrintf("use appDemoDragoniteDevSet(devId) to select device\n");
        return GT_FAIL;
    }
    /* Initialize Dragonite driver */
    rc = extDrvDragoniteDriverInit(
            hwInfoPtr, (void*)((GT_UINTPTR)dragoniteDevNum),
            (DRAGONITE_HW_WRITE_MASKED_FUNC)prvDrvCpssWriteMasked);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* config and enable dragonite sub-system */
    rc = extDrvDragoniteDevInit();
    if(rc != GT_OK)
    {
        return rc;
    }

    buf = cpssOsMalloc(APP_DEMO_DRAGONITE_FW_FILE_SIZE);
    if (buf == NULL)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: malloc failed\r\n");
        return GT_OUT_OF_CPU_MEM;
    }

    /* read FW file */
    rfd = open(APP_DEMO_DRAGONITE_FW_FILE_PATH, O_RDONLY);
    if (rfd <= 0)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: open file failed\r\n");
        rc = GT_BAD_STATE;
        goto fail_label;
    }

    nr = read(rfd, buf, APP_DEMO_DRAGONITE_FW_FILE_SIZE);
    if (nr != APP_DEMO_DRAGONITE_FW_FILE_SIZE)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: read file failed\r\n");
        rc = GT_BAD_STATE;
        goto fail_label;
    }

    rc = extDrvDragoniteFwDownload(buf, nr);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: download FW failed\r\n");
        goto fail_label;
    }

   /* Init DTCM with 0xFF */
    for (i = 0; i < 0xB00; i++)
    {
        rc = extDrvDragoniteRegisterWrite(HOST_OUTPUT_TO_POE_REG_ADDR + i, 0xFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("appDemoDragoniteSoftwareDownload: write register failed, addr=0x%x\r\n", (HOST_OUTPUT_TO_POE_REG_ADDR + i));
            goto fail_label;
        }
    }

    /* Init Host protocol version to '1' in order to 'speak' with Dragonite with the same 'language' 
	check microsemi dragonite firmware release note about protocol version */
    rc = extDrvDragoniteRegisterWrite(HOST_PROT_VER_REG_ADDR, 0x4);
    if(rc != GT_OK)
    {
        goto fail_label;
    }

/*
23021020_0817_001.bin:

Beta 2.10 for the Marvell Dragonite which supports 200Mhz & 250Mhz.

This version supports by default 200Mhz, and in order to set it to 250Mhz you need follow the instructions:
    Register 0x00 should be set to 0x00010004.
    Frequency by register 0x90 (each bit is 1Hz). 
	To Aldrin/AC3X , set register 0x90 = 0xEE6B280 (250M)
	To Aldrin/AC3X , set register 0x90 = 0xBEBC200 (200M)
    Actual frequency can be read from register 0x140
*/
    GT_U32 dragoniteRegData = 0;
    rc = extDrvDragoniteRegisterWrite(0x0,   0x00010004); if(rc != GT_OK){goto fail_label;}
    rc = extDrvDragoniteRegisterWrite(0x90,  0xBEBC200); if(rc != GT_OK){goto fail_label;}
	
	

    msg = GT_TRUE;
    rc = extDrvDragoniteUnresetSet(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: Dragonite UnReset failed\r\n");
        goto fail_label;
    }

    cpssOsTimerWkAfter(1000);

    /* Get system status message from POE that is ready after unreset */
    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteSoftwareDownload: MSG receive was failed\r\n");
        goto fail_label;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n\r\n\r\n", msg_reply[8]);


    poeInitializationDone = GT_TRUE;


/* fallthrough in case ok */
fail_label:
    if(buf != NULL)
    {
        cpssOsFree(buf);
    }

    close(rfd);

    return rc;
}


/**
* @internal appDemoDragoniteProtocolVersionShow function
* @endinternal
*
* @brief   Print version info from FW binary image.
*
* @param[in] buf                      - (pointer to) FW buffer
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteProtocolVersionShow
(
    IN char *buf
)
{
    POE_CUSTOMER_INFO_DATA_STC *customerInfoPtr;
    GT_U32 verInfoOffset;

    CPSS_NULL_PTR_CHECK_MAC(buf);

    verInfoOffset = *(GT_U32*)(buf + APP_DEMO_DRAGONITE_FW_FILE_SIZE - 8);
    customerInfoPtr = (POE_CUSTOMER_INFO_DATA_STC*)(buf + verInfoOffset);

    osPrintf("buf 0x%x, offset: 0x%x, ptr: 0x%x\n", buf, verInfoOffset, customerInfoPtr);

    osPrintf("sw build: %d\n", *(buf + (GT_UINTPTR)customerInfoPtr->software_build) );
    osPrintf("sw pn: %d\n", *(GT_U16*)(buf + (GT_UINTPTR)customerInfoPtr->software_pn) );
    osPrintf("date: %.15s\n", buf + (GT_UINTPTR)customerInfoPtr->sign_date);

    return GT_OK;
}



/**
* @internal appDemoDragoniteMcuVersionRead function
* @endinternal
*
* @brief   Read MCU Protocol Version register.
*
* @param[out] versionPtr               - (pointer to) version value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoDragoniteMcuVersionRead
(
    IN GT_U32 *versionPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(versionPtr);

    return extDrvDragoniteRegisterRead(PROT_VER_REG_ADDR ,versionPtr);
}

/**
* @internal appDemoDragoniteMcuResetCauseUpdate function
* @endinternal
*
* @brief   Inform the POE software about the reason that the host decided to
*         reset the POE MCU
* @param[in] cause                    -  reset
*                                      0xFF - power reset
*                                      0x01 - POE Not responding
*                                      0x02 - Application request
*                                      0x03 - Communication errors
*                                      0x04 - Host Application Reboot
*                                      0x05 - Undefined cause
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoDragoniteMcuResetCauseUpdate
(
    GT_U32 cause
)
{
    GT_STATUS  rc;
    GT_U32     regVal;
    GT_U16     counter;
    GT_U32     data;

    rc = extDrvDragoniteRegisterRead(HOST_RST_CAUSE_REG_ADDR, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    counter = (regVal & 0xFF);

    if (counter == MEM_INIT_VAL)
    {
        counter = 1;
    }
    else
    {
        counter++;
    }
    data = cause<<24 | counter;

    rc = extDrvDragoniteRegisterWrite(HOST_RST_CAUSE_REG_ADDR, data);
    return rc;
}

/**
* @internal appDemoDragoniteReadRegister function
* @endinternal
*
* @brief   Print value of Dragonite register
*
* @param[in] addr                     - address to read.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteReadRegister
(
    GT_U32 addr
)
{
    GT_STATUS rc;
    GT_U32 regValue;

    rc = extDrvDragoniteRegisterRead(addr, &regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\r\n\r\n extDrvDragoniteRegisterRead: FAILURE: rc = %d\r\n", rc);
        return rc;
    }
    cpssOsPrintf("\r\n\r\nextDrvDragoniteRegisterRead: value = 0x%x\r\n", regValue);

    return GT_OK;
}


/**
* @internal appDemoDragoniteWriteRegister function
* @endinternal
*
* @brief   Write value to Dragonite register.
*
* @param[in] addr                     - address to write.
* @param[in] regValue                 - value to write.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteWriteRegister
(
    GT_U32 addr,
    GT_U32 regValue
)
{
    GT_STATUS rc;

    rc = extDrvDragoniteRegisterWrite(addr, regValue);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\r\n\r\n appDemoDragoniteReadRegister: FAILURE: rc = %d\r\n", rc);
        return rc;
    }

    return GT_OK;
}

#if !defined(LINUX_NOKM)
/**
* @internal appDemoDragoniteMemTypeSet function
* @endinternal
*
* @brief   COnfigure memory type to Dragonite access
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteMemTypeSet(GT_BOOL memType)
{
    GT_STATUS rc;

    rc = extDrvDragoniteMemoryTypeSet(memType);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoDragoniteMemTypeSet: FAIL: rc = %d\r\n", rc);
        return rc;
    }

    return GT_OK;
}
#endif


/**
* @internal appDemoDragoniteTestExample1 function
* @endinternal
*
* @brief   POE 15B protocol command example
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteTestExample1()
{
    GT_STATUS rc;
    GT_U8 msg[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U16  checkSum = 0;

    /*
        Host Request:
            Get System Status
            DATA[0] = 0x2 - Request
            DATA[2] = 0x7 - Global
            DATA[3] = 0x3d - SystemStatus

        Controller Response:
            DATA[0] = 0x3 - Telemetry
            DATA[2] = CPU status 1
            DATA[3] = CPU status 2
            DATA[4] = Factory Default
            DATA[5] = GIE
            DATA[6] = Private Label
            DATA[7] = User byte
            DATA[8] = Device Fail
            DATA[9] = Temperature Disconnect
            DATA[10] = Temperature Alarm
            DATA[11] = Interrupt register
            DATA[12] = Interrupt register

        CPU status-1: Bit0 = ‘1’ indicates PoE controller error. Bit1 = ‘1’ indicates that firmware download is required.
        CPU status-2: Bit0 = ‘1’ indicates PoE controller memory error. Bit1 = ‘1’ indicates error (if there are less than eight
        PoE devices in the system, then Bit1= ‘1’).
        Factory default: Bit0 = ‘1’ indicates that factory default parameters are currently set.
        GIE (General Internal Error): When different from 0x00,it  indicates a general internal error.
        Private Label: Saved in the RAM. Equals 0x00 after reset. Refer to "Set System Status" command.
        User Byte - Saved in memory. Equals 0xFF, once set to factory default.
        Device Fail:  (1) Bits 0 to 7 indicate a failed PoE device(s).
            ’1’ = Fail or missing PoE Device, ‘0’ = PoE Device is OK.
        Temperature disconnect:  (1)  Bits 0 to 7 indicate over - heated PoE device(s).
        This over-heating causes disconnection of all ports.
            '1' = This PoE device caused disconnection due to high temperature, '0' = Temperature is OK.
        Temperature alarm:  (1)  Bits 0 to 7 indicate over heated PoE device(s).
        if temperature exceeds a pre-defined user defined limit, (Set PoE Device Params command),
        then the appropriate bit changes to ‘1’.
            '1' = High temperature, '0' = Temperature is OK.
        Interrupt register latches a transition when an event occurs. The transition might be one or more of several port
        status changes, PoE device status event/s or system event/s, depending on event definition.
        (1)  Bit 0 – PoE Device-‘0’, Bit 1 – PoE Device-‘1’, Bit 2 – PoE Device-2, Bit 7 – PoE Device-7.
        Device number is set according to its connectivity to the controller.

    */
    msg[0] = 0x2;
    msg[2] = 0x7;
    msg[3] = 0x3d;
    /* Fill byte[14] and byte[15] in 15B protocol with checksum */
    appDemoDragoniteMessageCheckSumCalc(msg, 13, &checkSum);
    msg[13] = U32_GET_FIELD_MAC(checkSum, 8, 8);
    msg[14] = U32_GET_FIELD_MAC(checkSum, 0, 8);

    rc = extDrvDragoniteMsgWrite(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
        return rc;
    }

    cpssOsTimerWkAfter(100);

    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
        return rc;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n", msg_reply[8]);
    cpssOsPrintf("REC_DATA[9] = 0x%x\r\n", msg_reply[9]);
    cpssOsPrintf("REC_DATA[10] = 0x%x\r\n", msg_reply[10]);
    cpssOsPrintf("REC_DATA[11] = 0x%x\r\n", msg_reply[11]);
    cpssOsPrintf("\r\n\r\n");

    return GT_OK;
}


/**
* @internal appDemoDragoniteTestExample2 function
* @endinternal
*
* @brief   POE 15B protocol command example
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDragoniteTestExample2()
{
    GT_STATUS rc;
    GT_U8 msg[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U16  checkSum = 0;

    /*
        Host Request:
            Get System Status
            DATA[0] = 0x2 - Request
            DATA[2] = 0x7 - Global
            DATA[3] = 0xb - Supply
            DATA[4] = 0x17 - Main

        Controller Response:
            DATA[0] = 0x3 - Telemetry
            DATA[2] = Power Consumption
            DATA[3] = Power Consumption
            DATA[4] = Max ShutDown Voltage
            DATA[5] = Max ShutDown Voltage
            DATA[6] = Min ShutDown Voltage
            DATA[7] = Min ShutDown Voltage
            DATA[8] = N
            DATA[9] = Power Bank
            DATA[10] = Power Limit
            DATA[11] = Power Limit
            DATA[12] = N

            Telemetry for main power supply parameters (within the working power budget):
            Power Consumption: Actual momentary total power consumption (units are in watts).
            Max Shutdown Voltage: Maximum voltage level set; above this level, PoE ports shutdown. Units are in decivolts.
            Min Shutdown Voltage: Minimum voltage level set; below this level, PoE ports shutdown. Units are given in decivolts.
            Power Bank: The current active Power Bank.
            Power Limit: If power consumption exceeds this level, lowest priority ports will be disconnected. To set the desired value, refer to
            (units are in watts).

    */
    msg[0] = 0x2;
    msg[2] = 0x7;
    msg[3] = 0xb;
    msg[4] = 0x17;
    /* Fill byte[14] and byte[15] in 15B protocol with checksum */
    appDemoDragoniteMessageCheckSumCalc(msg, 13, &checkSum);
    msg[13] = U32_GET_FIELD_MAC(checkSum, 8, 8);
    msg[14] = U32_GET_FIELD_MAC(checkSum, 0, 8);

    rc = extDrvDragoniteMsgWrite(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
        return rc;
    }

    cpssOsTimerWkAfter(100);

    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
        return rc;
    }

    cpssOsPrintf("REC_DATA[0] = 0x%x\r\n", msg_reply[0]);
    cpssOsPrintf("REC_DATA[1] = 0x%x\r\n", msg_reply[1]);
    cpssOsPrintf("REC_DATA[2] = 0x%x\r\n", msg_reply[2]);
    cpssOsPrintf("REC_DATA[3] = 0x%x\r\n", msg_reply[3]);
    cpssOsPrintf("REC_DATA[4] = 0x%x\r\n", msg_reply[4]);
    cpssOsPrintf("REC_DATA[5] = 0x%x\r\n", msg_reply[5]);
    cpssOsPrintf("REC_DATA[6] = 0x%x\r\n", msg_reply[6]);
    cpssOsPrintf("REC_DATA[7] = 0x%x\r\n", msg_reply[7]);
    cpssOsPrintf("REC_DATA[8] = 0x%x\r\n", msg_reply[8]);
    cpssOsPrintf("REC_DATA[9] = 0x%x\r\n", msg_reply[9]);
    cpssOsPrintf("REC_DATA[10] = 0x%x\r\n", msg_reply[10]);
    cpssOsPrintf("REC_DATA[11] = 0x%x\r\n", msg_reply[11]);
    cpssOsPrintf("\r\n\r\n");

    return GT_OK;
}

/* For debug use */
/* Example: shell-execute poeTest2 "2,51,5,11,0,78,78,78,78,78,78,78,78" */
GT_STATUS poeTest2(GT_CHAR *cmdPtr)
{
    GT_STATUS 	rc;
    GT_U8	msg[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U8	msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
    GT_U16	checkSum = 0;

    int i = 1;

    msg[0] = atoi(strtok(cmdPtr, ","));
    for (i = 1; i <= 12; i++ )
    {
        msg[i] = atoi(strtok(NULL, ","));
    }

    /* Fill byte[14] and byte[15] in 15B protocol with checksum */
    poeMessageCheckSumCalc(msg, 13, &checkSum);

    cpssOsPrintf("DEBUG: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
        msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9],
        msg[10], msg[11], msg[12], msg[13], msg[14]);

    rc = extDrvDragoniteMsgWrite(msg);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
        return rc;
    }

    cpssOsTimerWkAfter(100);
    rc = extDrvDragoniteMsgRead(msg_reply);
    if(rc != GT_OK)
    {
        cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
        return rc;
    }

    cpssOsPrintf("REPLY: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
        msg_reply[0], msg_reply[1], msg_reply[2], msg_reply[3], msg_reply[4],
        msg_reply[5], msg_reply[6], msg_reply[7], msg_reply[8], msg_reply[9],
        msg_reply[10], msg_reply[11], msg_reply[12], msg_reply[13], msg_reply[14]);

    return GT_OK;
}

GT_STATUS poeTest(GT_CHAR *cmdPtr)
{

    GT_STATUS rc = GT_OK;
    GT_U8 	*buf = NULL;
    GT_U32 	nr;
    GT_BOOL msg;
    GT_32 	rfd = 0, i;
    GT_U8 	msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
	GT_U16  checkSum = 0;

    int ports = 0;
    if (appDemoPpConfigDevAmount == 1)
        ports = 24;
    else if (appDemoPpConfigDevAmount == 2)
        ports = 48;
    else
        return rc;

	switch(cmdPtr[0])
	{
		case 'i':
		{
			/* call to init Dragonite function */
			if ( poeInitializationDone == GT_TRUE )
				cpssOsPrintf("Dragonite firmware initialization is already done, nothing executed \r\n");
			else
			{
				rc = appDemoDragoniteSoftwareDownload();
				if(rc != GT_OK)
				{
					return rc;
				}
			}

			break;
		}	
		/* show software version */	
		case 'v':
		{
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = 
							{0x2,0x0,0x7,0x1e,0x21,
								0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,
								0x0,0x0};
			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);

			rc = extDrvDragoniteMsgRead(msg_reply);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				return rc;
			}
			
			cpssOsPrintf("firmware verison = %2d%04d%02d_%04d_%03d \n", msg_reply[4], msg_reply[6],msg_reply[7], 
				(GT_U16)( (msg_reply[9] << 8) | msg_reply[10] ),
										msg_reply[8]
																);
			
			break;
		}
		/* 		enable channel
				msg[4] = 0x80  means all channels 
			*/
		case 'e':
		{
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0, 3,  5, 0xC, 0x80, 1, 1, 
														0x4E,0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 
														0x0, 0x0 };
			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);
			
			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				return rc;
			}
			
			break;
		}
		case 'd':
		{
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0, 0x10,  5, 0xC, 0x80, 0, 1, 
														0x4E,0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 
														0x0, 0x0 };
														
			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);
			
			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				return rc;
			}
			
			break;
		}
		
		case 's':
		{
			GT_U32	offset;
			GT_U32	value;
			
			cpssOsPrintf("PoE Dragonite DTCM dump :\n");
			
			for(offset=0 ; offset< 0x38 ; offset+=4 )
			{
				/* reserved area */
				if(offset > 0x1C && offset < 0x30)
					continue;
				
				rc = extDrvDragoniteRegisterRead(offset, &value);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteRegisterRead: failed\r\n");
					return rc;
				}else
					cpssOsPrintf("offset 0x%04X = 0x%x \n", offset, (GT_U16)value);
			}
			
			break;
		}
		
		case 't':
		{
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0, 0x20,  0x7, 0x2b, 0x3,
														0x4e, 0x4e,	0x4E,0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 
														0x0, 0x0 };
														
			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);
			
			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}
			
			break;
		}

		case 'f':
		{
			int i = 0, start = 0 , end = ports, count = 0;
			char *temp;

			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0};
			GT_U8 GetPortMeasurements[DRAGONITE_DATA_MSG_LEN] = {0};
			GT_U8 GetPortPriority[DRAGONITE_DATA_MSG_LEN] = {0};
			GT_U8 GetPortPowerLimit[DRAGONITE_DATA_MSG_LEN] = {0};
			GT_U8 GetSinglePortStatus[DRAGONITE_DATA_MSG_LEN] = {0};

			/* Show one port information */
			if (cmdPtr[1] == 44)
			{
				for (i = 0, count = 0; i < strlen(cmdPtr); i++)
				{
					if (cmdPtr[i] == 44)
						count++;
				}

				if (count != 1 || cmdPtr[strlen(cmdPtr)-1] == 44)
				{
					cpssOsPrintf("Wrong format\n");
					return rc;
				}

				strtok(cmdPtr, ",");
				temp = strtok(NULL, ",");
				for (i = 0; i < strlen(temp); i++)
				{
					if (!isdigit(temp[i]))
					{
						cpssOsPrintf("Wrong number: channel\n");
						return rc;
					}
				}
				start = atoi(temp);
				end = start + 1;

				if ( (start < 0 || start >= ports) )
				{
					cpssOsPrintf("Wrong number: channel\n");
					return rc;
				}

				goto SHOW_SINGLE_PORT;
			}

			/* 4.4.3 Get Total Power */
			msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x07;
			msg_request[3] = 0x0B; msg_request[4] = 0x60; msg_request[5] = 0x4E;
			msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
			msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
			msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if(rc != GT_OK || msg_request[1] != msg_reply[1])
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			cpssOsPrintf("\nPower Consumption: %d watts\n", (GT_U16)( (msg_reply[2] << 8) | msg_reply[3] ));
			cpssOsPrintf("Calculated Power: %d watts\n", (GT_U16)( (msg_reply[4] << 8) | msg_reply[5] ));
			cpssOsPrintf("Available Power: %d watts\n", (GT_U16)( (msg_reply[6] << 8) | msg_reply[7] ));
			cpssOsPrintf("Power Limit: %d watts\n", (GT_U16)( (msg_reply[8] << 8) | msg_reply[9] ));
			cpssOsPrintf("Power Bank: %d\n", msg_reply[10]);

			cpssOsPrintf("\nPoE Device Status:\n");
			cpssOsPrintf("Device\tASICstatus\tTemperature\n");
			for (i = 0; i < (ports / 8); i++)
			{
				/* 4.2.2 Get PoE Device Status */
				msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x07;
				msg_request[3] = 0x87; msg_request[4] = i; msg_request[5] = 0x00;
				msg_request[6] = 0x00; msg_request[7] = 0x00; msg_request[8] = 0x00;
				msg_request[9] = 0x00; msg_request[10] = 0x00; msg_request[11] = 0x00;
				msg_request[12] = 0x00; msg_request[13] = 0x0; msg_request[14] = 0x0;

				/* Fill byte[14] and byte[15] in 15B protocol with checksum */
				poeMessageCheckSumCalc(msg_request, 13, &checkSum);

				rc = extDrvDragoniteMsgWrite(msg_request);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
					return rc;
				}

				cpssOsTimerWkAfter(100);
				rc = extDrvDragoniteMsgRead(msg_reply);
				if(rc != GT_OK || msg_request[1] != msg_reply[1])
				{
					cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
					cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
					return rc;
				}

				cpssOsPrintf("%6d\t      0x%02X\t%10dC\n", i, msg_reply[5], 
					(int)msg_reply[9] > 205 ? 256 - (int)msg_reply[9] : msg_reply[9]);
			}

SHOW_SINGLE_PORT:
			cpssOsPrintf("\nPort Status & Configurations:\n");
			cpssOsPrintf("Port\tVOLTS\tmA\tWATTS\tClass\t   Status(Code)\tMode\tEnabled\tPowerLimit\tPriority\n"); 

			/* Show port status */
			for (i = start; i < end; i++)
			{
				/* 4.4.11 Get Port Measurements */
				msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
				msg_request[3] = 0x25; msg_request[4] = i; msg_request[5] = 0x4E;
				msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
				msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
				msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

				/* Fill byte[14] and byte[15] in 15B protocol with checksum */
				poeMessageCheckSumCalc(msg_request, 13, &checkSum);

				rc = extDrvDragoniteMsgWrite(msg_request);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
					return rc;
				}

				cpssOsTimerWkAfter(100);
				rc = extDrvDragoniteMsgRead(GetPortMeasurements);
				if((rc != GT_OK) || (msg_request[1] != GetPortMeasurements[1]))
				{
					cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
					cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], GetPortMeasurements[1]);
					return rc;
				}

				/* 4.3.18 Get Port Priority */
				msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
				msg_request[3] = 0x0A; msg_request[4] = i; msg_request[5] = 0x4E;
				msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
				msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
				msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

				/* Fill byte[14] and byte[15] in 15B protocol with checksum */
				poeMessageCheckSumCalc(msg_request, 13, &checkSum);

				rc = extDrvDragoniteMsgWrite(msg_request);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
					return rc;
				}

				cpssOsTimerWkAfter(100);
				rc = extDrvDragoniteMsgRead(GetPortPriority);
				if((rc != GT_OK) || (msg_request[1] != GetPortPriority[1]))
				{
					cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
					cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], GetPortPriority[1]);
					return rc;
				}

				/* 4.3.11 Get Port Power Limit */
				msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
				msg_request[3] = 0x0B; msg_request[4] = i; msg_request[5] = 0x4E;
				msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
				msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
				msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

				/* Fill byte[14] and byte[15] in 15B protocol with checksum */
				poeMessageCheckSumCalc(msg_request, 13, &checkSum);

				rc = extDrvDragoniteMsgWrite(msg_request);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
					return rc;
				}

				cpssOsTimerWkAfter(100);
				rc = extDrvDragoniteMsgRead(GetPortPowerLimit);
				if((rc != GT_OK) || (msg_request[1] != GetPortPowerLimit[1]))
				{
					cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
					cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], GetPortPowerLimit[1]);
					return rc;
				}

				/* 4.3.21 Get Single Port Status */
				msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
				msg_request[3] = 0x0E; msg_request[4] = i; msg_request[5] = 0x4E;
				msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
				msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
				msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

				/* Fill byte[14] and byte[15] in 15B protocol with checksum */
				poeMessageCheckSumCalc(msg_request, 13, &checkSum);

				rc = extDrvDragoniteMsgWrite(msg_request);
				if(rc != GT_OK)
				{
					cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
					return rc;
				}

				cpssOsTimerWkAfter(100);
				rc = extDrvDragoniteMsgRead(GetSinglePortStatus);
				if((rc != GT_OK) || (msg_request[1] != GetSinglePortStatus[1]))
				{
					cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
					cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], GetSinglePortStatus[1]);
					return rc;
				}

				cpssOsPrintf("%4d\t%5.2f\t%2d\t%5.2f\t%5d\t%9d(0x%02X)\t%4d\t%7d\t%9.3fW\t%8d\n", i,
					(GT_U16)( (GetPortMeasurements[9] << 8) | GetPortMeasurements[10] ) * 0.1,
					(GT_U16)( (GetPortMeasurements[4] << 8) | GetPortMeasurements[5] ),
					(GT_U16)( (GetPortMeasurements[6] << 8) | GetPortMeasurements[7] ) * 0.001,
					GetSinglePortStatus[6], (int)GetSinglePortStatus[3] == 1 ? 1 : 0,
					GetSinglePortStatus[3], GetSinglePortStatus[10], 
					(GT_U16)( GetSinglePortStatus[2] & 0x1 ), 
					(GT_U16)( (GetPortPowerLimit[2] << 8) | GetPortPowerLimit[3] ) * 0.001,
					GetPortPriority[2]);
			}
			cpssOsPrintf("\n");

			break;
		}

		case 'p':
		{
			int channel = 0, priority = 0, i = 0, count = 0;;
			char *temp;

			for (i = 0, count = 0; i < strlen(cmdPtr); i++)
			{
				if (cmdPtr[i] == 44)
					count++;
			}

			if (count != 2 || cmdPtr[strlen(cmdPtr)-1] == 44)
			{
				cpssOsPrintf("Wrong format\n");
				return rc;
			}

			strtok(cmdPtr, ",");
			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: channel\n");
					return rc;
				}
			}
			channel = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: priority\n");
					return rc;
				}
			}
			priority = atoi(temp);
		
			if ( (channel < 0 || channel >= ports) || (priority < 1 || priority >= 4) )
			{
				cpssOsPrintf("Wrong number\n");
				return rc;
			}

			/* 4.3.17 Set Power Limit for Channels */
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0x00, rand() % 200 + 10, 0x05, 0x0A, channel, priority,
									0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x0, 0x0};

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			break;
		}

		case 'l':
		{
			int channel = 0, power_limit = 0, i = 0, count = 0;
			char *temp;
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0};

			/* 4.4.1 Set PM Method */
			msg_request[0] = 0x00; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x07; msg_request[3] = 0x0B;
			msg_request[4] = 0x5F; msg_request[5] = 0x00; msg_request[6] = 0x00; msg_request[7] = 0x00;
			msg_request[8] = 0x4E; msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
			msg_request[12] = 0x4E; msg_request[13] = 0x00; msg_request[14] = 0x00;

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			/* Check argument format. Calculate number of "," and last char should not be ",". */
			for (i = 0, count = 0; i < strlen(cmdPtr); i++)
			{
				if (cmdPtr[i] == 44)
					count++;
			}

			if (count != 2 || cmdPtr[strlen(cmdPtr)-1] == 44)
			{
				cpssOsPrintf("Wrong format\n");
				return rc;
			}

			/* Parser argument */
			strtok(cmdPtr, ",");
			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: channel\n");
					return rc;
				}
			}
			channel = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: power limit\n");
					return rc;
				}
			}
			power_limit = atoi(temp);

			/* Check argument's range */
			if ( (channel < 0 || channel >= ports) )
			{
				cpssOsPrintf("Wrong argument\n");
				return rc;
                        }

			/* 4.3.9 Set Power Limit for Channels */
			msg_request[0] = 0x00; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05; 
			msg_request[3] = 0x0B; msg_request[4] = channel; 
			msg_request[5] = U32_GET_FIELD_MAC(power_limit, 8, 8);
			msg_request[6] = U32_GET_FIELD_MAC(power_limit, 0, 8); 
			msg_request[7] = 0x4E; msg_request[8] = 0x4E; msg_request[9] = 0x4E; 
			msg_request[10] = 0x4E; msg_request[11] = 0x4E; msg_request[12] = 0x4E;
			msg_request[13] = 0x00; msg_request[14] = 0x00;

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
                        rc = extDrvDragoniteMsgRead(msg_reply);
                        if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
                        {
                                cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
                                return rc;
                        }

			break;
		}

		case 'y':
                {
			int port =1,channel = 0, endis = 0, type = 1, i = 0, count = 0;
			char *temp;

			for (i = 0, count = 0; i < strlen(cmdPtr); i++)
			{
				if (cmdPtr[i] == 44)
					count++;
			}

			if (count != 3 || cmdPtr[strlen(cmdPtr)-1] == 44)
			{
				cpssOsPrintf("Wrong format\n");
				return rc;
			}

			strtok(cmdPtr, ",");
			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: channel\n");
					return rc;
				}
			}
			
			port = atoi(temp) ; 
			if(port <  1|| port > ports)
			{
				cpssOsPrintf("Error! Port number %d is out of range 1~%d \n",port,ports);
			}

			channel = port-1;

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: endis\n");
					return rc;
				}
			}
			endis = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: type\n");
					return rc;
				}
			}
			type = atoi(temp);

			if ( (channel < 0 || channel >= ports) || (endis < 0 || endis >= 2) || (type < 0 || type >= 3) )
			{
				cpssOsPrintf("Wrong argument\n");
				return rc;
                        }

			/* 4.3.5 Set Enable/Disable Channels */
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0x00, rand() % 200 + 10, 0x05, 0x0C, channel, endis, type,
									0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x0, 0x0};

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			if(endis)
				cpssOsPrintf("Port %d is enabled.\n",port);	
			else
				cpssOsPrintf("Port %d is disabled.\n",port);	
			
			break;
		}
		case 'b':
		{
			int bank = 0, power_limit = 0, max_shutdown_voltage = 0, min_shutdown_voltage = 0;
			int guard_band = 0, i = 0, count = 0;
			char *temp;

			/* Check argument format. Calculate number of "," and last char should not be ",". */
			for (i = 0, count = 0; i < strlen(cmdPtr); i++)
			{
				if (cmdPtr[i] == 44)
					count++;
			}

			if (count != 5 || cmdPtr[strlen(cmdPtr)-1] == 44)
			{
				cpssOsPrintf("Wrong format\n");
				return rc;
			}

			/* Parser argument */
			strtok(cmdPtr, ",");
			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: bank\n");
					return rc;
				}
			}
			bank = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: power limit\n");
					return rc;
				}
			}
			power_limit = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: max shutdown voltage\n");
					return rc;
				}
			}
			max_shutdown_voltage = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: max shutdown voltage\n");
					return rc;
				}
			}
			min_shutdown_voltage = atoi(temp);

			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: guard band\n");
					return rc;
				}
			}
			guard_band = atoi(temp);

			/* Check argument's range */
			if ( (bank < 0 || bank >= 16) || (power_limit < 0 || power_limit >= 6001) || 
				(max_shutdown_voltage < 0 || max_shutdown_voltage >= 586) || 
				(min_shutdown_voltage < 0 || min_shutdown_voltage >= 401) ||
				(guard_band < 0 || guard_band >= 256) )
			{
				cpssOsPrintf("Wrong argument\n");
				return rc;
			}

			/* 4.4.4 Set Power Banks */
                        GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0x00, rand() % 200 + 10, 0x07, 0x0B, 0x57, bank, 
									U32_GET_FIELD_MAC(power_limit, 8, 8),
									U32_GET_FIELD_MAC(power_limit, 0, 8),
									U32_GET_FIELD_MAC(max_shutdown_voltage, 8, 8),
									U32_GET_FIELD_MAC(max_shutdown_voltage, 0, 8),
									U32_GET_FIELD_MAC(min_shutdown_voltage, 8, 8),
									U32_GET_FIELD_MAC(min_shutdown_voltage, 0, 8),
									guard_band, 0x0, 0x0};

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);

			if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			break;
		}
		case 'g':
		{
			int port,channel;
			int i=0;
			char *temp;
			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0};	
			
			strtok(cmdPtr, ",");
			temp = strtok(NULL, ",");
			for (i = 0; i < strlen(temp); i++)
			{
				if (!isdigit(temp[i]))
				{
					cpssOsPrintf("Wrong number: port\n");
					return rc;
				}
			}

			port = atoi(temp);
			
			if(port < 1 || port > ports)
			{
				cpssOsPrintf("Error! Port number %d is out of range 1~%d \n",port,ports);
			}

			channel = port-1;
			
			/* 4.4.11 Get Port Measurements */
			msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
			msg_request[3] = 0x25; msg_request[4] = channel; msg_request[5] = 0x4E;
			msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
			msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
			msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

			/* Fill byte[14] and byte[15] in 15B protocol with checksum */
			poeMessageCheckSumCalc(msg_request, 13, &checkSum);

			rc = extDrvDragoniteMsgWrite(msg_request);
			if(rc != GT_OK)
			{
				cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
				return rc;
			}

			cpssOsTimerWkAfter(100);
			rc = extDrvDragoniteMsgRead(msg_reply);
			if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
			{
				cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
				cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
				return rc;
			}

			cpssOsPrintf("Port%d= %3.1f W\n", port,
                                                            (GT_U16)( (msg_reply[6] << 8) | msg_reply[7] ) * 0.001);
			break;
		}	
		case 'm':
		{
			int i = 0;

			GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0};

			switch(cmdPtr[1])
			{
				case 'p':
				{
					for (i = 0; i < ports; i++)
					{
						/* 4.4.11 Get Port Measurements */
						msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
						msg_request[3] = 0x25; msg_request[4] = i; msg_request[5] = 0x4E;
						msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
						msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
						msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

						/* Fill byte[14] and byte[15] in 15B protocol with checksum */
						poeMessageCheckSumCalc(msg_request, 13, &checkSum);

						rc = extDrvDragoniteMsgWrite(msg_request);
						if(rc != GT_OK)
						{
							cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
							return rc;
						}

						cpssOsTimerWkAfter(100);
						rc = extDrvDragoniteMsgRead(msg_reply);
						if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
						{
							cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
							cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
							return rc;
						}

						cpssOsPrintf("Port%d= %3.1f W  ", i+1,
                                                                (GT_U16)( (msg_reply[6] << 8) | msg_reply[7] ) * 0.001);

						if(((i + 1) % 6) == 0)
							cpssOsPrintf("\n");
					}

					cpssOsPrintf("\n");
					break;
				}

				case 't':
				{
					for (i = 0; i < (ports / 8); i++)
					{
						/* 4.2.2 Get PoE Device Status */
						msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x07;
						msg_request[3] = 0x87; msg_request[4] = i; msg_request[5] = 0x00;
						msg_request[6] = 0x00; msg_request[7] = 0x00; msg_request[8] = 0x00;
						msg_request[9] = 0x00; msg_request[10] = 0x00; msg_request[11] = 0x00;
						msg_request[12] = 0x00; msg_request[13] = 0x0; msg_request[14] = 0x0;

						/* Fill byte[14] and byte[15] in 15B protocol with checksum */
						poeMessageCheckSumCalc(msg_request, 13, &checkSum);

						rc = extDrvDragoniteMsgWrite(msg_request);
						if(rc != GT_OK)
						{
							cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
							return rc;
						}

						cpssOsTimerWkAfter(100);
						rc = extDrvDragoniteMsgRead(msg_reply);
						if(rc != GT_OK || msg_request[1] != msg_reply[1])
						{
							cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
							cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
							return rc;
						}

						cpssOsPrintf("Device%d= %d C  ", i,
								(int)msg_reply[9] > 205 ? 256 - (int)msg_reply[9] : msg_reply[9]);
					}

					cpssOsPrintf("\n");
					break;
				}

				case 'c':
				{
					for (i = 0; i < ports; i++)
					{
						/* 4.3.21 Get Single Port Status */
						msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
						msg_request[3] = 0x0E; msg_request[4] = i; msg_request[5] = 0x4E;
						msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
						msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
						msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

						/* Fill byte[14] and byte[15] in 15B protocol with checksum */
						poeMessageCheckSumCalc(msg_request, 13, &checkSum);

						rc = extDrvDragoniteMsgWrite(msg_request);
						if(rc != GT_OK)
						{
							cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
							return rc;
						}

						cpssOsTimerWkAfter(100);
						rc = extDrvDragoniteMsgRead(msg_reply);
						if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
						{
							cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
							cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
							return rc;
						}

						cpssOsPrintf("Class%d= %d  ", i, msg_reply[6]);	

						if(((i + 1) % 6) == 0)
							cpssOsPrintf("\n");

					}

					cpssOsPrintf("\n");
					break;
				}

				default:
					cpssOsPrintf("No valid option to do with %s \n", __func__);
			}
			break;
		}

		default:
		{
			/* cpssOsPrintf("No valid option to do with %s \n", __func__); */
			cpssOsPrintf("\nf[,channel]\t\t- Show information for all port or specify port.\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"f\", poeTest \"f,8\"\n");
			cpssOsPrintf("e\t\t\t- Enable all ports.\n");
			cpssOsPrintf("d\t\t\t- disable all ports.\n");
			cpssOsPrintf("v\t\t\t- Show firmware version.\n");
			cpssOsPrintf("i\t\t\t- Initial PoE chip.\n");
			cpssOsPrintf("p,channel,priority\t- Set port's priority.\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"p,3,1\"\n");
			cpssOsPrintf("l,channel,powerlimit\t- Set port's power limit.\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"l,2,10000\"\n");
			cpssOsPrintf("y,port,endis,type\t- Set port's enable/disable and type of IEEE802.3\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"y,2,1,1\"\n");
			cpssOsPrintf("g,port\t\t- Show loading of each port.\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"g,2\"\n");
			cpssOsPrintf("b,bank,bank_power_limit,max_shutdown_voltage,min_shutdown_voltage,guard_band\n\t\t\t- Set power bank.\n");
			cpssOsPrintf("\t\t\t\tExample: poeTest \"b,15,6000,585,400,1\"\n");
			cpssOsPrintf("Parameter:\n");
			cpssOsPrintf("\tchannel: decimal, 0-%d\n", ports);
			cpssOsPrintf("\tpriority: decimal, 1-3\n");
			cpssOsPrintf("\tpowerlimit: decimal (mW)\n");
			cpssOsPrintf("\tendis: 1: enable; 0: disable\n");
			cpssOsPrintf("\ttype: 0: AF; 1: AF/AT; 2: POH\n");
			cpssOsPrintf("\tbank: decimal, 0-15. Default value is 15.\n");
			cpssOsPrintf("\tbank_power_limit: decimal, 0-6000 (W). Default value is 1612.\n");
			cpssOsPrintf("\tmax_shutdown_voltage: decimal, 0-585 (0.1v).\n");
			cpssOsPrintf("\tmin_shutdown_voltage: decimal, 0-400 (0.1v)\n");
			cpssOsPrintf("\tguard_band: decimal, 0-255. Default value is 1.\n\n");
		}
	}

/* fallthrough in case ok */

fail_label:
    if(buf != NULL)
    {
        cpssOsFree(buf);
    }

    close(rfd);

	return GT_OK;
}


GT_STATUS poeGetPortLoading(
	IN	GT_U8 port,
   	OUT GT_U16 *portLoadingPtr
)
{
	GT_STATUS rc=GT_OK;
	GT_U16  checkSum = 0;
	GT_U8 portNum;
	GT_U16  portLoading = 0;
	GT_U8 msg_request[DRAGONITE_DATA_MSG_LEN] = {0};
	GT_U8 msg_reply[DRAGONITE_DATA_MSG_LEN] = {0};
	
	  if (appDemoPpConfigDevAmount == 1)
	  {
		if (port<1 || port >24)
		{
			cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
			return GT_FAIL;
		}
	  }
    	else if (appDemoPpConfigDevAmount == 2)
          {
		if (port<1 || port>48)
		{
			return GT_FAIL;
		}
	  }

	portNum = port -1;
				
	/* 4.4.11 Get Port Measurements */
	msg_request[0] = 0x02; msg_request[1] = rand() % 200 + 10; msg_request[2] = 0x05;
	msg_request[3] = 0x25; msg_request[4] = portNum; msg_request[5] = 0x4E;
	msg_request[6] = 0x4E; msg_request[7] = 0x4E; msg_request[8] = 0x4E;
	msg_request[9] = 0x4E; msg_request[10] = 0x4E; msg_request[11] = 0x4E;
	msg_request[12] = 0x4E; msg_request[13] = 0x0; msg_request[14] = 0x0;

	/* Fill byte[14] and byte[15] in 15B protocol with checksum */
	poeMessageCheckSumCalc(msg_request, 13, &checkSum);

	rc = extDrvDragoniteMsgWrite(msg_request);
	if(rc != GT_OK)
	{
		cpssOsPrintf("extDrvDragoniteMsgWrite: MSG send was failed\r\n");
		return rc;
	}

	cpssOsTimerWkAfter(100);
	rc = extDrvDragoniteMsgRead(msg_reply);
	if((rc != GT_OK) || (msg_request[1] != msg_reply[1]))
	{
		cpssOsPrintf("extDrvDragoniteMsgRead: MSG receive was failed\r\n");
		cpssOsPrintf("Sent ECHO: %02X, Recv ECHO: %02X\n", msg_request[1], msg_reply[1]);
		return rc;
	}

	/*cpssOsPrintf("Port%d= %3.1f W  ", port, (GT_U16)( (msg_reply[6] << 8) | msg_reply[7] ) * 0.001);*/
	portLoading =  (msg_reply[6] << 8) | msg_reply[7]  ;
	*portLoadingPtr = portLoading;
	/*cpssOsPrintf("Bernie Port%d= %d W  ", port,portLoading);*/

	return rc;
			
}


#endif



