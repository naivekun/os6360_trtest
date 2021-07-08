#include <Copyright.h>

/**
********************************************************************************
* @file gtBrgVtu.c
*
* @brief API definitions for Vlan Translation Unit for 802.1Q.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtBrgVtu.c
*
* DESCRIPTION:
*       API definitions for Vlan Translation Unit for 802.1Q.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/
#include <gtBrgVtu.h>
#include <msApiInternal.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>

/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/

static GT_STATUS vtuOperationPerform
(
    IN        GT_CPSS_QD_DEV         *dev,
    IN      GT_CPSS_VTU_OPERATION    vtuOp,
    INOUT   GT_U8               *valid,
    INOUT     GT_CPSS_VTU_ENTRY      *vtuEntry
);

/**
* @internal prvCpssDrvGvtuGetEntryNext function
* @endinternal
*
* @brief   Gets next valid VTU entry from the specified VID.
*
* @param[in,out] vtuEntry                 - the VID and VTUPage to start the search.
* @param[in,out] vtuEntry                 - match VTU  entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note Search starts from the VID and VTU Page (13th vid) in the vtuEntry
*       specified by the user.
*
*/
GT_STATUS prvCpssDrvGvtuGetEntryNext
(
    IN    GT_CPSS_QD_DEV     *dev,
    INOUT GT_CPSS_VTU_ENTRY  *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS           retVal;
    GT_U8               port;
    GT_CPSS_LPORT       lport;
    GT_CPSS_VTU_ENTRY   entry;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuGetEntryNext Called.\n"));

    if(vtuEntry->vid > 0xfff )
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad vid).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    entry.vid   = vtuEntry->vid;
    entry.vidExInfo.vtuPage = vtuEntry->vidExInfo.vtuPage;
    valid = 0;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrieve the value from the operation */

    if((entry.vid == 0xFFF) && (valid == 0))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    vtuEntry->DBNum = entry.DBNum;
    vtuEntry->vid   = entry.vid;

    vtuEntry->vidPolicy = entry.vidPolicy;
    vtuEntry->sid = entry.sid;

    vtuEntry->vidExInfo.useVIDQPri = entry.vidExInfo.useVIDQPri;
    vtuEntry->vidExInfo.vidQPri = entry.vidExInfo.vidQPri;
    vtuEntry->vidExInfo.useVIDFPri = entry.vidExInfo.useVIDFPri;
    vtuEntry->vidExInfo.vidFPri = entry.vidExInfo.vidFPri;
    vtuEntry->vidExInfo.dontLearn = entry.vidExInfo.dontLearn;
    vtuEntry->vidExInfo.filterUC = entry.vidExInfo.filterUC;
    vtuEntry->vidExInfo.filterBC = entry.vidExInfo.filterBC;
    vtuEntry->vidExInfo.filterMC = entry.vidExInfo.filterMC;
    vtuEntry->vidExInfo.vtuPage = entry.vidExInfo.vtuPage;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
        port = GT_CPSS_LPORT_2_PORT(lport);
        vtuEntry->memberTagP[lport]=entry.memberTagP[port];
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuFindVidEntry function
* @endinternal
*
* @brief   Find VTU entry for a specific VID and VTU Page, it will return the
*         entry, if found, along with its associated data
* @param[in,out] vtuEntry                 - contains the VID and VTU Page to search for.
*
* @param[out] found                    - GT_TRUE, if the appropriate entry exists.
* @param[in,out] vtuEntry                 - the entry parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error or entry does not exist.
* @retval GT_NO_SUCH               - no more entries.
*/
GT_STATUS prvCpssDrvGvtuFindVidEntry
(
    IN  GT_CPSS_QD_DEV       *dev,
    INOUT GT_CPSS_VTU_ENTRY  *vtuEntry,
    OUT GT_BOOL         *found
)
{
    GT_U8               valid;
    GT_STATUS           retVal;
    GT_U8               port;
    GT_CPSS_LPORT       lport;
    GT_CPSS_VTU_ENTRY   entry;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuFindVidEntry Called.\n"));

    *found = GT_FALSE;

    valid = 0; /* valid is not used as input in this operation */
    if ((1 == vtuEntry->vidExInfo.vtuPage) && (0 == vtuEntry->vid))
    {
        entry.vid = 0xfff;
        entry.vidExInfo.vtuPage = 0;
    }
    else if ((0 == vtuEntry->vidExInfo.vtuPage) && (0 == vtuEntry->vid))
    {
        entry.vid = 0xfff;
        entry.vidExInfo.vtuPage = 1;
    }
    else
    {
        /* Decrement 1 from vid    */
        entry.vid   = vtuEntry->vid-1;
        entry.vidExInfo.vtuPage = vtuEntry->vidExInfo.vtuPage;
    }

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */

    if( (entry.vid !=vtuEntry->vid) || (entry.vidExInfo.vtuPage != vtuEntry->vidExInfo.vtuPage) || (valid !=1) )
    {
          PRV_CPSS_DBG_INFO(("Failed.\n"));
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    vtuEntry->DBNum = entry.DBNum;
    vtuEntry->vid   = entry.vid;

    vtuEntry->vidPolicy = entry.vidPolicy;
    vtuEntry->sid = entry.sid;

    vtuEntry->vidExInfo.useVIDQPri = entry.vidExInfo.useVIDQPri;
    vtuEntry->vidExInfo.vidQPri = entry.vidExInfo.vidQPri;
    vtuEntry->vidExInfo.useVIDFPri = entry.vidExInfo.useVIDFPri;
    vtuEntry->vidExInfo.vidFPri = entry.vidExInfo.vidFPri;
    vtuEntry->vidExInfo.dontLearn = entry.vidExInfo.dontLearn;
    vtuEntry->vidExInfo.filterUC = entry.vidExInfo.filterUC;
    vtuEntry->vidExInfo.filterBC = entry.vidExInfo.filterBC;
    vtuEntry->vidExInfo.filterMC = entry.vidExInfo.filterMC;
    vtuEntry->vidExInfo.vtuPage = entry.vidExInfo.vtuPage;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
        port = GT_CPSS_LPORT_2_PORT(lport);
        vtuEntry->memberTagP[lport]=entry.memberTagP[port];
    }

    *found = GT_TRUE;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuFlush function
* @endinternal
*
* @brief   This routine removes all entries from VTU Table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuFlush
(
    IN  GT_CPSS_QD_DEV       *dev
)
{
    GT_STATUS       retVal;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuFlush Called.\n"));

    retVal = vtuOperationPerform(dev,FLUSH_ALL,NULL,NULL);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuAddEntry function
* @endinternal
*
* @brief   Creates the new entry in VTU table based on user input.
*
* @param[in] vtuEntry                 - vtu entry to insert to the VTU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvtuAddEntry
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_VTU_ENTRY  *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS           retVal;
    GT_U8               port;
    GT_CPSS_LPORT       lport;
    GT_CPSS_VTU_ENTRY   entry;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuAddEntry Called.\n"));

    if(vtuEntry->vid > 0xfff || vtuEntry->sid > 0x3f || vtuEntry->DBNum > 0xfff)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad vid or sid or DBNum).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    entry.DBNum = vtuEntry->DBNum;
    entry.vid   = vtuEntry->vid;
    entry.vidPolicy = vtuEntry->vidPolicy;
    entry.sid = vtuEntry->sid;
    entry.vidExInfo.useVIDQPri = vtuEntry->vidExInfo.useVIDQPri;
    entry.vidExInfo.vidQPri = vtuEntry->vidExInfo.vidQPri;
    entry.vidExInfo.useVIDFPri = vtuEntry->vidExInfo.useVIDFPri;
    entry.vidExInfo.vidFPri = vtuEntry->vidExInfo.vidFPri;
    entry.vidExInfo.dontLearn = vtuEntry->vidExInfo.dontLearn;
    entry.vidExInfo.filterUC = vtuEntry->vidExInfo.filterUC;
    entry.vidExInfo.filterBC = vtuEntry->vidExInfo.filterBC;
    entry.vidExInfo.filterMC = vtuEntry->vidExInfo.filterMC;
    entry.vidExInfo.vtuPage = vtuEntry->vidExInfo.vtuPage;

    valid = 1; /* for load operation */

    for(port=0; port<dev->maxPorts; port++)
    {
        lport = GT_CPSS_PORT_2_LPORT(port);
        if(lport == GT_CPSS_INVALID_PORT)
        {
            entry.memberTagP[port] = CPSS_MEMTAGP_NOT_A_MEMBER;
        }
        else
        {
            entry.memberTagP[port] = vtuEntry->memberTagP[lport];
        }
    }

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuDelEntry function
* @endinternal
*
* @brief   Deletes VTU entry specified by user.
*
* @param[in] vtuEntry                 - the VTU entry to be deleted
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvtuDelEntry
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_VTU_ENTRY  *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS           retVal;
    GT_CPSS_VTU_ENTRY   entry;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuDelEntry Called.\n"));

    if(vtuEntry->vid > 0xfff)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad vid).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    entry.vid   = vtuEntry->vid;
    entry.vidExInfo.vtuPage = vtuEntry->vidExInfo.vtuPage;

    valid = 0; /* for delete operation */

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuSetMode function
* @endinternal
*
* @brief   Set VTU Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuSetMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_VTU_MODE    mode
)
{
    GT_STATUS   retVal;
    GT_U16      data;
    GT_U32      retryCount = 0; /* Counter for busy wait loops */


    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuSetMode Called.\n"));
    prvCpssDrvGtSemTake(dev,dev->vtuRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    data = 1;
    while(data == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        /* the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    data = (GT_U16)mode;
    retVal = prvCpssDrvHwSetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,10,2,data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("ERROR to write VTU OPERATION Register.\n"));
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
    }
    prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuGetMode function
* @endinternal
*
* @brief   Get VTU Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGvtuGetMode
(
    IN  GT_CPSS_QD_DEV     *dev,
    OUT GT_CPSS_VTU_MODE   *mode
)
{
    GT_STATUS   retVal;
    GT_U16      data;
    GT_U32      retryCount = 0; /* Counter for busy wait loops */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuGetMode Called.\n"));
    prvCpssDrvGtSemTake(dev,dev->vtuRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    data = 1;
    while(data == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        /* the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));

    }

    retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,10,2,&data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("ERROR to write VTU OPERATION Register.\n"));
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
    }
    switch(data)
    {
        case 0:
            *mode = CPSS_VTU_MODE_0;
            break;
        case 1:
            *mode = CPSS_VTU_MODE_1;
            break;
        case 2:
            *mode = CPSS_VTU_MODE_2;
            break;
        case 3:
            *mode = CPSS_VTU_MODE_3;
            break;
        default:
            PRV_CPSS_DBG_INFO(("ERROR to Get VTU MODE.\n"));
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
    return GT_OK;
}

/**
* @internal prvCpssDrvGvtuGetViolation function
* @endinternal
*
* @brief   Get VTU Violation data
*
* @param[out] vtuIntStatus             - interrupt cause, source portID, and vid.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none
*
*/
GT_STATUS prvCpssDrvGvtuGetViolation
(
    IN  GT_CPSS_QD_DEV         *dev,
    OUT GT_CPSS_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_U8               spid;
    GT_U16              vid;
    GT_U8               page;
    GT_U16              intCause;
    GT_STATUS           retVal;
    GT_CPSS_VTU_ENTRY        entry;
    GT_U16              tmp;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvtuGetViolation Called.\n"));

    /* check Violation occurred */
    retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_GLOBAL_STATUS,5,1,&intCause);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

    if (intCause == 0)
    {
        /* No Violation occurred. */
        vtuIntStatus->vtuIntCause = 0;
        return GT_OK;
    }

    entry.DBNum = 0;

    retVal = vtuOperationPerform(dev,SERVICE_VIOLATIONS,NULL, &entry);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,&tmp);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

    intCause = (GT_U16)((tmp >> 4) & 0x7);
    spid = (GT_U8)(tmp & 0x1F);
    vid = entry.vid;
    page = entry.vidExInfo.vtuPage;

    if(spid == 0x1F)
    {
        vtuIntStatus->vtuIntCause = GT_CPSS_VTU_FULL_VIOLATION;
        vtuIntStatus->spid = spid;
        vtuIntStatus->vid = 0;
    }
    else
    {
        vtuIntStatus->vtuIntCause = intCause & (GT_CPSS_VTU_MEMBER_VIOLATION | GT_CPSS_VTU_MISS_VIOLATION);
        vtuIntStatus->spid = spid;
        vtuIntStatus->vid = vid|((page & 0x1)<<12);
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/**
* @internal vtuOperationPerform function
* @endinternal
*
* @brief   This function is used by all VTU control functions, and is responsible
*         to write the required operation into the VTU registers.
* @param[in] vtuOp                    - The VTU operation bits to be written into the VTU
*                                      operation register.
*                                      DBNum       - DBNum where the given vid belongs to
*                                      vid         - vlan id
* @param[in,out] valid                    -  bit
*                                      vtuData     - VTU Data with memberTag information
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/

static GT_STATUS vtuOperationPerform
(
    IN    GT_CPSS_QD_DEV          *dev,
    IN    GT_CPSS_VTU_OPERATION   vtuOp,
    INOUT GT_U8                   *valid,
    INOUT GT_CPSS_VTU_ENTRY       *entry
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U32      retryCount = 0;     /* Counter for busy wait loops */

    prvCpssDrvGtSemTake(dev,dev->vtuRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    data = 1;
    while(data == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        /* the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    /* Set the VTU data register    */
    /* There is no need to setup data reg. on flush, get next, or service violation */
    if((vtuOp != FLUSH_ALL) && (vtuOp != GET_NEXT_ENTRY) && (vtuOp != SERVICE_VIOLATIONS))
    {
        /****************** VTU DATA 1 REG *******************/

        /* get data and wirte to PRV_CPSS_QD_REG_VTU_DATA1_REG (ports 0 to 3) */
        data = (GT_U16) ((entry->memberTagP[0] & 3)|((entry->memberTagP[1] & 3)<<2)
            |((entry->memberTagP[2] & 3)<<4)|((entry->memberTagP[3] & 3)<<6)
            |((entry->memberTagP[4] & 3)<<8)|((entry->memberTagP[5] & 3)<<10)
            |((entry->memberTagP[6] & 3)<<12)|((entry->memberTagP[7] & 3)<<14));

        retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_DATA1_REG,data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        /****************** VTU DATA 2 REG *******************/
        if(dev->maxPorts > 8)
        {

            data =  (GT_U16)((entry->memberTagP[8] & 3)|((entry->memberTagP[9] & 3)<<2)
                |((entry->memberTagP[10] & 3)<<4));

            if(entry->vidExInfo.useVIDQPri == GT_TRUE)
                data |= ((1 << 15) | ((entry->vidExInfo.vidQPri & 0x7) << 12));

            if(entry->vidExInfo.useVIDFPri == GT_TRUE)
                data |= ((1 << 11) | ((entry->vidExInfo.vidFPri & 0x7) << 8));

            retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_DATA2_REG,data);
            if(retVal != GT_OK)
            {
              prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
              return retVal;
            }
        }
    }

    /* Set the VID register (PRV_CPSS_QD_REG_VTU_VID_REG) */
    /* There is no need to setup VID reg. on flush and service violation */
    if((vtuOp != FLUSH_ALL) && (vtuOp != SERVICE_VIOLATIONS) )
    {
        data = 0;

        data |= (((entry->vidExInfo.vtuPage & 0x1)<<13) | ((entry->vid) & 0xFFF ) | ( (*valid) << 12 ));
        retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,(GT_U8)(PRV_CPSS_QD_REG_VTU_VID_REG),data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
    }

    /* Set SID, FID, VIDPolicy, if it's Load operation */
    if((vtuOp == LOAD_PURGE_ENTRY) && (*valid == 1))
    {
        data= (entry->sid) & 0x3F;
        if(entry->vidExInfo.dontLearn)
            data |= 0x8000;
        if(entry->vidExInfo.filterUC)
            data |= 0x4000;
        if(entry->vidExInfo.filterBC)
            data |= 0x2000;
        if(entry->vidExInfo.filterMC)
            data |= 0x1000;

        retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,(GT_U8)(PRV_CPSS_QD_REG_STU_SID_REG),data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }


        data = 0;
        data= (GT_U16)((entry->vidPolicy) << 12);


        data |= (entry->DBNum & 0xFFF);

        retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,(GT_U8)(PRV_CPSS_QD_REG_VTU_FID_REG),data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
    }

    /* Start the VTU Operation by defining the DBNum, vtuOp and VTUBusy    */
    /*
     * Flush operation will skip the above two setup (for data and vid), and
     * come to here directly
    */

    retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR, PRV_CPSS_QD_REG_VTU_OPERATION, &data);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
    }

    data &= 0xc00;
    data |= (1 << 15) | (vtuOp << 12);

    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,data);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
    }

    /* only two operations need to go through the mess below to get some data
    * after the operations -  service violation and get next entry
    */

    /* If the operation is to service violation operation wait for the response   */
    if(vtuOp == SERVICE_VIOLATIONS)
    {
        /* Wait until the VTU in ready. */
        data = 1;
        retryCount = 0;
        while(data == 1)
        {
            retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
                return retVal;
            }
         /* the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
       }

        /* get the VID that was involved in the violation */
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_VID_REG,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        /* Get the vid - bits 0-11 */
        entry->vid   = data & 0xFFF;

        /* Get the page vid - bit 12 */
        entry->vidExInfo.vtuPage  = (GT_U8)((data & 0x2000) >> 13);

    } /* end of service violations */

    /* If the operation is a get next operation wait for the response   */
    if(vtuOp == GET_NEXT_ENTRY)
    {
        entry->vidExInfo.useVIDQPri = GT_FALSE;
        entry->vidExInfo.vidQPri = 0;
        entry->vidExInfo.useVIDFPri = GT_FALSE;
        entry->vidExInfo.vidFPri = 0;
        entry->vidExInfo.vtuPage = 0;

        entry->sid = 0;
        entry->vidPolicy = GT_FALSE;

        /* Wait until the VTU in ready. */
        data = 1;
        retryCount = 0;
        while(data == 1)
        {
            retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
                return retVal;
            }
            /* the number of iterations does not exceed the limit */
            PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
        }


        /****************** get the vid *******************/

        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_VID_REG,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        /* the vid is bits 0-11 */
        entry->vid   = data & 0xFFF;

        entry->vidExInfo.vtuPage = (GT_U8)((data >> 13) & 1);

        /* the vid valid is bits 12 */
        *valid   = (GT_U8)((data >> 12) & 1);

        if (*valid == 0)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return GT_OK;
        }

        /****************** get the SID *******************/
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,(GT_U8)(PRV_CPSS_QD_REG_STU_SID_REG),&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        entry->sid = (GT_U8)(data & 0x3F);

        entry->vidExInfo.dontLearn = (data & 0x8000)>>15;
        entry->vidExInfo.filterUC = (data & 0x4000)>>14;
        entry->vidExInfo.filterBC = (data & 0x2000)>>13;
        entry->vidExInfo.filterMC = (data & 0x1000)>>12;

        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,(GT_U8)(PRV_CPSS_QD_REG_VTU_FID_REG),&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        entry->vidPolicy = (data >> 12) & 0x1;


        entry->DBNum = data & 0xFFF;

        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_DATA1_REG,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        /* get data from data register for ports 0 to 7 */
        entry->memberTagP[0]  =  data & 3 ;
        entry->memberTagP[1]  = (data >> 2) & 3 ;
        entry->memberTagP[2]  = (data >> 4) & 3 ;
        entry->memberTagP[3]  = (data >> 6) & 3 ;
        entry->memberTagP[4]  = (data >> 8) & 3 ;
        entry->memberTagP[5]  = (data >> 10) & 3 ;
        entry->memberTagP[6]  = (data >> 12) & 3 ;
        entry->memberTagP[7]  = (data >> 14) & 3 ;

        /* get data from data register for ports 8 to 10 */
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_VTU_DATA2_REG,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        entry->memberTagP[8]  = data & 3 ;
        entry->memberTagP[9]  = (data >> 2) & 3 ;
        entry->memberTagP[10]  = (data >> 4) & 3 ;

        if (data & 0x8000)
        {
            entry->vidExInfo.useVIDQPri = GT_TRUE;
            entry->vidExInfo.vidQPri = (GT_U8)((data >> 12) & 0x7);
        }
        else
        {
            entry->vidExInfo.useVIDQPri = GT_FALSE;
            entry->vidExInfo.useVIDQPri = 0;
        }

        if (data & 0x800)
        {
            entry->vidExInfo.useVIDFPri = GT_TRUE;
            entry->vidExInfo.vidFPri = (GT_U8)((data >> 8) & 0x7);
        }
        else
        {
            entry->vidExInfo.useVIDFPri = GT_FALSE;
            entry->vidExInfo.useVIDFPri = 0;
        }
    } /* end of get next entry */

    prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
    return GT_OK;
}

