/*
 * End-to-end smoke test for the C client against a running pmlib_server
 * configured with tests/settings-dummy.json (a Dummy device whose line 0
 * starts at 0.01 W and grows by 1 W per sample, at 1 Hz).
 *
 * The client library calls exit() on protocol/connection errors, so any
 * wire-protocol mismatch with the server also fails this test.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pmlib.h"

#define FAIL(...) do { fprintf(stderr, "FAIL: " __VA_ARGS__); fputc('\n', stderr); return 1; } while (0)

int main(int argc, char *argv[])
{
    int port = argc > 1 ? atoi(argv[1]) : 16526;
    server_t server;
    device_t dev;
    counter_t counter;
    line_t lines;
    char **devices;
    int num_devices, i;

    pm_set_server("127.0.0.1", port, &server);

    pm_get_devices(server, &devices, &num_devices);
    if (num_devices != 1 || strcmp(devices[0], "DummyMeter") != 0)
        FAIL("pm_get_devices returned %d device(s), expected only DummyMeter", num_devices);

    pm_get_device_info(server, "DummyMeter", &dev);
    if (dev.max_frecuency != 1 || dev.n_lines != 24)
        FAIL("pm_get_device_info: max_freq=%d n_lines=%d, expected 1 and 24",
             dev.max_frecuency, dev.n_lines);

    LINE_CLR_ALL(&lines);
    LINE_SET(0, &lines);
    pm_create_counter("DummyMeter", lines, 1, 0, server, &counter);
    pm_start_counter(&counter);
    sleep(3);
    pm_stop_counter(&counter);
    pm_get_counter_data(&counter);

    pm_measures *m = &counter.measures->energy;
    if (m->watts_size < 2)
        FAIL("expected at least 2 samples in 3 s at 1 Hz, got %d", m->watts_size);
    for (i = 1; i < m->watts_size; i++)
        if (fabs(m->watts[i] - m->watts[i-1] - 1.0) > 1e-6)
            FAIL("sample %d is %f after %f, expected +1", i, m->watts[i], m->watts[i-1]);

    printf("C client OK: %d devices, %d samples (%.2f .. %.2f W)\n",
           num_devices, m->watts_size, m->watts[0], m->watts[m->watts_size-1]);
    pm_finalize_counter(&counter);
    return 0;
}
