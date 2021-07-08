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
* @file prvCpssDxChSmiUnitDrv.c
*
* @brief SMI unit interface (Switching Core - GOP - SMI).
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpss/generic/private/utils/prvCpssUnitGenDrv.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChSmiUnitDrv.h>

typedef enum
{
     PRV_CPSS_DXCH_BC2_A0_E = 0       /* Bobcat2 A0 */
    ,PRV_CPSS_DXCH_BC2_B0_E           /* Bobcat2 B0 */
    ,PRV_CPSS_DXCH_BOBK_CETUS_E       /* BobK Cetus (just pipe1)   */
    ,PRV_CPSS_DXCH_BOBK_CAELUM_E      /* BobK Cetus (pipe0 + pipe1 */
    ,PRV_CPSS_DXCH_ALDRIN_E           /* Aldrin   */
    ,PRV_CPSS_DXCH_BC3_A0_E           /* Bobcat3 A0 */
    ,PRV_CPSS_DXCH_ALDRIN2_A0_E           /* Armstrong A0 */
    ,PRV_CPSS_DXCH_FALCON_3_2_E           /* Falcon */
    ,PRV_CPSS_DXCH_FALCON_6_4_E           /* Falcon */
    ,PRV_CPSS_DXCH_FALCON_12_8_E           /* Falcon */
    ,PRV_CPSS_DXCH_MAX_E                          /* never cross this boundary */
    ,PRV_CPSS_DXCH_INVALID_E = (GT_U32)(~1)
}PRV_CPSS_DXCH_BC2_FAMILY_DEV_ENT;

/*--------------------------------------------------------*
 *  identify PP device/family/subfamily                   *
 *--------------------------------------------------------*/
