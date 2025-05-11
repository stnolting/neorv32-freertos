# FreeRTOS on NEORV32

[![neorv32-freertos](https://img.shields.io/github/actions/workflow/status/stnolting/neorv32-freertos/main.yml?branch=main&longCache=true&style=flat-square&label=neorv32-freertos%20sim&logo=Github%20Actions&logoColor=fff)](https://github.com/stnolting/neorv32-freertos/actions/workflows/main.yml)
[![License](https://img.shields.io/github/license/stnolting/neorv32-freertos?longCache=true&style=flat-square&label=License)](https://github.com/stnolting/neorv32-freertos/blob/main/LICENSE)

* [Requirements](#requirements)
* [How To Run](#how-to-run)
* [Porting Details](#porting-details)

This repository provides a full-featured port of [FreeRTOS](https://www.freertos.org/index.html)
for the [NEORV32](https://github.com/stnolting/neorv32) RISC-V Processor. It implements a simple
demo derived from the FreeRTOS "blinky" demo applications that is extended with some
processor-specific features.


## Requirements

#### Tools and Framework

* [NEORV32](https://github.com/stnolting/neorv32) submodule
* [FreeRTOS kernel-only](https://github.com/FreeRTOS/FreeRTOS-Kernel) submodule
* [RISC-V GCC toolchain](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack)
* [GHDL](https://github.com/ghdl/ghdl) for simulating the processor, optional

#### Minimal Processor Configuration

* CPU architecture and ISA extensions: `rv32i_zicsr_zifencei`
* Memory
  * IMEM (instruction memory): 16kB
  * DMEM (data memory): 8kB
* Peripherals
  * RISC-V machine timer (MTIME)
  * general-purpose timer (GPTMR), optional
  * UART0 as interface console
  * general purpose IO controller (GPIO); high-active LED connected to bits 1:0, LED at pin 0 is used as "heart beat"


## How To Run

1. Clone this repository recursively (to include the submodules):

```bash
$ git clone --recurse-submodules https://github.com/stnolting/neorv32-freertos.git
```

2. Install a RISC-V GCC toolchain that is able to emit code for a 32-bit architecture. Make sure that
(at least) the [required ISA extensions](#requirements) are supported. An exemplary prebuilt toolchain
for x86 Linux can be download from:

[github.com/stnolting/riscv-gcc-prebuilt](https://github.com/stnolting/riscv-gcc-prebuilt)

3. Before you can compile the firmware, you need to make sure your hardware configuration is sync to
the software configuration (ISA configuration and memory layout) If you are using the default configuration
you can move on to the next step. Otherwise you need to adjust the according parts of the [Makefile](demo/makefile):

```makefile
# Override the default CPU ISA and ABI
MARCH = rv32i_zicsr_zifencei
MABI  = ilp32

# Set RISC-V GCC prefix
RISCV_PREFIX ?= riscv-none-elf-

# Override default optimization goal
EFFORT = -Os

# Adjust processor IMEM size and base address
USER_FLAGS += -Wl,--defsym,__neorv32_rom_size=16k
USER_FLAGS += -Wl,--defsym,__neorv32_rom_base=0x00000000

# Adjust processor DMEM size and base address
USER_FLAGS += -Wl,--defsym,__neorv32_ram_size=8k
USER_FLAGS += -Wl,--defsym,__neorv32_ram_base=0x80000000
```


4. Navigate to the `demo` folder and compile the application:

```bash
neorv32-freertos/demo$ make clean_all exe
```

> [!TIP]
> You can check the RISC-V GCC installation by running `make check`.

5. Upload the generated `neorv32_exe.bin` file via the NEORV32 bootloader:

```
<< NEORV32 Bootloader >>

BLDV: Jul 28 2023
HWV:  0x01090003
CLK:  0x05f5e100
MISA: 0x40901105
XISA: 0xc0000fbb
SOC:  0xfffff06f
IMEM: 0x00008000
DMEM: 0x00002000

Autoboot in 8s. Press any key to abort.
Aborted.

Available CMDs:
 h: Help
 r: Restart
 u: Upload
 s: Store to flash
 l: Load from flash
 x: Boot from flash (XIP)
 e: Execute
CMD:> u
Awaiting neorv32_exe.bin... OK
CMD:> e
Booting from 0x00000000...

<<< NEORV32 running FreeRTOS V10.4.4+ >>>

GPTMR IRQ Tick
GPTMR IRQ Tick
GPTMR IRQ Tick
```

> [!TIP]
> Alternatively, you can also use the processor's on-chip debugger to upload the application via the
generated `main.elf` file.

6. If you have GHDL installed you can also run the demo in simulation using the processor's default
testbench / simulation mode:

```bash
neorv32-freertos/demo$ sh sim.sh
```


## Porting Details

The processor-specific FreeRTOS parts are configured by two files:

* `FreeRTOSConfig.h` (customize according to your needs)
* `freertos_risc_v_chip_specific_extensions.h` (do not change!)

The NEORV32-specific parts are configured right inside the `main.c` file and the according `makefile`.
The hardware abstraction layer (HAL) is provided by the NEORV32 software framework, which also provides
the start-up code and linker script.

> [!TIP]
> More information regarding the NEORV32 software framework can be found in the
[online data sheet](https://stnolting.github.io/neorv32/#_software_framework).

The NEORV32 supports all RISC-V exceptions and interrupts plus additional platform-specific
interrupts. As FreeRTOS only supports the MTIME timer interrupt and the "environment call" exception
two additional functions are provided to handle platform-specific exceptions and interrupts:

```c
void freertos_risc_v_application_interrupt_handler(void);
void freertos_risc_v_application_exception_handler(void);
```

These functions are populated in the `main.c` file to showcase how to attach handlers for these traps.

