#ifndef _ASIX_IOCTL_H_
#define _ASIX_IOCTL_H_

/* INCLUDE FILE DECLARATIONS */
#include "command.h"

/* CHANGE NETWORK INTERFACE WAY */
// DEFAULT_SCAN   : scan "eth0" - "eth255"
// INTERFACE_SCAN : scan all available network interfaces
#define NET_INTERFACE	DEFAULT_SCAN
#define	DEFAULT_SCAN	0x00
#define	INTERFACE_SCAN	0x01

/* NAMING CONSTANT DECLARATIONS */

struct ax_command_info { 
	int inet_sock;
	struct ifreq *ifr;
	int argc;
	char **argv;
	unsigned short ioctl_cmd;
	const char *help_ins;
	const char *help_desc;
};

const char help_str1[] =
"./asix-ioctl help [command]\n"
"    -- command description\n";
const char help_str2[] =
"        [command] - Display usage of specified command\n";

const char readreg_str1[] =
"./asix-ioctl rreg\n"
"    -- AX88179_178A Register read tool\n";
const char readreg_str2[] =
"\n";

const char writereg_str1[] =
"./asix-ioctl wreg [offset] [data]\n"
"    -- AX88179_178A Register write tool\n";
const char writereg_str2[] =
"        [offset]	- Register offset (HEX)\n"
"        [data]		- Register data (HEX)\n";

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void help_func (struct ax_command_info *info);
void readreg_func(struct ax_command_info *info);
void writereg_func(struct ax_command_info *info);
/* TYPE DECLARATIONS */

typedef void (*MENU_FUNC)(struct ax_command_info *info);

struct {
	char *cmd;
	unsigned short ioctl_cmd;
	MENU_FUNC OptFunc;
	const char *help_ins;
	const char *help_desc;
} command_list[] = {
	{"help",	AX_SIGNATURE,	help_func,	help_str1,	help_str2},
	{"rreg",	AX_READ_REG,	readreg_func,	readreg_str1,	readreg_str2},
	{"wreg", 	AX_WRITE_REG, 	writereg_func,	writereg_str1,	writereg_str2},
	{NULL},
};

#endif /* End of console_debug_h */
