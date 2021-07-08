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
* @file cpssCommonPortSerdes.c
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

/*----- Data structures and APIs for Cyclic tune which is used by Port-Manager module -------*/

/*

    Serdes Speed[GHz]       |1.25G    |   3.125G  |    5G (QSGMII)  |   5.156G  |    6.25G   |    10G-SR Optics (XLPPI)   |
    -----------------------------------------------------------------------------------------------------------------------
    TX Manual Configuration |Yes      |   Yes     |    Yes          |   Yes     |    Yes     |      Yes                   |
    -----------------------------------------------------------------------------------------------------------------------
    RX Manual Configuration |Yes      |   Yes     |    Yes          |   Yes     |    Yes     |                            |
    -----------------------------------------------------------------------------------------------------------------------
    Serdes Training         |         |           |                 |           |            |      RX                    |
    -----------------------------------------------------------------------------------------------------------------------
    Adaptive DFE            |         |           |                 |           |            |      Yes                   |
    -----------------------------------------------------------------------------------------------------------------------


    Serdes Speed[GHz]       |    10G-LR (non AP)    | 10G-KR/KR4 (AP)   |  12.1875G     |   12.89G      |   20.625G     |
    ---------------------------------------------------------------------------------------------------------------------
    TX Manual Configuration |   Yes                 |                   |   Yes         |   Yes         |   Yes         |
    ---------------------------------------------------------------------------------------------------------------------
    RX Manual Configuration |                       |                   |               |               |               |
    ---------------------------------------------------------------------------------------------------------------------
    Serdes Training         |   Enhance RX          |   T/RX            |   Enhance RX  |   Enhance RX  |   Enhance RX  |
    ---------------------------------------------------------------------------------------------------------------------
    Adaptive DFE            |   Yes                 |   Yes             |   Yes         |   Yes         |   Yes         |
    ---------------------------------------------------------------------------------------------------------------------


    Serdes Speed[GHz]       |   25.78125G Optics (bm,CAUI4) |   25.78125G (Non-AP) DAC, C2C |    25.78125G-KR/KR4   |    27.5G      |
    ---------------------------------------------------------------------------------------------------------------------------------
    TX Manual Configuration |   Yes                         |   Yes                         |                       |    Yes        |
    ---------------------------------------------------------------------------------------------------------------------------------
    RX Manual Configuration |                               |                               |                       |               |
    ---------------------------------------------------------------------------------------------------------------------------------
    Serdes Training         |   Enhance RX                  |   Enhance RX                  |       T/RX            |    Enhance RX |
    ---------------------------------------------------------------------------------------------------------------------------------
    Adaptive DFE            |   Yes                         |   Yes                         |       Yes             |    Yes        |
    ---------------------------------------------------------------------------------------------------------------------------------

*/

/* ComPhyH does not have enhance-tune (or equivalent) training algorithm */
PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC tuneSeqNone=
{
    /* Train-mode */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E,
    /* AdaptiveRx enabled */
    GT_FALSE,
    /* EdgeDetect enabled */
    GT_FALSE,

};

/* ComPhyH does not have enhance-tune (or equivalent) training algorithm */
PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC tuneSeq10g_sr_lr_ComPhyH=
{
    /* Train-mode */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E,
    /* AdaptiveRx enabled */
    GT_FALSE,
    /* EdgeDetect enabled */
    GT_FALSE,

};

/* until we will have separate modes for {10G-SR Optics (XLPPI)} and {10G-LR (non AP)}, 10G speed
   will have the following training data (that can be changed by port manager APIs) */
PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC tuneSeq10g_sr_lr_Avago=
{
    /* Train-mode */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E,
    /* AdaptiveRx enabled */
    GT_TRUE,
    /* EdgeDetect enabled */
    GT_FALSE,

};

PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC tuneSeq40g_sr_lr_Avago=
{
    /* Train-mode */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E,
    /* AdaptiveRx enabled */
    GT_TRUE,
    /* EdgeDetect enabled */
    GT_FALSE

};

