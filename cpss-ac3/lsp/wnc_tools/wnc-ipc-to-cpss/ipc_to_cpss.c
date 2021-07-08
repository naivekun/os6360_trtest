#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define LSOCKET_TO_CPSS "/tmp/lsocket_to_cpss"
#define LISTEN_BACKLOG  5
#define STR_BUF         1024

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void show_usage(void) 
{
    printf("Usage:\n");
    printf("NAME\n");
    printf("    ipc_to_cpss - to call CPSS APIs directly via IPC\n");
    printf("\n");
    printf("SYNOPSIS\n");
    printf("    ipc_to_cpss [-m <API_command>] [-h]\n");
    printf("\n");
    printf("DESCRIPTION\n");
    printf("    This command is to call CPSS APIs, not using Marvell lua CLI commands\n");
    printf("    but through an IPC to communicate with appDemo. The called API commands\n");
    printf("    need to follow the format of JSON style, which can refer to the script\n");
    printf("    sdkutils_mrvl.\n");
    printf("\n");
    printf("    Currently this command only supports APIs to set to CPSS, but not\n");
    printf("    support to get information from CPSS.\n");
    printf("\n");
    printf("MANDATORY OPTIONS\n");
    printf("    At least one optional option is needed.\n");
    printf("\n");
    printf("OPTIONAL OPTIONS\n");
    printf("    -m    send API command formed in JSON style to appDemo.\n");
    printf("\n");
    printf("    -h    Display this usage.\n");
    printf("\n");
    printf("EXAMPLE\n");
    printf("    ipc_to_cpss -m \"{\n");
    printf("      \\\"func\\\":\\\"cpssDxChBrgVlanTableInvalidate\\\",\n");
    printf("      \\\"params\\\":[{\\\"dir\\\":\\\"in\\\",\\\"type\\\":\\\"u8\\\",\\\"value\\\":\\\"0\\\"}]\n");
    printf("    }\"\n");
    printf("        : to clear the VLAN table of unit 0.\n");
    printf("    ipc_to_cpss -h\n");
    printf("        : to show this usage.\n");
    printf("\n");
}

int ConnectCpssLocalSocket(void)
{
    int socket_fd;
    struct sockaddr_un name;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        handle_error("socket");
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, LSOCKET_TO_CPSS, sizeof(name.sun_path));
    name.sun_path[strlen(name.sun_path)] = '\0';

    if(connect(socket_fd, (struct sockaddr *)&name, SUN_LEN(&name)) == -1)
    {
        close(socket_fd);
        handle_error("connect");
    }

    return socket_fd;
}

int main(int argc, char **argv)
{
    int srv_socket;
    int ch, ret;
    char mesg_buf[STR_BUF] = {0};
    char ret_mesg[32] = {0};

    while((ch = getopt(argc, argv, "m:h")) != -1)
    {
        switch(ch)
        {
            case 'm':
                snprintf(mesg_buf, sizeof(mesg_buf), "%s", optarg);
                break;
            case 'h':
                show_usage();
                return 0;
            default:
                printf("Unknown option: \"%c\"", ch);
                show_usage();
                return -1;
        }
    }

    srv_socket = ConnectCpssLocalSocket();
    ret = write(srv_socket, mesg_buf, strlen(mesg_buf));
    if(ret < 0)
    {
        close(srv_socket);
        handle_error("write");
    }
    else
    {
        ret = read(srv_socket, ret_mesg, sizeof(ret_mesg));
        if(ret > 0)
        {
            ret_mesg[ret]='\0';
            /* Return value of the called funcion */
            ret = atoi(ret_mesg);
        }
        else
        {
            close(srv_socket);
            handle_error("read");
        }
    }
    close(srv_socket);

    return ret;
}
