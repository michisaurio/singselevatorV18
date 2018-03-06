#include "state_machine.h"
#include "elev.h"
#include "order_controller.h"
#include "door_timer.h"
#include <stdio.h>
//
static state_type_t state;
static elev_motor_direction_t motor_direction;
double static const open_door_threshold_time = 3.0;
static int last_floor;
static int move_after_door_closes = 0;

void set_state_to_drive(elev_motor_direction_t new_dir){
	state = DRIVE;
	motor_direction = new_dir;
	elev_set_motor_direction(motor_direction);
}

void set_state_to_idle_at_floor(){
	state = IDLEATFLOOR;
	motor_direction = DIRN_STOP;
	elev_set_motor_direction(motor_direction);
}

void set_state_to_open_door(){
	state = OPENDOOR;
	start_door_timer();
	elev_set_door_open_lamp(1);
	elev_set_motor_direction(DIRN_STOP);
	move_after_door_closes = 0;
}

// set_state_to_emergency

int initialize_state(){
	if (!elev_init()) {
			return 0;
	}
	if(elev_get_floor_sensor_signal()==-1){
		elev_set_motor_direction(DIRN_DOWN);	// We use gravity. Gravity is environmental friendly ;-)
		while(elev_get_floor_sensor_signal()==-1){
		}
	}
	set_state_to_idle_at_floor();
	last_floor = elev_get_floor_sensor_signal();
	elev_set_floor_indicator(last_floor);
	return 1;
}

