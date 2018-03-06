typedef struct door_timer_struct{
      double start_time;
      double elapsed_time;
      int is_timer_on;
} door_timer_t;

void start_door_timer();

void reset_door_timer();

void update_door_timer();

int is_elapsed_time_over_threshold(double time_threshold);
