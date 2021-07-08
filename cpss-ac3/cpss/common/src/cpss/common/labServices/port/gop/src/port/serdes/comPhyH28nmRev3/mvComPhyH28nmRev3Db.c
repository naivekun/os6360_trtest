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
* mvserdes28nmDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 11 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3Db.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortApCoCpuIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconGpSram.h>

/******************************** enums ***************************************/

/******************************** pre-declaration *****************************/

GT_STATUS hwsComPhyH28nmRev3InitSeqFlow(MV_CFG_SEQ* hwSerdesSeqDbArr);

/******************************** globals **************************************/

extern MV_CFG_SEQ *hwsSerdesSeqDbPtr; /* used for sequence set/get */

#ifndef CO_CPU_RUN

static MV_CFG_ELEMENT SerdesSpeed_1_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0330, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0066, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x013F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x003E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2681, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_3_125gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0440, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0088, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x013F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_3_75gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x04C8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F, }}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0099, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x013F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_4_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0550, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* DFE_RES = 3.3mv */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* TX_AMP=31, AMP_ADJ=1 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* MUPI/F=2, rx_digclkdiv=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00AA, 0xFFFF}}, /* set password */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* Set Gen_RX/TX */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* clock set    */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_dv_force=1  */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}}, /* Force ICP=7 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}}, /* rxdigclk_dv_force=1*/
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}
};

static MV_CFG_ELEMENT SerdesSpeed_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0198, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0033, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_5_625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0198, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0033, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_5_15625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0198, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                            
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E40, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x0033, 0xFFFF}}, /* set password */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                               
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0xE014, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2561, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0000, 0x0800}}, /*  select ffe table mode  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */                 
};

static MV_CFG_ELEMENT SerdesSpeed_6_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x05D8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* DFE_RES = 3.3mv */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xF047, 0xFFFF}}, /* DFE UPDAE all coefficient */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* TX_AMP=31, AMP_ADJ=1 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E49, 0xFFFF}}, /* MUPI/F=2, rx_digclkdiv=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00BB, 0xFFFF}}, /* set password */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* Set Gen_RX/TX */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* clock set    */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x0800, 0xFFFF}}, /* clk 8T enable for 10G and up */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_dv_force=1  */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}}, /* Force ICP=7 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}}, /* rxdigclk_dv_force=1*/
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}}, 
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2541, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}},
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}
};

static MV_CFG_ELEMENT SerdesSpeed_7_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x06E8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC26, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00DD, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2233, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x3C00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x3C00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x503E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2541, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_10_3125gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x0770, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x4CDE, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00EE, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x027F, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x0F91, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x0F91, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0x603E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_10_9375gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x07F8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00FF, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2119, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x1B00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0xD03E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_11_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x07F8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00FF, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2119, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x1B00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0xD03E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_11_5625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x07F8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00FF, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2119, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x1B00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0xD03E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_12_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x07F8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00FF, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2119, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x1B00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0xD03E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

