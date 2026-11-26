LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib10c
LOCAL_TARGET := userspace

LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/lib10c

OPENBSD_STDLIB_SRCS := \
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

PLAN9_SRCS := \
	src/errno.c

LOCAL_SRC_FILES := \
	$(OPENBSD_STDLIB_SRCS) \
	$(OPENBSD_STDIO_SRCS) \
	$(OPENBSD_AMD64_GDTOA_SRCS) \
	$(OPENBSD_GDTOA_SRCS) \
	$(PLAN9_SRCS)

LOCAL_C_INCLUDES := \
					$(LOCAL_PATH)/amd64 		\
					$(LOCAL_PATH)/hidden		\
					$(LOCAL_PATH)/private		\
					$(LOCAL_PATH)/openbsd/gdtoa \
					$(LOCAL_PATH)/amd64/gdtoa 	\
					$(LOCAL_PATH)/include 		\
					$(LOCAL_PATH)/include/sys	\

LOCAL_EXPORT_C_INCLUDE_DIRS := 	$(LOCAL_PATH)/amd64 		\
								$(LOCAL_PATH)/include		

LOCAL_CFLAGS := \
	$(BOOT64_USER_CFLAGS) \
	-include $(LOCAL_PATH)/private/namespace.h

include $(BUILD_STATIC_LIBRARY)