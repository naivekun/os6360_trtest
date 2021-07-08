/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssCommonDiag.c
*
* @brief Internal CPSS functions.
*
* @version   1
********************************************************************************
*/
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif /*ASIC_SIMULATION*/

static GT_STATUS regsInfoSearch
(
    IN    GT_U8                              devNum,
    IN    GT_U32                             portGroupId,
    IN    PRV_CPSS_DIAG_REGS_SEARCH_TYPE_ENT searchType,
    IN    GT_U32                             *regsListPtr,
    IN    GT_U32                             regsListSize,
    IN    GT_U32                             offset,
    IN    GT_U32                             unitIdAddrPattern,
    IN    GT_U32                             unitIdAddrMask,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC checkRegExistFuncPtr,
    INOUT GT_U32                             *regsNumPtr,
    OUT   GT_U32                             *regsInfoPtr
);

/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/

#define DIAG_BYTE_SWAP_MAC(doSwap, data)            \
    (doSwap == GT_TRUE) ? (BYTESWAP_MAC(data)) : data;

/* Macro to calculate the memory address from the device, memory size and memory
   base address */
#define CALC_MEM_ADDR_MAC(memBase,memSize)          \
    memBase + memSize

/* Macro to calculate address completion for PHY access */
#define PRV_DIAG_CALC_PHY_ACCESS_COMPLETION_MAC(value)  ((value) >> 27) & 0x1

/* write the DATA into the address
    do the write 3 time ... to make sure it is done ...
*/
#define PCI_3_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)   \
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data);      \
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data);      \
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)

#ifdef ASIC_SIMULATION
    static GT_U32 hwUsed = 0;
#ifdef GM_USED  /* the GM supports address completion */
    static GT_U32 gmUsed = 1;
#else
    static GT_U32 gmUsed = 0;
#endif
    #define PCI_CFG_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)   \
        PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)
    #define READ_CFG_DATA_FROM_ADDR_MAC(_address , _data) \
        scibPciRegRead(simulationDeviceId,_address,1,&_data)
    /* write the DATA into the address
        do the write 1 time .
    */
    #define PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)   \
        scibWriteMemory(simulationDeviceId,_address,1,&_data)
    /* read DATA from the address */
    #define READ_DATA_FROM_ADDR_MAC(_address , _data) \
        scibReadMemory(simulationDeviceId,_address,1,&_data)
#else /* not GM_USED and not ASIC_SIMULATION*/
    static GT_U32 gmUsed = 0;
    static GT_U32 hwUsed = 1;
    /* write the DATA into the address
        do the write 1 time .
    */
    #define PCI_CFG_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)   \
        PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)

    #define READ_CFG_DATA_FROM_ADDR_MAC(_address , _data) \
        READ_DATA_FROM_ADDR_MAC(_address , _data)

    /* read DATA from the address */
    #define READ_DATA_FROM_ADDR_MAC(_address , _data) \
            _data = *(volatile GT_U32 *)(_address)
    /* write the DATA into the address
        do the write 1 time .
    */
    #define PCI_1_WRITE_DATA_INTO_ADDR_MAC(_address , _doByteSwap , _data)   \
        *(volatile GT_U32 *)(_address) = DIAG_BYTE_SWAP_MAC(_doByteSwap, _data)

#endif

#ifdef GM_USED  /* the GM supports address completion */
#elif defined ASIC_SIMULATION   /* cause compilation error , since not supports address completion */
#else /* not GM_USED and not ASIC_SIMULATION*/
#endif
/* offset of the address completion for PEX 8 completion regions */
#define PEX_MBUS_ADDR_COMP_REG_MAC(_index) (0x120 + (4 * _index))
/* bits of address passes as is throw PCI window */
#define NOT_ADDRESS_COMPLETION_BITS_NUM_CNS 19
/* bits of address extracted from address completion registers */
#define ADDRESS_COMPLETION_BITS_MASK_CNS    (0xFFFFFFFF << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)


/* use region 2 for address completion */
#define DIAG_ADDR_COMPLETION_REGION_CNS 2

#ifdef ASIC_SIMULATION
static GT_U32   simulationDeviceId = 0;
#endif


/**
* @internal prvCpssDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers in use for the PP.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id, to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
*
* @param[out] regsNumPtr               - number of registers in use
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegsNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              portGroupId,
    IN  GT_U32                              *regsListPtr,
    IN  GT_U32                              regsListSize,
    IN  GT_U32                              offset,
    IN  PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC  checkRegExistFuncPtr,
    OUT GT_U32                              *regsNumPtr
)
{
    return regsInfoSearch (devNum,
                           portGroupId,
                           PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E,
                           regsListPtr,
                           regsListSize,
                           offset,
                           0,
                           0,
                           checkRegExistFuncPtr,
                           regsNumPtr,
                           NULL);
}

/**
* @internal prvCpssDiagRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr           - in : number of registers to get their addresses/data
*                                       out: number of registers that their data were
*                                            successfully retrieved
*
* @param[out] regAddrPtr               - list of registers addresses
* @param[out] regDataPtr               - list of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                portGroupId,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
)
{
    return prvCpssDiagUnitRegsDataGet (devNum,
                                       portGroupId,
                                       regsListPtr,
                                       regsListSize,
                                       offset,
                                       0,
                                       0,
                                       checkRegExistFuncPtr,
                                       regsNumPtr,
                                       regAddrPtr,
                                       regDataPtr);
}


/**
* @internal prvCpssDiagResetAndInitControllerRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use for Reset and Init
*         Controller.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr           - in : number of registers to get their addresses/data
*                                       out: number of registers that their data were
*                                            successfully retrieved
*
* @param[out] regAddrPtr               - list of registers addresses
* @param[out] regDataPtr               - list of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagResetAndInitControllerRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
)
{
    GT_U32 ppRegsNum;     /* number of registers in use on the PP */

    /* Get number of registers in use */
    regsInfoSearch (devNum,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E,
                    regsListPtr,
                    regsListSize,
                    offset,
                    0,
                    0,
                    checkRegExistFuncPtr,
                    &ppRegsNum,
                    NULL);

    if (*regsNumPtr > ppRegsNum)
    {
        /* regsNumPtr is too big */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get registers addresses */
    regsInfoSearch (devNum,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    PRV_CPSS_DIAG_REGS_ADDR_SEARCH_E,
                    regsListPtr,
                    regsListSize,
                    offset,
                    0,
                    0,
                    checkRegExistFuncPtr,
                    regsNumPtr,
                    regAddrPtr);

    /* Get registers data */
    return regsInfoSearch (devNum,
                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                           PRV_CPSS_DIAG_REGS_DATA_RESET_AND_INIT_CTRL_SEARCH_E,
                           regsListPtr,
                           regsListSize,
                           offset,
                           0,
                           0,
                           checkRegExistFuncPtr,
                           regsNumPtr,
                           regDataPtr);
}

