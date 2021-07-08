#include <Copyright.h>

/**
********************************************************************************
* @file gtTCAM.c
*
* @brief API definitions for control of Ternary Content Addressable Memory
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtTCAM.c
*
* DESCRIPTION:
*       API definitions for control of Ternary Content Addressable Memory
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtTCAM.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>

/****************************************************************************/
/* Internal TCAM structure declaration.                                    */
/****************************************************************************/

/**
* @struct GT_CPSS_TCAM_PG_DATA
 *
 * @brief data required by Ingress TCAM entry page
*/
typedef struct{

    GT_U16 frame[26];

} GT_CPSS_TCAM_PG_DATA;

/**
* @struct GT_CPSS_TCAM_DATA_HW
 *
 * @brief data required by Ingress TCAM (Ternary Content Addressable Memory) Operation
*/
typedef struct{

    /** page 0 registers of TCAM */
    GT_CPSS_TCAM_PG_DATA pg0;

    /** page 1 registers of TCAM */
    GT_CPSS_TCAM_PG_DATA pg1;

    /** page 2 registers of TCAM */
    GT_CPSS_TCAM_PG_DATA pg2;

} GT_CPSS_TCAM_DATA_HW;

/**
* @struct GT_CPSS_TCAM_EGR_DATA_HW
 *
 * @brief data required by Egress TCAM Action Page - TCAM entry page 3
*/
typedef struct{

    GT_U16 frame[3];

} GT_CPSS_TCAM_EGR_DATA_HW;

typedef struct
{
    GT_U32    tcamEntry;
    GT_CPSS_TCAM_DATA_HW    tcamDataP;
} GT_CPSS_TCAM_OP_DATA;

typedef struct
{
    GT_U8  port;
    GT_U32      tcamEntry;
    GT_CPSS_TCAM_EGR_DATA_HW    tcamDataP;
} GT_CPSS_TCAM_EGR_OP_DATA;

typedef enum
{
    TCAM_FLUSH_ALL       = 0x1,
    TCAM_FLUSH_ENTRY     = 0x2,
    TCAM_LOAD_ENTRY      = 0x3,
    TCAM_PURGE_ENTRY     = 0x3,
    TCAM_GET_NEXT_ENTRY  = 0x4,
    TCAM_READ_ENTRY      = 0x5
} GT_CPSS_TCAM_OPERATION;