GT_STATUS prvCpssDxChBc2FamilySpecificPPGet
(
    IN  GT_U8 devNum,
    OUT PRV_CPSS_DXCH_BC2_FAMILY_DEV_ENT *ppPtr
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    *ppPtr = PRV_CPSS_DXCH_INVALID_E;
    switch (pDev->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch (pDev->genInfo.devSubFamily)
            {
                case CPSS_PP_SUB_FAMILY_NONE_E:
                {
                    if (pDev->genInfo.revision == 0)
                    {
                        *ppPtr = PRV_CPSS_DXCH_BC2_A0_E;
                    }
                    else
                    {
                        *ppPtr = PRV_CPSS_DXCH_BC2_B0_E;
                    }
                    return GT_OK;
                }
                case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
                {
                    switch (pDev->genInfo.devType)
                    {
                        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                            *ppPtr = PRV_CPSS_DXCH_BOBK_CETUS_E;
                        break;
                        case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                            *ppPtr = PRV_CPSS_DXCH_BOBK_CAELUM_E;
                        break;
                        case CPSS_ALDRIN_DEVICES_CASES_MAC:
                            *ppPtr = PRV_CPSS_DXCH_ALDRIN_E;
                        break;

                        default:
                        {
                            *ppPtr = PRV_CPSS_DXCH_INVALID_E;
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                    }
                    return GT_OK;
                }
                default:
                {
                    *ppPtr = PRV_CPSS_DXCH_INVALID_E;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        break;
        case  CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case  CPSS_PP_FAMILY_DXCH_AC3X_E:
        {
             *ppPtr = PRV_CPSS_DXCH_ALDRIN_E;
             return GT_OK;
        }
        case  CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        {
             *ppPtr = PRV_CPSS_DXCH_BC3_A0_E;
             return GT_OK;
        }
        case  CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        {
             *ppPtr = PRV_CPSS_DXCH_ALDRIN2_A0_E;
             return GT_OK;
        }
        case  CPSS_PP_FAMILY_DXCH_FALCON_E:
        {
             switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
             {
                default:
                case 1:
                    *ppPtr = PRV_CPSS_DXCH_FALCON_3_2_E;
                    break;
                case 2:
                    *ppPtr = PRV_CPSS_DXCH_FALCON_6_4_E;
                    break;
                case 4:
                    *ppPtr = PRV_CPSS_DXCH_FALCON_12_8_E;
                    break;
             }


             return GT_OK;
        }
        default:
        {
            *ppPtr = PRV_CPSS_DXCH_INVALID_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
}

/*--------------------------------------------------------*
 *  feature of SMI interface per BC2/BobK/BC3 PP families *
 *--------------------------------------------------------*/
typedef struct
{
    GT_U32 numberOfSmiIf;
    GT_U32 activeIfList   [CPSS_PHY_SMI_INTERFACE_MAX_E + 1]; /* shall be finished with CPSS_PHY_SMI_INTERFACE_INVALID_E */
    GT_U32 autoPollingList[CPSS_PHY_SMI_INTERFACE_MAX_E + 1]; /* shall be finished with CPSS_PHY_SMI_INTERFACE_INVALID_E */
    PRV_CPSS_DXCH_UNIT_ENT  sip6_smiUnitIdArr[CPSS_PHY_SMI_INTERFACE_MAX_E + 1]; /* shall be finished with PRV_CPSS_DXCH_UNIT_LAST_E */
}PRV_CPSS_DXCH_SMI_INFO_STC;

typedef struct
{
    PRV_CPSS_DXCH_BC2_FAMILY_DEV_ENT ppType;
    PRV_CPSS_DXCH_SMI_INFO_STC       smiIfno;
}PRV_CPSS_DXCH_PP_SMI_INFO_STC;

/* smi0,1 are consecutive */
#define STANDARD_2_SMI_UNITS_MAC    PRV_CPSS_DXCH_UNIT_SMI_0_E,PRV_CPSS_DXCH_UNIT_SMI_1_E,PRV_CPSS_DXCH_UNIT_LAST_E
#define STANDARD_4_SMI_UNITS_MAC    PRV_CPSS_DXCH_UNIT_SMI_0_E,PRV_CPSS_DXCH_UNIT_SMI_1_E,PRV_CPSS_DXCH_UNIT_SMI_2_E,PRV_CPSS_DXCH_UNIT_SMI_3_E,PRV_CPSS_DXCH_UNIT_LAST_E
#define _2_TILES_UNITS_OFFSET       (2 * PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS)
/* Falcon 12.8 : smi0,1 are consecutive but smi2,3 are in tile 2 */
#define FALCON_4_SMI_UNITS_MAC      PRV_CPSS_DXCH_UNIT_SMI_0_E,PRV_CPSS_DXCH_UNIT_SMI_1_E, \
                                    PRV_CPSS_DXCH_UNIT_SMI_0_E+_2_TILES_UNITS_OFFSET,PRV_CPSS_DXCH_UNIT_SMI_1_E+_2_TILES_UNITS_OFFSET,PRV_CPSS_DXCH_UNIT_LAST_E

/* Cetus and Aldrin do not support Out Of Band PHY Auto Polling */
static PRV_CPSS_DXCH_PP_SMI_INFO_STC prvCpssSMIInfoArr[] =
{
     /*-------------------------------------------------------------------------------------------------------------------------------------------*
      * ppType,                     {numberOfSmiIf,                 activeInterfaceList,                    autoPollingList                   } } *
      *-------------------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_DXCH_BC2_A0_E,      { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_4_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_BC2_B0_E,      { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_4_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_BOBK_CETUS_E,  { 2,          { 0,1,     (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, {          (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_2_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_BOBK_CAELUM_E, { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_4_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_BC3_A0_E,      { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_4_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_ALDRIN_E,      { 2,          { 0,1,     (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, {          (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_2_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_ALDRIN2_A0_E,  { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_4_SMI_UNITS_MAC}}}
    /* Falcon devices */
    ,{ PRV_CPSS_DXCH_FALCON_3_2_E,  { 2,          { 0,1,     (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1 ,    (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_2_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_FALCON_6_4_E,  { 2,          { 0,1,     (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1 ,    (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } , {STANDARD_2_SMI_UNITS_MAC}}}
    ,{ PRV_CPSS_DXCH_FALCON_12_8_E, { 4,          { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E }, { 0,1,2,3, (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E } ,  {FALCON_4_SMI_UNITS_MAC }}}

    ,{ PRV_CPSS_DXCH_INVALID_E,     { 0,          { (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E          }, { (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E          } , {PRV_CPSS_DXCH_UNIT_LAST_E}} }
};

/**
* @internal prvCpssDxChSMIInfoByPPSet function
* @endinternal
*
* @brief   set SMI info
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIInfoByPPSet
(
    GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32    i;
    PRV_CPSS_DXCH_PP_SMI_INFO_STC * ppSmiInfoPtr;
    PRV_CPSS_DXCH_BC2_FAMILY_DEV_ENT ppType;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    rc = prvCpssDxChBc2FamilySpecificPPGet(devNum, /*OUT*/&ppType);
    if (rc != GT_OK)
    {
        return rc;
    }

    ppSmiInfoPtr = NULL;
    for (i = 0 ; prvCpssSMIInfoArr[i].ppType != PRV_CPSS_DXCH_INVALID_E; i++)
    {
        if (prvCpssSMIInfoArr[i].ppType == ppType)
        {
            ppSmiInfoPtr = &prvCpssSMIInfoArr[i];
            break;
        }
    }
    if (ppSmiInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /* pp type found */
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    /* init data struct */
    pDev->hwInfo.smi_support.numberOfSmiIf = 0;
    for (i = 0 ; i < sizeof(pDev->hwInfo.smi_support.activeSMIList)/sizeof(pDev->hwInfo.smi_support.activeSMIList[0]); i++)
    {
        pDev->hwInfo.smi_support.activeSMIList[i]      = (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E;
        pDev->hwInfo.smi_support.smiAutoPollingList[i] = (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E;
        pDev->hwInfo.smi_support.activeSMIBmp[i] = GT_FALSE;
        pDev->hwInfo.smi_support.sip6_smiUnitIdArr[i] = PRV_CPSS_DXCH_UNIT___NOT_VALID___E;
    }


    /* fillthe data structure */
    pDev->hwInfo.smi_support.numberOfSmiIf = ppSmiInfoPtr->smiIfno.numberOfSmiIf;
    for (i = 0 ; ppSmiInfoPtr->smiIfno.activeIfList[i] != (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E; i++)
    {
        pDev->hwInfo.smi_support.activeSMIList[i] = ppSmiInfoPtr->smiIfno.activeIfList[i];
        pDev->hwInfo.smi_support.activeSMIBmp[ppSmiInfoPtr->smiIfno.activeIfList[i]] = GT_TRUE;
        /* used only for sip6 device */
        pDev->hwInfo.smi_support.sip6_smiUnitIdArr[i] = ppSmiInfoPtr->smiIfno.sip6_smiUnitIdArr[i];
    }
    for (i = 0 ; ppSmiInfoPtr->smiIfno.autoPollingList[i] != (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E; i++)
    {
        pDev->hwInfo.smi_support.smiAutoPollingList[i] = ppSmiInfoPtr->smiIfno.autoPollingList[i];
    }
    return GT_OK;
}


/*-------------------------------------------------------------------------------*
 * 1. phy addressos of active ports at SMI shall be aligned to upper boundary
 * 2. unused entry shall be set to some dummy address (e.g. 31)
 *
 * ex :
 *    let duummy address XX = 31
 *
 *    SMI-0  : 16 : port 0  - 15 :  addr 0 -15
 *    SMI-1  :  8 : port 16 - 23 :  addr 16-23
 *    SMI-2  : 16 : port 24 - 39 :  addr 0 -15
 *    SMI-3  :  8 : port 40 - 47 :  addr 16-23
 *
 *          SMI  0    1    2    3
 *      port  +----+----+----+----+
 *         0  |  0 | XX |  0 | XX |
 *         1  |  1 | XX |  1 | XX |
 *         2  |  2 | XX |  2 | XX |
 *         3  |  3 | XX |  3 | XX |
 *         4  |  4 | XX |  4 | XX |
 *         5  |  5 | XX |  5 | XX |
 *         6  |  6 | XX |  6 | XX |
 *         7  |  7 | XX |  7 | XX |
 *         8  |  8 | 16 |  8 | 16 |
 *         9  |  9 | 17 |  9 | 17 |
 *        10  | 10 | 18 | 10 | 18 |
 *        11  | 11 | 19 | 11 | 19 |
 *        12  | 12 | 20 | 12 | 20 |
 *        13  | 13 | 21 | 13 | 21 |
 *        14  | 14 | 22 | 14 | 22 |
 *        15  | 15 | 23 | 15 | 23 |
 *            +----+----+----+----+
 *-------------------------------------------------------------------------------*/

#define PRV_CPSS_DXCH_SMI_DUMMY_ADDRESS 0x1F

/******************************************************************************
 * specific driver : SMI unit
 ******************************************************************************/
/*
 * enum: struct PRV_CPSS_TG_DRV_FLD_ID_ENT
 *
 * Description: enum that describes fields of Mac-TG
 *
 *    PRV_CPSS_SMI_CONFIG_INVERT_MDC_E           - SMI Config / invert MDC
 *
 */


typedef enum PRV_CPSS_SMI_DRV_FLD_ID_ENTT
{
     PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E = 0
    ,PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_1_E
    /*  */
    ,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E
    ,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_1_E
    /* */
    ,PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E
    ,PRV_CPSS_SMI_CONFIG_INVERT_MDC_1_E
    /*  */
    ,PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E
    ,PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_1_E
    /*  */
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__0_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__1_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__2_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__3_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__4_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__5_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__6_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__7_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__8_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr__9_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_10_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_11_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_12_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_13_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_14_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_15_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_16_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_17_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_18_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_19_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_20_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_21_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_22_E
    ,PRV_CPSS_SMI_CONFIG_PhyAddr_23_E
    /* */
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__1_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__2_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__3_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__4_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__5_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__6_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__7_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__8_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__9_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_10_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_11_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_12_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_13_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_14_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_15_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_16_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_17_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_18_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_19_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_20_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_21_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_22_E
    ,PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_23_E
    ,PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E
    ,PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E
    /* */
    ,PRV_CPSS_SMI_FLD_MAX           /* never cross this boundary */
}PRV_CPSS_SMI_DRV_FLD_ID_ENT;


typedef struct PRV_CPSS_DXCH_SMI_DRV_STC
{
    PRV_CPSS_DXCH_DRV_STC               genDrv;
    PRV_CPSS_DRV_FLD_DEF_STC            fldDefList[PRV_CPSS_SMI_FLD_MAX];
}PRV_CPSS_DXCH_SMI_DRV_STC;


/*------------------------------------------
  where the register address are stored
   PRV_CPSS_DXCH_MAC_PACKET_GEN_CONFIG_STC - Lion2
   PRV_CPSS_DXCH_MAC_PACKET_GEN_VER1_CONFIG_STC - Bobcat2
*/
typedef struct SMI_ST       SMI_STC;
typedef struct LMS_ST       LMS_STC;

/*
#define FLD_OFF(STR,fld)      (GT_U32)  (((GT_CHAR *) (& ((STR*)0)      ->fld)) - ((GT_CHAR *)0))
#define FLD_OFFi(STR,idx,fld) (GT_U32)  (((GT_CHAR *)(&((((STR*)0)+idx)->fld))) - ((GT_CHAR *)0))
*/

#define FLD_OFF(STR,fld)      (GT_U32)offsetof(STR,fld)
#define FLD_OFFi(STR,idx,fld) idx*sizeof(STR) + offsetof(STR,fld)


static PRV_CPSS_DRV_FLD_INIT_STC prv_LmsSmiUnitFldInitStc[] =
{
    /*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                                      {          reg off in reg struct,                                      fld-offs,len }, {min/maxValue},   name     */
    /*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs),               15,  2 }, { 0,     2 },   "MDC Division 0" }
    ,{ PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_1_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs),               15,  2 }, { 0,     2 },   "MDC Division 1" }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs),                18, 2 }, { 0,     2 },   "AutoPollNumOfPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_1_E, { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs),                18, 2 }, { 0,     2 },   "AutoPollNumOfPorts 1"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E,         { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs),               20,  1 }, { 0,     1 },   "Inverted MDC 0" }
    ,{ PRV_CPSS_SMI_CONFIG_INVERT_MDC_1_E,         { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs),               20,  1 }, { 0,     1 },   "Inverted MDC 1" }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E,         { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),           7,  1 }, { 0,     1 },   "PHY AUTONEG 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_1_E,         { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),           7,  1 }, { 0,     1 },   "PHY AUTONEG 1"  }
 /* Phy Addr */
     /* LMS-IP0 - LMS0 Group 0 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__0_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),     0,5 }, { 0,    31 },   "PhyAddrPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__1_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),     5,5 }, { 0,    31 },   "PhyAddrPorts 1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__2_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),    10,5 }, { 0,    31 },   "PhyAddrPorts 2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__3_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),    15,5 }, { 0,    31 },   "PhyAddrPorts 3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__4_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),    20,5 }, { 0,    31 },   "PhyAddrPorts 4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__5_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5),    25,5 }, { 0,    31 },   "PhyAddrPorts 5"  }
     /* LMS-IP0 - LMS0 Group 1 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__6_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),    0,5 }, { 0,    31 },   "PhyAddrPorts 6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__7_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),    5,5 }, { 0,    31 },   "PhyAddrPorts 7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__8_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),   10,5 }, { 0,    31 },   "PhyAddrPorts 8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__9_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),   15,5 }, { 0,    31 },   "PhyAddrPorts 9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_10_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),   20,5 }, { 0,    31 },   "PhyAddrPorts 10" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_11_E,           { FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11),   25,5 }, { 0,    31 },   "PhyAddrPorts 11" }
     /* LMS-IP0 - LMS1 Group 0 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_12_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),   0,5 }, { 0,    31 },   "PhyAddrPorts 12" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_13_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),   5,5 }, { 0,    31 },   "PhyAddrPorts 13" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_14_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),  10,5 }, { 0,    31 },   "PhyAddrPorts 14" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_15_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),  15,5 }, { 0,    31 },   "PhyAddrPorts 15" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_16_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),  20,5 }, { 0,    31 },   "PhyAddrPorts 16" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_17_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17),  25,5 }, { 0,    31 },   "PhyAddrPorts 17" }
     /* LMS-IP0 - LMS1 Group 1 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_18_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),   0,5 }, { 0,    31 },   "PhyAddrPorts 18" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_19_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),   5,5 }, { 0,    31 },   "PhyAddrPorts 19" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_20_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),  10,5 }, { 0,    31 },   "PhyAddrPorts 20" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_21_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),  15,5 }, { 0,    31 },   "PhyAddrPorts 21" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_22_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),  20,5 }, { 0,    31 },   "PhyAddrPorts 22" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_23_E,           { FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23),  25,5 }, { 0,    31 },   "PhyAddrPorts 23" }
    /* AutoMediaSelect */
     /* LMS-IP0 - LMS0 Group 0 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             0,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__1_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             1,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__2_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             2,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__3_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             3,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__4_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             4,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__5_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0),             5,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 5"  }
     /* LMS-IP0 - LMS0 Group 1 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__6_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             0,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__7_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             1,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__8_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             2,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__9_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             3,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_10_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             4,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 10" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_11_E,{ FLD_OFF(LMS_STC, LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1),             5,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 11" }
     /* LMS-IP0 - LMS1 Group 0 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_12_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             0,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 12" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_13_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             1,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 13" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_14_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             2,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 14" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_15_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             3,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 15" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_16_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             4,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 16" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_17_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2),             5,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 17" }
     /* LMS-IP0 - LMS1 Group 1 */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_18_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             0,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 18" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_19_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             1,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 19" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_20_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             2,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 20" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_21_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             3,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 21" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_22_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             4,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 22" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_23_E,{ FLD_OFF(LMS_STC, LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3),             5,1 }, { 0,     1 },   "PhyAutoMediaSelectPorts 23" }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E,  { FLD_OFF(SMI_STC, SMIMiscConfiguration),                                       1,1 }, { 0,     1 },   "SMI FastMDC"                }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E,       { FLD_OFF(SMI_STC, SMIMiscConfiguration),                                       0,1 }, { 0,     1 },   "SMI ACCELERATE"             }
    ,{ PRV_BAD_VAL,                          {              PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL },    { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};


static PRV_CPSS_DRV_FLD_INIT_STC           prv_BC2B0_SmiUnitFldInitStc[] =
{
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                                      {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E,{ FLD_OFF(SMI_STC, SMIMiscConfiguration),          2,  2 },  { 0,       2 },   "MDC Division 0" }
    ,{ PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, { FLD_OFF(SMI_STC, SMIMiscConfiguration),          5,  5 },  { 0,      16 },   "AutoPollNumOfPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E,         { FLD_OFF(SMI_STC, SMIMiscConfiguration),         10,  1 },  { 0,       1 },   "Inverted MDC 0" }
    ,{ PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E,         { FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     17,  1 },  { 0,       1 },   "PHY AUTONEG 0"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__0_E,           { FLD_OFF(SMI_STC, PHYAddress[ 0]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__1_E,           { FLD_OFF(SMI_STC, PHYAddress[ 1]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__2_E,           { FLD_OFF(SMI_STC, PHYAddress[ 2]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__3_E,           { FLD_OFF(SMI_STC, PHYAddress[ 3]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__4_E,           { FLD_OFF(SMI_STC, PHYAddress[ 4]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__5_E,           { FLD_OFF(SMI_STC, PHYAddress[ 5]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__6_E,           { FLD_OFF(SMI_STC, PHYAddress[ 6]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__7_E,           { FLD_OFF(SMI_STC, PHYAddress[ 7]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__8_E,           { FLD_OFF(SMI_STC, PHYAddress[ 8]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__9_E,           { FLD_OFF(SMI_STC, PHYAddress[ 9]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_10_E,           { FLD_OFF(SMI_STC, PHYAddress[10]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_11_E,           { FLD_OFF(SMI_STC, PHYAddress[11]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 11"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_12_E,           { FLD_OFF(SMI_STC, PHYAddress[12]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 12"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_13_E,           { FLD_OFF(SMI_STC, PHYAddress[13]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 13"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_14_E,           { FLD_OFF(SMI_STC, PHYAddress[14]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 14"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_15_E,           { FLD_OFF(SMI_STC, PHYAddress[15]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 15"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      0,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__1_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      1,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__2_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      2,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__3_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      3,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__4_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      4,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__5_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      5,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__6_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      6,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__7_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      7,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__8_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      8,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__9_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      9,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_10_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     10,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 10" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_11_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     11,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 11" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_12_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     12,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 12" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_13_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     13,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 13" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_14_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     14,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 14" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_15_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     15,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 15" }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E,  { FLD_OFF(SMI_STC, SMIMiscConfiguration),          1,  1 },  { 0,       1 },   "SMI FastMDC"                }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E,       { FLD_OFF(SMI_STC, SMIMiscConfiguration),          0,  1 },  { 0,       1 },   "SMI ACCELERATE"             }
    ,{ PRV_BAD_VAL,                          {              PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL },  { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};


static PRV_CPSS_DRV_FLD_INIT_STC           prv_BobK_SmiUnitFldInitStc[] =
{
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                                      {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E,{ FLD_OFF(SMI_STC, SMIMiscConfiguration),          2,  2 },  { 0,       2 },   "MDC Division 0" }
    ,{ PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, { FLD_OFF(SMI_STC, SMIMiscConfiguration),          5,  5 },  { 0,      16 },   "AutoPollNumOfPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E,         { FLD_OFF(SMI_STC, SMIMiscConfiguration),         11,  1 },  { 0,       1 },   "Inverted MDC 0" }
    ,{ PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E,         { FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     17,  1 },  { 0,       1 },   "PHY AUTONEG 0"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__0_E,           { FLD_OFF(SMI_STC, PHYAddress[ 0]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__1_E,           { FLD_OFF(SMI_STC, PHYAddress[ 1]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__2_E,           { FLD_OFF(SMI_STC, PHYAddress[ 2]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__3_E,           { FLD_OFF(SMI_STC, PHYAddress[ 3]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__4_E,           { FLD_OFF(SMI_STC, PHYAddress[ 4]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__5_E,           { FLD_OFF(SMI_STC, PHYAddress[ 5]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__6_E,           { FLD_OFF(SMI_STC, PHYAddress[ 6]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__7_E,           { FLD_OFF(SMI_STC, PHYAddress[ 7]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__8_E,           { FLD_OFF(SMI_STC, PHYAddress[ 8]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__9_E,           { FLD_OFF(SMI_STC, PHYAddress[ 9]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_10_E,           { FLD_OFF(SMI_STC, PHYAddress[10]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_11_E,           { FLD_OFF(SMI_STC, PHYAddress[11]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 11"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_12_E,           { FLD_OFF(SMI_STC, PHYAddress[12]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 12"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_13_E,           { FLD_OFF(SMI_STC, PHYAddress[13]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 13"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_14_E,           { FLD_OFF(SMI_STC, PHYAddress[14]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 14"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_15_E,           { FLD_OFF(SMI_STC, PHYAddress[15]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 15"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      0,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__1_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      1,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__2_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      2,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__3_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      3,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__4_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      4,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__5_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      5,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__6_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      6,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__7_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      7,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__8_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      8,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__9_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      9,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_10_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     10,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 10" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_11_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     11,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 11" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_12_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     12,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 12" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_13_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     13,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 13" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_14_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     14,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 14" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_15_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     15,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 15" }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E,  { FLD_OFF(SMI_STC, SMIMiscConfiguration),          1,  1 },  { 0,       1 },   "SMI FastMDC"                }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E,       { FLD_OFF(SMI_STC, SMIMiscConfiguration),          0,  1 },  { 0,       1 },   "SMI ACCELERATE"             }
    ,{ PRV_BAD_VAL,                          {              PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL },  { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};

static PRV_CPSS_DRV_FLD_INIT_STC           prv_Aldrin2_SmiUnitFldInitStc[] =
{
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                                      {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*------------------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E,{ FLD_OFF(SMI_STC, SMIMiscConfiguration),          2,  3 },  { 0,       4 },   "MDC Division 0" }
    ,{ PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, { FLD_OFF(SMI_STC, SMIMiscConfiguration),          5,  5 },  { 0,      16 },   "AutoPollNumOfPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E,         { FLD_OFF(SMI_STC, SMIMiscConfiguration),         12,  1 },  { 0,       1 },   "Inverted MDC 0" }
    ,{ PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E,         { FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     17,  1 },  { 0,       1 },   "PHY AUTONEG 0"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__0_E,           { FLD_OFF(SMI_STC, PHYAddress[ 0]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__1_E,           { FLD_OFF(SMI_STC, PHYAddress[ 1]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__2_E,           { FLD_OFF(SMI_STC, PHYAddress[ 2]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__3_E,           { FLD_OFF(SMI_STC, PHYAddress[ 3]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__4_E,           { FLD_OFF(SMI_STC, PHYAddress[ 4]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__5_E,           { FLD_OFF(SMI_STC, PHYAddress[ 5]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__6_E,           { FLD_OFF(SMI_STC, PHYAddress[ 6]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__7_E,           { FLD_OFF(SMI_STC, PHYAddress[ 7]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__8_E,           { FLD_OFF(SMI_STC, PHYAddress[ 8]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr__9_E,           { FLD_OFF(SMI_STC, PHYAddress[ 9]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_10_E,           { FLD_OFF(SMI_STC, PHYAddress[10]),                0,  5 },  { 0,      31 },   "PhyAddrPorts  0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_11_E,           { FLD_OFF(SMI_STC, PHYAddress[11]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 11"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_12_E,           { FLD_OFF(SMI_STC, PHYAddress[12]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 12"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_13_E,           { FLD_OFF(SMI_STC, PHYAddress[13]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 13"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_14_E,           { FLD_OFF(SMI_STC, PHYAddress[14]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 14"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAddr_15_E,           { FLD_OFF(SMI_STC, PHYAddress[15]),                0,  5 },  { 0,      31 },   "PhyAddrPorts 15"  }
    /* */
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      0,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 0"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__1_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      1,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 1"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__2_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      2,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 2"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__3_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      3,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 3"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__4_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      4,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 4"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__5_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      5,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 5"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__6_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      6,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 6"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__7_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      7,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 7"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__8_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      8,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 8"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__9_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),      9,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 9"  }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_10_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     10,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 10" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_11_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     11,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 11" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_12_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     12,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 12" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_13_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     13,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 13" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_14_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     14,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 14" }
    ,{ PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect_15_E,{ FLD_OFF(SMI_STC, PHYAutoNegotiationConfig),     15,  1 },  { 0,       1 },   "PhyAutoMediaSelectPorts 15" }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E,  { FLD_OFF(SMI_STC, SMIMiscConfiguration),          1,  1 },  { 0,       1 },   "SMI FastMDC"                }
    ,{ PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E,       { FLD_OFF(SMI_STC, SMIMiscConfiguration),          0,  1 },  { 0,       1 },   "SMI ACCELERATE"             }
    ,{ PRV_BAD_VAL,                          {              PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL },  { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};


PRV_CPSS_DXCH_SMI_DRV_STC   prv_dxChLMS;
PRV_CPSS_DXCH_SMI_DRV_STC   prv_dxCh_BC2B0_SMI;
PRV_CPSS_DXCH_SMI_DRV_STC   prv_dxCh_BobK_SMI;
PRV_CPSS_DXCH_SMI_DRV_STC   prv_dxCh_Aldrin2_SMI;

GT_U32 prvCpssDxChSMI_LMSInstanceGet
(
    GT_U32 smiInstance
)
{
    return smiInstance /NUMBER_OF_SMI_PER_LMS_UNIT_CNS;
}

/*----------------------------------------------------------*
 * relevent just for LMS units  (coversion from SMI-->LMS   *
 *----------------------------------------------------------*/
GT_STATUS prvCpssDxChSMI_LMSInstanceLocalPortGet
(
    GT_U8       devNum,
    GT_U32      smiInstance,
    GT_U32      smiLocalPort,
    OUT GT_U32 *lmsInstancePtr,
    OUT GT_U32 *lmsLocalPortPtr
)
{
    GT_STATUS rc;
    GT_U32 autoPollSmi[CPSS_PHY_SMI_INTERFACE_MAX_E];
    GT_U32 smiIdx;
    GT_U32 globalPort;

    *lmsInstancePtr = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
    for (smiIdx = CPSS_PHY_SMI_INTERFACE_0_E; smiIdx < CPSS_PHY_SMI_INTERFACE_MAX_E ; smiIdx++)
    {
        rc = prvCpssDxChSMIAutoPollNumOfPortsGet(devNum,0,smiIdx,&autoPollSmi[smiIdx]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    globalPort = 0;
    for (smiIdx = CPSS_PHY_SMI_INTERFACE_0_E; smiIdx < smiInstance ; smiIdx++)
    {
        globalPort += autoPollSmi[smiIdx];
    }

    globalPort += smiLocalPort;
    *lmsLocalPortPtr = globalPort % NUMBER_OF_PORTS_PER_LMS_UNIT_CNS;
    return GT_OK;
}

/**
* @internal prvCpssDxChSMIDrvInit function
* @endinternal
*
* @brief   Mac SMI driver init
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChSMIDrvInit
(
    IN    GT_U8                   devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_FLD_INIT_STC * intSeqPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                                  CPSS_XCAT3_E | CPSS_LION_E | CPSS_LION2_E |CPSS_XCAT2_E);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        intSeqPtr = &prv_BC2B0_SmiUnitFldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxCh_BC2B0_SMI.genDrv
                                        ,&prv_dxCh_BC2B0_SMI.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_SMI_FLD_MAX);

        if (rc != GT_OK)
        {
            return rc;
        }

        intSeqPtr = &prv_BobK_SmiUnitFldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxCh_BobK_SMI.genDrv
                                        ,&prv_dxCh_BobK_SMI.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_SMI_FLD_MAX);

        if (rc != GT_OK)
        {
            return rc;
        }

        intSeqPtr = &prv_Aldrin2_SmiUnitFldInitStc[0];
        rc = prvCpssDrvInit(/*OUT*/&prv_dxCh_Aldrin2_SMI.genDrv
                                        ,&prv_dxCh_Aldrin2_SMI.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_SMI_FLD_MAX);

        if (rc != GT_OK)
        {
            return rc;
        }


        intSeqPtr = &prv_LmsSmiUnitFldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxChLMS.genDrv
                                        ,&prv_dxChLMS.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_SMI_FLD_MAX);


        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}


GT_STATUS prvCpssDxChSMIDrvManagementRegAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  smiInstance,
    OUT GT_U32 *regAddrPtr
)
{
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                                  CPSS_XCAT3_E | CPSS_LION_E | CPSS_LION2_E |CPSS_XCAT2_E);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
        {
            *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.SMI[smiInstance].SMIManagement;
        }
        else
        {
            switch  (smiInstance)
            {
                case CPSS_PHY_SMI_INTERFACE_0_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[0].LMS0.LMS0Group0.SMIConfig.SMI0Management;
                break;
                case CPSS_PHY_SMI_INTERFACE_1_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[0].LMS1.LMS1Group0.SMIConfig.SMI1Management;
                break;
                case CPSS_PHY_SMI_INTERFACE_2_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[1].LMS0.LMS0Group0.SMIConfig.SMI0Management;
                break;
                case CPSS_PHY_SMI_INTERFACE_3_E:
                    *regAddrPtr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[1].LMS1.LMS1Group0.SMIConfig.SMI1Management;
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChSMIDrvMiscConfigRegAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  smiInstance,
    OUT GT_U32 *regAddrPtr
)
{
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                                  CPSS_XCAT3_E | CPSS_LION_E | CPSS_LION2_E |CPSS_XCAT2_E);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
        {
            *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.SMI[smiInstance].SMIMiscConfiguration;
        }
        else
        {
            switch  (smiInstance)
            {
                case CPSS_PHY_SMI_INTERFACE_0_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[0].LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs;
                break;
                case CPSS_PHY_SMI_INTERFACE_1_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[0].LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs;
                break;
                case CPSS_PHY_SMI_INTERFACE_2_E:
                    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[1].LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs;
                break;
                case CPSS_PHY_SMI_INTERFACE_3_E:
                    *regAddrPtr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LMS[1].LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs;
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}



GT_STATUS prvCpssDxChSMIDrvPHYAutoNegConfigRegAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  smiInstance,
    OUT GT_U32 *regAddrPtr
)
{
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                                  CPSS_XCAT3_E | CPSS_LION_E | CPSS_LION2_E |CPSS_XCAT2_E);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
        {
            *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.SMI[smiInstance].PHYAutoNegotiationConfig;
        }
        else
        {
            switch  (smiInstance)
            {
                case CPSS_PHY_SMI_INTERFACE_0_E:
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, 0).LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0;
                break;
                case CPSS_PHY_SMI_INTERFACE_1_E:
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, 0).LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2;
                break;
                case CPSS_PHY_SMI_INTERFACE_2_E:
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, 1).LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0;
                break;
                case CPSS_PHY_SMI_INTERFACE_3_E:
                    *regAddrPtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, 1).LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2;
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChSMIDrvQeueryInit function
* @endinternal
*
* @brief   Mac-TG driver Init qeuery
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
static GT_STATUS prvCpssDxChSMIDrvQeueryInit
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 instance
)
{
    GT_STATUS rc;

    GT_U32 * regStructBaseAdr;
    PRV_CPSS_DXCH_DRV_STC  *drvPtr;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrVer1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
        {
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case  CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
                    {
                        regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.SMI[instance]);
                        drvPtr = &prv_dxCh_BC2B0_SMI.genDrv;
                    }
                    else if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                    {
                        regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.SMI[instance]);
                        drvPtr = &prv_dxCh_BobK_SMI.genDrv;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
                    }
                break;
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.SMI[instance]);
                    drvPtr = &prv_dxCh_Aldrin2_SMI.genDrv;
                break;
                case CPSS_PP_FAMILY_DXCH_FALCON_E:
                    /* currently Falcon as BC3 */
                    CPSS_TBD_BOOKMARK_FALCON
                    regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.SMI[instance]);
                    drvPtr = &prv_dxCh_BobK_SMI.genDrv;
                break;
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.SMI[instance]);
                    drvPtr = &prv_dxCh_BobK_SMI.genDrv;
                break;
                default:
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
                    }
            }
        }
        else
        {
            regStructBaseAdr = (GT_U32*)(&regsAddrVer1->GOP.LMS[instance]);
            drvPtr = &prv_dxChLMS.genDrv;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvQeueryInit(/*INOUT*/queryPtr,/*IN*/drvPtr,devNum,portGroupId,regStructBaseAdr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChSMIDrvQeueryFldSet function
* @endinternal
*
* @brief   Mac TG : Set field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] fldId                    - field id
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChSMIDrvQeueryFldSet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN PRV_CPSS_SMI_DRV_FLD_ID_ENT     fldId,
    IN GT_U32                          fldValue
)
{
    return prvCpssDrvQeueryFldSet(queryPtr,fldId,fldValue);
}

/**
* @internal prvCpssDxChSMIDrvQeueryFldGet function
* @endinternal
*
* @brief   MAc TG : Get the field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChSMIDrvQeueryFldGet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC   *queryPtr,
    IN  PRV_CPSS_SMI_DRV_FLD_ID_ENT      fldId,
    OUT GT_U32                          *fldValuePtr
)
{
    return prvCpssDrvQeueryFldGet(queryPtr,fldId,/*OUT*/fldValuePtr);
}

/**
* @internal prvCpssDxChSMIDrvQeueryFldCheck function
* @endinternal
*
* @brief   Set field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChSMIDrvQeueryFldCheck
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC   *queryPtr,
    IN PRV_CPSS_SMI_DRV_FLD_ID_ENT      fldId,
    IN GT_U32                           fldValue
)
{
    return prvCpssDrvQeueryFldCheck(queryPtr,fldId,fldValue);
}


#define SMI_DEBUG_CNS 0
#define LMS_DENUG_CNS 0

/*-----------------------------------------------------------------------------------*/
/* API and logical layer of Mac-TG driver                                            */
/*-----------------------------------------------------------------------------------*/
/**
* @internal prvCpssDxChSMIInvertMDCSet function
* @endinternal
*
* @brief   Enable/Disable InvertMDC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] state                    - GT_TRUE:  invert MDC
*                                      GT_FALSE: don't invert MDC
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIInvertMDCSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_BOOL              state
)
{
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E, state);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else
    {
        GT_U32 lmsInstance   = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        GT_U32 instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_INVERT_MDC_0_E+instanceOnLMS), state);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChSMIAutoNegStateSet function
* @endinternal
*
* @brief   Phy auto negaotiation set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] state                    - GT_TRUE:  Auto-Negotiation NotPerformed
*                                      GT_FALSE: Auto-Negotiation Performed
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoNegStateSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_BOOL              state
)
{
    GT_STATUS   rc;          /* function return value */
    GT_U32      value;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = BOOL2BIT_MAC(state);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E, value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else
    {
        GT_U32 lmsInstance   = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        GT_U32 instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E+instanceOnLMS), value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChSMIAutoNegStateGet function
* @endinternal
*
* @brief   Phy auto negaotiation get
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
*
* @param[out] statePtr                 - GT_TRUE:  Auto-Negotiation NotPerformed
*                                      GT_FALSE: Auto-Negotiation Performed
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoNegStateGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_BOOL             *statePtr
)
{
    GT_STATUS   rc;          /* function return value */
    GT_U32      value;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E, /*OUT*/&value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else
    {
        GT_U32 lmsInstance   = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        GT_U32 instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PHY_AUTNEG_0_E+instanceOnLMS), /*OUT*/&value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    if (value == 0)
    {
        *statePtr = GT_FALSE;
    }
    else
    {
        *statePtr = GT_TRUE;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChSMIAutoPollNumOfPortsSet function
* @endinternal
*
* @brief   Set the SMI Auto Polling number of ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] portGroupId              - portGroupId
* @param[in] state                    - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoPollNumOfPortsSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_U32               state
)
{
    GT_STATUS   rc;          /* function return value */
    GT_U32      value;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );


    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("\n    SMI DRV : AutoPollNumOfPortsSet() : smi = %d state = %d ... ",smiInstance,state);
    #endif

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }



    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldCheck(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E), state);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E), state);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else
    {
        GT_U32 lmsInstance   = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        GT_U32 instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;
        PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        switch (state)
        {
            case 8:  value = 0; break;
            case 12: value = 1; break;
            case 16: value = 2; break;
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        fldId  = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E + instanceOnLMS);
        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,fldId,value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("OK");
    #endif

    return GT_OK;
}

/**
* @internal prvCpssDxChSMIAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get the SMI Auto Polling number of ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] portGroupId              - portGroupId
*
* @param[out] statePtr                 - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoPollNumOfPortsGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_U32              *statePtr
)
{
    GT_STATUS   rc;          /* function return value */
    GT_U32      value;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("\n    SMI DRV : AutoPollNumOfPortsGet() : smi = %d ...",smiInstance);
    #endif


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, /*OUT*/&value);
        if( rc != GT_OK )
        {
            return rc;
        }

        *statePtr = value;
    }
    else
    {
        GT_U32 lmsInstance   = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        GT_U32 instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;
        PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E+instanceOnLMS);
        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,fldId, /*OUT*/&value);
        if( rc != GT_OK )
        {
            return rc;
        }

        switch (value)
        {
            case 0: *statePtr = 8;  break;
            case 1: *statePtr = 12; break;
            case 2: *statePtr = 16; break;
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("state = %d",*statePtr);
    #endif

    return GT_OK;
}



static GT_U32 prvCpssDxChSMILocalPortConvert
(
    IN  GT_U32  smiInstance,
    IN  GT_U32  smiLocalPort,
    IN  GT_U32  autoPollNumOfPorts
)
{
    GT_U32 portPlace;
    if (smiInstance == 1 || smiInstance == 3)
    {
        portPlace =  NUMBER_OF_PORTS_PER_SMI_UNIT_CNS - autoPollNumOfPorts + smiLocalPort;
    }
    else
    {
        portPlace = smiLocalPort;
    }
    return portPlace;
}

/**
* @internal prvCpssDxChSMIPortAutopollingPlaceGet function
* @endinternal
*
* @brief   Get the SMI autpolling place (Autopolling)
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] autoPollingPlacePtr      - auto Polling Place at SMI controller
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutopollingPlaceGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *autoPollingPlacePtr
)
{
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32      autoPollNumOfPorts;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    CPSS_NULL_PTR_CHECK_MAC(autoPollingPlacePtr);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("\n    SMI DRV : PortPhyAddSet() : smi = %d port %d phyAddr 0x%02x... ",smiInstance,smiLocalPort,phyAddr);
    #endif


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, /*OUT*/&autoPollNumOfPorts);
        if( rc != GT_OK )
        {
            return rc;
        }

        *autoPollingPlacePtr = prvCpssDxChSMILocalPortConvert(smiInstance,smiLocalPort,autoPollNumOfPorts);
    }
    else /* LMS unit */
    {
        #if 0
            GT_U32 lmsLocalPort;
            GT_U32 lmsInstance;
            PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

            rc = prvCpssDxChSMI_LMSInstanceLocalPortGet(devNum,smiInstance,smiLocalPort,/*OUT*/&lmsInstance,&lmsLocalPort);
            if( rc != GT_OK )
            {
                return rc;
            }
            *hwPlacePtr = lmsLocalPort;
        #endif
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf(" OK");
    #endif
    return GT_OK;
}



/**
* @internal prvCpssDxChSMIPortPhyAddSet function
* @endinternal
*
* @brief   Set the SMI local port phy adderss
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] phyAddr                  - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortPhyAddSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32               phyAddr
)
{
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32      autoPollNumOfPorts;
    GT_U32      regIdx;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("\n    SMI DRV : PortPhyAddSet() : smi = %d port %d phyAddr 0x%02x... ",smiInstance,smiLocalPort,phyAddr);
    #endif


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, /*OUT*/&autoPollNumOfPorts);
        if( rc != GT_OK )
        {
            return rc;
        }

        regIdx = prvCpssDxChSMILocalPortConvert(smiInstance,smiLocalPort,autoPollNumOfPorts);

        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAddr__0_E + regIdx), phyAddr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else /* LMS unit */
    {
        GT_U32 lmsLocalPort;
        GT_U32 lmsInstance;
        PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

        rc = prvCpssDxChSMI_LMSInstanceLocalPortGet(devNum,smiInstance,smiLocalPort,/*OUT*/&lmsInstance,&lmsLocalPort);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAddr__0_E + lmsLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,fldId, phyAddr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf(" OK");
    #endif
    return GT_OK;
}

/**
* @internal prvCpssDxChSMIPortPhyAddGet function
* @endinternal
*
* @brief   Get the SMI local port phy adderss
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] phyAddrPtr               - phy address
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortPhyAddGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *phyAddrPtr
)
{
    GT_STATUS   rc;          /* function return value */
    GT_U32      autoPollNumOfPorts;
    GT_U32      regIdx;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    CPSS_NULL_PTR_CHECK_MAC(phyAddrPtr);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("\n    SMI DRV : PortPhyAddGet() : smi = %d port %d ... ",smiInstance,smiLocalPort);
    #endif


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, /*OUT*/&autoPollNumOfPorts);
        if( rc != GT_OK )
        {
            return rc;
        }

        regIdx = prvCpssDxChSMILocalPortConvert(smiInstance,smiLocalPort,autoPollNumOfPorts);

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAddr__0_E + regIdx), /*OUT*/phyAddrPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else /* LMS unit  */
    {
        GT_U32 lmsLocalPort;
        GT_U32 lmsInstance;
        PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

        prvCpssDxChSMI_LMSInstanceLocalPortGet(devNum,smiInstance,smiLocalPort,/*OUT*/&lmsInstance,&lmsLocalPort);

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }
        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAddr__0_E + lmsLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,fldId, /*OUT*/phyAddrPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    #if (SMI_DEBUG_CNS==1)
        cpssOsPrintf("phyAddr 0x%02x",*phyAddrPtr);
    #endif
    return GT_OK;
}



/**
* @internal prvCpssDxChSMIPortAutoMediaDetectStatusSet function
* @endinternal
*
* @brief   Set the SMI local port Auto Media Detect Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
* @param[in] status                   - Auto Media Detect Status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutoMediaDetectStatusSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32               status
)
{
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E + smiLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,fldId, status);
        if( rc != GT_OK )
        {
            return rc;
        }
        return GT_OK;
    }
    else /* LMS unit */
    {
        GT_U32 lmsLocalPort;
        GT_U32 lmsInstance;

        rc = prvCpssDxChSMI_LMSInstanceLocalPortGet(devNum,smiInstance,smiLocalPort,/*OUT*/&lmsInstance,&lmsLocalPort);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E + lmsLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldSet(&query,fldId, status);
        if( rc != GT_OK )
        {
            return rc;
        }
        return GT_OK;
    }
}

/**
* @internal prvCpssDxChSMIPortAutoMediaDetectStatusGet function
* @endinternal
*
* @brief   Get the SMI local port Auto Media Detect Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] statusPtr                - Auto Media Detect Status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutoMediaDetectStatusGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *statusPtr
)
{
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }
        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E + smiLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,fldId, /*OUT*/statusPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
        return GT_OK;
    }
    else /* LMS unit  */
    {
        GT_U32 lmsLocalPort;
        GT_U32 lmsInstance;

        prvCpssDxChSMI_LMSInstanceLocalPortGet(devNum,smiInstance,smiLocalPort,/*OUT*/&lmsInstance,&lmsLocalPort);

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }
        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAutoMediaSelect__0_E + lmsLocalPort);
        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,fldId, /*OUT*/statusPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
        return GT_OK;
    }
}



GT_STATUS prvCpssDxChSMIMdcDivisionFactorSet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    IN  GT_U32   smiInstance,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32 value;
    PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query, /*IN*/devNum, portGroupId, smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        switch (divisionFactor)
        {
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E:
            {
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 1);
                if(rc != GT_OK)
                {
                    return rc;
                }
                break;
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E:
            {
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 1);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 0);
                if(rc != GT_OK)
                {
                    return rc;
                }
                break;
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E:
            {
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 1);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 1);
                if(rc != GT_OK)
                {
                    return rc;
                }
                break;
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E:
            {
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 1);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 2);
                if(rc != GT_OK)
                {
                    return rc;
                }
                break;
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E:
            {
                rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                {
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                {
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 1);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 3);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
                break;
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E:
            {
                if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                {
                    /* Fall-through*/
                }
                else
                {
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 1);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 4);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    break;
                }
            }
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E:
            {
                if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                {
                    /* Fall-through*/
                }
                else
                {
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChSMIDrvQeueryFldSet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    break;
                }
            }
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else /* LMS unit  */
    {
        GT_U32 lmsInstance;
        GT_U32 instanceOnLMS;

        lmsInstance = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }
        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E + instanceOnLMS);

        switch (divisionFactor)
        {
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E: value = 0; break;
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E: value = 1; break;
            case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E: value = 2; break;
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        rc = prvCpssDxChSMIDrvQeueryFldSet(&query, fldId, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChSMIMdcDivisionFactorGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    IN  GT_U32   smiInstance,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32      value;
    PRV_CPSS_SMI_DRV_FLD_ID_ENT fldId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    CPSS_NULL_PTR_CHECK_MAC(divisionFactorPtr);

    if (smiInstance >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum, portGroupId, smiInstance);
        if( rc != GT_OK )
        {
            return rc;
        }

        rc = prvCpssDxChSMIDrvQeueryFldGet(&query, PRV_CPSS_SMI_CONFIG_SMI_ACCELERATE_E, /*OUT*/&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (value == GT_TRUE)
        {
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
        }
        else
        {
            rc = prvCpssDxChSMIDrvQeueryFldGet(&query, PRV_CPSS_SMI_CONFIG_SMI_FAST_MDC_FACTOR_E, /*OUT*/&value);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (value == GT_TRUE)
            {
                rc = prvCpssDxChSMIDrvQeueryFldGet(&query, PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E, /*OUT*/&value);
                switch (value)
                {
                    case 0 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; break;
                    case 1 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E; break;
                    case 2 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E; break;
                    case 3 :
                        if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                        {
                            /*Fall through*/
                        }
                        else
                        {
                            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E;
                            break;
                        }
                    case 4 :
                        if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                        {
                            /*Fall through*/
                        }
                        else
                        {
                            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E;
                            break;
                        }
                    default:
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
            }
            else
            {
                if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_FALSE)
                {
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E;
                }
                else
                {
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E;
                }
            }
        }
    }
    else /* LMS unit  */
    {
        GT_U32 lmsInstance;
        GT_U32 instanceOnLMS;

        lmsInstance = prvCpssDxChSMI_LMSInstanceGet(smiInstance);
        instanceOnLMS = smiInstance % NUMBER_OF_SMI_PER_LMS_UNIT_CNS;

        rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,lmsInstance);
        if( rc != GT_OK )
        {
            return rc;
        }
        fldId = (PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_MDC_DIVISION_FACTOR_0_E + instanceOnLMS);
        rc = prvCpssDxChSMIDrvQeueryFldGet(&query,fldId, /*OUT*/&value);
        if( rc != GT_OK )
        {
            return rc;
        }
        switch (value)
        {
            case 0 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; break;
            case 1 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E; break;
            case 2 : *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E; break;
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}



GT_STATUS prvCpssDxChSMIStateGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    OUT PRV_CPSS_DXCH_SMI_STATE_STC * statePtr
)
{
    GT_STATUS rc;
    GT_U32 smiInstance;
    GT_U32 smiLocalPort;
    GT_U32 autoPollNumOfPorts;
    GT_U32 phyAddrReg;
    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                                  CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );

    if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        for (smiInstance = CPSS_PHY_SMI_INTERFACE_0_E; smiInstance < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInstance++)
        {
            rc = prvCpssDxChSMIDrvQeueryInit(&query,/*IN*/devNum,portGroupId,smiInstance);
            if( rc != GT_OK )
            {
                return rc;
            }

            rc = prvCpssDxChSMIDrvQeueryFldGet(&query,PRV_CPSS_SMI_CONFIG_AutoPollNumOfPorts_0_E, /*OUT*/&autoPollNumOfPorts);
            if( rc != GT_OK )
            {
                return rc;
            }
            statePtr->autoPollNumOfPortsArr[smiInstance] = autoPollNumOfPorts;

            for (smiLocalPort = 0; smiLocalPort < NUMBER_OF_PORTS_PER_SMI_UNIT_CNS; smiLocalPort++)
            {
                rc = prvCpssDxChSMIDrvQeueryFldGet(&query,(PRV_CPSS_SMI_DRV_FLD_ID_ENT)(PRV_CPSS_SMI_CONFIG_PhyAddr__0_E + smiLocalPort), /*OUT*/&phyAddrReg);
                if( rc != GT_OK )
                {
                    return rc;
                }
                statePtr->phyAddrRegArr[smiInstance][smiLocalPort] = phyAddrReg;
            }
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