/**
* @internal prvCpssDiagUnitRegsDataGet function
* @endinternal
*
* @brief   Get the addresses and data of the registers in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regsListPtr              - Pointer to the regsiters list
* @param[in] regsListSize             - Size of registers list
* @param[in] offset                   - Only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] unitIdAddrPattern        - pattern to search registers os specific unit. Only registers
*                                      with address that satisfies the formula
*                                      (register_address & unitIdAddrPattern) == unitIdAddrPattern
*                                      will be counted/dumped.
* @param[in] unitIdAddrMask           - mask to search registers of specific unit.
* @param[in] checkRegExistFuncPtr     - Pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr           - in : number of registers to get their addresses/data
*                                       out: number of registers that their data were
*                                            successfully retrieved
*
* @param[out] regAddrPtr               - List of registers addresses
* @param[out] regDataPtr               - List of registers data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagUnitRegsDataGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                portGroupId,
    IN    GT_U32                                *regsListPtr,
    IN    GT_U32                                regsListSize,
    IN    GT_U32                                offset,
    IN    GT_U32                                unitIdAddrPattern,
    IN    GT_U32                                unitIdAddrMask,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC    checkRegExistFuncPtr,
    INOUT GT_U32                                *regsNumPtr,
    OUT   GT_U32                                *regAddrPtr,
    OUT   GT_U32                                *regDataPtr
)
{
    GT_U32 ppRegsNum;     /* number of registers in use on the PP */

    /* Get number of registers in use */
    regsInfoSearch (devNum,
                    portGroupId,
                    PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E,
                    regsListPtr,
                    regsListSize,
                    offset,
                    unitIdAddrPattern,
                    unitIdAddrMask,
                    checkRegExistFuncPtr,
                    &ppRegsNum,
                    NULL);

    if (*regsNumPtr > ppRegsNum)
    {
        /* regsNumPtr is too big */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get registers addresses */
    regsInfoSearch (devNum,
                    portGroupId,
                    PRV_CPSS_DIAG_REGS_ADDR_SEARCH_E,
                    regsListPtr,
                    regsListSize,
                    offset,
                    unitIdAddrPattern,
                    unitIdAddrMask,
                    checkRegExistFuncPtr,
                    regsNumPtr,
                    regAddrPtr);

    /* Get registers data */
    return regsInfoSearch (devNum,
                           portGroupId,
                           PRV_CPSS_DIAG_REGS_DATA_SEARCH_E,
                           regsListPtr,
                           regsListSize,
                           offset,
                           unitIdAddrPattern,
                           unitIdAddrMask,
                           checkRegExistFuncPtr,
                           regsNumPtr,
                           regDataPtr);
}

/**
* @internal prvCpssDiagUnitRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers in use for the the given unit in use.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The device number
* @param[in] regsListPtr              - Pointer to the regsiters list
* @param[in] regsListSize             - Size of registers list
* @param[in] offset                   - Only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] unitIdAddrPattern        - pattern to search registers os specific unit. Only registers
*                                      with address that satisfies the formula
*                                      (register_address & unitIdAddrPattern) == unitIdAddrPattern
*                                      will be counted/dumped.
* @param[in] unitIdAddrMask           - mask to search registers of specific unit.
*
* @param[out] regsNumPtr               - Number of registers in use
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagUnitRegsNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *regsListPtr,
    IN  GT_U32                              regsListSize,
    IN  GT_U32                              offset,
    IN  GT_U32                              unitIdAddrPattern,
    IN  GT_U32                              unitIdAddrMask,
    OUT GT_U32                              *regsNumPtr
)
{
    return regsInfoSearch (devNum,
                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                           PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E,
                           regsListPtr,
                           regsListSize,
                           offset,
                           unitIdAddrPattern,
                           unitIdAddrMask,
                           NULL,
                           regsNumPtr,
                           NULL);
}

/***************************** Static Functions ******************************/


/**
* @internal regsInfoSearch function
* @endinternal
*
* @brief   Search for information about the registers in use.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group
*                                      devices that need to access specific port group
* @param[in] searchType               - type of information to search.
*                                      this can be:
*                                      (a) number of registers in use
*                                      (b) list of addresses for registers in use
*                                      (c) list of data for registers in use
* @param[in] regsListPtr              - pointer to the regsiters list
* @param[in] regsListSize             - size of registers list
* @param[in] offset                   - only addresses of registers bigger than this value
*                                      will be taken into account
* @param[in] unitIdAddrPattern        - pattern to search registers os specific unit. Only registers
*                                      with address that satisfies the formula
*                                      (register_address & unitIdAddrPattern) == unitIdAddrPattern
*                                      will be counted/dumped.
* @param[in] unitIdAddrMask           - mask to search registers of specific unit.
* @param[in] checkRegExistFuncPtr     - pointer to function that checks register existance in
*                                      particular port group.
* @param[in,out] regsNumPtr           - in : for (a) - not relevant
*                                            for (b) - number of registers to get their addresses
*                                            for (c) - number of registers to get their data
*                                       out: for (a) - number of registers in use
*                                            for (b) - number of registers that their addresses
*                                                      were successfully retrieved
*                                            for (c) - number of registers that their data were
*                                                      successfully retrieved
*
* @param[out] regsInfoPtr              - for (a) - not relevant
*                                      for (b) - list of registers addresses
*                                      for (c) - list of registers data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad search type
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS regsInfoSearch
(
    IN    GT_U8                              devNum,
    IN    GT_U32                             portGroupId,
    IN    PRV_CPSS_DIAG_REGS_SEARCH_TYPE_ENT searchType,
    IN    GT_U32                             *regsListPtr,
    IN    GT_U32                             regsListSize,
    IN    GT_U32                             offset,
    IN    GT_U32                             unitIdAddrPattern,
    IN    GT_U32                             unitIdAddrMask,
    IN    PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC checkRegExistFuncPtr,
    INOUT GT_U32                             *regsNumPtr,
    OUT   GT_U32                             *regsInfoPtr
)
{
    GT_U32 i;
    GT_U32 foundRegsNum = 0;
    GT_STATUS status;
    GT_U32 tempRegsNum;
        PRV_CPSS_BOOKMARK_STC *bookmark;

    switch(searchType)
    {
        case PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E:
        case PRV_CPSS_DIAG_REGS_ADDR_SEARCH_E:
        case PRV_CPSS_DIAG_REGS_DATA_SEARCH_E:
        case PRV_CPSS_DIAG_REGS_DATA_RESET_AND_INIT_CTRL_SEARCH_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; i < regsListSize; i++)
    {
        if (regsListPtr[i] == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            continue; /* Skip unused entry */
        }
        if (regsListPtr[i] == PRV_CPSS_SW_PTR_BOOKMARK_CNS)
        {
            bookmark = (PRV_CPSS_BOOKMARK_STC*)(&(regsListPtr[i]));
            /*Check for write only data*/
            if (bookmark->type & PRV_CPSS_SW_TYPE_WRONLY_CNS)
            {
                /* if the pointer field data is write only skip the pointer,
                   else skip the whole range*/
                if (bookmark->type & PRV_CPSS_SW_TYPE_PTR_FIELD_CNS)
                {
                    i += sizeof(*bookmark)/sizeof(GT_U32) - 1;
                }
                else
                {
                    i += 1 + bookmark->size;
                }
            }
            else
            {
                 /* assign temp register number to be reminder from already found */
                tempRegsNum = *regsNumPtr - foundRegsNum;

                regsInfoSearch (devNum,
                                portGroupId,
                                searchType,
                                bookmark->nextPtr,
                                bookmark->size / sizeof(GT_U32),
                                offset,
                                unitIdAddrPattern,
                                unitIdAddrMask,
                                checkRegExistFuncPtr,
                                &tempRegsNum,  /* supply temp registers number for recursive call */
                                regsInfoPtr);
                i += sizeof(*bookmark)/sizeof(GT_U32) - 1;

                /* move pointer to new position after recursive call */
                regsInfoPtr += tempRegsNum;

                /* increment found registers number */
                foundRegsNum += tempRegsNum;

            }
        }
        /* regular entry - address should be bigger than offset*/
        else if ((regsListPtr[i] >= offset) && ((regsListPtr[i] & unitIdAddrMask) == unitIdAddrPattern ))
        {
            /* check if function for register existance checking is specified */
            if (checkRegExistFuncPtr != NULL)
            {
                /* check if register address exist in specific port group */
                if (checkRegExistFuncPtr(devNum,portGroupId,regsListPtr[i]) == GT_FALSE)
                {
                    continue;
                }
            }

            if ((searchType == PRV_CPSS_DIAG_REGS_DATA_SEARCH_E) ||
                (searchType == PRV_CPSS_DIAG_REGS_DATA_RESET_AND_INIT_CTRL_SEARCH_E))
            {
                if (foundRegsNum < *regsNumPtr)
                {
                    /* For data searching read the data from the hardware */
                    if(searchType == PRV_CPSS_DIAG_REGS_DATA_SEARCH_E)
                    {
                        status = prvCpssHwPpPortGroupReadRegister(devNum,
                                                                  portGroupId,
                                                                  regsListPtr[i],
                                                                  regsInfoPtr);
                    }
                    else /* searchType == PRV_CPSS_DIAG_REGS_DATA_RESET_AND_INIT_CTRL_SEARCH_*/
                    {
                        status = prvCpssDrvHwPpResetAndInitControllerReadReg(
                                                                devNum,
                                                                regsListPtr[i],
                                                                regsInfoPtr);
                    }

                    if (status != GT_OK)
                    {
                        *regsNumPtr = foundRegsNum;
                        return status;
                    }

                    regsInfoPtr++;
                    foundRegsNum += 1;
                }
                else
                    break;
            }
            else if (searchType == PRV_CPSS_DIAG_REGS_ADDR_SEARCH_E)
            {
                /* For addresses searching take the address from the input
                   registers list */
                if (foundRegsNum < *regsNumPtr)
                {
                    *regsInfoPtr = regsListPtr[i];
                    foundRegsNum += 1;
                    regsInfoPtr++;
                }
                else
                    break;
            }
            else /* PRV_CPSS_DIAG_REGS_NUM_ONLY_SEARCH_E */
            {
                foundRegsNum += 1;
            }
        }
    }

    /* store number of found registers */
    *regsNumPtr = foundRegsNum;


    return GT_OK;
}

