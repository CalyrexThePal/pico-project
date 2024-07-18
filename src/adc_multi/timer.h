#include "hardware/timer.h"

// Low-level sleep function in microseconds
void sleep_us_low_level(uint64_t us) {
    absolute_time_t end_time = make_timeout_time_us(us);
    busy_wait_until(end_time);
}

// Low-level sleep function in milliseconds
void sleep_ms_low_level(uint64_t ms) {
    sleep_us_low_level(ms * 1000);
}