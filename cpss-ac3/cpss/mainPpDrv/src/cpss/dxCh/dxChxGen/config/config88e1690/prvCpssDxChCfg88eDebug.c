/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChCfg88eDebug.c
*
* @brief File to debug the 88e1690 PHY-MAC.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>

#include <msApi.h>
#include <msApiTypes.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>

#define SMI_DEVICE_MAX_NUM      6

#define PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(smiDev)      \
    if (smiDev >= SMI_DEVICE_MAX_NUM)                                    \
    {                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SMI device ID: Valid range [0...5]"); \
    }

#define PRV_CPSS_CHECK_PHY_PORT_NUM_MAC(phyPort)       \
    if(phyPort < 1 || phyPort > 10)                       \
    {                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY port: Valid range [1...10]"); \
    }

#define PRV_CPSS_CHECK_EXT_PHY_PORT_NUM_MAC(phyPort)       \
    if((phyPort < 1 || phyPort > 10) &&                      \
        (phyPort != PRV_CPSS_GLOBAL1_DEV_ADDR) &&                      \
        (phyPort != PRV_CPSS_GLOBAL2_DEV_ADDR) &&                      \
        (phyPort != PRV_CPSS_CPU_PORT_ADDR) &&                      \
        (phyPort != PRV_CPSS_TCAM_DEV_ADDR))                      \
    {                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY port: Valid range [1...10,27,28,30]"); \
    }

#define PRV_CPSS_CHECK_PHY_SERDES_PORT_NUM_MAC(phyPort)       \
    if(phyPort != 9 && phyPort != 10)                       \
    {                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY serdes port: Valid range [9, 10]"); \
    }

#define PRV_CPSS_CHECK_PHY_SERDES_PORT_LANE_NUM_MAC(phyPort)       \
    if(phyPort >= 4)                                \
    {                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY serdes port lane: Valid range [0...3]"); \
    }

#define PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(regAddr)   \
    if(regAddr >= 32)                                  \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SMI address: Valid range [0...31]"); \
    }

#define PRV_CPSS_CHECK_PHY_PAGE_NUM_MAC(phyPage)   \
    if(phyPage >= 8)                                  \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY page: Valid range [0...7]"); \
    }

#define PRV_CPSS_CHECK_PHY_GLOBAL_ADDRESS_MAC(regAddr)   \
    if((regAddr != 0x1B) && (regAddr != 0x1C))              \
    {                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong PHY global register: Valid range [0x1B, 0x1C]"); \
    }

#define SERDES_DEV_ADDRESS  4

static GT_U8 serdesSmiAdressArray[8] = {0x9, 0x12, 0x13, 0x14, 0xA, 0x15, 0x16, 0x17};

extern GT_STATUS   writeTo88E1690OnPort
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPort,
    IN GT_U32   smiDevAddr,
    IN GT_U32   regOffset,
    IN GT_U16   regData
);

extern GT_STATUS   readFrom88E1690OnPort
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPort,
    IN GT_U32   smiDevAddr,
    IN GT_U32   regOffset,
    IN GT_U16   *regDataPtr
);


static GT_STATUS  remotePhyMacPortGet
(
    IN GT_U8    devNum,
    IN GT_U32   macPortId,
    OUT GT_U32 * macPortNumPtr
)
{
    GT_U32  port, index = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    for(port = 0; port < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; port++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[port] == NULL)
        {
            continue;
        }

        if (macPortId == index)
        {
            /* Match found - return with remote mac port number */
            *macPortNumPtr = port;
            return GT_OK;
        }
        else
        {
            /* Match not found - continue to next valid remote mac port */
            index++;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssMacPhyPortRegisterWrite function
* @endinternal
*
* @brief   Write PHY port register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] portRegAddr              - port regiater address
* @param[in] data                     -  to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortRegisterWrite
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort,
    IN  GT_U32                      portRegAddr,
    IN  GT_U16                      data
)
{
    GT_STATUS rc;
    GT_U32 macPort;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_EXT_PHY_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(portRegAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Write the port register */
    return writeTo88E1690OnPort(devNum, macPort, phyPort, portRegAddr, data);
}

/**
* @internal prvCpssMacPhyPortRegisterRead function
* @endinternal
*
* @brief   Read PHY port register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] portRegAddr              - port regiater address
*
* @param[out] dataPtr                  - (pointer to ) data to be read to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssMacPhyPortRegisterRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort,
    IN  GT_U32                      portRegAddr,
    OUT  GT_U16                     *dataPtr
)
{
    GT_STATUS rc;
    GT_U32 macPort;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_EXT_PHY_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(portRegAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Read the port register */
    return readFrom88E1690OnPort(devNum, macPort, phyPort, portRegAddr, dataPtr);
}

/**
* @internal prvCpssMacPhyPortSmiRegisterWrite function
* @endinternal
*
* @brief   Write PHY port SMI register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] page                     - SMI  address
* @param[in] smiRegAddr               - port regiater address
* @param[in] data                     -  to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortSmiRegisterWrite
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       page,
    IN  GT_U8                       smiRegAddr,
    IN  GT_U16                      data
)
{
    GT_STATUS rc;
    GT_U32 macPort;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_PAGE_NUM_MAC(page);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(smiRegAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "SMI device not found: %d", phyMacDeviceNum);
    }

    rc = prvCpssDrvHwWritePagedPhyReg(portInfoPtr->drvInfoPtr, phyPort, page, smiRegAddr, data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Write PHY register HW error: page = %d, port = %d", phyPort, page);
    }

    return GT_OK;
}

/**
* @internal prvCpssMacPhyPortSmiRegisterRead function
* @endinternal
*
* @brief   Read PHY port SMI register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] page                     - SMI  address
* @param[in] smiRegAddr               - port regiater address
*
* @param[out] dataPtr                  - (pointer to) data to be read from register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssMacPhyPortSmiRegisterRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       page,
    IN  GT_U8                       smiRegAddr,
    OUT  GT_U16                     *dataPtr
)
{
    GT_STATUS rc;
    GT_U32 macPort;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_PAGE_NUM_MAC(page);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(smiRegAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "SMI device not found: %d", phyMacDeviceNum);
    }

    rc = prvCpssDrvHwReadPagedPhyReg(portInfoPtr->drvInfoPtr, phyPort, page, smiRegAddr, dataPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Read PHY register HW error");
    }

    return GT_OK;
}

/**
* @internal prvCpssMacPhyGlobalRegisterWrite function
* @endinternal
*
* @brief   Write PHY port global register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] smiAddr                  - global registers SMI address
* @param[in] regAddr                  - global regiater address
* @param[in] data                     -  to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyGlobalRegisterWrite
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       smiAddr,
    IN  GT_U8                       regAddr,
    IN  GT_U16                      data
)
{
    GT_STATUS rc;
    GT_U32 macPort;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_GLOBAL_ADDRESS_MAC(smiAddr);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(regAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Write the global register */
    return writeTo88E1690OnPort(devNum, macPort, smiAddr, regAddr, data);
}

/**
* @internal prvCpssMacPhyGlobalRegisterRead function
* @endinternal
*
* @brief   Read PHY port global register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] smiAddr                  - global registers SMI address
* @param[in] regAddr                  - global regiater address
*
* @param[out] dataPtr                  - (pointer to) data to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, SMI device address or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssMacPhyGlobalRegisterRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       smiAddr,
    IN  GT_U8                       regAddr,
    OUT  GT_U16                     *dataPtr
)
{
    GT_STATUS rc;
    GT_U32 macPort;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_GLOBAL_ADDRESS_MAC(smiAddr);
    PRV_CPSS_CHECK_PHY_SMI_ADDRESS_NUM_MAC(regAddr);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read the global register */
    return readFrom88E1690OnPort(devNum, macPort, smiAddr, regAddr, dataPtr);
}

/**
* @internal prvCpssMacPhySerdesRegisterWrite function
* @endinternal
*
* @brief   Write PHY port serdes register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] lane                     - remote physical port's lane
* @param[in] regAddr                  - regiater address
* @param[in] data                     -  to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or lane
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhySerdesRegisterWrite
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       lane,
    IN  GT_U16                      regAddr,
    IN  GT_U16                      data

)
{
    GT_STATUS rc;
    GT_U32 macPort;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;
    GT_U32 index;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_SERDES_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_SERDES_PORT_LANE_NUM_MAC(lane);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Remote PHY MAC device not found: %d", phyMacDeviceNum);
    }

    index = (phyPort % 9) * 4 + lane;

    /* Write serdes specific register */
    return prvCpssDrvHwSetSMIC45PhyReg(portInfoPtr->drvInfoPtr, SERDES_DEV_ADDRESS, serdesSmiAdressArray[index], regAddr, data);
}


/**
* @internal prvCpssMacPhySerdesRegisterRead function
* @endinternal
*
* @brief   Read PHY port serdes register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] lane                     - remote physical port's lane
* @param[in] regAddr                  - regiater address
*
* @param[out] dataPtr                  - (pointer to) data to be written to register
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or lane
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssMacPhySerdesRegisterRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       lane,
    IN  GT_U16                      regAddr,
    OUT  GT_U16                     *dataPtr
)
{
    GT_STATUS rc;
    GT_U32 macPort;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;
    GT_U32 index;

    PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
    PRV_CPSS_CHECK_PHY_SERDES_PORT_NUM_MAC(phyPort);
    PRV_CPSS_CHECK_PHY_SERDES_PORT_LANE_NUM_MAC(lane);

    rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Remote PHY MAC device not found: %d", phyMacDeviceNum);
    }

    index = (phyPort % 9) * 4 + lane;

    /* Write serdes specific register */
    return prvCpssDrvHwGetSMIC45PhyReg(portInfoPtr->drvInfoPtr, SERDES_DEV_ADDRESS, serdesSmiAdressArray[index], regAddr, dataPtr);
}


