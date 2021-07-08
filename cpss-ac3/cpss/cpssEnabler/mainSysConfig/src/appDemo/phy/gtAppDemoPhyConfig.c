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
* @file gtAppDemoPhyConfig.c
*
* @brief Generic support for PHY init.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#endif /*CHX_FAMILY*/

#ifndef ASIC_SIMULATION

#include <88x3240/mtdApiTypes.h>
#include <88x3240/mtdHwCntl.h>
#include <88x3240/mtdAPI.h>
#include <88x3240/mtdFwDownload.h>
#include <88x3240/mtdHunit.h>
#include <88x3240/mtdCunit.h>

/* for Lewis PHY driver */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#endif

#define PHY_ID_REG_DEV_NUM_CNS                1
#define PHY_VENDOR_ID_REG_ADDR_CNS            2
#define PHY_ID_REG_ADDR_CNS                   3

/* Marvell PHYs unique vendor ID - register 2 */
#define MRVL_PHY_REV_A_UID_CNS 0x2B
#define MRVL_PHY_UID_CNS 0x141
#define MRVL_PHY_3240_MODEL_NUM_CNS 0x18
#define MRVL_PHY_3340_MODEL_NUM_CNS 0x1A
#define MRVL_PHY_2010_MODEL_NUM_CNS 0x1A
#define MRVL_PHY_2180_MODEL_NUM_CNS 0x1B

#define PHY_TIMEOUT        10000

/* WNC SKU ID, need to sync with the definition in gtRd_xCat_24GE_4HGS.c */
#define AC3BXH_WNC_OS6360P48 7
#define AC3BXH_WNC_OS6360P24 8

GT_U16 vendorId;
GT_BOOL checkEnable = GT_TRUE;
GT_U16 phyModelNum;

extern GT_STATUS appDemoBoardTypeGet
(
    OUT GT_U32 *boardTypePtr
);

/* Add to get WNC SKU ID */
extern GT_STATUS xcat3BoardTypeGet
(
    OUT GT_U32 *xcat3BoardTypePtr
);

typedef struct GT_APPDEMO_XPHY_INFO_STCT
{
    GT_U32                      portGroupId;
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;
    GT_U16                      phyAddr;
    GT_U32                      phyType;
} GT_APPDEMO_XPHY_INFO_STC;

typedef enum
{
    GT_APPDEMO_PHY_RAM_E = 0,
    GT_APPDEMO_PHY_FLASH_E
}GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT;

GT_STATUS gtAppDemoXPhyVendorIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT  GT_U16                    *vendorIdPtr
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
    saved_system_recovery = system_recovery;

    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* read vendor ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_VENDOR_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS,
                                        /*OUT*/vendorIdPtr);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssXsmiPortGroupRegisterRead", rc);
    }
    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

GT_STATUS gtAppDemoXPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;
    /*GT_U16    phyRev = 0;  future use for 2180 phy*/

    /* read PHY ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS, &data);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssSmiRegisterReadShort", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {

        /*phyRev = data & 0xF;*/

        *phyIdPtr = (data >> 4) & 0x3F;
        phyModelNum = *phyIdPtr;
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}


