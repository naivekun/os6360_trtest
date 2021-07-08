/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsSharedIPC.h>


#ifdef SHARED_MEMORY


/******** implementation **********************************/
typedef struct {
    GT_SEM      semMutex;
    GT_SEM      semCommand;
    GT_SEM      semDone;
    enum {
        STATE_NONE_E,
        STATE_WAIT_E,
        STATE_INPROGRESS_E
    }           state;
    GT_STATUS   rc;
    CPSS_MP_REMOTE_COMMAND command;
} CPSS_MP_REMOTE;

static CPSS_MP_REMOTE remoteArray[CPSS_MP_REMOTE_MAX_E]; /* will be initialized with zeroes */
const char* remoteNames[CPSS_MP_REMOTE_MAX_E] = {
    "RxTx",
    "FdbLearning"
};

/**
* @internal cpssMultiProcComExecute function
* @endinternal
*
* @brief   Used to execute function in another process
*
* @param[in] paramPtr                 - pointer to struct CPSS_MP_REMOTE_COMMAND
*/
GT_STATUS cpssMultiProcComExecute
(
	CPSS_MP_REMOTE_COMMAND *paramPtr
)
{
    CPSS_MP_REMOTE  *mp;
    GT_STATUS       rc;

    /* check parameters */
    if (!paramPtr)
        return GT_BAD_PARAM;

    if (paramPtr->remote >= CPSS_MP_REMOTE_MAX_E)
        return GT_BAD_VALUE;

    mp = &(remoteArray[paramPtr->remote]);

    if (!mp->semMutex) {
        /* not initialized yet.
         * application not started/doesn't listen for commands
         */
        return GT_NOT_INITIALIZED;
    }

    /* Only one command at time for each application */
    osSemWait(mp->semMutex, OS_WAIT_FOREVER);

    /* store command parameters */
    osMemCpy(&(mp->command), paramPtr, sizeof(CPSS_MP_REMOTE_COMMAND));
    /* signal to application */
    osSemSignal(mp->semCommand);
    /* wait till command done */
    osSemWait(mp->semDone, OS_WAIT_FOREVER);
    /* handle timeout/failure */

    /* Command result */
    rc = mp->rc;
    osMemCpy(paramPtr, &(mp->command), sizeof(CPSS_MP_REMOTE_COMMAND));

    osSemSignal(mp->semMutex);

    return rc;
}

/**
* @internal cpssMultiProcComWait function
* @endinternal
*
* @brief   Used to to wait for command in another process
*
* @param[in] program                  -  identifier which waits for command
*
* @param[out] paramPtr                 - pointer to struct CPSS_MP_REMOTE_COMMAND
*/
GT_STATUS cpssMultiProcComWait
(
    IN  CPSS_MP_REMOTE_DEST    program,
	OUT CPSS_MP_REMOTE_COMMAND *paramPtr
)
{
    CPSS_MP_REMOTE *mp;

    /* check parameters */
    if (program >= CPSS_MP_REMOTE_MAX_E)
        return GT_BAD_PARAM;

    if (paramPtr == NULL)
        return GT_BAD_PARAM;

    mp = &(remoteArray[program]);
    if (!mp->semMutex) {
        /* not initialized yet */
        char name[64];

        osSprintf(name, "MP_%s_Cmd", remoteNames[program]);
        if (osSemBinCreate(name, OS_SEMB_EMPTY, &(mp->semCommand)) != GT_OK)
        {
            return GT_FAIL;
        }
        osSprintf(name, "MP_%s_Done", remoteNames[program]);
        if (osSemBinCreate(name, OS_SEMB_EMPTY, &(mp->semDone)) != GT_OK)
        {
            osSemDelete(mp->semCommand);
            return GT_FAIL;
        }
        osSprintf(name, "MP_%s_Mtx", remoteNames[program]);
        if (osSemBinCreate(name, OS_SEMB_FULL, &(mp->semMutex)) != GT_OK)
        {
            osSemDelete(mp->semCommand);
            osSemDelete(mp->semDone);
            return GT_FAIL;
        }
        mp->state = STATE_NONE_E;
    }

    mp->rc = GT_OK;
    mp->state = STATE_WAIT_E;
    /* wait for command */
    osSemWait(mp->semCommand, OS_WAIT_FOREVER);
    /* handle fail & timeout */

    /* semaphore signalled, parameters available */
    osMemCpy(paramPtr, &(mp->command), sizeof(CPSS_MP_REMOTE_COMMAND));

    mp->state = STATE_INPROGRESS_E;

    return GT_OK;
}


/**
* @internal cpssMultiProcComComplete function
* @endinternal
*
* @brief   Used to confirm remote command finished
*
* @param[in] paramPtr                 - pointer to struct CPSS_MP_REMOTE_COMMAND
* @param[in] rc                       - return code
*/
GT_STATUS cpssMultiProcComComplete
(
	IN  CPSS_MP_REMOTE_COMMAND *paramPtr,
	IN  GT_STATUS              rc
)
{
    CPSS_MP_REMOTE *mp;

    /* check parameters */
    if (!paramPtr)
        return GT_BAD_PARAM;

    if (paramPtr->remote >= CPSS_MP_REMOTE_MAX_E)
        return GT_BAD_VALUE;

    mp = &(remoteArray[paramPtr->remote]);
    if (!mp->semMutex) {
        /* not initialized yet, bad usage */
        return GT_NOT_INITIALIZED;
    }
    if (mp->state != STATE_INPROGRESS_E)
        return GT_FAIL;

    /* copy return values */
    osMemCpy(&(mp->command), paramPtr, sizeof(CPSS_MP_REMOTE_COMMAND));
    mp->rc = rc;
    mp->state = STATE_NONE_E;

    /* signal to caller */
    osSemSignal(mp->semDone);
    return GT_OK;
}

#endif