/**
* @internal testRegisterRandom function
* @endinternal
*
* @brief   Tests the device specific register, by writing a random pattern to it
*         and verifying the value.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS testRegisterRandom
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroupId,
    IN GT_U32                    regAddr,
    IN GT_U32                    regMask,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    GT_U32 writeValue = cpssOsRand();
    GT_U32 readValue;
    GT_STATUS status;

    /* Start the test - Write value into the register */
    status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                     regAddr, regMask, writeValue);
    if (status != GT_OK)
    {
        return status;
    }
    /* Read the register value */
    status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                    regAddr, regMask, &readValue);
    if (status != GT_OK)
    {
        return status;
    }

    /* Check if the test passed */
    if ((writeValue & regMask) == (readValue & regMask))
    {
        *testStatusPtr = GT_TRUE;
    }
    else
    {
        *testStatusPtr = GT_FALSE;
        *readValPtr = readValue;
        *writeValPtr = writeValue;
    }

    return GT_OK;
}

/**
* @internal testRegisterIncremental function
* @endinternal
*
* @brief   Tests the device specific register, by writing an incremental pattern to
*         it and verifying the value.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS testRegisterIncremental
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroupId,
    IN GT_U32                    regAddr,
    IN GT_U32                    regMask,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    static GT_U32 writeValue;
    GT_U32 readValue;
    GT_STATUS status;

    /* increment writeValue */
    writeValue = (writeValue + 1) % (0xFFFFFFFF);

    /* Start the test - Write value into the register */
    status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                     regAddr, regMask, writeValue);
    if (status != GT_OK)
    {
        return status;
    }
    /* Read the register value */
    status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                    regAddr, regMask, &readValue);
    if (status != GT_OK)
    {
        return status;
    }

    /* Check if the test passed */
    if ((writeValue & regMask) == (readValue & regMask))
    {
        *testStatusPtr = GT_TRUE;
    }
    else
    {
        *testStatusPtr = GT_FALSE;
        *readValPtr = readValue;
        *writeValPtr = writeValue;
    }

    return GT_OK;
}

/**
* @internal testRegisterToggle function
* @endinternal
*
* @brief   Tests the device specific register, by toggling each of the register
*         bits and verifying the value.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS testRegisterToggle
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroupId,
    IN GT_U32                    regAddr,
    IN GT_U32                    regMask,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    GT_U32 testedBit, writeValue, readValue;
    GT_STATUS status;

    *testStatusPtr = GT_TRUE;

    for (testedBit = 0; testedBit < 32; testedBit++)
    {
        /* Start testing the bit - Write value into the register */
        writeValue = 1 << testedBit;
        status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                         regAddr, regMask,
                                                         writeValue);
        if (status != GT_OK)
        {
            return status;
        }

        /* Read the register value */
        status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                        regAddr, regMask,
                                                        &readValue);
        if (status != GT_OK)
        {
            return status;
        }

        /* Check if the test passed */
        if ((writeValue & regMask) != (readValue & regMask))
        {
            *testStatusPtr = GT_FALSE;
            *readValPtr = readValue;
            *writeValPtr = writeValue;
            return GT_OK;
        }

        /* Repeat the test with toggle bit */
        writeValue = ~writeValue;
        status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                         regAddr, regMask,
                                                         writeValue);
        if (status != GT_OK)
        {
            return status;
        }

        /* Read the register value */
        status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                        regAddr, regMask,
                                                        &readValue);
        if (status != GT_OK)
        {
            return status;
        }

        /* Check if the test passed */
        if ((writeValue & regMask) != (readValue & regMask))
        {
            *testStatusPtr = GT_FALSE;
            *readValPtr = readValue;
            *writeValPtr = writeValue;
            return GT_OK;
        }
    }
    return GT_OK;
}

/**
* @internal testRegisterAA55 function
* @endinternal
*
* @brief   Tests the device specific register, by writing "AAAAAAAA" and "55555555"
*         and verifying the values.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS testRegisterAA55
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroupId,
    IN GT_U32                    regAddr,
    IN GT_U32                    regMask,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    GT_U32 writeValue = 0xAAAAAAAA;
    GT_U32 readValue;
    GT_STATUS status;

    *testStatusPtr = GT_TRUE;

    /* Start the test - Write "0xAAAAAAAA" into the register */
    status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                     regAddr, regMask,
                                                     writeValue);
    if (status != GT_OK)
    {
        return status;
    }
    /* Read the register value */
    status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                    regAddr, regMask,
                                                    &readValue);
    if (status != GT_OK)
    {
        return status;
    }

    /* Check if the test passed */
    if ((writeValue & regMask) != (readValue & regMask))
    {
        *testStatusPtr = GT_FALSE;
        *readValPtr = readValue;
        *writeValPtr = writeValue;
        return GT_OK;
    }

    /* Write "0x55555555" into the register */
    writeValue = 0x55555555;
    status = prvCpssHwPpPortGroupWriteRegBitMask (devNum, portGroupId,
                                                     regAddr, regMask,
                                                     writeValue);
    if (status != GT_OK)
    {
        return status;
    }
    /* Read the register value */
    status = prvCpssHwPpPortGroupReadRegBitMask (devNum, portGroupId,
                                                    regAddr, regMask,
                                                    &readValue);
    if (status != GT_OK)
    {
        return status;
    }

    /* Check if the test passed */
    if ((writeValue & regMask) != (readValue & regMask))
    {
        *testStatusPtr = GT_FALSE;
        *readValPtr = readValue;
        *writeValPtr = writeValue;
    }

    return GT_OK;
}

