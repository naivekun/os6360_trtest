#include <Copyright.h>

/********************************************************************************
* msApiPrototype.h
*
* DESCRIPTION:
*       API Prototypes for        Switch Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __prvCpssDrvMsApiInternal_h
#define __prvCpssDrvMsApiInternal_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GT_CPSS_USE_SIMPLE_PORT_MAPPING

  #define GT_CPSS_LPORT_2_PORT(_lport)          (GT_U8)((_lport) & 0xff)
  #define GT_CPSS_PORT_2_LPORT(_port)           (GT_CPSS_LPORT)((_port) & 0xff)
  #define GT_CPSS_LPORTVEC_2_PORTVEC(_lvec)     (GT_U32)((_lvec) & 0xffff)
  #define GT_CPSS_PORTVEC_2_LPORTVEC(_pvec)     (GT_U32)((_pvec) & 0xffff)
  #define GT_CPSS_LPORT_2_PHY(_lport)           (GT_U8)((_lport) & 0xff)

#else

  #define GT_CPSS_LPORT_2_PHY(_lport)           prvCpssDrvLport2phy(dev,_lport)
  #define GT_CPSS_LPORT_2_PORT(_lport)          prvCpssDrvLport2port(dev->validPortVec, _lport)
  #define GT_CPSS_PORT_2_LPORT(_port)           prvCpssDrvPort2lport(dev->validPortVec, _port)
  #define GT_CPSS_LPORTVEC_2_PORTVEC(_lvec)     prvCpssDrvLportvec2portvec(dev->validPortVec, _lvec)
  #define GT_CPSS_PORTVEC_2_LPORTVEC(_pvec)     prvCpssDrvPortvec2lportvec(dev->validPortVec, _pvec)

#endif

#define GT_CPSS_IS_PORT_SET(_portVec, _port)    \
            ((_portVec) & (0x1 << (_port)))

/* The following macro converts a binary    */
/* value (of 1 bit) to a boolean one.       */
/* 0 --> GT_FALSE                           */
/* 1 --> GT_TRUE                            */
#define PRV_CPSS_BIT_2_BOOL(binVal,boolVal)                     \
            (boolVal) = (((binVal) == 0) ? GT_FALSE : GT_TRUE)

/* The following macro converts a boolean   */
/* value to a binary one (of 1 bit).        */
/* GT_FALSE --> 0                           */
/* GT_TRUE --> 1                            */
#define PRV_CPSS_BOOL_2_BIT(boolVal,binVal)                     \
            (binVal) = (((boolVal) == GT_FALSE) ? 0 : 1)

/* The following macro converts a binary    */
/* value (of 1 bit) to a boolean one.       */
/* 0 --> GT_TRUE                            */
/* 1 --> GT_FALSE                           */
#define PRV_CPSS_BIT_2_BOOL_R(binVal,boolVal)                   \
            (boolVal) = (((binVal) == 0) ? GT_TRUE : GT_FALSE)

/* The following macro converts a boolean   */
/* value to a binary one (of 1 bit).        */
/* GT_FALSE --> 1                           */
/* GT_TRUE --> 0                            */
#define PRV_CPSS_BOOL_2_BIT_R(boolVal,binVal)                   \
            (binVal) = (((boolVal) == GT_TRUE) ? 0 : 1)

/* This macro checks for an broadcast mac address     */
#define PRV_CPSS_IS_BROADCAST_MAC(mac)      \
    (((mac).arEther[0] == 0xFF) &&          \
     ((mac).arEther[1] == 0xFF) &&          \
     ((mac).arEther[2] == 0xFF) &&          \
     ((mac).arEther[3] == 0xFF) &&          \
     ((mac).arEther[4] == 0xFF) &&          \
     ((mac).arEther[5] == 0xFF))

/* definition for Trunking */
#define PRV_CPSS_IS_TRUNK_ID_VALID(_dev, _id)    (((_id) < 32) ? 1 : 0)

/* Convert port number to smi device address */
#define GT_CPSS_CALC_SMI_DEV_ADDR(_dev, _portNum)  \
        ((_dev)->baseRegAddr + (_portNum))
/***************************************************************************/

/**
* @enum GT_CPSS_VTU_OPERATION
 *
 * @brief Defines the different ATU and VTU operations
*/
typedef enum{

    /** @brief Flush all entries.
     *  FLUSH_UNLOCKED    Flush all unlocked entries in ATU.
     */
    FLUSH_ALL = 1,

    FLUSH_NONSTATIC,

    /** Load / Purge entry. */
    LOAD_PURGE_ENTRY,

    /** Get next ATU or VTU entry. */
    GET_NEXT_ENTRY,

    /** @brief Flush all entries in a particular DBNum.
     *  FLUSH_UNLOCKED_IN_DB Flush all unlocked entries in a particular DBNum.
     *  SERVICE_VIOLATONS  - sevice violations of VTU
     */
    FLUSH_ALL_IN_DB,

    FLUSH_NONSTATIC_IN_DB,

    SERVICE_VIOLATIONS

} GT_CPSS_VTU_OPERATION;

