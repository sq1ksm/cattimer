#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define BUTTON_PIN 4 // Pin przycisku do przełączania czasu letniego i zimowego
#define EEPROM_ADDRESS 0 // Adres w EEPROM do przechowywania stanu czasu

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;

bool isSummerTime = false; // Zmienna przechowująca stan czasu (letni/zimowy)
unsigned long lastDebounceTime = 0; // Czas ostatniego przełączenia
const unsigned long debounceDelay = 50; // Opóźnienie drgań styków (debounce delay)

void setup() {
  Serial.begin(38400);

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Ustawienie pinu przycisku jako wejścia z podciąganiem
  
  if (!rtc.begin()) {
    Serial.println("Nie można znaleźć RTC");
    while (1);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.display();

  // Odczyt stanu czasu z EEPROM
  isSummerTime = EEPROM.read(EEPROM_ADDRESS);
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && millis() - lastDebounceTime > debounceDelay) {
    isSummerTime = !isSummerTime; // Przełączanie między czasem letnim i zimowym
    EEPROM.write(EEPROM_ADDRESS, isSummerTime); // Zapis stanu czasu do EEPROM
    lastDebounceTime = millis(); // Aktualizacja czasu ostatniego przełączenia
  }
  
  DateTime now = rtc.now();

  // Przeliczenie czasu na letni lub zimowy
  int timeOffset = 0;
  String timeLabel = "UTC";

  if (isSummerTime) {
    timeOffset = -2; // Czas letni, UTC - 2
    timeLabel += "-L";
  } else {
    timeOffset = -1; // Czas zimowy, UTC - 1
    timeLabel += "-Z";
  }

  now = now + TimeSpan(0, timeOffset, 0, 0);

  // Odczyt statusu transceivera (częstotliwość i tryb pracy)
  String status = readStatus();
  String frequency = status.substring(0, 10); // Częstotliwość ma 10 znaków
  String mode = status.substring(27, 28); // Tryb jest jednym znakiem na pozycji 28

  // Tłumaczenie kodu trybu na nazwę trybu
  if (mode == "1") mode = "LSB";
  else if (mode == "2") mode = "USB";
  else if (mode == "3") mode = "CW";
  else if (mode == "4") mode = "FM";
  else if (mode == "5") mode = "AM";
  else mode = ""; // Nie wyświetlaj nic, jeśli tryb pracy jest nieznany
  
  // Usunięcie początkowych zer z częstotliwości
  frequency.trim();
  while (frequency.startsWith("0")) {
    frequency = frequency.substring(1);
  }

  // Formatowanie częstotliwości z kropką
  if (frequency.length() == 6) {
    // Dodanie pierwszej kropki po pierwszej cyfrze
frequency = frequency.substring(0, 1) + "." + frequency.substring(1);

// Dodanie drugiej kropki po czwartej cyfrze (po pierwszej kropce)
frequency = frequency.substring(0, 5) + " " + frequency.substring(5);

  } else if (frequency.length() == 7) {
    frequency = frequency.substring(0, 2) + "." + frequency.substring(2);
  }

  // Formatowanie daty, godzin, minut i sekund
  String day = (now.day() < 10 ? "0" : "") + String(now.day());
  String month = (now.month() < 10 ? "0" : "") + String(now.month());
  String date = day + "." + month + "." + String(now.year());
  String hour = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String minute = (now.minute() < 10 ? "0" : "") + String(now.minute());
  String second = (now.second() < 10 ? "0" : "") + String(now.second());
  
  // Wyczyść wyświetlacz
  display.clearDisplay();
  
  // Wyświetlanie daty
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(date);

  // Wyświetlanie godziny w większym rozmiarze
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.print(hour + ":" + minute);
  
  // Wyświetlanie sekundnika w mniejszym rozmiarze
  display.setTextSize(2);
  display.setCursor(104, 6);
  display.print(second);
  display.setTextSize(1);
  // Wyświetlanie etykiety czasu letniego lub zimowego
  display.setCursor(95, 25);
  display.print(timeLabel);

  // Wyświetlanie poziomej kreski na wysokości 11
  display.drawLine(70, 0, SCREEN_WIDTH, 0, SSD1306_WHITE);
  display.drawLine(0, 35, SCREEN_WIDTH, 35, SSD1306_WHITE);
  display.drawLine(0, 63, SCREEN_WIDTH, 63, SSD1306_WHITE);

  // Wyświetlanie częstotliwości i trybu pracy w nowej linijce
  display.setCursor(0, 40);
  display.setTextSize(2);
  display.print(frequency);
  display.setTextSize(1);

  // Jeśli tryb nie jest pusty, dodaj " MHz " i tryb, w przeciwnym razie "---"
  if (mode != "") {
    display.setCursor(100, 40);
    display.print("MHz");
    display.setCursor(100, 50);
    display.print("-" + mode);
  } else {
    display.print("~Czekam na dane CAT ~");
    
  }

  // Aktualizacja wyświetlacza
  display.display();
  
  delay(1000);
}

String readStatus() {
  Serial.print("IF;");
  delay(100); // Małe opóźnienie dla stabilności

  String response = "";
  while (Serial.available()) {
    char c = Serial.read();
    response += c;
    if (c == ';') break;
  }
  
  // Usunięcie prefiksu 'IF' i zakończenia ';'
  if (response.startsWith("IF")) {
    response = response.substring(2, response.length() - 1);
  }

  return response;
}
