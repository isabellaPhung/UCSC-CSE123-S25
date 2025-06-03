### Thermal Cycling: Cold and Dry Heat
Introduction: Thermal Cycling exposes samples to extreme temperature changes, which replicate accelerated aging of the device and ensures that the sample can operate at its rated maximum and minimum temperatures. Test procedures follow [IEC 62133-2](https://www.sz-sts.com/static/upload/file/20230317/1679038143929808.pdf) series standards for safe thermal cycling procedures of small lithium-ion batteries for consumer electronics.

Scope: Operating temperatures, full functionality

Apparatus: productivity device, host machine for functionality tests, thermal testing chamber

Independent variables: full device functionality

Dependent variables: chamber temperature and humidity

Procedure:

1. Place the device inside of the thermal chamber.
2. Feed a cable connecting the device through the port of the thermal chamber. Connect the end of the cable to the monitoring host machine. Check that the productivity device is on, connected to the internet and is loaded with an existing test account with tasks, events, and habits. Host machine should be set up to monitor device functionality for the entire duration of the test.
3. Close the test chamber.
4. Enter temperature profile into test chamber settings in accordance to [IEC 62133-2](https://www.sz-sts.com/static/upload/file/20230317/1679038143929808.pdf) with a temperature maximum of 75C ± 2°C and a minimum of -20C ± 2°C. Up and down ramp should be set to 5C/min. Dwell at min and max temperatures for 4hrs each. Perform a total of five cycles, then set the chamber return to 20C ± 5°C with a ramp of 5C/min and dwell for 2 hrs to allow the device to return to room temperature. Ensure humidity is set as low as possible.

![IEC 60068-2-14 Thermal Cycling Graph](images/thermal.png){width="50%"}

Expectation: Device should remain operational through then entire duration of the test and outer casing should be free of cracks and remain structurally intact.
