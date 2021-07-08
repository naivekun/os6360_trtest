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
* @file prvCpssDxChRouterTunnelTermTcam.c
*
* @brief CPSS DxCh private router / tunnel termination TCAM.
*
* @version   45
********************************************************************************
*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>


/* forward declaration */
static GT_STATUS prvCpssDxChRouterTunnelTermTcamErrataTcamEccInterruptLock
(
    IN    GT_U8                             devNum,
    IN    GT_BOOL                           lock,
    INOUT GT_U32                            *savedStatePtr
);

static GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex
);

#if PRV_CPSS_DXCH_ERRATA_REFERENCE_CNS  /* macro for references purpose only -- must be 0 --> never 1 */
    /* note: the file uses 'direct read' from the ROUTER TCAM , so not needed
             the WA to FEr#2004 :
             Policy and Router TCAM Tables indirect read is not functional
       see --> PRV_CPSS_DXCH3_POLICY_AND_ROUTER_TCAM_TABLES_INDIRECT_READ_WA_E */
    PRV_CPSS_DXCH3_POLICY_AND_ROUTER_TCAM_TABLES_INDIRECT_READ_WA_E

    /* note: the file uses 'direct write' to the ROUTER TCAM , so not needed
             the WA to FEr#2744 :
             Indirect access to TCAM (Router and PCL) is not supported
       see --> PRV_CPSS_DXCH_LION_INDIRECT_ACCESS_TO_TCAM_IS_NOT_SUPPORTED_WA_E */
    PRV_CPSS_DXCH_LION_INDIRECT_ACCESS_TO_TCAM_IS_NOT_SUPPORTED_WA_E
#endif /* PRV_CPSS_DXCH_ERRATA_REFERENCE_CNS */


/**
* @internal prvCpssDxChLion2RouterTcamParityCalc function
* @endinternal
*
* @brief   Function calculates parity values for Router TCAM rules.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] dataArr[8]               - The data array to be written.
*
* @param[out] parityPtr                - (pointer to) calculated parity
*                                       None.
*/
GT_VOID prvCpssDxChLion2RouterTcamParityCalc
(
    IN  GT_U32 dataArr[8],
    OUT GT_U32 *parityPtr
)
{
    GT_U32 i;                   /* loop iterator */
    GT_U32 parityIdx;           /* parity bit index */
    GT_U32 globalIdx;           /* global array bit index */
    GT_U32 wordNumber;          /* number of word in array */
    GT_U32 wordOffset;          /* offset in word */
    GT_U32 tempParity = 0;      /* temp local parity */
    GT_U32 parityBits[8];       /* temp local parity array */

    cpssOsMemSet(&parityBits , 0, sizeof(parityBits));

    /*
        The parity calculation is as follows:
        Each line in the ecc memory (x & y) is 8 bits long, that represents
        the parity of 208 tcam bits. Each parity bit is in jumps of 8 bits:

            Data_Bits 0..8..16..  ->  parity_bit{4},     but 48 and 176  -> parity_bit{0}
            Data_Bits 1..9..17..  ->  parity_bit{5},     but 49 and 177  -> parity_bit{3}
            Data_Bits 2..10..18..  -> parity_bit{6},     but 50 and 178  -> parity_bit{1}
            Data_Bits 3..11..19..  -> parity_bit{7},     but 51 and 179  -> parity_bit{2}
            Data_Bits 4..12..20..  -> parity_bit{0},     but 112 and 240 -> parity_bit{4}
            Data_Bits 5..13..21..  -> parity_bit{1},     but 113 and 241 -> parity_bit{7}
            Data_Bits 6..14..22..  -> parity_bit{2},     but 114 and 242 -> parity_bit{5}
            Data_Bits 7..15..23..  -> parity_bit{3},     but 115 and 243 -> parity_bit{6}
    */

    for (parityIdx = 0; parityIdx < 8; parityIdx++)
    {
        for (i = 0 + parityIdx; i < 208; i += 8)
        {
            globalIdx = i % 52 + ((i / 52) << 6);

            wordNumber = globalIdx >> 5;
            wordOffset = globalIdx & 0x1F;

            if ((globalIdx == 48) || (globalIdx == 176))
            {
                parityBits[0] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 112) || (globalIdx == 240))
            {
                parityBits[4] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 49) || (globalIdx == 177))
            {
                parityBits[3] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 113) || (globalIdx == 241))
            {
                parityBits[7] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 50) || (globalIdx == 178))
            {
                parityBits[1] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 114) || (globalIdx == 242))
            {
                parityBits[5] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 51) || (globalIdx == 179))
            {
                parityBits[2] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else if ((globalIdx == 115) || (globalIdx == 243))
            {
                parityBits[6] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
            else
            {
                parityBits[(parityIdx + 4) % 8] ^= U32_GET_FIELD_MAC(dataArr[wordNumber], wordOffset, 1);
            }
        }
    }

    for (parityIdx = 0; parityIdx < 8; parityIdx++)
    {
        tempParity |= parityBits[parityIdx] << parityIdx;
    }

    *parityPtr = tempParity;

    return;
}


/**
* @internal prvCpssDxChRouterTcamDaemonDisable function
* @endinternal
*
* @brief   Store and disable daemon. Calculate partial TCAM x/y parity.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] columnIndex              - Column index in the TCAM line (0 to 3).
* @param[in] xDataArr[8]              - X Parity line array
* @param[in] yDataArr[8]              - Y Parity line array
*
* @param[out] xParityPtr               - (pointer to) partial calculated parity
* @param[out] yParityPtr               - (pointer to) partial calculated parity
* @param[out] enablePtr                - (pointer to) current daemon state
*                                       None.
*/
static GT_STATUS prvCpssDxChRouterTcamDaemonDisable
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      lineIndex,
    IN  GT_U32      columnIndex,
    IN  GT_U32      xDataArr[8],
    IN  GT_U32      yDataArr[8],
    OUT GT_U32      *xParityPtr,
    OUT GT_U32      *yParityPtr,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      hwAddr;         /* hardware address */
    GT_U32      entryIndex;     /* index of the entry to write  */
    GT_U32      xParityPartial; /* partial calculated parity X  */
    GT_U32      yParityPartial; /* partial calculated parity Y  */

    /* store current state */
    rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, enablePtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* disable parity daemon */
    rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Read current X parity */
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadPatternParityBase + lineIndex * 4;
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, hwAddr, xParityPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read current Y parity */
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadMaskParityBase + lineIndex * 4;
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, hwAddr, yParityPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read current entry */
    entryIndex = lineIndex * 4 + columnIndex;
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] + (entryIndex * 0x10);

    rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,hwAddr,2,&(xDataArr[columnIndex*2]));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,hwAddr+8,2,&(yDataArr[columnIndex*2]));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate current partial parity */
    prvCpssDxChLion2RouterTcamParityCalc(xDataArr, &xParityPartial);

    prvCpssDxChLion2RouterTcamParityCalc(yDataArr, &yParityPartial);

    /* remove from current parity calculation parity of updated entry  */
    *xParityPtr ^= xParityPartial;
    *yParityPtr ^= yParityPartial;

    return GT_OK;
}


