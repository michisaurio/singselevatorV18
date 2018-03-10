/* state_machine.c
See state_machine.h for a general description of the state_machine module.
*/
#include "state_machine.h"
#include "elev.h"
#include "order_controller.h"
#include "door_timer.h"
#include <stdio.h>
#include <stdlib.h>

static elev_state_type_t elevator_state;
/* elevator_direction represents in a broader context the direction the elevator
is or was moving or is going to move in. If the elevator is moving, it moves in
accordance with elevator_state. But if the elevator has stopped, the value of
elevator_direction is not necessarily DIRN_STOP.  */
static elev_motor_direction_t elevator_direction;
static int last_floor;
/* move_after_door_closes takes the values 1 or 0 depending on whether it has been
determined that the elevator is going to move after the door closes or not.  */
static int move_after_door_closes;

void
set_state_to_drive(elev_motor_direction_t direction)
{
  elevator_state = DRIVE;
  elevator_direction = direction;
  elev_set_motor_direction(elevator_direction);
}

void
set_state_to_idle_at_floor()
{
  elevator_state = IDLE_AT_FLOOR;
  elevator_direction = DIRN_STOP;
  elev_set_motor_direction(elevator_direction);
}

void
set_state_to_open_door()
{
  elevator_state = OPEN_DOOR;
  elev_set_motor_direction(DIRN_STOP);
  elev_set_door_open_lamp(1);
  start_door_timer();
  move_after_door_closes = 0;
}

int
initialize_state()
{
  if (!elev_init()) {
    return 0;
  }
  last_floor = elev_get_floor_sensor_signal();
  if (last_floor == -1) {
    elev_set_motor_direction(DIRN_DOWN); // Gravity helps us :-)
    do {
        last_floor = elev_get_floor_sensor_signal();
       }
    while (last_floor == -1);
  }
  set_state_to_idle_at_floor();
  elev_set_floor_indicator(last_floor);
  return 1;
}

