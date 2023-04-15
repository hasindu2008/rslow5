
#!/bin/bash

cd ..
R CMD build rslow5
sudo R CMD INSTALL rslow5
cd rslow5

# Run the tests
Rscript -e "library(rslow5); demo(helloworld)"