#include <Copyright.h>
/**
********************************************************************************
* @file gtSysConfig.h
*
* @brief API/Structure definitions for Marvell Soho driver System Configure functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtSysConfig.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell Soho driver System Configure functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtSysConfig_h
#define __prvCpssDrvGtSysConfig_h

#include <msApiTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported System Config Types                                             */
/****************************************************************************/

#define CPSS_MAX_SWITCH_PORTS    11
#define CPSS_VERSION_MAX_LEN     30

/**
* @struct GT_CPSS_VERSION
 *
 * @brief This struct holds the package version.
*/
typedef struct{

    GT_U8 version[CPSS_VERSION_MAX_LEN];

} GT_CPSS_VERSION;

/* Define the different device type that may exist in system */
typedef enum
{
    GT_CPSS_88E6390X    = 0x0A1,  /* 88E6390X - BGA package */
    GT_CPSS_88E6390     = 0x390,  /* 88E6390  - PQFP package */
    GT_CPSS_88E6190X    = 0x0A0,  /* 88E6190X - BGA package */
    GT_CPSS_88E6190     = 0x190,  /* 88E6190  - PQFP package */
    GT_CPSS_88E6190T    = 0x191   /* 88E6190  - PQFP package */
}GT_CPSS_DEVICE;

/* Definition for the revision number of the device        */
typedef enum
{
    GT_CPSS_REV_0 = 0,
    GT_CPSS_REV_1,
    GT_CPSS_REV_2,
    GT_CPSS_REV_3
}GT_CPSS_DEVICE_REV;

typedef struct _GT_QD_DEV GT_CPSS_QD_DEV;

typedef enum{
    GT_CPSS_CONTEXT_GENERIC, /* generic task/thread context */
    GT_CPSS_CONTEXT_ISR      /* ISR context */

}GT_CPSS_CONTEXT_ENT;
/*
 * definitions for registering MII access functions.
 */
typedef GT_STATUS (*FGT_CPSS_READ_MII)
(
    GT_CPSS_QD_DEV          *dev,
    GT_U8                   phyAddr,
    GT_U8                   miiReg,
    GT_U16*                 value,
    GT_CPSS_CONTEXT_ENT     context
);

typedef GT_STATUS (*FGT_CPSS_WRITE_MII)
(
    GT_CPSS_QD_DEV          *dev,
    GT_U8                   phyAddr,
    GT_U8                   miiReg,
    GT_U16                  value,
    GT_CPSS_CONTEXT_ENT     context
);

typedef enum
{
    GT_CPSS_SEM_EMPTY,
    GT_CPSS_SEM_FULL
} GT_CPSS_SEM_BEGIN_STATE;

/*
 * definitions for semaphore functions.
 */
typedef GT_CPSS_SEM (*FGT_CPSS_SEM_CREATE)
(
    GT_CPSS_SEM_BEGIN_STATE     state
);
typedef GT_STATUS (*FGT_CPSS_SEM_DELETE)
(
    GT_CPSS_SEM     semId
);
typedef GT_STATUS (*FGT_CPSS_SEM_TAKE)
(
    GT_CPSS_SEM     semId,
    GT_U32          timOut
);
typedef GT_STATUS (*FGT_CPSS_SEM_GIVE)
(
    GT_CPSS_SEM     semId
);

/*******************************************************************************
* FGT_CPSS_GLOBAL_PROTECTION
*
* DESCRIPTION:
*       global protection : sych between threads and the ISR operations.
*
* INPUTS:
*       dev - (pointer to) the device driver info.
*       startProtection - indication to start protection.
*                       GT_TRUE  - start the protection
*                       GT_FALSE - end   the protection
*       cookiePtr   - (pointer to) cookie that is internally set and used.
*                   the cookie must not be changed by the caller between calls
*                   for 'start' and 'stop'
* OUTPUTS:
*       None
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef void (*FGT_CPSS_GLOBAL_PROTECTION)
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         startProtection,
    IN  GT_32           *cookiePtr
);


/*
 * Typedef: struct GT_CPSS_QD_DEV
 *
 * Description: Includes Tapi layer switch configuration data.
 *
 * Fields:
 *   deviceId       - The device type identifier.
 *   revision       - The device revision number.
 *   baseRegAddr    - Switch Port Base Register address.
 *   numOfPorts     - Number of active ports.
 *   maxPorts       - max ports. This field is only for driver's use.
 *   cpuPortNum     - Logical port number whose physical port is connected to the CPU.
 *   maxPhyNum      - max configurable Phy address.
 *   stpMode        - current switch STP mode (0 none, 1 en, 2 dis)
 *   phyAddr        - SMI address used to access Switch registers(only for SMI_MULTI_ADDR_MODE).
 *   validPortVec   - valid port list in vector format
 *   validPhyVec    - valid phy list in vector format
 *   validSerdesVec - valid serdes list in vector format
 *   devName        - name of the device in group 0
 *   devName1       - name of the device in group 1
 *   multiAddrSem   - Semaphore for Accessing SMI Device
 *   atuRegsSem     - Semaphore for ATU access
 *   vtuRegsSem     - Semaphore for VTU access
 *   statsRegsSem   - Semaphore for RMON counter access
 *   pirlRegsSem    - Semaphore for PIRL Resource access
 *   ptpRegsSem     - Semaphore for PTP Resource access
 *   tblRegsSem     - Semaphore for various Table Resource access,
 *                    such as Trunk Tables and Device Table
 *   eepromRegsSem  - Semaphore for eeprom control access
 *   phyRegsSem     - Semaphore for PHY Device access
 *   fgtReadMii     - platform specific SMI register Read function
 *   fgtWriteMii    - platform specific SMI register Write function
 *   semCreate      - function to create semapore
 *   semDelete      - function to delete the semapore
 *   semTake        - function to get a semapore
 *   semGive        - function to return semaphore
 */
