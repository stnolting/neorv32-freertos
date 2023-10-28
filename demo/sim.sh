#!/usr/bin/env bash

set -e

cd $(dirname "$0")

# compile executable, generate ASM listing file and install executable as persistent memory image
make USER_FLAGS+="-DUART0_SIM_MODE" clean_all exe asm install

# simulate (-i to ignore the non-zero return code from GHDL when time-terminating)
make -i GHDL_RUN_FLAGS="--stop-time=1ms" sim

# check UART0 output file if program execution was successful
if grep -rniq ../neorv32/sim/simple/neorv32.uart0.sim_mode.text.out -e 'NEORV32 running FreeRTOS'
then
  echo "Test PASSED!"
  exit 0
else
  echo "Test FAILED!"
  exit 1
fi
