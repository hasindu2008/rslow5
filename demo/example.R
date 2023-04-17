library(rslow5)

##################################################################

# open a S/BLOW5 file
file <- rslow5_open("inst/extdata/example.slow5")

# print the header
hdr <- rslow5_header(file)
print(hdr)

###### sequential access ######

while (TRUE) {
    slow5 <- rslow5_get_next(file)
    if (length(slow5$read_id) == 0) {
        break
    }
    print(head(slow5,n=7)) #primary fields except raw signal
}

###### random access ######

#load the index
rslow5_idx_load(file)

# get readIDs r1,r3, r5
rid <- c("r1", "r3", "r5")
slow5 <- rslow5_get(file, rid)
print(head(slow5,n=7))

# now get read IDs r2 and r1
rid <- c("r2", "r1")
slow5 <- rslow5_get(file, rid)
print(head(slow5,n=7))

# unload the index at the end
rslow5_idx_unload(file)

##################################################################

# Open a different S/BLOW5 file
file <- rslow5_open("inst/extdata/example2.slow5")

# get the header
hdr <- rslow5_header(file)
print(hdr)

# sequentially read, this time with two threads and a batch size of 2
while (TRUE) {
    slow5 <- rslow5_get_next(file, num_thread = 2, batch_size = 2)
    if (length(slow5$read_id) == 0) {
        break
    }
    print(tail(slow5,n=5)) #aux fields
}

##################################################################
