/*==========================================================================
 * Module Name : console_debug.c
 * Purpose     : 
 * Author      : 
 * Date        : 
 * Notes       :
 * $Log$
 *==========================================================================
 */
 
/* INCLUDE FILE DECLARATIONS */
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <stdlib.h>
#include <ctype.h>
#if NET_INTERFACE == INTERFACE_SCAN
#include <ifaddrs.h>
#endif
#include "asix-ioctl.h"

/* STATIC VARIABLE DECLARATIONS */
#define AX88179_IOCTL_VERSION		"AX88179/AX88178A Linux Register R/W Tool v0.1.0"

/* LOCAL SUBPROGRAM DECLARATIONS */
static unsigned long STR_TO_U32(const char *cp,char **endp,unsigned int base);


/* LOCAL SUBPROGRAM BODIES */
static void show_usage(void)
{
	int i;
	printf ("Usage:\n");
	for (i = 0; command_list[i].cmd != NULL; i++)
		printf ("%s\n", command_list[i].help_ins);
}

static unsigned long STR_TO_U32(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	/*if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}*/
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;

	return result;
}

void help_func (struct ax_command_info *info)
{
	int i;

	if (info->argv[2] == NULL) {
		for(i=0; command_list[i].cmd != NULL; i++) {
			printf ("%s%s\n", command_list[i].help_ins, command_list[i].help_desc);
		}
	}

	for (i = 0; command_list[i].cmd != NULL; i++)
	{
		if (strncmp(info->argv[1], command_list[i].cmd, strlen(command_list[i].cmd)) == 0 ) {
			printf ("%s%s\n", command_list[i].help_ins, command_list[i].help_desc);
			return;
		}
	}

}

void readreg_func(struct ax_command_info *info)
{
	struct ifreq *ifr = (struct ifreq *)info->ifr;
	AX_IOCTL_COMMAND ioctl_cmd;
	unsigned short len;
	unsigned short offset = 0;
	int i;	
	unsigned char type = 0;

	if (info->argc < 2) {
		for(i=0; command_list[i].cmd != NULL; i++) {
			if (strncmp(info->argv[1], command_list[i].cmd, 
					strlen(command_list[i].cmd)) == 0 ) {
				printf ("%s%s\n", command_list[i].help_ins, 
						command_list[i].help_desc);
				return;
			}
		}
	}	

	offset = 0;
	len = 0;	

	ioctl_cmd.ioctl_cmd = info->ioctl_cmd;
	ioctl_cmd.size = len;
	ioctl_cmd.buf = NULL;
	ioctl_cmd.offset = offset & 0xFF;
	ioctl_cmd.delay = 0;

	ifr->ifr_data = (caddr_t)&ioctl_cmd;

	if (ioctl(info->inet_sock, AX_PRIVATE, ifr) < 0) {
		perror("ioctl");
		return;
	}	
	
	printf("read completely\n");
	return;
}

void writereg_func(struct ax_command_info *info)
{
	struct ifreq *ifr = (struct ifreq *)info->ifr;
	AX_IOCTL_COMMAND ioctl_cmd;
	int i;
	unsigned char *buf;
	unsigned short data;
	char c[2] = {'\0'};
	unsigned char retried = 0;
	unsigned short offset = 0;
	unsigned char type = 0;

	if (info->argc != 4) {
		for(i=0; command_list[i].cmd != NULL; i++) {
			if (strncmp(info->argv[1], command_list[i].cmd,
					strlen(command_list[i].cmd)) == 0) {
				printf ("%s%s\n", command_list[i].help_ins,
						command_list[i].help_desc);
				return;
			}
		}
	}

	offset = STR_TO_U32(info->argv[2], NULL, 16);
	data = STR_TO_U32(info->argv[3], NULL, 16);

	if ((offset > 255) || 
	    (data > 255) ||
	    (offset == 0)) {
		for(i=0; command_list[i].cmd != NULL; i++) {
			if (strncmp(info->argv[1], command_list[i].cmd, 
					strlen(command_list[i].cmd)) == 0 ) {
				printf ("%s%s\n", command_list[i].help_ins, 
						command_list[i].help_desc);
				return;
			}
		}
	}

	buf = (unsigned char *)malloc(sizeof(unsigned char));
	*buf = data & 0xFF;

	ioctl_cmd.ioctl_cmd = info->ioctl_cmd;
	ioctl_cmd.size = 1;
	ioctl_cmd.buf = buf;
	ioctl_cmd.offset = offset & 0xFF;

	ifr->ifr_data = (caddr_t)&ioctl_cmd;

  	if (ioctl(info->inet_sock, AX_PRIVATE, ifr) < 0) {
		free(buf);
		perror("ioctl");
		return;
	}

	printf("Write completely\n");
	free(buf);
	return;
}

