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
* @file gtTwsiHwCtrl.c
*
* @brief API implementation for TWSI facilities.
*
*/

/*Includes*/

#include <gtExtDrv/drivers/gtTwsiDrvCtrl.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>


/* Macroes definitions */
#define TWSI_SLAVE_ADDR(data)     (data)

#define SALSA_DEVICE
#ifdef  SALSA_DEVICE
#define MV_ON_READ_SET_BITS31(data)    (data[0] |= 0x80)
#define MV_ON_WRITE_RESET_BIT31(data)  (data[0] &= 0x7F)
#define MV_RESET_BIT_30(data)          (data[0] &= 0xBF)
#else
#define MV_ON_READ_SET_BITS31(data)    (data[0] |= 0x00)
#define MV_ON_WRITE_RESET_BIT31(data)  (data[0] &= ~0x00)
#define MV_RESET_BIT_30(data)          (data[0] &= ~0x00)
#endif


static GT_STATUS mvLongToChar (IN GT_U32 src, 
                               OUT GT_U8 dst[4]);

static GT_STATUS mvCharToLong (IN GT_U8 src[4], 
                               OUT GT_U32 *dst);

static GT_STATUS mvConcatCharArray (IN GT_U8 src0[4], 
                                    IN GT_U8 src1[4], 
                                    OUT GT_U8 dst[8]);

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
    GT_STATUS status;  
    status = extDrvDirectTwsiInitDriver();
    if (status != GT_OK) return GT_FAIL;

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
    GT_U8           regCharAddr[4];
    GT_U8           regCharData[4];
    GT_U8           regCharAddrData[8];
    
    GT_STATUS  status;  

    /*Phase 1: Master Drives Address and Data over TWSI*/
    mvLongToChar (regAddr, regCharAddr);
    MV_ON_WRITE_RESET_BIT31 (regCharAddr);
    MV_RESET_BIT_30 (regCharAddr);

    mvLongToChar (value, regCharData);
    mvConcatCharArray(regCharAddr,regCharData,regCharAddrData);

    status = extDrvDirectTwsiWaitNotBusy();
    if (status != GT_OK) return GT_FAIL;

    status = extDrvDirectTwsiMasterWriteTrans(TWSI_SLAVE_ADDR(devSlvId),
                                              regCharAddrData,8,GT_TRUE);
    if (status != GT_OK) return GT_FAIL;

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
    GT_U8           twsiRdDataBuff[4];
    GT_U8           regCharAddr[4];       

    GT_STATUS  status;  

    /*PHASE 1: Write Structure, Master drives regAddr over TWSI*/
    mvLongToChar(regAddr, regCharAddr);
    MV_ON_READ_SET_BITS31 (regCharAddr);
    MV_RESET_BIT_30 (regCharAddr);
    
    status = extDrvDirectTwsiWaitNotBusy();
    if (status != GT_OK) return GT_FAIL;

    status = extDrvDirectTwsiMasterWriteTrans(TWSI_SLAVE_ADDR(devSlvId),
                                              regCharAddr,4,GT_FALSE);
    if (status != GT_OK) return GT_FAIL;
    
    status = extDrvDirectTwsiMasterReadTrans(TWSI_SLAVE_ADDR(devSlvId),
                                              twsiRdDataBuff,4,GT_TRUE);
    if (status != GT_OK) return GT_FAIL;
    
    mvCharToLong(twsiRdDataBuff, dataPtr);
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
    GT_STATUS  status;

    if( dataPtr == NULL )
        return GT_BAD_PTR;

    if( dataLen < 1 || dataLen > 8 )
        return GT_BAD_PARAM;

    status = extDrvDirectTwsiWaitNotBusy();
    if (status != GT_OK) 
        return GT_FAIL;

    status = extDrvDirectTwsiMasterWriteTrans(TWSI_SLAVE_ADDR(devSlvId), dataPtr, dataLen, GT_TRUE);

    return status;
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
    GT_STATUS  status;

    if( dataPtr == NULL )
        return GT_BAD_PTR;

    if( dataLen < 1 || dataLen > 8 )
        return GT_BAD_PARAM;

    status = extDrvDirectTwsiWaitNotBusy();
    if (status != GT_OK) 
        return GT_FAIL;

    status = extDrvDirectTwsiMasterReadTrans(TWSI_SLAVE_ADDR(devSlvId), dataPtr, dataLen, GT_TRUE);

    return status;
}

/**
* @internal mvLongToChar function
* @endinternal
*
* @brief   Transforms unsigned long int type to 4 separate chars.
*
* @param[in] src                      - source unsigned long integer.
*
* @param[out] dst[4]                   - Array of 4 chars
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note MSB is copied to dst[0]!!
*
*/
static GT_STATUS mvLongToChar (IN GT_U32 src, 
                               OUT GT_U8 dst[4])
{
    GT_U32 i;

    for (i = 4 ; i > 0 ; i--)
    {
        dst[i-1] = (GT_U8) src & 0xFF;
        src>>=8;
    }

  return GT_OK;
}

/**
* @internal mvCharToLong function
* @endinternal
*
* @brief   Transforms an array of 4 separate chars to unsigned long integer type
*
* @param[in] src[4]                   - Source Array of 4 chars
*
* @param[out] dst                      - Unsigned long integer number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note MSB resides in src[0]!!
*
*/
static GT_STATUS mvCharToLong (IN GT_U8 src[4], 
                               OUT GT_U32 *dst)
{
    GT_U32  i;
    GT_U32  tempU32 = 0x00000000;    

    for (i = 4 ; i > 0 ; i--)
    {
        tempU32 += ((GT_U32)src[i-1]) << (8*(4-i));
    }

  *dst = tempU32;
  return GT_OK;
}


/**
* @internal mvConcatCharArray function
* @endinternal
*
* @brief   Concatinate 2 Arrays of Chars to one Array of chars
*
* @param[in] src0[4]                  - Source Array of 4 chars long
* @param[in] src1[4]                  - Source Array of 4 chars long
*
* @param[out] dst[8]                   - Conacatinated Array of 8 chars long {src1,src0}
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS mvConcatCharArray (IN GT_U8 src0[4], 
                                    IN GT_U8 src1[4], 
                                    OUT GT_U8 dst[8])
{
    GT_U32 i=0,j; /*Source and Dest Counters*/

    for (j=0; j < 8; j++)
    {
        if (j < 4)
            dst[j] = src0[i++];
        else if (j == 4)
        {
            dst[j] = src1[0];
            i=1;
        } else
            dst[j] = src1[i++];
    }
    return GT_OK;
}



#define DEBUG_ONLY
#ifdef  DEBUG_ONLY

#include <stdio.h>

GT_STATUS twsiwr
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS rc;
    rc = hwIfTwsiWriteReg(devSlvId,regAddr,value);
    printf("write TWSI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}


GT_STATUS twsird
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr
)
{
    GT_U32 value = 0;
    GT_STATUS rc;
    rc = hwIfTwsiReadReg(devSlvId,regAddr,&value);
    printf("read TWSI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}

#endif /* DEBUG_ONLY */



