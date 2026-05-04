BINDIR=/usr/local/bin

all: xsv2xsv dos2unix paths

clean:
	rm -f xsv2xsv
	rm -f dos2unix
	rm -f paths

install: all
	install -d -o root -g wheel $(BINDIR)
	install -c -o root -g wheel -m 0555 xsv2xsv $(BINDIR)
	install -l s -o root -g wheel $(BINDIR)/xsv2xsv $(BINDIR)/csv2tsv
	install -l s -o root -g wheel $(BINDIR)/xsv2xsv $(BINDIR)/tsv2csv
	install -c -o root -g wheel -m 0555 dos2unix $(BINDIR)
	install -l s -o root -g wheel $(BINDIR)/dos2unix $(BINDIR)/unix2dos
	install -c -o root -g wheel -m 0555 paths $(BINDIR)/paths

xsv2xsv: xsv2xsv.c
	cc -o xsv2xsv xsv2xsv.c

dos2unix: dos2unix.c
	cc -o dos2unix dos2unix.c

paths: paths.c
	cc -o paths paths.c
