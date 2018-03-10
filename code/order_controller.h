#include "elev.h"

void check_pressed_order_button ();

int get_order_status (elev_button_type_t button, int floor);

void clear_order_status (elev_button_type_t button, int floor);

void clear_all_orders ();

int is_order_upstairs (int current_floor);

int is_order_downstairs (int current_floor);
