// door_timer.c
#include "door_timer.h"
#include <sys/time.h>

#if !defined(NULL)
#define NULL ((void*)0)
#endif

static door_timer_t door_timer;
static const double open_door_time_threshold = 3.0; // in seconds

void
start_door_timer()
{
  door_timer.is_timer_on = 1;
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  door_timer.start_time =
      (double) current_time.tv_sec + (double) current_time.tv_usec * .000001;
}

void
reset_door_timer()
{
  door_timer.is_timer_on = 0;
}

void
update_door_timer()
{
  if (door_timer.is_timer_on)
    {
      struct timeval current_time;
      gettimeofday (&current_time, NULL);
      door_timer.elapsed_time =
  	     (double) current_time.tv_sec +
  	     (double) current_time.tv_usec * .000001 - door_timer.start_time;
    }
}

int
is_elapsed_time_over_threshold()
{
  if (door_timer.is_timer_on
      && door_timer.elapsed_time > open_door_time_threshold)
      return 1;
  else
      return 0;
}