/****************************************************************************/
/* TCAM operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS tcamOperationPerform
(
    IN    GT_CPSS_QD_DEV           *dev,
    IN    GT_CPSS_TCAM_OPERATION   tcamOp,
    INOUT GT_CPSS_TCAM_OP_DATA     *opData
);

static GT_STATUS tcamEgrOperationPerform
(
    IN    GT_CPSS_QD_DEV            *dev,
    IN    GT_CPSS_TCAM_OPERATION    tcamOp,
    INOUT GT_CPSS_TCAM_EGR_OP_DATA  *opData
);
static GT_STATUS setTcamHWData
(
    IN  GT_CPSS_TCAM_DATA     *iData,
    OUT GT_CPSS_TCAM_DATA_HW  *oData
);
static GT_STATUS getTcamHWData
(
    IN  GT_CPSS_TCAM_DATA_HW  *iData,
    OUT GT_CPSS_TCAM_DATA     *oData
);
static GT_STATUS setTcamEgrHWData
(
    IN  GT_CPSS_TCAM_EGR_DATA     *iData,
    OUT GT_CPSS_TCAM_EGR_DATA_HW  *oData
);
static GT_STATUS getTcamEgrHWData
(
    IN  GT_CPSS_TCAM_EGR_DATA_HW  *iData,
    OUT GT_CPSS_TCAM_EGR_DATA     *oData
);

/**
* @internal prvCpssDrvGtcamFlushAll function
* @endinternal
*
* @brief   This routine is to flush all entries. A Flush All command will initialize
*         TCAM Pages 0 and 1, offsets 0x02 to 0x1B to 0x0000, and TCAM Page 2 offset
*         0x02 to 0x1B to 0x0000 for all TCAM entries with the exception that TCAM
*         Page 0 offset 0x02 will be initialized to 0x00FF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGtcamFlushAll
(
    IN  GT_CPSS_QD_DEV     *dev
)
{
    GT_STATUS               retVal;
    GT_CPSS_TCAM_OPERATION  op;
    GT_CPSS_TCAM_OP_DATA    tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamFlushAll Called.\n"));

    /* Program Tuning register */
    op = TCAM_FLUSH_ALL;
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGtcamFlushEntry function
* @endinternal
*
* @brief   This routine is to flush a single entry. A Flush a single TCAM entry command
*         will write the same values to a TCAM entry as a Flush All command, but it is
*         done to the selected single TCAM entry only.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamFlushEntry
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          tcamPointer
)
{
    GT_STATUS               retVal;
    GT_CPSS_TCAM_OPERATION  op;
    GT_CPSS_TCAM_OP_DATA    tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamFlushEntry Called.\n"));

    /* check if the given pointer is valid */
    if (tcamPointer > 0xFF)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_FLUSH_ENTRY;
    tcamOpData.tcamEntry = tcamPointer;
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGtcamLoadEntry function
* @endinternal
*
* @brief   This routine loads a TCAM entry.
*         The load sequence of TCAM entry is critical. Each TCAM entry is made up of
*         3 pages of data. All 3 pages need to loaded in a particular order for the TCAM
*         to operate correctly while frames are flowing through the switch.
*         If the entry is currently valid, it must first be flushed. Then page 2 needs
*         to be loaded first, followed by page 1 and then finally page 0.
*         Each page load requires its own write TCAMOp with these TCAM page bits set
*         accordingly.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
* @param[in] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamLoadEntry
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_U32             tcamPointer,
    IN  GT_CPSS_TCAM_DATA  *tcamData
)
{
    GT_STATUS                retVal;
    GT_CPSS_TCAM_OPERATION   op;
    GT_CPSS_TCAM_OP_DATA     tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamLoadEntry Called.\n"));

    /* check if the given pointer is valid */
    if ((tcamPointer > 0xFF)||(tcamData==NULL))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_LOAD_ENTRY;
    tcamOpData.tcamEntry = tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    setTcamHWData(tcamData,&tcamOpData.tcamDataP);
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGtcamReadTCAMData function
* @endinternal
*
* @brief   This routine reads the global 3 offsets 0x02 to 0x1B registers with
*         the data found in the TCAM entry and its TCAM page pointed to by the TCAM
*         entry and TCAM page bits of this register (bits 6:0 and 11:10 respectively.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamReadTCAMData
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_U32             tcamPointer,
    OUT GT_CPSS_TCAM_DATA  *tcamData
)
{
    GT_STATUS                retVal;
    GT_CPSS_TCAM_OPERATION   op;
    GT_CPSS_TCAM_OP_DATA     tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamReadTCAMData Called.\n"));


    /* check if the given pointer is valid */
    if ((tcamPointer > 0xFF)||(tcamData==NULL))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_READ_ENTRY;
    tcamOpData.tcamEntry = tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    getTcamHWData(&tcamOpData.tcamDataP,tcamData);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGtcamGetNextTCAMData function
* @endinternal
*
* @brief   This routine finds the next higher TCAM Entry number that is valid (i.e.,
*         any entry whose Page 0 offset 0x02 is not equal to 0x00FF). The TCAM Entry
*         register (bits 6:0) is used as the TCAM entry to start from. To find
*         the lowest number TCAM Entry that is valid, start the Get Next operation
*         with TCAM Entry set to 0xFF.
* @param[in,out] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
* @param[in,out] tcamPointer              - next pointer entry of TCAM (0 ~ 255)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamGetNextTCAMData
(
    IN    GT_CPSS_QD_DEV     *dev,
    INOUT GT_U32             *tcamPointer,
    OUT   GT_CPSS_TCAM_DATA  *tcamData
)
{
    GT_STATUS                retVal;
    GT_CPSS_TCAM_OPERATION   op;
    GT_CPSS_TCAM_OP_DATA     tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamGetNextTCAMData Called.\n"));

    /* check if the given pointer is valid */
    if (tcamData==NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_GET_NEXT_ENTRY;
    tcamOpData.tcamEntry = *tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal == GT_NO_SUCH)
    {
        PRV_CPSS_DBG_INFO(("No higher valid entry founded return GT_NO_SUCH"));
        return retVal;
    }
    else if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    *tcamPointer = tcamOpData.tcamEntry;
    getTcamHWData(&tcamOpData.tcamDataP,tcamData);
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGtcamFindEntry function
* @endinternal
*
* @brief   Find the specified valid tcam entry in ingress TCAM Table.
*
* @param[in] tcamPointer              - the tcam entry index to search.
*
* @param[out] found                    - GT_TRUE, if the appropriate entry exists.
* @param[out] tcamData                 - the entry parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error or entry does not exist.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS prvCpssDrvGtcamFindEntry
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_U32             tcamPointer,
    OUT GT_CPSS_TCAM_DATA  *tcamData,
    OUT GT_BOOL            *found
)
{
    GT_STATUS               retVal;
    GT_CPSS_TCAM_OPERATION  op;
    GT_CPSS_TCAM_OP_DATA    tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamFindEntry Called.\n"));

    /* check if the given pointer is valid */
    if (tcamData==NULL || tcamPointer>0xFF)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *found = GT_FALSE;

    /* Program Tuning register */
    op = TCAM_GET_NEXT_ENTRY;
    tcamOpData.tcamEntry = (tcamPointer == 0 ? 0xff : (tcamPointer - 1));
    retVal = tcamOperationPerform(dev,op, &tcamOpData);
    if(retVal == GT_NO_SUCH )
    {
        PRV_CPSS_DBG_INFO(("No higher valid entry founded return GT_NO_SUCH"));
        return retVal;
    }

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    if(tcamOpData.tcamEntry != tcamPointer)
    {
        PRV_CPSS_DBG_INFO(("Not found this entry"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    *found = GT_TRUE;
    getTcamHWData(&tcamOpData.tcamDataP,tcamData);
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGtcamEgrFlushEntry function
* @endinternal
*
* @brief   This routine is to flush a single egress entry for a particular port. A
*         Flush a single Egress TCAM entry command will write the same values to a
*         Egress TCAM entry as a Flush All command, but it is done to the selected
*         single egress TCAM entry of the selected single port only.
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrFlushEntry
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U32          tcamPointer
)
{
    GT_STATUS                 retVal;
    GT_CPSS_TCAM_OPERATION    op;
    GT_CPSS_TCAM_EGR_OP_DATA  tcamOpData;
    GT_U8                     hwPort;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamEgrFlushEntry Called.\n"));

    /* check if the given port is valid */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check if the given pointer is valid */
    if ((tcamPointer > 0x3F) || (tcamPointer == 0))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_FLUSH_ENTRY;
    tcamOpData.port = hwPort;
    tcamOpData.tcamEntry = tcamPointer;
    retVal = tcamEgrOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamEgrOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGtcamEgrFlushEntryAllPorts function
* @endinternal
*
* @brief   This routine is to flush a single egress entry for all switch ports.
*
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrFlushEntryAllPorts
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          tcamPointer
)
{
    GT_STATUS                 retVal;
    GT_CPSS_TCAM_OPERATION    op;
    GT_CPSS_TCAM_EGR_OP_DATA  tcamOpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamEgrFlushEntryAllPorts Called.\n"));

    /* check if the given pointer is valid */
    if ((tcamPointer > 0x3F) || (tcamPointer == 0))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_FLUSH_ENTRY;
    tcamOpData.port = 0x1F;
    tcamOpData.tcamEntry = tcamPointer;
    retVal = tcamEgrOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamEgrOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGtcamEgrLoadEntry function
* @endinternal
*
* @brief   This routine loads a single egress TCAM entry for a specific port.
*
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
* @param[in] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrLoadEntry
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_CPSS_LPORT          port,
    IN  GT_U32                 tcamPointer,
    IN  GT_CPSS_TCAM_EGR_DATA  *tcamData
)
{
    GT_STATUS                 retVal;
    GT_CPSS_TCAM_OPERATION    op;
    GT_CPSS_TCAM_EGR_OP_DATA  tcamOpData;
    GT_U8                     hwPort;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamEgrLoadEntry Called.\n"));

    /* check if the given port is valid */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check if the given pointer is valid */
    if ((tcamPointer > 0x3F)|| (tcamPointer == 0) || (tcamData==NULL))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_LOAD_ENTRY;
    tcamOpData.port = hwPort;
    tcamOpData.tcamEntry = tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    setTcamEgrHWData(tcamData,&tcamOpData.tcamDataP);
    retVal = tcamEgrOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamEgrOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGtcamEgrGetNextTCAMData function
* @endinternal
*
* @brief   This routine finds the next higher Egress TCAM Entry number that is valid
*         (i.e.,any entry that is non-zero). The TCAM Entry register (bits 5:0) is
*         used as the Egress TCAM entry to start from. To find the lowest number
*         Egress TCAM Entry that is valid, start the Get Next operation with Egress
*         TCAM Entry set to 0x3F.
* @param[in] port                     - switch port
* @param[in,out] tcamPointer              - pointer to start search TCAM
* @param[in,out] tcamPointer              - next pointer entry of TCAM
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrGetNextTCAMData
(
    IN    GT_CPSS_QD_DEV         *dev,
    IN    GT_CPSS_LPORT          port,
    INOUT GT_U32                 *tcamPointer,
    OUT   GT_CPSS_TCAM_EGR_DATA  *tcamData
)
{
    GT_STATUS                 retVal;
    GT_CPSS_TCAM_OPERATION    op;
    GT_CPSS_TCAM_EGR_OP_DATA  tcamOpData;
    GT_U8                     hwPort;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamEgrGetNextTCAMData Called.\n"));

    /* check if the given port is valid */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check if the given pointer is valid */
    if ((*tcamPointer > 0x3F) || (tcamData==NULL))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_GET_NEXT_ENTRY;
    tcamOpData.port = hwPort;
    tcamOpData.tcamEntry = *tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    retVal = tcamEgrOperationPerform(dev,op, &tcamOpData);
    if(retVal == GT_NO_SUCH)
    {
        PRV_CPSS_DBG_INFO(("No higher valid entry founded return GT_NO_SUCH"));
        return retVal;
    }
    else if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamEgrOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    *tcamPointer = tcamOpData.tcamEntry;
    getTcamEgrHWData(&tcamOpData.tcamDataP,tcamData);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGtcamEgrReadTCAMData function
* @endinternal
*
* @brief   This routine reads a single Egress TCAM entry for a specific port.
*
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrReadTCAMData
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_CPSS_LPORT          port,
    IN  GT_U32                 tcamPointer,
    OUT GT_CPSS_TCAM_EGR_DATA  *tcamData
)
{
    GT_STATUS                 retVal;
    GT_CPSS_TCAM_OPERATION    op;
    GT_CPSS_TCAM_EGR_OP_DATA  tcamOpData;
    GT_U8                     hwPort;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGtcamEgrReadTCAMData Called.\n"));

    /* check if the given port is valid */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check if the given pointer is valid */
    if ((tcamPointer > 0x3F) || (tcamPointer == 0) ||(tcamData==NULL))
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad tcamPointer).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Program Tuning register */
    op = TCAM_READ_ENTRY;
    tcamOpData.port = hwPort;
    tcamOpData.tcamEntry = tcamPointer;
    /*tcamOpData.tcamDataP = tcamData;*/
    retVal = tcamEgrOperationPerform(dev,op, &tcamOpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (tcamEgrOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    getTcamEgrHWData(&tcamOpData.tcamDataP,tcamData);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/
static GT_STATUS setTcamHWData
(
    IN  GT_CPSS_TCAM_DATA     *iData,
    OUT GT_CPSS_TCAM_DATA_HW  *oData
)
{
    int i;

    prvCpssDrvGtMemSet(oData,0,sizeof(GT_CPSS_TCAM_DATA_HW));

    /*key*/
    PRV_CPSS_BF_SET(oData->pg0.frame[0],iData->frameTypeMask, 14, 2);
    PRV_CPSS_BF_SET(oData->pg0.frame[0],iData->frameType, 6, 2);
    PRV_CPSS_BF_SET(oData->pg0.frame[0],((iData->spvMask&0x700)>>8), 8, 3);
    PRV_CPSS_BF_SET(oData->pg0.frame[0],((iData->spv&0x700)>>8), 0, 3);

    PRV_CPSS_BF_SET(oData->pg0.frame[1],(iData->spvMask&0xff), 8, 8);
    PRV_CPSS_BF_SET(oData->pg0.frame[1],(iData->spv&0xff), 0, 8);

    PRV_CPSS_BF_SET(oData->pg0.frame[2],iData->ppriMask, 12, 4);
    PRV_CPSS_BF_SET(oData->pg0.frame[2],iData->ppri, 4, 4);
    PRV_CPSS_BF_SET(oData->pg0.frame[2],((iData->pvidMask&0xf00)>>8), 8, 4);
    PRV_CPSS_BF_SET(oData->pg0.frame[2],((iData->pvid&0xf00)>>8), 0, 4);

    PRV_CPSS_BF_SET(oData->pg0.frame[3],(iData->pvidMask&0xff), 8, 8);
    PRV_CPSS_BF_SET(oData->pg0.frame[3],(iData->pvid&0xff), 0, 8);

    /*Page 0 (0~21) bytes*/
    for(i=4; i<26; i++)
    {
        PRV_CPSS_BF_SET(oData->pg0.frame[i],iData->frameOctetMask[i-4],8,8);
        PRV_CPSS_BF_SET(oData->pg0.frame[i],iData->frameOctet[i-4],0,8);
    }

    /*Page 1 (22~47) bytes*/
    for(i=0; i<26; i++)
    {
        PRV_CPSS_BF_SET(oData->pg1.frame[i],iData->frameOctetMask[22+i],8,8);
        PRV_CPSS_BF_SET(oData->pg1.frame[i],iData->frameOctet[22+i],0,8);
    }

    /*Action*/
    PRV_CPSS_BF_SET(oData->pg2.frame[0],iData->continu, 15, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[0],iData->interrupt, 14, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[0],iData->IncTcamCtr, 13, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[0],(GT_U16)(iData->vidOverride), 12, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[0],iData->vidData, 0, 12);

    PRV_CPSS_BF_SET(oData->pg2.frame[1],iData->nextId, 8, 8);
    PRV_CPSS_BF_SET(oData->pg2.frame[1],(GT_U16)(iData->qpriOverride), 7, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[1],iData->qpriData, 4, 3);
    PRV_CPSS_BF_SET(oData->pg2.frame[1],(GT_U16)(iData->fpriOverride), 3, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[1],iData->fpriData, 0, 3);

    PRV_CPSS_BF_SET(oData->pg2.frame[2],iData->dpvSF, 11, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[2],(GT_U16)(iData->dpvData), 0, 11);

    /*
    PRV_CPSS_BF_SET(oData->pg2.frame[3], 0, 0, 16);
    */
    oData->pg2.frame[3] = 0;

    PRV_CPSS_BF_SET(oData->pg2.frame[4],iData->dpvMode, 14, 2);
    PRV_CPSS_BF_SET(oData->pg2.frame[4],iData->colorMode, 12, 2);
    PRV_CPSS_BF_SET(oData->pg2.frame[4],(GT_U16)(iData->vtuPageOverride), 11, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[4],iData->vtuPage, 10, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[4],iData->unKnownFilter, 8, 2);
    PRV_CPSS_BF_SET(oData->pg2.frame[4],iData->egActPoint, 0, 6);

    PRV_CPSS_BF_SET(oData->pg2.frame[5],(GT_U16)(iData->ldBalanceOverride), 15, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[5],iData->ldBalanceData, 12, 3);
    PRV_CPSS_BF_SET(oData->pg2.frame[5],(GT_U16)(iData->DSCPOverride), 6, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[5],iData->DSCP, 0, 6);

    PRV_CPSS_BF_SET(oData->pg2.frame[6],(GT_U16)(iData->factionOverride), 15, 1);
    PRV_CPSS_BF_SET(oData->pg2.frame[6],iData->factionData, 0, 15);

    return GT_OK;
}

static GT_STATUS getTcamHWData
(
    IN  GT_CPSS_TCAM_DATA_HW  *iData,
    OUT GT_CPSS_TCAM_DATA     *oData
)
{
    int i;
    /*key*/
    oData->frameTypeMask = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[0],14, 2));
    oData->frameType = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[0],6, 2));

    oData->spvMask = (PRV_CPSS_BF_GET(iData->pg0.frame[0],8, 3) << 8) | PRV_CPSS_BF_GET(iData->pg0.frame[1],8, 8);
    oData->spv = (PRV_CPSS_BF_GET(iData->pg0.frame[0],0, 3) << 8) | PRV_CPSS_BF_GET(iData->pg0.frame[1], 0, 8);

    oData->ppriMask = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[2],12, 4));
    oData->ppri = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[2],4, 4));

    oData->pvidMask = (PRV_CPSS_BF_GET(iData->pg0.frame[2], 8, 4) << 8) | PRV_CPSS_BF_GET(iData->pg0.frame[3],8, 8);

    oData->pvid = (PRV_CPSS_BF_GET(iData->pg0.frame[2], 0, 4) << 8) | PRV_CPSS_BF_GET(iData->pg0.frame[3], 0, 8);

    /*Page 0 (0~21) bytes*/
    for(i=4; i<26; i++)
    {
        oData->frameOctetMask[i-4] = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[i],8, 8));
        oData->frameOctet[i-4] = (GT_U8)(PRV_CPSS_BF_GET(iData->pg0.frame[i],0,8));
    }

    /*Page 1 (22~47) bytes*/
    for(i=0; i<26; i++)
    {
        oData->frameOctetMask[22+i] = (GT_U8)(PRV_CPSS_BF_GET(iData->pg1.frame[i],8,8));
        oData->frameOctet[22+i] = (GT_U8)(PRV_CPSS_BF_GET(iData->pg1.frame[i],0,8));
    }

    /*Action*/
    oData->continu = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[0], 15, 1));
    oData->interrupt = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[0], 14, 1));
    oData->IncTcamCtr = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[0], 13, 1));
    oData->vidOverride = PRV_CPSS_BF_GET(iData->pg2.frame[0], 12, 1);
    oData->vidData = PRV_CPSS_BF_GET(iData->pg2.frame[0], 0, 12);

    oData->nextId = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[1], 8, 8));
    oData->qpriOverride = PRV_CPSS_BF_GET(iData->pg2.frame[1], 7, 1);
    oData->qpriData = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[1], 4, 3));
    oData->fpriOverride = PRV_CPSS_BF_GET(iData->pg2.frame[1], 3, 1);
    oData->fpriData = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[1], 0, 3));

    oData->dpvSF = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[2], 11, 1));
    oData->dpvData = PRV_CPSS_BF_GET(iData->pg2.frame[2], 0, 11);

    oData->dpvMode = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[4], 14, 2));
    oData->colorMode = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[4], 12, 2));
    oData->vtuPageOverride = PRV_CPSS_BF_GET(iData->pg2.frame[4], 11, 1);
    oData->vtuPage = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[4], 10, 1));
    oData->unKnownFilter = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[4], 8, 2));
    oData->egActPoint = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[4], 0, 6));

    oData->ldBalanceOverride = PRV_CPSS_BF_GET(iData->pg2.frame[5], 15, 1);
    oData->ldBalanceData = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[5], 12, 3));
    oData->DSCPOverride = PRV_CPSS_BF_GET(iData->pg2.frame[5], 6, 1);
    oData->DSCP = (GT_U8)(PRV_CPSS_BF_GET(iData->pg2.frame[5], 0, 6));

    oData->factionOverride = PRV_CPSS_BF_GET(iData->pg2.frame[6], 15, 1);
    oData->factionData = PRV_CPSS_BF_GET(iData->pg2.frame[6], 0, 15);

    return GT_OK;
}

