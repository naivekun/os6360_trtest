#include <Copyright.h>
/**
********************************************************************************
* @file gtHwAccess.c
*
* @brief Functions definition for HW accessing PHY, Serdes and Switch registers.
*
*/
/********************************************************************************
* gtHwAccess.c
*
* DESCRIPTION:
*       Functions definition for HW accessing PHY, Serdes and Switch registers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtHwAccess.h>
#include <gtSem.h>
#include <gtUtils.h>
#include <msApiInternal.h>

#define USE_SMI_MULTI_ADDR

/**
* @internal globalProtection function
* @endinternal
*
* @brief   global protection : sych between threads and the ISR operations.
*
* @param[in] dev                      - (pointer to) the device driver info.
* @param[in] startProtection          - indication to start protection.
*                                      GT_TRUE  - start the protection
*                                      GT_FALSE - end   the protection
* @param[in] cookiePtr                - (pointer to) cookie that is internally set and used.
*                                      the cookie must not be changed by the caller between
*                                      calls for 'start' and 'stop'
*                                       None
*/
static void globalProtection
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         startProtection,
    IN  GT_32           *cookiePtr
)
{
    if(dev->globalProtection)
    {
        dev->globalProtection(dev,startProtection,cookiePtr);
    }
}

/* Definition for Multi Address Mode */
#define PRV_CPSS_QD_REG_SMI_COMMAND         0x0
#define PRV_CPSS_QD_REG_SMI_DATA            0x1

/* Bit definition for PRV_CPSS_QD_REG_SMI_COMMAND */
#define QD_SMI_BUSY                0x8000
#define QD_SMI_MODE                0x1000
#define QD_SMI_MODE_BIT            12
#define QD_SMI_FUNC_BIT            13
#define QD_SMI_FUNC_SIZE            2
#define QD_SMI_OP_BIT              10
#define QD_SMI_OP_SIZE              2
#define QD_SMI_DEV_ADDR_BIT         5
#define QD_SMI_DEV_ADDR_SIZE        5
#define QD_SMI_REG_ADDR_BIT         0
#define QD_SMI_REG_ADDR_SIZE        5

#define QD_SMI_CLAUSE45            0
#define QD_SMI_CLAUSE22            1

#define QD_SMI_WRITE_ADDR          0x00
#define QD_SMI_WRITE               0x01
#define QD_SMI_READ_22             0x02
#define QD_SMI_READ_45             0x03
#define QD_SMI_READ_INC            0x02

#ifdef USE_SMI_MULTI_ADDR
static GT_STATUS qdMultiAddrRead (GT_CPSS_QD_DEV* dev, GT_U8 phyAddr , GT_U8 regAddr, OUT GT_U16* value,GT_CPSS_CONTEXT_ENT  context);
static GT_STATUS qdMultiAddrWrite (GT_CPSS_QD_DEV* dev, GT_U8 phyAddr , GT_U8 regAddr, GT_U16 value,GT_CPSS_CONTEXT_ENT  context);
#endif

/****************************************************************************/
/* Switch Any registers direct R/W functions.                                    */
/****************************************************************************/

