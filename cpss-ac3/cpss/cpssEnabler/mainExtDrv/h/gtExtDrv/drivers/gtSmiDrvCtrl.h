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
* @file gtSmiDrvCtrl.h
*
* @brief API implementation for SMI facilities.
*
*/

#ifndef __gtSmiDrvCtrl
#define __gtSmiDrvCtrl

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>

/**
* @internal extDrvDirectSmiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiInitDriver
(
    GT_VOID
);

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
);

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
);

/**
* @internal extDrvXSmiInit function
* @endinternal
*
* @brief   Inits XSMI subsystem
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvXSmiInit(void);


/**
* @internal extDrvXSmiReadReg function
* @endinternal
*
* @brief   Reads a register using XSMI Address
*
* @param[in] phyId                    -  ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - pointer to data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
*/
GT_STATUS extDrvXSmiReadReg
(               
    IN  GT_U32  phyId,
    IN  GT_U32  devAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
);

/**
* @internal extDrvXSmiWriteReg function
* @endinternal
*
* @brief   Writes a register using XSMI Address
*
* @param[in] phyId                    - phy ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to write.
* @param[in] value                    - Data write to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
*/
GT_STATUS extDrvXSmiWriteReg
(
    IN GT_U32 phyId,
    IN  GT_U32 devAddr,
    IN GT_U32 regAddr,
    IN GT_U32 value
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtSmiDrvCtrl */



