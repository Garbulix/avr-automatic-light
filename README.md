# AVR motion light

This is a simple light controller which turns the lights on or off whether there is motion detected or not. It uses PWM to switch between light modes (it gently turns the lights on and off, like in dimmer). It uses PIR sensor for detecting motion and photoresistior to check the light level of the environment.

#### For the user

All the user have to do is to put PIR sensor and photoresistor in a good place - PIR schould be mounted that it could detect a motion  and photoresistor shouldn't be in shadow nor in very bright light.  
After plugging the power in, built-in diode blinks two times to signalize that it started to work.   
During that work, the program constantly checks if there is someone moving and if it's dark.

- if it's dark and there is someone is moving, the light turns on
- if it isn't dark and someone is moving and lights are turned off, nothing happens
- if there is no move and the lights are turned on, lights are slowly turned off (in that case there is no light check because of light turned on)
- when, during turning the lights off (using PWM), someone appears, the light is turned on immidiately (without using dimming)
