/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*includes*/
#include <gtExtDrv/os/extDrvOs.h>
#include "i2cDrv.h"
#include <private/85xx/gtCore.h>
#include <cpssCommon/cpssPresteraDefs.h>

#ifdef PRESTERA_DEBUG
#define I2C_DEBUG
#endif

#ifdef I2C_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif


/********* Internal functions *************************************************/
I2C_DATA_STRUCT gtI2cGenerateStartBit(GT_VOID );
GT_VOID         gtI2cGenerateStopBit (GT_VOID);
GT_VOID         gtI2cIntEnable       (GT_VOID);
GT_VOID         gtI2cIntDisable      (GT_VOID);
GT_VOID         gtI2cSetAckBit       (GT_VOID);
GT_STATUS       gtI2cReadStatus      (GT_VOID);
GT_BOOL         gtI2cReadIntFlag     (GT_VOID);
GT_VOID         gtI2cClearIntFlag    (GT_VOID);
GT_VOID         gtI2cReset           (GT_VOID);



/********* Variables **********************************************************/
GT_U32 gtI2cTclock;

/**
* @internal gtI2cSlaveInit function
* @endinternal
*
*/
GT_VOID gtI2cSlaveInit
(
    IN GT_U32                   deviceAddress,
    IN I2C_GENERAL_CALL_STATUS  generalCallStatus
)
{
    GT_U32 tempAddress;

    /* 10 Bit deviceAddress */
    if((deviceAddress & I2C_10BIT_ADDR) == I2C_10BIT_ADDR)
    {
        /* writing the 2 most significant bits of the 10 bit address*/
        GT_REG_WRITE(I2C_SLAVE_ADDR,(((deviceAddress & 0X300) >> 7)|0Xf0) |
                      generalCallStatus);
        /* writing the 8 least significant bits of the 10 bit address*/
        tempAddress = (deviceAddress & 0xff);
        GT_REG_WRITE(I2C_EXTENDED_SLAVE_ADDR,tempAddress);
    }
    /*7 bit address*/
    else
    {
       GT_REG_WRITE(I2C_EXTENDED_SLAVE_ADDR,0x0);
       GT_REG_WRITE(I2C_SLAVE_ADDR,(deviceAddress<<1) | generalCallStatus);
    }
}

/**
* @internal gtI2cMasterInit function
* @endinternal
*
*/
GT_U32 gtI2cMasterInit
(
    IN I2C_FREQ i2cFreq,
    IN GT_U32   tclk
)
{
    GT_U32    n,m;
    GT_U32    freq;
    GT_U32    margin;
    GT_U32    minMargin = 0xffffffff;
    GT_U32    power;
    GT_U32    actualFreq = 0;
    GT_U32    actualN = 0;
    GT_U32    actualM = 0;

    for(n = 0 ; n < 8 ; n++)
    {
        for(m = 0 ; m < 16 ; m++)
        {
            power = 2<<n; /* power = 2^(n+1) */
            freq = tclk/(10*(m+1)*power);
            margin = ((i2cFreq - freq)>0)?(i2cFreq - freq):(freq- i2cFreq);
            if(margin < minMargin)
            {
                minMargin   = margin;
                actualFreq  = freq;
                actualN     = n;
                actualM     = m;
            }
        }
    }
    /* Reset the I2C logic */
    gtI2cReset();
    /* Set the baud-rate */
    GT_REG_WRITE(I2C_STATUS_BAUDE_RATE,(actualM<<3) | actualN);
    /* Enable the I2C and slave */
    GT_REG_WRITE(I2C_CONTROL,I2C_ENABLE|I2C_ACK);
    gtI2cTclock = tclk;

    return (actualFreq);
}

/**
* @internal gtI2cIntEnable function
* @endinternal
*
*/
GT_VOID gtI2cIntEnable
(
    GT_VOID
)
{
    GT_U32  temp;

    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp | I2C_INT_ENABLE);
}

/**
* @internal gtI2cIntDisable function
* @endinternal
*
*/
GT_VOID gtI2cIntDisable
(
    GT_VOID
)
{
    GT_U32 temp;

    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp & ~I2C_INT_ENABLE);
}
/**
* @internal gtI2cReadStatus function
* @endinternal
*
*/
GT_STATUS gtI2cReadStatus
(
    GT_VOID
)
{
    GT_U32 status;

    GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);

    return (status);
}

/**
* @internal gtI2cReadIntFlag function
* @endinternal
*
*/
GT_BOOL gtI2cReadIntFlag
(
    GT_VOID
)
{
    GT_U32 temp;

    GT_REG_READ(I2C_CONTROL,&temp);

    if( (temp & I2C_INT_FLAG) != 0)
        return (GT_TRUE);
    else
        return (GT_FALSE);
}

