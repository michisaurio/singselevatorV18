#include "io.h"
#include "elev.h"
#include <stdio.h>

// matrix that registers orders. 1=existing order. 0=no order or button does not exist.
static int order_register_matrix[N_FLOORS][N_BUTTONS]={};	//since static, initialize with 0s

printf("%d\n", order_register_matrix[0][0]);


// checks if a hall or cab button has been pressed. Registers the order. Turns on corresponding light.
void check_pressed_floor_button(){
	for(int button=0; button<N_BUTTONS; button++ ){
		for(int floor=0; floor<N_FLOORS; floor++){
			if(!(button==BUTTON_CALL_UP && floor==N_FLOORS-1) && !(button==BUTTON_CALL_DOWN && floor==0)){
				if(elev_get_button_signal(button,floor)==1){
					order_register_matrix[floor][button]=1;
					elev_set_button_lamp(button,floor,1);
					//printf("Alejandro %d, %d, %d\n",i,j,elev_get_button_lamp(i,j));					
				}
			}
		}
	}

}
// returns 1 if order exists, 0 otherwise.
int get_order_status(elev_button_type_t button, int floor){
	return order_register_matrix[floor][button];
}
// set order status to 0 and turns off light
void clear_order_status(elev_button_type_t button, int floor){
	if(!(button==BUTTON_CALL_UP && floor==N_FLOORS-1) && !(button==BUTTON_CALL_DOWN && floor==0)){
		order_register_matrix[floor][button]=0;
		elev_set_button_lamp(button,floor,0);
	}
}
//
void clear_all_orders(){
	for(int button=0; button<N_BUTTONS; button++ ){
		for(int floor=0; floor<N_FLOORS; floor++){
			clear_order_status(button,floor);
		}
	}
}
// Cab orders are excluded!
int closest_hall_order_upstairs(int current_floor){
	int dist = N_FLOORS;
	for(int floor=N_FLOORS-1;floor>current_floor;floor--){
		for(int button=0; button<N_BUTTONS-1;button++){
				if(get_order_status(button,floor)==1){
					dist = floor-current_floor;
			}
		}
	}
	return dist;
}
// Cab orders are excluded!
int closest_hall_order_downstairs(int current_floor){
	int dist = N_FLOORS;
	for(int floor=0;floor<current_floor;floor++){
		for(int button=0; button<N_BUTTONS-1;button++){
				if(get_order_status(button,floor)==1){
					dist = current_floor-floor;
			}
		}
	}
	return dist;
}

/*
void updateFloorLights(){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor>=0){
		elev_set_floor_indicator(current_floor);
	}
}
*/