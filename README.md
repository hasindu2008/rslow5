# rslow5

rslow5 is an R wrapper for [slow5lib](https://github.com/hasindu2008/slow5lib) for reading [S/BLOW5](https://github.com/hasindu2008/slow5lib) files. It is under development and the API interface is subject to change. I am not a R user, so [feedback and suggestions from R users are appreciated](https://github.com/hasindu2008/rslow5/issues) to make the API interface R-user-friendly. Features are currently minimal and will be implemented based on demand which you can request [here](https://github.com/hasindu2008/rslow5/issues).

Note that the version of rslow5 is independent of the slow5lib.

# Installation

```
# clone
git clone --recursive https://github.com/hasindu2008/rslow5 && cd rslow5

# build the package
touch src/slow5lib/lib/KEEP # a hack till I find a propoer way to prevent R builder from not including empty directories
R CMD build ./

# install
R CMD INSTALL rslow5_*.tar.gz # you may specify -l /custom/path if you do not have permission
```

# Usage

See the example at [demo/example.R](demo/example.R).

You can run it as `R --vanilla < demo/example.R` or simply `Rscript -e "library(rslow5); demo(example)"`.   If you installed rslow5 to a custom path, make sure to first do `export R_LIBS_USER=/custom/path`.


## Acknowledgement


Thanks to to this great [tutorial](https://www.r-bloggers.com/2021/07/using-r-packaging-a-c-library-in-15-minutes-2/) and assistance from chatGPT.
