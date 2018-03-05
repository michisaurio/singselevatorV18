#include "elev.h"

typedef enum state_type {
	IDLEATFLOOR = 0,
	DRIVE,
	OPENDOOR,
	STOPATFLOOR,
	STOPBETWEENFLOORS,
	IDLEBETWEENFLOORS
} state_type_t;

int initialize_state();

void determine_next_state();

void set_state_to_opendoor();

void set_state_to_drive(elev_motor_direction_t new_dir);
