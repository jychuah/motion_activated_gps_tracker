#API Endpoints

There are currently two API endpoint types available. Only one of them is currently feasible - the PHP one. Why? Consumer level ATMega328u development boards like the Arduino don't have enough processor power or onboard RAM to implement OpenSSL, which is required for HTTPS communication. HTTPS is required for the Amazon Web Services Lambda endpoint. Therefore, use the PHP one. Dump it in a directory on your web server and make sure you know the URL. You will need it for the AVR firmware configuration.

Why is AWS Lambda code included? When the project migrates to a Particle Electron development board and the fine folks at Particle.io eventually implement HTTPS, then that's what it's there for. Or if you're implementing this on a heftier chip with built in SSL, like the ATMega256.

If you're using AWS, remember that you'll have to configure an IAM profile with e-mail access, and put in all the appropriate keys in the .json configuration files. Then zip up the entire handler and upload it to your Lambda.