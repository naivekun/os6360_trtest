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

    return (0);
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
    return (GT_OK);
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
    I2C_DATA_STRUCT gtI2cData = {0};
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


    return GT_OK;
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

    return GT_OK;
}





