/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file dxFamilyBoards.c
*
* @brief Includes board specific initialization definitions and data-structures.
* Dx Family board types.
*
* @version   45
********************************************************************************
*/

/*#include <appDemo/boardConfig/appDemoBoardConfig.h>*/
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

/*******************************************************************************
 * External variables
 ******************************************************************************/
#define EMPTY_BOARD ""
#define EMPTY_LINE  {NULL , EMPTY_BOARD ,1,{""}}
/*******************************************************************************
 * Local usage variables
 ******************************************************************************/
GT_BOARD_LIST_ELEMENT   boardsList[] =
{
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    {gtDbDxXcat24G4HgsBoardReg ,"xCat, xCat2, xCat3",11,{"Rev 0.1",      /* 19,1,0 xCat 24 4XG - PEX */
                                          "Rev 0.2 - SDMA",
                                          "Rev 0.3 - FC OFF",
                                          "Rev 0.4 - PBR",
                                          "Rev 0.5 - P24/25 SGMII mode, p26/27 QX mode",
                                          "Rev 0.6 - PBR SDMA",
                                          "Rev 0.7 - SMI only board",
                                          "Rev 0.8 - Native MII",
                                          "Rev 0.9 - PHYMAC",
                                          "Rev 0.10 - Legacy VPLS Enabled",
                                          "Rev 0.11 - +Linux BM kernel driver"}},
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    EMPTY_LINE,
    {gtDbDxLion296XgBoardReg,"Lion2",27,{"Rev 0.1 - 96X10G_SR_LR"
                                      ,"Rev 0.2 - 32X40G_SR_LR"
                                      ,"Rev 0.3 - 96X10G_KR"
                                      ,"Rev 0.4 - 32X40G_KR4"
                                      ,"Rev 0.5 - CT 10G 40G loopback Port"/*5*/
                                      ,"Rev 0.6 - 56X10G 210MHz"/*6*/
                                      ,"HooperRD:0..7-10G_KR;8...-40G_KR"/*7*/
                                      ,"HooperRD:0..7-10G_KR;8...-40G_KR: kernel driver"/*8*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*9*/
                                      ,"Rev 0.10- MII interface" /*10 = MII interface */
                                      ,"96X10G_SR_LR, PFC enable TC5_6 all ports"    /*11 = PFC mode */
                                      ,"96X10G_SR_LR, Lion2-4 cores "                /*12 = lion2 4 cores enabled*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*13*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*14*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*15*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*16*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*17*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*18*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*19*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*20*/
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS/*21*/
                                      ,"multi port group FDB lookup" /*  multi port group FDB lookup - specific infrastructure */
                                      ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS                /* 23 */
                                      ,"multi port group FDB lookup : 256K mode - auto init" /* 24 enhanced Lion2 96XG - 256K auto infrastructure */
                                      ,"multi port group FDB lookup : 128K mode - auto init" /* 25 enhanced Lion2 96XG - 128K auto infrastructure */
                                      ,"360MHz 0,1,2,3x10G 4,8,9x40G"    }},                 /* 26 40G --> 16G */
    EMPTY_LINE,
    {gtDbDxBobcat2BoardReg,"Bobcat2, Caelum, Cetus, Cygnus, Lewis, Aldrin, Bobcat3, Aldrin2", 12,
                                        {"All Devices Regular init, No TM"                              /* 1*/
                                        ,"Bobcat2, Caelum, Cetus - TM enabled"                          /* 2*/
                                        ,"Bobcat2 MTL RD board 48GE +4X10G/Aldrin MTL RD board 24*10G +2X40G"/* 3*/
                                        ,"Bobcat2 MTL RD board 4*10G +2X40G->10G TM Enable"             /* 4*/
                                        ,"All Devices Simplified Init DB board"                         /* 5*/
                                        ,"Bobcat2, Caelum, Cetus Simplified Init DB board - TM Enabled" /* 6*/
                                        ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS                         /* 7*/
                                        ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS                         /* 8*/
                                        ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS                         /* 9*/
                                        ,GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS                         /*10*/
                                        ,"Bobcat2 RD Board"                                             /*11*/
                                        ,"Bobcat2 RD Board TM Enabled"                                  /*12*/
                                        }},/* 29,x,BobCat2 */
    {gtDbDxBobcat2_X6_BoardReg,"Six BobCat2",1,{"Rev 0.1"                                 /*1*/
                                           }},/* 30,x,BobCat2 */
    {gtDbDxLion2_X2_Bobcat2_BoardReg,"Two Lion2 + Bobcat2",4,{"Rev 0.1 - only Two Lion2"         /*1*/
                                                             ,"Rev 0.2"                          /*2*/
                                                             ,"Rev 0.3 - Two Hoopers + Bobcat2"  /*3*/
                                                             ,"Rev 0.4 - One Hooper  + Bobcat2"  /*4*/
                                           }},/* 31,x,2xLion2 + BobCat2 */
    #if defined(CHX_FAMILY) && defined(PX_FAMILY)
    {gtDbDxAc3xBoardReg , "AC3x",5,         {"24/48*1G + 4*10G + 2*40G (3/6*PHY1690)"  /*1*/
                                            ,"24*2.5G  + 4*10G + 1*40G (no_PHY1690)"  /*2*/
                                            ,"32*1G  + 16*2.5G + 1*40G (4*PHY1690)"   /*3*/
                                            ,"24/48*1G  + 4*10G + 2*40G (3/6*PHY1690) - link state polling task"   /*4*/
                                            ,"32*1G(4*PHY1690) + 16*2.5G(PHY2180*2) + 4*10G + 2*25G(Pipe)" /*5*/
                                           }},/* 32,x, */
    #else
    {gtDbDxAc3xBoardReg , "AC3x",4,         {"24/48*1G + 4*10G + 2*40G (3/6*PHY1690)"  /*1*/
                                            ,"24*2.5G  + 4*10G + 1*40G (no_PHY1690)"  /*2*/
                                            ,"32*1G  + 16*2.5G + 1*40G (4*PHY1690)"   /*3*/
                                            ,"24/48*1G  + 4*10G + 2*40G (3/6*PHY1690) - link state polling task"   /*4*/
                                           }},/* 32,x, */
    #endif

#if defined(DXCH_CODE) && defined(PX_CODE)
    {gtDbPxPipeBoardReg      ,"PX-Pipe"     ,1,{"12*10G + 4*25G"
                                           }},  /* 33,x */
#else
    /*NOTE: this is place HOLDER for the Px:Pipe initialization
      in the PX image. */
    {/*PX-pipe device*/NULL      ,"PX-Pipe"     ,1,{"12*10G + 100G"
                                           }},  /* 33,x */
#endif

    {/*gtRdPxPipeBoard4DevsReg*/NULL ,"PX-Pipe * 4 (RD board)" ,1,{"4 devs each 12*10G + 4*25G"
                                           }},  /* 34,x */

    {gtDbPxFalconBoardReg,"Falcon",1,{"Rev 0.1"                                 /*1*/
                                           }},/* 35,x,BobCat2 */


    EMPTY_LINE,
    EMPTY_LINE
};
const GT_U32 boardListLen = sizeof(boardsList) / sizeof(GT_BOARD_LIST_ELEMENT);



