### Electromagnetic interference testing

Introduction: Electronic devices require rigorous testing to guarantee electromagnetic compatibility and reliable performance throughout their operational lifespan. Electromagnetic interference represents a significant regulatory and performance challenge for portable tablet devices, particularly those integrating multiple wireless functions and high-frequency digital circuits. The primary testing standards utilized for EMI/EMC evaluation include FCC Part 15 for North American markets, which establishes emission limits and certification requirements for both unintentional and intentional radiators. This regulatory framework is complemented by CISPR 32/EN 55032 for emission testing, CISPR 35/EN 55035 for immunity requirements, and IEC 61000-4 series standards for specific immunity phenomena.

Apparatus: productivity device, semi-anechoic chamber, EMI test receiver, calibrated antennas (biconical, log-periodic, horn), line impedance stabilization network (LISN), RF signal generator, power amplifier, coupling/decoupling networks (CDN), ESD generator, near-field probes, host machine for testing

Independent variables: test frequency ranges (150 kHz-30 MHz, 30 MHz-1 GHz, 1-6 GHz), field strength levels (1 V/m, 3 V/m, 10 V/m), EUT operational modes (standby, active use, wireless transmission), antenna orientations (horizontal, vertical polarization), measurement distances (3m, 10m)

Dependent variables: emission amplitude (dBμV, dBμV/m), immunity performance criteria (A, B, C), functional operation status (pass/fail), wireless communication quality (%), touchscreen sensitivity (%), error rates during immunity testing (%), field uniformity measurements (dB)

Procedure:

1. Verify the device is fully charged and operational by powering on and checking all basic functions.

2. FCC Part 15 Baseline Configuration:
   - Record initial device configuration including wireless module states
   - Document operational modes: standby, typical use, maximum transmission
   - Test touch sensitivity and wireless connectivity baseline performance
   - Establish cable configuration per ANSI C63.4 requirements

3. CISPR 32 Conducted Emissions Testing (150 kHz - 30 MHz):
   - Connect device AC adapter through 50μH/50Ω LISN on ground plane
   - Position device on non-metallic table 80 cm above ground plane
   - Configure EMI receiver with 9 kHz resolution bandwidth and quasi-peak detector
   - Exercise all device functions during measurement including wireless transmission
   - Monitor both line and neutral conductors with Class B limits (250-500 μV)

4. ANSI C63.4 Radiated Emissions Testing (30 MHz - 1 GHz):
   - Place device in semi-anechoic chamber 3 meters from receiving antenna
   - Scan antenna height 1-4 meters while rotating device through 360 degrees
   - Use biconical antenna (30-300 MHz) and log-periodic antenna (200 MHz-1 GHz)
   - Measure both horizontal and vertical polarizations with quasi-peak detector
   - Apply Class B limits (100-200 μV/m depending on frequency)

5. Extended Radiated Emissions Testing (1-6 GHz):
   - Continue testing using horn antennas for frequencies above 1 GHz
   - Switch to average detector with 1 MHz resolution bandwidth
   - Focus on wireless module fundamental and harmonic frequencies
   - Document peak emissions during maximum data transmission

6. IEC 61000-4-3 Radiated Immunity Testing (80 MHz - 6 GHz):
   - Generate 3 V/m uniform field using calibrated transmit antennas
   - Apply 80% amplitude modulation at 1 kHz with 1% frequency stepping
   - Monitor device functions continuously during 3-second dwell per frequency
   - Test both horizontal and vertical field polarizations
   - Assess performance per Criterion A (normal operation) requirements

7. IEC 61000-4-6 Conducted Immunity Testing (150 kHz - 80 MHz):
   - Inject 3V EMF using appropriate coupling/decoupling networks
   - Apply 150Ω common-mode impedance for unshielded cables
   - Use 80% amplitude modulation with systematic frequency stepping
   - Monitor charging, data transfer, and wireless functions

8. IEC 61000-4-2 ESD Immunity Testing:
   - Apply ±4 kV contact discharge to conductive surfaces
   - Apply ±8 kV air discharge to insulated surfaces and near openings
   - Test 10 discharges per point with 1-second intervals
   - Evaluate touchscreen functionality and wireless connectivity recovery

9. Wireless Coexistence Testing:
   - Activate all wireless modules simultaneously (Wi-Fi, Bluetooth, cellular)
   - Measure emissions during maximum data throughput scenarios
   - Verify no interference between internal wireless subsystems
   - Test performance across all supported wireless channels

10. Battery Operation Assessment:
    - Repeat critical emission measurements on battery power only
    - Verify compliance maintained without AC adapter connection
    - Document any emission differences between AC and battery operation

11. Multi-Configuration Testing:
    - Test device in portrait and landscape orientations
    - Connect various peripherals (USB devices, audio cables, stylus)
    - Evaluate EMC performance with different screen brightness levels
    - Test with various applications and processing loads active

Expectation: The device should demonstrate full compliance with FCC Part 15 Class B emission limits showing conducted emissions below 250-500 μV quasi-peak (150 kHz-30 MHz) and radiated emissions below 100-200 μV/m at 3 meters (30 MHz-1 GHz), maintain Performance Criterion A during radiated immunity testing at 3 V/m field strength, show no functional degradation during ±4 kV ESD contact discharge testing, exhibit wireless coexistence with no interference between internal modules during simultaneous operation, and demonstrate consistent EMC performance across all operational configurations including battery operation, multiple orientations, and various peripheral connections.