GT_VOID gtAppDemoXSmiScan
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U32      currBoardType;
    GT_U8       maxXSMIinterfaces=0;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("fail to get the BoardType=%d\n", currBoardType);
    }
    else
    {
        /* enable to load PHY_XSMI_INTERFACE_3 */
        if(currBoardType == APP_DEMO_ALDRIN2_BOARD_RD_CNS)
        {
            cpssDrvHwPpResetAndInitControllerWriteReg(0, 0xf8d24, 0x18200ce);
            maxXSMIinterfaces = CPSS_PHY_XSMI_INTERFACE_3_E;
        }
        else
        {
            maxXSMIinterfaces = CPSS_PHY_XSMI_INTERFACE_1_E;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E; xsmiInterface <= maxXSMIinterfaces; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = gtAppDemoXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                else
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,VendorId=0x%x\n",
                        devNum, portGroupId, xsmiInterface, xsmiAddr, data);
                }

                rc = gtAppDemoXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssSmiRegisterReadShort FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                else
                {
                    cpssOsPrintf("cpssSmiRegisterReadShort:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,PhyId=0x%x\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr, data);
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}


GT_VOID gtAppDemoXSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT GT_APPDEMO_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U32      currBoardType;
    GT_U8       maxXSMIinterfaces=0;
    GT_U32      xcat3BoardType;

    *phyInfoArrayLenPtr = 0;

    /* get WNC SKU ID */
    xcat3BoardTypeGet(&xcat3BoardType);

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("fail to get the BoardType=%d\n", currBoardType);
    }
    else
    {
        /* enable to load PHY_XSMI_INTERFACE_3 */
        if(currBoardType == APP_DEMO_ALDRIN2_BOARD_RD_CNS)
        {
            cpssDrvHwPpResetAndInitControllerWriteReg(0, 0xf8d24, 0x18200ce);
            maxXSMIinterfaces = CPSS_PHY_XSMI_INTERFACE_3_E;
        }
        else
        {
            maxXSMIinterfaces = CPSS_PHY_XSMI_INTERFACE_1_E;

            if(xcat3BoardType == AC3BXH_WNC_OS6360P48 || xcat3BoardType == AC3BXH_WNC_OS6360P24)
            {
                maxXSMIinterfaces = CPSS_PHY_XSMI_INTERFACE_0_E;
            }
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E; xsmiInterface <= maxXSMIinterfaces; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = gtAppDemoXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                if ((data != MRVL_PHY_UID_CNS) && (data != MRVL_PHY_REV_A_UID_CNS))
                {
                    continue;
                }
                else if ((data == MRVL_PHY_UID_CNS || data == MRVL_PHY_REV_A_UID_CNS) &&
                         checkEnable == GT_TRUE)
                {
                    vendorId = data;
                    checkEnable = GT_FALSE;
                }
                rc = gtAppDemoXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
                phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)xsmiAddr;
                phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = xsmiInterface;
                phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
                cpssOsPrintf("%d)portGroupId=%d,phyAddr=0x%x,xsmiInterface=%d,phyType=0x%x\n",
                             *phyInfoArrayLenPtr,
                             phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                             phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                             phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                             phyInfoArray[*phyInfoArrayLenPtr].phyType);
                (*phyInfoArrayLenPtr)++;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

#ifndef ASIC_SIMULATION

GT_STATUS gtAppDemoXPhyFwDownloadSingle
(
    GT_U32  devNum,
    GT_U32  portGroup,
        CPSS_PHY_XSMI_INTERFACE_ENT  xsmiInterface,
    GT_U32  phyType,
    GT_U16  startPhyAddr,
    GT_U16  endPhyAddr,
    GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT mode
)
{
    CPSS_OS_FILE    fd = 0, fd_slave = 0;
    GT_U8           *buffer = NULL, *buffer_slave = NULL;
    GT_U32          nmemb, nmemb_slave;
    MTD_BOOL        isPhyRead;
    GT_U32          timeout;
    int             ret;
    GT_U16          status;
    MTD_BOOL        tunitReady;
    GT_U32          xcat3BoardType;

    GT_U16 phyAddr[32] = {  0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0 };

    GT_U16 errorPorts[32] = {   0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0 };

    GT_PHYSICAL_PORT_NUM numPort = 0;
    GT_U32 i;
    GT_U8 major, minor, inc, test;
    PRESTERA_INFO switchInfo;

    char file_name[64];
    char file_name_slave[64];

    /* get WNC SKU ID */
    xcat3BoardTypeGet(&xcat3BoardType);

    /* !!! Pay attention - if you change names of files here you must change
        them in Makefile too for standalone version build !!! */
    if(MRVL_PHY_3240_MODEL_NUM_CNS == phyType)
    {/* important - see comment about file name above */
        cpssOsSprintf(file_name, "%s", "88X32xx-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "3240_0400_6270.bin");
    }
    else if(MRVL_PHY_3340_MODEL_NUM_CNS == phyType && MRVL_PHY_UID_CNS == vendorId)
    {/* important - see comment about file name above */
        cpssOsSprintf(file_name, "%s", "88X33xx-Z2-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "flashdownloadslave.bin");
    }
    else if (MRVL_PHY_2010_MODEL_NUM_CNS == phyType && xcat3BoardType == AC3BXH_WNC_OS6360P48)
    {
        /* 88E2010, note that vendorId of 88E2010 is also MRVL_PHY_REV_A_UID_CNS, using SKU id to distinguish. */
        cpssOsSprintf(file_name, "%s", "e2010fw.hdr");
    }
    else if (MRVL_PHY_3340_MODEL_NUM_CNS == phyType && MRVL_PHY_REV_A_UID_CNS == vendorId)
    {
        cpssOsSprintf(file_name, "%s", "x3310fw_0_2_8_0_8850.hdr");
    }

    else if (MRVL_PHY_2180_MODEL_NUM_CNS == phyType && MRVL_PHY_REV_A_UID_CNS == vendorId)
    {
        cpssOsSprintf(file_name, "%s", "e21x0fw_8_0_0_36_8923.hdr");
    }
    else
    {
        cpssOsPrintf("not supported PHY type\n");
        return GT_NOT_SUPPORTED;
    }

    /* create phy address array */
    if (GT_APPDEMO_PHY_RAM_E == mode)
    {
        numPort = endPhyAddr - startPhyAddr + 1;
        for (i = 0; i < numPort; i++)
        {
            phyAddr[i] = (i + startPhyAddr);
        }
    }
    else /* for SPI flash, only first address of each PHY */
    {
        /* This option not functional on Lewis RD board with PHY 3240.
            Get: "Expected 0x0100 from slave got 0x0400 on port" for
            every port we tried
         */
        numPort = (endPhyAddr - startPhyAddr + 1) / 4;
        for (i = 0; i < numPort; i++)
        {
            phyAddr[i] = (i * 4) + startPhyAddr;
            if(3240 == phyType)
            {
                phyAddr[i] += 2;
            }
        }
    }

    if (numPort >= 32)
    {
        cpssOsPrintf("numPort[%d]>=32\n", numPort);
        return GT_BAD_PARAM;
    }

    if (0 == numPort)
    {
        cpssOsPrintf("0 == numPort\n");
        return GT_BAD_PARAM;
    }

    cpssOsPrintf("Load firmware file %s\n", file_name);

    fd = cpssOsFopen(file_name, "r");
    if (0 == fd)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    nmemb = cpssOsFgetLength(fd);
    if (nmemb <= 0)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    /* for store image */
    buffer = cpssOsMalloc(216 * 1024);
    if (NULL == buffer)
    {
        cpssOsPrintf(" buffer allocation fail \n");
        return GT_ERROR;
    }

    ret = cpssOsFread(buffer, 1, nmemb, fd);
    if (ret < 0)
    {
        cpssOsPrintf("Read from file fail\n");
        return GT_ERROR;
    }

    switchInfo.devNum = devNum;
    switchInfo.portGroup = portGroup;
    switchInfo.xsmiInterface = xsmiInterface;

    if (GT_APPDEMO_PHY_FLASH_E == mode)   /* load slave .bin for SPI download */
    {
        cpssOsPrintf("Load slave bin file %s\n", file_name_slave);
        fd_slave = cpssOsFopen(file_name_slave, "r");
        if (0 == fd_slave)
        {
            cpssOsPrintf("open %s fail \n", file_name_slave);
            return GT_ERROR;
        }

        nmemb_slave = cpssOsFgetLength(fd_slave);
        if (nmemb_slave <= 0)
        {
            cpssOsPrintf("slave bin length fail \n");
            return GT_ERROR;
        }

        /* 20KB for store slave image */
        buffer_slave = cpssOsMalloc(20 * 1024);
        if (NULL == buffer_slave)
        {
            cpssOsPrintf(" slave buffer allocation fail \n");
            return GT_ERROR;
        }

        ret = cpssOsFread(buffer_slave, 1, nmemb_slave, fd_slave);
        if (ret < 0)
        {
            cpssOsPrintf("Read from slave bin file fail\n");
            return GT_ERROR;
        }

        cpssOsPrintf("MTL downloading firmware to SPI flash phyAddr=%x,numPort=%d ...", phyAddr[0], numPort);
        (GT_VOID)mtdParallelUpdateFlashImage(&switchInfo, phyAddr, buffer, nmemb,
                                                buffer_slave, nmemb_slave,
                                                (GT_U16)numPort, errorPorts,
                                                &status);
    }
    else            /* RAM */
    {
        cpssOsPrintf("MTL downloading firmware to ram phyAddr=%x,numPort=%d ...", phyAddr[0], numPort);
        (GT_VOID)mtdParallelUpdateRamImage(&switchInfo, phyAddr, buffer, nmemb,
                                            (GT_U16)numPort, errorPorts,
                                            &status);
    }

    status &= 0xFFFF;
    if (status == 0)
    {
        cpssOsPrintf("OK");
        (GT_VOID)mtdGetFirmwareVersion(&switchInfo, phyAddr[0], &major, &minor, &inc, &test);
        cpssOsPrintf(" - firmware ver = %u.%u.%u.%u\n", major, minor, inc, test);

        timeout = 0;
        while (timeout <= PHY_TIMEOUT)
        {
            if (timeout == PHY_TIMEOUT)
            {
                cpssOsPrintf("phy app is not ready before timeout \n ");
                return GT_TIMEOUT;
            }
            /* check phy code status, ready = MTD_TRUE */
            mtdDidPhyAppCodeStart(&switchInfo, phyAddr[0], &isPhyRead);
            if (isPhyRead == MTD_TRUE)
                    break;
            timeout++;
            cpssOsTimerWkAfter(1);
        }
    }
    else
    {
        cpssOsPrintf("FAIL , status = 0x%x\n", status);
        cpssOsPrintf("MTL phy status = ");
        for (i = 0; i < numPort; i++)
            cpssOsPrintf("%4x, ", errorPorts[i]);
        cpssOsPrintf("\n");
        return GT_ERROR;
    }

    cpssOsFree(buffer);
    cpssOsFclose(fd);

    if (GT_APPDEMO_PHY_FLASH_E == mode)
    {
        cpssOsFree(buffer_slave);
        cpssOsFclose(fd_slave);
    }

    if (MRVL_PHY_2010_MODEL_NUM_CNS == phyType && xcat3BoardType == AC3BXH_WNC_OS6360P48)
    {
        /* PHY 88E2010, default power down */
        
        /* To set speed to 2.5G and release power off */
        ret = cpssXsmiPortGroupRegisterWrite(devNum, 0, xsmiInterface,  phyAddr[0],  0x0000, 1,  0xa058);
        if (ret != GT_OK)
        {
            cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(%d, 0, %u,  %u,  0x0000, 1,  0xa058):ret=%d\n",
            devNum, xsmiInterface, phyAddr[i], ret);
            return ret;
        }
        /* Software reset the T unit */
        ret = cpssXsmiPortGroupRegisterWrite(devNum, 0, xsmiInterface,  phyAddr[0],  0x0000, 3,  0x8000);
        if (ret != GT_OK)
        {
            cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(%d, 0, %u,  %u,  0x0000, 3,  0x8000):ret=%d\n",
            devNum, xsmiInterface, phyAddr[i], ret);
            return ret;
        }
        /* Advertise PHY 2.5G capable */ 
        ret = cpssXsmiPortGroupRegisterWrite(devNum, 0, xsmiInterface,  phyAddr[0],  0x0020, 7,  0x0081);
        if (ret != GT_OK)
        {
            cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(%d, 0, %u,  %u,  0x0020, 7,  0x0081):ret=%d\n",
            devNum, xsmiInterface, phyAddr[i], ret);
            return ret;
        }
        /* Restart AN */
        ret = cpssXsmiPortGroupRegisterWrite(devNum, 0, xsmiInterface,  phyAddr[0],  0x0000, 7,  0x3200);
        if (ret != GT_OK)
        {
            cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(%d, 0, %u,  %u,  0x0000, 7,  0x3200):ret=%d\n",
            devNum, xsmiInterface, phyAddr[i], ret);
            return ret;
        }
        
        return GT_OK;
    }

        for(i=0;i < numPort;i++)
        {
                /*
                        set Host side XFI / SGMII-AN
                        active lane 0
                        MAC side always power up
                        do software reset after config
                */
                ret = mtdSetMacInterfaceControl(&switchInfo,
                                                                                        phyAddr[i],
                                                                                MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                                                                MTD_TRUE, /* TRUE = mac interface always power up */
                                                                                MTD_MAC_SNOOP_OFF,
                                                                                0,
                                                                                MTD_MAC_SPEED_10_MBPS,
                                                                                MTD_TRUE);
                if (ret != GT_OK)
                {
                                cpssOsPrintf("mtdSetMacInterfaceControl port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /*
                                set Line side AMD, prefer copper
                                F2R mode = b'00  (OFF)
                                no energy detect
                                no max power AMD
                                do software reset after config
                */
                ret = mtdSetCunitTopConfig(&switchInfo,
                                        phyAddr[i],
                                                                        MTD_F2R_OFF,
                                                                        MTD_MS_AUTO_PREFER_FBR,
                                                                        MTD_FT_10GBASER,
                                                                        MTD_FALSE,
                                                                        MTD_FALSE,
                                                                        MTD_TRUE);
                if (ret != GT_OK)
                {
                                cpssOsPrintf("mtdSetCunitTopConfig port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /* LED control */
                        ret = cpssXsmiPortGroupRegisterWrite(0, 0, xsmiInterface,  phyAddr[i],  0xf022, 31,  0x29);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(0, 0, %u,  %u,  0xf022, 31,  0x29):ret=%d\n",
                                                xsmiInterface, phyAddr[i], ret);
                        return ret;
                }
                        ret = cpssXsmiPortGroupRegisterWrite(0, 0, xsmiInterface,  phyAddr[i],  0xf023, 31,  0x31);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(0, 0, %u,  %u,  0xf023, 31,  0x31):ret=%d\n",
                                                xsmiInterface, phyAddr[i], ret);
                        return ret;
                }

                /* power up phy */
                        if ((ret = mtdRemovePhyLowPowerMode(&switchInfo, phyAddr[i])) != GT_OK)
                {
                                cpssOsPrintf("mtdRemovePhyLowPowerMode of port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /* release note 4.6  do T-unit reset, (1.0x0.15) */
                for (timeout = 0; timeout < PHY_TIMEOUT; timeout++)
                {
                        ret = mtdIsTunitResponsive(&switchInfo, phyAddr[i], &tunitReady);
                        if (tunitReady == MTD_TRUE)
                                        break;

                        cpssOsTimerWkAfter(1);
                }
                /* after flash burn on 3240 PHY timeout happens, but FW is burned and functional after HW reset */
                if (timeout == PHY_TIMEOUT)
                {
                                cpssOsPrintf("mtdIsTunitResponsive timeout:phyAddr=0x%x\n", phyAddr[i]);
                        return GT_TIMEOUT;
                }

                ret = mtdSoftwareReset(&switchInfo, phyAddr[i],100);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("mtdSoftwareReset port %u fail\n", phyAddr[i]);
                        return ret;
                }
        }
    return GT_OK;
}



