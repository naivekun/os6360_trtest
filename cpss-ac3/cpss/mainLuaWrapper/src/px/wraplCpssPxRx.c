/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxRx.c
*
* DESCRIPTION:
*       Cpss PX RX handler
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtOs/gtOsMsgQ.h>
#include <mainLuaWrapper/wraplNetIf.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>


/************* Externs *******************************************************/


/************* Locals ********************************************************/
static GT_BOOL luaPxRxCallbackRegistered = GT_FALSE;
static GT_BOOL luaPxRxConfigured = GT_FALSE;
static GT_MSGQ_ID luaPxRxMsgQ = 0;
typedef GT_STATUS (*appDemoDbEntryGet_TYPE)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);
extern appDemoDbEntryGet_TYPE appDemoDbEntryGet_func;

/*******************************************************************************
* luaPxPktReceive
*
* DESCRIPTION:
*       Function called to handle incoming Rx packet in the CPU
*
* APPLICABLE DEVICES: Pipe
*
* INPUTS:
*       devNum      - Device number.
*       queueIdx    - The queue from which this packet was received.
*       numOfBuffPtr - Num of used buffs in packetBuffs.
*       packetBuffs  - The received packet buffers list.
*       buffLen      - List of buffer lengths for packetBuffs.
*       rxParamsPtr  - (Pointer to)information parameters of received packets
*
* RETURNS:
*       GT_OK                    - no error
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS luaPxPktReceive
(
    IN GT_UINTPTR                cookie,
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8      devNum,
    IN GT_U8      queueIdx,
    IN GT_U32     numOfBuff,
    IN GT_U8     *packetBuffs[],
    IN GT_U32     buffLen[],
    IN GT_VOID   *rxParamsVoidPtr
)
{
    PX_RX_DATA_STC   rxData;
    GT_U32        ii;

    queueIdx = 0;
    rxParamsVoidPtr = NULL;/* Do nothing */

    /* check luaPxRx Configured */
    if (luaPxRxConfigured != GT_TRUE)
        return GT_OK;

    cmdOsMemSet(&rxData, 0, sizeof(rxData));

    /* copy packet info */
    rxData.devNum = devNum;

    /* copy packet data */
    for(ii = 0; ii < numOfBuff; ii++)
    {
        if (rxData.packet.len + buffLen[ii] > PACKET_MAX_SIZE)
        {
            /* packet length is bigger than destination buffer size */
            cmdOsMemCpy(rxData.packet.data + rxData.packet.len, packetBuffs[ii], PACKET_MAX_SIZE - rxData.packet.len);
            rxData.packet.len = PACKET_MAX_SIZE;
            break;
        }
        cmdOsMemCpy(rxData.packet.data + rxData.packet.len, packetBuffs[ii], buffLen[ii]);
        rxData.packet.len += buffLen[ii];
    }

    /* now send structure to MsgQ */
    osMsgQSend(luaPxRxMsgQ, &rxData, sizeof(rxData), OS_MSGQ_NO_WAIT);
    return GT_OK;
}

/*******************************************************************************
* luaPxRxConfigure
*
* DESCRIPTION:
*       Configure a CB function to send packet info/data to msgQ (first call only)
*       register a CB function to be called on every RX packet to CPU
*
* INPUTS:
*       msgqId   - message queue Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*
*
* COMMENTS:
*******************************************************************************/
GT_STATUS luaPxRxConfigure
(
    IN GT_MSGQ_ID msgqId
)
{
    luaPxRxMsgQ = msgqId;
    luaPxRxConfigured = GT_TRUE;
    if (luaPxRxCallbackRegistered == GT_FALSE)
    {
        rxEventHandlerAddCallback(
                RX_EV_HANDLER_DEVNUM_ALL,
                RX_EV_HANDLER_QUEUE_ALL,
                RX_EV_HANDLER_TYPE_ANY_E,
                luaPxPktReceive,
                0);
        luaPxRxCallbackRegistered = GT_TRUE;
    }
    return GT_OK;
}

int wrlPxRxConfigure(lua_State *L)
{
    GT_MSGQ_ID msgqId;
    GT_STATUS rc = GT_OK;
    PARAM_NUMBER(rc, msgqId, 1, GT_MSGQ_ID);
    if (rc == GT_OK)
    {
        rc = luaPxRxConfigure(msgqId);
    }
    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}

/*******************************************************************************
* luaPxRxDisable
*
* DESCRIPTION:
*       Disable RX callback handler
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
*
* COMMENTS:
*******************************************************************************/
GT_STATUS luaPxRxDisable(void)
{
    luaPxRxConfigured = GT_FALSE;
    return GT_OK;
}

int wrlPxRxDisable(lua_State *L)
{
    GT_STATUS rc = GT_OK;
    rc = luaPxRxDisable();
    lua_pushinteger(L, (lua_Integer)rc);
    return 1;
}