/* EXPORTED SUBPROGRAM BODIES */
int main(int argc, char **argv)
{
	int inet_sock;
	struct ifreq ifr;
	char buf[0xff];
	struct ax_command_info info;
	unsigned char i;
	char	input;
	unsigned char count = 0;
	int fd, console;
	const unsigned char length = sizeof(char);
	AX_IOCTL_COMMAND ioctl_cmd;
#if NET_INTERFACE == INTERFACE_SCAN
	struct ifaddrs *addrs, *tmp;
	unsigned char	dev_exist;
#endif

	printf ("\n%s\n",AX88179_IOCTL_VERSION);

	if (argc < 2) {
		show_usage();
		return 0;
	}

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
#if NET_INTERFACE == INTERFACE_SCAN
	/* Get Device */
	getifaddrs(&addrs);
	tmp = addrs;
	dev_exist = 0;

	while (tmp) {
		memset (&ioctl_cmd, 0, sizeof (AX_IOCTL_COMMAND));
		ioctl_cmd.ioctl_cmd = AX_SIGNATURE;
		// get network interface name
		sprintf (ifr.ifr_name, "%s", tmp->ifa_name);

		ifr.ifr_data = (caddr_t)&ioctl_cmd;
		tmp = tmp->ifa_next;


		if (ioctl (inet_sock, AX_PRIVATE, &ifr) < 0)			
			continue;
		
		
		if (strncmp (ioctl_cmd.sig, AX8817XX_DRV_NAME, strlen(AX8817XX_DRV_NAME)) == 0 ) {
			dev_exist = 1;
			break;
		}			
	}

	freeifaddrs(addrs);

	if (dev_exist == 0) {
		printf ("No %s found\n", AX8817XX_SIGNATURE);
		return 0;
	}
#else	
	for (i = 0; i < 255; i++) {

		memset (&ioctl_cmd, 0, sizeof (AX_IOCTL_COMMAND));
		ioctl_cmd.ioctl_cmd = AX_SIGNATURE;

		sprintf (ifr.ifr_name, "eth%d", i);
		
		ifr.ifr_data = (caddr_t)&ioctl_cmd;

		if (ioctl (inet_sock, AX_PRIVATE, &ifr) < 0)
			continue;

		if (strncmp (ioctl_cmd.sig, AX8817XX_DRV_NAME, strlen(AX8817XX_DRV_NAME)) == 0 )
			break;

	}

	if (i == 255) {
		printf ("No %s found\n", AX8817XX_SIGNATURE);
		return 0;
	}
#endif
	for(i=0; command_list[i].cmd != NULL; i++)
	{
		if (strncmp(argv[1], command_list[i].cmd, strlen(command_list[i].cmd)) == 0 ) {
			info.help_ins = command_list[i].help_ins;
			info.help_desc = command_list[i].help_desc;
			info.ifr = &ifr;
			info.argc = argc;
			info.argv = argv;
			info.inet_sock = inet_sock;
			info.ioctl_cmd = command_list[i].ioctl_cmd;
			(command_list[i].OptFunc)(&info);
			return 0;
		}
	}
	printf ("Wrong command\n");

	return 0;
}

