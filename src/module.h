#ifndef _X50NG_MODULE_H
#  define _X50NG_MODULE_H 1

#  include "types.h"
#  include "x50ng.h"

int x50ng_module_init( x50ng_t* x50ng, const char* name, int ( *init )( x50ng_module_t* ), int ( *exit )( x50ng_module_t* ),
                       int ( *reset )( x50ng_module_t*, x50ng_reset_t ), int ( *load )( x50ng_module_t*, GKeyFile* ),
                       int ( *save )( x50ng_module_t*, GKeyFile* ), void* user_data, x50ng_module_t** module );

int x50ng_module_register( x50ng_module_t* module );
int x50ng_module_unregister( x50ng_module_t* module );

int x50ng_module_get_filename( x50ng_module_t* module, GKeyFile*, const char*, char*, char**, char** );
int x50ng_module_set_filename( x50ng_module_t* module, GKeyFile*, const char*, const char* );
int x50ng_module_get_int( x50ng_module_t* module, GKeyFile*, const char*, int, int* );
int x50ng_module_set_int( x50ng_module_t* module, GKeyFile*, const char*, int );
int x50ng_module_get_u32( x50ng_module_t* module, GKeyFile*, const char*, uint32_t, uint32_t* );
int x50ng_module_set_u32( x50ng_module_t* module, GKeyFile*, const char*, uint32_t );
int x50ng_module_get_string( x50ng_module_t* module, GKeyFile*, const char*, char*, char** );
int x50ng_module_set_string( x50ng_module_t* module, GKeyFile*, const char*, const char* );
int x50ng_module_open_rodata( x50ng_module_t* module, const char* name, char** path );

int x50ng_modules_init( x50ng_t* );
int x50ng_modules_exit( x50ng_t* );
int x50ng_modules_reset( x50ng_t*, x50ng_reset_t );
int x50ng_modules_load( x50ng_t* );
int x50ng_modules_save( x50ng_t* );

#endif /* !(_X50NG_MODULE_H) */