/**
* @internal prvCpssMacPhyPortRegistersPrint function
* @endinternal
*
* @brief   Print-out PHY port registers data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortRegistersPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort
)
{
    GT_STATUS rc;
    GT_U16 data;
    GT_U32 port = 1, address, maxPorts = 11;

    if (phyPort)
    {
        PRV_CPSS_CHECK_EXT_PHY_PORT_NUM_MAC(phyPort)
        port = phyPort;
        maxPorts = phyPort + 1;
    }

    cpssOsPrintf("\n+----------------------------");
    cpssOsPrintf("\n| Port    Address      Data |");
    cpssOsPrintf("\n+----------------------------");

    for (; port < maxPorts; port++)
    {
        for (address = 0; address < 32; address++)
        {
            rc = prvCpssMacPhyPortRegisterRead(devNum, phyMacDeviceNum, port, address, &data);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (data)
            {
                cpssOsPrintf("\n%5d %10X %#10x", port, address, data);
            }
        }
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhyPortSmiRegistersPrint function
* @endinternal
*
* @brief   Print-out PHY port SMI registers data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortSmiRegistersPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort
)
{
    GT_STATUS rc;
    GT_U16 data;
    GT_U8 port = 1, address, maxPorts = 11;
    GT_U8 page = 0, maxPage = 8;

    if (phyPort)
    {
        PRV_CPSS_CHECK_PHY_PORT_NUM_MAC(phyPort)
        port = phyPort;
        maxPorts = phyPort + 1;
    }

    cpssOsPrintf("\n+--------------------------------");
    cpssOsPrintf("\n| Port  Page    Address    Data |");
    cpssOsPrintf("\n+--------------------------------");

    for (; port < maxPorts; port++)
    {
        for (; page < maxPage; page++)
        {
            for (address = 0; address < 32; address++)
            {
                rc = prvCpssMacPhyPortSmiRegisterRead(devNum, phyMacDeviceNum, port, page, address, &data);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (data)
                {
                    cpssOsPrintf("\n%5d %4d %10X %#10x", port, page, address, data);
                }
            }
        }
        page = 0;
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhyPortRegisterPrint function
* @endinternal
*
* @brief   Print PHY port register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] portRegAddr              - port regiater address
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortRegisterPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort,
    IN  GT_U32                      portRegAddr
)
{
    GT_STATUS   rc;
    GT_U16      data;

    rc = prvCpssMacPhyPortRegisterRead(devNum, phyMacDeviceNum, phyPort, portRegAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+------------------------+");
    cpssOsPrintf("\n| Port   Address    Data |");
    cpssOsPrintf("\n+------------------------+");
    cpssOsPrintf("\n%5d %8X %#10x", phyPort, portRegAddr, data);
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhyPortSmiRegisterPrint function
* @endinternal
*
* @brief   Print PHY port SMI register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] page                     - SMI  address
* @param[in] smiRegAddr               - port regiater address
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyPortSmiRegisterPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       page,
    IN  GT_U8                       smiRegAddr
)
{

    GT_STATUS   rc;
    GT_U16      data;

    rc = prvCpssMacPhyPortSmiRegisterRead(devNum, phyMacDeviceNum, phyPort, page, smiRegAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+--------------------------------");
    cpssOsPrintf("\n| Port  Page    Address    Data |");
    cpssOsPrintf("\n+--------------------------------");
    cpssOsPrintf("\n%5d %4d %10X %#10x", phyPort, page, smiRegAddr, data);
    cpssOsPrintf("\n");

    return GT_OK;

}

/**
* @internal prvCpssMacPhyGlobalRegisterPrint function
* @endinternal
*
* @brief   Print PHY port global register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] smiAddr                  - global registers SMI address
* @param[in] regAddr                  - global regiater address
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, SMI device address or register address
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhyGlobalRegisterPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       smiAddr,
    IN  GT_U8                       regAddr
)
{
    GT_STATUS   rc;
    GT_U16      data;

    rc = prvCpssMacPhyGlobalRegisterRead(devNum, phyMacDeviceNum, smiAddr, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+---------------------------------------+");
    cpssOsPrintf("\n| SMI Device   Register Address    Data |");
    cpssOsPrintf("\n+---------------------------------------+");
    cpssOsPrintf("\n%8d %16X %#13x", smiAddr, regAddr, data);
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhySerdesRegisterPrint function
* @endinternal
*
* @brief   Print PHY port serdes register
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number
* @param[in] phyPort                  - remote physical port
* @param[in] lane                     - remote physical port's lane
* @param[in] regAddr                  - regiater address
*
* @retval GT_OK                    - on successfull write
* @retval GT_BAD_PARAM             - on bad MAC device number, PHY port or lane
* @retval GT_NOT_FOUND             - on remote MAC port not found
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssMacPhySerdesRegisterPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U8                       phyPort,
    IN  GT_U8                       lane,
    IN  GT_U16                      regAddr
)
{
    GT_STATUS   rc;
    GT_U16      data;

    rc = prvCpssMacPhySerdesRegisterRead(devNum, phyMacDeviceNum, phyPort, lane, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+----------------------------------+");
    cpssOsPrintf("\n| Port    Lane    Address     Data |");
    cpssOsPrintf("\n+----------------------------------+");
    cpssOsPrintf("\n%5d %8X %#10x %#10x", phyPort, lane, regAddr, data);
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhyDevicesPrint function
* @endinternal
*
* @brief   Print information about all "macPhy" devices connected to DX
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on successfull write
* @retval GT_NOT_FOUND             - on remote MAC port not found
*/
GT_STATUS prvCpssMacPhyDevicesPrint
(
    IN  GT_U8                       devNum
)
{
    GT_STATUS rc;
    GT_U32 phyMacDeviceNum;
    GT_U32 macPortNum;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;
    GT_BOOL matchFound = GT_FALSE;

    /* Loop on all SMI devices */
    for (phyMacDeviceNum = 0; phyMacDeviceNum < SMI_DEVICE_MAX_NUM; phyMacDeviceNum++)
    {
        rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPortNum);
        if (rc != GT_OK)
        {
            continue;
        }

        if (matchFound == GT_FALSE)
        {
            cpssOsPrintf("\n+---------------------------------------------------+");
            cpssOsPrintf("\n| PHY-MAC Device : MAC port : SMI bus : SMI address |");
            cpssOsPrintf("\n+---------------------------------------------------+");

            matchFound = GT_TRUE;
        }

        /* Pointer to remote PHY device */
        portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPortNum);
        cpssOsPrintf("\n%12d %12d %#12d %#12x", phyMacDeviceNum, macPortNum,
                                               portInfoPtr->connectedPhyMacInfo.mngBusNumber,
                                               portInfoPtr->connectedPhyMacInfo.busBaseAddr);
        cpssOsPrintf("\n");
    }

    if (matchFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "No remote SMI devices connected to DX");
    }

    return GT_OK;
}

