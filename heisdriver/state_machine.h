
typedef enum state_type {
	IDLE = 0,
	DRIVE,
	OPENDOOR
} state_type_t;

//void determineDirection(elev_motor_direction_t* motor_dir, int floor);

int closestOrderUp(int floor);

int closestOrderDown(int floor);

void determineNextState();
