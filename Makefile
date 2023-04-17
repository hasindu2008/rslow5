.PHONY: clean test build check

all: build

build:
	-mkdir tmp/
	touch src/slow5lib/lib/KEEP
	R CMD build ./
	R CMD INSTALL -c --preclean -l ./tmp/ rslow5_*.tar.gz

check: build
	R CMD check rslow5_*.tar.gz

clean:
	rm -rf rslow5_*.tar.gz rslow5.Rcheck
	$(MAKE) -C src/slow5lib clean
	-R CMD REMOVE rslow5 -l ./tmp/

test: build
	test/test.sh

valgrind: build
	test/test.sh mem=1