#ifndef _X50NG_EMULATOR_H
#  define _X50NG_EMULATOR_H 1

#  include "types.h"
#  include "x50ng.h"

void x50ng_set_idle( x50ng_t*, x50ng_arm_idle_t idle );

void emulator_init( void );
void emulator_exit( void );

#endif /* !(_X50NG_EMULATOR_H) */
