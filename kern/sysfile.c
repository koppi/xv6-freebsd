//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and vfs.c.
//

#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/stat.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/vfs.h>
#include <xv6/file.h>
#include <xv6/fcntl.h>
#include <xv6/syscall.h>
#include <xv6/pcspkr.h>

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=proc->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;

  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd] == 0){
      proc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;
  
  if(argfd(0, 0, &f) < 0)
    return EBADF;
  if((fd=fdalloc(f)) < 0)
    return EBADF;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return EINVAL;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return EINVAL;
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;
  
  if(argfd(0, &fd, &f) < 0)
    return EBADF;
  proc->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;
  
  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return EINVAL;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return EINVAL;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ip->iops->ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return ENOENT;
  }

  ip->nlink++;
  ip->iops->iupdate(ip);
  ip->iops->iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  dp->iops->ilock(dp);
  if(dp->dev != ip->dev || ip->iops->dirlink(dp, name, ip->inum, ip->type) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ip->iops->ilock(ip);
  ip->nlink--;
  ip->iops->iupdate(ip);
  iunlockput(ip);
  end_op();
  return EEXIST;
}

//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return EINVAL;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return ENOENT;
  }

  dp->iops->ilock(dp);

  // Cannot unlink "." or "..".
  if(dp->fs_t->ops->namecmp(name, ".") == 0 || dp->fs_t->ops->namecmp(name, "..") == 0)
    goto bad;

  if((ip = dp->iops->dirlookup(dp, name, &off)) == 0)
    goto bad;
  ip->iops->ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !ip->iops->isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(dp->iops->writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    ip->iops->iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  ip->iops->iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return EPERM;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  uint off;
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  dp->iops->ilock(dp);

  if((ip = dp->iops->dirlookup(dp, name, &off)) != 0){
    iunlockput(dp);
    ip->iops->ilock(ip);
    if((type == T_FILE) && (ip->type != T_DIR))
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = dp->fs_t->ops->ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ip->iops->ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  ip->iops->iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    ip->iops->iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(ip->iops->dirlink(ip, ".", ip->inum, ip->type) < 0 || ip->iops->dirlink(ip, "..", dp->inum, dp->type) < 0)
      panic("create dots");
  }

  if(dp->iops->dirlink(dp, name, ip->inum, ip->type) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}

int
sys_mount(void)
{
  char *devf;
  char *path;
  char *fstype;
  struct inode *ip, *devi;

  if (argstr(0, &devf) < 0 || argstr(1, &path) < 0 || argstr(2, &fstype) < 0) {
    return -1;
  }

  if ((ip = namei(path)) == 0 || (devi = namei(devf)) == 0) {
    return -1;
  }

  struct filesystem_type *fs_t = getfs(fstype);

  if (fs_t == 0) {
    cprintf("FS type not found\n");
    return -1;
  }

  ip->iops->ilock(ip);
  devi->iops->ilock(devi);
  // we only can mount points over directories nodes
  if (ip->type != T_DIR && ip->ref > 1) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  // The device inode should be T_DEV
  if (devi->type != T_DEV) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  if (bdev_open(devi) != 0) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  if (devi->minor == 0 || devi->minor == ROOTDEV) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  // Add this to a list to retrieve the Filesystem type to current device
  if (putvfsonlist(devi->major, devi->minor, fs_t) == -1) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  int mounted = fs_t->ops->mount(devi, ip);

  if (mounted != 0) {
    ip->iops->iunlock(ip);
    devi->iops->iunlock(devi);
    return -1;
  }

  ip->type = T_MOUNT;

  ip->iops->iunlock(ip);
  devi->iops->iunlock(devi);

  return 0;
}

int sys_umount(void)
{
  char* path;
  struct inode *ip;
  
  if (argstr(0, &path) < 0) {
    return -1;
  }

  if ((ip = namei(path)) == 0) {
    return -1;
  }

  // XXX
    
  return 0;
}

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return EINVAL;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return EEXIST;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return ENOENT;
    }
    ip->iops->ilock(ip);
    if(ip->type == T_DIR && (omode != O_RDONLY)) {
      iunlockput(ip);
      end_op();
      return EISDIR;
    }
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return EACCES;
  }

  if((omode & O_TRUNC)) {
    // Not if someone else has this file open. But ref > 1 when O_CREAT
    // if(ip->ref>1) {
    //  iunlockput(ip);
    //  end_op();
    //  return EACCES;
    //}
    ip->iops->itrunc(ip);
  }
  ip->iops->iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  if(omode & O_APPEND)
    f->off= f->ip->size;
  return fd;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return EEXIST;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int len;
  int major, minor;
  
  begin_op();
  if((len=argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return EINVAL;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
ichdir(struct inode *ip)
{
  ip->iops->ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return ENOTDIR;
  }
  ip->iops->iunlock(ip);
  iput(proc->cwd);
  end_op();
  proc->cwd = ip;
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
    end_op();
    return ENOENT;
  }
  return(ichdir(ip));
}

int
sys_fchdir(void)
{
  struct file *f;

  if(argfd(0, 0, &f) < 0)
    return EBADF;
  return(ichdir(f->ip));
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return EINVAL;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return E2BIG;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return EINVAL;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return ENOENT;
  }
  return exec(path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return EINVAL;
  if(pipealloc(&rf, &wf) < 0)
    return EMFILE;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      proc->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return ENFILE;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

// lseek code derived from https://github.com/ctdk/xv6
int sys_lseek(void)
{
	int fd;
	int offset;
	int base;
	int newoff;
	int zerosize, i;
	char *zeroed, *z;

	struct file *f;

	if ((argfd(0, &fd, &f)<0) ||
		(argint(1, &offset)<0) || (argint(2, &base)<0))
			return(EINVAL);

	if( base == SEEK_SET) {
		newoff = offset;
	}

	if (base == SEEK_CUR)
		newoff = f->off + offset;

	if (base == SEEK_END)
		newoff = f->ip->size + offset;

	if (newoff < 0)
		return EINVAL;

	if (newoff > f->ip->size){
		zerosize = newoff - f->ip->size;
		zeroed = kalloc();
		z = zeroed;
		for (i = 0; i < PGSIZE; i++)
			*z++ = 0;
		while (zerosize > 0){
			filewrite(f, zeroed, zerosize);
			zerosize -= PGSIZE;
		}
		kfree(zeroed);
	}

	f->off = newoff;
	return newoff;
}

int
sys_ioctl(void)
{
  int fd;
  struct file *f;
  int request;
  
  if(argfd(0, &fd, &f) < 0 || argint(1, &request) < 0)
    return EINVAL;
  if((f->type != FD_INODE) || (f->ip->type != T_DEV))
    return ENOTTY;

  if(f->ip->major < 0 || f->ip->major >= NDEV || !devsw[f->ip->major].ioctl)
    return ENOTTY;
  return devsw[f->ip->major].ioctl(f->ip, request);
}

static unsigned long seed;

int sys_rng(void)
{
  int start, end;
  
  if (argint(0, &start) < 0 || argint(1, &end) < 0)
    return -1;

  // from the musl project
  seed = 6364136223846793005ULL*seed + 1;
  return (seed>>13) % (end - start) + start;
}

int sys_beep(void)
{
  int frq;
  
  if (argint(0, &frq) < 0)
    return -1;

  beep(frq);

  return 0;
}
