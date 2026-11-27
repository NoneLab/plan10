LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := crt0-lib10c-amd64
LOCAL_TARGET := userspace

LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/lib10c/crt0

LOCAL_SRC_FILES := \
    runtime/amd64/crt0.S

LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone

include $(BUILD_OBJECTS)


include $(CLEAR_VARS)

LOCAL_MODULE := lib10c
LOCAL_TARGET := userspace

LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/lib10c

OPENBSD_CRYPT_SRCS := \
    openbsd/crypt/arc4random.c \
    openbsd/crypt/arc4random_uniform.c \
    openbsd/crypt/bcrypt.c \
    openbsd/crypt/blowfish.c \
    openbsd/crypt/crypt.c \
    openbsd/crypt/cryptutil.c

OPENBSD_STDLIB_SRCS := \
    openbsd/stdlib/_Exit.c \
    openbsd/stdlib/abs.c \
    openbsd/stdlib/atexit.c \
    openbsd/stdlib/div.c \
    openbsd/stdlib/exit.c \
    openbsd/stdlib/getenv.c \
    openbsd/stdlib/labs.c \
    openbsd/stdlib/ldiv.c \
    openbsd/stdlib/recallocarray.c \
    openbsd/stdlib/a64l.c \
    openbsd/stdlib/abort.c \
    openbsd/stdlib/atoi.c \
    openbsd/stdlib/atof.c \
    openbsd/stdlib/atol.c \
    openbsd/stdlib/atoll.c \
    openbsd/stdlib/bsearch.c \
    openbsd/stdlib/ecvt.c \
    openbsd/stdlib/gcvt.c \
    openbsd/stdlib/getopt_long.c \
    openbsd/stdlib/getsubopt.c \
    openbsd/stdlib/hcreate.c \
    openbsd/stdlib/heapsort.c \
    openbsd/stdlib/imaxabs.c \
    openbsd/stdlib/imaxdiv.c \
    openbsd/stdlib/insque.c \
    openbsd/stdlib/l64a.c \
    openbsd/stdlib/llabs.c \
    openbsd/stdlib/lldiv.c \
    openbsd/stdlib/lsearch.c \
    openbsd/stdlib/malloc.c \
    openbsd/stdlib/__mktemp4.c \
    openbsd/stdlib/mkdtemp.c \
    openbsd/stdlib/mkstemp.c \
    openbsd/stdlib/mktemp.c \
    openbsd/stdlib/reallocarray.c \
    openbsd/stdlib/merge.c \
    openbsd/stdlib/posix_pty.c \
    openbsd/stdlib/qsort.c \
    openbsd/stdlib/radixsort.c \
    openbsd/stdlib/rand.c \
    openbsd/stdlib/random.c \
    openbsd/stdlib/realpath.c \
    openbsd/stdlib/remque.c \
    openbsd/stdlib/setenv.c \
    openbsd/stdlib/strtoimax.c \
    openbsd/stdlib/strtol.c \
    openbsd/stdlib/strtoll.c \
    openbsd/stdlib/strtonum.c \
    openbsd/stdlib/strtoul.c \
    openbsd/stdlib/strtoull.c \
    openbsd/stdlib/strtoumax.c \
    openbsd/stdlib/system.c \
    openbsd/stdlib/tfind.c \
    openbsd/stdlib/thread_atexit.c \
    openbsd/stdlib/tsearch.c \
    openbsd/stdlib/_rand48.c \
    openbsd/stdlib/drand48.c \
    openbsd/stdlib/erand48.c \
    openbsd/stdlib/jrand48.c \
    openbsd/stdlib/lcong48.c \
    openbsd/stdlib/lrand48.c \
    openbsd/stdlib/mrand48.c \
    openbsd/stdlib/nrand48.c \
    openbsd/stdlib/seed48.c \
    openbsd/stdlib/srand48.c \
    openbsd/stdlib/icdb.c

