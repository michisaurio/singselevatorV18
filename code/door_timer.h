/* door_timer.h
The door_timer module is used to control that the elevator door has been opened
for a predefined amount of time (time threshold).
*/

/*
A structure for the door timer is implemented.
A static variable of this type, named door_timer is defined in door_timer.c.
*/
typedef struct door_timer_struct {
  double start_time;    // in seconds
  double elapsed_time;  // in seconds
  int is_timer_on;
} door_timer_t;

/*
The member variable is_timer_on of door_timer is set equal to 1.
The member variable start_time of door_timer is set to the time of the day
in seconds using gettimeofdagy() (see time.h).
*/
void start_door_timer();

/*
The member variable is_timer_on of door_timer is set equal to 0.
*/
void reset_door_timer();

/*
If the door timer is on, the member variable elapsed_time is set to how much time
in seconds has passed since the door timer was started the last time. Error <= 1e-6
*/
void update_door_timer();

/*
Returns 1 if the elapsed time is over the predefined time threshold.
*/
int is_elapsed_time_over_threshold();
