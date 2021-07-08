/********************************************************************************
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
* @file cpssCommonPortPcsCfg.c
*
* @brief CPSS implementation for port serdes.
*
*
* @version   1
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortSerdes.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>

#include <cpss/common/port/cpssPortSerdes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/prvCpssCyclicLogger.h>

#define CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC   CPSS_LOG_ERROR_AND_RETURN_MAC/*CPSS_CYCLIC_LOGGER_LOG_ERROR_AND_RETURN_MAC*/
#define CPSS_PORT_SERDES_LOG_INFORMATION_MAC        CPSS_LOG_INFORMATION_MAC/*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC*/



/**
* @internal prvCpssPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortPcsGearBoxStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacNum,
    OUT GT_BOOL                 *gbLockPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    portNum = portNum;

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode,
                                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed,
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortCheckGearBox(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *laneLock)", devNum, portGroup, phyPortNum, portMode);
    rc = mvHwsPortCheckGearBox(devNum, portGroup, phyPortNum, portMode, gbLockPtr);

        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}


/**
* @internal prvCpssPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortPcsSyncStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacMap,
    OUT GT_BOOL                 *syncPtr
)
{
    GT_STATUS               rc;           /* return code */
    GT_U32                  portGroup;        /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */
    MV_HWS_TEST_GEN_STATUS  status;        /* structure for sync status get */
    GT_U32                  i;                 /* iterator */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portIfMode,
                                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portSpeed,
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portNum = portNum;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);
    i = 0;
    do
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTestGeneratorStatus(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portPattern[%d], *status)",devNum, portGroup, phyPortNum, portMode, TEST_GEN_PRBS7);
        rc = mvHwsPortTestGeneratorStatus(devNum, portGroup, phyPortNum, portMode,
                                    TEST_GEN_PRBS7,/* not used by API, value doesn't really matter */
                                    &status);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if(0 == status.checkerLock)
        {/* no lock/sync */
            break;
        }

    }while(i++ < 1000);

    *syncPtr = BIT2BOOL_MAC(status.checkerLock);

    return GT_OK;
}

/**
* @internal prvCpssPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS prvCpssPortPcsSyncStableStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacMap,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 interval = 100;
    GT_U32 window = 5;          /* 5 * 10msec = 50msec continuous time for stable signal indication */
    GT_U32 window_nosignal = 2; /* 2 * 10msec = 20msec continuous time for stable no signal indication */
    GT_U32 delayMS = 10;
    GT_BOOL signalStateFirst;
    GT_BOOL signalStateNext;
    GT_U32 stableInterval;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E
                                            | CPSS_LION_E | CPSS_XCAT_E | CPSS_XCAT2_E);
    if(NULL == signalStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *signalStatePtr = 0;

    rc = prvCpssPortPcsSyncStatusGet(devNum, portNum, portMacMap, &signalStateFirst);
    if(GT_OK != rc)
    {
        return rc;
    }
    stableInterval = 0;
    for(i = 0; i < interval; i++)
    {
        rc = prvCpssPortPcsSyncStatusGet(devNum, portNum, portMacMap, &signalStateNext);
        if(GT_OK != rc)
        {
            return rc;
        }
        if(signalStateNext == signalStateFirst)
        {
            stableInterval++;
            if((GT_TRUE == signalStateFirst) && (stableInterval >= window))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
            else if((GT_FALSE == signalStateFirst) && (stableInterval >= window_nosignal))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
        }
        else
        {
            signalStateFirst = signalStateNext;
            stableInterval = 0;
        }
        cpssOsTimerWkAfter(delayMS);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}


