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
* @file appDemoCfgMisc.h
*
* @brief Includes misc functions definitions to be used by the application demo
* configuration functions.
*
* @version   9
********************************************************************************
*/
#ifndef __appDemoCfgMisch
#define __appDemoCfgMisch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#ifdef IMPL_FA
#include <cpssFa/generic/cpssFaHwInit/gtCoreFaHwInit.h>
#endif

/**
* @internal appDemoRxBuffsCacheableSet function
* @endinternal
*
* @brief   This function set status of RX buffers - cacheable or not.
*         Should be called one time BEFORE cpssInitSystem
* @param[in] buffersCacheable         - Should be GT_TRUE if RX buffers are in cacheable memory.
*
* @retval GT_OK                    - always,
*/
GT_STATUS appDemoRxBuffsCacheableSet
(
    IN GT_BOOL buffersCacheable
);

/**
* @internal appDemoRawSocketModeSet function
* @endinternal
*
* @brief   Instruct the system to use raw packets in MII mode.
*
* @retval GT_OK                    - always,
*/
GT_STATUS appDemoRawSocketModeSet
(
    IN GT_VOID
);

/**
* @internal appDemoLinuxModeSet function
* @endinternal
*
* @brief   Set port <portNum> to Linux Mode (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
* @param[in] ip1                      ip2, ip3, ip4 - The ip address to assign to the port, 4 numbers
*
* @retval GT_OK                    - always,
*/

GT_STATUS appDemoLinuxModeSet
(
 IN GT_U32  portNum,
 IN GT_U32 ip1,
 IN GT_U32 ip2,
 IN GT_U32 ip3,
 IN GT_U32 ip4
);

/**
* @internal appDemoLinuxModeGet function
* @endinternal
*
* @brief   Get port <portNum> Linux Mode indication (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
*                                       GT_TRUE if Linux mode, or
*                                       GT_FALSE otherwise.
*/

GT_BOOL appDemoLinuxModeGet
(
 IN GT_U32  portNum
);

/**
* @internal appDemoAllocateDmaMem function
* @endinternal
*
* @brief   This function allocates memory for phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
* @param[in] devType                  - The Pp device type to allocate the memory for.
* @param[in] rxDescNum                - Number of Rx descriptors (and buffers) to allocate.
* @param[in] rxBufSize                - Size of each Rx Buffer to allocate.
* @param[in] rxBufAllign              - Ammount of allignment required on the Rx buffers.
* @param[in] txDescNum                - Number of Tx descriptors to allocate.
* @param[in] auDescNum                - Number of address update descriptors to allocate.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters including the required
*                                      allocations.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoAllocateDmaMem
(
    IN      CPSS_PP_DEVICE_TYPE         devType,
    IN      GT_U32                      rxDescNum,
    IN      GT_U32                      rxBufSize,
    IN      GT_U32                      rxBufAllign,
    IN      GT_U32                      txDescNum,
    IN      GT_U32                      auDescNum,
    INOUT   CPSS_PP_PHASE2_INIT_PARAMS    *ppPhase2Params
);

/**
* @internal appDemoDeAllocateDmaMem function
* @endinternal
*
* @param[in] devNum                - device number.
* @brief   This function deallocates memory of phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeAllocateDmaMem
(
    IN      GT_U8  devNum
);

/**
* @internal appDemoGetPpRegCfgList function
* @endinternal
*
* @brief   This function returns the list of registers to be configured to a given
*         device before and after the startInit operation. (Passed to
*         corePpHwStartInit() ).
*
* @param[out] regCfgList               - A pointer to the register list.
* @param[out] regCfgListSize           - Number of elements in regListPtr.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetPpRegCfgList
(
    IN  CPSS_PP_DEVICE_TYPE         devType,
    IN  GT_BOOL                     isB2bSystem,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);

#ifdef IMPL_FA
/**
* @internal appDemoGetFaRegCfgList function
* @endinternal
*
* @brief   This function returns the list of registers to be configured to a given
*         FA device before and after the startInit operation. (Passed to
*         coreFaHwStartInit() ).
* @param[in] faDevType                - The Fa device type.
* @param[in] devType                  - The PP device type. See comment.
*
* @param[out] regCfgList               - A pointer to the register list.
* @param[out] regCfgListSize           - Number of elements in regListPtr.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note The input of PP device type is needed as an indication to the BOARD
*       type. XG Boards frequency is different then other boards so FA has
*       a different configuration.
*
*/
GT_STATUS appDemoGetFaRegCfgList
(
    IN  GT_FA_DEVICE                faDevType,
    IN  CPSS_PP_DEVICE_TYPE         devType,  /* See comment */
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);
#endif

