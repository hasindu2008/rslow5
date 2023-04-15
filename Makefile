.PHONY: slow5lib slow5lib clean test

all: slow5lib rslow5

slow5lib:
	$(MAKE) -C src/slow5lib zstd=$(zstd) no_simd=$(no_simd) zstd_local=$(zstd_local) lib/libslow5.a

rslow5:
	R CMD build ./
#	R CMD check rslow5_*.tar.gz
	R CMD INSTALL -c --preclean -l ~/R/ rslow5_*.tar.gz

clean:
	rm -rf rslow5_*.tar.gz rslow5.Rcheck
	$(MAKE) -C src/slow5lib clean
	R CMD REMOVE rslow5 -l ~/R/

test: rslow5
	test/test.sh