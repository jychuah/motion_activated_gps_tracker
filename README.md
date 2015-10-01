#Motion Activated GPS Tracker

This project is a work in progress. It is designed to be a motion activated GPS tracker for use in securing a motorbike and alerting the owner if the bike is agitated, such as in the case of theft or another person laying it down. It will then activate and track GPS coordinates and report with GSM.

Uses the following components:

- [Arduino UNO or compatible](https://www.adafruit.com/products/50)
- [Adafruit 9DOF IMU Breakout](https://www.adafruit.com/products/1714)
- [Adafruit SIM808 GPS/GSM Arduino Shield](https://www.adafruit.com/products/2636)
- A SIM card with a GSM data plan. I used T-Mobile. They gave me SIM cards for $0.99 apiece with a free 200 megabyte per month On Demand data plan

Many thanks and acknowledgements to the engineers at [Adafruit](http://www.adafruit.com). Please support their business, as this project was based off of products and tutorials made by their hard work.

You will need to install the following libraries in your Arduino IDE. Download them as zip files from GitHub and them add them as libraries to compile:

- [Adafruit Unified Sensor Library](https://github.com/adafruit/Adafruit_Sensor)
- [LSM303DLHC Library](https://github.com/adafruit/Adafruit_LSM303DLHC)
- [L3GD20 Library](https://github.com/adafruit/Adafruit_L3GD20_U)
- [Adafruit 9DOF Library](https://github.com/adafruit/Adafruit_9DOF)