/**
* @internal prvCpssDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - The value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - The value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
GT_STATUS prvCpssDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    GT_U32      originalVal;        /* register original value  */
    GT_U32      portGroupId;        /* port group id            */
    GT_STATUS   status;             /* return code              */
    GT_STATUS   status1;             /* return code              */

    /* loop on all port groups */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {

        /* Save original register value */
        status = prvCpssHwPpPortGroupReadRegister (devNum, portGroupId,
                                                      regAddr, &originalVal);
        if (status != GT_OK)
        {
            return status;
        }

        switch (profile)
        {
            case CPSS_DIAG_TEST_RANDOM_E:
                status = testRegisterRandom (devNum, portGroupId, regAddr, regMask,
                                             testStatusPtr, readValPtr,
                                             writeValPtr);
                break;

            case CPSS_DIAG_TEST_INCREMENTAL_E:
                status = testRegisterIncremental (devNum, portGroupId, regAddr,
                                                  regMask,testStatusPtr,
                                                  readValPtr, writeValPtr);
                break;

            case CPSS_DIAG_TEST_BIT_TOGGLE_E:
                status = testRegisterToggle (devNum, portGroupId, regAddr,
                                             regMask, testStatusPtr,
                                             readValPtr, writeValPtr);
                break;

            case CPSS_DIAG_TEST_AA_55_E:
                status = testRegisterAA55 (devNum, portGroupId, regAddr,
                                           regMask, testStatusPtr,
                                           readValPtr, writeValPtr);
                break;

            default:
                status = GT_BAD_PARAM;
        }

        /* Restore original register value */
        if (status != GT_OK)
        {
            status1 = prvCpssHwPpPortGroupWriteRegister (devNum, portGroupId, regAddr,
                                                            originalVal);
            if(status1 != GT_OK)
                return status1;
            else
                return status;
        }

        status = prvCpssHwPpPortGroupWriteRegister (devNum, portGroupId, regAddr,
                                                       originalVal);
        if (status != GT_OK)
        {
            return status;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum         - The device number
* @param[in] regsPtr        - The array of register addresses to test
* @param[in] regMasksPtr    - The array of register masks to test
*                             Bits out of masks wriitten as read and not checked.
*                             This pointer can be NULL - so 0xFFFFFFFF masks will be used
* @param[in] regsArraySize  - Number of registers in regsArray
*
* @param[out] testStatusPtr  - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] badRegPtr     - Address of the register which caused the failure if
*                             testStatusPtr is GT_FALSE. Irrelevant if is GT_TRUE.
* @param[out] readValPtr    - The value read from the register which caused the
*                             failure if testStatusPtr is GT_FALSE. Irrelevant if
*                             testStatusPtr is GT_TRUE
* @param[out] writeValPtr   - The value written to the register which caused the
*                             failure if testStatusPtr is GT_FALSE. Irrelevant if
*                             testStatusPtr is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking prvCpssDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagAllRegTest
(
    IN  GT_U8   devNum,
    IN  GT_U32  *regsPtr,
    IN  GT_U32  *regMasksPtr,
    IN  GT_U32  regsArraySize,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
)
{
    GT_U32 i;
    GT_STATUS status;
    GT_U32 regAddr;
    GT_U32 regMask;

    for (i = 0; i < regsArraySize; i++)
    {
        regAddr = regsPtr[i];
        regMask = (regMasksPtr ? regMasksPtr[i] : 0xFFFFFFFF);
        *badRegPtr = regAddr;

        /* AA_55 test */
        status = prvCpssDiagRegTest (devNum, regAddr, regMask,
                                     CPSS_DIAG_TEST_AA_55_E, testStatusPtr,
                                     readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);

        /* Random test */
        status = prvCpssDiagRegTest (devNum, regAddr, regMask,
                                     CPSS_DIAG_TEST_RANDOM_E, testStatusPtr,
                                     readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);

        /* Bits toggle test */
        status = prvCpssDiagRegTest (devNum, regAddr, regMask,
                                     CPSS_DIAG_TEST_BIT_TOGGLE_E, testStatusPtr,
                                     readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);

        /* incremental test */
        status = prvCpssDiagRegTest (devNum, regAddr, regMask,
                                     CPSS_DIAG_TEST_INCREMENTAL_E,
                                     testStatusPtr, readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);
    }
    return GT_OK;
}

/* check if the device supports 8 regions .

    returns :
        mode before the change:
        4 - 4 regions
        8 - 8 regions
*/
static GT_U32 pexNumRegionsSet
(
    IN GT_UINTPTR                      baseAddr,
    IN GT_BOOL                         doByteSwap,
    IN GT_U32                          numOfRegions
)
{
    GT_U32      retVal;
    GT_U32      addCompCntrlVal;/*address of 'address completion control' register */
    GT_UINTPTR  addCompCntrlReg;/*value of 'address completion control' register */

    if(hwUsed == 0 && gmUsed == 0)
    {
        return 4;
    }

    addCompCntrlReg = baseAddr + 0x140;

    /* register addCompCntrl : get 8-region mode: regAddr = 0x140, bit16  :
        0 -->
            Legacy address completion mode.
            Notice that when working on address completion legacy mode , the user works with Address_completion register (Address = 0x0).
        0x0 = 8 regions address completion mode
        0x1 = 4 regions address completion mode; (Legacy mode)
    */
    READ_DATA_FROM_ADDR_MAC(addCompCntrlReg, addCompCntrlVal);
    addCompCntrlVal = DIAG_BYTE_SWAP_MAC(doByteSwap, addCompCntrlVal);

    if(numOfRegions == 8 && (addCompCntrlVal & BIT_16))
    {
        retVal = 4;

        /* 4 regions mode ... change it to 8 regions mode ... */
        addCompCntrlVal &= (~BIT_16);/* clear bit 16 */
        PCI_1_WRITE_DATA_INTO_ADDR_MAC(addCompCntrlReg , doByteSwap , addCompCntrlVal);
        GT_SYNC;
    }
    else
    if(numOfRegions == 4 && (0 == (addCompCntrlVal & BIT_16)))
    {
        retVal = 8;
        /* 8 regions mode ... change it to 4 regions mode ... */
        addCompCntrlVal |= BIT_16;/* set bit 16 */
        PCI_1_WRITE_DATA_INTO_ADDR_MAC(addCompCntrlReg , doByteSwap , addCompCntrlVal);
        GT_SYNC;
    }
    else
    {
        /* no need to change mode */
        retVal = (addCompCntrlVal & BIT_16) ? 4 : 8;
    }

    return retVal;

}

/* DB to hold the device types that the CPSS support */
extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const /*CORES_INFO_STC*/void *coresInfoPtr;
    const GT_U32                *activeCoresBmpPtr;
}cpssSupportedTypes[];
#define END_OF_TABLE    0xFFFFFFFF

