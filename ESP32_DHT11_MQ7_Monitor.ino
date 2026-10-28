#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// DHT11 configuration
#define DHTPIN 27
#define DHTTYPE DHT11

// Gas sensor analog pin
#define GAS_PIN 34

// LED pins
#define RED_LED 4
#define YELLOW_LED 5
#define GREEN_LED 18

// Gas thresholds (example values)
#define LOW_LIMIT 1000
#define MEDIUM_LIMIT 2000

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
const unsigned long blinkInterval = 500;
bool ledState = false;

void setup()
{
  Serial.begin(115200);

  // Initialize I2C LCD on GPIO 21 and GPIO 22
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Initialize DHT11
  dht.begin();

  // Initialize LEDs
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  analogReadResolution(12);

  lcd.setCursor(0, 0);
  lcd.print("ESP32 Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();
}

void loop()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);

  // Determine gas level
  const char* gasLevel;

  if (gasValue < LOW_LIMIT)
    gasLevel = "LOW";
  else if (gasValue < MEDIUM_LIMIT)
    gasLevel = "MEDIUM";
  else
    gasLevel = "HIGH";

  // LCD row 1: Temperature
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  if (isnan(temperature))
  {
    lcd.print("Temp: Error");
  }
  else
  {
    lcd.print("Temp:");
    lcd.print(temperature, 1);
    lcd.print((char)223);
    lcd.print("C");
  }

  // LCD row 2: Humidity and gas
  // A 16x2 LCD cannot display all three readings clearly
  // on two rows at once, so alternate the second row.
  static bool showHumidity = true;

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if (showHumidity)
  {
    if (isnan(humidity))
      lcd.print("Humidity: Error");
    else
    {
      lcd.print("Humidity:");
      lcd.print(humidity, 0);
      lcd.print("%");
    }
  }
  else
  {
    lcd.print("Gas:");
    lcd.print(gasValue);
    lcd.print(" ");
    lcd.print(gasLevel);
  }

  showHumidity = !showHumidity;

  // Serial Monitor output
  Serial.println("--------------------");

  Serial.print("Temperature: ");
  if (isnan(temperature))
    Serial.println("Read Error");
  else
  {
    Serial.print(temperature);
    Serial.println(" C");
  }

  Serial.print("Humidity: ");
  if (isnan(humidity))
    Serial.println("Read Error");
  else
  {
    Serial.print(humidity);
    Serial.println(" %");
  }

  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  Serial.print("Gas Level: ");
  Serial.println(gasLevel);

  // LED blinking
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkInterval)
  {
    previousMillis = currentMillis;
    ledState = !ledState;
  }

  digitalWrite(YELLOW_LED,
    (gasValue < LOW_LIMIT && ledState) ? HIGH : LOW);

  digitalWrite(GREEN_LED,
    (gasValue >= LOW_LIMIT &&
     gasValue < MEDIUM_LIMIT && ledState) ? HIGH : LOW);

  digitalWrite(RED_LED,
    (gasValue >= MEDIUM_LIMIT && ledState) ? HIGH : LOW);

  delay(1000);
}
