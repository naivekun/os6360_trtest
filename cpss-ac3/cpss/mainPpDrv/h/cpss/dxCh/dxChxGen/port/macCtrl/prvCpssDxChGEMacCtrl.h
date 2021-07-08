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
* @file prvCpssDxChGEMacCtrl.h
*
* @brief bobcat2 GE mac control API
*
* @version   3
********************************************************************************
*/

#ifndef PRV_CPSS_DXCH_GE_MAC_CTRL_H
#define PRV_CPSS_DXCH_GE_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChGEMacCtrl.h
*
* DESCRIPTION:
*       bobcat2 GE mac control 
*
* FILE REVISION NUMBER:
*       $Revision: 3$
*******************************************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>


/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register1
 * 15 - 15  Short preable    0x0 -- 8 bytes
 *                           0x1 -- 4 bytes
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register3
 * 6-14     IPG 
 *---------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<XLG MAC IP> XLG MAC IP Units%p/Port MAC Control Register5
 * 
 * 
 *  0 -  3  TxIPGLength          minimal vaue is 8 for 10G and 40G
 *  4 -  6  PreambleLengthTx     0 -- 8 bytes 
 *                               1..7 -- 1..7-bytes
 *                               for 10G 4,8 are only allowed 
 *  7 -  9  PreambleLengthRx     0 -- 8 bytes 
 *                               1..7 -- 1..7-bytes
 * 10 - 12  TxNumCrcBytes        legal value 1,2,3,4
 * 13 - 15  RxNumCrcBytes        legal value 1,2,3,4
 *---------------------------------------------------------------------------------------------
 */

/**
* @internal prvCpssDxChBobcat2PortMacGigaIsSupported function
* @endinternal
*
* @brief   check whether GE mac is supported for specific mac
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*                                      portNum  - physical port number
*
* @param[out] isSupportedPtr           - pointer to is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacGigaIsSupported
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     mac,
    OUT GT_BOOL                 *isSupportedPtr
);

/**
* @internal prvCpssDxChBobcat2PortGigaMacIPGLengthSet function
* @endinternal
*
* @brief   GE mac set IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortGigaMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChBobcat2PortGigaMacIPGLengthGet function
* @endinternal
*
* @brief   GE mac set IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
*
* @param[out] lengthPtr                - pointer to ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChBobcat2PortGigaMacIPGLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     mac,
    OUT GT_U32                  *lengthPtr
);

/**
* @internal prvCpssDxChBobcat2PortGigaMacPreambleLengthSet function
* @endinternal
*
* @brief   GE mac set preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/BOTH
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortGigaMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChBobcat2PortGigaMacPreambleLengthGet function
* @endinternal
*
* @brief   GE mac set preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
* @param[in] direction                - RX/TX/BOTH
*
* @param[out] lengthPtr                - pointer to preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortGigaMacPreambleLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     mac,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                   *lengthPtr
);


typedef enum 
{
     PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_SGMII_E = 0
    ,PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_1000BaseX_E
    ,PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_MAX
}PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_ENT;

GT_STATUS prvCpssDxChBobcat2PortGigaMacTypeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_ENT portType
);

GT_STATUS prvCpssDxChBobcat2PortGigaMacTypeGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT PRV_CPSS_DXCH_MAC_GE_PORT_TYPE_ENT *portTypePtr
);

/*-------------------------------------------------------*/

GT_STATUS prvCpssDxChBobcat2PortGigaMacAnMasterModeSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL  mode
);

GT_STATUS prvCpssDxChBobcat2PortGigaMacAnMasterModeGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL *modePtr
);
/*-------------------------------------------------------*/

GT_STATUS prvCpssDxChPortAutoNegLinkPartnerCodeWordGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 *codewordPtr
);

#define PRV_CPSS_DXCH_AN_CONF_FORCE_LINK_DOWN_OFFS_CNS        0
#define PRV_CPSS_DXCH_AN_CONF_FORCE_LINK_UP_OFFS_CNS          1
#define PRV_CPSS_DXCH_AN_CONF_INBAND_AN_EN_OFFS_NS            2
#define PRV_CPSS_DXCH_AN_CONF_INBAND_AN_BYPASS_EN_OFFS_CNS    3
#define PRV_CPSS_DXCH_AN_CONF_GMII_SPEED_OFFS_CNS             5
#define PRV_CPSS_DXCH_AN_CONF_MII_SPEED_OFFS_CNS              6
#define PRV_CPSS_DXCH_AN_CONF_AN_SPEED_EN_OFFS_CNS            7
#define PRV_CPSS_DXCH_AN_CONF_PAUSE_ADV_OFFS_CNS              9
#define PRV_CPSS_DXCH_AN_CONF_PAUSE_ASM_ADV_OFFS_CNS         10
#define PRV_CPSS_DXCH_AN_CONF_AN_FLOW_CTRL_EN_OFFS_CNS       11
#define PRV_CPSS_DXCH_AN_CONF_FULL_DUPLEX_MODE_OFFS_CNS      12
#define PRV_CPSS_DXCH_AN_CONF_AN_FULL_DUPLEX_EN_OFFS_CNS     13

typedef struct 
{
    GT_BOOL ForceLinkDown    ;
    GT_BOOL ForceLinkUp      ;
    GT_BOOL InBandAnEn       ;
    GT_BOOL InBandAnByPassEn ;
    GT_BOOL SetMIISpeed      ; 
    GT_BOOL SetGMIISpeed     ;   
    GT_BOOL AnSpeedEn        ;   
    GT_BOOL PauseAdv         ;   
    GT_BOOL PauseAsmAdv      ;    
    GT_BOOL AnFcEn           ;        
    GT_BOOL SetFullDuplex    ; 
    GT_BOOL AnDuplexEn       ;    
}PRV_CPSS_DXCH_PORT_AN_CONFIG_RES_STC;


GT_STATUS prvCpssDxChPortAutoNegConfigGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum,
    OUT PRV_CPSS_DXCH_PORT_AN_CONFIG_RES_STC *anConfigPtr
);


GT_STATUS prvCpssDxChPortAutoNegPauseAdvPauseAsmAdvGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum,
    OUT GT_BOOL *pauseAdvPtr,
    OUT GT_BOOL *pauseAsmAdvPtr
);


GT_STATUS prvCpssDxChPortAutoNegBypassActGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *inBandAnPypassActPtr
);


GT_STATUS prvCpssDxChPortAutoNegSyncChangeStateGet
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL * portSyncOKPtr,
    OUT GT_BOOL * syncFail10MsPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

