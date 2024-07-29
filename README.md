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
* Prebuilt [RISC-V GCC toolchain](https://github.com/stnolting/riscv-gcc-prebuilt)
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

3. Navigate to the `demo` folder and compile the application:

```bash
neorv32-freertos/demo$ make clean_all exe
```

> [!TIP]
> You can check the RISC-V GCC installation by running `make check`.

4. Upload the generated `neorv32_exe.bin` file via the NEORV32 bootloader:

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

5. If you have GHDL installed you can also run the demo in simulation using the processor's default
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

As the linker script is also responsible for configuring application- and setup-specific memory layout
the actual configuration has to be overridden according to the application setup. For example the heap
size is configured by `configTOTAL_HEAP_SIZE` in `FreeRTOSConfig.h`. This size also needs to be
configured for the linker script, which is done inside the `makefile`:

```makefile
override USER_FLAGS += "-Wl,--defsym,__neorv32_heap_size=3500"
```

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

