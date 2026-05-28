BINDIR=/usr/local/bin
LIBDIR=.
#DEBUG=-g
DEBUG=

BINARIES= xsv2xsv dos2unix paths convcrlf

.c.o:
	cc -c $(DEBUG) $<

all: $(BINARIES)

clean:
	rm -f $(BINARIES)
	rm *.o
	rm *.a
	rm -rf *.dSYM

install: all
	install -d -o root -g wheel $(BINDIR)
	install -c -o root -g wheel -m 0555 xsv2xsv $(BINDIR)
	install -l s -o root -g wheel $(BINDIR)/xsv2xsv $(BINDIR)/csv2tsv
	install -l s -o root -g wheel $(BINDIR)/xsv2xsv $(BINDIR)/tsv2csv
	install -c -o root -g wheel -m 0555 dos2unix $(BINDIR)
	install -l s -o root -g wheel $(BINDIR)/dos2unix $(BINDIR)/unix2dos
	install -c -o root -g wheel -m 0555 paths $(BINDIR)/paths
	install -c -o root -g wheel -m 0555 convcrlf $(BINDIR)/convcrlf


libmacfiletools.a: iohandler.o getseparatororquote.o
	ar -ur libmacfiletools.a iohandler.o getseparatororquote.o
	ranlib libmacfiletools.a
	
xsv2xsv: xsv2xsv.o libmacfiletools.a
	cc $(DEBUG) -o xsv2xsv xsv2xsv.o -L $(LIBDIR) -lmacfiletools

dos2unix: dos2unix.o libmacfiletools.a
	cc $(DEBUG) -o dos2unix dos2unix.o -L $(LIBDIR) -lmacfiletools

paths:	paths.o
	cc $(DEBUG) -o paths paths.o

convcrlf: convcrlf.o libmacfiletools.a
	cc $(DEBUG) -o convcrlf convcrlf.o -L $(LIBDIR) -lmacfiletools