void
determine_next_state()
{
  int sensor_signal;
  // See state_machine.h for an overview of all possible state transitions
  switch (elevator_state) {

  case IDLE_AT_FLOOR:
    // From IDLE_AT_FLOOR to EMERGENCY_AT_FLOOR ?
    if (elev_get_stop_signal()) {
      elevator_state = EMERGENCY_AT_FLOOR;
      break;
    }
    // From IDLE_AT_FLOOR to OPEN_DOOR ?
    if (get_order_status(BUTTON_COMMAND, last_floor)) {
      set_state_to_open_door();
      clear_order_status(BUTTON_COMMAND, last_floor);
    } else if (get_order_status(BUTTON_CALL_UP, last_floor)) {
      set_state_to_open_door();
      clear_order_status(BUTTON_CALL_UP, last_floor);
    } else if (get_order_status(BUTTON_CALL_DOWN, last_floor)) {
      set_state_to_open_door();
      clear_order_status(BUTTON_CALL_DOWN, last_floor);
    /* From IDLE_AT_FLOOR to DRIVE ? : Checks if the elevator is at the highest
    or lowest half part, and prioritizes moving towards the closest end.  */
    } else if (2*last_floor+1 >= N_FLOORS) {
      if (is_order_upstairs(last_floor)) {
        set_state_to_drive(DIRN_UP);
      } else if (is_order_downstairs(last_floor)) {
        set_state_to_drive(DIRN_DOWN);
      }
    } else {
      if (is_order_downstairs(last_floor)) {
        set_state_to_drive(DIRN_DOWN);
      } else if (is_order_upstairs(last_floor)) {
        set_state_to_drive(DIRN_UP);
      }
    }
    break;

  case DRIVE:
    sensor_signal = elev_get_floor_sensor_signal();
    // From DRIVE to EMERGENCY_AT_FLOOR or to EMERGENCY_BETWEEN_FLOORS ?
    if (elev_get_stop_signal()) {
      elev_set_motor_direction(DIRN_STOP);
      if (sensor_signal >= 0) { // i.e. at a floor
      	last_floor = sensor_signal;
      	elev_set_floor_indicator (last_floor);
      	elevator_state = EMERGENCY_AT_FLOOR;
      } else {
	       elevator_state = EMERGENCY_BETWEEN_FLOORS;
      }
      break;
    }
    if (sensor_signal >= 0) {	// i.e. a floor is reached
      last_floor = sensor_signal;
      elev_set_floor_indicator (last_floor);
      // From DRIVE to OPEN_DOOR ?
      /* Checks if there is a cab order to this floor or a hall order in the same
      direction as the elevator direction at this floor, or if there are no
      further orders to or at floors further in the elevator direction and there
      is a hall order order in the direction opposite to the elevator direction.  */
      if (elevator_direction == DIRN_UP) {
      	if (get_order_status(BUTTON_CALL_UP, last_floor)
      	    || get_order_status(BUTTON_COMMAND, last_floor)) {
      	  set_state_to_open_door();
      	  clear_order_status(BUTTON_CALL_UP, last_floor);
      	  clear_order_status(BUTTON_COMMAND, last_floor);
      	} else if (!is_order_upstairs(last_floor)
      		          && get_order_status(BUTTON_CALL_DOWN, last_floor)) {
      	  set_state_to_open_door();
      	  clear_order_status(BUTTON_CALL_DOWN, last_floor);
      	}
      } else if (elevator_direction == DIRN_DOWN) {
      	if (get_order_status(BUTTON_CALL_DOWN, last_floor)
      	    || get_order_status(BUTTON_COMMAND, last_floor)) {
      	  set_state_to_open_door();
      	  clear_order_status(BUTTON_CALL_DOWN, last_floor);
      	  clear_order_status(BUTTON_COMMAND, last_floor);
      	} else if (!is_order_downstairs(last_floor)
      		          && get_order_status(BUTTON_CALL_UP, last_floor)) {
      	  set_state_to_open_door();
      	  clear_order_status(BUTTON_CALL_UP, last_floor);
      	}
      }
    }
    break;

  case OPEN_DOOR:
    // From OPEN_DOOR to EMERGENCY_AT_FLOOR ?
    if (elev_get_stop_signal()) {
      elevator_state = EMERGENCY_AT_FLOOR;
      break;
    }
    // Pressing the cab order button for the current floor only restarts the door timer.
    if (get_order_status(BUTTON_COMMAND, last_floor)) {
      clear_order_status(BUTTON_COMMAND, last_floor);
      start_door_timer();
    }
    /* The new elevator direction after the door closes is set to DIRN_UP
    if there are orders at or to floors upstairs and if the elevator direction
    (before the door was opened) is already DIRN_UP or if it isDIRN_DOWN or if it
    is DIRN_DOWN, but there are no more orders at or to floors downstairs.
    Analogously, for setting the elevator direction to DIRN_DOWN.
    Note that setting the direction to DIRN_UP is prioritized over DIRN_DOWN. */
    if (!move_after_door_closes) {
      if (is_order_upstairs(last_floor)
	         && (elevator_direction == DIRN_UP
               || elevator_direction == DIRN_STOP
	             || !is_order_downstairs(last_floor))) {
      	elevator_direction = DIRN_UP;
      	move_after_door_closes = 1;
      } else if (is_order_downstairs(last_floor)
		              && (elevator_direction == DIRN_DOWN
              		    || elevator_direction == DIRN_STOP
              		    || !is_order_upstairs(last_floor))) {
      	elevator_direction = DIRN_DOWN;
      	move_after_door_closes = 1;
      /* If there are no orders to or at floors upstairs or downstairs, pressing
      the hall order buttons for the current floor only restarts the door timer. */
      } else if (get_order_status(BUTTON_CALL_UP, last_floor)
		              || get_order_status(BUTTON_CALL_DOWN, last_floor)) {
      	clear_order_status(BUTTON_CALL_UP, last_floor);
      	clear_order_status(BUTTON_CALL_DOWN, last_floor);
      	start_door_timer();
      }
    }
    /* If the elevator direction after the door closes has been determined,
    pressing the corresponding hall order button only restarts the door timer. */
    if (move_after_door_closes) {
      if (elevator_direction == DIRN_UP
	         && get_order_status(BUTTON_CALL_UP, last_floor)) {
      	clear_order_status(BUTTON_CALL_UP, last_floor);
      	start_door_timer();
      } else if (elevator_direction == DIRN_DOWN
		              && get_order_status(BUTTON_CALL_DOWN, last_floor)) {
      	clear_order_status(BUTTON_CALL_DOWN, last_floor);
      	start_door_timer();
      }
    }
    /* If the door timers elapsed time is over 3 seconds, the elevator state
    changes to DRIVE or to IDLE_AT_FLOOR. */
    update_door_timer();
    if (is_elapsed_time_over_threshold()) {
      reset_door_timer();
      elev_set_door_open_lamp(0);
      if (move_after_door_closes) {
	       set_state_to_drive(elevator_direction);
      } else {
	       set_state_to_idle_at_floor();
      }
    }
    break;

  case EMERGENCY_AT_FLOOR: // Only possible transition: to OPEN_DOOR.
    elev_set_motor_direction(DIRN_STOP);
    elev_set_stop_lamp(1);
    clear_all_orders();
    elev_set_door_open_lamp(1);
    while (elev_get_stop_signal()) {
    }
    elev_set_stop_lamp(0);
    set_state_to_open_door();
    break;

  case EMERGENCY_BETWEEN_FLOORS: // Only possible transition: to IDLE_BETWEEN_FLOORS.
    elev_set_motor_direction(DIRN_STOP);
    elev_set_stop_lamp(1);
    clear_all_orders();
    while (elev_get_stop_signal()) {
    }
    elev_set_stop_lamp(0);
    elevator_state = IDLE_BETWEEN_FLOORS;
    break;

  case IDLE_BETWEEN_FLOORS:
    // From IDLE_BETWEEN_FLOORS to EMERGENCY_BETWEEN_FLOORS ?
    if (elev_get_stop_signal()) {
      elevator_state = EMERGENCY_BETWEEN_FLOORS;
      break;
    }
    // From IDLE_BETWEEN_FLOORS to DRIVE ?
    /* Checks if there is an order somewhere. The location of the elevator is
    determined using elevator_direction and last_floor. If the elevator state
    is set to drive and the elevator direction is changed, the value of
    last_floor is modified in order to give the same location as before.
    Note that this does not affect which floor indicator light is on. */
    if (elevator_direction == DIRN_UP) {
      if (is_order_upstairs(last_floor)) {
	       set_state_to_drive(DIRN_UP);
      } else if (is_order_downstairs(last_floor + 1)) {
	       set_state_to_drive(DIRN_DOWN);
         last_floor++;
      }
    } else if (elevator_direction == DIRN_DOWN) {
      if (is_order_downstairs(last_floor)) {
	       set_state_to_drive(DIRN_DOWN);
      } else if (is_order_upstairs(last_floor - 1)) {
	       set_state_to_drive(DIRN_UP);
	       last_floor--;
      }
    }
    break;

  default:
    printf("Invalid elevator state. Exit program.");
    exit(EXIT_FAILURE);
  }
}
