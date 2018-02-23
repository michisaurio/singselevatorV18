#include "state_machine.h"
#include "elev.h"
#include "door_timer.h"
#include <stdio.h> //Eliminar mas tarde

// Assumes that we start at IDLE state, on a floor.
// Se tiene que iniciar motor_direction igual a DIRN_STOP?
static state_type_t state = IDLE;
static elev_motor_direction_t motor_direction = DIRN_STOP;

void determine_next_state(){

	int sensor_signal = elev_get_floor_sensor_signal();
	double open_door_threshold_time = 3.0;
	//printf("Sensor signal: %d \n",sensor_signal);

	switch(state){

	case IDLE: // In IDLE, sensor_signal>=0, i.e., the elevator is at a floor
		// From IDLE to OPENDOOR : If button for/at current floor pressed.
		// If cab button pressed
		if(get_order_status(BUTTON_COMMAND,sensor_signal)==1){
			state = OPENDOOR
			door_timer_start();
			elev_set_door_open_lamp(1);
			motor_direction = DIRN_STOP;
			clear_order_status(BUTTON_COMMAND,sensor_signal);			
			printf("La puerta se abre. Alguien se quedó dormido en el ascensor.\n");
		} // If hall button up pressed
		else if(get_order_status(BUTTON_CALL_UP,sensor_signal)==1){
			state = OPENDOOR
			door_timer_start();
			elev_set_door_open_lamp(1);
			motor_direction = DIRN_UP;
			clear_order_status(BUTTON_CALL_UP,sensor_signal);
			printf("La puerta se abre porque alguien quiere subir.\n");
		} // If hall button down pressed
		else if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1){
			state = OPENDOOR;
			door_timer_start();
			elev_set_door_open_lamp(1);
			motor_direction = DIRN_DOWN;
			printf("La puerta se abre porque alguien quiere bajar.\n");
		}
		else if // From IDLE to DRIVE : If button for/at another floor pressed.
		{ // A cab button for another floor is pressed : Biased towards up.
			for(int floor=N_FLOORS-1; floor>sensor_signal; floor--){
				if(get_order_status(BUTTON_COMMAND,floor)==1){
					state = DRIVE;
					motor_direction = DIRN_UP;
					elev_set_motor_direction(motor_direction);
					printf("Alguien se quedó dormido en el ascensor camino hacia arriba.\n");
					break;
				}
			}
			for(int floor=0; floor<sensor_signal; floor++){
				if(get_order_status(BUTTON_COMMAND,floor)==1){
					state = DRIVE;
					motor_direction = DIRN_DOWN;
					elev_set_motor_direction(motor_direction);
					printf("Alguien se quedó dormido en el ascensor camino hacia abajo.\n");
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
				printf("Alguien arriba quiere viajar.\n");				
			} else if (dist_order_downstairs<N_FLOORS && dist_order_downstairs<dist_order_upstairs) {
				state = DRIVE;
				motor_direction = DIRN_DOWN;
				elev_set_motor_direction(motor_direction);
				printf("Alguien abajo quiere viajar.\n");
			}
		}
		// Otherwise stays at IDLE.
		break;

	case DRIVE:
		// From IDLE to OPENDOOR : Is necessary that a floor is reached first.
		if(sensor_signal>=0){	//A floor is reached
			elev_set_floor_indicator(sensor_signal);	//Updates floor indicator
			if(get_order_status(BUTTON_COMMAND, sensor_signal)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_COMMAND, sensor_signal, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien se quiere bajar. \n");
			}else if(motor_direction==DIRN_UP && get_order_status(BUTTON_CALL_UP,sensor_signal)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_CALL_UP, sensor_signal, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien quiere unirse camino hacia arriba.\n");
			}else if (motor_direction==DIRN_DOWN && get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_CALL_DOWN, sensor_signal, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien quiere unirse camino hacia abajo.\n");
			}
		}
		break;

	case OPENDOOR:
		door_timer_update();
		elev_set_button_lamp(BUTTON_COMMAND, sensor_signal, 0);
		printf("No seas imbécil y entra!!!")

		if(is_elapsed_time_over_threshold(open_door_threshold_time)){
			int disOrderUp = closestOrderUp(sensor_signal);
			int distOrderDown = closestOrderDown(sensor_signal);
			if(motor_direction == DIRN_UP){
				if(get_order_status(BUTTON_CALL_UP,sensor_signal)==1 || disOrderUp>0){
					elev_set_button_lamp(BUTTON_CALL_UP, sensor_signal, 0);
					elev_set_motor_direction(motor_direction);
					state=DRIVE;
				}else if(distOrderDown>0){
					elev_set_motor_direction(DIRN_DOWN);
					state=DRIVE;
				}else{
					elev_set_motor_direction(DIRN_STOP);
					state=IDLE;
				}
			}
			if(motor_direction == DIRN_DOWN){
				if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1 || distOrderDown>0){
					elev_set_button_lamp(BUTTON_CALL_DOWN, sensor_signal, 0);
					elev_set_motor_direction(motor_direction);
					state=DRIVE;
				}else if(disOrderUp>0){
					elev_set_motor_direction(DIRN_DOWN);
					state=DRIVE;
				}else{
					elev_set_motor_direction(DIRN_STOP);
					state=IDLE;
				}
			}

		}else{

		}


		break;
	default:
		printf("Tarado! No sabes programar!");
	}
}