static GT_STATUS setTcamEgrHWData
(
    IN  GT_CPSS_TCAM_EGR_DATA    *iData,
    OUT GT_CPSS_TCAM_EGR_DATA_HW *oData
)
{
    prvCpssDrvGtMemSet(oData,0,sizeof(GT_CPSS_TCAM_EGR_DATA_HW));

    PRV_CPSS_BF_SET(oData->frame[0],(GT_U16)(iData->frameModeOverride), 14, 1);
    PRV_CPSS_BF_SET(oData->frame[0],iData->frameMode, 12, 2);
    PRV_CPSS_BF_SET(oData->frame[0],(GT_U16)(iData->tagModeOverride), 10, 1);
    PRV_CPSS_BF_SET(oData->frame[0],iData->tagMode, 8, 2);
    PRV_CPSS_BF_SET(oData->frame[0],iData->daMode, 4, 2);
    PRV_CPSS_BF_SET(oData->frame[0],iData->saMode, 0, 3);

    PRV_CPSS_BF_SET(oData->frame[1],(GT_U16)(iData->egVidModeOverride), 14, 1);
    PRV_CPSS_BF_SET(oData->frame[1],iData->egVidMode, 12, 2);
    PRV_CPSS_BF_SET(oData->frame[1],iData->egVidData, 0, 12);

    PRV_CPSS_BF_SET(oData->frame[2],iData->egDSCPMode, 14, 2);
    PRV_CPSS_BF_SET(oData->frame[2],iData->egDSCP, 8, 6);
    PRV_CPSS_BF_SET(oData->frame[2],(GT_U16)(iData->egfpriModeOverride), 6, 1);
    PRV_CPSS_BF_SET(oData->frame[2],iData->egfpriMode, 4, 2);
    PRV_CPSS_BF_SET(oData->frame[2],iData->egEC, 3, 1);
    PRV_CPSS_BF_SET(oData->frame[2],iData->egFPRI, 0, 3);

    return GT_OK;
}