/* for every portMode that is not in this list, default DB will be tuneSeq10g_sr_lr_Avago */
PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_PER_PORT_MODE_DB_STC tuneSeqPerModeAvago[]=
{
    /*    DB                 hws port mode  */
    { &(tuneSeq40g_sr_lr_Avago),   _100GBase_KR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _100GBase_SR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _50GBase_KR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _40GBase_SR_LR },
    { &(tuneSeq40g_sr_lr_Avago),   _40GBase_KR },
    { &(tuneSeq40g_sr_lr_Avago),   _40GBase_KR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _20GBase_SR_LR },
    { &(tuneSeq40g_sr_lr_Avago),   _20GBase_KR },
    { &(tuneSeq40g_sr_lr_Avago),   _25GBase_SR },
    { &(tuneSeq40g_sr_lr_Avago),   _25GBase_KR },
    { &(tuneSeq40g_sr_lr_Avago),   _25GBase_KR2 },
    { &(tuneSeq40g_sr_lr_Avago),   _50GBase_KR2 },
    { &(tuneSeq40g_sr_lr_Avago),   _102GBase_KR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _107GBase_KR4 },
    { &(tuneSeq40g_sr_lr_Avago),   _52_5GBase_KR2 },
    { &(tuneSeq40g_sr_lr_Avago),   _26_7GBase_KR },
    /* port manager 1G supported modes */
    { &(tuneSeqNone),   SGMII },
    { &(tuneSeqNone),   _1000Base_X },
    { &(tuneSeqNone),   QSGMII },

    /* Last */
    {NULL, NON_SUP_MODE},
};

/* for every portMode that is not in this list, default DB will be tuneSeq10g_sr_lr_ComPhyH.
   This list contain a single entry for convenience */
PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_PER_PORT_MODE_DB_STC tuneSeqPerModeComPhyH[]=
{
    /*    DB                 hws port mode  */
    { &(tuneSeq10g_sr_lr_ComPhyH),   _10GBase_SR_LR },
    /* port manager 1G supported modes */
    { &(tuneSeqNone),   SGMII },
    { &(tuneSeqNone),   _1000Base_X },
    { &(tuneSeqNone),   QSGMII },

    /* Last */
    {NULL, NON_SUP_MODE},
};

/* Avago cyclic training mode DB */
static PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC *tuneSeqDBAvago[LAST_PORT_MODE];
/* ComPhyH cyclic training mode DB */
static PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC *tuneSeqDBComPhyH[LAST_PORT_MODE];

/* whether or not cyclic tune database was initialized */
static GT_BOOL portSerdesCyclicTuneDbInitialized = GT_FALSE;

