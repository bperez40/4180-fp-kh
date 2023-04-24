# DJ Roomba
## 1. Intro
&nbsp;&nbsp;&nbsp;&nbsp;The function of our robot is to drive around while playing music, hence, it's called DJ Roomba.
## 2. Systems Overview
### a. WAV Player
&nbsp;&nbsp;&nbsp;&nbsp;The WAV player plays WAV files using mbed RTOS, similar to how it's done in lab 3. The wave player's hardware consists of:
- [uLCD screen](https://os.mbed.com/users/4180_1/notebook/ulcd-144-g2-128-by-128-color-lcd/)
- [Class D Audio Amplifier Breakout Board](https://os.mbed.com/users/4180_1/notebook/tpa2005d1-class-d-audio-amp/)
- [SD Card Reader Breakout Board](https://www.sparkfun.com/products/544)
- Potentiometer for volume control
- [Bluefruit LE UART Friend](https://os.mbed.com/users/4180_1/notebook/adafruit-bluefruit-le-uart-friend---bluetooth-low-/)

&nbsp;&nbsp;&nbsp;&nbsp;The WAV player loads .wav files on the SD card and uses PWM to transmit the song to the audio amp. Muliple songs are held on the SD card. Songs can be paused or skipped using the Adafruit bluetooth app. When mTrain receives a command from the bluetooth module, it will execute the command, allowing for remote control. Note that the remote control only extends as far as the WAV player functionality.  
  
The uLCD is used to display
### b. Room Navigation
&nbsp;&nbsp;&nbsp;&nbsp;Additionally, the roomba is able to navigate a room autonomously. The robot moves via wheels attached to DC motors and powered by an h-bridge. It senses it's environment using bumper switches mounted to the front of the chassis. The following parts from the course are used on the robot:
- [Dual TB6612FNG Motor Driver](https://www.sparkfun.com/products/14451)
- DC motor
- [Bumper switches](https://www.pololu.com/product/3674)


## 3. Demo
[![DJ Roomba Demo](assets/thumbnail.png)](https://youtu.be/2_f45RawExs "Cat Chases DJ Roomba")