void determine_next_state(){
	int sensor_signal = elev_get_floor_sensor_signal();
	int dist_order_upstairs;
	int dist_order_downstairs;

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
		if(get_order_status(BUTTON_COMMAND,sensor_signal)){
			// If cab button pressed. The door opens. Somebody fell asleep inside.
			set_state_to_open_door();
			clear_order_status(BUTTON_COMMAND,sensor_signal);
		} else if(get_order_status(BUTTON_CALL_UP,sensor_signal)){
			// If hall button up pressed. The door opens. Somebody wants to go to up.
			set_state_to_open_door();
			clear_order_status(BUTTON_CALL_UP,sensor_signal);
		}	else if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)){
			// If hall button down pressed. The door opens. Somebody wants to go to down.
			set_state_to_open_door();
			clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
		} else { // From IDLEATFLOOR to DRIVE : If button for/at another floor pressed.
		 // A cab button for another floor is pressed : Biased towards up. Somebody fell asleep and wants to go up
				if(is_cab_order_upstairs(sensor_signal)){
					set_state_to_drive(DIRN_UP);
				}else if(is_cab_order_downstairs(sensor_signal)){
					set_state_to_drive(DIRN_DOWN);
				}else{
				// A hall button at another floor is pressed : Moves towards the closest floor. Biased towards up.
					dist_order_upstairs   = closest_hall_order_upstairs(sensor_signal);
					dist_order_downstairs = closest_hall_order_downstairs(sensor_signal);
					if (dist_order_upstairs<N_FLOORS && dist_order_upstairs<=dist_order_downstairs) {
						set_state_to_drive(DIRN_UP);
					} else if (dist_order_downstairs<N_FLOORS && dist_order_downstairs<dist_order_upstairs) {
						set_state_to_drive(DIRN_DOWN);
					}
				}
		}		// Otherwise stays at IDLEATFLOOR.
		break;

	case DRIVE: // From IDLEATFLOOR to OPENDOOR : Is necessary that a floor is reached first.
		if (elev_get_stop_signal()) {
				state = STOPBETWEENFLOORS;
				elev_set_motor_direction(DIRN_STOP);
				elev_set_stop_lamp(1);
				break;
		}
		if(sensor_signal>=0){	//A floor is reached
			last_floor = sensor_signal;
			elev_set_floor_indicator(sensor_signal);	//Updates floor indicator
			if(motor_direction==DIRN_UP){
					if(get_order_status(BUTTON_CALL_UP,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						//The door opens. Somebody goes out or somebody wants to go upstairs
						set_state_to_open_door();
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
					}else if(!is_order_upstairs(sensor_signal) && get_order_status(BUTTON_CALL_DOWN,sensor_signal)){
						//The door opens. Nobody wants upstairs. But somebody wants to go downstairs.
						set_state_to_open_door();
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
					}
			}else if(motor_direction==DIRN_DOWN){
					if(get_order_status(BUTTON_CALL_DOWN,sensor_signal) || get_order_status(BUTTON_COMMAND,sensor_signal)){
						//The door opens. Somebody goes out or somebody wants to go downstairs
						set_state_to_open_door();
						clear_order_status(BUTTON_CALL_DOWN, sensor_signal);
						clear_order_status(BUTTON_COMMAND, sensor_signal);
					}else if(!is_order_downstairs(sensor_signal) && get_order_status(BUTTON_CALL_UP,sensor_signal)){
						//The door opens. Nobody wants upstairs. But somebody wants to go upstairs
						set_state_to_open_door();
						clear_order_status(BUTTON_CALL_UP, sensor_signal);
					}
			}
		}
		break;

	case OPENDOOR:
		if (elev_get_stop_signal()) {
				state = STOPATFLOOR;
				elev_set_door_open_lamp(1);
				elev_set_stop_lamp(1);
				break;
		}
		update_door_timer();
		if(get_order_status(BUTTON_COMMAND, sensor_signal)){
			clear_order_status(BUTTON_COMMAND, sensor_signal);
			reset_door_timer();
		}
		 // Keep cab button for the current floor turned off
		if(!move_after_door_closes){
			if(is_order_upstairs(sensor_signal) && (motor_direction==DIRN_UP || motor_direction==DIRN_STOP || !is_order_downstairs(sensor_signal))){
				motor_direction = DIRN_UP;
				move_after_door_closes = 1;
			}else if(is_order_downstairs(sensor_signal) && (motor_direction==DIRN_DOWN || motor_direction==DIRN_STOP || !is_order_upstairs(sensor_signal))){
				motor_direction = DIRN_DOWN;
				move_after_door_closes = 1;
			}else if(get_order_status(BUTTON_CALL_UP, sensor_signal)||get_order_status(BUTTON_CALL_DOWN, sensor_signal)){
				// no orders at or to other floors -> No need to move up or down
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
				reset_door_timer();
			}
		}
		if(move_after_door_closes){
			if(motor_direction==DIRN_UP || get_order_status(BUTTON_CALL_UP, sensor_signal)){
				clear_order_status(BUTTON_CALL_UP,sensor_signal);
				reset_door_timer();
			}else if(motor_direction==DIRN_DOWN || get_order_status(BUTTON_CALL_DOWN,sensor_signal)){
				clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
				reset_door_timer();
			}
		}
		if(is_elapsed_time_over_threshold(open_door_threshold_time)){
			reset_door_timer();
			elev_set_door_open_lamp(0);
			if(move_after_door_closes){
				set_state_to_drive(motor_direction);
			}else{
				set_state_to_idle_at_floor();
				// Or Here?
				//clear_order_status(BUTTON_CALL_UP,sensor_signal);
				//clear_order_status(BUTTON_CALL_DOWN,sensor_signal);
			}
		}
		break;

	case STOPATFLOOR:
		if(elev_get_stop_signal()){
			clear_all_orders();
		}else{
			elev_set_stop_lamp(0);
			set_state_to_open_door();
		}
		break;

	case STOPBETWEENFLOORS:
		if(elev_get_stop_signal()){
			clear_all_orders();
		}else{
			elev_set_stop_lamp(0);
			state = IDLEBETWEENFLOORS;
		}
		break;

	case IDLEBETWEENFLOORS:
		if (elev_get_stop_signal()) {
				state = STOPBETWEENFLOORS;
				elev_set_stop_lamp(1);
				break;
		}
		if(motor_direction == DIRN_UP){
			if(is_order_upstairs(last_floor)){
				set_state_to_drive(DIRN_UP);
			} else if(is_order_downstairs(last_floor+1)){
				set_state_to_drive(DIRN_DOWN);
			}
		}else if(motor_direction == DIRN_DOWN){
			if(is_order_downstairs(last_floor)){
				set_state_to_drive(DIRN_DOWN);
			}else if(is_order_upstairs(last_floor-1)){
				set_state_to_drive(DIRN_UP);
			}
		}
		break;

	//default:
		//printf("Emergency!"); No stdhio.h included
	}
}
