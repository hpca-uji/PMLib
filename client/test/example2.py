import libpmlib
import time

if __name__ == "__main__":
    frequency= 0
    aggregate= 0;
    
    server = libpmlib.pm_create_server("127.0.0.1", 6526)
    lines = libpmlib.pm_create_lines("0-2")
    counter = libpmlib.pm_create_counter("Jetson-Orin-Nano", lines, aggregate, frequency, server)
    
    libpmlib.pm_start_counter(counter)
    time.sleep(10)
    libpmlib.pm_stop_counter(counter)

    libpmlib.pm_get_counter_data(counter)
    libpmlib.pm_print_data_text("out.txt", counter, lines, -1)
    libpmlib.pm_finalize_counter(counter)
    
