#include "elev.h"
#include "order_controller.h"
#include "state_machine.h"
#include <stdio.h>
//#include <stdlib.h>


int main() {
    // Initialize hardware
    if (!initialize_state()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }
    printf("Pull 'OBSTRUKSJON' lever to turn stop the elevator and exit program.\n");

    while (1) {
        // Stop elevator and exit program if the stop button is pressed

        if (elev_get_obstruction_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            clear_all_orders();
            elev_set_door_open_lamp(0);
            elev_set_stop_lamp(0);
            break;
        }

        check_pressed_floor_button();
        determine_next_state();
      }
    return 0;
}
