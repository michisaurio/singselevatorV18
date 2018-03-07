#include "elev.h"
#include "order_controller.h"
#include "state_machine.h"
#include <stdio.h>


int main() {
    // Initialize hardware
    if (!initialize_state()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }

    while (1) {
        check_pressed_order_button();
        determine_next_state();
      }
    return 0;
}
