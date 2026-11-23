#include <u.h>
#include <libc.h>
#include "sys.h"
#include "syscall.h"

#define SC(n,a,b,c,d,e) p9_syscall5((n),(ulong)(a),(ulong)(b),(ulong)(c),(ulong)(d),(ulong)(e))
int open(char *p,int m) { return SC(OPEN,p,m,0,0,0); }
int close(int f) { return SC(CLOSE,f,0,0,0,0); }
int create(char *p,int m,ulong perm) { return SC(CREATE,p,m,perm,0,0); }
int remove(char *p) { return SC(REMOVE,p,0,0,0,0); }
int bind(char *a,char *b,int f) { return SC(BIND,a,b,f,0,0); }
int chdir(char *p) { return SC(CHDIR,p,0,0,0,0); }
int dup(int a,int b) { return SC(DUP,a,b,0,0,0); }
int pipe(int *p) { return SC(PIPE,p,0,0,0,0); }
int rfork(int f) { return SC(RFORK,f,0,0,0,0); }
int exec(char *p,char **v) { return SC(EXEC,p,v,0,0,0); }
void _exits(char *s) { SC(EXITS,s,0,0,0,0); for(;;) __asm__ volatile("ud2"); }
long alarm(ulong n) { return SC(ALARM,n,0,0,0,0); }
int sleep(long n) { return SC(SLEEP,n,0,0,0,0); }
int fauth(int fd,char *a) { return SC(FAUTH,fd,a,0,0,0); }
int fversion(int fd,int m,char *s,int n) { return SC(FVERSION,fd,m,s,n,0); }
int mount(int fd,int afd,char *p,int flags,char *spec) { return SC(MOUNT,fd,afd,p,flags,spec); }
int unmount(char *p,char *old) { return SC(UNMOUNT,p,old,0,0,0); }
int stat(char *p,uchar *b,int n) { return SC(STAT,p,b,n,0,0); }
int fstat(int fd,uchar *b,int n) { return SC(FSTAT,fd,b,n,0,0); }
int wstat(char *p,uchar *b,int n) { return SC(WSTAT,p,b,n,0,0); }
int fwstat(int fd,uchar *b,int n) { return SC(FWSTAT,fd,b,n,0,0); }
long pread(int fd,void *b,long n,vlong off) { return SC(PREAD,fd,b,n,off,0); }
long pwrite(int fd,void *b,long n,vlong off) { return SC(PWRITE,fd,b,n,off,0); }
long read(int fd,void *b,long n) { return pread(fd,b,n,-1); }
long write(int fd,void *b,long n) { return pwrite(fd,b,n,-1); }
int errstr(char *buf,uint n) { return SC(ERRSTR,buf,n,0,0,0); }
int noted(int state) { return SC(NOTED,state,0,0,0,0); }
int notify(void (*fn)(void*,char*)) { return SC(NOTIFY,fn,0,0,0,0); }
int await(char *buf,int n) { return SC(AWAIT,buf,n,0,0,0); }
int semacquire(long *p,int block) { return SC(SEMACQUIRE,p,block,0,0,0); }
long semrelease(long *p,long n) { return SC(SEMRELEASE,p,n,0,0,0); }
void *rendezvous(void *tag,void *v) { return (void*)(uintptr)SC(RENDEZVOUS,tag,v,0,0,0); }
int brk_(void *v) { return SC(BRK_,v,0,0,0,0); }
vlong seek(int fd,vlong offset,int whence) {
    vlong result=-1;
    if(SC(SEEK,&result,fd,offset,0,whence)<0) return -1;
    return result;
}