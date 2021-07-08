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
    char fname[128], addrBuf[64];
    int fd;
    int endian_cvt = 0;
    void *vaddr;
    long long map_offset, map_size;
    while ((opt = getopt(argc, argv, "d:r:s:o:lcb")) != -1)
    {
        switch (opt) {
        case 'd':
            {
                unsigned domain,bus,dev,func;
                pciAddr = optarg;
                if (strcmp(optarg, "mem") == 0)
                    break;
                if (sscanf(optarg, "%x:%x:%x.%x", &domain, &bus, &dev, &func) == 4)
                {
                    break;
                }
            }
            sprintf(addrBuf, "0000:%s", optarg);
            pciAddr = addrBuf;
            break;
        case 'r':
            resno = atoi(optarg);
            break;
        case 'c':
            resno = -2;
            break;
        case 's':
            if (sscanf(optarg, "%i", &size) != 1)
                return 1;
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
            fprintf(stderr, "Usage: %s [-l] [-b] -d 02:00.0 [-c] [-r resno] [-s size] [-o offset]\n", argv[0]);
            fprintf(stderr, "    -c   - access PCI config registers\n");
            fprintf(stderr, "    -l   - convert from little-endian\n");
            fprintf(stderr, "    -b   - convert from big-endian\n");
            exit(EXIT_FAILURE);
        }
    }
    if (!size || !pciAddr)
    {
        fprintf(stderr, "no PCI address of zero size\n");
        return 1;
    }
    if (strcmp(pciAddr, "mem") == 0)
        sprintf(fname, "/dev/mem");
    else
    {
        if (resno != -2)
            sprintf(fname, "/sys/bus/pci/devices/%s/resource%d", pciAddr, resno);
        else
            sprintf(fname, "/sys/bus/pci/devices/%s/config", pciAddr);
    }
    if ((fd = open(fname, O_RDWR)) < 0)
    {
        perror("open()");
        return 1;
    }
#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif
#define PAGE_MASK ((long long)((1<<PAGE_SHIFT)-1))
    map_offset = offset & (~PAGE_MASK);
    map_size = (offset & PAGE_MASK) + size;
    if (resno != -2)
        map_size = (map_size + PAGE_MASK) & (~PAGE_MASK);
    if (strcmp(pciAddr, "mem") != 0)
    {
        struct stat st;
        if (fstat(fd, &st) < 0)
        {
            close(fd);
            return 1;
        }
        if (map_offset + map_size > st.st_size)
        {
            fprintf(stderr, "Address/size out of range (>0x%llx)\n", (long long)st.st_size);
            close(fd);
            return 1;
        }
    }
    if (resno == -2)
    {
        vaddr = malloc(map_size);
        lseek(fd, map_offset, SEEK_SET);
        read(fd, vaddr, map_size);
        printf("Dump config of %s\n", pciAddr);
    }
    else
    {
        vaddr = mmap(NULL,
                    map_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    fd,
                    (off_t)map_offset);
        if (MAP_FAILED == vaddr)
        {
            perror("mmap");
            close(fd);
            return 1;
        }
        if (strcmp(pciAddr, "mem") != 0)
            printf("Dump BAR%d of %s\n", resno, pciAddr);
        else
            printf("Dump /dev/mem\n");
    }

    for (i = 0; i < size; i+= 4)
    {
        uint32_t value;
        if ((i % 16) == 0)
            printf("0x%08llx:", offset + (long long)i);
        value = *((volatile uint32_t*)(((uintptr_t)vaddr)+((uintptr_t)offset-(uintptr_t)map_offset)+i));
        if (endian_cvt == 1)
            value = le32toh(value);
        if (endian_cvt == 2)
            value = be32toh(value);
        printf(" 0x%08x", value);
        if (((i+4) % 16) == 0)
            printf("\n");
    }
    if ((i % 16) != 0)
        printf("\n");
    return 0;
}
