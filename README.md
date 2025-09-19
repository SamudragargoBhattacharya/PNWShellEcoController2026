# PNW Shell Eco Controller 2026
All-in-one motor controller and telemetry platform for the PNW ASME Shell Eco-Marathon vehicle.

## Hardware

### Mainboard (Power Stage)
- MOSFET half/three-phase bridge
- Gate drivers, flyback/TVS diodes
- DC/DC regulators
- Bulk/decoupling capacitors
- Current/voltage sensing
- Vehicle connectors

### Daughterboard (Control + Telemetry)
- **MCU**: [STM32 Nucleo-F042K6](https://www.newark.com/stmicroelectronics/nucleo-f042k6/dev-board-nucleo-32-mcu/dp/68Y1243)
- **GPS**: [Adafruit Ultimate GPS](https://www.adafruit.com/product/746)
- **IMU**: [Adafruit LSM6DSOX + LIS3MDL](https://www.adafruit.com/product/4646)

## Features
- VESC-tool compatibility.
- Telemetry (speed, current, voltage, GPS/IMU).
- Safety: OV/UV, OC, OT, temp sensors, watchdog, e-stop.
- Heads-up display.
- Battery level percentage.
- Status code LED.

## Roadmap
- [ ] **M0**: Initial motor controller with STM32 daughter board using [VESC](https://vesc-project.com/) firmware. 