/**
* @internal prvCpssDxChRouterTcamDaemonRestore function
* @endinternal
*
* @brief   Restore daemon state. Update TCAM x/y parity.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] xDataArr[8]              - updated X Parity line array
* @param[in] yDataArr[8]              - updated Y Parity line array
* @param[in] xParity                  - current X parity
* @param[in] yParity                  - current Y parity
* @param[in] enable                   - current daemon state for restore
*                                       None.
*/
static GT_STATUS prvCpssDxChRouterTcamDaemonRestore
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      lineIndex,
    IN  GT_U32      xDataArr[8],
    IN  GT_U32      yDataArr[8],
    IN  GT_U32      xParity,
    IN  GT_U32      yParity,
    IN  GT_BOOL     enable
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      hwAddr;         /* hardware address */
    GT_U32      xParityPartial; /* partial calculated parity X  */
    GT_U32      yParityPartial; /* partial calculated parity Y  */

    /* calculate partial parity for the new data */
    prvCpssDxChLion2RouterTcamParityCalc(xDataArr, &xParityPartial);

    prvCpssDxChLion2RouterTcamParityCalc(yDataArr, &yParityPartial);

    /* add to parity calculation parity of updated entry  */
    xParity ^= xParityPartial;
    yParity ^= yParityPartial;

    /* Write current X parity */
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadPatternParityBase + lineIndex * 4;
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, hwAddr, xParity);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Write current Y parity */
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadMaskParityBase + lineIndex * 4;
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, hwAddr, yParity);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* restore current state */
    return cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, enable);
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamSetLine function
* @endinternal
*
* @brief   Sets a single line in the router / tunnel termination TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3. 
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to
* @param[in] tcamKeyPtr               - TCAM key to write - hw format
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format
* @param[in] tcamActionPtr            - TCAM action (rule) to write - hw format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The line is set as valid.
*       for Cheetah3:
*       X,Y to Data + Mask translation
*       ------------------------------
*       \  Data  \  Mask  \   X  \   Y  \ Result
*       ----------------------------------------------------------------
*       \ Dont Care\   0  \   0  \   0  \ Always Hit
*       \  0   \   1  \   0  \   1  \ Match 0 (no mask)
*       \  1   \   1  \   1  \   0  \ Match 1 (no mask)
*       \ Dont Care\ Dont Care\   1  \   1  \ Always Miss
*       When mask is 0 Data is always reffered as '0' during translation from
*       Data & Mask to X & Y.
*
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamSetLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr,
    IN  GT_U32          *tcamActionPtr
)
{
    GT_U32  rc = GT_OK;              /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamActionPtr);

    /******************************/
    /* part 1: set key to invalid */
    /******************************/

    rc = prvCpssDxChRouterTunnelTermTcamInvalidateLine(devNum,portGroupId,lineIndex);
    if (rc != GT_OK)
        return rc;

    /*********************************************************/
    /* part 2: write the TCAM action while line is disabled  */
    /*********************************************************/
    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH3_LTT_TT_ACTION_E,
                                             lineIndex,
                                             tcamActionPtr);

    if (rc != GT_OK)
        return rc;

    /************************************/
    /* part 3: write the TCAM mask & key*/
    /************************************/

    rc =  prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine(devNum,
                                                          portGroupId,
                                                          lineIndex,
                                                          tcamKeyPtr,
                                                          tcamMaskPtr);

    if (rc != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal prvCpssDxChXcatRouterTunnelTermTcamGetLine function
* @endinternal
*
* @brief   Reads a tunnel termination Line from the router / tunnel termination
*         TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to read
*
* @param[out] validPtr                 - points to array of TCAM entry valid bits
* @param[out] compareModePtr           - points to compare mode of the TCAM entry:
*                                      0: single array compare mode
*                                      1: row compare mode
* @param[out] tcamKeyPtr               - points to TCAM key read - hw format
* @param[out] tcamMaskPtr              - points to TCAM mask read - hw format
* @param[out] tcamActionPtr            - points to TCAM action (rule) read - hw format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note X,Y to Data + Mask translation
*       ------------------------------
*       \  Data  \  Mask  \   X  \   Y  \ Result
*       ----------------------------------------------------------------
*       \ Dont Care\   0  \   0  \   0  \ Always Hit
*       \  0   \   1  \   0  \   1  \ Match 0 (no mask)
*       \  1   \   1  \   1  \   0  \ Match 1 (no mask)
*       \ Dont Care\ Dont Care\   1  \   1  \ Always Miss
*       When mask is 0 Data is always reffered as '0' during translation from
*       Data & Mask to X & Y.
*
*/
static GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamGetLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    OUT GT_U32          *validPtr,
    OUT GT_U32          *compareModePtr,
    OUT GT_U32          *tcamKeyPtr,
    OUT GT_U32          *tcamMaskPtr,
    OUT GT_U32          *tcamActionPtr
)
{
    GT_U32  hwAddr;             /* hardware address             */
    GT_U32  rc;                 /* function return code         */
    GT_U32  lineKey[8];         /* hw line (key)                */
    GT_U32  lineMask[8];        /* hw line (mask)               */
    GT_U32  lineDataX[8];       /* hw line (dataX)              */
    GT_U32  lineDataY[8];       /* hw line (dataY)              */
    GT_U32  wordOffset;         /* offset in key/mask arrays    */
    GT_U32  bank;               /* bank currently manipulated   */
    GT_U32  savedStatus;        /* ECC interrupt enable/disable */
    GT_U32  i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(compareModePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamActionPtr);

    /* get the address of first bank in the line to read */
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] +
             ((lineIndex * 4) * 0x10);

    /* for errata FEr#2050 */
    rc = prvCpssDxChRouterTunnelTermTcamErrataTcamEccInterruptLock(devNum,GT_TRUE/*lock*/,&savedStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* read the line bank by bank; note that in Xcat the indexes are row by row
       and that each bank takes 2 words for dataX and 2 words for dataY */
    for (bank = 0 ; bank < 4 ; bank++)
    {
        rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,hwAddr,2,&lineDataX[bank*2]);
        if (rc != GT_OK)
        {
            break;
        }

        rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,hwAddr+8,2,&lineDataY[bank*2]);
        if (rc != GT_OK)
        {
            break;
        }

        hwAddr += 16; /* advance to next bank */
    }

    /* for errata FEr#2050 */
    prvCpssDxChRouterTunnelTermTcamErrataTcamEccInterruptLock(devNum,GT_FALSE/*unlock*/,&savedStatus);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* convert data from X/Y format to key/mask */
    for (i = 0 ; i < 8 ; i++)
    {
        lineKey[i]  = PRV_BIT_X_Y_TO_PATTERN_MAC(lineDataX[i],lineDataY[i]);
        lineMask[i] = PRV_BIT_X_Y_TO_MASK_MAC(lineDataX[i],lineDataY[i]);
    }

    /* now extract data */
    for (bank = 0 ; bank < 4 ; bank++)
    {
        switch (bank)
        {
        case 0:
        case 2:
            wordOffset = (bank == 0) ? 0 : 3;

            tcamKeyPtr[wordOffset]    = lineKey[bank*2];
            tcamKeyPtr[wordOffset+1]  = lineKey[bank*2+1] & 0xFFFF;

            tcamMaskPtr[wordOffset]   = lineMask[bank*2];
            tcamMaskPtr[wordOffset+1] = lineMask[bank*2+1] & 0xFFFF;
            break;

        case 1:
        case 3:
            wordOffset = (bank == 1) ? 1 : 4;

            tcamKeyPtr[wordOffset]    |= ((lineKey[bank*2] & 0xFFFF) << 16);
            tcamKeyPtr[wordOffset+1]   = (lineKey[bank*2] >> 16) | ((lineKey[bank*2+1] & 0xFFFF) << 16);

            tcamMaskPtr[wordOffset]   |= ((lineMask[bank*2] & 0xFFFF) << 16);
            tcamMaskPtr[wordOffset+1]  = (lineMask[bank*2] >> 16) | ((lineMask[bank*2+1] & 0xFFFF) << 16);
            break;
        }

        /* get valid bit */
        if (((lineKey[bank*2+1] >> 16) & 0x1) == 1)
            validPtr[bank] = GT_TRUE;
        else
            validPtr[bank] = GT_FALSE;

        /* get compare mode */
        if (((lineKey[bank*2+1] >> 18) & 0x1) == 1)
            compareModePtr[bank] = 1;
        else
            compareModePtr[bank] = 0;
    }

    /* now read router / tunnel termination TCAM action */
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                            portGroupId,
                                            CPSS_DXCH3_LTT_TT_ACTION_E,
                                            lineIndex,
                                            tcamActionPtr);


    return rc;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamGetLine function