/**
* @internal gtI2cClearIntFlag function
* @endinternal
*
*/
GT_VOID gtI2cClearIntFlag
(
    GT_VOID
)
{
    GT_U32 temp;

    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp & (~I2C_INT_FLAG));
}

/**
* @internal gtI2cReset function
* @endinternal
*
*/
GT_VOID gtI2cReset
(
    GT_VOID
)
{
    GT_REG_WRITE(I2C_SOFT_RESET,0x0);
}

/**
* @internal gtI2cGenerateStartBit function
* @endinternal
*
*/
I2C_DATA_STRUCT gtI2cGenerateStartBit
(
    GT_VOID
)
{
    GT_U32          temp;
    GT_U32          timeOut;
    GT_U32          status;
    I2C_DATA_STRUCT gtI2cData;

    /* Generate the start bit */
    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp | I2C_START_BIT);

    /* Wait for it to be transmited */
    for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
    {
        GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
        switch(status)
        {
            case I2C_BUS_ERROR:
                gtI2cData.errorCode = I2C_GENERAL_ERROR;
                gtI2cData.status = status;
                gtI2cData.data = 0xffffffff;
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();

                return (gtI2cData);

            case I2C_START_CONDITION_TRA:
                timeOut = I2C_TIMEOUT_VALUE + 1;
                break;

            default:
                break;
        }
    }

    if(timeOut == I2C_TIMEOUT_VALUE)
    {
        gtI2cData.errorCode = I2C_TIME_OUT_ERROR;
        gtI2cData.status = status;
        gtI2cData.data = 0xffffffff;
        gtI2cReset();
      /*gtI2cGenerateStopBit();
        gtI2cClearIntFlag();
      */

        return (gtI2cData);
    }

    gtI2cData.errorCode = I2C_NO_ERROR;
    gtI2cData.status = status;
    gtI2cData.data = 0xffffffff; /* fix warning */

    return (gtI2cData);
}

/**
* @internal gtI2cGenerateStopBit function
* @endinternal
*
*/
GT_VOID gtI2cGenerateStopBit
(
    GT_VOID
)
{
    GT_U32    temp;

    /* Generate stop bit */
    GT_REG_READ(I2C_CONTROL,&temp);
    /*GT_REG_WRITE(I2C_CONTROL,(temp | I2C_STOP_BIT)&(~I2C_INT_FLAG));*/
    GT_REG_WRITE(I2C_CONTROL,(temp | I2C_STOP_BIT));
}
/**
* @internal gtI2cSetAckBit function
* @endinternal
*
*/
GT_VOID gtI2cSetAckBit
(
    GT_VOID
)
{
    GT_U32    temp;

    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp | I2C_ACK);
}


/**
* @internal i2cMasterWriteBytes function
* @endinternal
*
*/

static GT_STATUS i2cMasterWriteBytes
(
    IN  GT_U16   i2cAddress,
    IN  GT_U8   *i2cData,
    IN  GT_U8   i2cDataLen,
    IN  GT_BOOL enStop,
    OUT GT_U32  *i2cStaus
)

