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
* @file gtBoardsConfigFuncs.h
*
* @brief Registration function prototypes for all boards.
*
* @version   11
********************************************************************************
*/
#ifndef __gtBoardsConfigFuncsh
#define __gtBoardsConfigFuncsh

#ifdef __cplusplus
extern "C"
#endif /* __cplusplus */

#include <appDemo/os/appOs.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

/*******************************************************************************
* appDemoDxChVbMuxInit
*
* DESCRIPTION:
*         Initialize DXCH device as VB multiplexer
*
* INPUTS:
*       ppIndex       - index of the device in array of appDemoPpConfigList[]
*
*       networkPortsNum - network Ports amount
*       networkPortsArr - array of network Ports (refer to networkPortsNum)
*       networkPortCscdArr - array of cascade Ports that match as egress private
*                           edge vlan to the network ports (refer to networkPortsNum)
*
*       cascadePortsNum - cascade Ports amount
*       cascadePortsArr - the cascade Ports (refer to cascadePortsNum)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#ifdef CHX_FAMILY
GT_STATUS appDemoDxChVbMuxInit
(
    IN  GT_U8                       ppIndex,

    IN  GT_U32                      networkPortsNum,
    IN  GT_U8                       networkPortsArr[],
    IN  GT_U8                       networkPortCscdArr[],

    IN  GT_U32                      cascadePortsNum,
    IN  GT_U8                       cascadePortsArr[]
);
#endif /*CHX_FAMILY*/

