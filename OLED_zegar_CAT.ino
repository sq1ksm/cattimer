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

String lastFrequency = "";
String lastMode = "";
unsigned long lastCatDataTime = 0;
const unsigned long catDataTimeout = 2000; // 2 sekundy

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
  handleButton();
  DateTime now = rtc.now();

  updateDateTime(now);
  updateFrequency();
  delay(500); // Zwiększenie częstotliwości odświeżania
}

void handleButton() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && millis() - lastDebounceTime > debounceDelay) {
    isSummerTime = !isSummerTime; // Przełączanie między czasem letnim i zimowym
    EEPROM.write(EEPROM_ADDRESS, isSummerTime); // Zapis stanu czasu do EEPROM
    lastDebounceTime = millis(); // Aktualizacja czasu ostatniego przełączenia
  }
}

void updateDateTime(DateTime now) {
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

  // Formatowanie daty, godzin, minut i sekund
  String day = (now.day() < 10 ? "0" : "") + String(now.day());
  String month = (now.month() < 10 ? "0" : "") + String(now.month());
  String date = day + "." + month + "." + String(now.year());
  String hour = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String minute = (now.minute() < 10 ? "0" : "") + String(now.minute());
  String second = (now.second() < 10 ? "0" : "") + String(now.second());

  // Wyczyść tylko sekcję daty i czasu
  display.fillRect(0, 0, SCREEN_WIDTH, 35, SSD1306_BLACK);

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
  display.drawLine(0, 34, SCREEN_WIDTH, 34, SSD1306_WHITE);

  // Aktualizacja wyświetlacza
  display.display();
}

void updateFrequency() {
  // Odczyt statusu transceivera (częstotliwość i tryb pracy)
  String status = readStatus();
  unsigned long currentMillis = millis();
  
  if (status != "") {
    lastFrequency = status.substring(0, 10); // Częstotliwość ma 10 znaków
    lastCatDataTime = currentMillis;
    String mode = status.substring(27, 28); // Tryb jest jednym znakiem na pozycji 28

    // Tłumaczenie kodu trybu na nazwę trybu
    if (mode == "1") lastMode = "LSB";
    else if (mode == "2") lastMode = "USB";
    else if (mode == "3") lastMode = "CW";
    else if (mode == "4") lastMode = "FM";
    else if (mode == "5") lastMode = "AM";
    else lastMode = ""; // Nie wyświetlaj nic, jeśli tryb pracy jest nieznany
  }
  
  // Usunięcie początkowych zer z częstotliwości
  lastFrequency.trim();
  while (lastFrequency.startsWith("0")) {
    lastFrequency = lastFrequency.substring(1);
  }

  // Formatowanie częstotliwości z kropką
  if (lastFrequency.length() == 6) {
    lastFrequency = lastFrequency.substring(0, 1) + "." + lastFrequency.substring(1);
    lastFrequency = lastFrequency.substring(0, 5) + " " + lastFrequency.substring(5);
  } else if (lastFrequency.length() == 7) {
    lastFrequency = lastFrequency.substring(0, 2) + "." + lastFrequency.substring(2);
  }

  // Wyczyść tylko sekcję częstotliwości i trybu
  display.fillRect(0, 35, SCREEN_WIDTH, 29, SSD1306_BLACK);
  display.setCursor(0, 39);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  if (currentMillis - lastCatDataTime < catDataTimeout) {
    display.print(lastFrequency);

    // Wyświetlanie modulacji
    display.setTextSize(1);
    if (lastMode != "") {
      display.setCursor(100, 40);
      display.print("MHz");
      display.setCursor(100, 50);
      display.print("-" + lastMode);
    }
  } else {
    display.setTextSize(1); // Zmieniamy czcionkę na mniejszą
    display.setCursor(0, 45); // Dopasowanie pozycji
    display.print("~Czekam na dane CAT ~");
  }

  display.display();
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
