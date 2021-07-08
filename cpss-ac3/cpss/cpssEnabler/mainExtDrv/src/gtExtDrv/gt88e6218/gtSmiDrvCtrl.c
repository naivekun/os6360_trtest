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
* @file gtSmiDrvCtrl.c
*
* @brief API implementation for SMI facilities.
*
*/

/*Includes*/

#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>

#include <private/88e6218/gt88e6218Reg.h> 
#include <private/88e6218/gtCore.h>

#define GT_REG_ETHER_SMI_REG (UNIMAC_BASE_REG_ADDR | UNIMAC_SMI_REG)
#define SMI_TIMEOUT_COUNTER  1000

/* Access to registers (Read/Write) */
#define GT_REG_WRITE(reg, data) *(volatile GT_U32*)(reg) = data
#define GT_REG_READ(reg)        *(volatile GT_U32*)(reg)


/**
* @internal extDrvDirectSmiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *value
)
{
    register GT_U32 timeOut;
    register GT_U32 smiReg;

    /* not busy */
    smiReg =  (devSlvId << 16) | (SMI_OP_CODE_BIT_READ) | (regAddr << 21);
    
    /* first check that it is not busy */
    for (timeOut = SMI_TIMEOUT_COUNTER; ; timeOut--)
    {
        if ((GT_REG_READ(GT_REG_ETHER_SMI_REG) & SMI_STAT_BUSY) == 0)
            break;
        
        if (0 == timeOut)
            return GT_FAIL;
    }
    
    GT_REG_WRITE(GT_REG_ETHER_SMI_REG, smiReg);
    
    for (timeOut = SMI_TIMEOUT_COUNTER; ; timeOut--)
    {
        smiReg = GT_REG_READ(GT_REG_ETHER_SMI_REG);
        
        if (smiReg & SMI_STAT_READ_VALID)
            break;
        
        if (0 == timeOut)
            return GT_FAIL;
    }
    
    *value = (GT_U32)(smiReg & 0xffff);    
    
    return GT_OK;
}

/**
* @internal extDrvDirectSmiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*                                      dataPtr    - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    register GT_U32     timeOut;
    GT_U32              smiReg;
    
    smiReg = (devSlvId << 16) | (regAddr << 21) | (value & 0xffff);    

    /* first check that it is not busy */   
    for (timeOut = SMI_TIMEOUT_COUNTER; ; timeOut--)
    {
        if ((GT_REG_READ(GT_REG_ETHER_SMI_REG) & SMI_STAT_BUSY) == 0)
            break;
            
        if (timeOut == 0)
            return GT_FAIL;
    }
    
    /* not busy */
    GT_REG_WRITE(GT_REG_ETHER_SMI_REG, smiReg);   
    
    return GT_OK;
}



#define DEBUG_ONLY
#ifdef  DEBUG_ONLY

#include <stdio.h>
GT_STATUS smidwr
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS rc;
    rc = extDrvDirectSmiWriteReg(devSlvId,regAddr,value);
    printf("write Direct SMI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}


GT_STATUS smidrd
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr
)
{
    GT_U32 value = 0;
    GT_STATUS rc;
    rc = extDrvDirectSmiReadReg(devSlvId,regAddr,&value);
    printf("read Direct SMI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}

#endif /* DEBUG_ONLY */