struct _GT_QD_DEV
{
    GT_CPSS_DEVICE  deviceId;
    GT_CPSS_LPORT   cpuPortNum;
    GT_U8           revision;
    GT_U8           devNum;
    GT_U8       devEnabled;
    GT_U8       baseRegAddr;
    GT_U8       numOfPorts;
    GT_U8       maxPorts;
    GT_U8       maxPhyNum;
    GT_U8       stpMode;        /* rsvd for furture */
    GT_U8       accessMode; /* rsvd for furture */
    GT_U8       phyAddr;
    GT_U16      validPortVec;
    GT_U16      validPhyVec;
    /*GT_U16      validSerdesVec;*/
    GT_U32      devName;
    GT_U32      devName1;

    GT_CPSS_SEM      multiAddrSem;
    GT_CPSS_SEM      atuRegsSem;
    GT_CPSS_SEM      vtuRegsSem;
    GT_CPSS_SEM      statsRegsSem;
    GT_CPSS_SEM      pirlRegsSem;
    GT_CPSS_SEM      ptpRegsSem;
    GT_CPSS_SEM      tblRegsSem;
    GT_CPSS_SEM      eepromRegsSem;
    GT_CPSS_SEM      phyRegsSem;

    FGT_CPSS_READ_MII          fgtReadMii;
    FGT_CPSS_WRITE_MII         fgtWriteMii;

    FGT_CPSS_SEM_CREATE  semCreate;             /* create semaphore */
    FGT_CPSS_SEM_DELETE  semDelete;             /* delete the semaphore */
    FGT_CPSS_SEM_TAKE    semTake;            /* try to get a semaphore */
    FGT_CPSS_SEM_GIVE    semGive;            /* return semaphore */

    FGT_CPSS_GLOBAL_PROTECTION   globalProtection;   /* start,stop global protection */

    void            *appData;
};

/* Definition for BSP functions*/
typedef struct _BSP_FUNCTIONS
{
    FGT_CPSS_READ_MII     readMii;      /* read MII Registers */
    FGT_CPSS_WRITE_MII     writeMii;    /* write MII Registers */
    FGT_CPSS_SEM_CREATE    semCreate;   /* create semapore */
    FGT_CPSS_SEM_DELETE    semDelete;   /* delete the semapore */
    FGT_CPSS_SEM_TAKE    semTake;       /* try to get a semapore */
    FGT_CPSS_SEM_GIVE    semGive;       /* return semaphore */
}PRV_CPSS_BSP_FUNCTIONS;

/* System configuration Parameters struct*/
typedef struct
{
    GT_U8               devNum;         /* Device Number, 0~31*/
    GT_U8               baseAddr;       /* SMI Device Address, ADDR[4:0 ]*/
    GT_CPSS_LPORT       cpuPortNum;     /* CPU Port, rsvd for furture */
    GT_BOOL             initPorts;      /* rsvd for furture */
    PRV_CPSS_BSP_FUNCTIONS  BSPFunctions;
    GT_U32              skipInitSetup;  /* rsvd for furture */
}GT_CPSS_SYS_CONFIG;


/****************************************************************************/
/* Exported System Config Functions                                         */
/****************************************************************************/

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
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note prvCpssDrvQdUnloadDriver is also provided to do driver cleanup.
*
*/
GT_STATUS prvCpssDrvQdLoadDriver
(
    IN  GT_CPSS_SYS_CONFIG  *cfg,
    OUT GT_CPSS_QD_DEV      *dev
);

/**
* @internal prvCpssDrvQdUnloadDriver function
* @endinternal
*
* @brief   This function unloads the QuaterDeck Driver.
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
    IN  GT_CPSS_QD_DEV  *dev
);

/**
* @internal prvCpssDrvGtVersion function
* @endinternal
*
* @brief   This function returns the version of the QuarterDeck SW suite.
*
* @param[out] version                  - QuarterDeck software version.
*                                       GT_OK on success,
*                                       GT_BAD_PARAM on bad parameters,
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDrvGtVersion
(
    OUT GT_CPSS_VERSION   *version
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtSysConfig_h */