/**
* @internal prvCpssCommonPortSerdesCyclicTuneDbinit function
* @endinternal
*
* @brief   Init serdes cyclic tune defaults database.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssCommonPortSerdesCyclicTuneDbinit
(
    IN GT_U8 devNum
)
{
    GT_U32 modesStages;
    GT_U32 portModeIdx;

    modesStages=0;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("initializing cyclic tune databases \n");

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    /* Avago training modes */
    while (tuneSeqPerModeAvago[modesStages].trainDBObjPtr!=NULL)
    {
        tuneSeqDBAvago[tuneSeqPerModeAvago[modesStages].hwsMode] = tuneSeqPerModeAvago[modesStages].trainDBObjPtr;
        modesStages++;
    }

    for (portModeIdx=0; portModeIdx < LAST_PORT_MODE; portModeIdx++)
    {
        if (tuneSeqDBAvago[portModeIdx]==NULL)
        {
            tuneSeqDBAvago[portModeIdx] = &tuneSeq10g_sr_lr_Avago;
        }
    }

    modesStages = 0;

    /* ComPhyH training modes */
    while (tuneSeqPerModeComPhyH[modesStages].trainDBObjPtr!=NULL)
    {
        tuneSeqDBComPhyH[tuneSeqPerModeComPhyH[modesStages].hwsMode] = tuneSeqPerModeComPhyH[modesStages].trainDBObjPtr;
        modesStages++;
    }

    for (portModeIdx=0; portModeIdx < LAST_PORT_MODE; portModeIdx++)
    {
        if (tuneSeqDBComPhyH[portModeIdx]==NULL)
        {
            tuneSeqDBComPhyH[portModeIdx] = &tuneSeq10g_sr_lr_ComPhyH;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPortSerdesCyclicAutoTuneDefaultsSet function
* @endinternal
*
* @brief   Set cyclic tune database defaults.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwsPortMode              - hws port mode
* @param[in,out] trainDb                  - (pointer to) default training database
* @param[in,out] trainDb                  - (pointer to) default training database to be updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneDefaultsSet
(
    IN    GT_U8 devNum,
    IN    MV_HWS_PORT_STANDARD    hwsPortMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC *trainDb
)
{
    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("setting cyclic tune database defaults \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(trainDb);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    trainDb->cyclicTune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
    trainDb->cookie.trainSequence.tune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
    trainDb->cookie.trainSequence.adaptiveEnabled = GT_FALSE;
    trainDb->cookie.trainSequence.edgeDetectEnabled = GT_FALSE;
    trainDb->cookie.phaseFinished = GT_TRUE;
    trainDb->cookie.allPhasesFinished = GT_FALSE;
    trainDb->cookie.phase = 0;
    trainDb->cookie.max_LF = 0;
    trainDb->cookie.min_LF = 0;
    trainDb->cookie.overrideEtParams = GT_FALSE;

    if (portSerdesCyclicTuneDbInitialized==GT_FALSE)
    {
        prvCpssCommonPortSerdesCyclicTuneDbinit(devNum);
        portSerdesCyclicTuneDbInitialized = GT_TRUE;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        trainDb->cookie.trainSequence.tune = tuneSeqDBAvago[hwsPortMode]->tune;
        trainDb->cookie.trainSequence.adaptiveEnabled = tuneSeqDBAvago[hwsPortMode]->adaptiveEnabled;
        trainDb->cookie.trainSequence.edgeDetectEnabled = tuneSeqDBAvago[hwsPortMode]->edgeDetectEnabled;
    }
    else
    {
        trainDb->cookie.trainSequence.tune = tuneSeqDBComPhyH[hwsPortMode]->tune;
        trainDb->cookie.trainSequence.adaptiveEnabled = tuneSeqDBComPhyH[hwsPortMode]->adaptiveEnabled;
        trainDb->cookie.trainSequence.edgeDetectEnabled = tuneSeqDBComPhyH[hwsPortMode]->edgeDetectEnabled;
    }

    return GT_OK;
}

/**
* @internal cpssPortSerdesCyclicAutoTuneDefaultsGet function
* @endinternal
*
* @brief   Get cyclic tune database defaults.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwsPortMode              - hws port mode
*                                      trainDb      - (pointer to) default training database
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneDefaultsGet
(
    IN    GT_U8                  devNum,
    IN    MV_HWS_PORT_STANDARD   hwsPortMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC *trainSequenceDb
)
{
    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("getting cyclic tune database defaults \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(trainSequenceDb);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    if (portSerdesCyclicTuneDbInitialized==GT_FALSE)
    {
        prvCpssCommonPortSerdesCyclicTuneDbinit(devNum);
        portSerdesCyclicTuneDbInitialized = GT_TRUE;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        trainSequenceDb->tune = tuneSeqDBAvago[hwsPortMode]->tune;
        trainSequenceDb->adaptiveEnabled = tuneSeqDBAvago[hwsPortMode]->adaptiveEnabled;
        trainSequenceDb->edgeDetectEnabled = tuneSeqDBAvago[hwsPortMode]->edgeDetectEnabled;
    }
    else
    {
        trainSequenceDb->tune = tuneSeqDBComPhyH[hwsPortMode]->tune;
        trainSequenceDb->adaptiveEnabled = tuneSeqDBComPhyH[hwsPortMode]->adaptiveEnabled;
        trainSequenceDb->edgeDetectEnabled = tuneSeqDBComPhyH[hwsPortMode]->edgeDetectEnabled;
    }

    return GT_OK;
}

/**
* @internal cpssPortSerdesCyclicAutoTune function
* @endinternal
*
* @brief   Set the tune status of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      trainDb      - (pointer to) default training database
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS  cpssPortSerdesCyclicAutoTune
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC *cookieEnclosingDB
)
{
    GT_STATUS       rc;   /* return code */

    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;
    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC    *trainCookiePtr;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(cookieEnclosingDB);

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc);

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacMap);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                  "ppCheckAndGetMacFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc,"(portMac %d) calling "
                                                  "ppIfModeGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppSpeedGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    hwsPortMode = NON_SUP_MODE;

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    cpssSpeed,
                                    &hwsPortMode);
        if(rc != GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "portMacMap=%d,ifMode=%d,speed=%d\n",
                                     portMacMap, cpssIfMode, cpssSpeed);
        }
    }

    trainCookiePtr =  (&cookieEnclosingDB->cookie);

    /* first tune is edge detect, if enabled */
    if (cookieEnclosingDB->cyclicTune == CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - current tune mode = LAST \n");
        /* PRE_TRAINING TRAINING FOR AUTO TRAIN UPON CABLE INSERT - FOR SIGNAL STABILITY */
        if (0/*PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)*/)
        {
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum, portNum, CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc,"ppTuneExtSetFunc return code is %d", rc);
            }
        }
        /* first tune is edge detect, if enabled */
        if (trainCookiePtr->trainSequence.edgeDetectEnabled == GT_TRUE)
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - edge detect supported - will be performed first \n");
            cookieEnclosingDB->cyclicTune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E;
        }
        else
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - edge detect not supported\n");
            cookieEnclosingDB->cyclicTune = (trainCookiePtr)->trainSequence.tune;
        }
    }

    /* tune finished, advanced to next tune */
    if ( cookieEnclosingDB->cookie.allPhasesFinished == GT_TRUE &&
         (cookieEnclosingDB->cyclicTune == (trainCookiePtr)->trainSequence.tune ||
          cookieEnclosingDB->cyclicTune == CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E) )
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - current main tune finished \n");
        /* and adaptive training is enabled, setting the tune */
        if (cookieEnclosingDB->cookie.trainSequence.adaptiveEnabled == GT_TRUE)
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - current tune finished - adaptive enabled \n");
            if ( cookieEnclosingDB->cyclicTune != CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E )
            {
                cookieEnclosingDB->cyclicTune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E;
            }
            else
            {
                cookieEnclosingDB->cyclicTune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
            }
        }
        /* all finished */
        else
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - current tune finished - adaptive not enabled \n");
            cookieEnclosingDB->cyclicTune = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
        }
    }
    else
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - edge detect finished \n");
        if (cookieEnclosingDB->cyclicTune == CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E
            && cookieEnclosingDB->cookie.allPhasesFinished == GT_TRUE)
        {

            cookieEnclosingDB->cyclicTune = (trainCookiePtr)->trainSequence.tune;
        }
    }

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - calling tune - %d \n", cookieEnclosingDB->cyclicTune);

    if (cookieEnclosingDB->cyclicTune != CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
    {
        rc = prvCpssPortSerdesAutoTunePhase(devNum, portNum, cookieEnclosingDB->cyclicTune, &(cookieEnclosingDB->cookie)); /*ZZZ*/
        if (rc != GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortSerdesAutoTunePhase return code is %d", rc);
        }
    }

    return rc;
}