/**
* @internal gtDuneBrd0012aBoardReg function
* @endinternal
*
* @brief   Registeration function for the DUNE-BRD0012A board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDuneBrd0012aBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);
/**
* @internal gtDuneBrd0002aBoardReg function
* @endinternal
*
* @brief   Registeration function for the DUNE-BRD0002A.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDuneBrd0002aBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbMx620E1041BoardReg function
* @endinternal
*
* @brief   Registration function for the DB-MX620_E1041 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbMx620E1041BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbMx61048F4GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-MX610-48F4GS board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbMx61048F4GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtRdEx12024GBoardReg function
* @endinternal
*
* @brief   Registration function for the RD-EX120-24G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdEx12024GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx900Mx620ConfigBoardReg function
* @endinternal
*
* @brief   Registration function for the DBFx900Mx620 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx900Mx620ConfigBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbMx62010GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-MX620-10Gs board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbMx62010GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Ex120ConfigBoardReg function
* @endinternal
*
* @brief   Registration function for the DBFx910Ex120 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Ex120ConfigBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Fx9210Ex120CapConfigBoardReg function
* @endinternal
*
* @brief   Registration function for the DBFx910Fx9210Ex120 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Fx9210Ex120CapConfigBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtRdEx120D024GBoardReg function
* @endinternal
*
* @brief   Registration function for the 98-EX120-D0-24G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdEx120D024GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Mx62010GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DBFx910Mx62010G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Mx62010GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Mx61048F4GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-Fx910-MX610-48F4GS board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Mx61048F4GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbEx24248GBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-EX242-48G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx24248GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDX13xXGBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-EX130-XG/DB-EX135-XG board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDX13xXGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Mx62012GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-FX910-MX625-12GS.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Mx62012GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx910Mx61548F4GsBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-FX910-MX615-48F4GS board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx910Mx61548F4GsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDx24124GBoardReg function
* @endinternal
*
* @brief   Registration function for the Db-Dx241-24G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx24124GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDx16016GBoardReg function
* @endinternal
*
* @brief   Registration function for the Db-Dx160-16G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx16016GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtRdDx246_24GBoardReg function
* @endinternal
*
* @brief   Registration function for the RD-Dx246-24G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs_PTR        - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdDx246_24GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs_PTR
);


/**
* @internal gtEv88E6183BoardReg function
* @endinternal
*
* @brief   Registration function for the Ev88E6183 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtEv88E6183BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtEv88E6093BoardReg function
* @endinternal
*
* @brief   Registration function for the Db88E6093 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtEv88E6093BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtRd88E609324F4GBoardReg function
* @endinternal
*
* @brief   Registration function for the Db88E6093 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRd88E609324F4GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbMX635XGBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-MX635-1XG board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbMX635XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtFsFx902Ex125_24GBoardReg function
* @endinternal
*
* @brief   Registeration function for the Fs-24G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtFsFx902Ex125_24GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtFsFx902Ex115_48F4GBoardReg function
* @endinternal
*
* @brief   Registration function for the Fs-48F4G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtFsFx902Ex115_48F4GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbEx126_12G_BoardReg function
* @endinternal
*
* @brief   Registration function for the DB-EX126-12G (Tiger) board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx126_12G_BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDx27024G3XGBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-270-24G-3XG board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx27024G3XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbEx100_24F4G_BoardReg function
* @endinternal
*
* @brief   Registration function for the DB-EX100-24Fa-4G (Phoenix) board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx100_24F4G_BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtRdDx27048G2XGBoardReg function
* @endinternal
*
* @brief   Registration function for the RD-DX270-48G-2XG board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdDx27048G2XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtRdDx24148GBoardReg function
* @endinternal
*
* @brief   Registration function for the RD-DX241-48G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdDx24148GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtRd88E609348F4GBoardReg function
* @endinternal
*
* @brief   Registration function for the Gemstone: RD-88E6093-48F4G
*         48FE + 4G Management Switch.
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRd88E609348F4GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDx24624G3XGBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-246-24G-3XG board. (cheetah - SMI)
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx24624G3XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbEx126Dx240BoardReg function
* @endinternal
*
* @brief   Registration function for the Ex126Dx240 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx126Dx240BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbEx136Dx270BoardReg function
* @endinternal
*
* @brief   Registration function for the Ex126Dx240 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx136Dx270BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDx10748F2GBoardReg function
* @endinternal
*
* @brief   Registration function for the 107+Opal3 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx10748F2GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDuneEx136Dx270BoardReg function
* @endinternal
*
* @brief   Registration function for value blade board, consisting of
*         4 x (DX270 + EX136 + FAP10M)
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDuneEx136Dx270BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDuneEx126BoardReg function
* @endinternal
*
* @brief   Registeration function for the Dune FAP10M and 98EX126 board
*         boardS.
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDuneEx126BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbEx126Dx160BoardReg function
* @endinternal
*
* @brief   Registration function for the Ex126Dx160 board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx126Dx160BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbEx106Ex108BoardReg function
* @endinternal
*
* @brief   Registration function for board, consisting of
*         1 x EX106 or 1 x EX108
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbEx106Ex108BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDb2Ex126_3Dx246BoardReg function
* @endinternal
*
* @brief   Registration function for the 2Ex126_3Dx246BoardReg Value Blade
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDb2Ex126_3Dx246BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDx10610GBoardReg function
* @endinternal
*
* @brief   Registeration function for the DB-Dx106-10G board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx10610GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDx10648F2GBoardReg function
* @endinternal
*
* @brief   Registration function for the 106 + Opal6 Gemstone board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx10648F2GBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);
/**
* @internal gtDbDx28524G4XGBoardReg function
* @endinternal
*
* @brief   Registration function for the DB-285-24G-4XG board (Cheetah 2 devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx28524G4XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDb98Ex240BoardReg function
* @endinternal
*
* @brief   Registration function for the DB-98Ex240 board. -- EXMXPM 24G ports
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDb98Ex240BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDb98Ex240_3Dx246BoardReg function
* @endinternal
*
* @brief   Registration function for the 98Ex240_3Dx246 Board Value Blade
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDb98Ex240_3Dx246BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDx348GE4HGSBoardReg function
* @endinternal
*
* @brief   Registeration function for the DB-DX3-GP board (Cheetah 3 devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDx348GE4HGSBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDxXcat24G4HgsBoardReg function
* @endinternal
*
* @brief   Registration function for the xCat 24G + 4Hgs ports board (xCat devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxXcat24G4HgsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxLion48XgBoardReg function
* @endinternal
*
* @brief   Registration function for the Lion 48XG ports board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxLion48XgBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxLion_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Registration function for the Lion 48XG ports board.
*         ForMultiCoreFdbLookup specifics
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxLion_ForMultiCoreFdbLookup
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDxLion296XgBoardReg function
* @endinternal
*
* @brief   Registration function for the Lion2 96XG ports board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxLion296XgBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxLion2_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Registration function for the Lion2 96XG ports board.
*         ForMultiCoreFdbLookup specifics
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxLion2_ForMultiCoreFdbLookup
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtFpgaRhodesFaBoard function
* @endinternal
*
* @brief   Registration function for the FPGA(Rhodes) board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtFpgaRhodesFaBoard
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbFx950_DX3BoardReg function
* @endinternal
*
* @brief   Registeration function for the DB-FX950-DX3-GP board (Cheetah 3 devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx950_DX3BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbFx950Ex240BoardReg function
* @endinternal
*
* @brief   Registeration function for the DB-FX950-Ex240-GP board (Puma2 devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx950Ex240BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbFx9130BoardReg function
* @endinternal
*
* @brief   Registeration function for the DB-FX9130 board (FE200).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbFx9130BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDb98Ex240_DxLion48XGBoardReg function
* @endinternal
*
* @brief   Registration function for the gtDb98Ex240_DxLion48XGBoardReg Puma and
*         Lion Board Value Blade.
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDb98Ex240_DxLion48XGBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDb98ExMxPm3BoardReg function
* @endinternal
*
* @brief   Registration function for the DB-98 Puma3 board. -- EXMXPM3
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDb98ExMxPm3BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbDxBobcat2BoardReg function
* @endinternal
*
* @brief   Registration function for the BobCat2 (SIP5) board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxBobcat2BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxBobcat2_X6_BoardReg function
* @endinternal
*
* @brief   Registration function for the Six BobCat2 (SIP5) board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxBobcat2_X6_BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxLion2_X2_Bobcat2_BoardReg function
* @endinternal
*
* @brief   Registration function for the two Lion2 and one Bobcat2 ports board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxLion2_X2_Bobcat2_BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbDxAc3xBoardReg function
* @endinternal
*
* @brief   Registration function for the AC3x (xCat3) board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxAc3xBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);


/**
* @internal gtDbPxFalconBoardReg function
* @endinternal
*
* @brief   Registration function for the Falcon board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxFalconBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtDbPxPipeBoardReg function
* @endinternal
*
* @brief   Registration function for the PX Pipe board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxPipeBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

/**
* @internal gtRdPxPipeBoard4DevsReg function
* @endinternal
*
* @brief   Registration function for the PX Pipe 4 devices board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdPxPipeBoard4DevsReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtBoardsConfigFuncsh */





