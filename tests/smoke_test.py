"""End-to-end smoke test for the Python client (client/pmlib.py) against a
running pmlib_server configured with tests/settings-dummy.json.

Run with client/ on PYTHONPATH and libpmlib.so on LD_LIBRARY_PATH.
"""
import sys
import time

import pmlib

port = int(sys.argv[1]) if len(sys.argv) > 1 else 16526

p = pmlib.PMLib("127.0.0.1", port)
p.create_lines("0")
p.create_counter("DummyMeter", 1, 0)
p.start_counter()
time.sleep(3)
p.stop_counter()
p.get_counter_data()

# p.watts is (lines, samples); with aggregate=1 there is a single row.
watts = list(p.watts[0])
if len(watts) < 2:
    sys.exit(f"FAIL: expected at least 2 samples in 3 s at 1 Hz, got {len(watts)}")
if any(abs(b - a - 1.0) > 1e-6 for a, b in zip(watts, watts[1:])):
    sys.exit(f"FAIL: samples should grow by 1 W each, got {watts}")

joules = float(p.get_joules(p.times[0], p.times[-1])[0])
if not joules > 0:
    sys.exit(f"FAIL: expected positive energy, got {joules}")

print(f"Python client OK: {len(watts)} samples ({watts[0]:.2f} .. {watts[-1]:.2f} W), {joules:.2f} J")
p.finalize_counter()
