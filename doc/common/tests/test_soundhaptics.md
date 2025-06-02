### Haptics and Sound functionality Verification
Introduction: The productivity device is intended to play small noises and haptics when the user touches and interacts with the device. The following test ensures the speakers and vibration motors behave as expected. 

Scope: Speakers, Vibration motor

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: Sounds and haptics

Dependent variables: power on behaviour (planned)

Procedure:

1. Turn on the device and connect it to the host machine.  
2. Run the test_sound.py pytest tool. This test program will play all of the expected sounds and activate the vibration motor at varying intensities.

Expectation: Device sound is confirmed to work via auditory testing and vibrations are determined functional by touch inspection. 
