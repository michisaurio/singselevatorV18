#include "door_timer.h"
#include "elev.h"
#include <sys/time.h>

#if !defined(NULL)
    #define NULL ((void*)0)
#endif

static door_timer_t door_timer;

// Seconds is KING
void door_timer_start(){
  door_timer.is_timer_on = 1;
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  door_timer.start_time = (double)current_time.tv_sec + (double)current_time.tv_usec * .000001;
  elev_set_door_open_lamp(1);
}

void door_timer_reset(){
  door_timer.is_timer_on = 0;
  elev_set_door_open_lamp(0);
}

void door_timer_update(){
  if(door_timer.is_timer_on == 1){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    door_timer.elapsed_time = (double)current_time.tv_sec + (double)current_time.tv_usec * .000001 - door_timer.start_time;
  }
}

int is_elapsed_time_over_threshold(double time_threshold){
    if(door_timer.is_timer_on == 1 && door_timer.elapsed_time >= time_threshold)
      return 1;
    else
      return 0;
}
