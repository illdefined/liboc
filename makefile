all: liboc.a liboc.so identity pthrough sqlite

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

-include .depend

endif
endif

CC       ?= $(shell if which clang 2>&1 >/dev/null; then echo clang; else echo gcc; fi)
CPP      ?= $(CC) -E

CPPFLAGS += -std=c99 -D_XOPEN_SOURCE=600 -ggdb3
CFLAGS   += -pipe -O2 -Wall -Wno-parentheses -Wcast-align -Wwrite-strings -pedantic
CFLAGS   += -fmerge-all-constants -fstrict-overflow
CFLAGS   += -frename-registers -fPIC -fno-common
LDFLAGS  += -shared
LIBS     ?= -lc -ltokyocabinet

DESTDIR  ?= /
PREFIX   ?= usr/
LIBDIR   ?= lib
INCDIR   ?= include

hdr      := skein.h string.h storage.h transform.h trivial.h
src      := canonical.c skein.c storage.c string.c transform.c trivial.c
obj      := $(src:.c=.o)
tst      := rotate skein string

check: .depend .sparse $(src)
	for test in $(tst); \
	do \
		$(CC) $(CPPFLAGS) -DTEST $(CFLAGS) -o $$test $$test.c && ./$$test || exit 1; \
	done

clean:
	rm -f -- liboc.a liboc.so identity pthrough sqlite $(obj) $(tst)

distclean: clean
	rm -f -- .depend .sparse byteorder.o

install: liboc.a liboc.so identity pthrough sqlite
	install -d $(DESTDIR)$(PREFIX)$(INCDIR)/OC
	install -m 644 $(hdr) $(DESTDIR)$(PREFIX)$(INCDIR)/OC
	
	install -d $(DESTDIR)$(PREFIX)$(LIBDIR)
	install -m 644 liboc.a $(DESTDIR)$(PREFIX)$(LIBDIR)
	install -m 755 liboc.so $(DESTDIR)$(PREFIX)$(LIBDIR)
	
	install -d $(DESTDIR)$(PREFIX)share/opencorpus/0000000000000000000000000000000000000000000000000000000000000000
	install -m 755 identity $(DESTDIR)$(PREFIX)share/opencorpus/0000000000000000000000000000000000000000000000000000000000000000/transform
	echo -n pthrough >$(DESTDIR)$(PREFIX)share/opencorpus/0000000000000000000000000000000000000000000000000000000000000000/runtime
	echo -n OpenCorpus.identity >$(DESTDIR)$(PREFIX)share/opencorpus/0000000000000000000000000000000000000000000000000000000000000000/trivial
	
	install -d $(DESTDIR)$(PREFIX)libexec/opencorpus/runtime
	install -m 755 pthrough $(DESTDIR)$(PREFIX)libexec/opencorpus/runtime/pthrough
	
	install -d $(DESTDIR)$(PREFIX)libexec/opencorpus/storage
	install -m 755 sqlite $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/sqlite
	install -m 755 bzfile.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/bzfile
	install -m 755 curl.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/curl
	install -m 755 file.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/file
	install -m 755 tar.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/tar
	install -m 755 xzfile.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/xzfile
	install -m 755 zfile.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/zfile
	install -m 755 zip.sh $(DESTDIR)$(PREFIX)libexec/opencorpus/storage/zip

endian.h: byteorder.o
	if grep -l "BIGenDianSyS" byteorder.o; \
	then \
		sed -i -e 's/^\(#define BYTE_ORDER \).*$$/\1BIG_ENDIAN/' $@; \
	elif grep -l "LiTTleEnDian" byteorder.o; \
	then \
		sed -i -e 's/^\(#define BYTE_ORDER \).*$$/\1LITTLE_ENDIAN/' $@; \
	else \
		echo "Unable to determine the target system’s byte‐order!"; \
		exit 1; \
	fi

.depend: $(src)
	$(CPP) $(CPPFLAGS) -M $(src) >$@

.sparse: .depend $(src)
	sparse $(CPPFLAGS) \
		-gcc-base-dir "$$(gcc -print-search-dirs | awk '/^install: / { print $$2 }')" \
		-ftabstop=4 \
		-Wdefault-bitfield-sign \
		-Wparen-string \
		-Wptr-subtraction-blows \
		-Wreturn-void \
		-Wshadow \
		-Wtypesign \
		$(src)
	touch $@

liboc.a: .depend $(obj)
	$(AR) rc $@ $(obj)

liboc.so: .depend $(obj)
	$(CC) $(LDFLAGS) -o $@ $(obj) $(LIBS)

identity: identity.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

sqlite: sqlite.c string.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ -lsqlite3

pthrough: pthrough.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: all check clean distclean
.SUFFIXES: .c .o