/**
* @internal internal_prvCpssDrvHwSetAnyReg function
* @endinternal
*
* @brief   This function directly writes to a switch's register.
*
* @param[in] devAddr                  - device register.
* @param[in] regAddr                  - The register's address.
* @param[in] data                     - The  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_prvCpssDrvHwSetAnyReg
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_U8                 devAddr,
    IN  GT_U8                 regAddr,
    IN  GT_U16                data,
    IN  GT_CPSS_CONTEXT_ENT   context
)
{
    GT_STATUS   retVal;
    GT_32       protectionCookie;

    PRV_CPSS_DBG_INFO(("Write to switch register: devAddr 0x%x, regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("data 0x%x.\n",data));

    globalProtection(dev,GT_TRUE,&protectionCookie);
    prvCpssDrvGtSemTake(dev,dev->multiAddrSem,PRV_CPSS_OS_WAIT_FOREVER);

#ifdef USE_SMI_MULTI_ADDR
    retVal = qdMultiAddrWrite(dev,devAddr,regAddr,data,context);
#else
    retVal = dev->fgtWriteMii(dev,devAddr,regAddr,data,context);
#endif

    prvCpssDrvGtSemGive(dev,dev->multiAddrSem);
    globalProtection(dev,GT_FALSE,&protectionCookie);

    return retVal;
}

GT_STATUS prvCpssDrvHwSetAnyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
)
{
    return internal_prvCpssDrvHwSetAnyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}

/**
* @internal internal_prvCpssDrvHwGetAnyReg function
* @endinternal
*
* @brief   This function directly reads a switch's register.
*
* @param[in] devAddr                  - device register.
* @param[in] regAddr                  - The register's address.
*
* @param[out] data                     - The read register's data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_prvCpssDrvHwGetAnyReg
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_U8                 devAddr,
    IN  GT_U8                 regAddr,
    OUT GT_U16                *data,
    IN  GT_CPSS_CONTEXT_ENT   context
)
{
    GT_STATUS   retVal;
    GT_32       protectionCookie;

    globalProtection(dev,GT_TRUE,&protectionCookie);
    prvCpssDrvGtSemTake(dev,dev->multiAddrSem,PRV_CPSS_OS_WAIT_FOREVER);

#ifdef USE_SMI_MULTI_ADDR
    retVal = qdMultiAddrRead(dev,devAddr,regAddr,data,context);
#else
    retVal = dev->fgtReadMii(dev,devAddr,regAddr,data,context);
#endif

    prvCpssDrvGtSemGive(dev,dev->multiAddrSem);
    globalProtection(dev,GT_FALSE,&protectionCookie);

    PRV_CPSS_DBG_INFO(("Read register: devAddr 0x%x, regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}

GT_STATUS prvCpssDrvHwGetAnyReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_U8    devAddr,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    return internal_prvCpssDrvHwGetAnyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}



/**
* @internal prvCpssDrvHwSetAnyRegField function
* @endinternal
*
* @brief   This function writes to specified field in a switch's register.
*
* @param[in] devAddr                  - Device Address to write the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 1. The sum of fieldOffset & fieldLength parameters must be smaller-
*       equal to 16.
*
*/
GT_STATUS prvCpssDrvHwSetAnyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
    GT_STATUS   retVal;

    if(fieldOffset == 0 && fieldLength == 16)
    {
        tmpData = 0;/* ignored any way , so bypass the 'read register' */
    }
    else
    {
        retVal =  prvCpssDrvHwGetAnyReg(dev,devAddr,regAddr,&tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    PRV_CPSS_CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    PRV_CPSS_DBG_INFO(("Write to devAddr(%d): regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    retVal = prvCpssDrvHwSetAnyReg(dev,devAddr,regAddr,tmpData);

    return retVal;
}

/**
* @internal prvCpssDrvHwGetAnyRegField function
* @endinternal
*
* @brief   This function reads a specified field from a switch's register.
*
* @param[in] devAddr                  - device address to read the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to read.
*
* @param[out] data                     - The read register field.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 1. The sum of fieldOffset & fieldLength parameters must be smaller-
*       equal to 16.
*
*/
GT_STATUS prvCpssDrvHwGetAnyRegField
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_U8    devAddr,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
    GT_STATUS   retVal;

    retVal =  prvCpssDrvHwGetAnyReg(dev,devAddr,regAddr,&tmpData);
    if (retVal != GT_OK)
        return retVal;

    PRV_CPSS_CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    PRV_CPSS_DBG_INFO(("Read from deviceAddr(%d): regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/****************************************************************************/
/* SMI PHY Registers indirect R/W functions.                                         */
/****************************************************************************/

/**
* @internal prvCpssDrvHwSetSMIC45PhyReg function
* @endinternal
*
* @brief   This function indirectly write internal SERDES register through SMI PHY command.
*
* @param[in] devAddr                  - The device address.
* @param[in] phyAddr                  - The PHY address.
* @param[in] regAddr                  - The register address.
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvHwSetSMIC45PhyReg
(
    IN GT_CPSS_QD_DEV *dev,
    IN GT_U8    devAddr,
    IN GT_U8    phyAddr,
    IN GT_U16   regAddr,
    IN GT_U16   data
)
{
    unsigned int timeOut;
    GT_U16 smiReg;

    PRV_CPSS_DBG_INFO(("Writing SMI Phy register.\n"));

    /* first check that it is not busy */
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);


    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, regAddr) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing SMI Phy Data register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE_ADDR << QD_SMI_OP_BIT) |
            (devAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE45 << QD_SMI_MODE_BIT);

    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing Phy Command register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);


    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, data) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing SMI Phy Data Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE << QD_SMI_OP_BIT) |
            (devAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE45 << QD_SMI_MODE_BIT);

    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing Phy Command register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvHwGetSMIC45PhyReg function
