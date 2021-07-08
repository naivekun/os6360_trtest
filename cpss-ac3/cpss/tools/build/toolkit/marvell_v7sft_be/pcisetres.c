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
    fprintf(stderr, "Usage: %s -d 02:00.0 [-c] [-r resno] [-o offset] [-v value]\n", argv0);
    fprintf(stderr, "    -c   - access PCI config registers\n");
    fprintf(stderr, "    -l   - convert to little-endian\n");
    fprintf(stderr, "    -b   - convert to big-endian\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char * const * argv)
{
    int opt;
    char *pciAddr = NULL;
    int   resno = -1;
    long long value = 0;
    uint32_t value32;
    long long offset = -1;
    char fname[128];
    int fd;
    int endian_cvt = 0;
    struct stat st;
    void *vaddr;
    while ((opt = getopt(argc, argv, "d:r:cv:o:lb")) != -1)
    {
        switch (opt) {
        case 'd':
            pciAddr = optarg;
            break;
        case 'r':
            resno = atoi(optarg);
            break;
        case 'c':
            resno = -2;
            break;
        case 'v':
            if (sscanf(optarg, "%Li", &value) != 1)
                return 1;
            value32 = (uint32_t)value;
            break;
        case 'o':
            if (sscanf(optarg, "%Li", &offset) != 1)
                return 1;
            break;
        case 'l':
            endian_cvt = 1;
            break;
        case 'b':
            endian_cvt = 2;
            break;
        default: /* '?' */
            Usage(argv[0]);
        }
    }
    if (!pciAddr || resno == -1 || offset == -1)
    {
        Usage(argv[0]);
    }
    if (resno != -2)
        sprintf(fname, "/sys/bus/pci/devices/0000:%s/resource%d", pciAddr, resno);
    else
        sprintf(fname, "/sys/bus/pci/devices/0000:%s/config", pciAddr);
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
    if (resno != -2)
    {
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
        printf("Write 0x%08x to %p\n", value32, (void*)(((uintptr_t)vaddr)+((uintptr_t)offset)));
        if (endian_cvt == 1)
            value32 = htole32(value32);
        if (endian_cvt == 2)
            value32 = htobe32(value32);
        *((volatile uint32_t*)(((uintptr_t)vaddr)+((uintptr_t)offset))) = value32;
    }
    else
    {
        printf("Write 0x%08x to offset 0x%llx\n", value32, offset);
        if (endian_cvt == 1)
            value32 = htole32(value32);
        if (endian_cvt == 2)
            value32 = htobe32(value32);
        lseek(fd, offset, SEEK_SET);
        write(fd, &value32, 4);
    }
    return 0;
}
