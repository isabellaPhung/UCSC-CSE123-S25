### LCD functionality Verification
Introduction: The main form of interaction the user has to the productivity device is the LCD touchscreen. This aspect must remain functional to allow for the user to view and interact with their schedule. The following test ensures the screen behaves as expected. 

Scope: LCD Screen

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: LCD visuals

Dependent variables: power on behaviour (planned)

Procedure:

1. Turn on the device and connect it to the host machine. Visually confirm that the LCD boots within 10 seconds of powering on.
2. Run the test_screen.py pytest tool. This test program runs through a gamut of visuals and will calibrate touch input.

Expectation: LCD is visually confirmed to be capable of displaying a full range of RGB colors with no dead pixels or cracked screen. Touch input is reactive and calibrated such that touches are accurate to the screen display.
