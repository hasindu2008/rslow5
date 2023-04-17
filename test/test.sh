
#!/bin/bash


if [ "$1" = 'mem' ]; then
    mem=1
else
    mem=0
fi

# Run the tests
export R_LIBS_USER=./tmp

if [ $mem -eq 1 ]; then
    R --debugger=valgrind --debugger-args=--leak-check=full --vanilla < demo/example.R
else
    Rscript -e "library(rslow5); demo(example)"
fi