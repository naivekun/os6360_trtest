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
* @file prvTgfOamLossMeasurement.h
*
* @brief OAM Loss Measurement features testing
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfOamLossMeasurementh
#define __prvTgfOamLossMeasurementh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <oam/prvTgfOamGeneral.h>

/**
* @internal tgfOamLossMeasurementTest function
* @endinternal
*
* @brief   Loss Measurement Verification
*/
GT_VOID tgfOamLossMeasurementTest
(
    GT_VOID
);

/**
* @internal tgfOamLossMeasurementGreenCounterSnapshotTest function
* @endinternal
*
* @brief   Loss Measurement Verification (Green Counter Snapshot)
*/
GT_VOID tgfOamLossMeasurementGreenCounterSnapshotTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamLossMeasurementh */



