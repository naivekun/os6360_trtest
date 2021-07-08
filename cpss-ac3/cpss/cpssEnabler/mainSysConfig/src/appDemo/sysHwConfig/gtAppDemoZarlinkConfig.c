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
* @file gtAppDemoZarlinkConfig.c
*
* @brief APIs for Zarlink device.
*
*/
#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>

#define ZARLINK_DEBUG

#if (defined _linux)

typedef enum
{
    APP_DEMO_ZARLINK_FREQ_10M_E,
    APP_DEMO_ZARLINK_FREQ_25M_E,
    APP_DEMO_ZARLINK_FREQ_31_25M_E,
    APP_DEMO_ZARLINK_FREQ_39_0625M_E,
    APP_DEMO_ZARLINK_FREQ_50M_E,
    APP_DEMO_ZARLINK_FREQ_62_5M_E,
    APP_DEMO_ZARLINK_FREQ_64_453125M_E,
    APP_DEMO_ZARLINK_FREQ_78_125M_E,
    APP_DEMO_ZARLINK_FREQ_100M_E,
    APP_DEMO_ZARLINK_FREQ_125M_E
}APP_DEMO_ZARLINK_FREQ_E;


/*******************************************************************************
 * Global variables
 ******************************************************************************/
#if (!defined ASIC_SIMULATION)
extern GT_STATUS hwIfTwsiReadByte
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
);

extern GT_STATUS hwIfTwsiWriteByte
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
);
#endif
/*****************************************************************************
* Local variables
******************************************************************************/
GT_U32 appDemoZarlinkFreqInitSeq[10][8] = 
         {{0x9C,  0x40,  0,    0xFA,  0,  0x1,  0,  0x1},
          {0x9C,  0x40,  0x2,  0x71,  0,  0x1,  0,  0x1},
          {0x61,  0xA8,  0x4,  0xE2,  0,  0x1,  0,  0x1},
          {0xF4,  0x24,  0x2,  0x71,  0,  0x1,  0,  0x1},
          {0x9C,  0x40,  0x4,  0xE2,  0,  0x1,  0,  0x1},
          {0x61,  0xA8,  0x9,  0xC4,  0,  0x1,  0,  0x1},
          {0x3d,  0x9,   0x10, 0x1D,  0,  0x1,  0,  0x1},
          {0x61,  0xA8,  0xC,  0x35,  0,  0x1,  0,  0x1},
          {0x9C,  0x40,  0x9,  0xC4,  0,  0x1,  0,  0x1},
          {0x9C,  0x40,  0xC,  0x35,  0,  0x1,  0,  0x1}};

/*****************************************************************************
* Public API implementation
******************************************************************************/
/**
* @internal appDemoZarlinkPageSelect function
* @endinternal
*
* @brief   Sets register page for ZARLINK
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] offset                   - register offset
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkPageSelect
(
    IN GT_U32 devSlvId,
    IN GT_U32 offset
)
{
#if (!defined ASIC_SIMULATION)

    GT_STATUS rc;
    GT_U32 pageSelectReg = 0x7f;
    GT_U32 pageNum;

    if(offset >= 0 && offset <= 0x7f)
    {
        pageNum = 0;
    }
    else if(offset >= 0x80 && offset <= 0xff)
    {
        pageNum = 1;
    }
    else if(offset >= 0x100 && offset <= 0x17f)
    {
        pageNum = 2;
    }
    else if(offset >= 0x180 && offset <= 0x1ff)
    {
        pageNum = 3;
    }
    else if(offset >= 0x200 && offset <= 0x27f)
    {
        pageNum = 4;
    }
    else if(offset >= 0x280 && offset <= 0x2ff)
    {
        pageNum = 5;
    }
    else
    {
        return GT_OUT_OF_RANGE;
    }

    rc = hwIfTwsiWriteByte(devSlvId, pageSelectReg, pageNum);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}



/**
* @internal appDemoZarlinkRead function
* @endinternal
*
* @brief   Reads from ZARLINK
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] dataLen                  - number of bytes to read (APPLICABLE RANGES: 0..255)
* @param[in] offset                   - register offset
*
* @param[out] dataPtr                  - (pointer to) output data
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkRead
(
    IN GT_U32 devSlvId,
    IN GT_U32 dataLen,
    IN GT_U32 offset,
    OUT GT_U32 *dataPtr
)
{
#if (!defined ASIC_SIMULATION)

    GT_STATUS rc;
    GT_U32 i;

    if(dataLen < 1)
    {
        return GT_BAD_PARAM;
    }

    for(i = 0; i < dataLen; i++)
    {
        rc = appDemoZarlinkPageSelect(devSlvId, (offset + i));
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = hwIfTwsiReadByte(devSlvId, ((offset + i) & 0x7F), &dataPtr[i]);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif

}


/**
* @internal appDemoZarlinkReadByte function
* @endinternal
*
* @brief   Reads byte from ZARLINK
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] offset                   - register offset
*
* @param[out] dataPtr                  - (pointer to) output data
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkReadByte
(
    IN GT_U32 devSlvId,
    IN GT_U32 offset,
    OUT GT_U32 *dataPtr
)
{
#if (!defined ASIC_SIMULATION)

    GT_STATUS rc;


    rc = appDemoZarlinkPageSelect(devSlvId,offset);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = hwIfTwsiReadByte(devSlvId, (offset & 0x7F), dataPtr);
    return rc;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkWrite function
* @endinternal
*
* @brief   Reads from ZARLINK
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] dataLen                  - number of bytes to read
* @param[in] offset                   - register offset
* @param[in] dataPtr                  - (pointer to) data to be written
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkWrite
(
    IN GT_U32 devSlvId,
    IN GT_U32 dataLen,
    IN GT_U32 offset,
    IN GT_U32  *dataPtr
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;
    GT_U32 i;

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    for(i = 0; i < dataLen; i++)
    {
        rc = appDemoZarlinkPageSelect(devSlvId, (offset + i));
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = hwIfTwsiWriteByte(devSlvId, ((offset + i) & 0x7F), dataPtr[i]);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkWriteByte function
* @endinternal
*
* @brief   Write byte to ZARLINK
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] offset                   - register offset
* @param[in] data                     -  to be written
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkWriteByte
(
    IN GT_U32 devSlvId,
    IN GT_U32 offset,
    IN GT_U32 data
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;

    rc = appDemoZarlinkPageSelect(devSlvId,offset);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = hwIfTwsiWriteByte(devSlvId, (offset & 0x7F), data);
    return rc;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkDump function
* @endinternal
*
* @brief   Dump register function
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] dataLen                  - number of bytes to read
* @param[in] offset                   - register offset
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkDump
(
    IN GT_U32 devSlvId,
    IN GT_U32 dataLen,
    IN GT_U32 offset
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;
    GT_U32 data[256] = {0};
    GT_U32 i;

    if(dataLen > 255)
    {
        return GT_OUT_OF_RANGE;
    }

    rc = appDemoZarlinkRead(devSlvId,dataLen,offset, data);
    if(GT_OK != rc)
    {
        return rc;
    }

#ifdef ZARLINK_DEBUG
    for(i = 0; i < dataLen; i++)
    {
        osPrintf("data[%d] = 0x%x\r\n", i, data[i]);
    }
#endif

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkRefFreqSet function
* @endinternal
*
* @brief   Configures the input frequency parameters.
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] refClkSrc                - selects which ref clock is configured.
*                                      (APPLICABLE RANGES: 0..10)
* @param[in] freq                     - input reference frequency in Hz should be selected
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkRefFreqSet
(
    IN GT_U32 devSlvId,
    IN GT_U8  refClkSrc,
    IN APP_DEMO_ZARLINK_FREQ_E freq
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;   /* return code */
    GT_U32 regAddr; /* zarlink register address */

    if(refClkSrc > 10)
    {
        return GT_BAD_PARAM;
    }

    regAddr = 0x80 + (8 * refClkSrc);

    rc = appDemoZarlinkWrite(devSlvId, 8, regAddr, appDemoZarlinkFreqInitSeq[freq]);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkRefClkSelect function