OPENBSD_STDIO_SRCS := \
    openbsd/stdio/asprintf.c \
    openbsd/stdio/clrerr.c \
    openbsd/stdio/fclose.c \
    openbsd/stdio/fdclose.c \
    openbsd/stdio/fdopen.c \
    openbsd/stdio/feof.c \
    openbsd/stdio/ferror.c \
    openbsd/stdio/fflush.c \
    openbsd/stdio/fgetc.c \
    openbsd/stdio/fgetln.c \
    openbsd/stdio/fgetpos.c \
    openbsd/stdio/fgets.c \
    openbsd/stdio/fileno.c \
    openbsd/stdio/findfp.c \
    openbsd/stdio/flags.c \
    openbsd/stdio/fmemopen.c \
    openbsd/stdio/fopen.c \
    openbsd/stdio/fprintf.c \
    openbsd/stdio/fpurge.c \
    openbsd/stdio/fputc.c \
    openbsd/stdio/fputs.c \
    openbsd/stdio/fread.c \
    openbsd/stdio/freopen.c \
    openbsd/stdio/fscanf.c \
    openbsd/stdio/fseek.c \
    openbsd/stdio/fsetpos.c \
    openbsd/stdio/ftell.c \
    openbsd/stdio/funopen.c \
    openbsd/stdio/fvwrite.c \
    openbsd/stdio/fwalk.c \
    openbsd/stdio/fwrite.c \
    openbsd/stdio/getc.c \
    openbsd/stdio/getchar.c \
    openbsd/stdio/getw.c \
    openbsd/stdio/makebuf.c \
    openbsd/stdio/open_memstream.c \
    openbsd/stdio/open_wmemstream.c \
    openbsd/stdio/perror.c \
    openbsd/stdio/printf.c \
    openbsd/stdio/putc.c \
    openbsd/stdio/putchar.c \
    openbsd/stdio/puts.c \
    openbsd/stdio/putw.c \
    openbsd/stdio/refill.c \
    openbsd/stdio/remove.c \
    openbsd/stdio/rewind.c \
    openbsd/stdio/rget.c \
    openbsd/stdio/scanf.c \
    openbsd/stdio/setbuf.c \
    openbsd/stdio/setbuffer.c \
    openbsd/stdio/setvbuf.c \
    openbsd/stdio/snprintf.c \
    openbsd/stdio/sprintf.c \
    openbsd/stdio/sscanf.c \
    openbsd/stdio/stdio.c \
    openbsd/stdio/tempnam.c \
    openbsd/stdio/tmpfile.c \
    openbsd/stdio/tmpnam.c \
    openbsd/stdio/ungetc.c \
    openbsd/stdio/vasprintf.c \
    openbsd/stdio/vfprintf.c \
    openbsd/stdio/vfscanf.c \
    openbsd/stdio/vprintf.c \
    openbsd/stdio/vscanf.c \
    openbsd/stdio/vsnprintf.c \
    openbsd/stdio/vsprintf.c \
    openbsd/stdio/vsscanf.c \
    openbsd/stdio/wbuf.c \
    openbsd/stdio/wsetup.c \
    openbsd/stdio/flockfile.c \
    openbsd/stdio/fgetwc.c \
    openbsd/stdio/fgetwln.c \
    openbsd/stdio/fgetws.c \
    openbsd/stdio/fputwc.c \
    openbsd/stdio/fputws.c \
    openbsd/stdio/fwide.c \
    openbsd/stdio/getwc.c \
    openbsd/stdio/getwchar.c \
    openbsd/stdio/putwc.c \
    openbsd/stdio/putwchar.c \
    openbsd/stdio/ungetwc.c \
    openbsd/stdio/fwprintf.c \
    openbsd/stdio/swprintf.c \
    openbsd/stdio/vfwprintf.c \
    openbsd/stdio/vswprintf.c \
    openbsd/stdio/vwprintf.c \
    openbsd/stdio/wprintf.c \
    openbsd/stdio/fwscanf.c \
    openbsd/stdio/swscanf.c \
    openbsd/stdio/vfwscanf.c \
    openbsd/stdio/vswscanf.c \
    openbsd/stdio/vwscanf.c \
    openbsd/stdio/wscanf.c \
    openbsd/stdio/getdelim.c \
    openbsd/stdio/getline.c \
    openbsd/stdio/dprintf.c \
    openbsd/stdio/vdprintf.c \
    openbsd/stdio/fpending.c \
    openbsd/stdio/freadahead.c \
    openbsd/stdio/freading.c \
    openbsd/stdio/freadptr.c \
    openbsd/stdio/fseterr.c \
    openbsd/stdio/fwriting.c

