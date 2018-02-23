
typedef enum state_type {
	IDLE = 0,
	DRIVE,
	OPENDOOR
} state_type_t;

void determine_next_state();
