#include <Copyright.h>

/**
********************************************************************************
* @file gtSysConfig.c
*
* @brief API definitions for system configuration, and enabling.
*
*/
/********************************************************************************
* gtSysConfig.c
*
* DESCRIPTION:
*       API definitions for system configuration, and enabling.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <gtSysConfig.h>
#include <gtDrvSwRegs.h>
#include <gtSem.h>
#include <gtUtils.h>
#include <msApiInternal.h>
#include <gtHwAccess.h>

static GT_BOOL gtRegister(GT_CPSS_QD_DEV *qd_dev, PRV_CPSS_BSP_FUNCTIONS* pBSPFunctions);

/**
* @internal prvCpssDrvQdLoadDriver function
* @endinternal
*
* @brief   QuarterDeck Driver Initialization Routine.
*         This is the first routine that needs be called by system software.
*         It takes cfg from system software, and retures a pointer (dev)
*         to a data structure which includes infomation related to this QuarterDeck
*         device. This pointer (dev) is then used for all the API functions.
* @param[in] cfg                      - Holds device configuration parameters provided by system software.
*
* @param[out] dev                      - Holds device information to be used for each API call.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - if device already started
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note prvCpssDrvQdUnloadDriver is also provided to do driver cleanup.
*
*/
GT_STATUS prvCpssDrvQdLoadDriver
(
    IN  GT_CPSS_SYS_CONFIG  *cfg,
    OUT GT_CPSS_QD_DEV      *dev
)
{
    GT_U16      value;

    PRV_CPSS_DBG_INFO(("prvCpssDrvQdLoadDriver Called.\n"));

    /* Check for parameters validity        */
    if(dev == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check for parameters validity        */
    if(cfg == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* The initialization was already done. */
    if(dev->devEnabled)
    {
        PRV_CPSS_DBG_INFO(("QuarterDeck already started.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }


    if(gtRegister(dev,&(cfg->BSPFunctions)) != GT_TRUE)
    {
       PRV_CPSS_DBG_INFO(("gtRegister Failed.\n"));
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Assign switch device address, 0 default in single chip mode,non-zero in multi-chip mode*/
    dev->phyAddr = (GT_U8)cfg->baseAddr;

    /* Init switch port base address*/
    dev->baseRegAddr = 0x0;

    /* Initialize the MultiAddress Register Access semaphore.    */
    if((dev->multiAddrSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    /* Init the device's config struct.             */
    if((prvCpssDrvHwGetAnyReg(dev,dev->baseRegAddr,PRV_CPSS_QD_REG_SWITCH_ID,&value)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get Device ID Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    dev->deviceId       = value >> 4;
    dev->revision       = (GT_U8)value & 0xF;


    /* Initialize dev fields.         */
    dev->cpuPortNum = cfg->cpuPortNum;

    /* Assign Device Name */
    dev->devName = 0;
    dev->devName1 = 0;

    /*dev->validSerdesVec = 0;*/

    switch(dev->deviceId)
    {
        case GT_CPSS_88E6390X:
        case GT_CPSS_88E6390:
        case GT_CPSS_88E6190X:
        case GT_CPSS_88E6190:
        case GT_CPSS_88E6190T:
                dev->numOfPorts = 11;
                dev->maxPorts = 11;
                dev->maxPhyNum = 8;
                dev->validPortVec = (1 << dev->numOfPorts) - 1;
                /*dev->validPhyVec = (1 << dev->maxPhyNum) - 1;*/
                dev->validPhyVec = 0x1FE;/*Internal GPHY Vector List*/
                dev->devName = 0;
                break;
        default:
                PRV_CPSS_DBG_INFO(("Unknown Device. Initialization failed\n"));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the ATU semaphore.    */
    if((dev->atuRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the VTU semaphore.    */
    if((dev->vtuRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the STATS semaphore.    */
    if((dev->statsRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the PIRL semaphore.    */
    if((dev->pirlRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the PTP semaphore.    */
    if((dev->ptpRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the Table semaphore.    */
    if((dev->tblRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the EEPROM Configuration semaphore.    */
    if((dev->eepromRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Initialize the PHY Device Register Access semaphore.    */
    if((dev->phyRegsSem = prvCpssDrvGtSemCreate(dev,GT_CPSS_SEM_FULL)) == 0)
    {
        PRV_CPSS_DBG_INFO(("semCreate Failed.\n"));
        prvCpssDrvQdUnloadDriver(dev);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    dev->devEnabled = 1;
    dev->devNum = cfg->devNum;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}


/**
* @internal prvCpssDrvQdUnloadDriver function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 1. This function should be called only after successful execution of
*       prvCpssDrvQdLoadDriver().
*
*/
GT_STATUS prvCpssDrvQdUnloadDriver
(
    IN GT_CPSS_QD_DEV* dev
)
{
    PRV_CPSS_DBG_INFO(("prvCpssDrvQdUnloadDriver Called.\n"));

    /* Delete the MultiAddress mode reagister access semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->multiAddrSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the ATU semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->atuRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the VTU semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->vtuRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the STATS semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->statsRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the PIRL semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->pirlRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the PTP semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->ptpRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the Table semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->tblRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the EEPROM Configuration semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->eepromRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Delete the PHY Device semaphore.    */
    if(prvCpssDrvGtSemDelete(dev,dev->phyRegsSem) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtMemSet(dev,0,sizeof(GT_CPSS_QD_DEV));

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}



/**
* @internal prvCpssDrvLport2port function
* @endinternal
*
* @brief   This function converts logical port number to physical port number
*
* @param[in] port                     - physical port list in vector
* @param[in] port                     - logical  number
*                                       physical port number
*/
GT_U8 prvCpssDrvLport2port
(
    IN GT_U16    portVec,
    IN GT_CPSS_LPORT     port
)
{
    GT_U8    hwPort, tmpPort;

    tmpPort = hwPort = 0;

    while (portVec)
    {
        if(portVec & 0x1)
        {
            if((GT_CPSS_LPORT)tmpPort == port)
                break;
            tmpPort++;
        }
        hwPort++;
        portVec >>= 1;
    }

    if (!portVec)
        hwPort = GT_CPSS_INVALID_PORT;

    return hwPort;
}

/**
* @internal prvCpssDrvPort2lport function
* @endinternal
*
* @brief   This function converts physical port number to logical port number
*
* @param[in] portVec                  - physical port list in vector
*                                      port    - logical port number
*                                       physical port number
*/
GT_CPSS_LPORT prvCpssDrvPort2lport
(
    IN  GT_U16    portVec,
    IN  GT_U8     hwPort
)
{
    GT_U8        tmpPort,port;

    port = 0;

    if (hwPort == GT_CPSS_INVALID_PORT)
        return (GT_CPSS_LPORT)hwPort;

    if (!GT_CPSS_IS_PORT_SET(portVec, hwPort))
        return (GT_CPSS_LPORT)GT_CPSS_INVALID_PORT;

    for (tmpPort = 0; tmpPort <= hwPort; tmpPort++)
    {
        if(portVec & 0x1)
        {
            port++;
        }
        portVec >>= 1;
    }

    return (GT_CPSS_LPORT)port-1;
}

/**
* @internal prvCpssDrvLportvec2portvec function
* @endinternal
*
* @brief   This function converts logical port vector to physical port vector
*
* @param[in] portVec                  - physical port list in vector
* @param[in] lVec                     - logical port vector
*                                       physical port vector
*/
GT_U32 prvCpssDrvLportvec2portvec
(
    IN  GT_U16     portVec,
    IN  GT_U32     lVec
)
{
    GT_U32    pVec, vec;

    pVec = 0;
    vec = 1;

    while (portVec)
    {
        if(portVec & 0x1)
        {
            if(lVec & 0x1)
            {
                pVec |= vec;
            }
            lVec >>= 1;
        }

        vec <<= 1;
        portVec >>= 1;
    }

    if(lVec)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CPSS_INVALID_PORT_VEC, LOG_ERROR_NO_MSG);

    return pVec;
}

/**
* @internal prvCpssDrvPortvec2lportvec function
* @endinternal
*
* @brief   This function converts physical port vector to logical port vector
*
* @param[in] portVec                  - physical port list in vector
* @param[in] pVec                     - physical port vector
*                                       logical port vector
*/
GT_U32 prvCpssDrvPortvec2lportvec
(
    IN  GT_U16    portVec,
    IN  GT_U32    pVec
)
{
    GT_U32    lVec, vec;

    lVec = 0;
    vec = 1;

    while (portVec)
    {
        if(portVec & 0x1)
        {
            if(pVec & 0x1)
            {
                lVec |= vec;
            }
            vec <<= 1;
        }

        pVec >>= 1;
        portVec >>= 1;
    }

    return lVec;
}

/**
* @internal prvCpssDrvLport2phy function
* @endinternal
*
* @brief   This function converts logical port number to physical phy number.
*
* @param[in] port                     - physical port list in vector
* @param[in] port                     - logical  number
*                                       physical port number
*/
GT_U8 prvCpssDrvLport2phy
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port
)
{
  GT_U8    hwPort;

  if (dev->validPhyVec & (1<<port))
  {
    hwPort = GT_CPSS_LPORT_2_PORT(port);
  }
  else
    hwPort = (GT_U8)GT_CPSS_INVALID_PHY;

  return hwPort;
}

/**
* @internal gtRegister function
* @endinternal
*
* @brief   BSP should register the following functions:
*         1) MII Read - (Input, must provide)
*         allows QuarterDeck driver to read QuarterDeck device registers.
*         2) MII Write - (Input, must provice)
*         allows QuarterDeck driver to write QuarterDeck device registers.
*         3) Semaphore Create - (Input, optional)
*         OS specific Semaphore Creat function.
*         4) Semaphore Delete - (Input, optional)
*         OS specific Semaphore Delete function.
*         5) Semaphore Take - (Input, optional)
*         OS specific Semaphore Take function.
*         6) Semaphore Give - (Input, optional)
*         OS specific Semaphore Give function.
*         Notes: 3) ~ 6) should be provided all or should not be provided at all.
*
* @note This function should be called only once.
*
*/
static GT_BOOL gtRegister(GT_CPSS_QD_DEV *dev, PRV_CPSS_BSP_FUNCTIONS* pBSPFunctions)
{
    dev->fgtReadMii =  pBSPFunctions->readMii;
    dev->fgtWriteMii = pBSPFunctions->writeMii;

    dev->semCreate = pBSPFunctions->semCreate;
    dev->semDelete = pBSPFunctions->semDelete;
    dev->semTake   = pBSPFunctions->semTake  ;
    dev->semGive   = pBSPFunctions->semGive  ;

    return GT_TRUE;
}