/**
* @internal diag_getDevFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] deviceId                 - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr             - (pointer to) CPSS PP device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS diag_getDevFamily(
    IN  GT_U16                  deviceId,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr
)
{
    CPSS_PP_DEVICE_TYPE deviceType = ((deviceId) << 16) | (0x11AB);
    GT_U32                  ii;
    GT_U32                  jj;
    GT_BOOL                 found = GT_FALSE;

    /* get the info about our device */
    ii = 0;
    while (cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while (cpssSupportedTypes[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if (deviceType == cpssSupportedTypes[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if (GT_TRUE == found)
        {
            break;
        }
        ii++;
    }

    /* check if deviceType was found */
    if (GT_TRUE == found)
    {
        /* get family type from CPSS DB */
        *devFamilyPtr = cpssSupportedTypes[ii].devFamily;
    }
    else  /* device not managed by CPSS */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal deviceIsPrestera function
* @endinternal
*
* @brief   check if the device is of Prestera family .
*         the function checks register 0x50 to check that 0x11AB exists in it.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
*
* @param[out] needSwapPtr              - (pointer to) indication that need swap (achieved by read of register 0x50)
* @param[out] devFamilyPtr             - (pointer to) the device family
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS deviceIsPrestera
(
    IN GT_UINTPTR                      baseAddr,
    OUT GT_BOOL                        *needSwapPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT        *devFamilyPtr
)
{
    GT_U32  data;
    GT_UINTPTR  address;
    GT_U32 mgOffset;

    {
        GT_U8   devNum = 0;
        if(PRV_CPSS_PP_MAC(devNum) &&
           PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* need to support 'diag' functions for Falcon ... */
            CPSS_TBD_BOOKMARK_FALCON
            mgOffset = PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS;
        }
        else
        {
            mgOffset = 0x00000000;
        }
    }

    address = mgOffset + 0x50;

    if(hwUsed || gmUsed)
    {
        address += baseAddr;
    }

#ifdef ASIC_SIMULATION
    scibReadMemory(simulationDeviceId,address,1,&data);
#else
    READ_DATA_FROM_ADDR_MAC(address, data);
#endif

    if(data == 0x0011AB)
    {
        *needSwapPtr = GT_FALSE;
    }
    else
    if(data == 0xAB110000)/* check with swapped bytes */
    {
        *needSwapPtr = GT_TRUE;
    }
    else
    {
        /* register 0x50 not hold 0x11AB ??? */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = mgOffset + 0x4c;
    if(hwUsed || gmUsed)
    {
        address += baseAddr;
    }
#ifdef ASIC_SIMULATION
    scibReadMemory(simulationDeviceId,address,1,&data);
#else
    READ_DATA_FROM_ADDR_MAC(address, data);
#endif

    data = DIAG_BYTE_SWAP_MAC(*needSwapPtr,data);

    data >>= 4;

    return diag_getDevFamily((GT_U16)data , devFamilyPtr);
}

/* check if the device supports 8 regions .

    returns :
*       GT_OK   - on success
*       GT_FAIL - on error
*/
static GT_STATUS deviceSupportPex8Regions
(
    IN GT_UINTPTR                      baseAddr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS  rc;
    GT_BOOL    autoSwapDetect = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily = CPSS_PP_FAMILY_DXCH_BOBCAT2_E;

    rc = deviceIsPrestera(baseAddr , &autoSwapDetect , &devFamily);
    if(rc != GT_OK)
    {
        /* this is not prestera device ??? */
        return rc;
    }

    if(autoSwapDetect != doByteSwap)
    {
        cpssOsPrintf("autoSwapDetect[%d] != doByteSwap[%d] \n" , autoSwapDetect , doByteSwap);
    }

    switch(devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PX_FAMILY_PIPE_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            return GT_OK;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal regPciWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write through PCI to one of the PP PCI
*         configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] regData                  - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regPciWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          regData,
    IN GT_BOOL                         doByteSwap
)
{
    GT_UINTPTR  address;
    GT_U32      data;
    GT_UINTPTR  addressCompletionReg;
    GT_U32      origAddrCompletionReg;
    GT_U32      origNumRegions;

#ifdef ASIC_SIMULATION
    simulationDeviceId = scibGetDeviceId(baseAddr);
    if(simulationDeviceId == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#endif /* ASIC_SIMULATION */

    if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
    {
        /* write PCI register */
        address = offset;

        if(hwUsed)
        {
            address += baseAddr;
        }

        PCI_CFG_WRITE_DATA_INTO_ADDR_MAC(address , doByteSwap , regData);
    }
    else /* CPSS_DIAG_PP_REG_INTERNAL_E */
    {
        if(GT_OK ==  deviceSupportPex8Regions(baseAddr,doByteSwap))
        {
            /* set 4 regions */
            origNumRegions = pexNumRegionsSet(baseAddr,doByteSwap,4);
        }
        else
        {
            origNumRegions = 4;
        }

        if(hwUsed == 0 && gmUsed == 0)
        {
            PCI_1_WRITE_DATA_INTO_ADDR_MAC((GT_UINTPTR)offset,doByteSwap , regData);
        }
        else
        {
            /* write PP internal register */
            /* update HW Address Completion - using completion region 2 */
            addressCompletionReg = 0;

            addressCompletionReg += baseAddr;

            /* save address completion value */
            READ_DATA_FROM_ADDR_MAC(addressCompletionReg,origAddrCompletionReg);
            GT_SYNC;

            data = ((GT_U8)(offset >> 24)) << 16;
            PCI_3_WRITE_DATA_INTO_ADDR_MAC(addressCompletionReg , doByteSwap , data);
            GT_SYNC;

            /* for PP register access use region N 2 always */
            address = (DIAG_ADDR_COMPLETION_REGION_CNS << 24) | (offset & 0x00ffffff);
            address += baseAddr;
            PCI_3_WRITE_DATA_INTO_ADDR_MAC(address , doByteSwap , regData);
            GT_SYNC;

            /* restore address completion */
            PCI_3_WRITE_DATA_INTO_ADDR_MAC(addressCompletionReg,GT_FALSE , origAddrCompletionReg);
            GT_SYNC;
        }

        if(origNumRegions != 4)
        {
            /* restore regions */
            (void)pexNumRegionsSet(baseAddr,doByteSwap,origNumRegions);
        }
    }
    return GT_OK;
}

/**
* @internal regPciRead function
* @endinternal
*
* @brief   Performs single 32 bit data read through PCI from one of the PP
*         PCI configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] regDataPtr               - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regPciRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *regDataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_UINTPTR  address;
    GT_U32      data;
    GT_UINTPTR  addressCompletionReg;
    GT_U32      origAddrCompletionReg;
    GT_U32      origNumRegions;

#ifdef ASIC_SIMULATION
    simulationDeviceId = scibGetDeviceId(baseAddr);
    if(simulationDeviceId == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#endif /* ASIC_SIMULATION */

    if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
    {
        /* write PCI register */
        address = offset;

        if(hwUsed)
        {
            address += baseAddr;
        }
        READ_CFG_DATA_FROM_ADDR_MAC(address , data);
    }
    else /* CPSS_DIAG_PP_REG_INTERNAL_E */
    {
        if(GT_OK ==  deviceSupportPex8Regions(baseAddr,doByteSwap))
        {
            /* set 4 regions */
            origNumRegions = pexNumRegionsSet(baseAddr,doByteSwap,4);
        }
        else
        {
            origNumRegions = 4;
        }

        if(hwUsed == 0 && gmUsed == 0)
        {
            READ_DATA_FROM_ADDR_MAC((GT_UINTPTR)offset , data);
        }
        else
        {
            /* write PP internal register */
            /* update HW Address Completion - using completion region 2 */
            addressCompletionReg = 0;

            addressCompletionReg += baseAddr;

            /* save address completion value */
            READ_DATA_FROM_ADDR_MAC(addressCompletionReg,origAddrCompletionReg);
            GT_SYNC;

            data = ((GT_U8)(offset >> 24)) << 16;
            PCI_3_WRITE_DATA_INTO_ADDR_MAC(addressCompletionReg , doByteSwap , data);
            GT_SYNC;

            /* for PP register access use region N 2 always */
            address = (DIAG_ADDR_COMPLETION_REGION_CNS << 24) | (offset & 0x00ffffff);
            address += baseAddr;

            READ_DATA_FROM_ADDR_MAC(address , data);

            /* restore address completion */
            PCI_3_WRITE_DATA_INTO_ADDR_MAC(addressCompletionReg,GT_FALSE , origAddrCompletionReg);
            GT_SYNC;
        }

        if(origNumRegions != 4)
        {
            /* restore regions */
            (void)pexNumRegionsSet(baseAddr,doByteSwap,origNumRegions);
        }
    }

    *regDataPtr = DIAG_BYTE_SWAP_MAC(doByteSwap, data);
    return GT_OK;
}

/**
* @internal regPex8RegionsWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write through PEX/MBUS to one of the PP registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] regData                  - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regPex8RegionsWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN GT_U32                          offset,
    IN GT_U32                          regData,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS   rc;
    GT_UINTPTR  address;
    GT_UINTPTR  AddCompRegion2Reg;/*address of 'address completion region 2' register */
    GT_U32      AddCompRegion2Val,orig_AddCompRegion2Val;/*value of 'address completion region 2' register */
    GT_U32      origNumRegions;

#ifdef ASIC_SIMULATION
    simulationDeviceId = scibGetDeviceId(baseAddr);
    if(simulationDeviceId == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#endif /* ASIC_SIMULATION */

    rc = deviceSupportPex8Regions(baseAddr,doByteSwap);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(hwUsed == 0 && gmUsed == 0)
    {
        return regPciWrite (baseAddr, CPSS_DIAG_PP_REG_INTERNAL_E , offset, regData, doByteSwap);
    }

    /* set 8 regions */
    origNumRegions = pexNumRegionsSet(baseAddr,doByteSwap,8);

    /* the address of the 'region 2' register */
    AddCompRegion2Reg = PEX_MBUS_ADDR_COMP_REG_MAC(DIAG_ADDR_COMPLETION_REGION_CNS);
    AddCompRegion2Reg += baseAddr;

    /* save 'region 2' register value */
    READ_DATA_FROM_ADDR_MAC(AddCompRegion2Reg,orig_AddCompRegion2Val);
    GT_SYNC;

    /* set 'region 2' register value with MSBits of the 'offset' */
    AddCompRegion2Val = offset >> NOT_ADDRESS_COMPLETION_BITS_NUM_CNS;
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(AddCompRegion2Reg,doByteSwap , AddCompRegion2Val);
    GT_SYNC;

    /* for PP register access use region N 2 always */
    /* now we can access the 'offset' with it's LSBits */
    address = (DIAG_ADDR_COMPLETION_REGION_CNS << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS) |
              (offset & (~ ADDRESS_COMPLETION_BITS_MASK_CNS));
    address += baseAddr;

    PCI_1_WRITE_DATA_INTO_ADDR_MAC(address , doByteSwap , regData);
    GT_SYNC;

    /* restore 'region 2' register value */
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(AddCompRegion2Reg,GT_FALSE , orig_AddCompRegion2Val);
    GT_SYNC;

    if(origNumRegions != 8)
    {
        /* restore 4 regions mode */
        (void) pexNumRegionsSet(baseAddr,doByteSwap,origNumRegions);
    }

    return GT_OK;
}

/**
* @internal regPex8RegionsRead function
* @endinternal
*
* @brief   Performs single 32 bit data read through PEX/MBUS from one of the PP registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] regDataPtr               - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regPex8RegionsRead
(
    IN GT_UINTPTR                      baseAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *regDataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS   rc;
    GT_UINTPTR  address;
    GT_UINTPTR  AddCompRegion2Reg;/*address of 'address completion region 2' register */
    GT_U32      AddCompRegion2Val,orig_AddCompRegion2Val;/*value of 'address completion region 2' register */
    GT_U32      origNumRegions;
    GT_U32      regData;

#ifdef ASIC_SIMULATION
    simulationDeviceId = scibGetDeviceId(baseAddr);
    if(simulationDeviceId == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#endif /* ASIC_SIMULATION */

    rc = deviceSupportPex8Regions(baseAddr,doByteSwap);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(hwUsed == 0 && gmUsed == 0)
    {
        return regPciRead (baseAddr, CPSS_DIAG_PP_REG_INTERNAL_E , offset, regDataPtr, doByteSwap);
    }

    /* set 8 regions */
    origNumRegions = pexNumRegionsSet(baseAddr,doByteSwap,8);

    /* the address of the 'region 2' register */
    AddCompRegion2Reg = PEX_MBUS_ADDR_COMP_REG_MAC(DIAG_ADDR_COMPLETION_REGION_CNS);
    AddCompRegion2Reg += baseAddr;

    /* save 'region 2' register value */
    READ_DATA_FROM_ADDR_MAC(AddCompRegion2Reg,orig_AddCompRegion2Val);
    GT_SYNC;

    /* set 'region 2' register value with MSBits of the 'offset' */
    AddCompRegion2Val = offset >> NOT_ADDRESS_COMPLETION_BITS_NUM_CNS;
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(AddCompRegion2Reg,doByteSwap , AddCompRegion2Val);
    GT_SYNC;

    /* for PP register access use region N 2 always */
    /* now we can access the 'offset' with it's LSBits */
    address = (DIAG_ADDR_COMPLETION_REGION_CNS << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS) |
              (offset & (~ ADDRESS_COMPLETION_BITS_MASK_CNS));
    address += baseAddr;

    READ_DATA_FROM_ADDR_MAC(address , regData);
    GT_SYNC;

    *regDataPtr = DIAG_BYTE_SWAP_MAC(doByteSwap, regData);

    /* restore 'region 2' register value */
    PCI_1_WRITE_DATA_INTO_ADDR_MAC(AddCompRegion2Reg,GT_FALSE , orig_AddCompRegion2Val);
    GT_SYNC;

    if(origNumRegions != 8)
    {
        /* restore 4 regions mode */
        (void) pexNumRegionsSet(baseAddr,doByteSwap,origNumRegions);
    }

    return GT_OK;
}


/**
* @internal regSmiWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write through SMI to one of the PP PCI
*         configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regSmiWrite
(
    IN GT_U32                          baseAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    data = DIAG_BYTE_SWAP_MAC(doByteSwap, data);
    return cpssExtDrvHwIfSmiWriteReg (baseAddr, offset, data);
}

/**
* @internal regSmiRead function
* @endinternal
*
* @brief   Performs single 32 bit data read through SMI from one of the PP
*         PCI configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regSmiRead
(
    IN GT_U32                          baseAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS retVal;

    retVal = cpssExtDrvHwIfSmiReadReg (baseAddr, offset, dataPtr);
    *dataPtr = DIAG_BYTE_SWAP_MAC(doByteSwap, *dataPtr);

    return retVal;
}

/**
* @internal regTwsiWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write through TWSI to one of the PP PCI
*         configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regTwsiWrite
(
    IN GT_U32                          baseAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    data = DIAG_BYTE_SWAP_MAC(doByteSwap, data);
    return cpssExtDrvHwIfTwsiWriteReg (baseAddr, offset, data);
}

/**
* @internal regTwsiRead function
* @endinternal
*
* @brief   Performs single 32 bit data read through TWSI from one of the PP
*         PCI configuration or internal registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS regTwsiRead
(
    IN GT_U32                          baseAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS retVal;

    retVal = cpssExtDrvHwIfTwsiReadReg (baseAddr, offset, dataPtr);
    *dataPtr = DIAG_BYTE_SWAP_MAC(doByteSwap, *dataPtr);

    return retVal;
}

/**
* @internal prvCpssDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    /* check input parameter */
    if (ifChannel >= CPSS_CHANNEL_LAST_E || regType > CPSS_DIAG_PP_REG_PCI_CFG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ifChannel)
    {
        case CPSS_CHANNEL_TWSI_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            return regTwsiWrite ((GT_U32)baseAddr, offset, data, doByteSwap);
        case CPSS_CHANNEL_SMI_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            return regSmiWrite ((GT_U32)baseAddr, offset, data, doByteSwap);
        case CPSS_CHANNEL_PEX_MBUS_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                return regPciWrite (baseAddr, regType, offset, data, doByteSwap);
            }
            /* do 8 regions write */
            return regPex8RegionsWrite (baseAddr, offset, data, doByteSwap);
        default:/*pci/pex/mbas*/
            return regPciWrite (baseAddr, regType, offset, data, doByteSwap);
    }
}

/**
* @internal prvCpssDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the registers.
*
* @note   APPLICABLE DEVICES:      All devices.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    /* check input parameter */
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    if (ifChannel >= CPSS_CHANNEL_LAST_E || regType > CPSS_DIAG_PP_REG_PCI_CFG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ifChannel)
    {
        case CPSS_CHANNEL_TWSI_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            return regTwsiRead ((GT_U32)baseAddr, offset, dataPtr, doByteSwap);
        case CPSS_CHANNEL_SMI_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            return regSmiRead ((GT_U32)baseAddr, offset, dataPtr, doByteSwap);
        case CPSS_CHANNEL_PEX_MBUS_E:
            if (regType == CPSS_DIAG_PP_REG_PCI_CFG_E)
            {
                return regPciRead (baseAddr, regType, offset, dataPtr, doByteSwap);
            }
            /* do 8 regions read */
            return regPex8RegionsRead (baseAddr, offset, dataPtr, doByteSwap);
        default:/*pci/pex*/
            return regPciRead (baseAddr, regType, offset, dataPtr, doByteSwap);
    }
}

/**
* @internal prvCpssDfxBuildAddr function
* @endinternal
*
* @brief   Function calculates DFX client/memory address.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - device number
* @param[in] addrStrPtr               - (pointer to) DFX address structure
*
* @param[out] addrPtr                  - (pointer to) calculated address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong addrStrPtr members
*/
GT_STATUS prvCpssDfxBuildAddr
(
    IN  GT_U8                                           devNum,
    IN  PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC       *addrStrPtr,
    OUT GT_U32                                          *addrPtr
)
{
    GT_U32 tmpData; /* local temp variable */

    /* init DFX Server Unit Base Address */
    tmpData = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ?
        (0x18 << 20) : 0;

    if(addrStrPtr->dfxUnitClientIdx >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* bits [19:15] */
    tmpData |= (addrStrPtr->dfxUnitClientIdx << 15);

    if(addrStrPtr->dfxUnitClientIdx == 31)
    {
        if(addrStrPtr->dfxServerRegisterOffset >= BIT_15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* bits [14:0] */
        tmpData |= addrStrPtr->dfxServerRegisterOffset;
    }
    else if(/* (addrStrPtr->dfxUnitClientIdx >= 0) && */(addrStrPtr->dfxUnitClientIdx <= 29))
    {
        /* bit [14] */
        tmpData |= (addrStrPtr->dfxXsbSelect << 14);

        if(addrStrPtr->dfxXsbSelect == 1)
        {
            if((addrStrPtr->dfxXsbNumber >= BIT_6) || (addrStrPtr->dfxXsbRegOffset >= BIT_9))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* bits [8:0] */
            tmpData |= addrStrPtr->dfxXsbRegOffset;
            /* bits [13:9] */
            tmpData |= (addrStrPtr->dfxXsbNumber) << 9;
        }
        else
        {
            /* bit [13] */
            tmpData |= (addrStrPtr->dfxClientSelect << 13);
            if(addrStrPtr->dfxClientSelect == 1)
            {
                if(addrStrPtr->dfxClientRegisterOffset >= BIT_13)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                /* bits [12:0] */
                tmpData |= (addrStrPtr->dfxClientRegisterOffset);
            }
            else
            {
                if((addrStrPtr->dfxClientMemoryNumber >= BIT_7) || (addrStrPtr->dfxClientMemoryRegisterOffset >= BIT_6))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                /* bits [12:6] */
                tmpData |= (addrStrPtr->dfxClientMemoryNumber) << 6;
                /* bits [5:0] */
                tmpData |= (addrStrPtr->dfxClientMemoryRegisterOffset);
            }
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *addrPtr = tmpData;

    return GT_OK;
}

/**
* @internal prvCpssDfxClientSetRegField function
* @endinternal
*
* @brief   Function set a field in a DFX client register.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] pipe                     - DFX pipe the client belongs to.
*                                      (not relevant for BC, MC or Data logging clients).
* @param[in] client                   - DFX client to write to.
* @param[in] clientRegAddr            - The client register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note Since reading BC, MC or Data logging DFX clients registers is prohibited
*       client 0 value is read to emulate the default or already existing value.
*
*/
GT_STATUS prvCpssDfxClientSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   pipe,
    IN GT_U32   client,
    IN GT_U32   clientRegAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_U32  regAddr = 0;    /* register address */
    GT_U32  regData;    /* register data */
    GT_STATUS rc;       /* return code */

    if( (PRV_CPSS_DFX_CLIENT_BC_CNS == client) ||
        (PRV_CPSS_DFX_CLIENT_MC_CNS == client) ||
        (PRV_CPSS_DFX_CLIENT_LOG_CNS == client))
    {
        /* BC, MC or Data logging DFX client */

        if(fieldLength != 32)
        {
            /* read register's data from reference client.
               calculate address */
            rc = prvCpssDfxClientRegAddressGet(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetClient, clientRegAddr, &regAddr);
            if( GT_OK != rc )
            {
                return rc;
            }

            /* Activate reference pipe */
            rc = prvCpssDfxMemoryPipeIdSet(devNum, PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetPipe);
            if( GT_OK != rc )
            {
                return rc;
            }

            /* Read register value in reference client to use as deafult */
            rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
            if( GT_OK != rc )
            {
                return rc;
            }
        }
        else
        {
            /* full register update. Need not to read data from reference client. */
            regData = 0;
        }

        /* Enable all pipes */
        rc = prvCpssDfxMemoryAllPipesSet(devNum);
        if( GT_OK != rc )
        {
            return rc;
        }

        U32_SET_FIELD_MAC(regData, fieldOffset, fieldLength, fieldData);

        rc = prvCpssDfxClientRegAddressGet(client, clientRegAddr, &regAddr);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Write to requested client */
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, regData);
    }
    else /* "Regular" DFX client*/
    {
        rc = prvCpssDfxClientRegAddressGet(client, clientRegAddr, &regAddr);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Activate pipe */
        rc = prvCpssDfxMemoryPipeIdSet(devNum, pipe);
        if( GT_OK != rc )
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,
                                                             regAddr,
                                                             fieldOffset,
                                                             fieldLength,
                                                             fieldData);
    }

    return rc;
}

/**
* @internal prvCpssDfxClientRegAddressGet function
* @endinternal
*
* @brief   Get DFX client register address
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] client                   - DFX client number.
* @param[in] regAddr                  - Relative register address in client memory space.
*
* @param[out] clientRegAddrPtr         - (pointer to) register addrress in specific client
*                                      memory space.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong client number
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxClientRegAddressGet
(
    IN  GT_U32   client,
    IN  GT_U32   regAddr,
    OUT GT_U32   *clientRegAddrPtr
)
{
    /* There are 31 clients:    */
    /* 0-27 simple pipe clients */
    /* 28 - BC client           */
    /* 29 - MC client           */
    /* 30 - data logging client */
    if( client > PRV_CPSS_DFX_CLIENT_LOG_CNS )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* bit 13 is "DFX Client Select" bit */
    *clientRegAddrPtr = BIT_13 | (client << 15) | regAddr;

    return GT_OK;
}

/**
* @internal prvCpssDfxMemoryRegAddressGet function
* @endinternal
*
* @brief   Get DFX memory register address
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] client                   - DFX client number.
* @param[in] memory                   - DFX client RAM number.
* @param[in] regAddr                  - Relative register address in RAM space.
*
* @param[out] memoryRegAddrPtr         - (pointer to) register addrress in for specific client
*                                        memory
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong client or RAM number
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegAddressGet
(
    IN  GT_U32   client,
    IN  GT_U32   memory,
    IN  GT_U32   regAddr,
    OUT GT_U32   *memoryRegAddrPtr
)
{
    /* There are 31 clients:    */
    /* 0-27 simple pipe clients */
    /* 28 - BC client           */
    /* 29 - MC client           */
    /* 30 - data logging client */
    if( client > PRV_CPSS_DFX_CLIENT_LOG_CNS )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* There are up to 128 RAMs per client */
    if( memory > 0x7F )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *memoryRegAddrPtr = (client << 15) + (memory << 6) + regAddr;

    return GT_OK;
}

/**
* @internal prvCpssDfxMemoryAllPipesSet function
* @endinternal
*
* @brief   Function sets all pipes in DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryAllPipesSet
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */
    GT_U32 allPipesBmp; /* bitmap of all pipes */


    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                    DFXServerUnits.DFXServerRegs.pipeSelect;

    if(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxPipesBmpGetFunc)
    {
        /* call to CB function to get the results */
        rc = PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxPipesBmpGetFunc(devNum, &allPipesBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* Use only first DFX pipe for defices like PIPE */
        allPipesBmp = 0x1;
    }

    /* Active all pipes */
    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, allPipesBmp);

    return rc;
}

