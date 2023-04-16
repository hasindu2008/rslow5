library(rslow5)
# Load the slow5 file
# slow5 <- rslow5_load("inst/extdata/example.slow5")
# head(slow5)

file <- rslow5_open("inst/extdata/example.slow5")

hdr <- rslow5_header(file)
print(hdr)

while (TRUE) {
    slow5 <- rslow5_get_next(file, num_thread = 2, batch_size = 2)
    if (length(slow5$read_id) == 0) {
        break
    }
    print(head(slow5,n=7))
}

rslow5_idx_load(file)
rid <- c("r1", "r3", "r5")
slow5 <- rslow5_get(file, rid, num_thread = 2, batch_size = 2)
print(head(slow5,n=7))
rslow5_idx_unload(file)


file <- rslow5_open("inst/extdata/example2.slow5")
hdr <- rslow5_header(file)
print(hdr)

while (TRUE) {
    slow5 <- rslow5_get_next(file, num_thread = 2, batch_size = 2)
    if (length(slow5$read_id) == 0) {
        break
    }
    print(tail(slow5,n=5))
}