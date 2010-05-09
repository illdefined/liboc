all: liboc.a liboc.so

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

-include .depend

endif
endif

CC       := $(shell if which clang 2>&1 >/dev/null; then echo clang; else echo gcc; fi)
CPP      := $(CC) -E
LD       := ld

CPPFLAGS := -std=gnu99
CFLAGS   := -pipe -O2 -Wall
CFLAGS   += -fmerge-all-constants -fstrict-overflow
CFLAGS   += -frename-registers -fPIC -fno-common
LDFLAGS  := -O1 -shared

src      := endian.c hardtick.c rotate.c
obj      := $(src:.c=.o)

clean:
	rm -f -- liboc.a liboc.so $(obj)

distclean: clean
	rm -f -- .depend byteorder.o

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

liboc.a: .depend $(obj)
	$(AR) rc $@ $(obj)

liboc.so: .depend $(obj)
	$(LD) $(LDFLAGS) -o $@ $(obj)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.pyo.py:
	python -O -m compileall $<

.PHONY: all clean distclean
.SUFFIXES: .c .o .py .pyo
