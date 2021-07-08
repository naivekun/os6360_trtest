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
* @file prvCpssDxChPortTxQHighSpeedPortsDrv.c
*
* @brief bobcat2 and higher TxQ high speed port support
*
* @version   6
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>



/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortEnable function
* @endinternal
*
* @brief   TxQ Unit LL : assign High Speed Port to specific TxQ port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
* @param[in] portNum                  - txQ port num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortEnable
(
    IN GT_U8       devNum,
    IN GT_U32      highSpeedPortIdx,
    IN GT_PORT_NUM portNum
)
{
    GT_U32 regAddr;
    GT_U32 fldVal;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (highSpeedPortIdx >= PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portNum >= 1<<PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.ll.global.globalLLConfig.highSpeedPort[highSpeedPortIdx];
    fldVal = 1 | (portNum << PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_OFFS_CNS);
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_OFFS_CNS,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_LEN_CNS + PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS,
                                            fldVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortDisable function
* @endinternal
*
* @brief   TxQ Unit LL : disable High Speed Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortDisable
(
    IN GT_U8       devNum,
    IN GT_U32      highSpeedPortIdx
)
{
    GT_U32 regAddr;
    GT_U32 fldVal;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (highSpeedPortIdx >= PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.ll.global.globalLLConfig.highSpeedPort[highSpeedPortIdx];
    fldVal = 0;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_OFFS_CNS,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_LEN_CNS + PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS,
                                            fldVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortGet function
* @endinternal
*
* @brief   TxQ Unit LL : Get Configuration of High Speed Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
*
* @param[out] isEnabledPtr             - (pointer on) is enabled
* @param[out] portNumPtr               - pointer to TxQ port assigned to this high speed port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      highSpeedPortIdx,
    OUT GT_BOOL     *isEnabledPtr,
    OUT GT_PORT_NUM *portNumPtr
)
{
    GT_U32 regAddr;
    GT_U32 fldVal;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E |CPSS_BOBCAT3_E);

    if (highSpeedPortIdx >= PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(isEnabledPtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.ll.global.globalLLConfig.highSpeedPort[highSpeedPortIdx];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_OFFS_CNS,
                                            PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_LEN_CNS + PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS,
                                            /*OUT*/&fldVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    *isEnabledPtr = (fldVal >> PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_OFFS_CNS )  & ((1<<PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_LEN_CNS  ) -1);
    *portNumPtr   = (fldVal >> PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_OFFS_CNS ) & ((1<<PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS ) -1);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortAllConfigure function
* @endinternal
*
* @brief   TxQ Unit LL : Configure All HighSpeedPort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] numberOfHighSpeedPorts   - number of TxQ ports to configure as High Speed
* @param[in] highSpeedPortsPortArr    - array of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortAllConfigure
(
    IN GT_U8  devNum,
    GT_U32    numberOfHighSpeedPorts,
    GT_U32    *highSpeedPortsPortArr
)
{
    GT_STATUS rc;
    GT_U32 highSpeedPortIdx;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (numberOfHighSpeedPorts != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(highSpeedPortsPortArr);
    }
    if (numberOfHighSpeedPorts > CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* enable all declared */
    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < numberOfHighSpeedPorts; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitLLHighSpeedPortEnable(devNum,highSpeedPortIdx,highSpeedPortsPortArr[highSpeedPortIdx]);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    /* disable all others */
    for (highSpeedPortIdx = numberOfHighSpeedPorts ; highSpeedPortIdx < CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitLLHighSpeedPortDisable(devNum,highSpeedPortIdx);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* all ports above CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS are already diabled */
    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortDisableAll function
* @endinternal
*
* @brief   TxQDQ Unit LL : Disable All HighSpeedPort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortDisableAll
(
    IN GT_U8      devNum
)
{
    GT_STATUS rc;
    GT_U32 highSpeedPortIdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_NUM_CNS; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitLLHighSpeedPortDisable(devNum,highSpeedPortIdx);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}


/* DQ Unit */
/*
    BC2 A0
    * High speed ports enable/disable
        /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Scheduler Configuration/High Speed Ports
        address : 0x4000100C
         0 -  0   En High Speed Port 0
         1 -  1   En High Speed Port 1
         2 -  8   High Speed Port 0               The Local port number which is high speed
         9 - 15   High Speed Port 1               The Local port number which is high speed


    BC2 B0
    * High speed ports enable/disable
        /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Scheduler Configuration/High Speed Ports
        address : 0x4000100C,0x40001010

         0 -  0   En High Speed Port 0
         1 -  1   En High Speed Port 1
         2 -  2   En High Speed Port 2
         3 -  3   En High Speed Port 3
         4 - 10   High Speed Port 0               The Local port number which is high speed
        11 - 17   High Speed Port 1               The Local port number which is high speed
        18 - 24   High Speed Port 2               The Local port number which is high speed
        25 - 31   High Speed Port 2               The Local port number which is high speed



*/


typedef struct
{
    GT_U32 offs;
    GT_U32 len;
}PRV_CPSS_DXCH_REG_FIELDC_STC;

typedef struct
{
    PRV_CPSS_DXCH_REG_FIELDC_STC fldEn;
    PRV_CPSS_DXCH_REG_FIELDC_STC fldPortNum;
}PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC;

PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC prvBobcat2A0TxQDqHighSpeedPort[PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS] =
{
    /* High Speed Port 0 */
    {                      /* offs                                                         len                              */
        /* fld en*/      { PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_0_EN_OFFS_CNS,        PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS        }
        /* fld portNum*/,{ PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_0_PORT_NUM_OFFS_CNS,  PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS  }
    }
    /* High Speed Port 1 */
   ,{                      /* offs                                                         len                              */
        /* fld en*/      { PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_1_EN_OFFS_CNS,        PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS        }
        /* fld portNum*/,{ PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_1_PORT_NUM_OFFS_CNS,  PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS  }
    }
};

PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC prvBobcat2B0TxQDqHighSpeedPort[PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS] =
{
    /* High Speed Port 0,4 */
    {                    /* offs     len */
        /* fld en*/      {  0,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS}
        /* fld portNum*/,{  4,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS}
    }
    /* High Speed Port 1,5 */
   ,{                    /* offs  len */
        /* fld en*/      {  1,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS}
        /* fld portNum*/,{ 11,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS}
    }
    /* High Speed Port 2,6 */
   ,{                    /* offs  len */
        /* fld en*/      {  2,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS}
        /* fld portNum*/,{ 18,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS}
    }
    /* High Speed Port 3,7 */
   ,{                    /* offs  len */
        /* fld en*/      {  3,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS}
        /* fld portNum*/,{ 25,       PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS}
    }
};

/**
* @internal prvCpssDxChPortTxQUnitDQHighSpeedPortEnable function
* @endinternal
*
* @brief   TxQ Unit DQ : assign High Speed Port to specific TxQ port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
* @param[in] portNum                  - txQ port num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortEnable
(
    IN GT_U8       devNum,
    IN GT_U32      highSpeedPortIdx,
    IN GT_PORT_NUM portNum
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32    regIdx;
    GT_U32    portIdxPerReg;
    PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC * prvTxQDqHighSpeedPortPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (portNum >= 1<<PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2B0TxQDqHighSpeedPort[portIdxPerReg];
    }
    else
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2A0TxQDqHighSpeedPort[portIdxPerReg];
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.highSpeedPorts[regIdx];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            prvTxQDqHighSpeedPortPtr->fldPortNum.offs,
                                            prvTxQDqHighSpeedPortPtr->fldPortNum.len,
                                            portNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            prvTxQDqHighSpeedPortPtr->fldEn.offs,
                                            prvTxQDqHighSpeedPortPtr->fldEn.len,
                                            1);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitDQHighSpeedPortDisable function
* @endinternal
*
* @brief   TxQ Unit DQ : disable High Speed Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortDisable
(
    IN GT_U8       devNum,
    IN GT_U32      highSpeedPortIdx
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32    regIdx;
    GT_U32    portIdxPerReg;
    PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC * prvTxQDqHighSpeedPortPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2B0TxQDqHighSpeedPort[portIdxPerReg];
    }
    else
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2A0TxQDqHighSpeedPort[portIdxPerReg];
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.highSpeedPorts[regIdx];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            prvTxQDqHighSpeedPortPtr->fldEn.offs,
                                            prvTxQDqHighSpeedPortPtr->fldEn.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitDQHighSpeedPortGet function
* @endinternal
*
* @brief   TxQ Unit DQ : Get Configuration of High Speed Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon;
*
* @param[in] devNum                   - device number
* @param[in] highSpeedPortIdx         - idx of high speed port
*
* @param[out] isEnabledPtr             - (pointer on) is enabled
* @param[out] portNumPtr               - pointer to TxQ port assigned to this high speed port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       highSpeedPortIdx,
    OUT GT_BOOL     *isEnabledPtr,
    OUT GT_PORT_NUM *portNumPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32    regIdx;
    GT_U32    portIdxPerReg;
    PRV_CPSS_DXCH_HIGHSPEED_REG_FIELDC_STC * prvTxQDqHighSpeedPortPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(isEnabledPtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2B0TxQDqHighSpeedPort[portIdxPerReg];
    }
    else
    {
        if (highSpeedPortIdx >= PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regIdx        = highSpeedPortIdx / PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        portIdxPerReg = highSpeedPortIdx % PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS;
        prvTxQDqHighSpeedPortPtr = &prvBobcat2A0TxQDqHighSpeedPort[portIdxPerReg];
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.highSpeedPorts[regIdx];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            prvTxQDqHighSpeedPortPtr->fldPortNum.offs,
                                            prvTxQDqHighSpeedPortPtr->fldPortNum.len,
                                            /*OUT*/portNumPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            prvTxQDqHighSpeedPortPtr->fldEn.offs,
                                            prvTxQDqHighSpeedPortPtr->fldEn.len,
                                            /*OUT*/&regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    *isEnabledPtr = (GT_BOOL)regVal;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxQUnitDQHighSpeedPortAllConfigure function
* @endinternal
*
* @brief   TxQ Unit DQ : Configure All HighSpeedPort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2;
*
* @param[in] devNum                   - device number
* @param[in] numberOfHighSpeedPorts   - number of TxQ ports to configure as High Speed
* @param[in] highSpeedPortsPortArr    - array of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortAllConfigure
(
    IN GT_U8  devNum,
    GT_U32    numberOfHighSpeedPorts,
    GT_U32    *highSpeedPortsPortArr
)
{
    GT_STATUS rc;
    GT_U32 highSpeedPortIdx;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (numberOfHighSpeedPorts != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(highSpeedPortsPortArr);
    }
    if (numberOfHighSpeedPorts > CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* enable all declared */
    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < numberOfHighSpeedPorts; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortEnable(devNum,highSpeedPortIdx,highSpeedPortsPortArr[highSpeedPortIdx]);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    /* disable all others */
    for (highSpeedPortIdx = numberOfHighSpeedPorts ; highSpeedPortIdx < CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitLLHighSpeedPortDisable(devNum,highSpeedPortIdx);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* all ports above CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS are already diabled */
    return GT_OK;
}


/**
* @internal prvCpssDxChPortTxQUnitDQHighSpeedPortDisableAll function
* @endinternal
*
* @brief   TxQ Unit DQ : disable All HighSpeedPort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2;
*
* @param[in] devNum                   - device number
*                                      numberOfHighSpeedPorts - number of TxQ ports to configure as High Speed
*                                      highSpeedPortsPortArr  - array of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortDisableAll
(
    IN GT_U8      devNum
)
{
    GT_STATUS rc;
    GT_U32 highSpeedPortIdx;
    GT_U32 maxIdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    maxIdx = (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) ? PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS :
                                                     PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS;

    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < maxIdx; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortDisable(devNum,highSpeedPortIdx);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/*-----------------------------------------------------------------------------------
 *
 *   * Port Shaper
 *     -----------
 *   * Port Request Mask
 *       /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Port Shaper/Port Request Maskf
 *       address : 0x40001904
 *
 *
 *
 *       4 fields PortReqMask0-3, each one 8 bits
 *       0 - 7 PortReqMask<%d>    Defines the minimum number of core clock cycles between two consecutive service grants to the same port.
 *                                This configuration is useful when Deficit mode is enabled to reduce the potential amount of burstiness.
 *                                NOTE: The minimum value is 0x3 for regular ports, for high speed ports it is 0x2
 *-----------------------------------------------------------------------------------*/


#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS 4
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS     8

#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_0_OFFS_CNS 0
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_1_OFFS_CNS 8
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_2_OFFS_CNS 16
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_3_OFFS_CNS 24


#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_REGULAR_PORT_VAL_CNS        4
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_HIGH_SPEED_PORT_VAL_CNS     4

#define PRV_CPSS_DXCH_TXQDQ_PORT_BC3_REQUEST_MASK_HIGH_SPEED_PORT_VAL_CNS 2


#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS    0
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_HIGH_SPEED_PORT_CNS 3

static GT_STATUS prvCpssDxChPortTxQUnitDQPortRequestMaskSet
(
    IN GT_U8  devNum,
    IN GT_U32  dqIdx,
    IN GT_U32 maskId,
    IN GT_U32 maskVal
)
{
    GT_STATUS  rc;
    GT_U32 regAddr;
    GT_U32 offs;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
    {
        if (dqIdx >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (dqIdx != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (maskId >= PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dqIdx).scheduler.portShaper.portRequestMask;
    offs    = PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS*maskId;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr
                                            ,offs
                                            ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS
                                            ,maskVal);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortTxQUnitDQPortRequestMaskGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 dqIdx,
    IN  GT_U32 maskId,
    OUT GT_U32 *maskValPtr
)
{
    GT_STATUS  rc;
    GT_U32 regAddr;
    GT_U32 offs;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(maskValPtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
    {
        if (dqIdx >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (dqIdx != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (maskId >= PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dqIdx).scheduler.portShaper.portRequestMask;
    offs    = PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS*maskId;
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr
                                            ,offs
                                            ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS
                                            /*OUT*/,maskValPtr);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}



static GT_STATUS prvCpssDxChPortTxQUnitDQPortRequestMaskInit
(
    GT_U8 devNum
)
{
    GT_STATUS  rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (   PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum)
        || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) || PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        /* Prepare request mask N 3 for high speed port, i.e. set its value to PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_HIGH_SPEED_PORT_CNS */
        rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSet(devNum
                                                        ,0
                                                        ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_HIGH_SPEED_PORT_CNS
                                                        ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_HIGH_SPEED_PORT_VAL_CNS);
        if (GT_OK != rc )
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        return GT_OK;
    }
    /* BC3/Aldrin2 */
    {
        GT_U32 dpIdx;
        GT_U32 reqMaskIdx;

        for (dpIdx = 0; dpIdx < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq; dpIdx++)
        {
            for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
            {
                rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSet(devNum
                                                                ,dpIdx
                                                                ,reqMaskIdx
                                                                ,PRV_CPSS_DXCH_TXQDQ_PORT_BC3_REQUEST_MASK_HIGH_SPEED_PORT_VAL_CNS);
                if (GT_OK != rc )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
        return GT_OK;
    }
}

/*-----------------------------------------------------------------------------------
 *    * Port Request Mask Selector
 *      /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Port Shaper/Port <%n> Request Mask Selector
 *      address: 0x4000190C
 *      offset : 0x4000190C + 0x4*n: where n (0-71) represents Port (TxQ port !!!)
 *
 *
 *      Set masj N 3 == 0 ---> High Speed Port Mask
 *      Port request Mask selector -->
 *                 3 --> High Speed Port
 *                 0 --> Regular Port
 *----------------------------------------------------------------------------------*/

#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_SELECTOR_OFFS_CNS 0
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_SELECTOR_LEN_CNS  2

static GT_STATUS prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet
(
    IN GT_U8  devNum,
    IN GT_U32 txqPortNum,
    IN GT_U32 maskId
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 txQPortsMax;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    txQPortsMax = sizeof(PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.portShaper.portRequestMaskSelector)/
                    sizeof(PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.portShaper.portRequestMaskSelector[0]);
    if (txqPortNum >= txQPortsMax)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (maskId >= PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.portShaper.portRequestMaskSelector[txqPortNum];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr
                                            ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_SELECTOR_OFFS_CNS
                                            ,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_SELECTOR_LEN_CNS
                                            ,maskId);
    if (GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/*

Configure Procedure
Input
   List Of High Speed Ports to configure
1.  Get currently defined High speed Port List
2.  Build
    1. List of excluded from High Speed Port ( Current that not in "to configure" )
    2. List of new High Speed Ports ("to configure" that are not in current)
3.  For all ports in list of excluded set
    1.  Mask Selector <portN> = 0  (uses port mask request 0)
    2.  Port<portN> Scheduler Profile = 0 (uses default port profile)
4.  For all ports in list of new high speed ports
    1.  Mask Selector <portN> = 3  (uses port mask request 3)
    2.  Port<portN> Scheduler Profile = 15 (uses port profile 15)
5.  For all <idx,ports> in configure List
       High speed ports <idx> enable, port = 1, port
    for idx = portN; idx < 2; idx++
       High speed ports <idx> enable, port = 0,

*/


typedef struct PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC
{
    GT_U32 num;
    GT_U32 portList[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
    GT_U32 highSpeedPortIdxArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
}PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC;

static GT_STATUS PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Init
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr
)
{
    GT_U32 i;
    listPtr->num = 0;
    for (i = 0 ; i < CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS; i++)
    {
        listPtr->portList[i]            = PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
        listPtr->highSpeedPortIdxArr[i] = PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
    }
    return GT_OK;
}

static GT_U32 PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_FindByPortNum
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr,
    GT_U32 portNum
)
{
    GT_U32 i;

    for (i = 0 ; i < CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS; i++)
    {
        if (listPtr->portList[i] == portNum)
        {
            return i;
        }
    }
    return PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
}

static GT_U32 PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_FindEmpty
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr
)
{
    GT_U32 i;

    for (i = 0 ; i < CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS; i++)
    {
        if (listPtr->portList[i] == PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS)
        {
            return i;
        }
    }
    return PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
}



static GT_STATUS PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Set
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr,
    GT_U32 idx,
    GT_U32 portNum,
    GT_U32 highSpeedPortIdx
)
{
    if (idx >= CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    listPtr->portList[idx]            = portNum;
    listPtr->highSpeedPortIdxArr[idx] = highSpeedPortIdx;
    listPtr->num++;
    return GT_OK;
}

static GT_STATUS PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Clear
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr,
    GT_U32 idx
)
{
    listPtr->highSpeedPortIdxArr[idx] = PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
    listPtr->portList[idx]            = PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS;
    listPtr->num--;
    return GT_OK;
}



static GT_STATUS PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Append
(
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC * listPtr,
    GT_U32 portNum,
    GT_U32 highSpeedPortIdx
)
{
    if (listPtr->num >= CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    listPtr->portList           [listPtr->num] = portNum;
    listPtr->highSpeedPortIdxArr[listPtr->num] =  highSpeedPortIdx;
    listPtr->num++;
    return GT_OK;
}



/**
* @internal prvCpssDxChPortDynamicPATxQHighSpeedPortSet function
* @endinternal
*
* @brief   TxQ assign high speed ports to TxQ ports
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon;
*
* @param[in] devNum                   - device number
* @param[in] operation                - enum member
* @param[in] highSpeedPortNumber - number of TxQ ports to configure as High Speed
* @param[in] highSpeedPortArr       - array of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - bad pointer
* @retval GT_FAIL                  - on error
*
* @note BOBCAT2 B0: only txq port 64 can be declared as High Speed Port
*
*/
GT_STATUS prvCpssDxChPortDynamicPATxQHighSpeedPortSet
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ENT operation,
    IN  GT_U32                                      highSpeedPortNumber,
    IN  GT_U32                                      highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
)
{
    GT_STATUS rc;
    GT_U32    oldPortNum;
    GT_BOOL   oldIsEnabled;
    GT_U32    i;
    GT_U32    place;
    GT_U32    highSpeedPortIdx;
    GT_U32    portNum;
    GT_U32    var_PRV_CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM;

    static PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC toBeConfiguredPortList;
    static PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_STC currPortList;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    if (highSpeedPortNumber > 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(highSpeedPortArr);
    }

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (highSpeedPortNumber > 1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (highSpeedPortNumber == 1)
        {
            if (highSpeedPortArr[0] != PRV_CPSS_DXCH_PORT_NUM_TM_TxQ_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        var_PRV_CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM = 1;
    }
    else
    {
        var_PRV_CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM = PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS;
    }
   /*-----------------------------------------------------------------------------
    * BOBCAT2 A0 :                                                                *
    * I take as advantage that there is SINGLE  High Speed Port in the System     *
    *                                  -------                                    *
    * port is port TxQ port 64 !!!                                                *
    *
    * therefore I read whether any port is declared as high speed                 *
    * in general case one need to read all port declared as high speed previously *
    * take symmetic difference of new and old                                     *
    * for old (not in new) mask selector shall be set to 0 (regular port)         *
    * for new (not in old) mask selector shall be set to 3 (high speed port)      *
    * Whether this operation shall be done under traffic ?                        *
    * Answer : Yes                                                                *
    * Algo :
    *    0. Given list of ports to be set as HighSpeed -List                      *
    *    1. build bit map of all (configured and not) ports -                     *
    *                <0 , portN >                                                 *
    *                 1 ,  x                                                      *
    *    2. already configured = all ports that appears at bitMap & List - do nothing
    *    3. to be excluded : bitMap \ List
    *            - change mask selector to regular
    *                    prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(devNum,oldPortNum,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS);
    *            - clear that port in bitmap
    *    4. to be included : List \ bitMap
    *            -
    *-----------------------------------------------------------------------------*/
    if (highSpeedPortNumber > CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    /*-----------------------------------------------------------------------------------------------------*/
    /* build list of ports to be configured                                                                */
    /*    high speed port idx currently undefined , therefore use PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS*/
    /*-----------------------------------------------------------------------------------------------------*/
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Init(&toBeConfiguredPortList);
    for (i = 0; i < highSpeedPortNumber; i++)
    {
        rc = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Append(&toBeConfiguredPortList,highSpeedPortArr[i],PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /*----------------------------------------------------------------------------*/
    /* build map of already configured port                                        */
    /* when port is not mapped use PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS      */
    /*----------------------------------------------------------------------------*/
    PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Init(&currPortList);
    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < var_PRV_CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortGet(devNum,highSpeedPortIdx,&oldIsEnabled,&oldPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (oldIsEnabled == GT_TRUE)
        {
            rc = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Set(&currPortList,highSpeedPortIdx,oldPortNum,highSpeedPortIdx);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*--------------------------------------------------------------------------*/
    /* check for port to be deleted                                              */
    /*  i.e. all port already configured (in map), that not present in list     */
    /*--------------------------------------------------------------------------*/
    for (highSpeedPortIdx = 0 ; highSpeedPortIdx < var_PRV_CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM; highSpeedPortIdx++)
    {
        portNum = currPortList.portList[highSpeedPortIdx];
        if (portNum != PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS)
        {
            place = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_FindByPortNum(&toBeConfiguredPortList,portNum);
            if (((operation == PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_REMOVE_E) &&                                         /* operation: remove */
                 (place != PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS)) ||   /* found: delete it !!! */
                ((operation == PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ASSIGN_E) &&                                        /* operation: assign */
                 (place == PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS)))     /* not found, delete it !!! */
            {
                if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
                {
                    rc = prvCpssDxChPortTxQUnitLLHighSpeedPortDisable(devNum, highSpeedPortIdx);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChPortTxQUnitDQHighSpeedPortDisable(devNum,highSpeedPortIdx);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(devNum,portNum,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                /* mark current High Speed port as empty */
                rc = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Clear(&currPortList,highSpeedPortIdx);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /* now configure ports that are still not configured */
    for (i = 0 ; i < toBeConfiguredPortList.num; i++)
    {
        GT_U32 place;

        portNum = toBeConfiguredPortList.portList[i];
        place = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_FindByPortNum(&currPortList, portNum);
        if (((operation == PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ADD_E) ||
             (operation == PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ASSIGN_E)) && /* operation: assign or add */
            (place == PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS))   /* not found, configure !!! */
        {
            /* find empty place */
            highSpeedPortIdx = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_FindEmpty(&currPortList);
            if (highSpeedPortIdx == PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            /* mark port as used */
            rc = PRV_CPSS_DXCH_HIGHSPEED_PORT_LIST_Set(&currPortList,highSpeedPortIdx,portNum,highSpeedPortIdx);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
            {
                rc = prvCpssDxChPortTxQUnitLLHighSpeedPortEnable(devNum,highSpeedPortIdx,portNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChPortTxQUnitDQHighSpeedPortEnable(devNum,highSpeedPortIdx,portNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(devNum,portNum,PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_HIGH_SPEED_PORT_CNS);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}





/**
* @internal prvCpssDxChPortDynamicPATxQHighSpeedPortInit function
* @endinternal
*
* @brief   TxQ High Speed ports Init
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note BC2/BobK/Aldrin have high speed ports
*       BC3 -- all port are high speed
*       ==> NO TXQ-LL-High-speed-port
*       ==> NO TXQ-DQ-High-speed-port
*       therefore just port request mask shall be initialized
*
*/
GT_STATUS prvCpssDxChPortDynamicPATxQHighSpeedPortInit
(
    IN  GT_U8  devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /*----------------------------------------*
     *  BC2/BobK/Aldrin have high speed ports *
     *  BC3/Aldrin2 -- all port are high speed   *
     *   ==> NO TXQ-LL-High-speed-port        *
     *   ==> NO TXQ-DQ-High-speed-port        *
     *  therefore just port request mask shall be initialized
     *----------------------------------------*/
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        rc = prvCpssDxChPortTxQUnitLLHighSpeedPortDisableAll(devNum);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortDisableAll(devNum);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

    }

    rc = prvCpssDxChPortTxQUnitDQPortRequestMaskInit(devNum);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }
#if 0
    /*-----------------------------------------*
     * BC3 : set all txq ports to use mask N 3 *
     *-----------------------------------------*/
    if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum))
    {
        GT_U32 dpIdx;
        GT_U32 reqMaskIdx;
        GT_U32 txqPort;
        GT_U32 globalTxQPort;

        for (dpIdx = 0; dpIdx < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq; dpIdx++)
        {
            for (txqPort = 0 ; txqPort < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp; txqPort++)
            {
                globalTxQPort = SIP_5_20_LOCAL_TXQ_DQ_PORT_TO_GLOBAL_TXQ_DQ_PORT_MAC(devNum,txqPort,dpIdx);
                rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(devNum,globalTxQPort,3);
                if (GT_OK != rc )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
#endif
    return GT_OK;
}



/**
* @internal prvCpssDxChPortHighSpeedPortSet function
* @endinternal
*
* @brief   High Speed ports Set (override default configuration by pizza arbiter)
*         get as input local physical ports
*         ports that have no TM setting are configured as high speed TxQ port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon;
*
* @param[in] devNum                   - device number
* @param[in] operation                - enum member
* @param[in] highSpeedPortNumber - number of physical ports to configure as High Speed
* @param[in] highSpeedPortArrPtr  - array of physical ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortHighSpeedPortSet
(
    IN  GT_U8                                        devNum,
    IN  PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ENT  operation,
    IN  GT_U32                                       highSpeedPortNumber,
    IN  GT_PHYSICAL_PORT_NUM                         *highSpeedPortArrPtr
)
{
    GT_STATUS rc;
    GT_U32    i;

    GT_PHYSICAL_PORT_NUM               highSpeedTxQPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
    GT_U32                             highSpeedTxQPortNumber;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (highSpeedPortNumber > CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS) /* max supported txq port to be configured as high speed*/
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    highSpeedTxQPortNumber = 0;
    for (i = 0 ; i < highSpeedPortNumber; i++)
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifMode;
        CPSS_PORT_SPEED_ENT          speed;

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,highSpeedPortArrPtr[i],/*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapShadowPtr->valid == GT_FALSE) /* non mapped port */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (operation == PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ASSIGN_E)
        {
            /* Only on assign operation: check valid ifMode / speed */
            ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);
            speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,   portMapShadowPtr->portMap.macNum);

            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode && CPSS_PORT_SPEED_NA_E != speed) ||
                (CPSS_PORT_INTERFACE_MODE_NA_E != ifMode && CPSS_PORT_SPEED_NA_E == speed)
              )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something bad at system  both shall either valid or NA */
            }

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode && CPSS_PORT_SPEED_NA_E == speed) /* non active port can not be high speed port -- non-sense */
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); /* just ethernet mapping */
        }
        if ((portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE) &&
            (portMapShadowPtr->portMap.txqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS))
        {
            highSpeedTxQPortArr[highSpeedTxQPortNumber] = portMapShadowPtr->portMap.txqNum;
            highSpeedTxQPortNumber++;
        }
    }

    rc = prvCpssDxChPortDynamicPATxQHighSpeedPortSet(devNum,operation,highSpeedTxQPortNumber,&highSpeedTxQPortArr[0]);
    if (rc != GT_OK)
    {
        return  rc;
    }
    return GT_OK;
}


/*---------------------------------------------------------------------------*
 * Applicative part for BobK devices                                         *
 *---------------------------------------------------------------------------*/

typedef struct
{
    GT_PHYSICAL_PORT_NUM               highSpeedTxQPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
    GT_U32                             highSpeedTxQPortNumber;
    GT_U32                             maxHighSpeedTxQPortNumber;
}PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC;

static GT_STATUS prvCpssDxChHighSpeedTxQPortListInit
(
    PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC * listPtr
)
{
    GT_U32 i;
    listPtr->highSpeedTxQPortNumber = 0;
    listPtr->maxHighSpeedTxQPortNumber = CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS;
    for (i = 0 ; i < listPtr->maxHighSpeedTxQPortNumber; i++)
    {
        listPtr->highSpeedTxQPortArr[i] = 0;
    }
    return GT_OK;
}

static GT_STATUS prvCpssDxChHighSpeedTxQPortListAppend
(
    IN PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC * listPtr,
    IN GT_U32 txqPort
)
{
    GT_U32 i;
    /* check whether it already exists */
    for (i = 0 ; i < listPtr->highSpeedTxQPortNumber; i++)
    {
        if (listPtr->highSpeedTxQPortArr[i] == txqPort)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }
    }
    /* check whether list is full */
    if (listPtr->highSpeedTxQPortNumber == listPtr->maxHighSpeedTxQPortNumber)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    listPtr->highSpeedTxQPortArr[listPtr->highSpeedTxQPortNumber] = txqPort;
    listPtr->highSpeedTxQPortNumber++;

    return GT_OK;
}

static GT_STATUS prvCpssDxChBobKHighSpeedPortListOfTxQCorrespondToHighCapacityMacsBuild
(
    IN  GT_U8 devNum,
    IN  GT_U32 *bobk_listOfPossibleMacs,
    OUT PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC *listPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 maxPortNum;
    GT_U32 portMacNum;
    GT_U32 portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    maxPortNum  = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for (i = 0 ; bobk_listOfPossibleMacs[i] != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS; i++)
    {
        portMacNum = bobk_listOfPossibleMacs[i];
        /* find port having that mac,not mapped to TM, and take its TxQ */
        for (portNum = 0; portNum < maxPortNum; portNum++)
        {
            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portMapShadowPtr->valid == GT_TRUE )
            {
                if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
                {
                    if (portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE)
                    {
                        if (portMapShadowPtr->portMap.macNum == portMacNum)
                        {
                            /* cascade port has no TXQ , therefore it cannot be declared as high speed port at TXQ */
                            if (portMapShadowPtr->portMap.txqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
                            {
                                rc = prvCpssDxChHighSpeedTxQPortListAppend(listPtr,portMapShadowPtr->portMap.txqNum);
                                if (rc != GT_OK)
                                {
                                    return rc;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return GT_OK;
}


static GT_STATUS prvCpssDxChBobKHighSpeedPortTMTxQIfExistsAppend
(
    IN  GT_U8 devNum,
    OUT PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC *listPtr
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_U32 portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    maxPortNum  = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    /*------------------------------------------------------------------------------------------------*
     * now check whether TM is enabled, i.e. there exists at least one ETHERNET port with enabled TM  *
     *------------------------------------------------------------------------------------------------*/
    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapShadowPtr->valid == GT_TRUE )
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
                {
                    rc = prvCpssDxChHighSpeedTxQPortListAppend(listPtr,portMapShadowPtr->portMap.txqNum);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    return GT_OK;
                }
            }
        }
    }
    return GT_OK;
}

static GT_U32 bobk_listOfPossibleMacs[]   = {  56,58,64,66,68,70, CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS  };

/**
* @internal prvCpssDxChBobKHighSpeedPortConfigure function
* @endinternal
*
* @brief   define high speed ports for BobK Cetus/Caelum device
*         currently all ports TXQ corresponding to MAC that are capable to support 20G and above
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChBobKHighSpeedPortConfigure
(
    IN  GT_U8                 devNum
)
{
    GT_STATUS rc;

    /* shall be changed */
    static PRV_CPSS_DXCH_HIGH_SPEED_TXQ_PORT_LIST_STC list;
    GT_U32 *listOfPossibleMacsPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    rc = prvCpssDxChHighSpeedTxQPortListInit(&list);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------*
     * BC2    : single high speed port for TM           *
     * BobK   : all port having capacity 20G and higher *
     * Aldrin : all port having capacity 40G and higher *
     *--------------------------------------------------*/
    listOfPossibleMacsPtr = (GT_U32*)NULL;
    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        listOfPossibleMacsPtr = &bobk_listOfPossibleMacs[0];
    }
    /*------------------------------------------------------------------------------------------------*
     * build list of all TxQ ports that are mapped to some any physical port with MAC from list above *
     *------------------------------------------------------------------------------------------------*/
    if ( listOfPossibleMacsPtr != NULL)
    {
        rc = prvCpssDxChBobKHighSpeedPortListOfTxQCorrespondToHighCapacityMacsBuild(devNum,&listOfPossibleMacsPtr[0],/*OUT*/&list);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /*------------------------------------------------------------------------------------------------*
     * now check whether TM is enabled, i.e. there exists at least one ETHERNET port with enabled TM  *
     * BC2/BobK -- TM exists,                                                                         *
     * Aldrin   -- NO                                                                                 *
     *------------------------------------------------------------------------------------------------*/
    rc = prvCpssDxChBobKHighSpeedPortTMTxQIfExistsAppend(devNum,/*OUT*/&list);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*------------------------------------------------------------------------------------------------*
     * now configure High Speed TxQ Ports                                                             *
     *------------------------------------------------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPATxQHighSpeedPortSet(
        devNum,
        PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ASSIGN_E,
        list.highSpeedTxQPortNumber,
        &list.highSpeedTxQPortArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


