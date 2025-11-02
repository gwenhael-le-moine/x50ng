#ifndef GDBSTUB_H
#define GDBSTUB_H

#include <stdbool.h>

#include "qemu/qemu-common.h"

#define DEFAULT_GDBSTUB_PORT 1234

/* GDB breakpoint/watchpoint types */
#define GDB_BREAKPOINT_SW 0
#define GDB_BREAKPOINT_HW 1
#define GDB_WATCHPOINT_WRITE 2
#define GDB_WATCHPOINT_READ 3
#define GDB_WATCHPOINT_ACCESS 4

typedef void ( *gdb_syscall_complete_cb )( CPUState* env, target_ulong ret, target_ulong err );

/* Get or set a register.  Returns the size of the register.  */
typedef int ( *gdb_reg_cb )( CPUState* env, uint8_t* buf, int reg );
extern void gdb_register_coprocessor( CPUState* env, gdb_reg_cb get_reg, gdb_reg_cb set_reg, int num_regs, const char* xml, int g_pos );

extern void gdb_do_syscall( gdb_syscall_complete_cb cb, const char* fmt, ... );
extern bool use_gdb_syscalls( void );
extern void gdb_set_stop_cpu( CPUState* env );

extern bool gdb_poll( CPUState* );

/* extern int gdb_queuesig( void ); */
extern int gdb_handlesig( CPUState*, int );
extern void gdb_exit( CPUState*, int );
extern void gdb_signalled( CPUState*, int );
extern int gdbserver_start( int );
extern void gdbserver_fork( CPUState* );

extern bool gdbserver_isactive();

#endif
