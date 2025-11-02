#ifndef _X50NG_MODULE_H
#  define _X50NG_MODULE_H 1

#  include "types.h"
#  include "hdw.h"

int x50ng_module_init( hdw_t* hdw_state, const char* name, int ( *init )( hdw_module_t* ), int ( *exit )( hdw_module_t* ),
                       int ( *reset )( hdw_module_t*, hdw_reset_t ), int ( *load )( hdw_module_t*, GKeyFile* ),
                       int ( *save )( hdw_module_t*, GKeyFile* ), void* user_data, hdw_module_t** module );

int module_register( hdw_module_t* module );
int module_unregister( hdw_module_t* module );

int module_get_filename( hdw_module_t* module, GKeyFile*, const char*, char*, char**, char** );
int module_set_filename( hdw_module_t* module, GKeyFile*, const char*, const char* );
int module_get_int( hdw_module_t* module, GKeyFile*, const char*, int, int* );
int module_set_int( hdw_module_t* module, GKeyFile*, const char*, int );
int module_get_u32( hdw_module_t* module, GKeyFile*, const char*, uint32_t, uint32_t* );
int module_set_u32( hdw_module_t* module, GKeyFile*, const char*, uint32_t );
int module_get_string( hdw_module_t* module, GKeyFile*, const char*, char*, char** );
int module_set_string( hdw_module_t* module, GKeyFile*, const char*, const char* );
int module_open_rodata( hdw_module_t* module, const char* name, char** path );

int init_modules( hdw_t* );
int exit_modules( hdw_t* );
int reset_modules( hdw_t*, hdw_reset_t );
int load_modules( hdw_t* );
int save_modules( hdw_t* );

#endif /* !(_X50NG_MODULE_H) */
