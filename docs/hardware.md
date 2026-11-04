# Hardware Reference

This document records the verified pin assignments, power wiring, and board-level
test procedure for the NUCLEO-F746ZG controller.

## Board and clocks

- Board: NUCLEO-F746ZG, STM32F746ZG.
- System clock: 72 MHz.
- Debug transport: SWD through the onboard debugger. The tested board identifies
  as a SEGGER J-Link; boards still identifying as ST-LINK need an ST-LINK tool.
- RTT telemetry uses the same J-Link SWD connection and does not require a serial
  adapter or additional target pin.
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
| Water sensor OUT | PF4 | ADC3_IN14, analog | 3.3 V sensor signal |
| LD1 | PB0 | Push-pull output | Blinks only while a valve is closing |
| LD2 | PB7 | Push-pull output | Blinks while water is detected |
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

## Water sensor wiring and behavior

The YwRobot/YUROBOT water-level module is powered from **3.3 V** for this
controller. Connect its VCC to the Nucleo 3.3 V pin, GND to Nucleo GND, and OUT
to PF4. Do not power the approximately 35 mA sensor directly from a GPIO. Do not
power it from 5 V without a signal divider because the STM32 ADC input must stay
within the 3.3 V range. Keep the connector out of the water; only the sensing
traces should contact it.

The measured signal is active-high: approximately 0 V dry, 0.5 V damp, 1.3 V at
roughly 25% coverage, and 1.7 V fully covered. Firmware samples ADC3 every 100 ms,
averages four samples, detects water above 400 mV, and clears detection below
200 mV after confirmation samples. LD2 blinks while the state is WET. The reported
signal percentage is normalized against the measured 1.7 V full-scale value; it is
not a calibrated physical water-depth percentage.

## RTT telemetry

The firmware emits CSV on RTT channel 0 every 100 ms:

```text
water_ms,raw,filtered_raw,millivolts,signal_percent,state
```

Open `JLinkRTTViewer.exe` from the SEGGER J-Link installation and select
`STM32F746ZG`, `SWD`, and approximately 4000 kHz. Connect after flashing and view
Terminal 0. J-Link Commander must be closed before RTT Viewer connects.

## Board test checklist

1. Confirm PF2/PF3 are LOW after reset and the actuator does not move.
2. Press PF1 once: actuator extends and stops within 1 s.
3. Press again: actuator retracts and stops within 1 s.
4. Press during motion: actuator stops promptly.
5. Hold PF1 at reset: no motion until release and a new press.
6. Press each valve button: valve opens, dwells, closes; LD1 blinks while closing.
7. Confirm actuator movement does not disturb an active valve cycle.
8. Confirm the water sensor is 0 V dry and rises when the traces are wetted; verify
   LD2 blinks when the detection threshold is crossed.
9. Confirm RTT Terminal 0 reports raw ADC, filtered ADC, millivolts, signal
   percentage, and state.
10. Verify the embedded version with
   `arm-none-eabi-strings build/Debug/wcs_controller.elf | grep '^wcs_controller'`
   and compare it with `CHANGELOG.md`.
