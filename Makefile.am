# Obvious program stuff.
bin_PROGRAMS = src/prwd
dist_man1_MANS = prwd.1

# Distribution tarball options.
EXTRA_DIST = \
	ChangeLog README TODO examples \
	array.h prwd.h prwd.1

all: src/prwd

src/prwd:
	cd src/ && make

test:
	cd src/ && make test

# install: src/prwd
# 	install -m 755 src/prwd $(DESTDIR)/bin
# 	install -m 644 prwd.1 $(DESTDIR)/man/man1
# 
clean:
	cd src/ && make clean

mantest:
	nroff -man prwd.1 | less -R

README: prwd.1
	nroff -man prwd.1 | col -b > README
