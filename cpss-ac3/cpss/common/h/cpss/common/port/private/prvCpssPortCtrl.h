/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvCpssPortCtrl.h
*
* @brief CPSS definitions for port configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPortCtrl_h
#define __prvCpssPortCtrl_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

/**
* @struct PRV_CPSS_PORT_REG_CONFIG_STC
 *
 * @brief A structure to describe data to write to register
*/
typedef struct{

    /** register address */
    GT_U32 regAddr;

    /** bit number inside register */
    GT_U32 fieldOffset;

    /** bit number inside register */
    GT_U32 fieldLength;

    /** value to write into the register */
    GT_U32 fieldData;

} PRV_CPSS_PORT_REG_CONFIG_STC;

/**
* @struct PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC
 *
 * @brief Port SERDES Cyclic tune sequence depicting which tune
 * modes and algorithms should be performed.
*/
typedef struct{

    /** main tunning mode */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT tune;

    /** whether or not adaptive tune should be performed */
    GT_BOOL adaptiveEnabled;

    /** whether or not edge */
    GT_BOOL edgeDetectEnabled;

} PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC;

/**
* @struct PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC
 *
 * @brief Port SERDES Cyclic tune data regarding current tune mode
 * being performed.
*/
typedef struct{

    /** structure containing data about the training sequence */
    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC trainSequence;

    /** current phase of the tune mode */
    GT_U8 phase;

    /** whether or not phase was finished */
    GT_BOOL phaseFinished;

    /** whether or not all tune phases were finished */
    GT_BOOL allPhasesFinished;

    GT_BOOL overrideEtParams;

    GT_U8 min_LF;

    GT_U8 max_LF;

} PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC;

/**
* @struct PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC
 *
 * @brief Port SERDES Cyclic tune database.
*/
typedef struct{

    /** @brief structure containing data about the tunning sequence
     *  and statuses
     */
    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC cookie;

    /** current tune mode being performed */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT cyclicTune;

} PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC;

/**
* @struct PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_PER_PORT_MODE_DB_STC
 *
 * @brief Port SERDES Cyclic tune database.
*/
typedef struct{

    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC *trainDBObjPtr;

    /** hws port mode related for the trainDBObjPtr */
    MV_HWS_PORT_STANDARD hwsMode;

} PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_PER_PORT_MODE_DB_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortCtrl_h */