static GT_STATUS getTcamEgrHWData
(
    IN  GT_CPSS_TCAM_EGR_DATA_HW  *iData,
    OUT GT_CPSS_TCAM_EGR_DATA     *oData
)
{
    oData->frameModeOverride = PRV_CPSS_BF_GET(iData->frame[0], 14, 1);
    oData->frameMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[0], 12, 2));
    oData->tagModeOverride = PRV_CPSS_BF_GET(iData->frame[0], 10, 1);
    oData->tagMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[0], 8, 2));
    oData->daMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[0], 4, 2));
    oData->saMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[0], 0, 3));

    oData->egVidModeOverride = PRV_CPSS_BF_GET(iData->frame[1], 14, 1);
    oData->egVidMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[1], 12, 2));
    oData->egVidData = PRV_CPSS_BF_GET(iData->frame[1], 0, 12);

    oData->egDSCPMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[2], 14, 2));
    oData->egDSCP = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[2], 8, 6));
    oData->egfpriModeOverride = PRV_CPSS_BF_GET(iData->frame[2], 6, 1);
    oData->egfpriMode = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[2], 4, 2));
    oData->egEC = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[2], 3, 1));
    oData->egFPRI = (GT_U8)(PRV_CPSS_BF_GET(iData->frame[2], 0, 3));

    return GT_OK;
}
static GT_STATUS tcamSetPage0Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<0x1c; i++)
    {
        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,tcamDataP->pg0.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamSetPage1Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<0x1c; i++)
    {
        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,tcamDataP->pg1.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamSetPage2Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<9; i++)
    {
        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,tcamDataP->pg2.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamSetPage3Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_EGR_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<5; i++)
    {
        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,tcamDataP->frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamGetPage0Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;


    for(i=2; i<0x1c; i++)
    {
        retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,&tcamDataP->pg0.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamGetPage1Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;


    for(i=2; i<0x1c; i++)
    {
        retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,&tcamDataP->pg1.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamGetPage2Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<9; i++)
    {
        retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,&tcamDataP->pg2.frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS tcamGetPage3Data(GT_CPSS_QD_DEV *dev, GT_CPSS_TCAM_EGR_DATA_HW *tcamDataP)
{
    GT_STATUS       retVal;    /* Functions return value */
    int i;

    for(i=2; i<5; i++)
    {
        retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,(GT_U8)i,&tcamDataP->frame[i-2]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}
static GT_STATUS waitTcamReady(GT_CPSS_QD_DEV           *dev)
{
    GT_STATUS       retVal;    /* Functions return value */

    GT_U16          data;     /* temporary Data storage */
    GT_U32      retryCount = 0; /* Counter for busy wait loops */

    data = 1;
    while(data == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        /* check that the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }
    return GT_OK;
}

/**
* @internal tcamOperationPerform function
* @endinternal
*
* @brief   This function accesses TCAM Table
*
* @param[in] tcamOp                   - The tcam operation
*                                      tcamData - address and data to be written into TCAM
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS tcamOperationPerform
(
    IN    GT_CPSS_QD_DEV           *dev,
    IN    GT_CPSS_TCAM_OPERATION   tcamOp,
    INOUT GT_CPSS_TCAM_OP_DATA     *opData
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          data;     /* temporary Data storage */

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* Wait until the tcam in ready. */
    retVal = waitTcamReady(dev);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    /* Set the TCAM Operation register */
    switch (tcamOp)
    {
        case TCAM_FLUSH_ALL:
        {
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (tcamOp << 12));
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
        }
        break;

        case TCAM_FLUSH_ENTRY:
        {
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | opData->tcamEntry);
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
        }
        break;

        case TCAM_LOAD_ENTRY:
        /*    case TCAM_PURGE_ENTRY: */
        {
            /* load Page 2 */
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamSetPage2Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (TCAM_LOAD_ENTRY << 12) | (2 << 10) | opData->tcamEntry);
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* load Page 1 */
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamSetPage1Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            data = 0;
            data = (GT_U16)((1 << 15) | (TCAM_LOAD_ENTRY << 12) | (1 << 10) | opData->tcamEntry) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            /* load Page 0 */
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamSetPage0Data(dev,  &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (TCAM_LOAD_ENTRY << 12) | (0 << 10) | opData->tcamEntry);
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

        }
        break;

        case TCAM_GET_NEXT_ENTRY:
        {
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (opData->tcamEntry)) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION, &data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            if ((data&0xff)==0xff)
            {
            GT_U16 data1;
            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_P0_KEYS_1, &data1);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            if(data1==0x00ff)
            {
                /* No higher valid TCAM entry */
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
            }
            else
            {
                /* The highest valid TCAM entry found*/
            }
            }

            /* Get next entry and read the entry */
            opData->tcamEntry = data&0xff;

        }
        case TCAM_READ_ENTRY:
        {
            tcamOp = TCAM_READ_ENTRY;

            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            /* Read page 0 */
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (0 << 10) | opData->tcamEntry) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamGetPage0Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            /* Read page 1 */
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (1 << 10) | opData->tcamEntry) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamGetPage1Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            /* Read page 2 */
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (2 << 10) | opData->tcamEntry) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamGetPage2Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
        }
        break;

        default:
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
    return retVal;
}


