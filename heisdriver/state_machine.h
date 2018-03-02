
typedef enum state_type {
	IDLEATFLOOR = 0,
	DRIVE,
	OPENDOOR,
	STOPATFLOOR,
	STOPBETWEENFLOORS,
	IDLEBETWEENFLOORS
} state_type_t;

void determine_next_state();

void from_idle_or_drive_to_opendoor();
