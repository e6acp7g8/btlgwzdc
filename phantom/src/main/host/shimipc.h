/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

/*
 * Shadow glue/helpers for communicating with the shim.
 */

#include <stdbool.h>
#include <sys/types.h>

// Whether to send an explicit message to the shim when its plugin is blocked.
bool shimipc_sendExplicitBlockMessageEnabled();

// Number of iterations to spin when waiting on IPC between Shadow and the shim
// before blocking.
ssize_t shimipc_spinMax();


// Whether to use a seccomp filter in the shim to catch syscalls that would
// otherwise not be interposed.
bool shimipc_getUseSeccomp();