/**
* @internal prvCpssDfxMemoryPipeIdSet function
* @endinternal
*
* @brief   Function sets DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon, Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryPipeIdSet
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */

    if(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryPipeIdSetFunc)
    {
        /* call to CB function to get the results */
        rc = PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryPipeIdSetFunc(devNum, pipeId);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                    DFXServerUnits.DFXServerRegs.pipeSelect;

        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, (1 << pipeId));
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDfxMemoryErrorInjectionSet function
* @endinternal
*
* @brief   Inject error to a selected DFX client RAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] mode                     - error injection mode
* @param[in] enable                   - enable/disable error injection
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxMemoryErrorInjectionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           pipeId,
    IN GT_U32                                           clientId,
    IN GT_U32                                           memNumber,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT   mode,
    IN GT_BOOL                                          enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regData;    /* register data */

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    rc = prvCpssDfxMemoryRegRead(devNum, pipeId, clientId, memNumber,
                  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                           DFXRam.memoryControl,
                                         &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Error Injection Mode */
    U32_SET_FIELD_MAC(regData, 2, 1,
      ((CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E == mode) ? 0 : 1));

    if( GT_TRUE == enable )
    {
        /* Memory write stop (used by other units) */
        U32_SET_FIELD_MAC(regData, 10, 2, 0x3);
        /* Error Injection Enable */
        U32_SET_FIELD_MAC(regData, 1, 1, 1);

    }
    else /* GT_FALSE == enable */
    {
        /* Memory write stop disable */
        U32_SET_FIELD_MAC(regData, 10, 2, 0x0);
        /* Error Injection Disable */
        U32_SET_FIELD_MAC(regData, 1, 1, 0);
    }

    return prvCpssDfxMemoryRegWrite(devNum, pipeId, clientId, memNumber,
                  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                           DFXRam.memoryControl,
                                         regData);
}

