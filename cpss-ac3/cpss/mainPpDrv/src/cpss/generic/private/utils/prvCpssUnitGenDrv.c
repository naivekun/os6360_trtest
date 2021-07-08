/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChDiagPacketGenerator.c
*
* DESCRIPTION:
*/
/**
********************************************************************************
* @file prvCpssUnitGenDrv.c
*
* @brief prvCpssUnitGenDrv.c.
*
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/private/utils/prvCpssUnitGenDrv.h>

#define PRV_CPSS_GEB_DRV_DEBUG 0

GT_STATUS prvCpssDrvFldListInit
(
    INOUT PRV_CPSS_DXCH_DRV_STC      *drvPtr,
    IN    GT_U32                      fldListLen,
    IN    PRV_CPSS_DRV_FLD_INIT_STC  *fldInitListPtr
)
{
    GT_U32 i;
    PRV_CPSS_DRV_FLD_DEF_STC *fldPtr;
    PRV_CPSS_DRV_FLD_INIT_STC * intSeqPtr;

    intSeqPtr = fldInitListPtr;
    for (i = 0 ; intSeqPtr->fldId != PRV_BAD_VAL; i++, intSeqPtr++)
    {
        if (intSeqPtr->fldId >= fldListLen)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        fldPtr = &drvPtr->fldDefListPtr[intSeqPtr->fldId];

        fldPtr->isSupported          = GT_TRUE;
        fldPtr->regAddrOffsInStruct  = intSeqPtr->regDef.regAddrOffsInStruct;
        fldPtr->offs                 = intSeqPtr->regDef.offs;
        fldPtr->len                  = intSeqPtr->regDef.len;
        fldPtr->minValue             = intSeqPtr->fldBnd.minValue;
        fldPtr->maxValue             = intSeqPtr->fldBnd.maxValue;
        fldPtr->namePtr              = intSeqPtr->namePtr;
    }
    return GT_OK;
}

/**
* @internal prvCpssDrvInit function
* @endinternal
*
* @brief   Init general field oriented driver
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] drvPtr                   - pointer to driver description
* @param[in] fldDefListPtr            - pointer to field declaration table
* @param[in] fldInitListPtr           - pointer to array of all supported list
* @param[in] totalFldNum              - total number of fields in driver (both spuurted and not) = table size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on hardware error
*/
GT_STATUS prvCpssDrvInit
(
    INOUT PRV_CPSS_DXCH_DRV_STC     *drvPtr,
    IN    PRV_CPSS_DRV_FLD_DEF_STC  *fldDefListPtr, /* memory where field definition shall be stored */
    IN    PRV_CPSS_DRV_FLD_INIT_STC *fldInitListPtr,
    IN    GT_U32                     totalFldNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_FLD_DEF_STC *fldPtr;
    GT_U32    i;

    CPSS_NULL_PTR_CHECK_MAC(drvPtr);
    CPSS_NULL_PTR_CHECK_MAC(fldDefListPtr);
    CPSS_NULL_PTR_CHECK_MAC(fldInitListPtr);

    drvPtr->fldDefListPtr      = fldDefListPtr;
    drvPtr->totalFldNum        = totalFldNum;

    /* init field table to empty */
    for (i = 0 ; i < totalFldNum; i++)
    {
        fldPtr = &drvPtr->fldDefListPtr[i];

        fldPtr->isSupported          = GT_FALSE;
        fldPtr->regAddrOffsInStruct  = 0;
        fldPtr->offs                 = 0;
        fldPtr->len                  = 0;
        fldPtr->minValue             = 0;
        fldPtr->maxValue             = 0;
        fldPtr->namePtr              = (GT_CHAR *)NULL;
    }

    /* init all suported fields */
    rc = prvCpssDrvFldListInit(drvPtr,totalFldNum,fldInitListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDrvQeueryInit function
* @endinternal
*
* @brief   Init qeuery
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] queryPtr                 - pointer to query description
* @param[in] drvPtr                   - pointer to driver description
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] regStructBaseAddrPtr     - base address of structure from where addresses of appropriate registers will be taken
* @param[in,out] queryPtr                 - pointer to query description
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDrvQeueryInit
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_DXCH_DRV_STC       *drvPtr,
    IN    GT_U8                        devNum,
    IN    GT_U32                       portGroupId,
    IN    GT_U32                      *regStructBaseAddrPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(queryPtr);
    CPSS_NULL_PTR_CHECK_MAC(drvPtr);

    queryPtr->drvPtr               = drvPtr;
    queryPtr->devNum               = devNum;
    queryPtr->portGroupId          = portGroupId;
    queryPtr->regStructBaseAddrPtr = regStructBaseAddrPtr;
    return GT_OK;
}


