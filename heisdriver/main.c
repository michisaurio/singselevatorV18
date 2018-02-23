#include "elev.h"
#include "button_lights.h"
#include "state_machine.h"
#include <stdio.h>
//#include <stdlib.h>


int main() {
    // Initialize hardware
    if (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }
    printf("Press STOP button to stop elevator and exit program.\n");

    while (1) {
        // Stop elevator and exit program if the stop button is pressed
        if (elev_get_stop_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            break;
        }

        determineNextState();
        checkPushedFloorButton();
      }
    return 0;
}
