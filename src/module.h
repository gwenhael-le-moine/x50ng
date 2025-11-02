#ifndef _X50NG_MODULE_H
#  define _X50NG_MODULE_H 1

#  include "options.h"
#  include "types.h"
#  include "hdw.h"

extern int x50ng_module_init( hdw_t* hdw_state, const char* name, int ( *init )( hdw_module_t* ), int ( *exit )( hdw_module_t* ),
                              int ( *reset )( hdw_module_t*, hdw_reset_t ), int ( *load )( hdw_module_t*, GKeyFile* ),
                              int ( *save )( hdw_module_t*, GKeyFile* ), void* user_data, hdw_module_t** module );

extern int module_register( hdw_module_t* module );
extern int module_unregister( hdw_module_t* module );

extern int module_get_filename( hdw_module_t* module, GKeyFile*, const char*, char*, char**, char** );
extern int module_set_filename( hdw_module_t* module, GKeyFile*, const char*, const char* );
extern int module_get_int( hdw_module_t* module, GKeyFile*, const char*, int, int* );
extern int module_set_int( hdw_module_t* module, GKeyFile*, const char*, int );
extern int module_get_u32( hdw_module_t* module, GKeyFile*, const char*, uint32_t, uint32_t* );
extern int module_set_u32( hdw_module_t* module, GKeyFile*, const char*, uint32_t );
extern int module_get_string( hdw_module_t* module, GKeyFile*, const char*, char*, char** );
extern int module_set_string( hdw_module_t* module, GKeyFile*, const char*, const char* );
extern int module_open_rodata( hdw_module_t* module, const char* name, char** path );

extern int init_modules( hdw_t*, config_t* );
extern int exit_modules( hdw_t* );
extern int reset_modules( hdw_t*, hdw_reset_t );
extern int load_modules( hdw_t* );
extern int save_modules( hdw_t* );

#endif /* !(_X50NG_MODULE_H) */