/**
* @internal prvCpssPortSerdesAutoTunePhase function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in,out] trainCookie              - current phase of training
* @param[in,out] trainCookie              - current phase of training to be updated if needed
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortSerdesAutoTunePhase
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC   *trainCookie
)
{
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD    hwsPortMode = NON_SUP_MODE; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;
    CPSS_PORT_FEC_MODE_ENT  fecCorrect = CPSS_PORT_FEC_MODE_DISABLED_E;
    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(trainCookie);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc);

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
        CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
        CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);
        CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppFecModeGetFunc);
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                      "ppCheckAndGetMacFunc on port returned %d ",
                                                      portNum, rc);
        }

        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                      "ppIfModeGetFromExtFunc on port returned %d ",
                                                      portNum, rc);
        }

        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
        if (rc != GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                      "ppSpeedGetFromExtFunc on port returned %d ",
                                                      portNum, rc);
        }

        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppFecModeGetFunc(devNum, portNum, &fecCorrect);
        if (rc != GT_OK)
        {
                        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                      "ppFecModeGetFunc on port returned %d ",
                                                      portNum, rc);
        }

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                    cpssIfMode,
                                    cpssSpeed,
                                    &hwsPortMode);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (portTuningMode==CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - ehnace \n");
        if (trainCookie->phase == 0)
        {
            trainCookie->allPhasesFinished = GT_FALSE;
            trainCookie->phase = 1;
        }
        else if (trainCookie->phaseFinished == GT_TRUE)
        {
            trainCookie->phase++;
        }
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase=%d \n", trainCookie->phase);
        if (trainCookie->overrideEtParams == GT_TRUE)
        {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - using overriden values minLF %d"
            "maxLF %d \n", trainCookie->min_LF, trainCookie->max_LF);
        rc = prvCpssPortSerdesEnhancedAutoTuneByPhase(devNum, portNum, trainCookie->min_LF, trainCookie->max_LF,
                                        trainCookie->phase, &(trainCookie->phaseFinished));
        }
        else
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - using ET defaults \n");
            rc = prvCpssPortSerdesEnhancedAutoTuneByPhase(devNum, portNum, 0, 15,
                                         trainCookie->phase, &(trainCookie->phaseFinished));
        }

        if (rc!=GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if (trainCookie->phase==6 && trainCookie->phaseFinished == GT_TRUE)
        {
            trainCookie->allPhasesFinished = GT_TRUE;
            trainCookie->phase = 0;
        }
    }
    else if (portTuningMode==CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - edje detection \n");
        if (trainCookie->phase == 0)
        {
            trainCookie->allPhasesFinished = GT_FALSE;
            trainCookie->phase = 1;
        }
        else if (trainCookie->phaseFinished == GT_TRUE)
        {
            trainCookie->phase++;
        }
        rc = prvCpssPortSerdesEdgeDetectByPhase(devNum, portNum, trainCookie->phase, &(trainCookie->phaseFinished));
        if (rc!=GT_OK)
        {
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (trainCookie->phase==2 && trainCookie->phaseFinished == GT_TRUE)
        {
            trainCookie->allPhasesFinished = GT_TRUE;
            trainCookie->phase = 0;
        }
    }
    else
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - rx tune \n");
        if ((HWS_TWO_LANES_MODE_CHECK(hwsPortMode)) &&
            (portTuningMode == CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E) &&
            (fecCorrect == CPSS_PORT_FEC_MODE_DISABLED_E))
        {
             if (trainCookie->phase == 0)
            {
                trainCookie->allPhasesFinished = GT_FALSE;
                trainCookie->phase = 1;
            }
            else if (trainCookie->phaseFinished == GT_TRUE)
            {
                trainCookie->phase++;
            }
            rc = prvCpssPortSerdesLunchOneShotDfe(devNum, portNum, trainCookie->phase, &(trainCookie->phaseFinished));
            if (rc != GT_OK)
            {
                CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc,"(port %d)ppTuneExtSetFunc return code is %d", portNum, rc);
            }
            if (trainCookie->phase==2 && trainCookie->phaseFinished == GT_TRUE)
            {
                trainCookie->allPhasesFinished = GT_TRUE;
                trainCookie->phase = 0;
            }
        }
        else
        {
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing auto tune phase - adaptive rx/tx tune \n");
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum, portNum, portTuningMode);
            if (rc != GT_OK)
            {
                CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc,"(port %d)ppTuneExtSetFunc return code is %d", portNum, rc);
            }
            trainCookie->phaseFinished = GT_TRUE;
            trainCookie->allPhasesFinished = GT_TRUE;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPortSerdesCyclicAutoTuneStatusGet function
* @endinternal
*
* @brief   Get the tune status of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tune                     -  mode
*
* @param[out] tuneStatusPtr            - (pointer to) status of tune
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT    tune,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  rxTuneStatusPtr;
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  txTuneStatusPtr;
    GT_STATUS rc;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune status get \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(tuneStatusPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc);

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc(devNum, portNum,
                                                                     &rxTuneStatusPtr, &txTuneStatusPtr);
    if (rc!=GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, " (port %d)ppTuneStatusGetFunc failed %d ", portNum, rc );
    }

    *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune status get - tune=%d\n",tune);

    switch (tune)
    {
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E:
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E:
            *tuneStatusPtr = rxTuneStatusPtr;
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E:
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E:
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E:
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E:
            /* Adaptive running means pass */
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E:
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E:
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E:
            /* enhanced tune runs rx training, so rx result is what we want, same as 'RxTrainingOnly' */
            *tuneStatusPtr = rxTuneStatusPtr;
            break;
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E:
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            break;
        default:
            CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune status get - DEFAULT! ");
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
            CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;

    }

    return GT_OK;
}

