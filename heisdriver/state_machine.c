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

#include "elev.h"

int closestOrderUp(int floor){
	int dist = 0;
	for(int i=N_FLOORS-1;i>floor;i--){
		for(int j=0; j<N_BUTTONS;j++){			
				if(elev_get_button_lamp(i,j)==1){
					dist = i-floor;
			}
		}
	}
	return dist;
}

int closestOrderDown(int floor){
	int dist = 0;
	for(int i=0;i<floor;i++){
		for(int j=0; j<N_BUTTONS;j++){			
				if(elev_get_button_lamp(i,j)==1){
					dist = floor-i;
			}
		}
	}
	return dist;
}

void determineNextState(state_type_t* state, elev_motor_direction_t* motor_dir_memory, int* last_floor){

	switch(*state){
	
	case IDLE:
		//checks if there is an order at the current floor, and opens the door if it is the case
		if(elev_get_button_lamp(BUTTON_COMMAND,*last_floor)==1 || elev_get_button_lamp(BUTTON_CALL_UP,*last_floor)==1 || elev_get_button_lamp(BUTTON_CALL_DOWN,*last_floor)==1) {
			*state = OPENDOOR;
			printf("The door is open! Come inside\n");
		}
		 else 
		{ //checks if there are orders in the floors over or under, and decides where to go 
			int distOrderUp = closestOrderUp(last_floor);
			int distOrderDown = closestOrderDown(last_floor);
			if (distOrderUp>0 && distOrderDown >= distOrderUp) {
				*motor_dir_memory = DIRN_UP;
				elev_set_motor_direction(*motor_dir_memory);
				*state = DRIVE;
				printf("Naa kjoerer vi!\n");
			} else if (distOrderDown>0 || distOrderUP>distOrderDown) {
				*motor_dir_memory = DIRN_DOWN;
				elev_set_motor_direction(*motor_dir_memory);
				*state = DRIVE;
				printf("Naa kjoerer vi!\n");
			}
		}
		break;
		
	case DRIVE:
		int current_floor = elev_get_floor_sensor_signal();
		if(current_floor>=0){	//A floor is reached
			*last_floor=current_floor;
			elev_set_floor_indicator(last_floor);	//updates floor indicator
			if(elev_get_button_lamp(BUTTON_COMMAND, *last_floor)){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_COMMAND, *last_floor, 0);
				*state = OPENDOOR;
				printf("The door is open! Come inside\n");
			}else if(*motor_dir_memory==DIRN_UP && elev_get_button_lamp(BUTTON_UP,*last_floor)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_UP, *last_floor, 0);
				*state = OPENDOOR;
				printf("The door is open! Come inside\n");
			}else if (*motor_dir_memory==DIRN_DOWN && elev_get_button_lamp(BUTTON_DOWN,*last_floor)==1){
				elev_set_motor_direction(DIRN_STOP);
				elev_set_button_lamp(BUTTON_DOWN, *last_floor, 0);
				*state = OPENDOOR;
				printf("The door is open! Come inside\n");
			}
		}
		break;
		
		
	case OPENDOOR:
		printf("The door is closing! Hurry up!\n");
		elev_set_button_lamp(BUTTON_COMMAND, *last_floor, 0);	//Don't be an idiot
		int distOrderUp = closestOrderUp(*last_floor);
		int distOrderDown = closestOrderDown(*last_floor);
		if(*motor_dir_memory = DIR_UP){
			if(elev_get_button_lamp(BUTTON_UP,*last_floor)==1) || distOrderUp>0){
				elev_set_button_lamp(BUTTON_UP, *last_floor, 0);
				elev_set_motor_direction(motor_dir_memory);
				*state=DRIVE;
			}else if(distOrderDown>0){
				elev_set_motor_direction(DIRN_DOWN);
				*state=DRIVE;
			}else{
				elev_set_motor_direction(DIRN_STOP);
				*state=IDLE;
			}
		}
		if(*motor_dir_memory = DIR_DOWN){
			if(elev_get_button_lamp(BUTTON_DOWN,*last_floor)==1 || distOrderDown>0){
				elev_set_button_lamp(BUTTON_DOWN, *last_floor, 0);
				elev_set_motor_direction(*motor_dir_memory);
				*state=DRIVE;
			}else if(distOrderUp>0){
				elev_set_motor_direction(DIRN_DOWN);
				*state=DRIVE;
			}else{
				elev_set_motor_direction(DIRN_STOP);
				*state=IDLE;
			}
		}
		break;
		
	default:
		printf("Emergency!!! Push the STOP button");
	}		
}
