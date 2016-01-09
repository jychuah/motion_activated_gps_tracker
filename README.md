#Motion Activated GPS Tracker

This project is a work in progress. It is designed to be a motion activated GPS tracker for use in securing a motorcycle and alerting the owner if the bike is agitated, such as in the case of theft or a hit and run knockover. It will then activate and track GPS coordinates and alert the user over cellular.

###Hardware Components
- [Arduino Trinket Pro](https://www.adafruit.com/products/2000)
- [Adafruit MMA8451 Accelerometer Breakout](https://www.adafruit.com/product/2019)
- [Adafruit SIM808 GPS/GSM Arduino Shield](https://www.adafruit.com/products/2542)
- A SIM card with a GSM data plan.

I will be migrating the product to the Particle.io Electron development board once they are released. I will attempt to make the API endpoints stay the same.

### Software Components
- [HTML](./html): Web interface for configuring and interacting with tracker
- [api_endpoints](.html): API end points for tracker to register events
- [avr_firmware](.html): AVR based firmware for tracker

Many thanks and acknowledgements to the engineers at [Adafruit](http://www.adafruit.com). Please support their business, as this project was based off of products and libraries made by their hard work.