/**
* @internal prvCpssDrvQeueryFldSet function
* @endinternal
*
* @brief   Set field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query description
* @param[in] fldId                    - field id
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
*/
GT_STATUS prvCpssDrvQeueryFldSet
(
    IN  PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    IN  GT_U32                       fldValue
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    PRV_CPSS_DRV_FLD_DEF_STC * fldDefPtr;
    PRV_CPSS_DXCH_DRV_STC     *drvPtr;

    CPSS_NULL_PTR_CHECK_MAC(queryPtr);

    drvPtr = queryPtr->drvPtr;

    if (fldId >= drvPtr->totalFldNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fldDefPtr = &drvPtr->fldDefListPtr[fldId];
    if (GT_FALSE == fldDefPtr->isSupported)
    {
        #if (PRV_CPSS_GEN_DRV_DEBUG == 1)
            cpssOsPrintf("\n DRV : field is not supported : %d %s",fldId,fldDefPtr->namePtr);
        #endif
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    
    if (fldDefPtr-> minValue > 0)
    {
        if (fldValue < fldDefPtr-> minValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    if (fldValue > fldDefPtr-> maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = *(GT_U32*)(fldDefPtr->regAddrOffsInStruct + (GT_CHAR *)queryPtr->regStructBaseAddrPtr);
    rc = prvCpssDrvHwPpPortGroupSetRegField(queryPtr->devNum,queryPtr->portGroupId,regAddr,
                                        fldDefPtr->offs,
                                        fldDefPtr->len,
                                        fldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}

/**
* @internal prvCpssDrvQeueryFldGet function
* @endinternal
*
* @brief   Get the field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query
* @param[in] fldId                    - field id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*/
GT_STATUS prvCpssDrvQeueryFldGet
(
    IN  PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    OUT GT_U32                      *fldValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    PRV_CPSS_DXCH_DRV_STC     *drvPtr;
    PRV_CPSS_DRV_FLD_DEF_STC * fldDefPtr;

    CPSS_NULL_PTR_CHECK_MAC(fldValuePtr);

    drvPtr = queryPtr->drvPtr;

    if (fldId >= drvPtr->totalFldNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fldDefPtr = &drvPtr->fldDefListPtr[fldId];
    if (GT_FALSE == fldDefPtr->isSupported)
    {
        #if (PRV_CPSS_GEN_DRV_DEBUG == 1)
            cpssOsPrintf("\n DRV : field is not supported : %d %s",fldId,fldDefPtr->namePtr);
        #endif
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    regAddr = *(GT_U32*)(fldDefPtr->regAddrOffsInStruct + (GT_CHAR *)queryPtr->regStructBaseAddrPtr);
    rc = prvCpssDrvHwPpPortGroupGetRegField(queryPtr->devNum,queryPtr->portGroupId,regAddr,
                                        fldDefPtr->offs,
                                        fldDefPtr->len,
                                        /*OUT*/fldValuePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDrvQeueryFldCheck function
* @endinternal
*
* @brief   Check field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query
* @param[in] fldId                    - field id
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*/
GT_STATUS prvCpssDrvQeueryFldCheck
(
    IN   PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN   GT_U32                       fldId,
    OUT  GT_U32                       fldValue
)
{
    PRV_CPSS_DRV_FLD_DEF_STC * fldDefPtr;
    PRV_CPSS_DXCH_DRV_STC     *drvPtr;

    CPSS_NULL_PTR_CHECK_MAC(queryPtr);

    drvPtr = queryPtr->drvPtr;
    if (fldId >= drvPtr->totalFldNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fldDefPtr = &drvPtr->fldDefListPtr[fldId];
    if (GT_FALSE == fldDefPtr->isSupported)
    {
        #if (PRV_CPSS_GEN_DRV_DEBUG == 1)
            cpssOsPrintf("\n DRV : field is not supported : %d %s",fldId,fldDefPtr->namePtr);
        #endif
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    
    if (fldDefPtr-> minValue > 0)
    {
        if (fldValue < fldDefPtr-> minValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    if (fldValue > fldDefPtr-> maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