{
    GT_U32          timeOut,status;
    I2C_DATA_STRUCT I2cResult;
    GT_U8           i;          /* counter for the number of bytes read*/
    GT_U32          temp;

    /* Unset  stop bit */
    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp & (~I2C_STOP_BIT));

    /*gtI2cSetAckBit();*/
    I2cResult = gtI2cGenerateStartBit();
    if(I2cResult.errorCode != I2C_NO_ERROR)
    {
        *i2cStaus = I2cResult.status;
        DBG_INFO(("GT_FAIL1-start bit condition\n"));
        return GT_FAIL;
    }

     /* Transmit the i2cAddress */
    /* 7 Bit i2cAddress */

    GT_REG_WRITE(I2C_DATA,i2cAddress<<1);

    gtI2cClearIntFlag();
    /*while(!gtI2cReadIntFlag());*/
    /* Wait for the i2cAddress to be transmited */
    for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
    {
        GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
        switch(status)
            {
            case I2C_BUS_ERROR:
                /* pI2cData->errorCode = I2C_GENERAL_ERROR;*/
                *i2cStaus = status;
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();
                DBG_INFO(("GT_FAIL2\n"));
                return GT_FAIL;

            case I2C_ADDR_PLUS_WRITE_BIT_TRA_ACK_REC:
                timeOut = I2C_TIMEOUT_VALUE + 1;
                break;
            case I2C_ADDR_PLUS_WRITE_BIT_TRA_ACK_NOT_REC:
                /* pI2cData->errorCode = I2C_GENERAL_ERROR;*/
                *i2cStaus = status;
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();
                DBG_INFO(("GT_FAIL3-\n"));
                return GT_FAIL;
        /* case I2C_SECOND_ADDR_PLUS_WRITE_BIT_TRA_ACK_REC:
                timeOut = I2C_TIMEOUT_VALUE + 1;
                break;
            case I2C_SECOND_ADDR_PLUS_WRITE_BIT_TRA_ACK_NOT_REC:
                pI2cData->errorCode = I2C_GENERAL_ERROR;
                *i2cStaus = status;
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();
                return;
        */
            default:
                break;
        }
    }
    if(timeOut == I2C_TIMEOUT_VALUE)
    {
     /* pI2cData->errorCode = I2C_NO_DEVICE_WITH_SUCH_ADDR;*/
        *i2cStaus = status;
        gtI2cGenerateStopBit();
        gtI2cClearIntFlag();
            DBG_INFO(("GT_FAIL2"));
            return GT_FAIL;

    }

    while(!gtI2cReadIntFlag());

    GT_REG_WRITE(I2C_DATA,i2cData[0]);
    gtI2cClearIntFlag();

    for ( i= 1 ; i < i2cDataLen + 1 ; i++)
    {
        /* wait for the data to be transmited */
        for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
        {

            if (gtI2cReadIntFlag() != 1) continue;
            GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
            switch(status)
            {
                case I2C_BUS_ERROR:
                    /*pI2cData->errorCode = I2C_GENERAL_ERROR;*/
                    *i2cStaus = status;
                    gtI2cGenerateStopBit();
                    gtI2cClearIntFlag();
                    DBG_INFO(("GT_FAIL10- byte%x\n",i));
                    return GT_FAIL;

                case I2C_MAS_TRAN_DATA_BYTE_ACK_NOT_REC:
                    /*pI2cData->errorCode = I2C_GENERAL_ERROR;*/
                    *i2cStaus = status;
                    gtI2cGenerateStopBit();
                    gtI2cClearIntFlag();
                    DBG_INFO(("GT_FAIL11- byte%x\n",i));
                    return GT_FAIL;
                case I2C_MAS_TRAN_DATA_BYTE_ACK_REC:
                    timeOut = I2C_TIMEOUT_VALUE + 1;
                    DBG_INFO(("i num=%x\n",i-1));

                    /* Generate stop bit */
                    break;
                default:
                    DBG_INFO(("def status=%x\n, byte=%x",status,i));
                    break;
            }
        }

        if(timeOut == I2C_TIMEOUT_VALUE)
        {
            /*pI2cData->errorCode = I2C_TIME_OUT_ERROR;*/
            *i2cStaus = status;
            gtI2cGenerateStopBit();
            gtI2cClearIntFlag();
            DBG_INFO(("GT_FAIL12"));

            return GT_FAIL;

        }

        if (i< i2cDataLen)
        {
                /* Write the data */
            GT_REG_WRITE(I2C_DATA,i2cData[i]);
            gtI2cClearIntFlag();
        }


        /*gtI2cClearIntFlag();*/
    }
    /*pI2cData->errorCode = I2C_NO_ERROR;*/
    if (enStop == GT_TRUE)
    {
        gtI2cGenerateStopBit();
        gtI2cClearIntFlag();
    }

    *i2cStaus = status;
    DBG_INFO(("GT_OK\n"));

    return GT_OK;

}

