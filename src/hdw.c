#include "hdw.h"
#include "types.h"

void hdw_set_idle( hdw_t* hdw_state, hdw_arm_idle_t idle )
{
#ifdef DEBUG_HDW_ARM_IDLE
    if ( idle != hdw_state->arm_idle ) {
        printf( "%s: arm_idle %u, idle %u\n", __func__, hdw_state->arm_idle, idle );
    }
#endif

    hdw_state->arm_idle = idle;

    hdw_state->cpu->halted = ( hdw_state->arm_idle != HDW_ARM_RUN );
    if ( hdw_state->cpu->halted )
        cpu_exit( hdw_state->cpu );
}

void hdw_stop( hdw_t* hdw_state )
{
    hdw_state->arm_exit = true;
    cpu_exit( hdw_state->cpu );
}
