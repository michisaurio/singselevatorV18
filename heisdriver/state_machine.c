/*
void determineDirection(elev_motor_direction_t* motor_dir, int floor){
	//idle
	if(*motor_dir==DIRN_STOP){
		if(floor<=ceil(N_FLOORS/2)){
			for(int i=0; i<N_FLOORS;i++){
				if(i!=floor && elev_get_button_lamp(BUTTON_CALL_DOWN,floor)==1))

					elev_set_motor_direction(DIRN_DOWN);
					break;
				} else if(i!=floor && elev_get_button_lamp(BUTTON_CALL_UP,floor)==1))
					elev_set_motor_direction(DIRN_UP);
					break;
				}
			}
		} else(){
			for(int i=N_FLOORS-1;i>=0;i--){
				if(i!=floor && elev_get_button_lamp(BUTTON_CALL_UP,floor)==1))
					elev_set_motor_direction(DIRN_UP);
					break;
				}else if(i!=floor && elev_get_button_lamp(BUTTON_CALL_DOWN,floor)==1))
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
				if(elev_get_button_lamp(b,f)==1){
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
				if(elev_get_button_lamp(b,f)==1){
					dist = floor-f;
			}
		}
	}
	return dist;
}

void determineNextState(){
	// Assumes that we start at IDLE state, on a floor.
	// Se tiene que iniciar motor_dir_memory igual a DIRN_STOP?
	static state_type_t state = IDLE;
	static elev_motor_direction_t motor_dir_memory = DIRN_STOP;
	int current_floor = elev_get_floor_sensor_signal();
	double open_door_threshold_time = 3.0;
	//printf("Sensor signal: %d \n",current_floor);

	switch(state){

	case IDLE:
		//printf("State IDLE\n");
		//checks if there is an order at the current floor, and opens the door if it is the case
		if(elev_get_button_lamp(BUTTON_COMMAND,current_floor)==1 || elev_get_button_lamp(BUTTON_CALL_UP,current_floor)==1 || elev_get_button_lamp(BUTTON_CALL_DOWN,current_floor)==1) {
			state = OPENDOOR;
			door_timer_start();
			elev_set_door_open_lamp(1);
			printf("La puerta se abre. Alguien quiere entrar.\n");
		}
		 else
		{ //checks if there are orders in the floors over or under, and decides where to go
			int disOrderUp = closestOrderUp(current_floor);
			int distOrderDown = closestOrderDown(current_floor);
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
		//int current_floor = elev_get_floor_sensor_signal();
		if(current_floor>=0){	//A floor is reached
			//current_floor=current_floor;
			elev_set_floor_indicator(current_floor);	//updates floor indicator
			if(elev_get_button_lamp(BUTTON_COMMAND, current_floor)){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_COMMAND, current_floor, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien se quiere bajar. \n");
			}else if(motor_dir_memory==DIRN_UP && elev_get_button_lamp(BUTTON_CALL_UP,current_floor)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_CALL_UP, current_floor, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien quiere unirse camino hacia arriba.\n");
			}else if (motor_dir_memory==DIRN_DOWN && elev_get_button_lamp(BUTTON_CALL_DOWN,current_floor)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_CALL_DOWN, current_floor, 0);
				state = OPENDOOR;
				door_timer_start();
				elev_set_door_open_lamp(1);
				printf("La puerta se abre. Alguien quiere unirse camino hacia abajo.\n");
			}
		}
		break;


	case OPENDOOR:
		door_timer_update();
		elev_set_button_lamp(BUTTON_COMMAND, current_floor, 0);
		printf("No seas imbécil y entra!!!")

		if(is_elapsed_time_over_threshold(open_door_threshold_time)){
			int disOrderUp = closestOrderUp(current_floor);
			int distOrderDown = closestOrderDown(current_floor);
			if(motor_dir_memory == DIRN_UP){
				if(elev_get_button_lamp(BUTTON_CALL_UP,current_floor)==1 || disOrderUp>0){
					elev_set_button_lamp(BUTTON_CALL_UP, current_floor, 0);
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
				if(elev_get_button_lamp(BUTTON_CALL_DOWN,current_floor)==1 || distOrderDown>0){
					elev_set_button_lamp(BUTTON_CALL_DOWN, current_floor, 0);
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