static MV_CFG_ELEMENT SerdesSpeed_12_1875gSeq[] = {
    {WRITE_OP, {SERDES_UNIT,     0x0000, 0x07F8, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */                    
    {WRITE_OP, {SERDES_UNIT,     0x0028, 0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0004, 0xFD8C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0018, 0x4F00, 0xFFFF}}, /* squelch in */                                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x001C, 0xFC2A, 0xFFFF}}, /* DFE UPDAE all coefficient DFE_RES = 3.3mv */         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0034, 0x406C, 0xFFFF}}, /* g1_setting for Tx Tuning */                          
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0038, 0x1E52, 0xFFFF}}, /* g1_setting for Rx Tuning: MUPI/F=2, rx_digclkdiv=3 */
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0094, 0x1FFF, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0098, 0x00EE, 0xFFFF}}, /* set password */                                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0104, 0x2208, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0108, 0x243F, 0xFFFF}}, /* continues vdd_calibration */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0114, 0x46CF, 0xFFFF}}, /* EMPH0 enable, EMPH_mode=2 */                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0134, 0x004A, 0xFFFF}}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */                        
    {WRITE_OP, {SERDES_PHY_UNIT, 0x013C, 0xE028, 0xFBFF}}, /* Force ICP=8 */                                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0140, 0x1800, 0xFFFF}}, /* clk 8T enable for 10G and up */                      
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0154, 0x0087, 0xFFFF}}, /* rxdigclk_div_force=1  */                             
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0168, 0x6014, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x016C, 0x0014, 0xFFFF}}, /*  DTL_FLOOP_EN=0  */                                  
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0184, 0x10F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01A8, 0x4000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01AC, 0x8498, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01DC, 0x0780, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01E0, 0x03FE, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0214, 0x4418, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC   */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0220, 0x0400, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0228, 0x2FC0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0268, 0x8C02, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0278, 0x21F3, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0280, 0xC9F8, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x029C, 0x05BC, 0xFFFF}}, /* Tx amplitude Min & Max limits */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x02DC, 0x2119, 0xFFFF}}, /* Tx Tuning amplitude for Initialize mode */           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x031C, 0x0318, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0330, 0x012F, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0334, 0x0C03, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0338, 0x1B00, 0xFFFF}}, /* default_2 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x033C, 0x1B00, 0xFFFF}}, /* default_3 */                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0368, 0x1000, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x036C, 0x0AD9, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0378, 0x1800, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0418, 0xE737, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0420, 0x9CE0, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0440, 0xD03E, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0444, 0x2421, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0468, 0x0001, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x046C, 0xFC7C, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A0, 0x0104, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A4, 0x0302, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04A8, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x04AC, 0x0202, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x010C, 0x0830, 0xFFFF}}, /* txclk regulator threshold set */                     
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0110, 0x0F30, 0xFFFF}}, /* txdata regulator threshold set */                    
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0424, 0x0000, 0x0060}}, /* os_ph_step_size[1:0]= 0 */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x039C, 0x0559, 0xFFFF}},                                                         
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0244, 0x3F3E, 0xFFFF}}, /*  FFE table RC= 0F/1F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0248, 0x4F4E, 0xFFFF}}, /*  FFE table RC= 2F/3F    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x024C, 0x5F5E, 0xFFFF}}, /*  FFE table RC= 4F/0E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0250, 0x6F6E, 0xFFFF}}, /*  FFE table RC= 1E/2E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0254, 0x3D3C, 0xFFFF}}, /*  FFE table RC= 3E/4E    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0258, 0x4D4C, 0xFFFF}}, /*  FFE table RC= 0D/1D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x025C, 0x5D5C, 0xFFFF}}, /*  FFE table RC= 2D/3D    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0260, 0x2F2E, 0xFFFF}}, /*  FFE table RC= 4D/0C    */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x022C, 0x0800, 0x0800}}, /*  select ffe table mode  */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x01CC, 0x0451, 0xFFFF}}, /*  select smplr vcm=0.65V */                           
    {WRITE_OP, {SERDES_PHY_UNIT, 0x0170, 0xCD8E, 0x00FF}}, /*  update process calibration */                       
    {WRITE_OP, {SERDES_PHY_UNIT, 0x041C, 0x003F, 0x003F}}  /*  update cdr_EO_bypass_trash[0:5]=63 */               
};

/* SERDES_PT_AFTER_PATTERN_NORMAL */
static MV_CFG_ELEMENT serdesPtAfterPatternNormalParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0xE0,   0xF0}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x0,    0x8000}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x4000, 0x4000}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x0,    0x4000}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,            0x800,  0x800}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,            0x0,    0x800}}
};

/* SERDES_PT_AFTER_PATTERN_TEST_SEQ */
static MV_CFG_ELEMENT serdesPtAfterPatternTestParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,  0x80E0, 0x80F0}}
};

/* SERDES_CORE_RESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesCoreResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x20}},
};

/* SERDES_CORE_UNRESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesCoreUnResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x20,   0x20}},
};

/* SERDES_SD_RESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesSdResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x8}}
};

/* SERDES_SD_UNRESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesSdUnResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x8,    0x8}}
};

/* SERDES_RF_RESET Sequence init */
static MV_CFG_ELEMENT serdesRfResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x40}}
};

/* SERDES_RF_UNRESET Sequence init */
static MV_CFG_ELEMENT serdesRfUnResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x40,   0x40}}
};

