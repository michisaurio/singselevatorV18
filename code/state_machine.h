#include "elev.h"

typedef enum state_type {
	DRIVE = 0,
	OPEN_DOOR,
	IDLE_AT_FLOOR,
	EMERGENCY_AT_FLOOR,
	IDLE_BETWEEN_FLOORS,
	EMERGENCY_BETWEEN_FLOORS
} state_type_t;

void set_state_to_drive(elev_motor_direction_t new_dir);

void set_state_to_idle_at_floor();

void set_state_to_open_door();

int initialize_state();

void determine_next_state();
