#include "io.h"
#include "elev.h"
#include <stdio.h>


void checkPushedFloorButton(){
	int button_bit;
	for(int i=0; i<N_BUTTONS; i++ ){
		for(int j=0; j<N_FLOORS; j++){
			if(!(i==0 && j==N_FLOORS-1) && !(i==1 && j==0)){
				button_bit=elev_get_button_signal(i,j);
				if(button_bit==1){
					elev_set_button_lamp(i,j,button_bit);
					printf("Alejandro %d, %d, %d\n",i,j,elev_get_button_lamp(i,j));					
				}
			}
		}	
	}

}

void updateFloorLights(){
	int current_floor = elev_get_floor_sensor_signal();
	if(current_floor>=0){
		elev_set_floor_indicator(current_floor);
	}
}