/**
* @internal gtI2cWriteRegister function
* @endinternal
*
* @brief   This function utilizes managment write registers to GT devices
*         via I2c interface.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS gtI2cWriteRegister
(
    IN GT_U16   deviceAddress,
    IN GT_U32   regAddr,
    IN GT_U32   data
)
{
    GT_U8   i2cData[8];
    GT_U32  i2cStatus;


    i2cData[0] = (regAddr & 0xFF000000)>>24;
    i2cData[1] = (regAddr & 0xFF0000)>>16;
    i2cData[2] = (regAddr & 0xFF00)>>8;
    i2cData[3] = regAddr & 0xFF;
    i2cData[4] = (data & 0xFF000000)>>24;
    i2cData[5] = (data & 0xFF0000)>>16;
    i2cData[6] = (data & 0xFF00)>>8;
    i2cData[7] = data & 0xFF;


    return i2cMasterWriteBytes(deviceAddress,i2cData,8,GT_TRUE,&i2cStatus);
}


/**
* @internal gtI2cReadRegister function
* @endinternal
*
* @brief   This function utilizes managment register read from GT devices
*         via I2c interface.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS gtI2cReadRegister
(
    IN GT_U16   deviceAddress,
    IN GT_U32   regAddr,
    OUT GT_U32  *dataWord
)
{
    GT_U32  temp;
    GT_U32  timeOut;
    GT_U32  status;
    GT_U8 i2cData[4];
    GT_STATUS gtStatus;
    GT_U32 i2cStatus;
    GT_U8   i;

    *dataWord = 0;

    i2cData[0] =  (regAddr >> 24) & 0xff;
    i2cData[1] =  (regAddr >> 16) & 0xff;
    i2cData[2] =  (regAddr >> 8) & 0xff;
    i2cData[3] =  (regAddr) & 0xff;
    gtI2cSetAckBit();
    gtStatus = i2cMasterWriteBytes(deviceAddress,i2cData,4,GT_FALSE,&i2cStatus);
    if (gtStatus == GT_FAIL)
        return GT_FAIL;

    /* generate a repeated start bit */
    GT_REG_READ(I2C_CONTROL,&temp);
    GT_REG_WRITE(I2C_CONTROL,temp | I2C_START_BIT);
    while(!gtI2cReadIntFlag());
    gtI2cClearIntFlag();
    /* Wait for it to be transmited */
    for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
    {
        GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
        switch(status)
        {
        case I2C_BUS_ERROR:
            gtI2cGenerateStopBit();
            gtI2cClearIntFlag();
            return GT_FAIL;
        case I2C_REPEATED_START_CONDITION_TRA:
            timeOut = I2C_TIMEOUT_VALUE + 1;
            DBG_INFO(("Repeat start bit\n"));
            break;
        default:
            break;
        }
    }
    if(timeOut == I2C_TIMEOUT_VALUE)
    {
        gtI2cGenerateStopBit();
        gtI2cClearIntFlag();
        return GT_FAIL;
    }
    /* Transmit the deviceAddress */
    GT_REG_WRITE(I2C_DATA,(deviceAddress<<1) | I2C_READ);
    while(!gtI2cReadIntFlag());
    gtI2cClearIntFlag();
    /* Wait for the deviceAddress to be transmited */
    for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
    {
        if (gtI2cReadIntFlag() != 1) continue;
        GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
        switch(status)
        {
        case I2C_BUS_ERROR:
            gtI2cGenerateStopBit();
            gtI2cClearIntFlag();
            return GT_FAIL;
        case I2C_ADDR_PLUS_READ_BIT_TRA_ACK_REC:
            timeOut = I2C_TIMEOUT_VALUE + 1;
            DBG_INFO(("Addr+read tx, Ack rcv\n"));
            break;
        case I2C_ADDR_PLUS_READ_BIT_TRA_ACK_NOT_REC:
            gtI2cGenerateStopBit();
            gtI2cClearIntFlag();
            return GT_FAIL;
        default:
            break;
        }
    }
    if(timeOut == I2C_TIMEOUT_VALUE)
    {
        gtI2cGenerateStopBit();
        gtI2cClearIntFlag();
        return GT_FAIL;
    }

    for ( i= 0 ;i<4; i++)
    {
        GT_REG_READ(I2C_CONTROL,&temp);
        if (i== 3 )
        {
            GT_REG_WRITE(I2C_CONTROL,temp & (~(I2C_ACK | I2C_INT_FLAG)));
        }
        else
        {
            GT_REG_WRITE(I2C_CONTROL,temp & (~I2C_INT_FLAG));
        }

     /* Wait for the data */
        for(timeOut = 0 ; timeOut < I2C_TIMEOUT_VALUE ; timeOut++)
        {

            if (gtI2cReadIntFlag() != 1) continue;

            GT_REG_READ(I2C_STATUS_BAUDE_RATE,&status);
            switch(status)
            {
            case I2C_BUS_ERROR:
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();
                return GT_FAIL;
            case I2C_MAS_REC_READ_DATA_ACK_NOT_TRA:
                GT_REG_READ(I2C_DATA,&temp);
                *dataWord |= ((temp & 0xff) << (8*(3-i)));
                DBG_INFO(("Ack not Tr,data rcv=%x\n",temp));
                timeOut = I2C_TIMEOUT_VALUE + 1;
                gtI2cGenerateStopBit();
                gtI2cClearIntFlag();
                break;
            case I2C_MAS_REC_READ_DATA_ACK_TRA:
                GT_REG_READ(I2C_DATA,&temp);
                *dataWord |= ((temp & 0xff) << (8*(3-i)));
                DBG_INFO(("Ack Tr, data rcv=%x\n",temp));
                /*gtI2cClearIntFlag();*/
                timeOut = I2C_TIMEOUT_VALUE + 1;
                break;
            default:
                break;
            }
        }
        if(timeOut == I2C_TIMEOUT_VALUE)
        {
            gtI2cGenerateStopBit();
            gtI2cClearIntFlag();
            return GT_FAIL;
        }
           /* Clear the interrupt flag and signal no-acknowledge */


    }

    return GT_OK;
}





