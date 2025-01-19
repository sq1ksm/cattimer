<p><strong>OLED Display with RTC and Frequency Display</strong></p>
<p>This Arduino project utilizes an OLED display and a RTC module to show the current date, time, and frequency information. It also allows switching between summer and winter time using a button and saves this state in the EEPROM. When no frequency information is available, a placeholder text is displayed instead.</p>

<p><strong>Components Used</strong></p>
<ul>
<li>Arduino Nano</li>
<li>DS3231 RTC Module</li>
<li>SSD1306 OLED Display</li>
<li>Button for switching between summer and winter time</li>
</ul>

<p><strong>Features</strong></p>
<ul>
<li><strong>Date and Time Display</strong>: Shows the current date and time with seconds.</li>
<li><strong>Frequency Display</strong>: Displays the frequency information. Adds a decimal point for readability.</li>
<li><strong>Mode Display</strong>: Shows the current mode (LSB, USB, CW, FM, AM). If no mode information is available, a placeholder text ("~Czytam~") is displayed.</li>
<li><strong>Summer/Winter Time Switch</strong>: Button to switch between summer and winter time. The selected mode is saved in the EEPROM.</li>
<li><strong>Debouncing</strong>: Includes debouncing for the button to avoid multiple toggles.</li>
</ul>

<p><strong>Code Description</strong></p>
<p>The code initializes the OLED display, RTC module, and button. It reads the current date and time from the RTC and formats it for display. The button state is read to switch between summer and winter time, and the selected mode is saved to and read from the EEPROM.</p>

<p><strong>How to Use</strong></p>
<ol>
<li><strong>Connect the Components</strong>:
   <ul>
   <li>OLED display to the I2C pins on the Arduino.</li>
   <li>DS3231 RTC module to the I2C pins on the Arduino.</li>
   <li>Button to pin D4 on the Arduino.</li>
   </ul>
</li>
<li><strong>Upload the Code</strong>: Upload the provided code to the Arduino Nano using the Arduino IDE.</li>
<li><strong>Switch Time Mode</strong>: Press the button to switch between summer and winter time. The selected mode will be displayed and saved.</li>
</ol>

<p><strong>Supported Modes by CAT (KENWOOD TS-480 & uSDX...)</strong></p>
<ul>
<li>LSB (Lower Side Band)</li>
<li>USB (Upper Side Band)</li>
<li>CW (Continuous Wave)</li>
<li>FM (Frequency Modulation)</li>
<li>AM (Amplitude Modulation)</li>
</ul>

<p><strong>Example Code (First Versions)</strong></p>
<pre>
<code>
#include &lt;Wire.h&gt;
#include &lt;Adafruit_GFX.h&gt;
#include &lt;Adafruit_SSD1306.h&gt;
#include &lt;RTClib.h&gt;
#include &lt;EEPROM.h&gt;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C
#define BUTTON_PIN 4
#define EEPROM_ADDRESS 0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;

bool isSummerTime = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(38400);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!rtc.begin()) {
    Serial.println("Nie można znaleźć RTC");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.display();
  isSummerTime = EEPROM.read(EEPROM_ADDRESS);
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && millis() - lastDebounceTime &gt; debounceDelay) {
    isSummerTime = !isSummerTime;
    EEPROM.write(EEPROM_ADDRESS, isSummerTime);
    lastDebounceTime = millis();
  }

  DateTime now = rtc.now();
  int timeOffset = 0;
  String timeLabel = "UTC";

  if (isSummerTime) {
    timeOffset = -2;
    timeLabel += "-L";
  } else {
    timeOffset = -1;
    timeLabel += "-Z";
  }

  now = now + TimeSpan(0, timeOffset, 0, 0);
  String status = readStatus();
  String frequency = status.substring(0, 10);
  String mode = status.substring(27, 28);

  if (mode == "1") mode = "LSB";
  else if (mode == "2") mode = "USB";
  else if (mode == "3") mode = "CW";
  else if (mode == "4") mode = "FM";
  else if (mode == "5") mode = "AM";
  else mode = "";

  frequency.trim();
  while (frequency.startsWith("0")) {
    frequency = frequency.substring(1);
  }

  if (frequency.length() == 6) {
    frequency = frequency.substring(0, 1) + "." + frequency.substring(1);
  } else if (frequency.length() == 7) {
    frequency = frequency.substring(0, 2) + "." + frequency.substring(2);
  }

  String day = (now.day() &lt; 10 ? "0" : "") + String(now.day());
  String month = (now.month() &lt; 10 ? "0" : "") + String(now.month());
  String date = day + "." + month + "." + String(now.year());
  String hour = (now.hour() &lt; 10 ? "0" : "") + String(now.hour());
  String minute = (now.minute() &lt; 10 ? "0" : "") + String(now.minute());
  String second = (now.second() &lt; 10 ? "0" : "") + String(now.second());

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(date);
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.print(hour + ":" + minute);
  display.setTextSize(2);
  display.setCursor(104, 6);
  display.print(second);
  display.setTextSize(1);
  display.setCursor(95, 25);
  display.print(timeLabel);
  display.setCursor(0, 40);
  display.setTextSize(2);
  display.print(frequency);
  display.setTextSize(1);

  if (mode != "") {
    display.setCursor(90, 40);
    display.print("MHz");
    display.setCursor(100, 50);
    display.print("-" + mode);
  } else {
    display.print("~Czytam~");
  }

  display.display();
  delay(1000);
}

String readStatus() {
  Serial.print("IF;");
  delay(100);
  String response = "";
  while (Serial.available()) {
    char c = Serial.read();
    response += c;
    if (c == ';') break;
  }

  if (response.startsWith("IF")) {
    response = response.substring(2, response.length() - 1);
  }

  return response;
}
</code>
</pre>

<p><strong>License</strong></p>
<p>This project is licensed under the MIT License. See the LICENSE file for details.</p>