/* SERDES_SYNCE_RESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesSynceResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,  0x0,    0x8}}
};

/* SERDES_SYNCE_UNRESET_SEQ Sequence init */
static MV_CFG_ELEMENT serdesSynceUnResetParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0xDD00, 0xFF00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,  0xB,    0xB}}
};

/* SERDES_SERDES_POWER_DOWN_CTRL_SEQ Sequence init */
static MV_CFG_ELEMENT serdesPowerDownCtrlParams[] =
{
    {WRITE_OP, {SERDES_UNIT,   SERDES_EXTERNAL_CONFIGURATION_0,    0x0,    0x1802}},
    {WRITE_OP, {SERDES_UNIT,   SD_METAL_FIX,                       0x100,  0xFFFF}},
    {WRITE_OP, {SERDES_UNIT,   SERDES_RESERVED_REGISTER_2,         0x0,    0xFFFF}}
};

/* SERDES_SERDES_RXINT_UP Sequence init */
static MV_CFG_ELEMENT serdesRxIntUpParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_PHY_Isolate_Reg,     0x400,  0x400}},
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_SEL_BITS_Reg,        0x40,   0x40 }},
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_PHY_Isolate_Reg,     0,      0x400}}
};

/* SERDES_SERDES_RXINT_DOWN Sequence init */
static MV_CFG_ELEMENT serdesRxIntDownParams[] =
{
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,  0x10 }},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PHY_Isolate_Reg,   0,  0x400}}
};

/* SERDES_SERDES_WAIT_PLL Sequence init */
static MV_CFG_ELEMENT serdesWaitPllParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0X8000, 0x8000}},
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0x4000, 0x4000}},
    {WRITE_OP, {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0,      0x8000}},
};

/* SERDES_SD_LPBK_NORMAL_SEQ */
static MV_CFG_ELEMENT serdesLpbkNormalParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,           0,      0xF000}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,     0,      0x8000}},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PU_PLL_Reg,             0,      0x800 }}
};

/* SERDES_SD_ANA_TX_2_RX_SEQ */
static MV_CFG_ELEMENT serdesLpbkAnaTx2RxParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,       0x2000, 0x2000}}
};

/* SERDES_SD_DIG_RX_2_TX_SEQ */
static MV_CFG_ELEMENT serdesLpbkDigRx2TxParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PU_PLL_Reg,              0x800,  0x800   }},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0xD0,   0xF0    }},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x8000, 0x8000  }},
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,              0x9000, 0x9000}}
};

/* SERDES_SD_DIG_TX_2_RX_SEQ */
static MV_CFG_ELEMENT serdesLpbkDigTx2RxParams[] =
{
    {WRITE_OP, {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,    0x4000, 0x4000}}
};

/* SERDES_TX_TRAINING_ENABLE_SEQ Sequence init */
static MV_CFG_ELEMENT serdesTxTrainingEnParams[] =
{
    {WRITE_OP, { SERDES_PHY_UNIT,  0x1C,     0x8000,    0x8000}},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x440,    0,         0x80  }},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x168,    0x8000,    0x8000}},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x188,    0,         0x1FF }},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x94,     0xFFF,     0xFFFF}},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x31C,    0,         4     }}
};

/* SERDES_TX_TRAINING_DISABLE_SEQ Sequence init */
static MV_CFG_ELEMENT serdesTxTrainingDisParams[] =
{
    {WRITE_OP, {SERDES_UNIT,       0x8,    0x0,        0x20  }},
    {WRITE_OP, {SERDES_UNIT,       0xC,    0x0,        0x100 }},
    {WRITE_OP, {SERDES_PHY_UNIT,   0x22C,  0x0,        0x2   }},
    {WRITE_OP, {SERDES_PHY_UNIT,   0x94,   0x1FFF,     0xFFFF}},
    {WRITE_OP, {SERDES_PHY_UNIT,   0x8C,   0x72,       0xFFFF}}
};

/* SERDES_RX_TRAINING_DISABLE_SEQ Sequence init */
static MV_CFG_ELEMENT serdesRxTrainingDisParams[] =
{
    {WRITE_OP, {SERDES_UNIT,       0x8,    0x0,        0x20}},
    {WRITE_OP, {SERDES_UNIT,       0xC,    0x0,        0x80}},
    {WRITE_OP, {SERDES_PHY_UNIT,   0x22C,  0x0,        0x1 }}
};

