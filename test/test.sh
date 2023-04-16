
#!/bin/bash

# Run the tests
export R_LIBS_USER=~/R
Rscript -e "library(rslow5); demo(helloworld)"