* @endinternal
*
* @brief   Reads a tunnel termination entry from the router / tunnel termination
*         TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to read
*
* @param[out] validPtr                 - points to whether TCAM entry is valid or not
* @param[out] compareModePtr           - points to compare mode of the TCAM entry:
*                                      0: single array compare mode
*                                      1: row compare mode
* @param[out] tcamKeyPtr               - points to TCAM key read - hw format
* @param[out] tcamMaskPtr              - points to TCAM mask read - hw format
* @param[out] tcamActionPtr            - points to TCAM action (rule) read - hw format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamGetLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    OUT GT_U32          *validPtr,
    OUT GT_U32          *compareModePtr,
    OUT GT_U32          *tcamKeyPtr,
    OUT GT_U32          *tcamMaskPtr,
    OUT GT_U32          *tcamActionPtr
)
{
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        return prvCpssDxChXcatRouterTunnelTermTcamGetLine(devNum,
                                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          lineIndex,
                                                          validPtr,
                                                          compareModePtr,
                                                          tcamKeyPtr,
                                                          tcamMaskPtr,
                                                          tcamActionPtr);
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        return prvCpssDxChXcatRouterTunnelTermTcamGetLine(devNum,
                                                          portGroupId,
                                                          lineIndex,
                                                          validPtr,
                                                          compareModePtr,
                                                          tcamKeyPtr,
                                                          tcamMaskPtr,
                                                          tcamActionPtr);
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChXcatRouterTunnelTermTcamInvalidateLine function
* @endinternal
*
* @brief   Invalidate a single line in the router / tunnel termination TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamInvalidateLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);

    /* to invalidate the line, all needed is to invalidate the entry on bank 0 */
    return prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry(
        devNum,portGroupId,lineIndex,0);
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamInvalidateLine function
* @endinternal
*
* @brief   Invalidate a single line in the router / tunnel termination TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamInvalidateLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex
)
{
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        return prvCpssDxChXcatRouterTunnelTermTcamInvalidateLine(devNum,
                                                                 CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                 lineIndex);
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        return prvCpssDxChXcatRouterTunnelTermTcamInvalidateLine(devNum,
                                                                 portGroupId,
                                                                 lineIndex);
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry function
* @endinternal
*
* @brief   Invalidate a single entry in the router / tunnel termination TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to (APPLICABLE RANGES: 0..3K-1)
* @param[in] columnIndex              - column index in TCAM line to write to (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex
)
{
    GT_U32  hwAddr;             /* hardware address             */
    GT_U32  rc;                 /* function return code         */
    GT_U32  entryIndex;         /* index of the entry to write  */
    GT_U32  entryKey[2];        /* entry to write (key)         */
    GT_U32  entryMask[2];       /* entry to write (mask)        */
    GT_U32  entryDataX[2];      /* entry to write (dataX)       */
    GT_U32  entryDataY[2];      /* entry to write (dataY)       */
    GT_U32  i;

    GT_U32      tempKeyArr[6];          /* TCAM key in hw format            */
    GT_U32      tempMaskArr[6];         /* TCAM mask in hw format           */
    /* TCAM action (rule) in hw format  */
    GT_U32      tempActionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32      dummyValidArr[5];       /* TCAM line valid bits             */
    GT_U32      dummyCompareModeArr[5]; /* TCAM line compare mode           */
    GT_BOOL enable = GT_FALSE;  /* current state of daemon      */
    GT_U32  xDataArr[8];        /* buffer accumulator for X     */
    GT_U32  yDataArr[8];        /* buffer accumulator for Y     */
    GT_U32  xParity = 0;        /* calculated parity X          */
    GT_U32  yParity = 0;        /* calculated parity Y          */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_COLUMN_INDEX_MAC(columnIndex);

    cpssOsMemSet(&xDataArr , 0, sizeof(xDataArr));
    cpssOsMemSet(&yDataArr , 0, sizeof(yDataArr));

    /* Disable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        rc = prvCpssDxChRouterTcamDaemonDisable(devNum, portGroupId, lineIndex, columnIndex, xDataArr, yDataArr, &xParity, &yParity, &enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* In direct PCL TCAM writing, there must be a burst of two words
       (48 bits of data + 4 control bits). The actual write of the two words
       is executed by the device only when writing to the second word (the
       one with the control nibble).
       When invalidating an entry make sure not to overwrite the hw data
       (which is set together with the control bits).
       This is done by reading the needed data from the hw using
       prvCpssDxChRouterTunnelTermTcamGetLine and updating the data to be set. */

    /* read the current line from TCAM */
    rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                portGroupId,
                                                lineIndex,
                                                dummyValidArr,
                                                dummyCompareModeArr,
                                                tempKeyArr,
                                                tempMaskArr,
                                                tempActionArr);
    if (rc != GT_OK)
        return rc;

    /* To invalidate entry, all that is needed is to set valid bit in DataY
       to 1. That will make sure that the valid bit will not be matched.
       Note that write access to the data word must be followed by a write
       access to the control word. The write access to the control word
       triggers the actual write to both data word and control word. Therefore
       whole DataY will be written. */

    /* prepare invalid entry */
    entryKey[0] = tempKeyArr[0];
    entryKey[1] = tempKeyArr[1] & ~(0x3<<16); /* valid bit (bit 16) is set to 0,
                                                 spare bit (bit 17) must be 0 */
    entryKey[1] &= ~(1<<18);                     /* line compare mode must be 0,
                                                 other parameters doesn't matter */

    entryMask[0] = tempMaskArr[0];
    entryMask[1] = tempMaskArr[1] | (0xF << 16);   /* control bit must not be masked */

    /* convert entry to X/Y format */
    for (i = 0 ; i < 2 ; i++)
    {
        entryDataX[i] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[i], entryMask[i]);
        entryDataY[i] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[i], entryMask[i]);

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
        {
            xDataArr[columnIndex*2 + i] = entryDataX[i];
            yDataArr[columnIndex*2 + i] = entryDataY[i];
        }
    }

    entryIndex = lineIndex * 4 + columnIndex;
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] + (entryIndex * 0x10);

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr,2,entryDataX);
        if (rc != GT_OK)
            return rc;

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr+8,2,entryDataY);
    if (rc != GT_OK)
        return rc;

    /* Enable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        rc = prvCpssDxChRouterTcamDaemonRestore(devNum, portGroupId, lineIndex, xDataArr, yDataArr, xParity, yParity, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamInvalidateEntry function
* @endinternal
*
* @brief   Invalidate a single entry in the router / tunnel termination TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to
* @param[in] columnIndex              - column index in TCAM line to write to
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamInvalidateEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex
)
{
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        return prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry(devNum,
                                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                  lineIndex,
                                                                  columnIndex);
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        return prvCpssDxChXcatRouterTunnelTermTcamInvalidateEntry(devNum,
                                                                  portGroupId,
                                                                  lineIndex,
                                                                  columnIndex);
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteLine function
* @endinternal
*
* @brief   Write the TCAM mask and key. writing operation is preformed on all
*         192 bits in the line.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 192 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 192 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{
    GT_U32  hwAddr;             /* hardware address             */
    GT_U32  rc;                 /* function return code         */
    GT_U32  entryKey[2];        /* entry to write (key)         */
    GT_U32  entryMask[2];       /* entry to write (mask)        */
    GT_U32  entryDataX[2];      /* entry to write (dataX)       */
    GT_U32  entryDataY[2];      /* entry to write (dataY)       */
    GT_U32  entryIndex;         /* index of the entry to write  */
    GT_U32  wordOffset;         /* offset in key/mask arrays    */
    GT_32   bank;               /* current bank written         */
    GT_U32  i;                  /* loop iterator                */
    GT_BOOL enable = GT_FALSE;  /* current state of daemon      */
    GT_U32  xDataArr[8];        /* buffer accumulator for X     */
    GT_U32  yDataArr[8];        /* buffer accumulator for Y     */
    GT_U32  xParity;            /* calculated parity X          */
    GT_U32  yParity;            /* calculated parity Y          */
    GT_U32  regAddr;            /* register address             */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    cpssOsMemSet(&xDataArr , 0, sizeof(xDataArr));
    cpssOsMemSet(&yDataArr , 0, sizeof(yDataArr));

    /* Disable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        /* store current state */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, &enable);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* disable daemon */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Write the banks in the line. Write in reverse order so line valid bit
       will be written last */

    for (bank = 3 ; bank >= 0 && bank <= 3 ; bank--)
    {
        /* get bank data to set */
        switch (bank)
        {
        case 0:
        case 2:
            wordOffset = (bank == 0) ? 0 : 3;

            entryKey[0] = tcamKeyPtr[wordOffset];
            entryKey[1] = (tcamKeyPtr[wordOffset + 1] & 0xFFFF) |
                          (1 << 16) | /* entry is valid      */
                          (1 << 18);  /* line compare mode   */

            /* spare bit (17) must be 0 */
            entryKey[1]&=~(0x1<<17);

            entryMask[0] = tcamMaskPtr[wordOffset];
            entryMask[1] = (tcamMaskPtr[wordOffset + 1] & 0xFFFF) |
                           (0xF << 16); /* control bits can't be masked */
            break;

        case 1:
        case 3:
            wordOffset = (bank == 1) ? 1 : 4;

            entryKey[0] = (tcamKeyPtr[wordOffset] >> 16) |
                          ((tcamKeyPtr[wordOffset + 1] & 0xFFFF) << 16) ;
            entryKey[1] = (tcamKeyPtr[wordOffset + 1] >> 16) |
                          (1 << 16) | /* entry is valid      */
                          (1 << 18);  /* line compare mode   */

            /* spare bit (17) must be 0 */
            entryKey[1]&=~(0x1<<17);

            entryMask[0] = (tcamMaskPtr[wordOffset] >> 16) |
                           ((tcamMaskPtr[wordOffset + 1] & 0xFFFF) << 16) ;
            entryMask[1] = (tcamMaskPtr[wordOffset + 1] >> 16) |
                           (0xF << 16); /* control bits can't be masked */

            break;
        }

        /* convert entry to X/Y format */
        for (i = 0 ; i < 2 ; i++)
        {
            entryDataX[i] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[i], entryMask[i]);
            entryDataY[i] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[i], entryMask[i]);

            if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
            {
                /* store for parity calc */
                xDataArr[bank*2 + i] = entryDataX[i];
                yDataArr[bank*2 + i] = entryDataY[i];
            }
        }

        /* Write dataX/dataY to the ram using direct write. Note that write
           access to the data word must be followed by a write access to the
           control word. The write access to the control word triggers the actual
           write to both data word and control word. */

        entryIndex = lineIndex * 4 + bank;
        hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] + (entryIndex * 0x10);

        rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr,2,entryDataX);
        if (rc != GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr+8,2,entryDataY);
        if (rc != GT_OK)
            return rc;
    }


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        prvCpssDxChLion2RouterTcamParityCalc(xDataArr, &xParity);

        prvCpssDxChLion2RouterTcamParityCalc(yDataArr, &yParity);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadPatternParityBase;

        /* Write Parity_X table */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, (regAddr + (lineIndex * 4)), xParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadMaskParityBase;

        /* Write Parity_Y table */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, (regAddr + (lineIndex * 4)), yParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Restore Previous State */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamKeyMaskWriteModifiedLine function
