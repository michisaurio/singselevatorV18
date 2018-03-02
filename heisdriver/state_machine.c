#include "state_machine.h"
#include "elev.h"
#include "order_controller.h"
#include "door_timer.h"
#include <stdio.h> // Eliminar mas tarde

// Assumes that we start at IDLE state, on a floor.
// Se tiene que iniciar motor_direction igual a DIRN_STOP?
static state_type_t state = IDLE;
static elev_motor_direction_t motor_direction = DIRN_STOP;
double static const open_door_threshold_time = 3.0;

void determine_next_state(){

	int sensor_signal = elev_get_floor_sensor_signal();
	//printf("Sensor signal: %d \n",sensor_signal);

	switch(state){

	case IDLE: // In IDLE, sensor_signal>=0, i.e., the elevator is at a floor
		// From IDLE to OPENDOOR : If button for/at current floor pressed.
		// If cab button pressed
		if(get_order_status(BUTTON_COMMAND,sensor_signal)==1){
			state = OPENDOOR;
			door_timer_start();
			//motor_direction = DIRN_STOP;
			clear_order_status(BUTTON_COMMAND,sensor_signal);
			printf("The door opens. Somebody fell asleep inside.\n");
		} // If hall button up pressed
		else if(get_order_status(BUTTON_CALL_UP,sensor_signal)==1){
			state = OPENDOOR;
			door_timer_start();
			//motor_direction = DIRN_UP;
			clear_order_status(BUTTON_CALL_UP,sensor_signal);
			printf("The door opens. Somebody wants to go to up.\n");
		} // If hall button down pressed
		else if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1){
			state = OPENDOOR;
			door_timer_start();
			//motor_direction = DIRN_DOWN;
			clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			printf("The door opens. Somebody wants to go to down.\n");
		}
		else // From IDLE to DRIVE : If button for/at another floor pressed.
		{ // A cab button for another floor is pressed : Biased towards up.
			for(int floor=N_FLOORS-1; floor>sensor_signal; floor--){
				if(get_order_status(BUTTON_COMMAND,floor)==1){
					state = DRIVE;
					motor_direction = DIRN_UP;
					elev_set_motor_direction(motor_direction);
					printf("Somebody fell asleep and wants to go up.\n");
					break;
				}
			}
			for(int floor=0; floor<sensor_signal; floor++){
				if(get_order_status(BUTTON_COMMAND,floor)==1){
					state = DRIVE;
					motor_direction = DIRN_DOWN;
					elev_set_motor_direction(motor_direction);
					printf("Somebody fell asleep and wants to go down.\n");
					break;
					}
				}
			// A hall button at another is pressed : Moves towards the closest floor. Biased towards up.
			int dist_order_upstairs   = closest_hall_order_upstairs(sensor_signal);
			int dist_order_downstairs = closest_hall_order_downstairs(sensor_signal);
			if (dist_order_upstairs<N_FLOORS && dist_order_upstairs<=dist_order_downstairs) {
				state = DRIVE;
				motor_direction = DIRN_UP;
				elev_set_motor_direction(motor_direction);
				printf("Somebody upstairs wants to ride.\n");
			} else if (dist_order_downstairs<N_FLOORS && dist_order_downstairs<dist_order_upstairs) {
				state = DRIVE;
				motor_direction = DIRN_DOWN;
				elev_set_motor_direction(motor_direction);
				printf("Somebody downstairs wants to ride.\n");
			}
		}
		// Otherwise stays at IDLE.
		break;

	case DRIVE: // From IDLE to OPENDOOR : Is necessary that a floor is reached first.
		if(sensor_signal>=0){	//A floor is reached
			elev_set_floor_indicator(sensor_signal);	//Updates floor indicator
			if(motor_direction==DIRN_UP){
					if(get_order_status(BUTTON_CALL_UP,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						state = OPENDOOR;
						door_timer_start();
						elev_set_motor_direction(DIRN_STOP);
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
						printf("The door opens. Somebody goes out or somebody wants to go upstairs.\n");
					}else if(!is_order_upstairs(sensor_signal) && get_order_status(BUTTON_CALL_DOWN,sensor_signal)){
						state = OPENDOOR;
						door_timer_start();
						elev_set_motor_direction(DIRN_STOP);
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
						printf("The door opens. Nobody wants upstairs. But somebody wants to go downstairs.\n");
					}
			}else if(motor_direction==DIRN_DOWN){
					if(get_order_status(BUTTON_CALL_DOWN,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						state = OPENDOOR;
						door_timer_start();
						elev_set_motor_direction(DIRN_STOP);
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
						printf("The door opens. Somebody goes out or somebody wants to go downstairs.\n");
					}else if(!is_order_downstairs(sensor_signal) && get_order_status(BUTTON_CALL_UP,sensor_signal)){
						state = OPENDOOR;
						door_timer_start();
						elev_set_motor_direction(DIRN_STOP);
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
						printf("The door opens. Nobody wants upstairs. But somebody wants to go upstairs.\n");
					}
			}
		}
		break;

	case OPENDOOR:
		door_timer_update();
		// Keep cab button for the current floor turned off
		if(get_order_status(BUTTON_COMMAND, sensor_signal)){
			clear_order_status(BUTTON_COMMAND, sensor_signal);
			printf("Don't be a fool and step outside!!!\n");
		}
		// Checks if the door should be closed
		if(is_elapsed_time_over_threshold(open_door_threshold_time)==1){
			door_timer_reset();
			// From OPENDOOR to DRIVE
			if(is_order_upstairs(sensor_signal) && (motor_direction==DIRN_UP || motor_direction==DIRN_STOP || !is_order_downstairs(sensor_signal))){
				state = DRIVE;
				motor_direction = DIRN_UP;
				elev_set_motor_direction(motor_direction);
				printf("We are going to pick up somebody upstairs.\n");
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
			} else if(is_order_downstairs(sensor_signal) && (motor_direction==DIRN_DOWN || motor_direction==DIRN_STOP || !is_order_upstairs(sensor_signal))){
				state = DRIVE;
				motor_direction = DIRN_DOWN;
				elev_set_motor_direction(motor_direction);
				printf("We are going to pick up somebody downstairs.\n");
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			} else {// From OPENDOOR to IDLE : no orders upstairs and no orders downstairs
				state = IDLE;
				motor_direction = DIRN_STOP;
				printf("No more customers.\n");
			}
		} else { // the door remains open
			if(is_order_upstairs(sensor_signal)==1 && (motor_direction==DIRN_UP || motor_direction==DIRN_STOP || is_order_downstairs(sensor_signal)==0)){
				motor_direction = DIRN_UP;
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
			}else if(is_order_downstairs(sensor_signal)==1 && (motor_direction==DIRN_DOWN || motor_direction==DIRN_STOP || is_order_upstairs(sensor_signal)==0)){
				motor_direction = DIRN_DOWN;
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
			} else {// no orders upstairs and no orders downstairs
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			}
			// Apagar luces de hall_orders en la direccion de movimiento del ascensor
		}
		break;

	case STOPATAFLOOR:

	case STOPBETWEENFLOORS

	default:
		printf("That was bad luck...");
	}
}