/**
* @internal prvCpssMacPhyQueuesPrint function
* @endinternal
*
* @brief   Print information about all Queues utilization on all "macPhy" devices connected to DX
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on successfull write
* @retval GT_NOT_FOUND             - on remote MAC port not found
*/
GT_STATUS prvCpssMacPhyQueuesPrint
(
    IN  GT_U8                       devNum
)
{
    GT_STATUS rc;
    GT_U32 phyMacDeviceNum;
    GT_U32 phyPortNum;
    GT_U16      data;

    /* Loop on all SMI devices */
    cpssOsPrintf("\n+---------------------------------------------------------+");
    cpssOsPrintf("\n| PHY   P1   P2   P3   P4   P5   P6   P7   P8   P9   CPU  |");
    cpssOsPrintf("\n+---------------------------------------------------------+");
    for (phyMacDeviceNum = 0; phyMacDeviceNum < SMI_DEVICE_MAX_NUM; phyMacDeviceNum++)
    {
        cpssOsPrintf("\n%5d", phyMacDeviceNum);
        /* Loop on all ports */
        for (phyPortNum = 1; phyPortNum < 10; phyPortNum++)
        {
            rc = prvCpssMacPhyPortRegisterRead(devNum, phyMacDeviceNum, phyPortNum, PRV_CPSS_QD_REG_Q_COUNTER, &data);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsPrintf("%5d", data & 0x1ff);

        }
        phyPortNum = PRV_CPSS_CPU_PORT_ADDR;
        rc = prvCpssMacPhyPortRegisterRead(devNum, phyMacDeviceNum, phyPortNum, PRV_CPSS_QD_REG_Q_COUNTER, &data);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("%5d", data & 0x1ff);
    }
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssMacPhyStringSend function
* @endinternal
*
* @brief   Send string to IMP, in order to configure some firmware parameter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on successfull write
* @retval GT_NOT_FOUND             - on remote MAC port not found
*/
GT_STATUS prvCpssMacPhyStringSend
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  char                     *stringIn
)
{
    GT_STATUS rc;
    GT_U32 minPhyDeviceNums = 0;
    GT_U32 maxPhyDeviceNums = SMI_DEVICE_MAX_NUM;
    GT_U16      data;
    char                     *string;

    GT_U32  retryCount;             /* Counter for busy wait loops */


    if (phyMacDeviceNum != 99)
    {
        PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
        minPhyDeviceNums = phyMacDeviceNum;
        maxPhyDeviceNums = phyMacDeviceNum + 1;
    }

    /* Loop on all SMI devices */
    for (phyMacDeviceNum = minPhyDeviceNums; phyMacDeviceNum < maxPhyDeviceNums; phyMacDeviceNum++)
    {
        string = stringIn;
        /* Loop on string characters */
        while ((*string) != '\0')
        {
            data = 0x8200 + (*string++);
            rc = prvCpssMacPhyPortRegisterWrite(devNum, phyMacDeviceNum, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_IMP_COMM_DBG, data);
            if (rc != GT_OK)
            {
                return rc;
            }
            data = 0;
            rc = prvCpssMacPhyPortRegisterWrite(devNum, phyMacDeviceNum, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_IMP_COMM_DBG, data);
            if (rc != GT_OK)
            {
                return rc;
            }
            retryCount = 0;
            do
            {

                rc = prvCpssMacPhyPortRegisterRead(devNum, phyMacDeviceNum, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_IMP_COMM_DBG, &data);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /* check that the number of iterations does not exceed the limit */
                PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,(retryCount++));
            } while ((data & 0x10) != 0);

        }

    }

    return GT_OK;
}

