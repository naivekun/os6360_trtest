/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*Includes*/

#include <gtExtDrv/drivers/gtTwsiDrvCtrl.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <asicSimulation/SCIB/scib.h>


#define MAX_SLAVE_ID (256)

static GT_U32 i2cSimDevDb[MAX_SLAVE_ID];

/**
* @internal hwIfTwsiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfTwsiInitDriver
(
    GT_VOID
)
{
    
    GT_U32     i2cSlave;
    GT_U32     simuDevId;

    for (i2cSlave = 0; i2cSlave < MAX_SLAVE_ID; i2cSlave++)
    {
        simuDevId = scibGetDeviceId(i2cSlave );
        i2cSimDevDb[ i2cSlave ] = simuDevId;      
    }
    
  
    return GT_OK;
}


/**
* @internal hwIfTwsiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfTwsiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    scibWriteMemory(i2cSimDevDb[devSlvId], regAddr, 1, &value);
   
    return GT_OK;
}


/**
* @internal hwIfTwsiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfTwsiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    scibReadMemory(i2cSimDevDb[devSlvId], regAddr, 1, dataPtr);

    return GT_OK;
}

/**
* @internal hwIfTwsiWriteData function
* @endinternal
*
* @brief   Generic TWSI Write operation.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] dataPtr                  - (pointer to) data to be send on the TWSI.
* @param[in] dataLen                  - number of bytes to send on the TWSI (from dataPtr).
*                                      range: 1-8.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS hwIfTwsiWriteData
(
    IN GT_U32 devSlvId,
    IN GT_U8  *dataPtr,
    IN GT_U8  dataLen
)
{
    return GT_OK;
}

/**
* @internal hwIfTwsiReadData function
* @endinternal
*
* @brief   Generic TWSI Read operation.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] dataLen                  - number of bytes to recieve on the TWSI (into dataPtr).
*                                      range: 1-8.
*
* @param[out] dataPtr                  - (pointer to) data to be send on the TWSI.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS hwIfTwsiReadData
(               
    IN GT_U32 devSlvId,
    IN GT_U8  dataLen,
    OUT GT_U8 *dataPtr
)
{
    return GT_OK;
}



