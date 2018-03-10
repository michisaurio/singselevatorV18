/*state_machine.h
The state_machine module handles the elevator seen as a finite
state machine (FSM). 6 di
*/
#include "elev.h"

typedef enum state_type {
  DRIVE = 0,			// The elevator is moving.
  OPEN_DOOR,				// The elevator is at a floor with the door open.
  IDLE_AT_FLOOR,			// The elevator is at a floor with the door closed.
  EMERGENCY_AT_FLOOR,		// The elevator is at a floor with the door open and with the stop button pressed down.
  IDLE_BETWEEN_FLOORS,		// The elevator is between 2 floors and does not move.
  EMERGENCY_BETWEEN_FLOORS	// The elevator is between 2 floors, does not move and the stop button
} state_type_t;
/*
Sets the elevator state to DRIVE:
*/
void
set_state_to_drive(elev_motor_direction_t new_dir);
/*

*/
void
set_state_to_idle_at_floor();
/*

*/
void
set_state_to_open_door();
/*

*/
int
initialize_state();
/*

*/
void
determine_next_state();