/**
* @internal prvCpssMacPhyMibPrint function
* @endinternal
*
* @brief   Print all MIB counters on one port - delta from last call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phyMacDeviceNum          - physical MAC device number - use 99 for all devices
* @param[in] phyPort                  - remote physical port - 0 for all ports
*
* @retval GT_OK                    - on successfull write
* @retval GT_NOT_FOUND             - on remote MAC port not found
*/
GT_STATUS prvCpssMacPhyMibPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort
)
{
    GT_STATUS rc;
    GT_U32 macPort;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;
    GT_CPSS_STATS_COUNTER_SET    statsCounterSet;    /* Temporary result of counter read */
    GT_U32 minPhyPorts = 1;
    GT_U32 maxPhyPorts = 10;
    GT_U32 minPhyDeviceNums = 0;
    GT_U32 maxPhyDeviceNums = SMI_DEVICE_MAX_NUM;
    GT_U32          startSeconds;
    GT_U32          startNanoSeconds;
    GT_U32          stopSeconds;
    GT_U32          stopNanoSeconds;
    GT_U32          elapsedSeconds;
    GT_U32          elapsedNanoSeconds;
    static GT_CPSS_STATS_COUNTER_SET    saveStatsCounterSet[SMI_DEVICE_MAX_NUM][10] = {{{0}}};    /* Saved result of counter read */


    if (devNum == 99)
    {
        cpssOsMemSet(&saveStatsCounterSet, 0, sizeof(saveStatsCounterSet));
        return GT_OK;
    }

    if (phyMacDeviceNum != 99)
    {
        PRV_CPSS_CHECK_SMI_DEVICE_NUM_MAC(phyMacDeviceNum);
        minPhyDeviceNums = phyMacDeviceNum;
        maxPhyDeviceNums = phyMacDeviceNum + 1;
    }

    if (phyPort != 0)
    {
        PRV_CPSS_CHECK_PHY_PORT_NUM_MAC(phyPort);
        minPhyPorts = phyPort;
        maxPhyPorts = phyPort + 1;
    }

    /* Loop on all SMI devices */
    cpssOsPrintf("\n+------------------------------+");
    cpssOsPrintf("\n| PHY   Port Name      Value   |");
    cpssOsPrintf("\n+------------------------------+");
    cpssOsTimeRT(&startSeconds, &startNanoSeconds);
    for (phyMacDeviceNum = minPhyDeviceNums; phyMacDeviceNum < maxPhyDeviceNums; phyMacDeviceNum++)
    {
        rc = remotePhyMacPortGet(devNum, phyMacDeviceNum, &macPort);
        if (rc != GT_OK)
        {
            return rc;
        }

        portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, macPort);
        if(portInfoPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Remote PHY MAC device not found: %d", phyMacDeviceNum);
        }

        /* Loop on all ports */
        for (phyPort = minPhyPorts; phyPort < maxPhyPorts; phyPort++)
        {
            /* Read all counters for one port */
            rc = prvCpssDrvGstatsGetPortAllCounters(portInfoPtr->drvInfoPtr, phyPort, &statsCounterSet);
            if(rc != GT_OK)
            {
                return rc;
            }

#define PRV_STAT_COUNTER_PRINT(_counterName) \
            if (statsCounterSet._counterName != saveStatsCounterSet[phyMacDeviceNum][phyPort]._counterName)      \
            {       \
                cpssOsPrintf("\n%5d%5d %s:\t%5u", phyMacDeviceNum, phyPort, #_counterName, \
                                (statsCounterSet._counterName - saveStatsCounterSet[phyMacDeviceNum][phyPort]._counterName));   \
                saveStatsCounterSet[phyMacDeviceNum][phyPort]._counterName = statsCounterSet._counterName;      \
            }

#define PRV_STAT_DOUBLE_COUNTER_PRINT(_counterNameLo,_counterNameHi) \
            if (statsCounterSet._counterNameLo < saveStatsCounterSet[phyMacDeviceNum][phyPort]._counterNameLo)      \
            {       \
                saveStatsCounterSet[phyMacDeviceNum][phyPort]._counterNameHi++;      \
            }       \
            PRV_STAT_COUNTER_PRINT(_counterNameLo)  \
            PRV_STAT_COUNTER_PRINT(_counterNameHi)

            PRV_STAT_DOUBLE_COUNTER_PRINT(InGoodOctetsLo,InGoodOctetsHi)
            PRV_STAT_COUNTER_PRINT(InBadOctets)
            PRV_STAT_COUNTER_PRINT(OutFCSErr)
            PRV_STAT_COUNTER_PRINT(InUnicasts)
            PRV_STAT_COUNTER_PRINT(Deferred)
            PRV_STAT_COUNTER_PRINT(InBroadcasts)
            PRV_STAT_COUNTER_PRINT(InMulticasts)
            PRV_STAT_COUNTER_PRINT(Octets64)
            PRV_STAT_COUNTER_PRINT(Octets127)
            PRV_STAT_COUNTER_PRINT(Octets255)
            PRV_STAT_COUNTER_PRINT(Octets511)
            PRV_STAT_COUNTER_PRINT(Octets1023)
            PRV_STAT_COUNTER_PRINT(OctetsMax)
            PRV_STAT_DOUBLE_COUNTER_PRINT(OutOctetsLo,OutOctetsHi)
            PRV_STAT_COUNTER_PRINT(OutUnicasts)
            PRV_STAT_COUNTER_PRINT(Excessive)
            PRV_STAT_COUNTER_PRINT(OutMulticasts)
            PRV_STAT_COUNTER_PRINT(OutBroadcasts)
            PRV_STAT_COUNTER_PRINT(Single)
            PRV_STAT_COUNTER_PRINT(OutPause)
            PRV_STAT_COUNTER_PRINT(InPause)
            PRV_STAT_COUNTER_PRINT(Multiple)
            PRV_STAT_COUNTER_PRINT(InUndersize)
            PRV_STAT_COUNTER_PRINT(InFragments)
            PRV_STAT_COUNTER_PRINT(InOversize)
            PRV_STAT_COUNTER_PRINT(InJabber)
            PRV_STAT_COUNTER_PRINT(InRxErr)
            PRV_STAT_COUNTER_PRINT(InFCSErr)
            PRV_STAT_COUNTER_PRINT(Collisions)
            PRV_STAT_COUNTER_PRINT(Late)
            /* Bank 1 */
            PRV_STAT_COUNTER_PRINT(InDiscards)
            PRV_STAT_COUNTER_PRINT(InFiltered)
            PRV_STAT_COUNTER_PRINT(InAccepted)
            PRV_STAT_COUNTER_PRINT(InBadAccepted)
            PRV_STAT_COUNTER_PRINT(InGoodAvbClassA)
            PRV_STAT_COUNTER_PRINT(InGoodAvbClassB)
            PRV_STAT_COUNTER_PRINT(InBadAvbClassA)
            PRV_STAT_COUNTER_PRINT(InBadAvbClassB)
            PRV_STAT_COUNTER_PRINT(TCAMCounter0)
            PRV_STAT_COUNTER_PRINT(TCAMCounter1)
            PRV_STAT_COUNTER_PRINT(TCAMCounter2)
            PRV_STAT_COUNTER_PRINT(TCAMCounter3)
            PRV_STAT_COUNTER_PRINT(InDroppedAvbA)
            PRV_STAT_COUNTER_PRINT(InDroppedAvbB)
            PRV_STAT_COUNTER_PRINT(InDaUnknown)
            PRV_STAT_COUNTER_PRINT(InMGMT)
            PRV_STAT_COUNTER_PRINT(OutQueue0)
            PRV_STAT_COUNTER_PRINT(OutQueue1)
            PRV_STAT_COUNTER_PRINT(OutQueue2)
            PRV_STAT_COUNTER_PRINT(OutQueue3)
            PRV_STAT_COUNTER_PRINT(OutQueue4)
            PRV_STAT_COUNTER_PRINT(OutQueue5)
            PRV_STAT_COUNTER_PRINT(OutQueue6)
            PRV_STAT_COUNTER_PRINT(OutQueue7)
            PRV_STAT_COUNTER_PRINT(OutCutThrough)
            PRV_STAT_COUNTER_PRINT(reserved_19)
            PRV_STAT_COUNTER_PRINT(OutOctetsA)
            PRV_STAT_COUNTER_PRINT(OutOctetsB)
            PRV_STAT_COUNTER_PRINT(OutYel)
            PRV_STAT_COUNTER_PRINT(OutDroppedYel)
            PRV_STAT_COUNTER_PRINT(OutDiscards)
            PRV_STAT_COUNTER_PRINT(OutMGMT)
        }
    }
    cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
    if (stopNanoSeconds >= startNanoSeconds)
    {
        elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
        elapsedSeconds = (stopSeconds - startSeconds);
    }
    else
    {
        elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
        elapsedSeconds = (stopSeconds - startSeconds) - 1;
    }
    cpssOsPrintf("\nCreate forward time: [%d.%03d] secs ",
        elapsedSeconds, elapsedNanoSeconds / (1000*1000));

    cpssOsPrintf("\n");