* @endinternal
*
* @brief   Selects manualy which ref clk is used by the DPLL.
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] dpllSrc                  - selects which DPLL is configured.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] refClkSrc                - selects which ref clock is configured.
*                                      (APPLICABLE RANGES: 0..10)
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkRefClkSelect
(
    IN GT_U32 devSlvId,
    IN GT_U8  dpllSrc,
    IN GT_U8  refClkSrc
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;           /* return code */
    GT_U32 regAddr;         /* ZARLINK register address */
    GT_U32 regValue;        /* ZARLINK register value */

    if(refClkSrc > 10)
    {
        return GT_BAD_PARAM;
    }

    switch(dpllSrc)
    {
        case 0:
            regAddr = 0x103;
            break;
        case 1:
            regAddr = 0x123;
            break;
        default:
            return GT_BAD_PARAM;            
    }

    regValue = 2;
    regValue |= refClkSrc << 4;

    rc = appDemoZarlinkWrite(devSlvId, 1, regAddr, &regValue);

    return rc;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal appDemoZarlinkDpllStatusGet function
* @endinternal
*
* @brief   Reads the Lock and holdover indication of the DPLLs
*
* @param[in] devSlvId                 - ZARLINK I2C address.
* @param[in] dpllSrc                  - selects which DPLL is configured.
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] dpllHoldoverPtr          - (pointer to) holdover status of DPLL
* @param[out] dpllLockPtr              - (pointer to) lock status of DPLL
*                                       GT_OK
*/
GT_STATUS appDemoZarlinkDpllStatusGet
(
    IN  GT_U32 devSlvId,
    IN  GT_U8  dpllSrc,
    OUT GT_BOOL  *dpllHoldoverPtr,
    OUT GT_BOOL  *dpllLockPtr
)
{
#if (!defined ASIC_SIMULATION)
    GT_STATUS rc;           /* return code */
    GT_U32 regValue;        /* ZARLINK register value */
    GT_U32 tmpValue;        /* ZARLINK register value */
    
    CPSS_NULL_PTR_CHECK_MAC(dpllHoldoverPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpllLockPtr);

    if(dpllSrc > 1)
    {
        return GT_BAD_PARAM;            
    }

    regValue = 0x1;
    rc = appDemoZarlinkWrite(devSlvId, 1, 0x11, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    /*cpssOsTimerWkAfter(10);*/

    regValue = 0x0;
    rc = appDemoZarlinkWrite(devSlvId, 1, 0x180, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    /*cpssOsTimerWkAfter(10);*/

    rc = appDemoZarlinkWrite(devSlvId, 1, 0x11, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    
    /* sleep */
    cpssOsTimerWkAfter(25);
    rc = appDemoZarlinkRead(devSlvId, 1, 0x180, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    tmpValue = U32_GET_FIELD_MAC(regValue, (dpllSrc<<2), 1);
    *dpllHoldoverPtr = BIT2BOOL_MAC(tmpValue);

    tmpValue = U32_GET_FIELD_MAC(regValue, (1 + (dpllSrc<<2)), 1);
    *dpllLockPtr = BIT2BOOL_MAC(tmpValue);

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


#endif