/**
* @internal prvCpssDfxMemoryRegWrite function
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegWrite
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 regData
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */

    /* Set Pipe Select Register */
    rc = prvCpssDfxMemoryPipeIdSet(devNum, pipeId);
    if(GT_OK != rc)
    {
        return rc;
    }
    if(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryWriteFunc)
    {
        /* call to CB function to get the results */
        rc = PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryWriteFunc(devNum, 
                                                         pipeId, 
                                                         clientId, 
                                                         memNumber, 
                                                         memReg, 
                                                         regData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else /* SIP5 */
    {
        rc = prvCpssDfxMemoryRegAddressGet(clientId,
                                           memNumber,
                                           memReg,
                                           &regAddr);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,
                                                          regAddr,
                                                          regData);
    }

    return rc;
}

/**
* @internal prvCpssDfxMemoryRegRead function
* @endinternal
*
* @brief   Function reads DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    OUT GT_U32 *regDataPtr
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */

    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);

    /* Set Pipe Select Register */
    rc = prvCpssDfxMemoryPipeIdSet(devNum, pipeId);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryRegReadFunc)
    {
        /* call to CB function to get the results */
        rc = PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryRegReadFunc(devNum, 
                                                           pipeId, 
                                                           clientId, 
                                                           memNumber, 
                                                           memReg, 
                                                           regDataPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else /* SIP5 */
    {
        rc = prvCpssDfxMemoryRegAddressGet(clientId,
                                           memNumber,
                                           memReg,
                                           &regAddr);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,
                                                         regAddr,
                                                         regDataPtr);
    }

    return rc;
}

