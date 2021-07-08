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
* @file gtTwsiDrvCtrl.c
*
* @brief API implementation for TWSI facilities.
*
*/

/*Includes*/

#include <gtExtDrv/drivers/gtTwsiDrvCtrl.h>

#include <private/88e6218/gt88e6218Reg.h> 
#include <private/88e6218/gtCore.h>

/* typedefs */
typedef enum _i2cBusStatus 
{
    I2C_OK,
    I2C_BUSY,
    I2C_TIMEOUT,
    I2C_ERROR =0xff

} I2C_BUS_STATUS;

typedef enum _i2cfreq      
{
    
    _100KHZ = 100000,
    _400KHZ = 400000

} I2C_FREQ;

typedef enum _i2cTransType
{
    I2C_WRITE,
    I2C_READ

} I2C_TRANS_TYPE;

/* Describes i2c transaction */
typedef struct _i2cTrans
{
    GT_U8           devId;       /* I2c slave ID                              */ 
    I2C_TRANS_TYPE  transType;   /* Read / Write transaction                  */
    GT_U8           *pData;      /* Pointer to array of chars (address / data)*/
    GT_U8           len;         /* pData array size (in chars).              */
    GT_BOOL         stop;        /* Indicates if stop bit is needed in the end 
                                    of the transaction                        */ 
} I2C_TRANS;            

/* Function declerations */
static I2C_BUS_STATUS      mvI2cInit(IN I2C_FREQ freq , IN GT_U32 inClk);
static I2C_BUS_STATUS      mvI2cMasterTrans(IN I2C_TRANS* i2cTrans);
static I2C_BUS_STATUS      mvI2cWaitNotBusy();

/* defines */
#define I2C_TIMEOUT_VALUE   0x2000000 /* 1 Second at 500 Mhz */
#define TWSI_SLAVE_FREQ     _100KHZ
#define FFX_CORE_FREQ       133.33e6
#define MAX_TIMEOUT         2


#define PRESCALAR_MULTIPLE_VALUE               5
#define LBU_CLK_DIV_ADD_VALUE                  1


/* The macro I2C_BUS_BUSY returns a non-zero number if the i2c bus is busy 
   or zero if the i2c bus isn't busy */  
#define I2C_BUS_BUSY                                                           \
        (MVREGREAD_CHAR(I2C_BASE_REG_ADDR | I2C_STATUS) & (I2C_SR_BUSY))

/* The macro IS_RX_ACK returns a non-zero number if non-ACK bit is recieved 
   from slave or zero if recieving ACK bit from memory slave*/  
#define IS_RX_ACK                                                              \
        (MVREGREAD_CHAR(I2C_BASE_REG_ADDR | I2C_STATUS) & (RX_SR_ACK))


/**
* @internal extDrvDirectTwsiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiInitDriver
(
    GT_VOID
)
{
    I2C_BUS_STATUS  i2cStatus;  
    i2cStatus = mvI2cInit(TWSI_SLAVE_FREQ, FFX_CORE_FREQ);
    if (i2cStatus != I2C_OK) return GT_FAIL;

    return GT_OK;
}


/**
* @internal extDrvDirectTwsiWaitNotBusy function
* @endinternal
*
* @brief   Wait for TWSI interface not BUSY
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiWaitNotBusy
(
    GT_VOID
)
{
    I2C_BUS_STATUS  i2cStatus;  
    i2cStatus = mvI2cWaitNotBusy();
    if (i2cStatus != I2C_OK) return GT_FAIL;

    return GT_OK;
}



/**
* @internal extDrvDirectTwsiMasterReadTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiMasterReadTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
)
{
    I2C_TRANS       i2cTrans;
    I2C_BUS_STATUS  i2cStatus;  

    i2cTrans.devId = devId;
    i2cTrans.len   = len;
    i2cTrans.pData = pData;
    i2cTrans.stop  = stop;
    i2cTrans.transType =  I2C_READ;
    i2cStatus = mvI2cMasterTrans(&i2cTrans);

    if (i2cStatus != I2C_OK) return GT_FAIL;

    return GT_OK;
}



/**
* @internal extDrvDirectTwsiMasterWriteTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiMasterWriteTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
)
{
    I2C_TRANS       i2cTrans;
    I2C_BUS_STATUS  i2cStatus;  

    i2cTrans.devId = devId;
    i2cTrans.len   = len;
    i2cTrans.pData = pData;
    i2cTrans.stop  = stop;
    i2cTrans.transType =  I2C_WRITE;
    i2cStatus = mvI2cMasterTrans(&i2cTrans);

    if (i2cStatus != I2C_OK) return GT_FAIL;

    return GT_OK;
}


/**
* @internal mvI2cInit function
* @endinternal
*
*/
static I2C_BUS_STATUS mvI2cInit(I2C_FREQ freq , GT_U32 inClk)
{
    unsigned int   i2cTimeOut=0;
    unsigned short clkPrescalar = 0;
    unsigned int pClkDiv;
    
    /* Checking for correct inputs */
    if ( (freq == 0) || (inClk == 0) ) 
    {
        return I2C_ERROR;
    }
    
    /*Set <CLKDiv> to SYSCLK/2*/
    MV_SET_REG_BITS(LBU_BASE_REG_ADDR+LBU_CLK_DIV,BIT0);

    MV_RESET_REG_BITS(GPIO_BASE_REG_ADDR + GPIO_SELECT_8_15 , BIT15 | BIT14 |
                      BIT13 | BIT12);
    
    /* Calculating clock prescalar register value and write it */
    /* I2C_REG_READ(LBU_CLK_DIV , &pClkDiv);//From original Driver!!*/
    
    /*Changed By Amit*/
    MV_REG_READ(LBU_BASE_REG_ADDR | LBU_CLK_DIV, &pClkDiv);
    
    clkPrescalar = (inClk) / ( (pClkDiv + LBU_CLK_DIV_ADD_VALUE) * 
                               (PRESCALAR_MULTIPLE_VALUE) * (freq) );

    MV_REG_WRITE_SHORT(I2C_BASE_REG_ADDR | I2C_PRER , clkPrescalar);
    /* Loading i2c global control register */
    MV_REG_WRITE_CHAR(I2C_BASE_REG_ADDR | I2C_GLOBAL_CTRL ,
                       (I2C_GCR_MODE_CTRL_I2C | I2C_GCR_MEM_SEL ));

    /*Checking if the i2c bus is busy */
    while(I2C_BUS_BUSY)
    {
        /* Checking i2c time out */
        if(i2cTimeOut++ == I2C_TIMEOUT_VALUE)
            return I2C_TIMEOUT;
    }      

    return I2C_OK; 
}


