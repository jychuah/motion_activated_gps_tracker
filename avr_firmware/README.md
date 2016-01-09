#AVR Firmware

####This section is subject to deprecation when the hardware platform switches to Particle.io Electron. 

Here's the AVR (Arduino) based firmware for the device itself. Configuration steps:

- Set up a GSM data provider. I use T-Mobile's data only plans. If they require an APN, make sure you write down the APN's name, like "fast.t-mobile.com"
- Make an account on your front end
- Claim a tracker on your front end using the IMEI printed on your SIM808 chip, and write down the UID
- Open <code>motorbike_tracker.ino</code> and in the User Specific Settings:
    - Change the APN to your APN, or make it a blank string
    - Change the UID to your UID
    - Change the HELPER_URL to your full API endpoint URL
- Load the firmware onto your Trinket