// order_controller.c
#include "io.h"
#include "elev.h"
#include <stdio.h>

/* N_BUTTONS is defined as 3 in elev.c. We could move this definition to elev.h,
   but it was decided not to modify the given files.  */
#if !defined(N_BUTTONS)
#define N_BUTTONS 3
#endif

/* order_register_matrix[f][b]=1 if and only if there is an order for floor f
   and button b.
   order_register_matrix[f][b]=0 if and only if there is no order placed
   for floor f and button b, or such an order cannot exist.  */
static int order_register_matrix[N_BUTTONS][N_FLOORS] = { }; // initialized with 0.

void
check_pressed_order_button()
{
  for (elev_button_type_t button = 0; button < N_BUTTONS; button++) {
    for (int floor = 0; floor < N_FLOORS; floor++) {
      // Unvalid button and floor combinations are avoided.
      if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)
	        && !(button == BUTTON_CALL_DOWN && floor == 0)) {
      	if (elev_get_button_signal(button, floor)) {
      	  order_register_matrix[button][floor] = 1;
      	  elev_set_button_lamp(button, floor, 1);
      	}
      }
    }
  }
}

int
get_order_status(elev_button_type_t button, int floor)
{
  return order_register_matrix[button][floor];
}

void
clear_order_status(elev_button_type_t button, int floor)
{
  // Unvalid button and floor combinations are avoided.
  if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)
      && !(button == BUTTON_CALL_DOWN && floor == 0)) {
    order_register_matrix[button][floor] = 0;
    elev_set_button_lamp(button, floor, 0);
  }
}

void
clear_all_orders()
{
  for (elev_button_type_t button = 0; button < N_BUTTONS; button++) {
    for (int floor = 0; floor < N_FLOORS; floor++) {
      clear_order_status(button, floor);
    }
  }
}

int
is_order_upstairs(int current_floor)
{
  for (elev_button_type_t button = 0; button < N_BUTTONS; button++) {
    for (int floor = N_FLOORS - 1; floor > current_floor; floor--) {
      if (get_order_status(button, floor)) {
        return 1;
      }
    }
  }
  return 0;
}

int
is_order_downstairs(int current_floor)
{
  for (elev_button_type_t button = 0; button < N_BUTTONS; button++) {
    for (int floor = 0; floor < current_floor; floor++) {
      if (get_order_status(button, floor)) {
        return 1;
      }
    }
  }
  return 0;
}
