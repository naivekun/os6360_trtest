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
* @file prvCpssDxChPortTxQHighSpeedPortsDrv.h
*
* @brief bobcat2 and higher TxQ high speed port support
*
* @version   1
********************************************************************************
*/

#ifndef __PRV_CPSS_DXCH_PORT_TXQ_HIGH_SPEED_PORTS_DRV_H
#define __PRV_CPSS_DXCH_PORT_TXQ_HIGH_SPEED_PORTS_DRV_H

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortTxQHighSpeedPortsDrv.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*
#define PRV_CPSS_DXCH_PORT_BC2_A0_HIGH_SPEED_PORT_NUM_CNS 2
#define PRV_CPSS_DXCH_PORT_BC2_B0_HIGH_SPEED_PORT_NUM_CNS 8
*/

/*-------------------------------------------------------------------------------
 * Genaral design
 *
 *
 * 1.  DQ Unit
 *     a.  Scheduler profile:
 *         1. /Cider/EBU-SIP/TXQ_IP/SIP5.0 (Bobcat2)/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Priority Arbiter Weights/Port<%n> Scheduler Profile
 *         2. Total 72 register (per TxQ ports ?)
 *     3.  Total 16 profiles. Each port is assigned a single profile. Several ports may share same profile.
 *         b.  Enable Fast Scheduler
 *             1.  /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Global/Global DQ Config/Profile <%p> Byte count modification register   field Enable High Speed Scheduler Profile <%p>
 *             2.       Total 16 profiles
 *
 * When particular port is declared as <High Speed>, Its <Enable Fast Scheduler> shall be set to 1. You propose set <Enable Fast Scheduler> = 1 to all  profiles . Is that right ?
 *
 *         c.  Port Shaper
 *             1.  Port Request Mask
 *                 1. /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Port Shaper/Port Request Maskf
 *                 2. total 4 different masks
 *                 3. default value – 3 (Register misconfiguration changed to 4) , for High Speed Port mask shall be 0
 *
 *             2.  Port Request Mask Selector
 *                 1.  /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Scheduler/Port Shaper/Port <%n> Request Mask Selector
 *                 2.  total 72 (per TxQ Port )
 *                 3.  Default value 0 (? )
 *
 * Lets assign the mask #3 for High Speed Ports, than
 *     when port declared as <High Speed>, its  Port Request Mask Selector  shall be changed to #3  from default value
 *
 * Bobcat2 B0
 *-----------
 *   In order to enable fix the following configurations must be applied:
 *   1. 0x40000A00[16] = 0x1 (DQ Metal Fix Register)
 *
 *   /Cider/EBU-SIP/TXQ_IP/SIP5.0 (Bobcat2)/TXQ_IP_SIP5 {Current}/TXQ_IP_dq/Global/Debug Bus/DQ Metal Fix Register
 *   'Dq_Metal_Fix'  => 0x1ffff,   # enable MF
 *
 *   This should be done only if TM is enabled
 *
 *-------------------------------------------------------------------------------
 *   LL unit fast speed port config
 *-------------------------------------------------------------------------------
 *   /Cider/EBU-SIP/TXQ_IP/SIP5.0/TXQ_IP_SIP5 {Current}/TXQ_IP_ll/Global/Global LL Config/High Speed Port <%n>
 *   address        : 0x3d0a0008
 *   offset formula : 0x3d0a0008 + n*0x4: where n (0-7) represents port
 *   1 - 7 : high speed port <%p> port number
 *   0 - 0 : high speed port <%p> enable
 *----------------------------------------------------------------------------------------------------------------*/


/* DQ */
#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_EN_LEN_CNS          1
#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS    7


#define PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS  2   /* driver */
#define PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS     2
#define PRV_CPSS_DXCH_BOBCAT2_A0_PORT_TXQ_DQ_REG_NUM_CNS              1

#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_0_EN_OFFS_CNS       0
#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_0_PORT_NUM_OFFS_CNS 2


#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_1_EN_OFFS_CNS       1
#define PRV_CPSS_DXCH_PORT_TXQ_DQ_HIGH_SPEED_PORT_1_PORT_NUM_OFFS_CNS 9

#define PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS  8   /* driver */
#define PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_PORT_NUM_PER_REG_CNS     4
#define PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_REG_NUM_CNS              2




/* LL */
#define PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_NUM_CNS 8   /* driver */
#define PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_OFFS_CNS 1
#define PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_NUM_LEN_CNS  7
#define PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_OFFS_CNS  0
#define PRV_CPSS_DXCH_PORT_TXQ_LL_HIGH_SPEED_PORT_PORT_EN_LEN_CNS   1


#define PRV_CPSS_DXCH_HIGH_SPEED_PORT_INVALID_CNS (GT_U32)(~0)

/**
* @enum PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ENT
*
* @brief This enum defines operation on High Speed Ports Set.
*/
typedef enum
{
    /** add high speed ports to the set */
    PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ADD_E,

    /** remove high speed ports from the set */
    PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_REMOVE_E,

    /** assign set of high speed ports by new content */
    PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ASSIGN_E

} PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ENT;

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
);

/**
* @internal prvCpssDxChPortTxQUnitLLHighSpeedPortGet function
* @endinternal
*
* @brief   TxQ Unit LL : Get Configuration of High Speed Port
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
GT_STATUS prvCpssDxChPortTxQUnitLLHighSpeedPortGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      highSpeedPortIdx,
    OUT GT_BOOL     *isEnabledPtr,
    OUT GT_PORT_NUM *portNumPtr
);


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
);

/**
* @internal prvCpssDxChPortDynamicPATxQHighSpeedPortSet function
* @endinternal
*
* @brief   TxQ assign high speed ports to TxQ ports
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
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
    IN  GT_U8                                         devNum,
    IN  PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ENT   operation,
    IN  GT_U32                                        highSpeedPortNumber,
    IN  GT_U32                                        highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

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
);

/**
* @internal prvCpssDxChBobKHighSpeedPortConfigure function
* @endinternal
*
* @brief   define high speed ports for BobK Cetus/Caelum device
*         currently all TXQ ports corresponding to MAC that are capable to support 20G and above
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note BC2 shall include just TxQ port 64 (TM) stiil not done
*       BC3 : all TXQ ports are high speed , therefore this function shall not be called or return GT_OK
*
*/
GT_STATUS prvCpssDxChBobKHighSpeedPortConfigure
(
    IN  GT_U8                 devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

