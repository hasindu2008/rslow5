/***************************************************************************
 R callable function
 ***************************************************************************/
#include <R.h>
#include <Rdefines.h>
#include <slow5/slow5.h>
#include <stdio.h>
#include <stdlib.h>


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