#undef PRV_STAT_COUNTER_PRINT
#undef PRV_STAT_DOUBLE_COUNTER_PRINT

    return GT_OK;
}

/**
* @internal prvCpssMacPhyMibFastPrint function
* @endinternal
*
* @brief   Print all MIB counters on one or all ports.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - remote port number - 0 for all ports
*
* @retval GT_OK                    - on successfull write
* @retval GT_NOT_FOUND             - on remote MAC port not found
*/
GT_STATUS prvCpssMacPhyMibFastPrint
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MAC_COUNTER_SET_STC    portMacCounterSetArray;    /* Temporary result of counter read */
    GT_U32 minPhyPorts = 12;
    GT_U32 maxPhyPorts = 60;
    GT_BOOL         captureIsDone;
    GT_U32          i;
    GT_U32          startSeconds;
    GT_U32          startNanoSeconds;
    GT_U32          stopSeconds;
    GT_U32          stopNanoSeconds;
    GT_U32          elapsedSeconds;
    GT_U32          elapsedNanoSeconds;


    if (portNum != 0)
    {
        minPhyPorts = portNum;
        maxPhyPorts = portNum + 1;
    }

    /* Loop on all SMI devices */
    cpssOsPrintf("\n+------------------------------+");
    cpssOsPrintf("\n| PHY   Port Name      Value   |");
    cpssOsPrintf("\n+------------------------------+");
    cpssOsTimeRT(&startSeconds, &startNanoSeconds);
    {

        /* Loop on all ports */
        for (portNum = minPhyPorts; portNum < maxPhyPorts; portNum++)
        {
            /* Trigger read of all counters for one port */
            rc = cpssDxChPortMacCountersCaptureTriggerSet(devNum, portNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        for (portNum = minPhyPorts; portNum < maxPhyPorts; portNum++)
        {
            i = 0;
            rc = cpssDxChPortMacCountersCaptureTriggerGet(devNum,portNum,&captureIsDone);
            while ((rc == GT_OK) &&
                    (!captureIsDone) &&
                    (i++ < 2000))
            {
                cpssOsTimerWkAfter(1);
                rc = cpssDxChPortMacCountersCaptureTriggerGet(devNum,portNum,&captureIsDone);
            }

            if(rc != GT_OK)
            {
                return rc;
            }
            if(!captureIsDone)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
            }

            rc = cpssDxChPortMacCountersCaptureOnPortGet(devNum,
                            portNum,
                            &portMacCounterSetArray);

#define PRV_STAT_COUNTER_PRINT(_counterName) \
            if (portMacCounterSetArray._counterName.l[0] != 0)      \
            {       \
                cpssOsPrintf("\n%5d%5d %s:\t%5u",                   \
                                (portNum-12) / 8,                   \
                                ((portNum-12) % 8) + 1,             \
                                #_counterName, \
                                portMacCounterSetArray._counterName.l[0]);      \
            }

            PRV_STAT_COUNTER_PRINT(goodOctetsRcv)
            PRV_STAT_COUNTER_PRINT(badOctetsRcv)
            PRV_STAT_COUNTER_PRINT(macTransmitErr)
            PRV_STAT_COUNTER_PRINT(goodPktsRcv)
            PRV_STAT_COUNTER_PRINT(badPktsRcv)
            PRV_STAT_COUNTER_PRINT(brdcPktsRcv)
            PRV_STAT_COUNTER_PRINT(mcPktsRcv)
            PRV_STAT_COUNTER_PRINT(pkts64Octets)
            PRV_STAT_COUNTER_PRINT(pkts65to127Octets)
            PRV_STAT_COUNTER_PRINT(pkts128to255Octets)
            PRV_STAT_COUNTER_PRINT(pkts256to511Octets)
            PRV_STAT_COUNTER_PRINT(pkts512to1023Octets)
            PRV_STAT_COUNTER_PRINT(pkts1024tomaxOoctets)
            PRV_STAT_COUNTER_PRINT(goodOctetsSent)
            PRV_STAT_COUNTER_PRINT(goodPktsSent)
            PRV_STAT_COUNTER_PRINT(excessiveCollisions)
            PRV_STAT_COUNTER_PRINT(mcPktsSent)
            PRV_STAT_COUNTER_PRINT(brdcPktsSent)
            PRV_STAT_COUNTER_PRINT(unrecogMacCntrRcv)
            PRV_STAT_COUNTER_PRINT(fcSent)
            PRV_STAT_COUNTER_PRINT(goodFcRcv)
            PRV_STAT_COUNTER_PRINT(dropEvents)
            PRV_STAT_COUNTER_PRINT(undersizePkts)
            PRV_STAT_COUNTER_PRINT(fragmentsPkts)
            PRV_STAT_COUNTER_PRINT(oversizePkts)
            PRV_STAT_COUNTER_PRINT(jabberPkts)
            PRV_STAT_COUNTER_PRINT(macRcvError)
            PRV_STAT_COUNTER_PRINT(badCrc)
            PRV_STAT_COUNTER_PRINT(collisions)
            PRV_STAT_COUNTER_PRINT(lateCollisions)
            PRV_STAT_COUNTER_PRINT(badFcRcv)
            PRV_STAT_COUNTER_PRINT(ucPktsRcv)
            PRV_STAT_COUNTER_PRINT(ucPktsSent)
            PRV_STAT_COUNTER_PRINT(multiplePktsSent)
            PRV_STAT_COUNTER_PRINT(deferredPktsSent)
            PRV_STAT_COUNTER_PRINT(pkts1024to1518Octets)
            PRV_STAT_COUNTER_PRINT(pkts1519toMaxOctets)

         }
    }
    cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
    if (stopNanoSeconds >= startNanoSeconds)
    {
        elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
        elapsedSeconds = (stopSeconds - startSeconds);
    }
    else
    {
        elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
        elapsedSeconds = (stopSeconds - startSeconds) - 1;
    }
    cpssOsPrintf("\nCaptured read time: [%d.%03d] secs ",
        elapsedSeconds, elapsedNanoSeconds / (1000*1000));

    cpssOsPrintf("\n");

#undef PRV_STAT_COUNTER_PRINT

    return GT_OK;
}



