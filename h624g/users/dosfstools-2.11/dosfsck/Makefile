ifeq ($(wildcard $(DIR_LINUX)/.config),)
DIR_LINUX_KERNEL = $(DIR_ROOT)/linux-2.6.30
CFLAGS=-I$(DIR_LINUX_KERNEL)/include -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN
else
KERNEL_VER=$(patsubst $(DIR_ROOT)/%,%,$(DIR_LINUX))
ifeq ($(KERNEL_VER),linux-2.6.30)
CFLAGS=-I$(DIR_LINUX_KERNEL)/include -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN
endif
ifeq ($(KERNEL_VER),linux-3.10)
CFLAGS=-I$(DIR_LINUX_KERNEL)/include/uapi -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN
endif
DIR_LINUX_KERNEL = $(DIR_LINUX)
endif

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
#CFLAGS=-I../../linux-2.6.30/include -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN
#CFLAGS=-I$(DIR_LINUX_KERNEL)/include -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN

OBJECTS = boot.o check.o common.o dosfsck.o fat.o file.o io.o lfn.o

all: dosfsck

dosfsck: $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $^

.c.o:
	$(CC) -c $(CFLAGS) $*.c

install: dosfsck
	mkdir -p $(SBINDIR) $(MANDIR)
	install -m 755 dosfsck $(SBINDIR)
	install -m 644 dosfsck.8 $(MANDIR)
	rm -f $(SBINDIR)/fsck.msdos
	rm -f $(SBINDIR)/fsck.vfat
	ln -s dosfsck $(SBINDIR)/fsck.msdos
	ln -s dosfsck $(SBINDIR)/fsck.vfat
	rm -f $(MANDIR)/fsck.msdos.8
	ln -s dosfsck.8 $(MANDIR)/fsck.msdos.8
	ln -s dosfsck.8 $(MANDIR)/fsck.vfat.8

clean:
	rm -f *.o *.s *.i *~ \#*# tmp_make .#* .new*

distclean: clean
	rm -f *.a dosfsck

dep:
	sed '/\#\#\# Dependencies/q' <Makefile >tmp_make
	$(CPP) $(CFLAGS) -MM *.c >>tmp_make
	mv tmp_make Makefile

### Dependencies
boot.o: boot.c common.h dosfsck.h io.h boot.h
check.o: check.c common.h dosfsck.h io.h fat.h file.h lfn.h check.h
common.o: common.c common.h
dosfsck.o: dosfsck.c common.h dosfsck.h io.h boot.h fat.h file.h \
 check.h
fat.o: fat.c common.h dosfsck.h io.h check.h fat.h
file.o: file.c common.h file.h
io.o: io.c dosfsck.h common.h io.h
lfn.o: lfn.c common.h io.h dosfsck.h lfn.h file.h

romfs:
	$(ROMFSINST) /bin/dosfsck