/**
* @internal prvCpssDrvLport2port function
* @endinternal
*
* @brief   This function converts logical port number to physical port number
*
* @param[in] port                     - physical port list in vector
* @param[in] port                     - logical  number
*                                       physical port number
*/
GT_U8 prvCpssDrvLport2port
(
    IN  GT_U16          portVec,
    IN  GT_CPSS_LPORT   port
);

/**
* @internal prvCpssDrvPort2lport function
* @endinternal
*
* @brief   This function converts physical port number to logical port number
*
* @param[in] portVec                  - physical port list in vector
*                                      port    - logical port number
*                                       physical port number
*/
GT_CPSS_LPORT prvCpssDrvPort2lport
(
    IN  GT_U16    portVec,
    IN  GT_U8     hwPort
);

/**
* @internal prvCpssDrvLportvec2portvec function
* @endinternal
*
* @brief   This function converts logical port vector to physical port vector
*
* @param[in] portVec                  - physical port list in vector
* @param[in] lVec                     - logical port vector
*                                       physical port vector
*/
GT_U32 prvCpssDrvLportvec2portvec
(
    IN  GT_U16    portVec,
    IN  GT_U32    lVec
);

/**
* @internal prvCpssDrvPortvec2lportvec function
* @endinternal
*
* @brief   This function converts physical port vector to logical port vector
*
* @param[in] portVec                  - physical port list in vector
* @param[in] pVec                     - physical port vector
*                                       logical port vector
*/
GT_U32 prvCpssDrvPortvec2lportvec
(
    IN  GT_U16    portVec,
    IN  GT_U32    pVec
);

/**
* @internal prvCpssDrvLport2phy function
* @endinternal
*
* @brief   This function converts logical port number to physical phy number.
*
* @param[in] port                     - physical port list in vector
* @param[in] port                     - logical  number
*                                       physical port number
*/
GT_U8 prvCpssDrvLport2phy
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port
);


/**
* @internal prvCpssDrvHwSetAnyRegField function
* @endinternal
*
* @brief   This function writes to specified field in a switch's register.
*
* @param[in] devAddr                  - Device Address to write the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
*
* @note 1. The sum of fieldOffset & fieldLength parameters must be smaller-
*       equal to 16.
*
*/
GT_STATUS prvCpssDrvHwSetAnyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          data
);

/**
* @internal prvCpssDrvHwGetAnyRegField function
* @endinternal
*
* @brief   This function reads a specified field from a switch's register.
*
* @param[in] devAddr                  - device address to read the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to read.
*
* @param[out] data                     - The read register field.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
*
* @note 1. The sum of fieldOffset & fieldLength parameters must be smaller-
*       equal to 16.
*
*/
GT_STATUS prvCpssDrvHwGetAnyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    OUT GT_U16          *data
);

/**
* @internal prvCpssDrvHwSetSMIPhyRegField function
* @endinternal
*
* @brief   This function indirectly write internal PHY specified register field through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
* @param[in] data                     - register date to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
*/

GT_STATUS prvCpssDrvHwSetSMIPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          data
);

/**
* @internal prvCpssDrvHwGetSMIPhyRegField function
* @endinternal
*
* @brief   This function indirectly read internal PHY specified register field through SMI PHY command.
*
* @param[in] devAddr                  - The PHY address to be read.
* @param[in] regAddr                  - The register address to read.
* @param[in] fieldOffset              - The field start bit index. (0 - 15)
* @param[in] fieldLength              - Number of bits to write.
*
* @param[out] data                     - The storage where register date to be saved.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDrvHwGetSMIPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    OUT GT_U16          *data
);

GT_STATUS prvCpssDrvHwWritePagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
);

GT_STATUS prvCpssDrvHwReadPagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
);

GT_STATUS prvCpssDrvHwSetPagedPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          data
);

GT_STATUS prvCpssDrvHwGetPagedPhyRegField
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U8           fieldOffset,
    IN  GT_U8           fieldLength,
    IN  GT_U16          *data
);

GT_STATUS prvCpssDrvHwPhyReset
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U16          u16Data
);

/**** function called in ISR context ****/
GT_STATUS ISR_prvCpssDrvHwReadPagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
);
GT_STATUS ISR_prvCpssDrvHwWritePagedPhyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           portNum,
    IN  GT_U8           pageNum,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
);
GT_STATUS ISR_prvCpssDrvHwGetAnyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    OUT GT_U16          *data
);
GT_STATUS ISR_prvCpssDrvHwSetAnyReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           devAddr,
    IN  GT_U8           regAddr,
    IN  GT_U16          data
);




#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvMsApiInternal_h */

