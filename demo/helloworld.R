library(rslow5)
# Load the slow5 file
slow5 <- rslow5_load("inst/extdata/example.slow5")
# Print the slow5 object
head(slow5)
# # Print the slow5 object as a data.frame
# print(as.data.frame(slow5))
# # Print the slow5 object as a data.frame with the read_id column as the rownames
# print(as.data.frame(slow5, rownames = "read_id"))
