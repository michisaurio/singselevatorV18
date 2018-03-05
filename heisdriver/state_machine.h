#include "elev.h"

typedef enum state_type {
	DRIVE = 0,
	IDLEATFLOOR,
	IDLEBETWEENFLOORS,
	OPENDOOR,
	STOPATFLOOR,
	STOPBETWEENFLOORS
} state_type_t;

void set_state_to_drive(elev_motor_direction_t new_dir);

void set_state_to_idle_at_floor();

void set_state_to_open_door();

int initialize_state();

void determine_next_state();
