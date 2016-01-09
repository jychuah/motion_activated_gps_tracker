#Motion Activated GPS Tracker

This project is a work in progress. It is designed to be a motion activated GPS tracker for use in securing a motorcycle and alerting the owner if the bike is agitated, such as in the case of theft or a hit and run knockover. It will then activate and track GPS coordinates and alert the user over cellular.

###Hardware Components
- [Arduino Trinket Pro](https://www.adafruit.com/products/2000)
- [Adafruit MMA8451 Accelerometer Breakout](https://www.adafruit.com/product/2019)
- [Adafruit SIM808 GPS/GSM Arduino Shield](https://www.adafruit.com/products/2542)
- A SIM card with a GSM data plan.

I will be migrating the product to the Particle.io Electron development board once they are released. I will attempt to make the API endpoints stay the same.

### Software Components

These are the software components you need to get it working. You will need PHP hosting and a free acount from [Firebase.com](http://firebase.com). Setup each of the software components in order:

- [HTML](./html): Web interface for configuring and interacting with tracker
- [Firebase](./firebase): Rules and configuration for Firebase backend
- [api_endpoints](./api_endpoints): API end points for tracker to register events
- [avr_firmware](./avr_firmware): AVR based firmware for tracker

Many thanks and acknowledgements to the engineers at [Adafruit](http://www.adafruit.com). Please support their business, as this project was based off of products and libraries made by their hard work.

### Licensing

All software components from 3rd parties subject to their original licensing. All original software components and original hardware configuration subject to the following MIT license:

Copyright (c) 2015 Joon-Yee Chuah

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.