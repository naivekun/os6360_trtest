#ifndef __appDemoWncUtilsTfgenh
#define __appDemoWncUtilsTfgenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif

/* File path of link status output for TfGen */
#define TFGEN_LINK_STAT "/tmp/tfgen_link_status"

/* For Tx fixed packet number */
#define TFGEN_TX_FOLDER         "/cmdFS/ForTfGen"
#define TFGEN_TX_DONE_FILE      "/tmp/tfgen_fixed_pkt_num"
#define TFGEN_TX_BASE           2000
#define TFGEN_THREAD_PER_FILE   10

typedef struct{
    GT_U8   dev_num;
    GT_U32  port_num;
    GT_CHAR file_path[64];
    GT_U32  burst_base;
    GT_U32  send_cnt;
} WNC_UTILS_TFGEN_SEND_PORT_FIXED_NUM_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoWncUtilsTfgenh */
