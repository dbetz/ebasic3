SRCS=\
ebasic.c \
db_compiler.c \
db_edit.c \
db_expr.c \
db_generate.c \
db_image.c \
db_scan.c \
db_statement.c \
db_symbols.c \
db_system.c \
db_vmdebug.c \
db_vmint.c \
editbuf.c \

POSIXSRCS=\
osint_posix.c

PROPSRCS=\
osint_prop2.c \
simple_vsnprintf.c

HDRS=\
db_compiler.h \
db_edit.h \
db_image.h \
db_inttypes.h \
db_system.h \
db_types.h \
db_vm.h \
db_vmdebug.h

SPIN=\
ebasic_vm.spin

CFLAGS=-D PROPELLER -D LINE_EDIT

ebasic:	$(SRCS) $(PROPSRCS) fastspin_stuff.c $(HDRS)
	fastspin -o $@ -2 $(CFLAGS) -D vsprintf=__simple_vsprintf $(SRCS) $(PROPSRCS) fastspin_stuff.c

eb_p2gcc:	$(SRCS) $(PROPSRCS) $(HDRS)
	p2gcc -o $@ $(CFLAGS) $(SRCS) $(PROPSRCS)

eb_mac:	$(SRCS) $(POSIXSRCS) $(HDRS)
	cc -Wall -o $@ -D MAC $(SRCS) $(POSIXSRCS)

eb_linux:	$(SRCS) $(POSIXSRCS) $(HDRS)
	cc -Wall -o $@ -D LINUX $(SRCS) $(POSIXSRCS)

zip:	$(SRCS) Makefile
	rm -f ebasic3.zip
	zip ebasic3 README.txt count.bas $(SRCS) $(PROPSRCS) $(POSIXSRCS) $(HDRS) $(SPIN) Makefile

clean:
	rm -f ebasic eb_p2gcc eb_mac *.o
