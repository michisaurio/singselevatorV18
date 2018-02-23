typedef struct door_timer_struct{
      double start_time;
      double elapsed_time;
      int is_timer_on;
} door_timer_t;

void door_timer_start();

void door_timer_reset();

void door_timer_update();

int is_elapsed_time_over_threshold(double time_threshold);