* @endinternal
*
* @brief   This function indirectly read internal SERDES register through SMI PHY command.
*
* @param[in] devAddr                  - The device address.
* @param[in] phyAddr                  - The PHY address.
* @param[in] regAddr                  - The register address to read.
*
* @param[out] data                     - The storage where register  to be saved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvHwGetSMIC45PhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           phyAddr,
    IN  GT_U16          regAddr,
    OUT GT_U16          *data
)
{
    unsigned int timeOut;
    GT_U16 smiReg;
    GT_U16 tmp;

    PRV_CPSS_DBG_INFO(("Read Phy register while PPU Enabled\n"));

     /* first check that it is not busy */
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);


    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, regAddr) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing SMI Phy Data register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE_ADDR << QD_SMI_OP_BIT) |
            (devAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE45 << QD_SMI_MODE_BIT);

    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing Phy Command register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_READ_45 << QD_SMI_OP_BIT) |
            (devAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE45 << QD_SMI_MODE_BIT);

    if(prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing Phy Command register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);


    if(prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, &tmp) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing SMI Phy Data register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *data = tmp;

    return GT_OK;
}

/**
* @internal internal_prvCpssDrvHwSetSMIPhyReg function
* @endinternal
*
* @brief   This function indirectly write internal PHY register through SMI
*         PHY command.
* @param[in] devAddr                  - The PHY address.
* @param[in] regAddr                  - The register address.
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_prvCpssDrvHwSetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_U8                 devAddr,
    IN  GT_U8                 regAddr,
    IN  GT_U16                data,
    IN  GT_CPSS_CONTEXT_ENT   context

)
{
    unsigned int timeOut;
    GT_U16 smiReg;

    PRV_CPSS_DBG_INFO(("Writing SMI Phy register.\n"));

    /* first check that it is not busy */
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(internal_prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg,context) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Writing SMI Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);


    if(internal_prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, data,context) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing SMI Phy Data register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    smiReg = QD_SMI_BUSY | (devAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE << QD_SMI_OP_BIT) |
            (regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(internal_prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg,context) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing Phy Command register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

GT_STATUS prvCpssDrvHwSetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
)
{
    return internal_prvCpssDrvHwSetSMIPhyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}

