/******************************************************************************
 * FreeRTOS Demo for the NEORV32 RISC-C Processor
 * https://github.com/stnolting/neorv32
 ******************************************************************************
 * FreeRTOS Kernel V10.4.4
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 ******************************************************************************/

/* Standard libraries */
#include <stdint.h>

/* FreeRTOS kernel */
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>

/* NEORV32 HAL */
#include <neorv32.h>

/* UART0 transmission speed */
#define BAUD_RATE 19200

/* External definitions */
extern const unsigned __crt0_max_heap;
extern void main_blinky(void);
extern void freertos_risc_v_trap_handler(void);

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file. See https://www.freertos.org/a00016.html */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);

/* Platform specifics */
void vToggleLED(void);
void vSendString(const char * pcString);
static void prvSetupHardware(void);


/******************************************************************************
 * Main function (should never return).
 ******************************************************************************/
int main( void ) {

	prvSetupHardware(); // setup hardware

  neorv32_uart0_printf("<<< NEORV32 running FreeRTOS %s >>>\n\n", tskKERNEL_VERSION_NUMBER); // say hello

  main_blinky(); // run actual application code

  return -1; // we should never reach this
}


//#################################################################################################
// NEORV32-Specific
//#################################################################################################

/******************************************************************************
 * Setup the hardware for this demo.
 ******************************************************************************/
static void prvSetupHardware(void) {

  // install the freeRTOS trap handler
  neorv32_cpu_csr_write(CSR_MTVEC, (uint32_t)&freertos_risc_v_trap_handler);

  // clear GPIO.out port
  neorv32_gpio_port_set(0);

  // setup UART0 at default baud rate, no interrupts
  neorv32_uart0_setup(BAUD_RATE, 0);

  // check heap size configuration
  volatile uint32_t neorv32_max_heap = (uint32_t)&__crt0_max_heap;
  if (neorv32_max_heap != (uint32_t)configTOTAL_HEAP_SIZE){
    neorv32_uart0_printf("WARNING! Incorrect 'configTOTAL_HEAP_SIZE' configuration!\n"
                         "FreeRTOS configTOTAL_HEAP_SIZE: %u bytes\n"
                         "NEORV32 makefile heap size:     %u bytes\n\n",
                         (uint32_t)configTOTAL_HEAP_SIZE, neorv32_max_heap);
  }

  // check clock frequency configuration
  if (NEORV32_SYSINFO->CLK != (uint32_t)configCPU_CLOCK_HZ) {
    neorv32_uart0_printf("WARNING! Incorrect 'configCPU_CLOCK_HZ' configuration!\n"
                         "FreeRTOS configCPU_CLOCK_HZ: %u Hz\n"
                         "NEORV32 clock speed:         %u Hz\n\n",
                         (uint32_t)configCPU_CLOCK_HZ, NEORV32_SYSINFO->CLK);
  }

  // enable and configure further NEORV32-specific modules if required
  // ...

  // enable NEORV32-specific interrupts if required
  // ...
}


/******************************************************************************
 * Handle NEORV32-/application-specific interrupts.
 ******************************************************************************/
void freertos_risc_v_application_interrupt_handler(void) {

  // acknowledge/clear ALL pending fast-interrupt sources (FIRQs) - adapt this for your setup
  neorv32_cpu_csr_write(CSR_MIP, 0);

  // use the value from the mcause CSR to call interrupt-specific handlers
  uint32_t mcause = neorv32_cpu_csr_read(CSR_MCAUSE);

  // debug output
  neorv32_uart0_printf("\n<NEORV32-IRQ> mcause = 0x%x </NEORV32-IRQ>\n", mcause);
}


/******************************************************************************
 * Handle NEORV32-/application-specific exceptions.
 ******************************************************************************/
void freertos_risc_v_application_exception_handler(void) {

  // use the value from the mcause CSR to call exception-specific handlers
  uint32_t mcause = neorv32_cpu_csr_read(CSR_MCAUSE);

  // debug output
  neorv32_uart0_printf("\n<NEORV32-EXC> mcause = 0x%x </NEORV32-EXC>\n", mcause);
}


/******************************************************************************
 * Toggle GPIO.out(0) pin.
 ******************************************************************************/
void vToggleLED(void) {

	neorv32_gpio_pin_toggle(0);
}


/******************************************************************************
 * Send a plain string via UART0.
 ******************************************************************************/
void vSendString(const char * pcString) {

	neorv32_uart0_puts( ( const char * ) pcString );
}


/******************************************************************************
 * Assert terminator.
 ******************************************************************************/
void vAssertCalled(void) {

  volatile uint32_t i;

	taskDISABLE_INTERRUPTS();

	/* Clear LEDs */
  neorv32_gpio_port_set(0);

	/* Flash the lowest 4 LEDs to indicate that assert was hit - interrupts are off
	here to prevent any further tick interrupts or context switches, so the
	delay is implemented as a busy-wait loop instead of a peripheral timer. */
	for (;;) {
		for (i=0; i<(configCPU_CLOCK_HZ/10); i++) {
			__asm volatile( "nop" );
		}
		neorv32_gpio_pin_toggle(0);
		neorv32_gpio_pin_toggle(1);
		neorv32_gpio_pin_toggle(2);
		neorv32_gpio_pin_toggle(3);
	}
}


//#################################################################################################
// FreeRTOS Hooks
//#################################################################################################

/******************************************************************************
 * Hook for failing malloc.
 ******************************************************************************/
void vApplicationMallocFailedHook(void) {

	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created. It is also called by various parts of the
	demo application. If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */

	taskDISABLE_INTERRUPTS();
  neorv32_uart0_puts("FreeRTOS_FAULT: vApplicationMallocFailedHook "
                     "(increase 'configTOTAL_HEAP_SIZE' in FreeRTOSConfig.h)\n");
	__asm volatile( "ebreak" );
	while(1);
}


/******************************************************************************
 * Hook for the idle process.
 ******************************************************************************/
void vApplicationIdleHook(void) {

	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
	task. It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()). If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */

  neorv32_cpu_sleep();
}


/******************************************************************************
 * Hook for task stack overflow.
 ******************************************************************************/
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {

	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook
	function is called if a stack overflow is detected. */

	taskDISABLE_INTERRUPTS();
  neorv32_uart0_puts("FreeRTOS_FAULT: vApplicationStackOverflowHook "
                     "(increase 'configISR_STACK_SIZE_WORDS' in FreeRTOSConfig.h)\n");
	__asm volatile( "ebreak" );
	while(1);
}


/******************************************************************************
 * Hook for the application tick (unused).
 ******************************************************************************/
void vApplicationTickHook(void) {

  __asm volatile( "nop" ); // nothing to do here yet
}