/* SERDES_RX_TRAINING_ENABLE_SEQ Sequence init */
static MV_CFG_ELEMENT serdesRxTrainingEnParams[] =
{
    {WRITE_OP, { SERDES_PHY_UNIT,  0x1C,   0x8000,     0x8000}},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x440,  0,          0x80  }},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x168,  0x8000,     0x8000}},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x188,  0,          0x1FF }},
    {WRITE_OP, { SERDES_PHY_UNIT,  0x22C,  0x1,        0x1   }},
    {WRITE_OP, { SERDES_UNIT,      0xC,    0x80,       0x80  }}
};


MV_CFG_SEQ hwsSerdes28nmRev3SeqDb[MV_SERDES_LAST_SEQ] = {
    {serdesSdResetParams,      MV_SEQ_SIZE(serdesSdResetParams)      },     /* SERDES_SD_RESET_SEQ */
    {serdesSdUnResetParams,    MV_SEQ_SIZE(serdesSdUnResetParams)    },     /* SERDES_SD_UNRESET_SEQ */
    {serdesRfResetParams,      MV_SEQ_SIZE(serdesRfResetParams)      },     /* SERDES_RF_RESET_SEQ */
    {serdesRfUnResetParams,    MV_SEQ_SIZE(serdesRfUnResetParams)    },     /* SERDES_RF_UNRESET_SEQ */
    {serdesSynceResetParams,   MV_SEQ_SIZE(serdesSynceResetParams)   },     /* SERDES_SYNCE_RESET_SEQ */
    {serdesSynceUnResetParams, MV_SEQ_SIZE(serdesSynceUnResetParams) },     /* SERDES_SYNCE_UNRESET_SEQ */

    {NULL,                     0                                     },     /* SERDES_SERDES_POWER_UP_CTRL_SEQ */
    {serdesPowerDownCtrlParams,MV_SEQ_SIZE(serdesPowerDownCtrlParams)},     /* SERDES_SERDES_POWER_DOWN_CTRL_SEQ */
    {serdesRxIntUpParams,      MV_SEQ_SIZE(serdesRxIntUpParams)      },     /* SERDES_SERDES_RXINT_UP_SEQ */
    {serdesRxIntDownParams,    MV_SEQ_SIZE(serdesRxIntDownParams)    },     /* SERDES_SERDES_RXINT_DOWN_SEQ */
    {serdesWaitPllParams,      MV_SEQ_SIZE(serdesWaitPllParams)      },     /* SERDES_SERDES_WAIT_PLL_SEQ */

    {SerdesSpeed_1_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_1_25gSeq)     },     /* SERDES_SPEED_1_25G_SEQ               */
    {SerdesSpeed_3_125gSeq,    MV_SEQ_SIZE(SerdesSpeed_3_125gSeq)    },     /* SERDES_SPEED_3_125G_SEQ              */
    {SerdesSpeed_3_75gSeq,     MV_SEQ_SIZE(SerdesSpeed_3_75gSeq)     },     /* SERDES_SPEED_3_75G_SEQ               */
    {SerdesSpeed_4_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_4_25gSeq)     },     /* SERDES_SPEED_4_25G_SEQ               */
    {SerdesSpeed_5gSeq,        MV_SEQ_SIZE(SerdesSpeed_5gSeq)        },     /* SERDES_SPEED_5G_SEQ                  */
    {SerdesSpeed_6_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_6_25gSeq)     },     /* SERDES_SPEED_6_25G_SEQ               */
    {SerdesSpeed_7_5gSeq,      MV_SEQ_SIZE(SerdesSpeed_7_5gSeq)      },     /* SERDES_SPEED_7_5G_SEQ                */
    {SerdesSpeed_10_3125gSeq,  MV_SEQ_SIZE(SerdesSpeed_10_3125gSeq)  },     /* SERDES_SPEED_10_3125G_SEQ            */

    {serdesLpbkNormalParams,   MV_SEQ_SIZE(serdesLpbkNormalParams)   },     /* SERDES_SD_LPBK_NORMAL_SEQ */
    {serdesLpbkAnaTx2RxParams, MV_SEQ_SIZE(serdesLpbkAnaTx2RxParams) },     /* SERDES_SD_ANA_TX_2_RX_SEQ */
    {serdesLpbkDigTx2RxParams, MV_SEQ_SIZE(serdesLpbkDigTx2RxParams) },     /* SERDES_SD_DIG_TX_2_RX_SEQ */
    {serdesLpbkDigRx2TxParams, MV_SEQ_SIZE(serdesLpbkDigRx2TxParams) },     /* SERDES_SD_DIG_RX_2_TX_SEQ */

    {serdesPtAfterPatternNormalParams, MV_SEQ_SIZE(serdesPtAfterPatternNormalParams)}, /* SERDES_PT_AFTER_PATTERN_NORMAL_SEQ */
    {serdesPtAfterPatternTestParams,   MV_SEQ_SIZE(serdesPtAfterPatternTestParams)  }, /* SERDES_PT_AFTER_PATTERN_TEST_SEQ */


    {serdesRxTrainingEnParams, MV_SEQ_SIZE(serdesRxTrainingEnParams) },     /* SERDES_RX_TRAINING_ENABLE_SEQ */
    {serdesRxTrainingDisParams,MV_SEQ_SIZE(serdesRxTrainingDisParams)},     /* SERDES_RX_TRAINING_DISABLE_SEQ */
    {serdesTxTrainingEnParams, MV_SEQ_SIZE(serdesTxTrainingEnParams) },     /* SERDES_TX_TRAINING_ENABLE_SEQ */
    {serdesTxTrainingDisParams,MV_SEQ_SIZE(serdesTxTrainingDisParams)},     /* SERDES_TX_TRAINING_DISABLE_SEQ */

    {SerdesSpeed_12_5gSeq,     MV_SEQ_SIZE(SerdesSpeed_12_5gSeq)     },     /* SERDES_SPEED_12_5G_SEQ */
    {NULL,                      0                                    },     /* SERDES_SPEED_3_3G_SEQ */


    {SerdesSpeed_11_5625gSeq,  MV_SEQ_SIZE(SerdesSpeed_11_5625gSeq)  },     /* SERDES_SPEED_11_5625G_SEQ */
    {NULL,                      0                                    },     /* SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ */
    {NULL,                      0                                    },     /* SERDES_SERDES_PARTIAL_POWER_UP_SEQ */

    {SerdesSpeed_11_25gSeq,    MV_SEQ_SIZE(SerdesSpeed_11_25gSeq)    },     /* SERDES_SPEED_11_25G_SEQ */
    {serdesCoreResetParams,    MV_SEQ_SIZE(serdesCoreResetParams)    },     /* SERDES_CORE_RESET_SEQ */
    {serdesCoreUnResetParams,  MV_SEQ_SIZE(serdesCoreUnResetParams)  },     /* SERDES_CORE_UNRESET_SEQ */

    {NULL,                      0                                    },     /* SERDES_FFE_TABLE_LR_SEQ */
    {NULL,                      0                                    },     /* SERDES_FFE_TABLE_SR_SEQ */

    {SerdesSpeed_10_9375gSeq,  MV_SEQ_SIZE(SerdesSpeed_10_9375gSeq)  },     /* SERDES_SPEED_10_9375G_SEQ */
    {SerdesSpeed_12_1875gSeq,  MV_SEQ_SIZE(SerdesSpeed_12_1875gSeq)  },     /* SERDES_SPEED_12_1875G_SEQ */
    {SerdesSpeed_5_625gSeq,    MV_SEQ_SIZE(SerdesSpeed_12_1875gSeq)  },     /* SERDES_SPEED_5_625G_SEQ */
    {SerdesSpeed_5_15625gSeq,  MV_SEQ_SIZE(SerdesSpeed_5_15625gSeq)  },     /* SERDES_SPEED_5_15625G_SEQ */
};

GT_STATUS hwsComPhyH28nmRev3SeqInit(void)
{
    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    mvCfgSerdesSeqExecInit(hwsSerdes28nmRev3SeqDb, (sizeof(hwsSerdes28nmRev3SeqDb) / sizeof(MV_CFG_SEQ)));

    hwsSerdesSeqDbPtr = hwsSerdes28nmRev3SeqDb; /* used for sequence set/get */

    return GT_OK;
}

#endif
