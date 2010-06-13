all: liboc.a liboc.so

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

-include .depend

endif
endif

CC       ?= $(shell if which clang 2>&1 >/dev/null; then echo clang; else echo gcc; fi)
CPP      ?= $(CC) -E

CPPFLAGS += -std=c99 -D_XOPEN_SOURCE=600
CFLAGS   += -pipe -O2 -Wall -Wno-parentheses -pedantic
CFLAGS   += -fmerge-all-constants -fstrict-overflow
CFLAGS   += -frename-registers -fPIC -fno-common
LDFLAGS  += -Wl,-O1 -shared
LIBS     ?= -lc -ltokyocabinet

DESTDIR  ?= /
PREFIX   ?= usr/
LIBDIR   ?= lib
INCDIR   ?= include

hdr      := skein.h string.h storage.h transform.h trivial.h
src      := skein.c storage.c string.c transform.c trivial.c
obj      := $(src:.c=.o)
tst      := rotate skein string

check: .depend .sparse $(src)
	for test in $(tst); \
	do \
		$(CC) $(CPPFLAGS) -DTEST $(CFLAGS) -o $$test $$test.c && ./$$test || exit 1; \
	done

clean:
	rm -f -- liboc.a liboc.so $(obj) $(tst)

distclean: clean
	rm -f -- .depend .sparse byteorder.o

install: liboc.a liboc.so
	install -d $(DESTDIR)$(PREFIX)$(INCDIR)/OC
	install -m 644 $(hdr) $(DESTDIR)$(PREFIX)$(INCDIR)/OC
	
	install -d $(DESTDIR)$(PREFIX)$(LIBDIR)
	install -m 644 liboc.a $(DESTDIR)$(PREFIX)$(LIBDIR)
	install -m 755 liboc.so $(DESTDIR)$(PREFIX)$(LIBDIR)

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

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: all check clean distclean
.SUFFIXES: .c .o