* @endinternal
*
* @brief   Write the TCAM mask and key. writing operation is preformed on all
*         192 bits in the line.
*         Supports SrcPort field modification for multi port group devices.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 192 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 192 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamKeyMaskWriteModifiedLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{

    typedef struct
    {
        GT_U32  pattern;   /* Rule's pattern */
        GT_U32  mask;      /* Rule's mask */
    }prvRulePatternMask_STC;

    prvRulePatternMask_STC srcPortTrunk        = {0,0};    /* src port or Source trunk-ID   */
    prvRulePatternMask_STC srcIsTrunk          = {0,0};    /* whether src is trunk          */
    prvRulePatternMask_STC dsaSrcDevice        = {0,0};    /* DSA tag source device         */
    prvRulePatternMask_STC dsaSrcIsTrunk       = {0,0};    /* whether DSA tag src is trunk  */
    prvRulePatternMask_STC dsaSrcPortTrunk     = {0,0};    /* DSA tag source port or trunk  */
    prvRulePatternMask_STC portGroupIdData     = {0,0};    /* port group Id Data            */
    prvRulePatternMask_STC dsaPortGroupIdData  = {0,0};    /* DSA port group id Data        */
    GT_STATUS rc = GT_FALSE;                               /* return code                   */
    GT_U32 tcamKey[TTI_RULE_SIZE_CNS];                     /* save value of tcam key        */
    GT_U32 tcamMask[TTI_RULE_SIZE_CNS];                    /* save value of tcam mask       */
    GT_BOOL multiPortGroupModify = GT_FALSE;               /* modify rule for different port groups     */
    GT_U32 portGroupIdMask = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum) << 4;

    cpssOsMemCpy(tcamKey,tcamKeyPtr,sizeof(GT_U32)*TTI_RULE_SIZE_CNS);
    cpssOsMemCpy(tcamMask,tcamMaskPtr,sizeof(GT_U32)*TTI_RULE_SIZE_CNS);

    /* SrcPort field modification for multi port group devices is supported only
       for TT entry. tcamKeyPtr[5] bit 31 must be 0 (to indicate IPv6 entry and not TT entry)*/
    if (((tcamKey[5] >> 31) & 0x1) == 0) /* IPv6 entry */
    {
         rc = prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteLine
             (devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,lineIndex,tcamKey,tcamMask);
         return rc;
    }

    if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        srcPortTrunk.mask        = ((tcamMask[0] >> 10) & PORT_TRUNK_MASK_CNS);
        srcPortTrunk.pattern     = ((tcamKey[0]  >> 10) & PORT_TRUNK_MASK_CNS);
        srcIsTrunk.mask          = ((tcamMask[0] >> 18) & 0x1);
        srcIsTrunk.pattern       = ((tcamKey[0]  >> 18) & 0x1);
        dsaSrcDevice.mask        = ((tcamMask[2] >> 16) & DEV_NUM_MASK_CNS);
        dsaSrcDevice.pattern     = ((tcamKey[2]  >> 16) & DEV_NUM_MASK_CNS);
        dsaSrcPortTrunk.mask     = ((tcamMask[5] >>  0) & DSA_PORT_TRUNK_MASK_CNS);
        dsaSrcPortTrunk.pattern  = ((tcamKey[5]  >>  0) & DSA_PORT_TRUNK_MASK_CNS);
        dsaSrcIsTrunk.mask       = ((tcamMask[5] >>  7) & 0x1);
        dsaSrcIsTrunk.pattern    = ((tcamKey[5]  >>  7) & 0x1);


        /**********************************************************************/
        /*  Case: <dsaSrcDevice> is exact match and equals "own device".      */
        /**********************************************************************/

        /* mask on <dsaSrcDevice> = "exact match" and pattern of <dsaSrcDevice> = "own device" */
        if (((dsaSrcDevice.mask & DEV_NUM_MASK_CNS) == DEV_NUM_MASK_CNS) &&
            ((dsaSrcDevice.pattern & DEV_NUM_MASK_CNS) == PRV_CPSS_HW_DEV_NUM_MAC(devNum)))
        {

            /**********************************************************************/
            /*  Case: <dsaSrcIsTrunk> is don't care and                           */
            /*        DSA port group bits in global port are partial/exact match. */
            /*        The case can't be supported:                                */
            /*        - if dsa source is trunk it must be written to all cores,   */
            /*        - if dsa source is port it must be written to spesific cores*/
            /**********************************************************************/

            /* mask of <dsaSrcIsTrunk> = "don't care" (trunk or port) and */
            /* bits 4,5 in mask of <dsaSrcPortTrunk> != "don't care" (trunk or port) */
            if ((dsaSrcIsTrunk.mask == 0) && ((dsaSrcPortTrunk.mask & portGroupIdMask) != 0))
            {
                /* CPSS can't support this option */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /**********************************************************************/
        /*  Case: <srcIsTrunk> is don't care and                              */
        /*        port group bits in global port are partial or exact match.  */
        /*        The case can't be supported:                                */
        /*        - if source is trunk it must be written to all cores,       */
        /*        - if source is port it must be written to specific cores.   */
        /**********************************************************************/

        /* mask of <srcIsTrunk> = "don't care" (trunk or port) and */
        /* bits 4,5 in mask of <srcPortTrunk> != "don't care" (trunk or port) */
        if ((srcIsTrunk.mask == 0) && ((srcPortTrunk.mask & portGroupIdMask) != 0))
        {
            /* CPSS can't support this option */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /**********************************************************************/
        /*  Case: port group bits in global port are don't care.              */
        /*        The same global port should be written to all cores.        */
        /**********************************************************************/

        /* bits 4,5 in mask of <srcPortTrunk> = "don't care" (trunk or port) */
        if ((srcPortTrunk.mask & portGroupIdMask) == 0)
        {
            /* write <srcPortTrunk> pattern, mask to all cores */
            multiPortGroupModify = GT_TRUE;
        }


        /**********************************************************************/
        /*  Case: <srcIsTrunk> is trunk.                                      */
        /*        The same global port should be written to all cores.        */
        /**********************************************************************/

        /* mask of <srcIsTrunk> = "exact match" and pattern <srcIsTrunk> = 1 (trunk)*/
        if ((srcIsTrunk.mask == 1) && ((srcIsTrunk.pattern & 0x1) == 1))
        {
            /* write <srcPortTrunk> pattern, mask to all cores */
            multiPortGroupModify = GT_TRUE;
        }

        /**********************************************************************/
        /*  Case: <srcIsTrunk> is port.                                       */
        /*        If port group bits in global port >= 60: CPU port/NULL port,*/
        /*        the same global port should be written to all cores.        */
        /**********************************************************************/
        /* mask of <srcIsTrunk> = "exact match" and pattern <srcIsTrunk> = 0 (port)*/
        if ((srcIsTrunk.mask == 1) && ((srcIsTrunk.pattern & 0x1) == 0))
        {
            /* 4 MSB of pattern <srcPortTrunk> are set */
            if ((srcPortTrunk.pattern & 0x7C) == 0x3C)
            {
                /* write <srcPortTrunk> pattern, mask to all cores */
                multiPortGroupModify = GT_TRUE;
            }
        }
    }

    /* Multi port group modifying write */

    portGroupIdData.mask        = PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devNum,srcPortTrunk.mask);
    portGroupIdData.pattern     = PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devNum,srcPortTrunk.pattern);
    dsaPortGroupIdData.mask     = PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devNum,dsaSrcPortTrunk.mask);
    dsaPortGroupIdData.pattern  = PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devNum,dsaSrcPortTrunk.pattern);


    /************* <dsaSrcPortTrunk> and <dsaSrcIsTrunk> ******************/

    /* clear dsaSrcPortTrunk and dsaSrcIsTrunk bits in mask and pattern */
    tcamMask[5] &= (~ (DSA_PORT_TRUNK_MASK_CNS << 0)); /* dsaSrcPortTrunk */
    tcamKey[5]  &= (~ (DSA_PORT_TRUNK_MASK_CNS << 0)); /* dsaSrcPortTrunk */
    tcamMask[5] &= (~ (0x1 << 7)); /* dsaSrcIsTrunk   */
    tcamKey[5]  &= (~ (0x1 << 7)); /* dsaSrcIsTrunk   */

    /* put dsaSrcIsTrunk mask and pattern */
    tcamMask[5] |= (dsaSrcIsTrunk.mask << 7);      /* dsaSrcIsTrunk */
    tcamKey[5]  |= (dsaSrcIsTrunk.pattern << 7);   /* dsaSrcIsTrunk */

    /* apply change to all cores */
    if ((dsaPortGroupIdData.mask & portGroupId) == dsaPortGroupIdData.pattern)
    {
        /* own device */
        if (((dsaSrcDevice.mask & DEV_NUM_MASK_CNS) == DEV_NUM_MASK_CNS) &&
            ((dsaSrcDevice.pattern & DEV_NUM_MASK_CNS) == PRV_CPSS_HW_DEV_NUM_MAC(devNum)))
        {
            /* truncate pattern to port-group-local port value */
            dsaSrcPortTrunk.pattern &= 0x0F;
        }
    }
    /* put dsaSrcPortTrunk mask and pattern */
    tcamMask[5] |= (dsaSrcPortTrunk.mask << 0);    /* dsaSrcPortTrunk */
    tcamKey[5]  |= (dsaSrcPortTrunk.pattern << 0); /* dsaSrcPortTrunk */

    /************* <srcPortTrunk> and <srcIsTrunk> ******************/

    /* clear srcPortTrunk and srcIsTrunk bits in mask and pattern */
    tcamMask[0] &= (~ (PORT_TRUNK_MASK_CNS << 10)); /* srcPortTrunk */
    tcamKey[0]  &= (~ (PORT_TRUNK_MASK_CNS << 10)); /* srcPortTrunk */
    tcamMask[0] &= (~ (0x1 << 18));  /* srcIsTrunk */
    tcamKey[0]  &= (~ (0x1 << 18));  /* srcIsTrunk */

    /* put srcIsTrunk mask and pattern */
    tcamMask[0] |= (srcIsTrunk.mask << 18);      /* srcIsTrunk */
    tcamKey[0]  |= (srcIsTrunk.pattern << 18);   /* srcIsTrunk */

    if (multiPortGroupModify) /* apply change to all cores */
    {
        /* put srcPortTrunk mask and pattern */
        tcamMask[0] |= (srcPortTrunk.mask << 10);
        tcamKey[0]  |= (srcPortTrunk.pattern << 10);
    }
    else /* apply change to specific core */
    {
        /* truncate pattern to port-group-local port value */
        srcPortTrunk.pattern    &= 0x0F;

        if ((portGroupIdData.mask & portGroupId) == portGroupIdData.pattern)
        {
            /* put srcPortTrunk mask and pattern */
            tcamMask[0] |= (srcPortTrunk.mask << 10);
            tcamKey[0]  |= (srcPortTrunk.pattern << 10);
        }
        else
        {
            /* require srcPort to be NULL port */
            tcamMask[0] |= (PORT_TRUNK_MASK_CNS << 10);
            tcamKey[0]  |= (PRV_CPSS_DXCH_NULL_PORT_NUM_CNS << 10);
        }
    }

    rc = prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteLine(
            devNum,portGroupId,lineIndex,tcamKey,tcamMask);

    return rc;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine function
