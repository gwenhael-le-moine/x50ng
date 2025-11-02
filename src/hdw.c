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

    if ( hdw_state->arm_idle == HDW_ARM_RUN )
        hdw_state->env->halted = 0;
    else {
        hdw_state->env->halted = 1;
        cpu_exit( hdw_state->env );
    }
}

void hdw_stop( hdw_t* hdw_state )
{
    hdw_state->arm_exit = 1;
    cpu_exit( hdw_state->env );
}