/**
* @internal appDemoSetPortPhyAddr function
* @endinternal
*
* @brief   Configure the port's default phy address, this function should be used
*         to change the default port's phy address.
* @param[in] devNum                   - The Pp device number.
* @param[in] portNum                  - The port number to update the phy address for.
* @param[in] phyAddr                  - The new phy address, only the lower 5 bits of this param are
*                                      relevant.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function should be called after corePpHwPhase1Init(), and before
*       gtPortSmiInit() (in Tapi level).
*
*/
GT_STATUS appDemoSetPortPhyAddr
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U8   phyAddr
);

/**
* @internal appDemoGetRegisterWriteTime function
* @endinternal
*
* @brief   Get the time needed for register write consecutive operations
*         as stated in the request.
* @param[in] devNum                   - the device number to write to.
* @param[in] regAddr1                 - first register address to write to.
* @param[in] regAddr2                 - second register address to write to.
* @param[in] regAddr3                 - third register address to write to.
* @param[in] regData                  - the data to write to the register.
* @param[in] numOfWrites              - the number of times to perform the register write operation.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetRegisterWriteTime
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr1,
    IN GT_U32 regAddr2,
    IN GT_U32 regAddr3,
    IN GT_U32 regData,
    IN GT_U32 numOfWrites
);

/**
* @internal appDemoGetRegisterReadTime function
* @endinternal
*
* @brief   Get the time needed for register read consecutive operations
*         as stated in the request.
* @param[in] devNum                   - the device number to read from.
* @param[in] regAddr1                 - first register address to read.
* @param[in] regAddr2                 - second register address to read.
* @param[in] regAddr3                 - third register address to read.
* @param[in] numOfReads               - the number of times to perform the register read operation.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetRegisterReadTime
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr1,
    IN GT_U32 regAddr2,
    IN GT_U32 regAddr3,
    IN GT_U32 numOfReads
);

GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
);

CPSS_PORT_SPEED_ENT CPSS_SPEED_Mbps_2_ENM
(
    GT_U32 speedMbps
);

GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
);

typedef struct
{
    GT_U32   u32_Num;
    GT_CHAR *u32_Str;
}APPDEMO_GT_U32_2_STR_STC;

/*******************************************************************************
* u32_2_STR
*
* DESCRIPTION:
*       conver integer to string according to table
*
* APPLICABLE DEVICES:
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*       u32_Num          - number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       converter string , or string represtantion of integer value if not found
*
* COMMENTS:
*
*******************************************************************************/
GT_CHAR * u32_2_STR
(
    IN GT_U32 u32_Num,
    IN APPDEMO_GT_U32_2_STR_STC * tbl
);

GT_CHAR * RXDMA_IfWidth_2_STR
(
    IN GT_U32 RXDMA_IfWidth
);


GT_CHAR * TX_FIFO_IfWidth_2_STR
(
    IN GT_U32 TX_FIFO_IfWidth
);

GT_CHAR * TXDMA_RateLimitResidueValue_2_STR
(
    IN GT_U32 TXDMA_residueValue
);

#define PHY_1548M_NUM_OF_PORTS_CNS 4
/*
 * typedef: struct QUAD_PHY_CFG_STC
 *
 * Description: PHY Configuration entry
 *
 * Fields:
 *    allPorts           - Configure all 4 ports of PHY
 *                             AD_FIRST_PORT  - configure only port 0 of PHY
 *                             AD_ALL_PORTS   - configure all  ports on phy
 *                             AD_BAD_PORT    - bad port , may be used for end of sequence determination
 *    phyRegAddr         - Address of PHY register
 *    phyRegData         - Data to write to a PHY register
 *    sleepTime          - sleep after write in ms
 *
 * Comments:
 */

typedef enum GT_PHY_WRITE_TYPE_ENT
{
    AD_FIRST_PORT = 0
   ,AD_ALL_PORTS  = 1
   ,AD_BAD_PORT  = -1
}GT_PHY_WRITE_TYPE_ENT;

typedef struct
{
    GT_PHY_WRITE_TYPE_ENT allPorts;
    GT_U8                 phyRegAddr;
    GT_U16                phyRegData;
}APP_DEMO_QUAD_PHY_CFG_STC;

typedef struct
{
    APP_DEMO_QUAD_PHY_CFG_STC *seqPtr;
    GT_U32            sleepTimeAfter_ms;
}APP_DEMO_QUAD_PHY_CFG_STC_PHASE_STC;

/**
* @internal appDemoBoardExternalPhyConfig function
* @endinternal
*
* @brief   External PHY configuration.
*
* @param[in] devNum                   - device number
* @param[in] phyType                  - PHY type
* @param[in] smiInterface             - SMI interface
* @param[in] smiStartAddr             - address of SMI device to be configured
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad phyType or smiInterface
* @retval GT_NOT_READY             - SMI is busy
* @retval GT_HW_ERROR              - HW error
*/
GT_STATUS appDemoBoardExternalPhyConfig
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       phyType,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32                       smiStartAddr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoCfgMisch */



