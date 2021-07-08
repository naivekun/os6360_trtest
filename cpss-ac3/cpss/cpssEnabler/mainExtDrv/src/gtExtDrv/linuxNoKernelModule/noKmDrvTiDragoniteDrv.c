/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE


#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/ipc/mvShmIpc.h>

#undef  DRAGONITE_DEBUG
#ifdef  DRAGONITE_DEBUG
#define dragonite_debug_printf(x, ...) printf(x "\n", ##__VA_ARGS__)
#else
#define dragonite_debug_printf(x, ...)
#endif


#define EXTHWG_POE_DRG_Ipc_DTCM_ACTUAL_SIZE         (32*_1K)
#define EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_SIZE  (2*_1K)
#define EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_OFFSET        (EXTHWG_POE_DRG_Ipc_DTCM_ACTUAL_SIZE - EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_SIZE)
#define EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_TIMEOUT_uSec   10000
#define EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_NUM_OF_RETRIES 10

struct EXTHWG_POE_DRG_Ipc_TX_MESSAGE {
    GT_32     opcode;
    GT_32     data_length;
    GT_U8     data[EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_TX_DATA_MSG_SIZE];
};

struct EXTHWG_POE_DRG_Ipc_RX_MESSAGE {
    GT_32     opcode;
    GT_32     status;
    GT_32     data_length;
    GT_U8     data[EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_RX_DATA_MSG_SIZE];
};

static IPC_SHM_STC EXTHWG_POE_DRG_Ipc_shm;

/*      IPC_SHM_SYNC_FUNC_MODE_READ_E
*                          - Direct read from target's absolute address
*                            ptr is pointer to store data to
*                            targetPtr is target's address
*                            size is data size to read
*      IPC_SHM_SYNC_FUNC_MODE_WRITE_E
*                          - Direct write to target's absolute address
*                            ptr is pointer to data
*                            targetPtr is target's address
*                            size is data size to write
*/
static void directSyncFunc(
    IN  void*   cookie,
    IN  IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    IN  void*   ptr,
    IN  IPC_UINTPTR_T targetPtr,
    IN  IPC_U32 size
)
{
    IPC_U32 *srcPtr, *dstPtr;
    /* printf("mode %d ptr: %p targetPtr 0x%x\n", mode, ptr, targetPtr); */

    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E) {
        srcPtr = (IPC_U32*)targetPtr;
        dstPtr = (IPC_U32*)ptr;
    } else {
        srcPtr = (IPC_U32*)ptr;
        dstPtr = (IPC_U32*)targetPtr;
    }

    for (; (int)size>0; size-=4, srcPtr++, dstPtr++)
        *dstPtr = *srcPtr;
}

/**
* @internal EXTHWG_POE_DRG_Ipc_driverInit function
* @endinternal
*
* @brief   Initialize the IPC messaging driver that communicates with the
*          Dragonite
*
* @param[in] hwInfoPtr            - pointer to a structure contains HW addresses
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/

GT_STATUS EXTHWG_POE_DRG_Ipc_driverInit(
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS rc;

    rc = shmIpcInit(
        &EXTHWG_POE_DRG_Ipc_shm,
        (void *)(hwInfoPtr->resource.dragonite.dtcm.start + EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_OFFSET),
        EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_SIZE,
        1, /* master */
        directSyncFunc,
        NULL);

    dragonite_debug_printf(
        "IPC init params: Base %#x\n area offset = actual size - area size: %#x = %#x - %#x",
        hwInfoPtr->resource.dragonite.dtcm.start,
        EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_OFFSET, EXTHWG_POE_DRG_Ipc_DTCM_ACTUAL_SIZE, EXTHWG_POE_DRG_Ipc_COMMUNICATION_AREA_SIZE);

    /* Create channel #1 with 1 buffer for RX and for TX */
    rc = shmIpcConfigChannel(
        &EXTHWG_POE_DRG_Ipc_shm,
        1, /* chn */
        1, EXTHWG_POE_DRG_Ipc_MAX_MSG_SIZE,   /* rx */
        1, EXTHWG_POE_DRG_Ipc_MAX_MSG_SIZE);  /* tx */
    if (rc) {
        dragonite_debug_printf("Failed to configure IPC Channel: %d", rc);
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal EXTHWG_POE_DRG_Ipc_MsgRead function
* @endinternal
*
* @brief   Dragonite TI message read.
*
* @param[in] msg_opcode               - Opcode of requested msg
* @param[in] msg_request_len          -
* @param[in] msg_request_PTR          - Buffer of max length
*       DRAGONITE_DTCM_IPC_MAX_INTERNAL_TX_DATA_MSG_SIZE
*
* @param[out] msg_response_len_PTR    - actual msg len read
* @param[out] msg_response            - the message that was read
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/

GT_STATUS EXTHWG_POE_DRG_Ipc_MsgRead(
    IN   GT_32 msg_opcode,
    IN   GT_32 msg_request_len,
    IN   GT_U8 *msg_request_PTR, /* Buffer of max length DRAGONITE_DTCM_IPC_MAX_INTERNAL_TX_DATA_MSG_SIZE */
    OUT  GT_32 *msg_response_len_PTR,
    OUT  GT_U8 msg_response[EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_RX_DATA_MSG_SIZE]
)
{
    struct EXTHWG_POE_DRG_Ipc_TX_MESSAGE tx_msg;
    struct EXTHWG_POE_DRG_Ipc_RX_MESSAGE rx_msg;
    int size, num_of_retries, send_res;
    struct timespec time_before, time_after;

    /* Send Get message */
    tx_msg.opcode = msg_opcode;
    tx_msg.data_length = msg_request_len;
    if (msg_request_len > EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_TX_DATA_MSG_SIZE)
        return GT_FAIL;
    memcpy(tx_msg.data, msg_request_PTR, msg_request_len);

    clock_gettime(CLOCK_MONOTONIC, &time_before);

    send_res = shmIpcSend(&EXTHWG_POE_DRG_Ipc_shm, 1, &tx_msg, EXTHWG_POE_DRG_Ipc_TX_HEADER_SIZE + tx_msg.data_length/*DRAGONITE_DTCM_IPC_MAX_MSG_SIZE*/);
    if (send_res != 0) {
        dragonite_debug_printf("Failed to send message with opcode %#x", msg_opcode);
        return GT_FAIL;
    }

    /* Wait for response */
    size = EXTHWG_POE_DRG_Ipc_MAX_MSG_SIZE;
    for (num_of_retries = 0; num_of_retries < EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_NUM_OF_RETRIES; num_of_retries++) {
        usleep(EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_TIMEOUT_uSec);
        if (shmIpcRxChnReady(&EXTHWG_POE_DRG_Ipc_shm, 1)) {
            clock_gettime(CLOCK_MONOTONIC, &time_after);
            dragonite_debug_printf("IPC msg_opcode [%#x] took time in ms: %d, number of tries %d",
                msg_opcode, (time_after.tv_nsec - time_before.tv_nsec)/1000000, num_of_retries+1);
            if (shmIpcRecv(&EXTHWG_POE_DRG_Ipc_shm, 1, &rx_msg, &size)==1) {
                /* Got a response. First make sure that the response matches the requested opcode */
                if (rx_msg.opcode != msg_opcode) {
                    dragonite_debug_printf("Received message with opcode %#x, different from requested opcode %#x",
                                           rx_msg.opcode, msg_opcode);
                    return GT_FAIL;
                }
                if (rx_msg.status == GT_FAIL) {
                    dragonite_debug_printf("Status of received message is 'failed'");
                    return GT_FAIL;
                }
                if (rx_msg.data_length > EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_RX_DATA_MSG_SIZE) {
                    dragonite_debug_printf("Received message is too long");
                    return GT_FAIL;
                }

                /* Got a good response - copy the message */
                *msg_response_len_PTR = rx_msg.data_length;
                memcpy(msg_response, rx_msg.data, rx_msg.data_length);
                return GT_OK;

            }
        }
    }

    dragonite_debug_printf("Exceeded number of tries %d", EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_NUM_OF_RETRIES);

    return GT_FAIL;
}

/**
* @internal EXTHWG_POE_DRG_Ipc_MsgWrite function
* @endinternal
*
* @brief   Dragonite TI message write.
*
* @param[in] msg_opcode               - Opcode of requested msg
* @param[in] msg_request_len          -
* @param[in] msg_request_PTR          - Buffer of max length
*       DRAGONITE_DTCM_IPC_MAX_INTERNAL_TX_DATA_MSG_SIZE
*

* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/

GT_STATUS EXTHWG_POE_DRG_Ipc_MsgWrite(
    IN   GT_32 msg_opcode,
    IN   GT_32 msg_request_len,
    IN   GT_U8 *msg_request_PTR, /* Buffer with max length DRAGONITE_DTCM_IPC_MAX_INTERNAL_TX_DATA_MSG_SIZE */
    IN   GT_BOOL is_debug
    )
{
    struct EXTHWG_POE_DRG_Ipc_TX_MESSAGE tx_msg;
    struct EXTHWG_POE_DRG_Ipc_RX_MESSAGE rx_msg;
    int size, num_of_retries, send_res;
    struct timespec time_before, time_after;

    /* Send Set message */
    tx_msg.opcode = msg_opcode;
    tx_msg.data_length = msg_request_len;
    if (msg_request_len > EXTHWG_POE_DRG_Ipc_MAX_INTERNAL_TX_DATA_MSG_SIZE)
        return GT_FAIL;
    memcpy(tx_msg.data, msg_request_PTR, msg_request_len);

    clock_gettime(CLOCK_MONOTONIC, &time_before);

    send_res = shmIpcSend(&EXTHWG_POE_DRG_Ipc_shm, 1, &tx_msg, EXTHWG_POE_DRG_Ipc_TX_HEADER_SIZE + tx_msg.data_length/*DRAGONITE_DTCM_IPC_MAX_MSG_SIZE*/);
    dragonite_debug_printf("IPC params: header size %#x, data length %#x\n", EXTHWG_POE_DRG_Ipc_TX_HEADER_SIZE, tx_msg.data_length);
    if (send_res != 0)
    {
        dragonite_debug_printf("Failed to send message with opcode %#x\n", msg_opcode);
        return GT_FAIL;
    }

    /* Wait for response */
    size = EXTHWG_POE_DRG_Ipc_MAX_MSG_SIZE;
    for (num_of_retries = 0; num_of_retries < EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_NUM_OF_RETRIES; num_of_retries++) {
        usleep(EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_TIMEOUT_uSec);
        if (shmIpcRxChnReady(&EXTHWG_POE_DRG_Ipc_shm, 1)) {
            clock_gettime(CLOCK_MONOTONIC, &time_after);
            dragonite_debug_printf("IPC msg_opcode [%#x] took time in ms: %d, number of tries %d",
                msg_opcode, (time_after.tv_nsec - time_before.tv_nsec)/ 1000000, num_of_retries+1);
            if (shmIpcRecv(&EXTHWG_POE_DRG_Ipc_shm, 1, &rx_msg, &size)==1) {
                /* Got a response. First make sure that the response matches the requested opcode */
                if (rx_msg.opcode != msg_opcode) {
                    dragonite_debug_printf("Received message with opcode %#x, different from requested opcode %#x",
                        rx_msg.opcode, msg_opcode);
                    return GT_FAIL;
                }
                if (rx_msg.status == GT_FAIL) {
                    dragonite_debug_printf("Status of received message is 'failed'");
                    return GT_FAIL;
                }

                /* Got a good response, for debug : copy the data (not needed) */
                if (is_debug==GT_TRUE)
                    memcpy(msg_request_PTR, rx_msg.data, rx_msg.data_length);
                return GT_OK;
            }
        }
    }

    dragonite_debug_printf("Exceeded number of tries %d", EXTHWG_POE_DRG_Ipc_WAIT_FOR_RESPONSE_NUM_OF_RETRIES);

    return GT_FAIL;
}


