#include <Copyright.h>

/**
********************************************************************************
* @file gtHwAccess.h
*
* @brief Functions declarations for HW accessing PHY, Serdes and Switch registers.
*
*/
/********************************************************************************
* gtHwAccess.h
*
* DESCRIPTION:
*       Functions declarations for HW accessing PHY, Serdes and Switch registers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __prvCpssDrvGtHwAccess_h
#define __prvCpssDrvGtHwAccess_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This macro calculates the mask for partial read /    */
/* write of register's data.                            */
#define PRV_CPSS_CALC_MASK(fieldOffset,fieldLen,mask)                       \
    if((fieldLen + fieldOffset) >= 16)                                      \
        mask = (0 - (1 << fieldOffset));                                    \
    else                                                                    \
        mask = (((1 << (fieldLen + fieldOffset))) - (1 << fieldOffset));

#define PRV_CPSS_QD_SMI_ACCESS_LOOP        1000


/******************************************************************************/
/* Switch Registers related functions.                                        */
/******************************************************************************/

/**
* @internal prvCpssDrvHwSetAnyReg function
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
GT_STATUS prvCpssDrvHwSetAnyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
);

/**
* @internal prvCpssDrvHwGetAnyReg function
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
GT_STATUS prvCpssDrvHwGetAnyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
);

/****************************************************************************/
/* SMI PHY Registers indirect R/W functions.                                         */
/****************************************************************************/

/**
* @internal prvCpssDrvHwSetSMIPhyReg function
* @endinternal
*
* @brief   This function indirectly write internal PHY register through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address.
* @param[in] regAddr                  - The register address.
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvHwSetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
);

/**
* @internal prvCpssDrvHwGetSMIPhyReg function
* @endinternal
*
* @brief   This function indirectly read internal PHY register through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
*
* @param[out] data                     - The storage where register  to be saved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvHwGetSMIPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
);

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
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           phyAddr,
    IN  GT_U16          regAddr,
    IN  GT_U16          data
);

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
);

#ifdef __cplusplus
}
#endif
#endif /* __prvCpssDrvGtHwAccess_h */

