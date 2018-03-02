#include "state_machine.h"
#include "elev.h"
#include "order_controller.h"
#include "door_timer.h"
#include <stdio.h> // Eliminar mas tarde

// Assumes that we start at IDLEATFLOOR state, at the 1. floor.
// Se tiene que iniciar motor_direction igual a DIRN_STOP?
static state_type_t state = IDLEATFLOOR;
static elev_motor_direction_t motor_direction = DIRN_STOP;
double static const open_door_threshold_time = 3.0;
static int last_floor = 0;

void from_idle_or_drive_to_opendoor(){
	state = OPENDOOR;
	door_timer_start();
	elev_set_motor_direction(DIRN_STOP);
}

void determine_next_state(){

	int sensor_signal = elev_get_floor_sensor_signal();
	//printf("Sensor signal: %d \n",sensor_signal);

	switch(state){

	case IDLEATFLOOR: // In IDLEATFLOOR, sensor_signal>=0, i.e., the elevator is at a floor
		if (elev_get_stop_signal()) {
				state = STOPATFLOOR;
				clear_all_orders();
				elev_set_door_open_lamp(1);
				elev_set_stop_lamp(1);
				break;
		}
		// From IDLEATFLOOR to OPENDOOR : If button for/at current floor pressed.
		if(get_order_status(BUTTON_COMMAND,sensor_signal)==1){
			// If cab button pressed. The door opens. Somebody fell asleep inside.
			from_idle_or_drive_to_opendoor();
			clear_order_status(BUTTON_COMMAND,sensor_signal);
		} else if(get_order_status(BUTTON_CALL_UP,sensor_signal)==1){
			// If hall button up pressed. The door opens. Somebody wants to go to up.
			from_idle_or_drive_to_opendoor();
			clear_order_status(BUTTON_CALL_UP,sensor_signal);
		}	else if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1){
			// If hall button down pressed. The door opens. Somebody wants to go to down.
			from_idle_or_drive_to_opendoor();
			clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
		} else // From IDLEATFLOOR to DRIVE : If button for/at another floor pressed.
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
		// Otherwise stays at IDLEATFLOOR.
		break;

	case DRIVE: // From IDLEATFLOOR to OPENDOOR : Is necessary that a floor is reached first.
		if (elev_get_stop_signal()) {
				state = STOPBETWEENFLOORS;
				elev_set_motor_direction(DIRN_STOP);
				clear_all_orders();
				elev_set_stop_lamp(1);
				break;
		}
		if(sensor_signal>=0){	//A floor is reached
			last_floor = sensor_signal;
			elev_set_floor_indicator(sensor_signal);	//Updates floor indicator
			if(motor_direction==DIRN_UP){
					if(get_order_status(BUTTON_CALL_UP,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						//The door opens. Somebody goes out or somebody wants to go upstairs
						from_idle_or_drive_to_opendoor();
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
					}else if(!is_order_upstairs(sensor_signal) && get_order_status(BUTTON_CALL_DOWN,sensor_signal)){
						//The door opens. Nobody wants upstairs. But somebody wants to go downstairs.
						from_idle_or_drive_to_opendoor();
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
					}
			}else if(motor_direction==DIRN_DOWN){
					if(get_order_status(BUTTON_CALL_DOWN,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						//The door opens. Somebody goes out or somebody wants to go downstairs
						from_idle_or_drive_to_opendoor();
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
					}else if(!is_order_downstairs(sensor_signal) && get_order_status(BUTTON_CALL_UP,sensor_signal)){
						//The door opens. Nobody wants upstairs. But somebody wants to go upstairs
						from_idle_or_drive_to_opendoor();
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
					}
			}
		}
		break;

	case OPENDOOR:
		if (elev_get_stop_signal()) {
				state = STOPATFLOOR;
				clear_all_orders();
				elev_set_door_open_lamp(1);
				elev_set_stop_lamp(1);
				break;
		}
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
			} else {// From OPENDOOR to IDLEATFLOOR : no orders upstairs and no orders downstairs
				state = IDLEATFLOOR;
				motor_direction = DIRN_STOP;
				printf("No more customers.\n");
			}
		} else { // the door remains open
			if(is_order_upstairs(sensor_signal)==1 && (motor_direction==DIRN_UP || motor_direction==DIRN_STOP || is_order_downstairs(sensor_signal)==0)){
				motor_direction = DIRN_UP;
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
			}else if(is_order_downstairs(sensor_signal)==1 && (motor_direction==DIRN_DOWN || motor_direction==DIRN_STOP || is_order_upstairs(sensor_signal)==0)){
				motor_direction = DIRN_DOWN;
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			} else {// no orders upstairs and no orders downstairs
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			}
			// Apagar luces de hall_orders en la direccion de movimiento del ascensor
		}
		break;

	case STOPATFLOOR:
		while(elev_get_stop_signal()){
					clear_all_orders();
			}
			elev_set_stop_lamp(0);
			state = OPENDOOR;
			door_timer_start();
		break;

	case STOPBETWEENFLOORS:
		while(elev_get_stop_signal()){
					clear_all_orders();
			}
			elev_set_stop_lamp(0);
			state = IDLEBETWEENFLOORS;
		break;

	case IDLEBETWEENFLOORS:
		if (elev_get_stop_signal()) {
				state = STOPBETWEENFLOORS;
				clear_all_orders();
				elev_set_stop_lamp(1);
				break;
		}
		if(motor_direction == DIRN_UP){
			if(is_order_upstairs(last_floor)){
				state = DRIVE;
				motor_direction = DIRN_UP;
				elev_set_motor_direction(motor_direction);
			} else if(is_order_downstairs(last_floor+1)){
				state = DRIVE;
				motor_direction = DIRN_DOWN;
				elev_set_motor_direction(motor_direction);
			}
		}else if(motor_direction == DIRN_DOWN){
			if(is_order_downstairs(last_floor)){
				state = DRIVE;
				motor_direction = DIRN_DOWN;
				elev_set_motor_direction(motor_direction);
			}else if(is_order_upstairs(last_floor-1)){
				state = DRIVE;
				motor_direction = DIRN_UP;
				elev_set_motor_direction(motor_direction);
			}
		}
		break;

	default:
		printf("That was bad luck...");
	}
}
