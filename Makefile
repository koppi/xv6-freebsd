MAKEFLAGS += --no-print-directory

TOP_SRCDIR = .
include $(TOP_SRCDIR)/Makefile.common

.PHONY: all
all: xv6.img

# delete target if error building it
.DELETE_ON_ERROR:

xv6.img: boot/bootblock kern/kernel fs.img
	dd if=/dev/zero of=xv6.img count=10000
	dd if=boot/bootblock of=xv6.img conv=notrunc
	dd if=kern/kernel of=xv6.img seek=1 conv=notrunc

xv6memfs.img: boot/bootblock kern/kernelmemfs
	dd if=/dev/zero of=xv6memfs.img count=10000
	dd if=boot/bootblock of=xv6memfs.img conv=notrunc
	dd if=kern/kernelmemfs of=xv6memfs.img seek=1 conv=notrunc

boot/bootblock:
	$(MAKE) -C boot bootblock

tools/mkfs:
	$(MAKE) -C tools mkfs

fs/cat:
	mkdir -p fs/bin
	mkdir -p fs/sbin
	mkdir -p fs/etc
	$(MAKE) -C lib all
	$(MAKE) -C usr.bin all
	$(MAKE) -C usr.sbin all

.PHONY: kern/kernel
kern/kernel:
	$(MAKE) -C kern kernel

#fs.img: tools/mkfs README usr.bin/_cat
#	tools/mkfs fs.img README _*

fs.img: tools/mkfs README fs/cat
	cp README fs
	mv fs/bin/init fs/etc
	tools/mkfs fs.img fs
	tools/mkfs -noroot fs2.img fs

-include *.d

clean: 
	$(MAKE) -C boot clean
	$(MAKE) -C lib clean
	$(MAKE) -C usr.bin clean
	$(MAKE) -C kern clean
	$(MAKE) -C tools clean
	$(MAKE) -C doc clean
	rm -rf *.img fs/bin/* cscope.* fs/README

# make a printout
doc: doc/xv6.pdf
print: doc/xv6.pdf
doc/xv6.pdf:
	$(MAKE) -C doc all


# run in emulators

bochs : fs.img xv6.img
	if [ ! -e .bochsrc ]; then ln -s misc/dot-bochsrc .bochsrc; fi
	bochs -q

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 2
endif
QEMUOPTS = -drive file=xv6.img,index=0,media=disk,format=raw \
           -drive file=fs.img,index=1,media=disk,format=raw \
	   -drive file=ext2.img,index=2,media=disk,format=raw \
	   -smp $(CPUS) -m 512M \
	   -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
	   -net none \
	   -soundhw pcspk \
	   -soundhw sb16 \
	   $(QEMUEXTRA)

qemu: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-memfs: xv6memfs.img
	$(QEMU) xv6memfs.img -smp $(CPUS) -m 256

qemu-nox: fs.img xv6.img
	$(QEMU) -nographic $(QEMUOPTS)

gdb:
	$(GDB) -n -x .gdbinit

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

cscope:
	find -type f -name "*.[chsS]" -print > cscope.files
	cscope -q -k

.PHONY: clean distclean run depend qemu qemu-nox qemu-gdb qemu-nox-gdb gdb
.PHONY: bochs cscope

# CUT HERE
# prepare dist for students
# after running make dist, probably want to
# rename it to rev0 or rev1 or so on and then
# check in that version.

EXTRA=\
	mkfs.c ulib.c user.h cat.c echo.c forktest.c grep.c kill.c\
	ln.c ls.c mkdir.c rm.c stressfs.c usertests.c wc.c zombie.c\
	printf.c umalloc.c\
	README misc/dot-bochsrc *.pl toc.* runoff runoff1 runoff.list\
	.gdbinit.tmpl gdbutil\

dist:
	rm -rf dist
	mkdir dist
	for i in $(FILES); \
	do \
		grep -v PAGEBREAK $$i >dist/$$i; \
	done
	sed '/CUT HERE/,$$d' Makefile >dist/Makefile
	echo >dist/runoff.spec
	cp $(EXTRA) dist

dist-test:
	rm -rf dist
	make dist
	rm -rf dist-test
	mkdir dist-test
	cp dist/* dist-test
	cd dist-test; $(MAKE) print
	cd dist-test; $(MAKE) bochs || true
	cd dist-test; $(MAKE) qemu

# update this rule (change rev#) when it is time to
# make a new revision.
tar:
	rm -rf /tmp/xv6
	mkdir -p /tmp/xv6
	cp dist/* dist/.gdbinit.tmpl /tmp/xv6
	(cd /tmp; tar cf - xv6) | gzip >xv6-rev9.tar.gz  # the next one will be 9 (6/27/15)

.PHONY: dist-test dist tar
