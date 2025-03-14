### LCD functionality Verification
Scope: LCD Screen

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: LCD visuals

Dependent variables: power on behaviour (planned)

Procedure:

1. Turn on the device. Visually confirm that the LCD boots within 10 seconds of powering on.
2. Run the test_screen.py pytest tool. This test program runs through a gamut of visuals and will prompt for touch input.

Expectation: LCD is visually confirmed to be capable of displaying a full range of RGB colors with no dead pixels or cracked screen.
