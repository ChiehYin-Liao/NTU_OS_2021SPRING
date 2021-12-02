# include <stddef.h> 

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#ifdef MP2

#define NVMA 0x16
#define VMA_START (MAXVA / 2)
 
#define PROT_NONE       0x0
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4

#define MAP_SHARED      0x01
#define MAP_PRIVATE     0x02

#define ERROR_MMAP_NOT_A_ADDR      0x1
#define ERROR_MMAP_UNREADABLE      0x2
#define ERROR_MMAP_UNWRITABLE      0x3
#define ERROR_KALLOC_FAIL          0x4
#define ERROR_MMAP_PHYSICAL_FAIL   0x5

#endif
