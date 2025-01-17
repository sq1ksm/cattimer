OLED Display with RTC and Frequency Display
This Arduino project utilizes an OLED display and a RTC module to show the current date, time, and frequency information. It also allows switching between summer and winter time using a button and saves this state in the EEPROM. When no frequency information is available, a placeholder text is displayed instead.
Components Used
Arduino Nano
DS3231 RTC Module
SSD1306 OLED Display
Button for switching between summer and winter time
EEPROM for storing the time mode (summer/winter)

Features
Date and Time Display: Shows the current date and time with seconds.
Frequency Display: Displays the frequency information. Adds a decimal point for readability.
Mode Display: Shows the current mode (LSB, USB, CW, FM, AM). If no mode information is available, a placeholder text ("Czytam") is displayed.
Summer/Winter Time Switch: Button to switch between summer and winter time. The selected mode is saved in the EEPROM.
Debouncing: Includes debouncing for the button to avoid multiple toggles.
Code Description
The code initializes the OLED display, RTC module, and button. It reads the current date and time from the RTC and formats it for display. The button state is read to switch between summer and winter time, and the selected mode is saved to and read from the EEPROM.

How to Use
Connect the Components:
OLED display to the I2C pins on the Arduino.
DS3231 RTC module to the I2C pins on the Arduino.
Button to pin D4 on the Arduino.

Upload the Code: Upload the provided code to the Arduino Nano using the Arduino IDE.

Switch Time Mode: Press the button to switch between summer and winter time. The selected mode will be displayed and saved.