/**
* @internal mvI2cWaitNotBusy function
* @endinternal
*
*/
static I2C_BUS_STATUS mvI2cWaitNotBusy()
{
    unsigned int i2cTimeOut=0;

    /*Checking if the i2c bus is busy */
    while(I2C_BUS_BUSY)
    {
        /* Checking i2c time out */
        if(i2cTimeOut++ == I2C_TIMEOUT_VALUE)
            return I2C_TIMEOUT;
    }      

    return I2C_OK; 
}

/**
* @internal mvI2cMasterTrans function
* @endinternal
*
*/
static I2C_BUS_STATUS mvI2cMasterTrans(I2C_TRANS *pI2cTrans)
{

    unsigned int    timeout,offset;
    unsigned char   devIdAndOperation;

    if ((pI2cTrans->transType == I2C_WRITE) && (I2C_BUS_BUSY)) 
    {
        return I2C_BUSY;
    }
     
    /* Setting start bit */
    MV_SET_REG_BITS(I2C_BASE_REG_ADDR | I2C_CTRL , I2C_CR_START);
    
    /* Preparing slave ID + operation (read/write) */
    devIdAndOperation = ((pI2cTrans->devId<<1) | (pI2cTrans->transType));
    
    /* Writing Slave ID + operation (read/write) to data register */ 
    MV_REG_WRITE_CHAR(I2C_BASE_REG_ADDR | I2C_DATA , devIdAndOperation);
    
    /* Waiting for RX acknowledge */
    timeout = 0;
    while (IS_RX_ACK)
    {
        /* Checking i2c time out */
        if(timeout++ == I2C_TIMEOUT_VALUE)
            return I2C_TIMEOUT;
        
        return I2C_OK;
    }
    
    /* Writing / Reading data */
    for (offset = 0 ; offset < (pI2cTrans->len) ; offset++) 
    {
        switch (pI2cTrans->transType) 
        {
            case I2C_WRITE: /* Write case */
            
                /* The last phase of writing data transaction includes setting 
                   stop bit before writing the final data */
                if ((pI2cTrans->stop == GT_TRUE) && (offset == pI2cTrans->len - 1))
                {   
                    /* Setting stop bit */
                    MV_SET_REG_BITS(I2C_BASE_REG_ADDR | I2C_CTRL , I2C_CR_STOP); 

                }                   
                MV_REG_WRITE_CHAR(I2C_BASE_REG_ADDR | I2C_DATA , (*(pI2cTrans->pData + offset)));

                /* Waiting for RX acknowledge */
                timeout = 0;
                while(IS_RX_ACK)
                {
                    /* Checking i2c time out */
                    if(timeout++ == I2C_TIMEOUT_VALUE)
                        return I2C_TIMEOUT;
                }      
                
                break;

            case I2C_READ:
                
                /* The last phase of reading data transaction includes setting 
                   stop bit before reading the final data */
                if (offset == pI2cTrans->len - 1)
                {   
                    /* Setting stop bit */
                    MV_SET_REG_BITS(I2C_BASE_REG_ADDR | I2C_CTRL , I2C_CR_STOP);
                    
                }   
                /* Reading data from slave into pData array */
                MV_REG_READ_CHAR(I2C_BASE_REG_ADDR | I2C_DATA , (pI2cTrans->pData + offset));
                
                /* Waiting for RX acknowledge */
                timeout = 0;
                while(IS_RX_ACK)
                {
                    /* Checking i2c time out */
                    if(timeout++ == I2C_TIMEOUT_VALUE)
                        return I2C_TIMEOUT;

                } 
                break;

        }
    }

    return I2C_OK;
}