/**
* @internal prvCpssPortSerdesEnhancedAutoTuneByPhase function
* @endinternal
*
* @brief   Set Rx training process and starts the
*         auto tune process, by phases.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phase                   - current phase
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesLunchOneShotDfe
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  phase,
    IN  GT_BOOL                *phaseFinishedPtr
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - enhance tune by phase \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(phaseFinishedPtr);

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "device not applicable for prvCpssPortSerdesLunchOneShotDfe API");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                  "ppCheckAndGetMacFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppIfModeGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppSpeedGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                cpssIfMode,
                                cpssSpeed,
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);
    rc = mvHwsAvagoSerdesOneShotTuneByPhase(devNum,portGroup,phyPortNum,hwsPortMode,phase, phaseFinishedPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortSerdesLunchSingleiCal function
* @endinternal
*
* @brief   run single iCal.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPortSerdesLunchSingleiCal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing single iCal tune \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "device not applicable for prvCpssPortSerdesLunchSingleiCal API");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                  "ppCheckAndGetMacFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppIfModeGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppSpeedGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                cpssIfMode,
                                cpssSpeed,
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);
    rc = mvHwsAvagoSerdesLunchOneShotiCal(devNum, portGroup,phyPortNum,hwsPortMode);

    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal prvCpssPortSerdesEdgeDetectByPhase function
* @endinternal
*
* @brief   Perform edge detection algorithm in order to detect TX change of peer,
*         than adjust rx according to it.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phase                    -  to perform
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesEdgeDetectByPhase
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 phase,
    IN  GT_BOOL                *phaseFinishedPtr
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - edje detection by phase \n");

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(phaseFinishedPtr);

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "device not applicable for SerdesEdgeDetectByPhase API");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                  "ppCheckAndGetMacFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppIfModeGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppSpeedGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                cpssIfMode,
                                cpssSpeed,
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Calling: mvHwsPortEnhanceTuneSet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d])",
                             (GT_U8)devNum, (GT_U32)portGroup, (GT_U32)phyPortNum, (MV_HWS_PORT_STANDARD)hwsPortMode);

    rc = mvHwsAvagoSerdesEdgeDetectByPhase(devNum, portGroup, phyPortNum, hwsPortMode, phase, phaseFinishedPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal prvCpssPortSerdesEnhancedAutoTuneByPhase function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process, by phases.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesEnhancedAutoTuneByPhase
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF,
    IN  GT_U32                 phase,
    IN  GT_BOOL                *phaseFinishedPtr
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    CPSS_PORT_SPEED_ENT          cpssSpeed;

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("performing cyclic tune - enhance tune by phase %d\n", phase);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(phaseFinishedPtr);

    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc);
    CPSS_NULL_PTR_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "device not applicable for SerdesEdgeDetectByPhase API");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d) calling "
                                                  "ppCheckAndGetMacFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &cpssIfMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppIfModeGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &cpssSpeed);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "(portMac %d) calling "
                                                  "ppSpeedGetFromExtFunc on port returned %d ",
                                                  portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                cpssIfMode,
                                cpssSpeed,
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Calling: mvHwsPortEnhanceTuneSet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], min_LF[%d], max_LF[%d])",
                             (GT_U8)devNum, (GT_U32)portGroup, (GT_U32)phyPortNum, (MV_HWS_PORT_STANDARD)hwsPortMode, (GT_U8)min_LF, (GT_U8)max_LF);
    rc = mvHwsAvagoSerdesEnhanceTuneSetByPhase(
        (GT_U8)                   devNum,
        (GT_U32)                  portGroup,
        (GT_U32)                  phyPortNum,
        (MV_HWS_PORT_STANDARD)    hwsPortMode,
        (GT_U8)                   min_LF,
        (GT_U8)                   max_LF,
        (GT_U32)                  phase,
        (GT_BOOL*)                phaseFinishedPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPortTxRxDefaultsDbEntryGet function
* @endinternal
*
* @brief   Get SerDes RX and TX default parameters.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMode                 - port mode to override the serdes parameters with.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; xCat3)
* @param[in] serdesSpeed              - SerDes speed/baud rate
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] txParamsPtr              - tx parameters
* @param[out] ctleParamsPtr            - rx (CTLE) parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  cpssPortTxRxDefaultsDbEntryGet
(
    IN  GT_U8                         devNum,
    IN  MV_HWS_PORT_STANDARD          portMode,
    IN  MV_HWS_SERDES_SPEED           serdesSpeed,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC *txParamsPtr,
    OUT CPSS_PORT_SERDES_RX_CONFIG_STC *ctleParamsPtr
)
{
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA    txParams;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA  ctleParams;
    GT_STATUS rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = GT_OK;

    rc = mvHwsPortTxRxDefaultsDbEntryGet(devNum, portMode, serdesSpeed, &txParams, &ctleParams);
    if(rc != GT_OK)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsPortTxRxDefaultsDbEntryGet"
                                      " with port mode %d returned error code %d ",
                                      portMode, rc);
    }

    /* Rx Parametes */
    if (ctleParamsPtr!=NULL)
    {
        ctleParamsPtr->bandWidth = ctleParams.bandWidth;
        ctleParamsPtr->dcGain = ctleParams.dcGain;
        ctleParamsPtr->ffeCap = ctleParams.highFrequency;
        ctleParamsPtr->loopBandwidth = ctleParams.loopBandwidth;
        ctleParamsPtr->ffeRes = ctleParams.lowFrequency;
        ctleParamsPtr->sqlch = ctleParams.squelch;
    }

    /* Tx Parametes */
    if (txParamsPtr!=NULL)
    {
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            txParamsPtr->txAmp = txParams.txAmp;
            txParamsPtr->emph0 = txParams.txEmph0;
            txParamsPtr->emph1 = txParams.txEmph1;
        }
        else
        {
            txParamsPtr->txAmp = txParams.txAmp;
            txParamsPtr->emph0 = txParams.txEmph0;
            txParamsPtr->emph1 = txParams.txEmph1;
            txParamsPtr->txAmpAdjEn = txParams.txAmpAdj;
            txParamsPtr->txAmpShft = txParams.txAmpShft;
        }
    }

    if (txParamsPtr==NULL && ctleParamsPtr==NULL)
    {
        CPSS_PORT_SERDES_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "both pointers are null");
    }

    return rc;
}