GT_STATUS gtAppDemoXPhyFwDownload
(
    IN  GT_U8   devNum
)
{
    GT_STATUS                     rc;
    GT_APPDEMO_XPHY_INFO_STC    phyInfoArray[CPSS_MAX_PORTS_NUM_CNS];
    GT_APPDEMO_XPHY_INFO_STC    phyInfo3340[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                      i;
    GT_U32                      phyInfoArrayLen = 0;
    GT_U32                      phyInfo3340Len = 0;
    GT_U32                      xSmiInterface = 0;
    GT_U32                      startAddr, endAddr;
    GT_U32                      initPhy;
    GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT phyFwLoadMode;
    GT_U32                      xcat3BoardType;
    GT_U32                      got3220Phy = 0;

    /* get WNC SKU ID */
    xcat3BoardTypeGet(&xcat3BoardType);

    rc = appDemoDbEntryGet("initPhy", &initPhy);
    if (rc != GT_OK)
    {
        initPhy = 1;
    }

    if (!initPhy)
    {
        return GT_OK;
    }

    rc = appDemoDbEntryGet("phyFwLoadMode", &phyFwLoadMode);
    if (rc != GT_OK)
    {
        phyFwLoadMode = GT_APPDEMO_PHY_RAM_E;
    }
    if (GT_APPDEMO_PHY_FLASH_E == phyFwLoadMode)
    {
        return GT_NOT_IMPLEMENTED;
    }
    gtAppDemoXSmiPhyAddrArrayBuild(devNum, phyInfoArray, &phyInfoArrayLen);

    cpssOsPrintf("Allen : phyInfoArrayLen=%d \n",phyInfoArrayLen);
    for (i = 0; i < phyInfoArrayLen; i++)
    {
        if(MRVL_PHY_3240_MODEL_NUM_CNS == phyInfoArray[i].phyType)
        {
            if((xcat3BoardType == AC3BXH_WNC_OS6360P24) || (xcat3BoardType == AC3BXH_WNC_OS6360P48))
            {
                /* to do 88X3220 FW download later */
                got3220Phy = 1;
                continue;
            }

            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0 /* portGroup */, phyInfoArray[i].xsmiInterface,
                                                MRVL_PHY_3240_MODEL_NUM_CNS,
                                                phyInfoArray[i].phyAddr,
                                                phyInfoArray[i].phyAddr,
                                                phyFwLoadMode);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
        else if((MRVL_PHY_2010_MODEL_NUM_CNS == phyInfoArray[i].phyType) && 
                (xcat3BoardType == AC3BXH_WNC_OS6360P48))
        {
            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0 /* portGroup */, phyInfoArray[i].xsmiInterface,
                                                MRVL_PHY_2010_MODEL_NUM_CNS,
                                                phyInfoArray[i].phyAddr,
                                                phyInfoArray[i].phyAddr,
                                                phyFwLoadMode);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
        /* phy 88E2180 is a sub family of 3340*/
        else if((MRVL_PHY_3340_MODEL_NUM_CNS == phyInfoArray[i].phyType) ||
                (MRVL_PHY_2180_MODEL_NUM_CNS == phyInfoArray[i].phyType))
        {
            osMemCpy(&(phyInfo3340[phyInfo3340Len]), &(phyInfoArray[i]), sizeof(GT_APPDEMO_XPHY_INFO_STC));
            phyInfo3340Len++;
        }
        else
        {
            return GT_NOT_SUPPORTED;
        }
    }

    if (got3220Phy)
    {
        /* refer to patch from Allen */
        if (((devNum == 0) && (xcat3BoardType == AC3BXH_WNC_OS6360P24)) ||
            ((devNum == 1) && (xcat3BoardType == AC3BXH_WNC_OS6360P48)))
        {
            /* reassign array */
            phyInfoArrayLen = 2; 
            phyInfoArray[0].portGroupId = 0;
            phyInfoArray[0].xsmiInterface = 0;
            phyInfoArray[0].phyAddr = 0xC;
            phyInfoArray[0].phyType = MRVL_PHY_3240_MODEL_NUM_CNS;
            phyInfoArray[1].portGroupId = 0;
            phyInfoArray[1].xsmiInterface = 0;
            phyInfoArray[1].phyAddr = 0xD;
            phyInfoArray[1].phyType = MRVL_PHY_3240_MODEL_NUM_CNS; 

            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0, phyInfoArray[0].xsmiInterface,
                                               phyInfoArray[0].phyType,
                                               phyInfoArray[0].phyAddr,
                                               phyInfoArray[1].phyAddr,
                                               GT_APPDEMO_PHY_RAM_E);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
    }

    if (phyInfo3340Len != 0) {
        for (i=0; i<phyInfo3340Len;) {
            xSmiInterface = phyInfo3340[i].xsmiInterface;
            startAddr = 0;
            endAddr = 0;
            while ((phyInfo3340[i].xsmiInterface == xSmiInterface) && (i<phyInfo3340Len)) {
                if (startAddr>=phyInfo3340[i].phyAddr)
                {
                    startAddr = phyInfo3340[i].phyAddr;
                }
                if (endAddr<=phyInfo3340[i].phyAddr)
                {
                    endAddr = phyInfo3340[i].phyAddr;
                }
                i++;
            }
            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0 /* portGroup */, xSmiInterface,
                                                phyModelNum,
                                                startAddr,
                                                endAddr,
                                                phyFwLoadMode);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

#endif





