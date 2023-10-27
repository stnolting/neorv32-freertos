#!/usr/bin/env bash

set -e

cd $(dirname "$0")

# run FreeRTOS demo
make -i USER_FLAGS+="-DUART0_SIM_MODE" GHDL_RUN_FLAGS="--stop-time=500us" clean_all sim

# check UART0 output file if program execution was successful
if grep -rniq ../neorv32/sim/simple/neorv32.uart0.sim_mode.text.out -e 'NEORV32 running FreeRTOS'
then
  echo "Test PASSED!"
  exit 0
else
  echo "Test FAILED!"
  exit 1
fi
