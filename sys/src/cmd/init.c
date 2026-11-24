#include <u.h>
#include <libc.h>
#include <auth.h>
#include <authsrv.h>

/* U3.5: /boot/boot mounts KFS under /root, but init's newns() may
 * reconfigure the process namespace.  Perform disk /bin binding in the
 * final namespace, and log both source and destination lookup results.
 * No effect on the U3.4 embedded-rc build or the original non-pc64 port.
 */
#if defined(P9_NATIVE_BOOT64) && !defined(P9_BOOT_RC_EMBED)
static void
pc64diskbin(void)
{
    int fd;

    fd = open("/root/bin/rc", OREAD);
    if(fd < 0){
        print("init: KFS source /root/bin/rc unavailable: %r\n");
        print("init: check mkbootfs.kfs image and KFS /root mount\n");
        return;
    }
    close(fd);
    print("init: KFS /root/bin/rc accessible\n");

    if(bind("/root/bin", "/bin", MREPL) < 0)
        print("init: failed to bind KFS /root/bin to /bin: %r\n");
    else
        print("init: KFS /bin mounted from /root/bin\n");

    /* rcmain is not embedded either; give rc a disk-backed /rc tree. */
    if(bind("/root/rc", "/rc", MREPL) < 0)
        print("init: failed to bind KFS /root/rc to /rc: %r\n");
    else
        print("init: KFS /rc mounted from /root/rc\n");

    fd = open("/bin/rc", OREAD);
    if(fd < 0)
        print("init: KFS /bin/rc still inaccessible: %r\n");
    else{
        close(fd);
        print("init: KFS /bin/rc lookup PASS\n");
    }
}
#endif

char*	readenv(char*);
void	setenv(char*, char*);
void	cpenv(char*, char*);
void	closefds(void);
void	fexec(void(*)(void));
void	rcexec(void);
void	cpustart(void);
void	pass(int);

#if defined(P9_NATIVE_BOOT64) && defined(P9_BOOT_RC_EMBED)
#define RC_EXEC_PATH "/boot/rc"
#else
#define RC_EXEC_PATH "/bin/rc"
#endif

char	*service;
char	*cmd;
char	*cpu;
char	*systemname;
int	manual;
int	iscpu;

void
main(int argc, char *argv[])
{
	char *user;
	int fd;
	char ctl[128];

	closefds();
	alarm(0);

	service = "cpu";
	manual = 0;
	ARGBEGIN{
	case 'c':
		service = "cpu";
		break;
	case 'm':
		manual = 1;
		break;
	case 't':
		service = "terminal";
		break;
	}ARGEND
	cmd = *argv;

	snprint(ctl, sizeof(ctl), "#p/%d/ctl", getpid());
	fd = open(ctl, OWRITE);
	if(fd < 0)
		print("init: warning: can't open %s: %r\n", ctl);
	else
		if(write(fd, "pri 10", 6) != 6)
			print("init: warning: can't set priority: %r\n");
	close(fd);

	cpu = readenv("#e/cputype");
	setenv("#e/objtype", cpu);
	setenv("#e/service", service);
	cpenv("/adm/timezone/local", "#e/timezone");
	user = readenv("#c/user");
	systemname = readenv("#c/sysname");

	newns(user, 0);
#if defined(P9_NATIVE_BOOT64) && !defined(P9_BOOT_RC_EMBED)
	pc64diskbin();
#endif
	iscpu = strcmp(service, "cpu")==0;

	if(iscpu && manual == 0)
		fexec(cpustart);

	for(;;){
		print("\ninit: starting %s\n", RC_EXEC_PATH);
		fexec(rcexec);
		manual = 1;
		cmd = 0;
		sleep(1000);
	}
}

void
pass(int fd)
{
	char key[DESKEYLEN];
	char typed[32];
	char crypted[DESKEYLEN];
	int i;

	for(;;){
		print("\n%s password:", systemname);
		for(i=0; i<sizeof typed; i++){
			if(read(0, typed+i, 1) != 1){
				print("init: can't read password; insecure\n");
				return;
			}
			if(typed[i] == '\n'){
				typed[i] = 0;
				break;
			}
		}
		if(i == sizeof typed)
			continue;
		if(passtokey(crypted, typed) == 0)
			continue;
		seek(fd, 0, 0);
		if(read(fd, key, DESKEYLEN) != DESKEYLEN){
			print("init: can't read key; insecure\n");
			return;
		}
		if(memcmp(crypted, key, sizeof key))
			continue;
		/* clean up memory */
		memset(crypted, 0, sizeof crypted);
		memset(key, 0, sizeof key);
		return;
	}
}

static int gotnote;

void
pinhead(void*, char *msg)
{
	gotnote = 1;
	fprint(2, "init got note '%s'\n", msg);
	noted(NCONT);
}

void
fexec(void (*execfn)(void))
{
	Waitmsg *w;
	int pid;

	switch(pid=fork()){
	case 0:
		rfork(RFNOTEG);
		(*execfn)();
		print("init: exec error: %r\n");
		exits("exec");
	case -1:
		print("init: fork error: %r\n");
		exits("fork");
	default:
	casedefault:
		notify(pinhead);
		gotnote = 0;
		w = wait();
		if(w == nil){
			if(gotnote)
				goto casedefault;
			print("init: wait error: %r\n");
			break;
		}
		if(w->pid != pid){
			free(w);
			goto casedefault;
		}
		if(strstr(w->msg, "exec error") != 0){
			print("init: exit string %s\n", w->msg);
			print("init: sleeping because exec failed\n");
			free(w);
			for(;;)
				sleep(1000);
		}
		if(w->msg[0])
			print("init: rc exit status: %s\n", w->msg);
		free(w);
		break;
	}
}

void
rcexec(void)
{
#ifdef P9_NATIVE_BOOT64
	if (cmd)
		execl(RC_EXEC_PATH, "rc", "-c", cmd, nil);
	else
		execl(RC_EXEC_PATH, "rc", "-i", nil);
#else
	if(cmd)
		execl(RC_EXEC_PATH, "rc", "-c", cmd, nil);
	else if(manual || iscpu)
		execl(RC_EXEC_PATH, "rc", nil);
	else if(strcmp(service, "terminal") == 0)
		execl(RC_EXEC_PATH, "rc", "-c", ". /rc/bin/termrc; home=/usr/$user; cd; . lib/profile", nil);
	else
		execl(RC_EXEC_PATH, "rc", nil);
#endif
}

void
cpustart(void)
{
	execl(RC_EXEC_PATH, "rc", "-c", "/rc/bin/cpurc", nil);
}

char*
readenv(char *name)
{
	int f, len;
	Dir *d;
	char *val;

	f = open(name, OREAD);
	if(f < 0){
		print("init: can't open %s: %r\n", name);
		return "*unknown*";	
	}
	d = dirfstat(f);
	if(d == nil){
		print("init: can't stat %s: %r\n", name);
		return "*unknown*";
	}
	len = d->length;
	free(d);
	if(len == 0)	/* device files can be zero length but have contents */
		len = 64;
	val = malloc(len+1);
	if(val == nil){
		print("init: can't malloc %s: %r\n", name);
		return "*unknown*";
	}
	len = read(f, val, len);
	close(f);
	if(len < 0){
		print("init: can't read %s: %r\n", name);
		return "*unknown*";
	}else
		val[len] = '\0';
	return val;
}

void
setenv(char *var, char *val)
{
	int fd;

	fd = create(var, OWRITE, 0644);
	if(fd < 0)
		print("init: can't open %s\n", var);
	else{
		fprint(fd, val);
		close(fd);
	}
}

void
cpenv(char *file, char *var)
{
	int i, fd;
	char buf[8192];

	fd = open(file, OREAD);
	if(fd < 0)
		print("init: can't open %s\n", file);
	else{
		i = read(fd, buf, sizeof(buf)-1);
		if(i <= 0)
			print("init: can't read %s: %r\n", file);
		else{
			close(fd);
			buf[i] = 0;
			setenv(var, buf);
		}
	}
}

/*
 *  clean up after /boot
 */
void
closefds(void)
{
	int i;

	for(i = 3; i < 30; i++)
		close(i);
}
