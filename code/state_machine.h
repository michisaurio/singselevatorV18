/* state_machine.h
The elevator is seen as a finite state machine (FSM).
The state_machine module handles the elevator state.
An enum type is defined for the different possible states.
Functions to initialize the elevator state and
to determine if the current state has to be changed are implemented,
as well as functions to set to state to particular state values.
*/
#include "elev.h"

/*
A total of 6 elevator states are considered:
  DRIVE : The elevator is moving up or down.
          Implies: Closed door and that some order exists.
  OPEN_DOOR : The elevator is at a floor with the door open.
              Implies: No movement.
  IDLE_AT_FLOOR : The elevator is at a floor with the door closed.
                  Implies: No movement and that no orders exist.
  EMERGENCY_AT_FLOOR : The elevator is at a floor with the door open
                       and the stop button is pressed down.
                      Implies: No movement, no orders registered nor taken.
  IDLE_BETWEEN_FLOORS : The elevator is between 2 floors and does not move.
                        Implies: No order exists.
  EMERGENCY_BETWEEN_FLOORS: The elevator is between to 2 floors, does not move,
                            and the stop button is pressed down.
*/
typedef enum elevator_state_type {
  DRIVE = 0,
  OPEN_DOOR,
  IDLE_AT_FLOOR,
  EMERGENCY_AT_FLOOR,
  IDLE_BETWEEN_FLOORS,
  EMERGENCY_BETWEEN_FLOORS
} elev_state_type_t;

/*
Sets the elevator state to DRIVE: The elevator direction is set to the value given
by "direction", and the elevator moves accordingly. It is assumed by the rest
of the implementation that "direction" only takes the values DIRN_UP or DIRN_DOWN,
not the value DIRN_STOP.
*/
void
set_state_to_drive(elev_motor_direction_t direction);

/*
Sets the elevator state to IDLE_AT_FLOOR: The elevator direction is set to DIRN_STOP.
This function stops the elevator. However, this step is only needed during initialization.
*/
void
set_state_to_idle_at_floor();

/*
Sets the elevator state to OPEN_DOOR: The elevator is stopped, the door open
light is turned on and the door timer is started.
*/
void
set_state_to_open_door();

/*
The elevator hardware is initialize first (see elev.h). If this is done successfully,
the elevator is driven down to the nearest floor if it did not started at a floor already.
When at a floor, the elevator state is set to IDLE_AT_FLOOR and the corresponding
floor indicator light is turned on.
*/
int
initialize_state();

/*
Given the current elevator state and the overall situation, the conditions for
a state transition are checked. If this is the case, the corresponding actions
are executed. The possible state transitions listed after current state are:
From IDLE_AT_FLOOR:
  to EMERGENCY_AT_FLOOR: If the stop button is pressed.
  to OPEN_DOOR: If a cab or hall order button at the current floor is pressed.
  to DRIVE: If a cab or hall order button at another floor is pressed.
From DRIVE:
  If the stop button is pressed the state will change to EMERGENCY_AT_FLOOR or
  EMERGENCY_BETWEEN_FLOORS depending on whether the elevator is at a floor or
  between 2 floors.
  to OPEN_DOOR: If the elevator reaches a new floor and there is a cab order to
                that floor or and hall order from that floor.
  Note that each time the elevator reaches a new floor the corresponding floor
  indicator light is updated.
From OPEN_DOOR:
  to EMERGENCY_AT_FLOOR: If the stop button is pressed.
  to DRIVE: If there is an order.
  to IDLE_AT_FLOOR: If there are no more orders.
From EMERGENCY_AT_FLOOR:
  to OPEN_DOOR : If the stop button is released.
From EMERGENCY_BETWEEN_FLOORS:
  to IDLE_BETWEEN_FLOORS: If the stop button is released.
From IDLE_BETWEEN_FLOORS:
  to EMERGENCY_BETWEEN_FLOORS: If the stop button is pressed.
  to DRIVE: If there is an order.
Note that the transitions are checked in the order they are presented above.
See state_machine.c for more details.
*/
void
determine_next_state();