/**
* @internal prvCpssDfxClientRegRead function
* @endinternal
*
* @brief   Function reads DFX client registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] clientReg                - DFX client register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxClientRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 clientReg,
    OUT GT_U32 *regDataPtr
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */

    /* Set Pipe Select Register */
    rc = prvCpssDfxMemoryPipeIdSet (devNum, pipeId);

    if(PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxClientReadRegFunc)
    {
        /* call to CB function to get the results */
        rc = PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxClientReadRegFunc(devNum, 
                                                           pipeId, 
                                                           clientId, 
                                                           clientReg, 
                                                           regDataPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC   addrStr;    /* DFX address struct */

        addrStr.dfxUnitClientIdx = clientId;
        addrStr.dfxXsbSelect = 0;
        addrStr.dfxClientSelect = 1;
        addrStr.dfxClientRegisterOffset = clientReg;

        rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
        if(GT_OK != rc)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, 
                                                         regAddr, 
                                                         regDataPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDfxMemoryGetRegField function
* @endinternal
*
* @brief   Function reads DFX memory register's field
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
*
* @param[out] fieldDataPtr             - (pointer to) register's field data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemoryGetRegField
(
    IN GT_U8   devNum,
    IN GT_U32  pipeId,
    IN GT_U32  clientId,
    IN GT_U32  memNumber,
    IN GT_U32  memReg,
    IN GT_U32  fieldOffset,
    IN GT_U32  fieldLength,
    OUT GT_U32 *fieldDataPtr
)
{
    GT_STATUS rc;
    GT_U32    data;
    GT_U32    mask;
    rc = prvCpssDfxMemoryRegRead(devNum, pipeId, clientId, memNumber, memReg,
                                 &data);

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldDataPtr = (GT_U32)((data & mask) >> fieldOffset);

    return rc;
}

/**
* @internal prvCpssDfxMemorySetRegField function
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDfxMemorySetRegField
(
    IN GT_U8  devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
)
{
    GT_STATUS rc;
    GT_U32    regData;
    GT_U32    mask;
    rc = prvCpssDfxMemoryRegRead(devNum, pipeId, clientId, memNumber, memReg,
                                 &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    regData &= ~mask;
    regData |= ((fieldData << fieldOffset) & mask);

    return prvCpssDfxMemoryRegWrite(devNum, pipeId, clientId, memNumber, memReg,
                                    regData);
}

/**
* @internal prvCpssDfxDiagBistStartSet function
* @endinternal
*
* @brief   Start Stop MC BIST on specific DFX clients registers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] enable                   - GT_TRUE:  Enable and Start.
*                                      GT_FALSE: Disable and Stop.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssDfxDiagBistStartSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;       /* return code */

    /* Start/Stop MC Bist */
    rc = prvCpssDfxClientSetRegField(
            devNum,
            0, PRV_CPSS_DFX_CLIENT_MC_CNS,
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.clientControl,
            0, 1, (enable ? 1 : 0));

    if( GT_OK != rc )
    {
        return rc;
    }

    /* Performing BIST for the first time after system start-up while    */
    /* injecting errors cause may false alarms (BIST reporting errors on */
    /* some RAMs although none really occurred. A workaround to overcome */
    /* this problem is adding the following short sleep.                 */
    cpssOsTimerWkAfter(1);

    /* Enable/Disable MC bist */
    rc = prvCpssDfxClientSetRegField(
            devNum,
            0, PRV_CPSS_DFX_CLIENT_MC_CNS,
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.BISTControl,
            0, 23, (enable ? 0x2011 : 0x2010));

    return rc;
}


/**
* @internal prvCpssDfxDiagBistStatusCheck function
* @endinternal
*
* @brief  Check BIST status of all relevant clients.
*         Configuration done according to memory map DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                - PP device number
* @param[in] dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*  
* @param[out] resultsStatusPtr        - (pointer to) BIST results status
*  
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDfxDiagBistStatusCheck
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              dfxClientsBmpArr[],
    OUT PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT   *resultsStatusPtr
)
{
    /* array of DFX clients bitmaps, index is pipe */
    GT_U32   regAddr;  /* register's address     */
    GT_U32   regData;  /* register's data     */
    GT_U32   pipe;     /* DFX pipe               */
    GT_U32   client;   /* DFX client             */
    GT_U32   clientRegAddr;   /* DFX client register address             */
    GT_STATUS rc;       /* return code            */
    GT_BOOL  bistResult;


    CPSS_NULL_PTR_CHECK_MAC(resultsStatusPtr);

    *resultsStatusPtr = PRV_CPSS_DFX_DIAG_BIST_STATUS_NOT_READY_E;


    bistResult = GT_TRUE;

    clientRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                             DFXClientUnits.clientStatus;

    for (pipe = 0; pipe < PRV_CPSS_DFX_MAX_PIPES_CNS; pipe++)
    {
        if (dfxClientsBmpArr[pipe] == 0)
        {
            /* there are no clients on the pipe */
            continue;
        }

        /* activate pipe */
        rc = prvCpssDfxMemoryPipeIdSet(devNum, pipe);
        if( GT_OK != rc )
        {
            return rc;
        }

        for (client = 0; client < PRV_CPSS_DFX_MAX_CLIENTS_CNS; client++)
        {
            if (dfxClientsBmpArr[pipe] & (1 << client))
            {
                /* calculate full address of client's register */
                rc = prvCpssDfxClientRegAddressGet(client,clientRegAddr, &regAddr);
                if( GT_OK != rc )
                {
                    return rc;
                }

                rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                                                                     0, 2, &regData);
                if( GT_OK != rc )
                {
                    return rc;
                }

                /* check BIST status:
                   bit#0 - BIST Done: 0x0 = Busy or Idle;  0x1 = Done
                   bit#1 - BIST Result: 0x0 = Pass; 0x1 = Fail */
                if ((regData & 1) == 0)
                {
                    /* BIST is not done yet. resultsStatusPtr is already set above */
                    return GT_OK;
                }

                if (regData & 2)
                {
                    /* at least one client failed */
                    bistResult = GT_FALSE;
                }
            }
        }
    }

    *resultsStatusPtr = (bistResult == GT_TRUE) ? PRV_CPSS_DFX_DIAG_BIST_STATUS_PASS_E : PRV_CPSS_DFX_DIAG_BIST_STATUS_FAIL_E;
    
    return GT_OK;
}

