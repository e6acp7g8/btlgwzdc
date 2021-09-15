#ifndef SHD_SHIM_SHIM_EVENT_H_
#define SHD_SHIM_SHIM_EVENT_H_

// Communication between Shadow and the shim. This is a header-only library
// used in both places.

#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>

#include "main/host/syscall_types.h"
#include "main/shmem/shmem_allocator.h"

// Shared state between Shadow and a plugin-thread. The shim-side code can modify
// directly; synchronization is achieved via the Shadow/Plugin IPC mechanisms
// (ptrace-stops and the shim IPC locking).
typedef struct _ShimSharedMem {
    // While true, Shadow allows syscalls to be executed natively.
    bool ptrace_allow_native_syscalls;
    // Store the latest simulation time to avoid inter-process time syscalls.
    struct timespec sim_time;
} ShimSharedMem;

// Returns 0 on success. Non-zero and sets errno on failure.
int shadow_set_ptrace_allow_native_syscalls(bool val);

// Returns 0 on success. Non-zero and sets errno on failure.
int shadow_get_ipc_blk(ShMemBlockSerialized* ipc_blk_serialized);

// Returns 0 on success. Non-zero and sets errno on failure.
int shadow_get_shm_blk(ShMemBlockSerialized* shm_blk_serialized);

// Asks shadow to find the ipv4 `addr` associated with hostname `name`.
// Return 0 if an address was found and written to addr. Returns -1 and sets errno on failure.
int shadow_hostname_to_addr_ipv4(const char* name, size_t name_len, uint32_t* addr,
                                 size_t addr_len);

typedef enum {
    // Next val: 13
    SHD_SHIM_EVENT_NULL = 0,
    SHD_SHIM_EVENT_START = 1,
    SHD_SHIM_EVENT_STOP = 2,
    SHD_SHIM_EVENT_SYSCALL = 3,
    SHD_SHIM_EVENT_SYSCALL_COMPLETE = 4,
    SHD_SHIM_EVENT_SYSCALL_DO_NATIVE = 8,
    SHD_SHIM_EVENT_CLONE_REQ = 5,
    SHD_SHIM_EVENT_CLONE_STRING_REQ = 9,
    SHD_SHIM_EVENT_SHMEM_COMPLETE = 6,
    SHD_SHIM_EVENT_WRITE_REQ = 7,
    SHD_SHIM_EVENT_BLOCK = 10,
    SHD_SHIM_EVENT_ADD_THREAD_REQ = 11,
    SHD_SHIM_EVENT_ADD_THREAD_PARENT_RES = 12,
} ShimEventID;

typedef struct _ShimEvent {
    ShimEventID event_id;

    union {
        struct {
            // Update shim-side simulation clock
            uint64_t simulation_nanos;
        } start;

        struct {
            struct timespec ts;
        } data_nano_sleep;

        int rv; // TODO (anonymized) hack, remove me

        struct {
            // We wrap this in the surrounding struct in case there's anything
            // else we end up needing in the message besides the literal struct
            // we're going to pass to the syscall handler.
            SysCallArgs syscall_args;
        } syscall;

        struct {
            SysCallReg retval;
            // Update shim-side simulation clock
            uint64_t simulation_nanos;
        } syscall_complete;

        struct {
            ShMemBlockSerialized serial;
            PluginPtr plugin_ptr;
            size_t n;
        } shmem_blk;

        struct {
            ShMemBlockSerialized ipc_block;
        } add_thread_req;
    } event_data;

} ShimEvent;

#endif // SHD_SHIM_SHIM_EVENT_H_