* @endinternal
*
* @brief   Write the TCAM mask and key. writing operation is preformed on all
*         192 bits in the line.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 192 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 192 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {

    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        return prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteLine(devNum,
                                                                   CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                   lineIndex,
                                                                   tcamKeyPtr,
                                                                   tcamMaskPtr);
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        return prvCpssDxChRouterTunnelTermTcamKeyMaskWriteModifiedLine(devNum,
                                                                       portGroupId,
                                                                       lineIndex,
                                                                       tcamKeyPtr,
                                                                       tcamMaskPtr);
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }
}

/**
* @internal prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteEntry function
* @endinternal
*
* @brief   Write the TCAM mask and key.
*         Writing operation is preformed on one column entry (out of 4).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] columnIndex              - Column index in the TCAM line (0 to 3).
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 192 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 192 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function does not guarantee atomic write. It does make sure to
*       set the valid bit after all entry is valid, meaning that if the entry
*       was invalid before, the write is atomic.
*
*/
GT_STATUS prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{
    GT_U32  hwAddr;             /* hardware address             */
    GT_U32  rc;                 /* function return code         */
    GT_U32  entryKey[2];        /* entry to write (key)         */
    GT_U32  entryMask[2];       /* entry to write (mask)        */
    GT_U32  entryDataX[2];      /* entry to write (dataX)       */
    GT_U32  entryDataY[2];      /* entry to write (dataY)       */
    GT_U32  entryIndex;         /* index of the entry to write  */
    GT_U32  wordOffset;         /* offset in key/mask arrays    */
    GT_U32  i;                  /* loop iterator                */
    GT_BOOL enable = GT_FALSE;  /* current state of daemon      */
    GT_U32  xDataArr[8];        /* buffer accumulator for X     */
    GT_U32  yDataArr[8];        /* buffer accumulator for Y     */
    GT_U32  xParity = 0;        /* calculated parity X          */
    GT_U32  yParity = 0;        /* calculated parity Y          */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_COLUMN_INDEX_MAC(columnIndex);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    /* initialize entryKey,entryMask,entryDataX and entryDataY */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));
    cpssOsMemSet(&xDataArr , 0, sizeof(xDataArr));
    cpssOsMemSet(&yDataArr , 0, sizeof(yDataArr));

    /* Disable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        rc = prvCpssDxChRouterTcamDaemonDisable(devNum, portGroupId, lineIndex, columnIndex, xDataArr, yDataArr, &xParity, &yParity, &enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }


    /* get entry to be set from key/mask arrays */
    switch (columnIndex)
    {
    case 0:
    case 2:
        wordOffset = (columnIndex == 0) ? 0 : 3;

        entryKey[0] = tcamKeyPtr[wordOffset];
        entryKey[1] = (tcamKeyPtr[wordOffset + 1] & 0xFFFF) |
                      (1 << 16) ; /* entry is valid      */

        /* spare bit (17) and entry compare mode (18)  must be 0 */
        entryKey[1]&=~(0x3<<17);

        entryMask[0] = tcamMaskPtr[wordOffset];
        entryMask[1] = (tcamMaskPtr[wordOffset + 1] & 0xFFFF) |
                       (0xF << 16); /* control bits can't be masked */
        break;

    case 1:
    case 3:
        wordOffset = (columnIndex == 1) ? 1 : 4;

        entryKey[0] = (tcamKeyPtr[wordOffset] >> 16) |
                      ((tcamKeyPtr[wordOffset + 1] & 0xFFFF) << 16) ;
        entryKey[1] = (tcamKeyPtr[wordOffset + 1] >> 16) |
                      (1 << 16); /* entry is valid      */

        /* spare bit (17) and entry compare mode (18)  must be 0 */
        entryKey[1]&=~(0x3<<17);

        entryMask[0] = (tcamMaskPtr[wordOffset] >> 16) |
                       ((tcamMaskPtr[wordOffset + 1] & 0xFFFF) << 16) ;
        entryMask[1] = (tcamMaskPtr[wordOffset + 1] >> 16) |
                       (0xF << 16); /* control bits can't be masked */

        break;
    }

    /* convert entry to X/Y format */
    for (i = 0 ; i < 2 ; i++)
    {
        entryDataX[i] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[i], entryMask[i]);
        entryDataY[i] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[i], entryMask[i]);

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
        {
            xDataArr[columnIndex*2 + i] = entryDataX[i];
            yDataArr[columnIndex*2 + i] = entryDataY[i];
        }
    }

    /* Write dataX/dataY to the ram using direct write. Note that write
       access to the data word must be followed by a write access to the
       control word. The write access to the control word triggers the actual
       write to both data word and control word. Note that data Y is written
       last to ensure setting valid bit after all entry data is set. */

    entryIndex = lineIndex * 4 + columnIndex;
    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] + (entryIndex * 0x10);

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr,2,entryDataX);
    if (rc != GT_OK)
        return rc;

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr+8,2,entryDataY);
    if (rc != GT_OK)
        return rc;


    /* Enable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        rc = prvCpssDxChRouterTcamDaemonRestore(devNum, portGroupId, lineIndex, xDataArr, yDataArr, xParity, yParity, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChSip5RouterTunnelTermTcamKeyMaskWriteEntry function
* @endinternal
*
* @brief   Write the TCAM mask and key.
*         Writing operation is preformed on one column entry (out of 5).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] columnIndex              - Column index in the TCAM line (0 to 4).
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 240 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 240 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function does not guarantee atomic write. It does make sure to
*       set the valid bit after all entry is valid, meaning that if the entry
*       was invalid before, the write is atomic.
*
*/
GT_STATUS prvCpssDxChSip5RouterTunnelTermTcamKeyMaskWriteEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{
    GT_U32  hwAddr;             /* hardware address             */
    GT_U32  rc;                 /* function return code         */
    GT_U32  entryKey[2];        /* entry to write (key)         */
    GT_U32  entryMask[2];       /* entry to write (mask)        */
    GT_U32  entryDataX[2];      /* entry to write (dataX)       */
    GT_U32  entryDataY[2];      /* entry to write (dataY)       */
    GT_U32  wordOffset;         /* offset in key/mask arrays    */
    GT_U32  i;
    GT_U32  numOfBanks = PRV_NUM_BANKS_5_CNS;    /* number of banks */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_LINE_INDEX_MAC(devNum,lineIndex);
    PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_CHECK_COLUMN_INDEX_MAC(columnIndex);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    /* initialize entryKey,entryMask,entryDataX and entryDataY */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));


    /* get entry to be set from key/mask arrays */
    switch (columnIndex)
    {
    case 0:
    case 2:
    case 4:
        if (columnIndex == 0)
            wordOffset = 0;
        else if (columnIndex == 2)
            wordOffset = 3;
        else
            wordOffset = 6;

        entryKey[0] = tcamKeyPtr[wordOffset];
        entryKey[1] = (tcamKeyPtr[wordOffset + 1] & 0xFFFF) |
                      (1 << 16);  /* entry is valid      */

        /* spare bit (17) and entry compare mode (18)  must be 0 */
        entryKey[1]&=~(0x3<<17);

        entryMask[0] = tcamMaskPtr[wordOffset];
        entryMask[1] = (tcamMaskPtr[wordOffset + 1] & 0xFFFF) |
                       (0xF << 16); /* control bits can't be masked */
        break;

    case 1:
    case 3:
        wordOffset = (columnIndex == 1) ? 1 : 4;

        entryKey[0] = (tcamKeyPtr[wordOffset] >> 16) |
                      ((tcamKeyPtr[wordOffset + 1] & 0xFFFF) << 16) ;
        entryKey[1] = (tcamKeyPtr[wordOffset + 1] >> 16) |
                      (1 << 16) ; /* entry is valid      */

        /* spare bit (17) and entry compare mode (18)  must be 0 */
        entryKey[1]&=~(0x3<<17);

        entryMask[0] = (tcamMaskPtr[wordOffset] >> 16) |
                       ((tcamMaskPtr[wordOffset + 1] & 0xFFFF) << 16) ;
        entryMask[1] = (tcamMaskPtr[wordOffset + 1] >> 16) |
                       (0xF << 16); /* control bits can't be masked */

        break;
    }

    /* convert entry to X/Y format */
    for (i = 0 ; i < 2 ; i++)
    {
        entryDataX[i] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[i], entryMask[i]);
        entryDataY[i] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[i], entryMask[i]);
    }

    /* Write dataX/dataY to the ram using direct write. Note that write
       access to the data word must be followed by a write access to the
       control word. The write access to the control word triggers the actual
       write to both data word and control word. Note that data Y is written
       last to ensure setting valid bit after all entry data is set. */

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerTtTcamBankAddr[0] +
             PRV_TCAM_LINE_OFFSET_MAC(lineIndex,numOfBanks) + /*offset to start of line*/
             columnIndex * PRV_TCAM_BANK_OFFSET_CNS;/*offset to start of bank*/

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr,2,entryDataX);
    if (rc != GT_OK)
        return rc;

    rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId,hwAddr+8,2,entryDataY);
    if (rc != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamKeyMaskWriteEntry function
* @endinternal
*
* @brief   Write the TCAM mask and key.
*         Writing operation is preformed on one column entry (out of 6).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lineIndex                - line index in TCAM to write to.
* @param[in] columnIndex              - Column index in the TCAM line (0 to 5).
* @param[in] tcamKeyPtr               - TCAM key to write - hw format 192 bit.
* @param[in] tcamMaskPtr              - TCAM mask to write - hw format 192 bit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamKeyMaskWriteEntry
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          lineIndex,
    IN  GT_U32          columnIndex,
    IN  GT_U32          *tcamKeyPtr,
    IN  GT_U32          *tcamMaskPtr
)
{
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return prvCpssDxChSip5RouterTunnelTermTcamKeyMaskWriteEntry(devNum,
                                                                     portGroupId,
                                                                     lineIndex,
                                                                     columnIndex,
                                                                     tcamKeyPtr,
                                                                     tcamMaskPtr);
    }

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        return prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteEntry(devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    lineIndex,
                                                                    columnIndex,
                                                                    tcamKeyPtr,
                                                                    tcamMaskPtr);
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        return prvCpssDxChXcatRouterTunnelTermTcamKeyMaskWriteEntry(devNum,
                                                                    portGroupId,
                                                                    lineIndex,
                                                                    columnIndex,
                                                                    tcamKeyPtr,
                                                                    tcamMaskPtr);
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamErrataTcamEccInterruptLock function
* @endinternal
*
* @brief   Locks/unlocks the task switching and disables/enables the TCAM TCC interrupt
*
* @param[in] devNum                   - device number
* @param[in] lock                     - GT_TRUE - to  task switching and to disable interrupt
*                                      - GT_FALSE - to enable interrupt and to unlock task switching
* @param[in,out] savedStatePtr            - for "lock" - nothing, for "unlock" - saved state
* @param[in,out] savedStatePtr            - for "lock" - place to save state, for "unlock" - nothing
*
* @retval GT_OK                    - success
* @retval GT_NOT_INITIALIZED       - the internal DB for the 'WA' was not initialized
*/
static GT_STATUS prvCpssDxChRouterTunnelTermTcamErrataTcamEccInterruptLock
(
    IN    GT_U8                             devNum,
    IN    GT_BOOL                           lock,
    INOUT GT_U32                            *savedStatePtr
)
{
    GT_BOOL eccIntEn;    /* TCAM TCC interrupt enable */
    GT_U32  data;        /* work data                 */
    GT_STATUS rc;        /* return code               */

    if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
        devNum, PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E))
    {
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.routerInterruptCauseRegAddr ==
        0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lock == GT_FALSE)
    {
        /* UNLOCK case */

        eccIntEn = (GT_BOOL)*savedStatePtr;

        /* ECC - interrupt enable */

        if (GT_FALSE != eccIntEn)
        {
            /* clear Policy Engine Interrupt Cause register by read */
            rc = prvCpssDrvHwPpReadRegister(devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.routerInterruptCauseRegAddr,
                &data);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* unmask - i.e. event enable */
            prvCpssDrvEventsMask(devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.routerInterruptIndex,
                GT_FALSE);
        }

        PRV_CPSS_UNLOCK_BY_MUTEX;
    }
    else
    {
        /* LOCK case */

        PRV_CPSS_LOCK_BY_MUTEX;

        prvCpssDrvEventsMaskGet(devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.routerInterruptIndex,
                &eccIntEn);

        if (GT_FALSE != eccIntEn)
        {
            /* ECC - interrupt disable */
            /* mask - i.e. event disable */
            prvCpssDrvEventsMask(devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.routerInterruptIndex,
                GT_TRUE);
        }

        *savedStatePtr = (GT_U32)eccIntEn;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChRouterTunnelTermTcamMultiPortGroupsLineProcess function
* @endinternal
*
* @brief   This function check received tcam line data for given portGroupId
*         and modify srcPortTrunk and dsaSrcPortTrunk if necessary
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id.
* @param[in,out] hwTtiKeyPerPortGroupArr[] - TTI key in hw format read from TCAM.
* @param[in] hwTtiMaskPerPortGroupArr[] - TTI mask in hw format read from TCAM .
* @param[in,out] hwTtiKeyPerPortGroupArr[] - TTI key in hw format read from TCAM.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamMultiPortGroupsLineProcess
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    INOUT  GT_U32       hwTtiKeyPerPortGroupArr[],
    IN  GT_U32          hwTtiMaskPerPortGroupArr[]
)
{
    typedef struct
    {
        GT_U32  pattern;   /* Rule's pattern */
        GT_U32  mask;      /* Rule's mask */
    }prvRulePatternMask_STC;

    prvRulePatternMask_STC srcPortTrunk        = {0,0};    /* src port or Source trunk-ID   */
    prvRulePatternMask_STC srcIsTrunk          = {0,0};    /* whether src is trunk          */
    prvRulePatternMask_STC dsaSrcDevice        = {0,0};    /* DSA tag source device         */
    prvRulePatternMask_STC dsaSrcPortTrunk     = {0,0};    /* DSA tag source port or trunk  */
    prvRulePatternMask_STC sourcePortGroupId  = {0,0};     /* source port group id Data     */
    GT_U32  globalPortId = 0;
    GT_U32 portGroupIdMask = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum) << 4;
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (portGroupId >= CPSS_MAX_PORT_GROUPS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    srcPortTrunk.pattern = (U32_GET_FIELD_MAC(hwTtiKeyPerPortGroupArr[0],10,8));
    srcPortTrunk.mask = (U32_GET_FIELD_MAC(hwTtiMaskPerPortGroupArr[0],10,8));
    if (srcPortTrunk.pattern != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
    {
        /* check if it is port interface */
        srcIsTrunk.mask  = ((hwTtiMaskPerPortGroupArr[0] >> 18) & 0x1);
        srcIsTrunk.pattern  = ((hwTtiKeyPerPortGroupArr[0] >> 18) & 0x1);
        if ((srcIsTrunk.mask == 1) && ((srcIsTrunk.pattern & 0x1) == 0))
        {
            /* port interface */
            if ((srcPortTrunk.pattern & 0x3C)< 0x3C)
            {
                /* check that bits 4,5 in mask of <srcPortTrunk> != "don't care" (trunk or port) */
                if ((srcPortTrunk.mask & portGroupIdMask) != 0)
                {
                    /* srcPortTrunkPattern reprsents local port.
                       It should be converted to global */
                    globalPortId = srcPortTrunk.pattern  | (portGroupId << 4);
                    /* zero port id field */
                    hwTtiKeyPerPortGroupArr[0] &= (~ (PORT_TRUNK_MASK_CNS << 10));

                    /* write globalPortId */
                    hwTtiKeyPerPortGroupArr[0]  |= (globalPortId << 10);
                }
            }

        }
    }
    dsaSrcDevice.mask        = ((hwTtiMaskPerPortGroupArr[2] >> 16) & DEV_NUM_MASK_CNS);
    dsaSrcDevice.pattern     = ((hwTtiKeyPerPortGroupArr[2]  >> 16) & DEV_NUM_MASK_CNS);
    dsaSrcPortTrunk.mask     = ((hwTtiMaskPerPortGroupArr[5] >>  0) & DSA_PORT_TRUNK_MASK_CNS);
    dsaSrcPortTrunk.pattern  = ((hwTtiKeyPerPortGroupArr[5]  >>  0) & DSA_PORT_TRUNK_MASK_CNS);
    sourcePortGroupId.pattern = (U32_GET_FIELD_MAC(hwTtiKeyPerPortGroupArr[5],29,2));
    sourcePortGroupId.mask = (U32_GET_FIELD_MAC(hwTtiMaskPerPortGroupArr[5],29,2));
    /* check that sourcePortGroupId is not don't care */
    if ((sourcePortGroupId.mask & sourcePortGroupId.pattern)  == sourcePortGroupId.pattern)
    {
        if (sourcePortGroupId.pattern == portGroupId)
        {
           /* own device */
            if (((dsaSrcDevice.mask & DEV_NUM_MASK_CNS) == DEV_NUM_MASK_CNS) &&
                ((dsaSrcDevice.pattern & DEV_NUM_MASK_CNS) == devNum))
            {
                if (dsaSrcPortTrunk.mask && 0x30)
                {
                    globalPortId = dsaSrcPortTrunk.pattern  | (portGroupId << 4);
                    /* zero dsaSrcPortTrunk */
                    hwTtiKeyPerPortGroupArr[5] &= 0xFFFFFF80;
                    hwTtiKeyPerPortGroupArr[5]  |= (globalPortId << 0);
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location fore multi-port groups devices.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] routerTtiTcamRow         - Index of the rule in the TCAM
*
* @param[out] validPtr                 - points to whether TCAM entry is valid or not
* @param[out] compareModePtr           - points to compare mode of the TCAM entry:
*                                      0: single array compare mode
*                                      1: row compare mode
* @param[out] tcamKeyPtr               - points to TTI key in hardware format
* @param[out] tcamMaskPtr              - points to TTI mask in hardware format
* @param[out] tcamActionPtr            - points to TTI action in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerTtiTcamRow,
    OUT GT_U32                              *validPtr,
    OUT GT_U32                              *compareModePtr,
    OUT GT_U32                              *tcamKeyPtr,
    OUT GT_U32                              *tcamMaskPtr,
    OUT GT_U32                              *tcamActionPtr
)
{
    /* Per Port Group TTI key in hw format     */
    GT_U32      hwTtiKeyArrayPerPortGroup[CPSS_MAX_PORT_GROUPS_CNS][TTI_RULE_SIZE_CNS];
    /* Per Port Group TTI mask in hw format    */
    GT_U32      hwTtiMaskArrayPerPortGroup[CPSS_MAX_PORT_GROUPS_CNS][TTI_RULE_SIZE_CNS];
    /* port group Id */
    GT_U32      portGroupId;
    /* SrcPortTrunk in all port groups are NULL*/
    GT_BOOL     allNullSrcPortTrunk = GT_TRUE;
    /* return value */
    GT_STATUS rc = GT_OK;
    /* iterators */
    GT_U32    j = 0;

    /* check pararmeters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(compareModePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamActionPtr);
    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,routerTtiTcamRow);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    cpssOsMemSet(hwTtiKeyArrayPerPortGroup,0,sizeof(hwTtiKeyArrayPerPortGroup));
    cpssOsMemSet(hwTtiMaskArrayPerPortGroup,0,sizeof(hwTtiMaskArrayPerPortGroup));

    /* read the pattern & mask from all port groups */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* read tunnel termination configuration and action from hardware */
        rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                    portGroupId,
                                                    routerTtiTcamRow,
                                                    validPtr,
                                                    compareModePtr,
                                                    hwTtiKeyArrayPerPortGroup[portGroupId],
                                                    hwTtiMaskArrayPerPortGroup[portGroupId],
                                                    tcamActionPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* like xcat - the ports are 'global' and not local in the key
                so no 'Lion style' conversion needed */

            /* avoid any conversion outsize this loop */
            allNullSrcPortTrunk = GT_FALSE;

            continue;
        }

        rc = prvCpssDxChRouterTunnelTermTcamMultiPortGroupsLineProcess(devNum,
                                                                       portGroupId,
                                                                       hwTtiKeyArrayPerPortGroup[portGroupId],
                                                                       hwTtiMaskArrayPerPortGroup[portGroupId]);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (U32_GET_FIELD_MAC(hwTtiKeyArrayPerPortGroup[portGroupId][0],10,8) != PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
        {
            allNullSrcPortTrunk = GT_FALSE;
        }
        else
        {
            /* clear srcPortTrunk and srcIsTrunk bits in mask and pattern */
            hwTtiKeyArrayPerPortGroup[portGroupId][0] &= (~ (PORT_TRUNK_MASK_CNS << 10)); /* srcPortTrunk */
            hwTtiMaskArrayPerPortGroup[portGroupId][0]&= (~ (PORT_TRUNK_MASK_CNS << 10)); /* srcPortTrunk */
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
          devNum, portGroupsBmp, portGroupId)

    for (j=0; j<TTI_RULE_SIZE_CNS; j++)
    {
        /* Clear the whole key array in preparation for OR operation */
        tcamKeyPtr[j] = 0;
        tcamMaskPtr[j] = 0;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        if (allNullSrcPortTrunk)
        {
                U32_SET_FIELD_MAC(hwTtiKeyArrayPerPortGroup[portGroupId][0],10,8,PRV_CPSS_DXCH_NULL_PORT_NUM_CNS);
                U32_SET_FIELD_MAC(hwTtiMaskArrayPerPortGroup[portGroupId][0],10,8,PORT_TRUNK_MASK_CNS);
        }

        for (j=0; j<TTI_RULE_SIZE_CNS; j++)
        {
            /* OR the whole key array in order to the port as well as
               the DSA tag and future port group dependant fields*/
            tcamKeyPtr[j] |= hwTtiKeyArrayPerPortGroup[portGroupId][j];
            tcamMaskPtr[j] |= hwTtiMaskArrayPerPortGroup[portGroupId][j];
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
          devNum, portGroupsBmp, portGroupId)
    return rc;
}