/**
* @internal internal_prvCpssDrvHwGetSMIPhyReg function
* @endinternal
*
* @brief   This function indirectly read internal PHY register through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
*
* @param[out] data                     - The storage where register date to be saved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_prvCpssDrvHwGetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_U8                 devAddr,
    IN  GT_U8                 regAddr,
    OUT GT_U16                *data,
    IN  GT_CPSS_CONTEXT_ENT   context
)
{
    unsigned int timeOut;
    GT_U16 smiReg;

    PRV_CPSS_DBG_INFO(("Read Phy register while PPU Enabled\n"));

    /* first check that it is not busy */
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(internal_prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg,context) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Reading Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Reading Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);

    smiReg =  QD_SMI_BUSY | (devAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_READ_22 << QD_SMI_OP_BIT) |
            (regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(internal_prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, smiReg,context) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(internal_prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_CMD, &smiReg,context) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Reading Phy register Failed\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            PRV_CPSS_DBG_INFO(("Reading Phy register Timed Out\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);

    if(internal_prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SMI_PHY_DATA, &smiReg,context) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Reading Phy register Failed\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *data = smiReg;

    return GT_OK;
}

GT_STATUS prvCpssDrvHwGetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            devAddr,
    IN  GT_U8            regAddr,
    OUT GT_U16           *data
)
{
    return internal_prvCpssDrvHwGetSMIPhyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}

/**
* @internal prvCpssDrvHwSetSMIPhyRegField function
* @endinternal
*
* @brief   This function indirectly write internal PHY specified register field through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
* @param[in] data                     - register date to be written.
*
* @retval GT_TRUE                  - on success
* @retval GT_FALSE                 - on error
*/
GT_STATUS prvCpssDrvHwSetSMIPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          data
)
{
    GT_U16      mask;
    GT_U16      tmpData;
    GT_STATUS   retVal;

    retVal =  prvCpssDrvHwGetSMIPhyReg(dev,devAddr,regAddr,&tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    PRV_CPSS_CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    PRV_CPSS_DBG_INFO(("Write to devAddr(%d): regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    retVal = prvCpssDrvHwSetSMIPhyReg(dev,devAddr,regAddr,tmpData);

    return retVal;
}

/**
* @internal prvCpssDrvHwGetSMIPhyRegField function
* @endinternal
*
* @brief   This function indirectly read internal PHY specified register field
*         through SMI PHY command.
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
*
* @param[out] data                     - The storage where register date to be saved.
*
* @retval GT_TRUE                  - on success
* @retval GT_FALSE                 - on error
*/
GT_STATUS prvCpssDrvHwGetSMIPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    OUT GT_U16          *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
    GT_STATUS   retVal;

    retVal =  prvCpssDrvHwGetSMIPhyReg(dev,devAddr,regAddr,&tmpData);
    if (retVal != GT_OK)
        return retVal;

    PRV_CPSS_CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    PRV_CPSS_DBG_INFO(("Read from deviceAddr(%d): regAddr 0x%x, ",
              devAddr,regAddr));
    PRV_CPSS_DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}

GT_STATUS prvCpssDrvHwSetPagedPhyRegField
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            portNum,
    IN  GT_U8            pageNum,
    IN  GT_U8            regAddr,
    IN  GT_U8            fieldOffset,
    IN  GT_U8            fieldLength,
    IN  GT_U16           data
)
{
    GT_STATUS retVal;
    retVal = prvCpssDrvHwSetSMIPhyReg(dev, portNum, 22, pageNum);
    if(retVal != GT_OK)
        return retVal;
    retVal = prvCpssDrvHwSetSMIPhyRegField(dev, portNum, regAddr, fieldOffset, fieldLength, data);
    return retVal;
}

GT_STATUS prvCpssDrvHwGetPagedPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          *data
)
{
    GT_STATUS retVal;
    retVal = prvCpssDrvHwSetSMIPhyReg(dev, portNum, 22, pageNum);
    if(retVal != GT_OK)
        return retVal;
    retVal = prvCpssDrvHwGetSMIPhyRegField(dev, portNum, regAddr, fieldOffset, fieldLength, data);
    return retVal;
}

static GT_STATUS internal_prvCpssDrvHwWritePagedPhyReg
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_U8                 portNum,
    IN  GT_U8                 pageNum,
    IN  GT_U8                 regAddr,
    IN  GT_U16                data,
    IN  GT_CPSS_CONTEXT_ENT   context
)
{
    GT_STATUS retVal;
    retVal = internal_prvCpssDrvHwSetSMIPhyReg(dev, portNum, 22, pageNum,context);
    if(retVal != GT_OK)
        return retVal;
    retVal = internal_prvCpssDrvHwSetSMIPhyReg(dev, portNum, regAddr, data,context);
    return retVal;
}

GT_STATUS prvCpssDrvHwWritePagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
)
{
    return internal_prvCpssDrvHwWritePagedPhyReg(dev,portNum,pageNum,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}


static GT_STATUS internal_prvCpssDrvHwReadPagedPhyReg
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_U8                portNum,
    IN  GT_U8                pageNum,
    IN  GT_U8                regAddr,
    OUT GT_U16               *data,
    IN  GT_CPSS_CONTEXT_ENT  context
)
{
    GT_STATUS retVal;
    retVal = internal_prvCpssDrvHwSetSMIPhyReg(dev, portNum, 22, pageNum,context);
    if(retVal != GT_OK)
        return retVal;
    retVal = internal_prvCpssDrvHwGetSMIPhyReg(dev, portNum, regAddr, data,context);
    return retVal;
}

GT_STATUS prvCpssDrvHwReadPagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
)
{
    return internal_prvCpssDrvHwReadPagedPhyReg(dev,portNum,pageNum,regAddr,data,GT_CPSS_CONTEXT_GENERIC);
}

/**
* @internal prvCpssDrvHwPhyReset function
* @endinternal
*
* @brief   This function performs softreset and waits until reset completion.
*
* @param[in] portNum                  - Port number to write the register for.
* @param[in] u16Data                  - data should be written into Phy control register.
*                                      if this value is 0xFF, normal operation occcurs (read,
*                                      update, and write back.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvHwPhyReset
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U8             portNum,
    IN  GT_U8             pageNum,
    IN  GT_U16            u16Data
)
{
    GT_U16 tmpData;
    GT_STATUS   retVal;
    GT_U32 retryCount;
    GT_BOOL    pd = GT_FALSE;

    /*if((retVal=prvCpssDrvHwGetSMIPhyReg(dev,portNum,0,&tmpData))
           != GT_OK)*/
    if((retVal=prvCpssDrvHwReadPagedPhyReg(dev, portNum, pageNum, 0,&tmpData))
       != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Reading Register failed\n"));
        return retVal;
    }

    if (tmpData & 0x800)
    {
        pd = GT_TRUE;
    }

    if (u16Data != 0xFF)
    {
        tmpData = u16Data;
    }

    /* Set the desired bits to 0. */
    if (pd)
    {
        tmpData |= 0x800;
    }
    else
    {
        if(((tmpData&0x4000)==0)||(u16Data==0xFF)) /* setting loopback do not set reset */
            tmpData |= 0x8000;
    }

    /*if((retVal=prvCpssDrvHwSetSMIPhyReg(dev,portNum,0,tmpData))
        != GT_OK)*/
    if((retVal=prvCpssDrvHwWritePagedPhyReg(dev, portNum, pageNum, 0,tmpData))
        != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Writing to register failed\n"));
        return retVal;
    }

    if (pd)
    {
        return GT_OK;
    }

    for (retryCount = 0x1000; retryCount > 0; retryCount--)
    {
        /*if((retVal=prvCpssDrvHwGetSMIPhyReg(dev,portNum,0,&tmpData)) != GT_OK)*/
        if((retVal=prvCpssDrvHwReadPagedPhyReg(dev, portNum, pageNum, 0,&tmpData)) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Reading register failed\n"));
            return retVal;
        }
        if ((tmpData & 0x8000) == 0)
            break;
    }

    if (retryCount == 0)
    {
        PRV_CPSS_DBG_INFO(("Reset bit is not cleared\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/****************************************************************************/
/* Internal functions.                                                      */
/****************************************************************************/
#ifdef ASIC_SIMULATION
static GT_U32 directAccessMode1 =  0;
void debug_set_directAccessMode1(GT_U32  newValue)
{
    if(directAccessMode1 == newValue)
    {
        return;
    }

    cpssOsPrintf("debug_set_directAccessMode1 : oldMode[%d] , newValue[%d]",
        directAccessMode1,newValue);

    directAccessMode1 = newValue;
}
void smemSohoDirectAccess
(
    IN  GT_U8    deviceNumber,
    IN  GT_U32   DevAddr,
    IN  GT_U32   RegAddr,
    IN  GT_U32 * memPtr,
    IN  GT_BOOL   doRead/*GT_TRUE - read , GT_FALSE - write*/
);
static GT_STATUS simSohoDirectAccessWrite
(
    GT_CPSS_QD_DEV* dev,
    GT_U8 phyAddr ,
    GT_U8 regAddr,
    GT_U16 value,
    GT_CPSS_CONTEXT_ENT  context
)
{
    GT_U32  _32_data;
    GT_U32  DevAddr,RegAddr;

    context = context;
    _32_data = value;
    DevAddr = phyAddr;
    RegAddr = regAddr;
    smemSohoDirectAccess(dev->devNum,DevAddr,RegAddr,&_32_data,GT_FALSE/*write*/);

    return GT_OK;
}
static GT_STATUS simSohoDirectAccessRead
(
    GT_CPSS_QD_DEV* dev,
    GT_U8 phyAddr ,
    GT_U8 regAddr,
    GT_U16 *value,
    GT_CPSS_CONTEXT_ENT  context
)
{
    GT_U32  _32_data;
    GT_U32  DevAddr,RegAddr;

    context = context;
    DevAddr = phyAddr;
    RegAddr = regAddr;
    smemSohoDirectAccess(dev->devNum,DevAddr,RegAddr,&_32_data,GT_TRUE/*read*/);
    *value = (GT_U16)_32_data;

    return GT_OK;
}
#endif

#ifdef USE_SMI_MULTI_ADDR
/**
* @internal qdMultiAddrRead function
* @endinternal
*
* @brief   This function reads data from a device in the secondary MII bus.
*
* @param[in] phyAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] value                    - The storage where register date to be saved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS qdMultiAddrRead
(
    GT_CPSS_QD_DEV* dev,
    GT_U8 phyAddr ,
    GT_U8 regAddr,
    OUT GT_U16* value,
    GT_CPSS_CONTEXT_ENT  context
)
{
    GT_U16 smiReg;
    unsigned int timeOut;

#ifdef ASIC_SIMULATION
    if(directAccessMode1 &&
       dev->devNum != 0/* initialized*/)
    {
        return simSohoDirectAccessRead(dev,phyAddr,regAddr,value,context);
    }
#endif

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if((dev->fgtReadMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_COMMAND, &smiReg,context)) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);

    smiReg =  QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_READ_22 << QD_SMI_OP_BIT) |
            (regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if((dev->fgtWriteMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_COMMAND, smiReg,context)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if((dev->fgtReadMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_COMMAND, &smiReg,context)) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);

    if((dev->fgtReadMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_DATA, &smiReg,context)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *value = smiReg;

    return GT_OK;
}

/**
* @internal qdMultiAddrWrite function
* @endinternal
*
* @brief   This function writes data to the device in the secondary MII bus.
*
* @param[in] phyAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] value                    - The data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS qdMultiAddrWrite (
    GT_CPSS_QD_DEV* dev,
    GT_U8 phyAddr ,
    GT_U8 regAddr,
    GT_U16 value,
    GT_CPSS_CONTEXT_ENT  context
)
{
    GT_U16 smiReg;
    unsigned int timeOut;

#ifdef ASIC_SIMULATION
    if(directAccessMode1 &&
       dev->devNum != 0/* initialized*/)
    {
        return simSohoDirectAccessWrite(dev,phyAddr,regAddr,value,context);
    }
