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
typedef enum state_type {
  DRIVE = 0,
  OPEN_DOOR,
  IDLE_AT_FLOOR,
  EMERGENCY_AT_FLOOR,
  IDLE_BETWEEN_FLOORS,
  EMERGENCY_BETWEEN_FLOORS
} state_type_t;

/*
Sets the elevator state to DRIVE: The motor direction is set to the value given
by new_dir, and the elevator moves in that direction. It is assumed by the rest
of the implementation that new_dir only takes the values DIRN_UP or DIRN_DOWN,
not the value DIRN_STOP.
*/
void
set_state_to_drive(elev_motor_direction_t new_dir);

/*
Sets the elevator state to IDLE_AT_FLOOR: The motor direction is set to DIRN_STOP.
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
are executed. See state_machine.c for more details.
*/
void
determine_next_state();