OPENBSD_STRING_SRCS := \
	openbsd/string/bcmp.c \
    openbsd/string/bcopy.c \
    openbsd/string/bzero.c \
    openbsd/string/ffs.c \
    openbsd/string/memchr.c \
    openbsd/string/memcmp.c \
    openbsd/string/memcpy.c \
    openbsd/string/memmove.c \
    openbsd/string/memset.c \
    openbsd/string/strcat.c \
    openbsd/string/strchr.c \
    openbsd/string/strcmp.c \
    openbsd/string/strcpy.c \
    openbsd/string/strcspn.c \
    openbsd/string/strlen.c \
    openbsd/string/strlcat.c \
    openbsd/string/strlcpy.c \
    openbsd/string/strncat.c \
    openbsd/string/strncmp.c \
    openbsd/string/strncpy.c \
    openbsd/string/strpbrk.c \
    openbsd/string/strrchr.c \
    openbsd/string/strsep.c \
    openbsd/string/strspn.c \
    openbsd/string/strstr.c \
    openbsd/string/swab.c \
    openbsd/string/explicit_bzero.c \
    openbsd/string/ffsl.c \
    openbsd/string/ffsll.c \
    openbsd/string/memccpy.c \
    openbsd/string/memmem.c \
    openbsd/string/memrchr.c \
    openbsd/string/stpcpy.c \
    openbsd/string/stpncpy.c \
    openbsd/string/strcasecmp.c \
    openbsd/string/strcasecmp_l.c \
    openbsd/string/strcasestr.c \
    openbsd/string/strcoll.c \
    openbsd/string/strcoll_l.c \
    openbsd/string/strdup.c \
    openbsd/string/strerror.c \
    openbsd/string/strerror_l.c \
    openbsd/string/strerror_r.c \
    openbsd/string/strmode.c \
    openbsd/string/strndup.c \
    openbsd/string/strnlen.c \
    openbsd/string/strsignal.c \
    openbsd/string/strtok.c \
    openbsd/string/strxfrm.c \
    openbsd/string/strxfrm_l.c \
    openbsd/string/timingsafe_bcmp.c \
    openbsd/string/timingsafe_memcmp.c \
    openbsd/string/wcscat.c \
    openbsd/string/wcschr.c \
    openbsd/string/wcscmp.c \
    openbsd/string/wcscpy.c \
    openbsd/string/wcscspn.c \
    openbsd/string/wcslcat.c \
    openbsd/string/wcslcpy.c \
    openbsd/string/wcslen.c \
    openbsd/string/wcsncat.c \
    openbsd/string/wcsncmp.c \
    openbsd/string/wcsncpy.c \
    openbsd/string/wcsnlen.c \
    openbsd/string/wcspbrk.c \
    openbsd/string/wcsrchr.c \
    openbsd/string/wcsspn.c \
    openbsd/string/wcsstr.c \
    openbsd/string/wcstok.c \
    openbsd/string/wcswcs.c \
    openbsd/string/wcswidth.c \
    openbsd/string/wmemchr.c \
    openbsd/string/wmemcmp.c \
    openbsd/string/wmemcpy.c \
    openbsd/string/wmemmove.c \
    openbsd/string/wmemset.c \
    openbsd/string/wcsdup.c \
    openbsd/string/wcscasecmp.c \
    openbsd/string/wcscasecmp_l.c

OPENBSD_GEN_SRCS := \
    openbsd/gen/assert.c \
    openbsd/gen/basename.c \
    openbsd/gen/ctype_.c \
    openbsd/gen/dirname.c \
    openbsd/gen/err.c \
    openbsd/gen/errc.c \
    openbsd/gen/errlist.c \
    openbsd/gen/errx.c \
    openbsd/gen/fpclassify.c \
    openbsd/gen/frexp.c \
    openbsd/gen/getbsize.c \
    openbsd/gen/getprogname.c \
    openbsd/gen/isatty.c \
    openbsd/gen/isctype.c \
    openbsd/gen/isfinite.c \
    openbsd/gen/isinf.c \
    openbsd/gen/isnan.c \
    openbsd/gen/isnormal.c \
    openbsd/gen/ldexp.c \
    openbsd/gen/modf.c \
    openbsd/gen/raise.c \
    openbsd/gen/setprogname.c \
    openbsd/gen/siglist.c \
    openbsd/gen/signame.c \
    openbsd/gen/signbit.c \
    openbsd/gen/sigsetops.c \
    openbsd/gen/syslog.c \
    openbsd/gen/syslog_r.c \
    openbsd/gen/tolower_.c \
    openbsd/gen/toupper_.c \
    openbsd/gen/tree.c \
    openbsd/gen/unvis.c \
    openbsd/gen/verr.c \
    openbsd/gen/verrc.c \
    openbsd/gen/verrx.c \
    openbsd/gen/vis.c \
    openbsd/gen/vwarn.c \
    openbsd/gen/vwarnc.c \
    openbsd/gen/vwarnx.c \
    openbsd/gen/warn.c \
    openbsd/gen/warnc.c \
    openbsd/gen/warnx.c

OPENBSD_GDTOA_SRCS := \
	openbsd/gdtoa/dmisc.c \
    openbsd/gdtoa/dtoa.c \
    openbsd/gdtoa/gdtoa.c \
    openbsd/gdtoa/gethex.c \
    openbsd/gdtoa/gmisc.c \
    openbsd/gdtoa/hd_init.c \
    openbsd/gdtoa/hdtoa.c \
    openbsd/gdtoa/hexnan.c \
    openbsd/gdtoa/ldtoa.c \
    openbsd/gdtoa/misc.c \
    openbsd/gdtoa/smisc.c \
    openbsd/gdtoa/strtod.c \
    openbsd/gdtoa/strtodg.c \
    openbsd/gdtoa/strtof.c \
    openbsd/gdtoa/strtold.c \
    openbsd/gdtoa/sum.c \
    openbsd/gdtoa/ulp.c

