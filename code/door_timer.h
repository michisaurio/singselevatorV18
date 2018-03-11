/* door_timer.h
   The door_timer module is used to control that the elevator door has been
   open for at least a predefined amount of time (time threshold).  */

/* A structure for the door timer is implemented. A static variable of
   this type, named door_timer is defined in door_timer.c.  */
typedef struct door_timer_struct
{
  double start_time;    // in seconds
  double elapsed_time;  // in seconds
  int is_timer_on;
} door_timer_t;

/* Starts the door timer: The member is_timer_on of door_timer is set to 1.
   The member start_time of door_timer is set to the time of the day
   in seconds using gettimeofdagy() (see time.h).  */
void
start_door_timer();

/* Resets the door timer: The member is_timer_on of door_timer is set to 0.  */
void
reset_door_timer();

/* Updates the member variable elapsed_time to the time in seconds that has
   passed since the door timer was started.  */
void
update_door_timer();

/* Returns 1 if the elapsed time is over the predefined time threshold.
   Returns 0 otherwise.  */
int
is_elapsed_time_over_threshold();
