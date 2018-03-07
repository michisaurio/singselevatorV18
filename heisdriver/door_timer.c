#include "door_timer.h"
#include <sys/time.h>
#include <stdio.h>

#if !defined(NULL)
    #define NULL ((void*)0)
#endif

static door_timer_t door_timer;

// Seconds is KING
void start_door_timer(){
  door_timer.is_timer_on = 1;
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  door_timer.start_time = (double)current_time.tv_sec + (double)current_time.tv_usec * .000001;
}

void reset_door_timer(){
  door_timer.is_timer_on = 0;
}

void update_door_timer(){
  if(door_timer.is_timer_on == 1){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    door_timer.elapsed_time = (double)current_time.tv_sec + (double)current_time.tv_usec * .000001 - door_timer.start_time;
  }
}

int is_elapsed_time_over_threshold(double time_threshold){
    if(door_timer.is_timer_on == 1 && door_timer.elapsed_time > time_threshold){
    printf("Over time threshold\n");
      return 1;
  }else{
      printf("%f seconds left\n", time_threshold-door_timer.elapsed_time);
      return 0;
  }
}
