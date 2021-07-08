#include <stdio.h>
#include <stdarg.h>
#include <extUtils/wncIpcToCpss/debug.h>

int con_pr(char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);

	return ret;
}

static uint32_t dbg_flags = DBG_F_DEFAULT;

void dbg_set(uint32_t flags)
{
	dbg_flags |= flags;
}

void dbg_unset(uint32_t flags)
{
	dbg_flags &= ~flags;
}

int dbg_isset(uint32_t flags)
{
	return (dbg_flags & flags);
}
