# wrapper function to invoke rslow5_load
rslow5_load <- function(pathname) {
  result <- .Call("rslow5_load",pathname)
  return(result)
}