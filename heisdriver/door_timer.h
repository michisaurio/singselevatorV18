typedef struct door_timer{
      double start_time
      double elapsed_time
      int is_timer_on
}

void door_timer_start();

void door_timer_reset();

void door_timer_update();

int is_elapsed_time_over_threshold(double time_threshold);
