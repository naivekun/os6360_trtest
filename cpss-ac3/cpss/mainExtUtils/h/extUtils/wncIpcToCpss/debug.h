#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

int con_pr(char *fmt, ...);

#define DBG_F_ERR	0x00000001
#define DBG_F_WARN	0x00000002
#define DBG_F_INFO	0x00000004
#define DBG_F_DEFAULT 	(DBG_F_ERR | DBG_F_WARN)

void dbg_set(uint32_t flags);
void dbg_unset(uint32_t flags);
int dbg_isset(uint32_t flags);

#define dbg_pr(flags, fmt, ...) do { \
        if (dbg_isset(flags)) \
                con_pr("[%lu][%s():%d] "fmt, getpid(), \
			__func__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define dbg_err(fmt, ...) \
	dbg_pr(DBG_F_ERR, fmt, ##__VA_ARGS__)

#define dbg_warn(fmt, ...) \
	dbg_pr(DBG_F_WARN, fmt, ##__VA_ARGS__)

#define dbg_info(fmt, ...) \
	dbg_pr(DBG_F_INFO, fmt, ##__VA_ARGS__)

#endif /* __DEBUG_H__ */