OPENBSD_SYS_SRCS := \
    openbsd/sys/canceled.c \
    openbsd/sys/microtime.c \
    openbsd/sys/posix_madvise.c \
    openbsd/sys/pthread_sigmask.c \
    openbsd/sys/ptrace.c \
    openbsd/sys/semctl.c \
    openbsd/sys/stack_protector.c \
    openbsd/sys/timer_create.c \
    openbsd/sys/timer_delete.c \
    openbsd/sys/timer_getoverrun.c \
    openbsd/sys/timer_gettime.c \
    openbsd/sys/timer_settime.c \
    openbsd/sys/w_accept.c \
    openbsd/sys/w_accept4.c \
    openbsd/sys/w_clock_gettime.c \
    openbsd/sys/w_closefrom.c \
    openbsd/sys/w_connect.c \
    openbsd/sys/w_fork.c \
    openbsd/sys/w_fsync.c \
    openbsd/sys/w_gettimeofday.c \
    openbsd/sys/w_msgrcv.c \
    openbsd/sys/w_msgsnd.c \
    openbsd/sys/w_msync.c \
    openbsd/sys/w_nanosleep.c \
    openbsd/sys/w_open.c \
    openbsd/sys/w_openat.c \
    openbsd/sys/w_poll.c \
    openbsd/sys/w_ppoll.c \
    openbsd/sys/w_pread.c \
    openbsd/sys/w_preadv.c \
    openbsd/sys/w_pselect.c \
    openbsd/sys/w_pwrite.c \
    openbsd/sys/w_pwritev.c \
    openbsd/sys/w_readv.c \
    openbsd/sys/w_recvfrom.c \
    openbsd/sys/w_recvmmsg.c \
    openbsd/sys/w_recvmsg.c \
    openbsd/sys/w_select.c \
    openbsd/sys/w_sendmmsg.c \
    openbsd/sys/w_sendmsg.c \
    openbsd/sys/w_sendto.c \
    openbsd/sys/w_sigaction.c \
    openbsd/sys/w_sigprocmask.c \
    openbsd/sys/w_sigsuspend.c \
    openbsd/sys/w_vfork.c \
    openbsd/sys/w_wait4.c \
    openbsd/sys/w_waitid.c 

PLAN9_SRCS := \
	src/errno.c \
	src/thread_callbacks.c \
	src/native_bootstrap_control.c \
    src/native_bootstrap_entropy.c \
    src/native_bootstrap_io.c \
    src/native_bootstrap_signal.c \
    src/native_bootstrap_thread.c \
    src/native_bootstrap_vm.c \
	runtime/crti.c

LOCAL_SRC_FILES := \
	$(OPENBSD_CRYPT_SRCS) \
	$(OPENBSD_STDLIB_SRCS) \
	$(OPENBSD_STDIO_SRCS) \
	$(OPENBSD_STRING_SRCS) \
	$(OPENBSD_GEN_SRCS) \
	$(OPENBSD_AMD64_GDTOA_SRCS) \
	$(OPENBSD_GDTOA_SRCS) \
	$(OPENBSD_SYS_SRCS) \
	$(PLAN9_SRCS)

LOCAL_C_INCLUDES := \
					$(LOCAL_PATH)/amd64 		\
					$(LOCAL_PATH)/hidden		\
					$(LOCAL_PATH)/private		\
					$(LOCAL_PATH)/openbsd/		\
					$(LOCAL_PATH)/openbsd/gdtoa \
					$(LOCAL_PATH)/amd64/gdtoa 	\
					$(LOCAL_PATH)/include 		\
					$(LOCAL_PATH)/include/sys	\

LOCAL_EXPORT_C_INCLUDE_DIRS := 	$(LOCAL_PATH)/amd64 		\
								$(LOCAL_PATH)/include		

# Compile the existing Plan 9 syscall entry into this archive. Do not link
# libc-plan9: its public interfaces can conflict with the OpenBSD ABI.
LOCAL_SRC_FILES += \
    src/native_syscall.c \
    src/native_io.c \
    src/native_runtime.c \
    ../libc/amd64/syscall.S

LOCAL_ASFLAGS += --target=$(TARGET_TRIPLE) -mno-red-zone

LOCAL_CFLAGS := \
	$(BOOT64_USER_CFLAGS) \
	-D__LIBC__ \
	-DMALLOC_SMALL \
	-DP9_LIB10C_BOOTSTRAP \
	-include $(LOCAL_PATH)/private/namespace.h

include $(BUILD_STATIC_LIBRARY)
