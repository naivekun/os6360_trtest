#include <Copyright.h>
/**
********************************************************************************
* @file gtIMP.h
*
* @brief API/Structure definitions for Marvell IMP functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtIMP.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell IMP functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtIMP_h
#define __prvCpssDrvGtIMP_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @internal prvCpssDrvImpRun function
* @endinternal
*
* @brief   This routine is to run IMP with specified start address
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] addr                     - specified start address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpRun
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          addr
);

/**
* @internal prvCpssDrvImpStop function
* @endinternal
*
* @brief   This routine is to stop IMP
*
* @param[in] dev                      - (pointer to) device driver structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpStop
(
    IN  GT_CPSS_QD_DEV   *dev
);

/**
* @internal prvCpssDrvImpReset function
* @endinternal
*
* @brief   This routine is to reset imp
*
* @param[in] dev                      - (pointer to) device driver structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpReset
(
    IN  GT_CPSS_QD_DEV   *dev
);

/**
* @internal prvCpssDrvImpWriteEEPROM function
* @endinternal
*
* @brief   This routine is to write data to EEPROM
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] addr                     - EEPROM address
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Not currently implemented
*
*/
GT_STATUS prvCpssDrvImpWriteEEPROM
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          addr,
    IN  GT_U8           data
);

/**
* @internal prvCpssDrvImpLoadToEEPROM function
* @endinternal
*
* @brief   This routine is to load data to eeprom devices
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] data                     - data size
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Not currently implemented
*
*/
GT_STATUS prvCpssDrvImpLoadToEEPROM
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          dataSize,
    IN  GT_U8*          data
);

/**
* @internal prvCpssDrvImpLoadToRAM function
* @endinternal
*
* @brief   This routine is to load data to memory
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] addr                     - start address
* @param[in] data                     - data size
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpLoadToRAM
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          addr,
    IN  GT_U16          dataSize,
    IN  GT_U8*          data
);

/**
* @internal prvCpssDrvImpWriteComm function
* @endinternal
*
* @brief   This routine is to write data to IMP Comm Interface
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] data                     -  (one character) to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpWriteComm
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           data
);

/**
* @internal prvCpssDrvImpReadComm function
* @endinternal
*
* @brief   This routine is to read data from IMP Comm Interface
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] dataPtr                  - data (one character) to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on timeout waiting for data
*/
GT_STATUS prvCpssDrvImpReadComm
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           *dataPtr
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtIMP_h */

