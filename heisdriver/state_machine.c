/*
void determineDirection(elev_motor_direction_t* motor_dir, int floor){
	//idle
	if(*motor_dir==DIRN_STOP){
		if(floor<=ceil(N_FLOORS/2)){
			for(int i=0; i<N_FLOORS;i++){
				if(i!=floor && get_order_status(BUTTON_CALL_DOWN,floor)==1))

					elev_set_motor_direction(DIRN_DOWN);
					break;
				} else if(i!=floor && get_order_status(BUTTON_CALL_UP,floor)==1))
					elev_set_motor_direction(DIRN_UP);
					break;
				}
			}
		} else(){
			for(int i=N_FLOORS-1;i>=0;i--){
				if(i!=floor && get_order_status(BUTTON_CALL_UP,floor)==1))
					elev_set_motor_direction(DIRN_UP);
					break;
				}else if(i!=floor && get_order_status(BUTTON_CALL_DOWN,floor)==1))
					elev_set_motor_direction(DIRN_DOWN);
					break;
				}
			}
		}
	}
}
*/

#include "state_machine.h"
#include "elev.h"
#include "door_timer.h"
#include <stdio.h> //Eliminar mas tarde

// Mover a otro archivo. Trata más sobre cómo se selecciona la siguiente orden
int closestOrderUp(int floor){
	int dist = -1;
	for(int f=N_FLOORS-1;f>floor;f--){
		for(int b=0; b<N_BUTTONS;b++){
				if(get_order_status(b,f)==1){
					dist = f-floor;
			}
		}
	}
	return dist;
}
// Mover a otro archivo. Trata más sobre cómo se selecciona la siguiente orden
int closestOrderDown(int floor){
	int dist = -1;
	for(int f=0;f<floor;f++){
		for(int b=0; b<N_BUTTONS;b++){
				if(get_order_status(b,f)==1){
					dist = floor-f;
			}
		}
	}
	return dist;
}

// Assumes that we start at IDLE state, on a floor.
// Se tiene que iniciar motor_dir_memory igual a DIRN_STOP?
static state_type_t state = IDLE;
static elev_motor_direction_t motor_dir_memory = DIRN_STOP;

void determine_next_state(){

	int sensor_signal = elev_get_floor_sensor_signal();
	double open_door_threshold_time = 3.0;
	//printf("Sensor signal: %d \n",sensor_signal);

	switch(state){

	case IDLE:
		// In IDLE, sensor_signal>=0, i.e., the elevator is at a floor
		// If cab button pressed, opens door and turns off cab button light.
		if(get_order_status(BUTTON_COMMAND,sensor_signal)==1){
			state = OPENDOOR
			door_timer_start();
			elev_set_door_open_lamp(1);
			clear_order_status(BUTTON_COMMAND,sensor_signal);			
			printf("La puerta se abre. Alguien se quedó dormido en el ascensor.\n");
		} // 
		else if( get_order_status(BUTTON_CALL_UP,sensor_signal)==1 || get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1) {
			state = OPENDOOR;
			door_timer_start();
			elev_set_door_open_lamp(1);
			printf("La puerta se abre. Alguien quiere entrar.\n");
		}
		 else
		{ //checks if there are orders in the floors over or under, and decides where to go
			int disOrderUp = closestOrderUp(sensor_signal);
			int distOrderDown = closestOrderDown(sensor_signal);
			if (disOrderUp>0 && (distOrderDown==-1 || distOrderDown >= disOrderUp)) {
				motor_dir_memory = DIRN_UP;
				elev_set_motor_direction(motor_dir_memory);
				state = DRIVE;
				printf("Se mueve hacia arriba!\n");
			} else if (distOrderDown>0 && (disOrderUp==-1|| disOrderUp>distOrderDown)) {
				motor_dir_memory = DIRN_DOWN;
				elev_set_motor_direction(motor_dir_memory);
				state = DRIVE;
				printf("Se mueve hacia abajo!\n");
			}
		}
		break;

	case DRIVE:
		//int sensor_signal = elev_get_floor_sensor_signal();
		if(sensor_signal>=0){	//A floor is reached
			//sensor_signal=sensor_signal;
			elev_set_floor_indicator(sensor_signal);	//updates floor indicator
			if(get_order_status(BUTTON_COMMAND, sensor_signal)){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_COMMAND, sensor_signal, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien se quiere bajar. \n");
			}else if(motor_dir_memory==DIRN_UP && get_order_status(BUTTON_CALL_UP,sensor_signal)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_CALL_UP, sensor_signal, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien quiere unirse camino hacia arriba.\n");
			}else if (motor_dir_memory==DIRN_DOWN && get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1){
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
			if(motor_dir_memory == DIRN_UP){
				if(get_order_status(BUTTON_CALL_UP,sensor_signal)==1 || disOrderUp>0){
					elev_set_button_lamp(BUTTON_CALL_UP, sensor_signal, 0);
					elev_set_motor_direction(motor_dir_memory);
					state=DRIVE;
				}else if(distOrderDown>0){
					elev_set_motor_direction(DIRN_DOWN);
					state=DRIVE;
				}else{
					elev_set_motor_direction(DIRN_STOP);
					state=IDLE;
				}
			}
			if(motor_dir_memory == DIRN_DOWN){
				if(get_order_status(BUTTON_CALL_DOWN,sensor_signal)==1 || distOrderDown>0){
					elev_set_button_lamp(BUTTON_CALL_DOWN, sensor_signal, 0);
					elev_set_motor_direction(motor_dir_memory);
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
