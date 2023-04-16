/***************************************************************************
 R callable function
 ***************************************************************************/
#include <R.h>
#include <Rdefines.h>
#include <slow5/slow5.h>
#include <slow5/slow5_mt.h>
#include <stdio.h>
#include <stdlib.h>


void rslow5_close(SEXP ptr) {
    slow5_file_t *file = (slow5_file_t*) R_ExternalPtrAddr(ptr);
    if (file != NULL) {
        slow5_close(file);
        R_ClearExternalPtr(ptr);
    }
}

SEXP rslow5_open(SEXP pathname, SEXP mode) {
    const char *pathname_c = CHAR(STRING_ELT(pathname, 0));
    const char *mode_c = CHAR(STRING_ELT(mode, 0));

    if(strcmp(mode_c, "r") != 0){
        Rf_error("Only 'r' is supported yet\n");
    }

    slow5_file_t *sp = slow5_open(pathname_c, mode_c);
    if(sp==NULL){
        Rf_error("Error in opening file\n");
    }
    SEXP ptr = PROTECT(R_MakeExternalPtr(sp, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(ptr, rslow5_close, TRUE);
    UNPROTECT(1);
    return ptr;
}


SEXP rslow5_header(SEXP sp_ptr){

    SEXP df;

    slow5_file_t* sp = (slow5_file_t*) R_ExternalPtrAddr(sp_ptr);
    if (sp == NULL) {
        Rf_error("External pointer is invalid or has been destroyed\n");
    }

    const slow5_hdr_t* header = sp->header;

    uint64_t num_keys = 0;
    const char **keys = slow5_get_hdr_keys(header, &num_keys);
    if(keys==NULL){
        fprintf(stderr,"Error in getting header keys\n");
        exit(EXIT_FAILURE);
    }

    uint32_t num_read_groups = header->num_read_groups;
    fprintf(stderr,"num_read_groups: %d\n", num_read_groups);

    // Allocate result data frame
    PROTECT(df = allocVector(VECSXP, num_keys));

    // Allocate column vectors
    for (int i = 0; i < num_keys; i++) {
        SET_VECTOR_ELT(df, i, allocVector(STRSXP, num_read_groups));
    }

    // Allocate jey names
    SEXP names = PROTECT(allocVector(STRSXP, num_keys));

    for(int i=0; i<num_keys; i++) {
        printf("%s\t",keys[i]);
        SET_STRING_ELT(names, i, mkChar(keys[i]));
        //SET_VECTOR_ELT(df, i, allocVector(STRSXP, num_read_groups));

        for(int j=0; j<num_read_groups; j++){
            char* value = slow5_hdr_get(keys[i], j, header);
            if(value){
                //printf("%s\t",value);
                SET_STRING_ELT(VECTOR_ELT(df, i), j, mkChar(value));
            }else{
                //printf(".\t");
                SET_STRING_ELT(VECTOR_ELT(df, i), j, mkChar("."));
            }

        }
        //printf("\n");
    }

	free(keys);

    setAttrib(df, R_NamesSymbol, names);
    UNPROTECT(2);

    return df;
}

SEXP rslow5_idx_unload(SEXP sp_ptr){
    slow5_file_t* sp = (slow5_file_t*) R_ExternalPtrAddr(sp_ptr);
    if (sp == NULL) {
        Rf_error("External pointer is invalid or has been destroyed\n");
    }
    fprintf(stderr, "Unloading index\n");
    slow5_idx_unload(sp);
    return R_NilValue;
}

SEXP rslow5_idx_load(SEXP sp_ptr){
    slow5_file_t* sp = (slow5_file_t*) R_ExternalPtrAddr(sp_ptr);
    if (sp == NULL) {
        Rf_error("External pointer is invalid or has been destroyed\n");
    }
    int ret = slow5_idx_load(sp);
    if(ret < 0){
        Rf_error("Error in loading index\n");
    }
    return R_NilValue;
}


static char *get_channel_number(slow5_rec_t *rec){ //char*
    int ret=0;
    uint64_t len;
    char* str = slow5_aux_get_string(rec, "channel_number", &len, &ret);
    if(ret!=0){
        Rf_error("Error in getting auxiliary field %s from the file. Error code %d\n","channel_number",ret);
    }
    return str;
}
static double get_median_before(slow5_rec_t *rec){ //double
    int ret=0;
    double t = slow5_aux_get_double(rec, "median_before", &ret);
    if(ret!=0){
        Rf_error("Error in getting auxiliary field %s from the file. Error code %d\n","median_before",ret);
    }
    return t;
}
static int32_t get_read_number(slow5_rec_t *rec){ //int32_t
    int ret=0;
    int32_t t = slow5_aux_get_int32(rec, "read_number", &ret);
    if(ret!=0){
        Rf_error("Error in getting auxiliary field %s from the file. Error code %d\n","read_number",ret);
    }
    return t;
}
static uint8_t get_start_mux(slow5_rec_t *rec){ //uint8_t
    int ret=0;
    uint8_t t = slow5_aux_get_uint8(rec, "start_mux", &ret);
    if(ret!=0){
        Rf_error("Error in getting auxiliary field %s from the file. Error code %d\n","start_mux",ret);
    }
    return t;
}

static uint64_t get_start_time(slow5_rec_t *rec){ //uint64_t
    int ret=0;
    uint64_t t = slow5_aux_get_uint64(rec, "start_time", &ret);
    if(ret!=0){
        Rf_error("Error in getting auxiliary field %s from the file. Error code %d\n","start_time",ret);
    }
    return t;
}

SEXP rslow5_get_core(SEXP sp_ptr, SEXP numthread, SEXP batchsize, int8_t rand, SEXP list) {

    // Parse optional arguments
    int num_thread = 4;
    if (length(numthread) > 0) {
        num_thread = INTEGER(numthread)[0];
    }
    int batch_size = 4000;
    if (length(batchsize) > 0) {
        batch_size = INTEGER(batchsize)[0];
    }

    // Extract slow5_file_t pointer from external pointer
    slow5_file_t* sp = (slow5_file_t*) R_ExternalPtrAddr(sp_ptr);
    if (sp == NULL) {
        Rf_error("External pointer is invalid or has been destroyed\n");
    }

    // Initialize multithreaded reader and batch
    slow5_mt_t* mt = slow5_init_mt(num_thread, sp);
    slow5_batch_t* read_batch = slow5_init_batch(batch_size);

    // Get next batch
    int nrecs = 0;

    if(rand == 1){
        /* convert R object to C array */
        int num_rid = length(list);
        if(num_rid <= 0){
            Rf_error("Read id list must have at least one element\n");
        }
        char **rid = (char **)malloc(num_rid * sizeof(char *));
        if(rid == NULL){
            Rf_error("Error in allocating memory\n");
        }
        for (int i = 0; i < num_rid; i++) {
            rid[i] = (char *)CHAR(STRING_ELT(list, i));
        }
        nrecs = slow5_get_batch(mt, read_batch, rid, num_rid);
        free(rid);
        if(nrecs != num_rid){
            Rf_error("Error in getting batch. %d requested, only %d found\n", num_rid, nrecs);
        }
    }else{
        nrecs = slow5_get_next_batch(mt, read_batch, batch_size);
    }

    fprintf(stderr, "nrecs: %d\n", nrecs);

    // Create R vectors for each column of the data frame
    // data types make consistent with C
    SEXP read_id = PROTECT(allocVector(STRSXP, nrecs));
    SEXP read_group = PROTECT(allocVector(INTSXP, nrecs));
    SEXP digitisation = PROTECT(allocVector(REALSXP, nrecs));
    SEXP offset = PROTECT(allocVector(REALSXP, nrecs));
    SEXP range = PROTECT(allocVector(REALSXP, nrecs));
    SEXP sampling_rate = PROTECT(allocVector(REALSXP, nrecs));
    SEXP len_raw_signal = PROTECT(allocVector(REALSXP, nrecs)); //long int?
    SEXP raw_signal = PROTECT(allocVector(VECSXP, nrecs));

    //static aux
    SEXP channel_number = PROTECT(allocVector(STRSXP, nrecs));
    SEXP median_before = PROTECT(allocVector(REALSXP, nrecs));
    SEXP read_number = PROTECT(allocVector(INTSXP, nrecs));
    SEXP start_mux = PROTECT(allocVector(INTSXP, nrecs));
    SEXP start_time = PROTECT(allocVector(REALSXP, nrecs));

    uint64_t num_aux = 0;
    char **aux = slow5_get_aux_names(sp->header, &num_aux);

    for (int i = 0; i < nrecs; i++) {
        slow5_rec_t *rec = read_batch->slow5_rec[i];
        SET_STRING_ELT(read_id, i, mkChar(rec->read_id));
        INTEGER(read_group)[i] = rec->read_group;
        REAL(digitisation)[i] = rec->digitisation;
        REAL(offset)[i] = rec->offset;
        REAL(range)[i] = rec->range;
        REAL(sampling_rate)[i] = rec->sampling_rate;
        REAL(len_raw_signal)[i] = rec->len_raw_signal;

        SEXP raw_signal_i = PROTECT(allocVector(REALSXP, rec->len_raw_signal)); //int?
        for (uint64_t j = 0; j < rec->len_raw_signal; j++) {
            REAL(raw_signal_i)[j] = rec->raw_signal[j];
        }
        SET_VECTOR_ELT(raw_signal, i, raw_signal_i);
        UNPROTECT(1);

        fprintf(stderr, "read_id: %s\n", rec->read_id);
        if(aux != NULL){
            SET_STRING_ELT(channel_number, i, mkChar(get_channel_number(rec)));
            REAL(median_before)[i] = get_median_before(rec);
            INTEGER(read_number)[i] = get_read_number(rec);
            INTEGER(start_mux)[i] = get_start_mux(rec);
            REAL(start_time)[i] = get_start_time(rec);
        }
    }

    // Free memory
    slow5_free_batch(read_batch);
    slow5_free_mt(mt);


    int cols = aux == NULL ? 8 : 8 + 5;

    // create a data frame from the vectors
    SEXP df = PROTECT(allocVector(VECSXP, cols));
    SET_VECTOR_ELT(df, 0, read_id);
    SET_VECTOR_ELT(df, 1, read_group);
    SET_VECTOR_ELT(df, 2, digitisation);
    SET_VECTOR_ELT(df, 3, offset);
    SET_VECTOR_ELT(df, 4, range);
    SET_VECTOR_ELT(df, 5, sampling_rate);
    SET_VECTOR_ELT(df, 6, len_raw_signal);
    SET_VECTOR_ELT(df, 7, raw_signal);
    if(aux != NULL){
        SET_VECTOR_ELT(df, 8, channel_number);
        SET_VECTOR_ELT(df, 9, median_before);
        SET_VECTOR_ELT(df, 10, read_number);
        SET_VECTOR_ELT(df, 11, start_mux);
        SET_VECTOR_ELT(df, 12, start_time);
    }

    // set the names of the data frame
    SEXP names = PROTECT(allocVector(STRSXP, cols));
    SET_STRING_ELT(names, 0, mkChar("read_id"));
    SET_STRING_ELT(names, 1, mkChar("read_group"));
    SET_STRING_ELT(names, 2, mkChar("digitisation"));
    SET_STRING_ELT(names, 3, mkChar("offset"));
    SET_STRING_ELT(names, 4, mkChar("range"));
    SET_STRING_ELT(names, 5, mkChar("sampling_rate"));
    SET_STRING_ELT(names, 6, mkChar("len_raw_signal"));
    SET_STRING_ELT(names, 7, mkChar("raw_signal"));

    if(aux != NULL){
        SET_STRING_ELT(names, 8, mkChar("channel_number"));
        SET_STRING_ELT(names, 9, mkChar("median_before"));
        SET_STRING_ELT(names, 10, mkChar("read_number"));
        SET_STRING_ELT(names, 11, mkChar("start_mux"));
        SET_STRING_ELT(names, 12, mkChar("start_time"));
    }

    setAttrib(df, R_NamesSymbol, names);

    UNPROTECT(15);

    return df;
}

SEXP rslow5_get_next(SEXP sp_ptr, SEXP numthread, SEXP batchsize) {
    return rslow5_get_core(sp_ptr, numthread, batchsize, 0, NULL);
}

SEXP rslow5_get(SEXP sp_ptr, SEXP list, SEXP numthread, SEXP batchsize) {
    return rslow5_get_core(sp_ptr, numthread, batchsize, 1, list);
}

SEXP rslow5_load(SEXP pathname) {

    const char *path = CHAR(STRING_ELT(pathname, 0));

    //open the SLOW5 file
    slow5_file_t *sp = slow5_open(path,"r");
    if(sp==NULL){
       Rf_error("Error in opening file\n");
    }

    int ret = slow5_idx_load(sp);
    if(ret<0){
        Rf_error("Error in loading index\n");
    }

    uint64_t nrecs = 0;
    char **read_ids = slow5_get_rids(sp, &nrecs);
    if(read_ids==NULL){
        Rf_error("Error in getting list of read IDs\n");
    }

    slow5_idx_unload(sp);

    slow5_rec_t *rec = NULL; //slow5 record to be read
    ret=0; //for return value

    // Create R vectors for each column of the data frame
    SEXP read_id = PROTECT(allocVector(STRSXP, nrecs));
    SEXP read_group = PROTECT(allocVector(INTSXP, nrecs));
    SEXP digitisation = PROTECT(allocVector(REALSXP, nrecs));
    SEXP offset = PROTECT(allocVector(REALSXP, nrecs));
    SEXP range = PROTECT(allocVector(REALSXP, nrecs));
    SEXP sampling_rate = PROTECT(allocVector(REALSXP, nrecs));
    SEXP len_raw_signal = PROTECT(allocVector(REALSXP, nrecs));
    SEXP raw_signal = PROTECT(allocVector(VECSXP, nrecs));

    int64_t i = 0;

    fprintf(stderr, "nrecs: %ld\n", nrecs);

    //iterate through the file until end
    while((ret = slow5_get_next(&rec,sp)) >= 0){
        SET_STRING_ELT(read_id, i, mkChar(rec->read_id));
        INTEGER(read_group)[i] = rec->read_group;
        REAL(digitisation)[i] = rec->digitisation;
        REAL(offset)[i] = rec->offset;
        REAL(range)[i] = rec->range;
        REAL(sampling_rate)[i] = rec->sampling_rate;
        REAL(len_raw_signal)[i] = rec->len_raw_signal;

        SEXP raw_signal_i = PROTECT(allocVector(REALSXP, rec->len_raw_signal));
        for (uint64_t j = 0; j < rec->len_raw_signal; j++) {
            REAL(raw_signal_i)[j] = rec->raw_signal[j];
        }
        SET_VECTOR_ELT(raw_signal, i, raw_signal_i);
        UNPROTECT(1);

        fprintf(stderr, "read_id: %s\n", rec->read_id);

        // printf("%s\t",rec->read_id);
        // uint64_t len_raw_signal = rec->len_raw_signal;
        // for(uint64_t i=0;i<len_raw_signal;i++){ //iterate through the raw signal and print in picoamperes
        //     double pA = TO_PICOAMPS(rec->raw_signal[i],rec->digitisation,rec->offset,rec->range);
        //     printf("%f ",pA);
        // }
        // printf("\n");
        i++;
        if(i>nrecs){
            Rf_error("Error in slow5_get_next. Number of records read exceeds number of records in index\n");
        }
    }

    if(ret != SLOW5_ERR_EOF){  //check if proper end of file has been reached
        Rf_error("Error in slow5_get_next. Error code %d\n",ret);
    }

    if(i<nrecs){
        Rf_error("Error in slow5_get_next. Number of records read is less than number of records in index\n");
    }

    //free the SLOW5 record
    slow5_rec_free(rec);

    //close the SLOW5 file
    slow5_close(sp);

    // create a data frame from the vectors
    SEXP df = PROTECT(allocVector(VECSXP, 8));
    SET_VECTOR_ELT(df, 0, read_id);
    SET_VECTOR_ELT(df, 1, read_group);
    SET_VECTOR_ELT(df, 2, digitisation);
    SET_VECTOR_ELT(df, 3, offset);
    SET_VECTOR_ELT(df, 4, range);
    SET_VECTOR_ELT(df, 5, sampling_rate);
    SET_VECTOR_ELT(df, 6, len_raw_signal);
    SET_VECTOR_ELT(df, 7, raw_signal);

    // set the names of the data frame
    SEXP names = PROTECT(allocVector(STRSXP, 8));
    SET_STRING_ELT(names, 0, mkChar("read_id"));
    SET_STRING_ELT(names, 1, mkChar("read_group"));
    SET_STRING_ELT(names, 2, mkChar("digitisation"));
    SET_STRING_ELT(names, 3, mkChar("offset"));
    SET_STRING_ELT(names, 4, mkChar("range"));
    SET_STRING_ELT(names, 5, mkChar("sampling_rate"));
    SET_STRING_ELT(names, 6, mkChar("len_raw_signal"));
    SET_STRING_ELT(names, 7, mkChar("raw_signal"));
    setAttrib(df, R_NamesSymbol, names);

    UNPROTECT(10);

    return(df);
}