#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <inttypes.h>

int main(int argc, char * const * argv)
{
    int opt;
    char *pciAddr = NULL;
    int   resno = 0;
    int   size = 64;
    long long offset = 0;
    int   i;
    char fname[128];
    int fd;
    struct stat st;
    void *vaddr;
    while ((opt = getopt(argc, argv, "d:r:s:o:")) != -1)
    {
        switch (opt) {
        case 'd':
            pciAddr = optarg;
            break;
        case 'r':
            resno = atoi(optarg);
            break;
        case 's':
            if (sscanf(optarg, "%i", &size) != 1)
                return 1;
            break;
        case 'o':
            if (sscanf(optarg, "%Li", &offset) != 1)
                return 1;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -d 02:00.0 [-r resno] [-s size] [-o offset]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (!size || !pciAddr)
    {
        fprintf(stderr, "no PCI address of zero size\n");
        return 1;
    }
    sprintf(fname, "/sys/bus/pci/devices/0000:%s/resource%d", pciAddr, resno);
    if ((fd = open(fname, O_RDWR)) < 0)
    {
        perror("open()");
        return 1;
    }
    if (fstat(fd, &st) < 0)
    {
        close(fd);
        return 1;
    }
    vaddr = mmap(NULL,
                st.st_size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                (off_t)0);
    if (MAP_FAILED == vaddr)
    {
        perror("mmap");
        close(fd);
        return 1;
    }
    
    printf("Dump BAR%d of %s\n", resno, pciAddr);
    for (i = 0; i < size; i+= 4)
    {
        if ((i % 16) == 0)
            printf("0x%08x:", (uint32_t)offset + i);
        printf(" 0x%08x", *((volatile uint32_t*)(((uintptr_t)vaddr)+((uintptr_t)offset)+i)));
        if (((i+4) % 16) == 0)
            printf("\n");
    }
    if ((i % 16) != 0)
        printf("\n");
    return 0;
}
