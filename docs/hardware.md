# Hardware Reference

This document records the verified pin assignments, power wiring, and board-level
test procedure for the NUCLEO-F746ZG controller.

## Board and clocks

- Board: NUCLEO-F746ZG, STM32F746ZG.
- System clock: 72 MHz.
- Debug transport: SWD through the onboard debugger. The tested board identifies
  as a SEGGER J-Link; boards still identifying as ST-LINK need an ST-LINK tool.
- USART3 is initialized at 115200 8N1 (PD8 TX, PD9 RX) for future use. The
  current firmware sends no runtime serial output.

## Pin map

| Signal | Pin | Mode | Notes |
| --- | --- | --- | --- |
| Valve 1 output | PE2 | Push-pull output | HIGH opens, LOW closes |
| Valve 2 output | PE3 | Push-pull output | HIGH opens, LOW closes |
| Valve 3 output | PE4 | Push-pull output | HIGH opens, LOW closes |
| Valve 1 button | PE5 | EXTI, pull-down | Active-high, debounced |
| Valve 2 button | PE6 | EXTI, pull-down | Active-high, debounced |
| Valve 3 button | PF0 | EXTI, pull-down | Active-high, debounced |
| Linear actuator button | PF1 | EXTI, pull-down | Active-high, debounced |
| L298N IN1 | PF2 | Push-pull output | LOW at boot |
| L298N IN2 | PF3 | Push-pull output | LOW at boot |
| LD1 | PB0 | Push-pull output | Blinks only while a valve is closing |
| User button | PC13 | Input | Not used by the application |
| USART3 TX | PD8 | Alternate function | Virtual COM port TX |
| USART3 RX | PD9 | Alternate function | Virtual COM port RX |

Buttons connect between the pin and 3.3 V. All valve and actuator outputs are
driven LOW at startup.

## 12 V actuator power and L298N wiring

- Connect the 12 V supply **V+** to the L298N motor supply input only.
- Connect the 12 V supply **V−** to the L298N **GND** terminal and to the Nucleo
  **GND**. V− is the circuit reference, not necessarily earth ground.
- Connect the two actuator motor leads to L298N OUT1/OUT2.
- Leave the ENA jumper installed. There is no ENA GPIO.
- Do not connect the L298N 5 V terminal to the Nucleo.
- Do not connect 12 V to any Nucleo pin.
- Check the actuator stall current against the driver and supply rating before
  powering the load. Begin away from both mechanical ends.

The L298N direction truth table is:

| IN1 (PF2) | IN2 (PF3) | Actuator |
| --- | --- | --- |
| LOW | LOW | Stop/brake |
| HIGH | LOW | Extend |
| LOW | HIGH | Retract |
| HIGH | HIGH | Never commanded |

If the physical direction is reversed, exchange the motor leads.

## Valve behavior

Each valve button starts one timed operation. A valve opens, dwells fully open,
closes, and then enters a lockout. Operations are exclusive: a second button press
while a cycle is active is ignored. LD1 blinks at 250 ms only during the closing
phase. These are commanded states; there is no valve position feedback.

## Linear actuator behavior

The first debounced OFF → ON press commands extend. The next successful start
commands retract. A press during motion stops the actuator, and release only
rearms the button. Each movement stops automatically after 1 s of drive. After a
stop, a 50 ms LOW/LOW interval is enforced before a new direction starts; a press
during that interval schedules the direction, and another press cancels it. A
button held HIGH at boot cannot start motion until released and pressed again.

There are no limit switches or position feedback, so the 1 s timeout is a safety
limit rather than position control.

## Board test checklist

1. Confirm PF2/PF3 are LOW after reset and the actuator does not move.
2. Press PF1 once: actuator extends and stops within 1 s.
3. Press again: actuator retracts and stops within 1 s.
4. Press during motion: actuator stops promptly.
5. Hold PF1 at reset: no motion until release and a new press.
6. Press each valve button: valve opens, dwells, closes; LD1 blinks while closing.
7. Confirm actuator movement does not disturb an active valve cycle.
8. Verify the embedded version with
   `arm-none-eabi-strings build/Debug/wcs_controller.elf | grep '^wcs_controller'`
   and compare it with `CHANGELOG.md`.