#endif

    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if((dev->fgtReadMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_COMMAND, &smiReg,context)) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if(timeOut-- < 1 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (smiReg & QD_SMI_BUSY);

    if((dev->fgtWriteMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_DATA, value,context)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE << QD_SMI_OP_BIT) |
            (regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if((dev->fgtWriteMii(dev,dev->phyAddr,PRV_CPSS_QD_REG_SMI_COMMAND, smiReg,context)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}
#endif

/**** function called in ISR context ****/

GT_STATUS ISR_prvCpssDrvHwReadPagedPhyReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    pageNum,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    return internal_prvCpssDrvHwReadPagedPhyReg(dev,portNum,pageNum,regAddr,data,GT_CPSS_CONTEXT_ISR);
}

GT_STATUS ISR_prvCpssDrvHwWritePagedPhyReg
(
    IN GT_CPSS_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    pageNum,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    return internal_prvCpssDrvHwWritePagedPhyReg(dev,portNum,pageNum,regAddr,data,GT_CPSS_CONTEXT_ISR);
}

GT_STATUS ISR_prvCpssDrvHwGetAnyReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_U8    devAddr,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    return internal_prvCpssDrvHwGetAnyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_ISR);
}

GT_STATUS ISR_prvCpssDrvHwSetAnyReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_U8    devAddr,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    return internal_prvCpssDrvHwSetAnyReg(dev,devAddr,regAddr,data,GT_CPSS_CONTEXT_ISR);
}



