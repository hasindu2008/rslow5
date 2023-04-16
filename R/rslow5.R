
rslow5_open <- function(pathname, mode = "r") {
  result <- .Call("rslow5_open", pathname, mode)
  return(result)
}

rslow5_header <- function(file_ptr) {
  result <- .Call("rslow5_header",file_ptr)
  return(result)
}

rslow5_get_next <- function(file_ptr, num_thread = 4, batch_size = 4000) {
  result <- .Call("rslow5_get_next", file_ptr, as.integer(num_thread), as.integer(batch_size))
  return(result)
}

rslow5_get <- function(file_ptr, rid_list, num_thread = 4, batch_size = 4000) {
  result <- .Call("rslow5_get", file_ptr, rid_list, as.integer(num_thread), as.integer(batch_size))
  return(result)
}

# wrapper function to invoke rslow5_load
rslow5_load <- function(pathname) {
  result <- .Call("rslow5_load",pathname)
  return(result)
}

# wrapper function to invoke rslow5_close
rslow5_close <- function(file_ptr) {
  result <- .Call("rslow5_close",file_ptr)
  return(result)
}

# rslow5_idx_load
rslow5_idx_load <- function(file_ptr) {
  result <- .Call("rslow5_idx_load",file_ptr)
  return(result)
}

# rslow5_idx_unload
rslow5_idx_unload <- function(file_ptr) {
  result <- .Call("rslow5_idx_unload",file_ptr)
  return(result)
}