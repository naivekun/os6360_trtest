/*******************************************************************************
*              (c), Copyright 2014, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file extUtilDevPort.h
*
* @brief Device/Port common functions
*
* @version   2
********************************************************************************
*/
#ifndef __extUtilDevPorth
#define __extUtilDevPorth

#include <cpssCommon/cpssPresteraDefs.h>

/*
 * Example:
 *
 * GT_U8 devNum;
 *
 * EXT_UTIL_ITERATE_DEVICES_BEGIN(devNum)
 *     do_smth_for_device(devNum);
 * EXT_UTIL_ITERATE_DEVICES_END(devNum)\
 */
#define EXT_UTIL_ITERATE_DEVICES_BEGIN(_devNum)
#define EXT_UTIL_ITERATE_DEVICES_END(_devNum)

/* _type:
 * PHYSICAL
 * PHYSICAL_OR_CPU
 * VIRTUAL
 * MAC
 * TXQ
 * RXDMA
 * TXDMA
 */
#define EXT_UTIL_GET_MAX_PORTS_NUM(_devNum, _type)

/*
 * Example:
 *
 * GT_U8 devNum = 0;
 * GT_PORT_NUM portNum;
 *
 * EXT_UTIL_ITERATE_PORTS_BEGIN(PHYSICAL, devNum, portNum)
 *     do_smth_for_devport(devNum, portNum);
 * EXT_UTIL_ITERATE_PORTS_END(PHYSICAL, devNum, portNum)
 */
#define EXT_UTIL_ITERATE_PORTS_BEGIN(_type, _devNum,_portNum)
#define EXT_UTIL_ITERATE_PORTS_END(_type, _devNum,_portNum)


/**
* @internal extUtilHwDeviceNumberGet function
* @endinternal
*
* @brief   Return Hardware device Id
*
* @param[in] swDevNum                 - software number
*
* @param[out] hwDevPtr                 - HW device id
*                                       GT_OK on success
*/
GT_STATUS extUtilHwDeviceNumberGet
(
  IN  GT_U32              swDevNum,
  OUT GT_HW_DEV_NUM       *hwDevPtr
);

/**
* @internal extUtilSwDeviceNumberGet function
* @endinternal
*
* @brief   Return software device numver by hardware device id
*
* @param[in] hwDev                    - HW number
*
* @param[out] swDevNumPtr              - software number
*                                       GT_OK on success
*/
GT_STATUS extUtilSwDeviceNumberGet
(
  IN  GT_HW_DEV_NUM       hwDev,
  OUT GT_U32              *swDevNumPtr
);

/**
* @internal extUtilDoesDeviceExists function
* @endinternal
*
* @brief   Return true if device exists (configured)
*
* @param[in] devNum                   - device ID
*                                       GT_TRUE if device exist, GT_FALSE otherwise
*/
GT_BOOL extUtilDoesDeviceExists
(
  IN  GT_U8         devNum
);

/**
* @internal extUtilDoesPortExists function
* @endinternal
*
* @brief   Return true if port exists
*
* @param[in] devNum                   - device ID
* @param[in] portNum                  - port number
*                                       GT_TRUE if port exist, GT_FALSE otherwise
*/
GT_BOOL extUtilDoesPortExists
(
  IN  GT_U8         devNum,
  IN  GT_PORT_NUM   portNum
);

/**
* @internal extUtilDoesHwPortExists function
* @endinternal
*
* @brief   Return true if physical port exists
*
* @param[in] hwDevNum                 - HW number
* @param[in] portNum                  - port number
*                                       GT_TRUE if port exist, GT_FALSE otherwise
*/
GT_BOOL extUtilDoesHwPortExists
(
  IN  GT_HW_DEV_NUM hwDevNum,
  IN  GT_PORT_NUM   portNum
);

#endif /* __extUtilDevPorth */

