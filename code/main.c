/*
main.c
Main file for the elevator program.
First the hardware and elevator state are initialized.
Then new orders are registered and the elevator state is updated indefinitely.
*/
#include "order_controller.h"
#include "state_machine.h"
#include <stdio.h>

int
main()
{
  if(!initialize_state()) {
    printf ("Unable to initialize elevator hardware!\n");
    return 1;
  }
  while(1) {
    check_pressed_order_button();
    determine_next_state();
  }
  return 0;
}
