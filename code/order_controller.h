/* order_controller.h
   The order_controller module bookkeeps the placed and served orders. */
   
#include "elev.h"

/* Checks if any of the order buttons have been pressed. The lights of the
   pressed buttons are turned on, and the orders are saved.  */
void
check_pressed_order_button();

/* Returns 1 if there is an order for the particular combination of button and
   floor. Returns 0 if not or if that particular combination does not exist.  */
int
get_order_status(elev_button_type_t button, int floor);

/* Erases the order for the particular combination of button and floor equal,
   and turns off the corresponding light.  */
void
clear_order_status(elev_button_type_t button, int floor);

/* Applies clear_order_status(button, floor) to all possible combinations
   of button and floor.  */
void
clear_all_orders();

/* Returns 1 if there is a cab order to a floor above current_floor or if there
   is a hall order at a floor above current_floor. Returns 0 otherwise.  */
int
is_order_upstairs(int current_floor);

/* Returns 1 if there is a cab order to a floor below current_floor or if there
   is a hall order at a floor below current_floor. Returns 0 otherwise.  */
int
is_order_downstairs(int current_floor);
