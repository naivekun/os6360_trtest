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

void Usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s -d 02:00.0 [-r resno] [-o offset] [-v value]\n", argv0);
    exit(EXIT_FAILURE);
}
int main(int argc, char * const * argv)
{
    int opt;
    char *pciAddr = NULL;
    int   resno = -1;
    long long value = 0;
    long long offset = -1;
    char fname[128];
    int fd;
    struct stat st;
    void *vaddr;
    while ((opt = getopt(argc, argv, "d:r:v:o:")) != -1)
    {
        switch (opt) {
        case 'd':
            pciAddr = optarg;
            break;
        case 'r':
            resno = atoi(optarg);
            break;
        case 'v':
            if (sscanf(optarg, "%Li", &value) != 1)
                return 1;
            break;
        case 'o':
            if (sscanf(optarg, "%Li", &offset) != 1)
                return 1;
            break;
        default: /* '?' */
            Usage(argv[0]);
        }
    }
    if (!pciAddr || resno == -1 || offset == -1)
    {
        Usage(argv[0]);
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
    printf("%s resource %d mapped to %p, size=0x%x\n", pciAddr, resno, vaddr, (unsigned)st.st_size);
    
    printf("Write 0x%08x to %p\n", (uint32_t)value, (void*)(((uintptr_t)vaddr)+((uintptr_t)offset)));
    *((volatile uint32_t*)(((uintptr_t)vaddr)+((uintptr_t)offset))) = (uint32_t) value;
    return 0;
}
