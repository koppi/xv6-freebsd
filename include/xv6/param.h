#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define USTACKPAGES   4  // number of pages on the user stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define MAXBDEV       4  // maximum numbers of block devices
#define ROOTDEV       1  // device number of file system root disk
#define PIPESIZE    512  // number of bytes in an empty pipe
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       10000  // size of file system in blocks
#define MAXVFSSIZE   4  // size of file system in blocks
#define IDEMAJOR     0  // IDE major block device
#define ROOTFSTYPE   "s5"
#define MAXBSIZE     4096 // The Maximum BSIZE
#define BSIZE        512

