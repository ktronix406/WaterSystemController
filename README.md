# Deep Well and Pond Controller

This project is an Arduino-based control system to manage a deep well pump, pond fill valve, and a spigot valve for a whole-home water system. The system allows for manual and automatic control based on user commands and float switch states.

## Features

- Control a deep well pump to manage water supply.
- Open or close the pond fill and spigot valves.
- Respond to float switches that indicate pond water levels.
- Manual and automatic interruption and reset of pond filling process.
- Serial communication for remote commands.
- Winter mode for disabling pond filling during colder seasons.
- Pause button to temporarily stop all operations.

## Components Used

- **Arduino (e.g., Mega)**
- **Deep Well Pump**: Controlled through a relay connected to pin `deepWellPumpPin` (Pin 12).
- **Pond Fill Valve**: Controlled through a relay connected to pin `pondFillValvePin` (Pin 11).
- **Spigot Valve**: Controlled through a relay connected to pin `spigotValvePin` (Pin 8).
- **Float Switches**:
  - Float Switch 1 (Min level) - `floatValve1MinPin` (Pin 2)
  - Float Switch 2 (Max level) - `floatValve2MaxPin` (Pin 3)
- **Interrupt Button**: Connected to pin `interruptButtonPin` (Pin 4).
- **Button Indicator**: Connected to pin `buttonIndicator` (Pin 5).
- **Winter Mode Buttons**:
  - Activate Winter Mode - `winterModeButtonPin` (Pin A0)
  - Deactivate Winter Mode - `deactivateWinterModeButtonPin` (Pin A1)
- **Winter Mode LEDs**:
  - Winter Mode Active LED - `ledWinterModeActivePin` (Pin A2)
  - Winter Mode Inactive LED - `ledWinterModeInactivePin` (Pin A3)
- **Pause Button**: Connected to pin `pauseButtonPin` (Pin A4).
- **Pause LED**: Connected to pin `ledPauseActivePin` (Pin A5).
- **Serial Communication**: Operates at `9600 baud` to receive control commands from a connected computer or microcontroller.

## Getting Started

### Wiring Diagram

1. Connect the **deep well pump** control relay to **pin 12**.
2. Connect the **pond fill valve** control relay to **pin 11**.
3. Connect the **spigot valve** control relay to **pin 8**.
4. Attach **float switch 1** (min) to **pin 2** with `INPUT_PULLUP` configuration.
5. Attach **float switch 2** (max) to **pin 3** with `INPUT_PULLUP` configuration.
6. Connect the **interrupt button** to **pin 4** with `INPUT_PULLUP` configuration.
7. Connect the **button indicator** to **pin 5**.
8. Connect the **winter mode activation button** to **pin A0** with `INPUT_PULLUP` configuration.
9. Connect the **winter mode deactivation button** to **pin A1** with `INPUT_PULLUP` configuration.
10. Connect the **winter mode active LED** to **pin A2**.
11. Connect the **winter mode inactive LED** to **pin A3**.
12. Connect the **pause button** to **pin A4** with `INPUT_PULLUP` configuration.
13. Connect the **pause LED** to **pin A5**.
14. Make sure all components are properly grounded.

### Installation

1. Clone or download this repository.
2. Upload the code to your Arduino using the Arduino IDE.
3. Open the Serial Monitor (`9600 baud`) to interact with the system manually.

### Usage

The following commands can be sent via Serial to control the system:

- **`pump on`**: Turns on the deep well pump.
- **`pump off`**: Turns off the deep well pump and all valves.
- **`pond fill on`**: Starts automatic pond filling (if not in winter mode).
- **`pond fill off`**: Stops pond filling and closes valves.
- **`pond fill reset`**: Resets the pond fill state to default (all valves closed).
- **`open fill valve`**: Opens the pond fill valve.
- **`close fill valve`**: Closes the pond fill valve.
- **`open spigot valve`**: Opens the spigot valve.
- **`close spigot valve`**: Closes the spigot valve.
- **`check float status`**: Displays the current status of the float switches.
- **`activate winter mode`**: Activates winter mode, resetting all systems to default.
- **`deactivate winter mode`**: Deactivates winter mode and resumes normal operation.
- **`activate cistern fill`**: Activates cistern fill mode.
- **`deactivate cistern fill`**: Deactivates cistern fill mode.

### Automatic Logic

The system relies on two float switches to determine the water level in the pond and operate the valves:

- **Float Switch 1 (Min Level)** and **Float Switch 2 (Max Level)** are used to determine the pond water level:
  - If **Min is LOW** and **Max is HIGH**: The pond level is low; take action to fill the pond.
  - If **Min is HIGH** and **Max is LOW**: Pond level is between Min and Max; no action needed.
  - If both are HIGH: High water level detected; stop filling.
  - If both are LOW: Error state; invalid sensor configuration.

The interrupt button (`interruptButtonPin`) can be pressed at any time to stop the pond filling and open the spigot valve.

### Winter Mode and Pause Functionality

- **Winter Mode**: Winter mode can be activated or deactivated using dedicated buttons. When winter mode is active, pond filling is disabled, and all systems are reset to default.
- **Pause Button**: The pause button can be used to temporarily stop all operations. When paused, the system ignores all commands until unpaused.

### Functions

- **`controlDeepWellPump(bool on)`**: Controls the deep well pump.
- **`controlPondFillValve(bool open)`**: Controls the pond fill valve.
- **`controlSpigotValve(bool open)`**: Controls the spigot valve.
- **`handleFloatSwitches()`**: Manages logic based on the current states of the float switches.
- **`checkPauseButton()`**: Checks the state of the pause button to pause or resume operations.
- **`checkWinterModeButtons()`**: Checks the state of the winter mode buttons to activate or deactivate winter mode.
- **`checkButtonPress()`**: Handles the interrupt button logic to manage pond fill and spigot valve.

## Notes

- Make sure that the relay control pins are configured correctly based on whether your relays are active HIGH or active LOW.
- The system is designed to handle both manual and automatic operations; ensure that you are familiar with the Serial commands to operate the system.
- Winter mode disables pond filling to prevent issues during cold weather.

## Troubleshooting

- **Serial Monitor Not Working**: Verify that the baud rate is set to `9600`.
- **Valves Not Operating**: Ensure the wiring is correct and that relays are receiving the right signals.
- **Float Switch Errors**: Double-check the float switches are correctly wired and have valid logic states.
- **Winter Mode Issues**: Ensure the winter mode buttons are properly connected and debounce logic is functioning.

## Contributing

Feel free to submit pull requests or create issues if you have suggestions for improvement or have identified any bugs.

## License

This project is licensed under the MIT License - see the `LICENSE` file for details.
