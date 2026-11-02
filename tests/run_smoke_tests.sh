#!/usr/bin/env bash
# Starts pmlib_server with a Dummy device and runs the C and Python client
# smoke tests against it. Expects the server and client/libpmlib.so built:
#
#   cmake -S server -B build && cmake --build build
#   make -C client
#   tests/run_smoke_tests.sh build/pmlib_server
set -euo pipefail

SERVER=$(realpath "${1:?usage: $0 path/to/pmlib_server}")
REPO=$(cd "$(dirname "$0")/.." && pwd)
PORT=16526
WORK=$(mktemp -d)
trap 'kill "$SERVER_PID" 2>/dev/null || true; rm -rf "$WORK"' EXIT

cd "$WORK"
"$SERVER" --configfile "$REPO/tests/settings-dummy.json" > server.out 2>&1 &
SERVER_PID=$!
for _ in $(seq 30); do
    grep -q "Server listening" server.out && break
    kill -0 "$SERVER_PID" 2>/dev/null || { cat server.out; exit 1; }
    sleep 0.5
done
grep -q "Server listening" server.out || { echo "server did not start"; cat server.out; exit 1; }

gcc -Wall -I"$REPO/client" "$REPO/tests/smoke_test.c" -L"$REPO/client" -lpmlib -lm -o smoke_test
LD_LIBRARY_PATH="$REPO/client" ./smoke_test "$PORT"

LD_LIBRARY_PATH="$REPO/client" PYTHONPATH="$REPO/client" python3 "$REPO/tests/smoke_test.py" "$PORT"