static GT_STATUS tcamEgrOperationPerform
(
    IN    GT_CPSS_QD_DEV           *dev,
    IN    GT_CPSS_TCAM_OPERATION   tcamOp,
    INOUT GT_CPSS_TCAM_EGR_OP_DATA *opData
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          data;     /* temporary Data storage */

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* Wait until the tcam in ready. */
    retVal = waitTcamReady(dev);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    /* Set the TCAM Operation register */
    switch (tcamOp)
    {
        case TCAM_FLUSH_ENTRY:
        {
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_PORT,opData->port);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (3<<10) | opData->tcamEntry);
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_PORT,0);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
        }
        break;

        case TCAM_LOAD_ENTRY:
        {
            /* load Page 2 */
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_PORT,opData->port);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamSetPage3Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((1 << 15) | (TCAM_LOAD_ENTRY << 12) | (3 << 10) | (opData->tcamEntry));
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

        }
        break;

        case TCAM_GET_NEXT_ENTRY:
        {
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_PORT,opData->port);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            data = (GT_U16)((tcamOp << 12) | (3 << 10)| (opData->tcamEntry)) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (3 << 10)| (opData->tcamEntry)) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION, &data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            if ((data&0x3f)==0x3f)
            {
            GT_U16 data1,data2,data3;
            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_ACTION_1, &data1);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_ACTION_2, &data2);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_ACTION_3, &data3);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            if(data1==0x0 && data2==0x0 && data3==0x0)
            {
                /* No higher valid TCAM entry */
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
            }
            else
            {
                /* The highest valid TCAM entry found*/
            }
            }

            /* Get next entry and read the entry */
            opData->tcamEntry = data&0xff;

        }

        case TCAM_READ_ENTRY:
        {
            tcamOp = TCAM_READ_ENTRY;

            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_EGR_PORT,opData->port);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            data = 0;
            /* Read page 0 */
            data = (GT_U16)((1 << 15) | (tcamOp << 12) | (3 << 10) | opData->tcamEntry) ;
            retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_TCAM_DEV_ADDR,PRV_CPSS_QD_REG_TCAM_OPERATION,data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            retVal = tcamGetPage3Data(dev, &opData->tcamDataP);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }

            /* Wait until the tcam in ready. */
            retVal = waitTcamReady(dev);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
                return retVal;
            }
        }
        break;

        default:
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
    return retVal;
}


