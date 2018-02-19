#include "elev.h"
#include "button_lights.h"
#include "state_machine.h"
#include <stdio.h>


int main() {
    // Initialize hardware
    if (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }

    printf("Press STOP button to stop elevator and exit program.\n");

	elev_motor_direction_t motor_dir_memory = DIRN_STOP;
    elev_set_motor_direction(motor_dir_memory);
    
    state_type_t current_state = IDLE;
    
    int last_floor = elev_get_floor_sensor_signal();
    
    while (1) {
    	/*
        // Change direction when we reach top/bottom floor
        if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
            elev_set_motor_direction(DIRN_DOWN);
        } else if (elev_get_floor_sensor_signal() == 0) {
            elev_set_motor_direction(DIRN_UP);
        }

        // Stop elevator and exit program if the stop button is pressed
        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            break;
        }
        */
        
        //lamp button dance
        determineNextState(&current_state, &motor_dir_memory, &last_floor);
        checkPushedFloorButton();
        
	}     

    return 0;
}
