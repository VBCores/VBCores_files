/*
 * VBCores IO Board v0.6
 * Тест электрических функций платы на модуле VB32G4 (STM32G474RE).
 * Electrical function test for the VB32G4 (STM32G474RE) based board.
 *
 * CAN в этом скетче не инициализируется и не используется.
 * CAN is not initialized or used in this sketch.
 * Для запуска проверки раскомментируйте нужную функцию в loop().
 * Uncomment the required function in loop() to run a test.
 * Желательно выполнять только один силовой тест за один раз.
 * Run only one power-output test at a time.
 */

#include <VBCoreG4_arduino_system.h>
#include <Wire.h>

// -----------------------------------------------------------------------------
// Назначение выводов по разъёмам платы
// Board connector-to-pin mapping
// -----------------------------------------------------------------------------

// X1 — вход питания платы. Управляющих выводов STM32 нет.
// X1 is the board power input and has no STM32 control pins.

// X3 — выход 12 В для сигнальной колонны / индикаторов.
// X3 provides 12 V outputs for a signal tower or other indicators.
// Контакты: 1 GND, 2 Red, 3 Yellow, 4 Green, 5 Buzzer.
// Pins: 1 GND, 2 Red, 3 Yellow, 4 Green, 5 Buzzer.
#define X3_RED       PB3
#define X3_YELLOW    PB4
#define X3_GREEN     PB5
#define X3_BUZZER    PB6

// X4 и X5 — силовые MOSFET-ключи верхнего плеча.
// X4 and X5 are high-side MOSFET power outputs.
// ВНИМАНИЕ: на выходе появляется VIN, а не стабилизированные 12 В.
// WARNING: the output voltage is VIN, not regulated 12 V.
// Контакты: 1 GND, 2 switched VIN.
// Pins: 1 GND, 2 switched VIN.
#define X4_OUT1      PC12
#define X5_OUT2      PC11

// X6 и X7 — реле.
// X6 and X7 are relay outputs.
// Контакты: 1 NC, 2 COM, 3 NO.
// Pins: 1 NC, 2 COM, 3 NO.
#define X6_RELAY1    PA15
#define X7_RELAY2    PC10

// Совместимые имена из исходного теста.
// Compatibility names used by the original test.
#define Relay1       X6_RELAY1
#define Relay2       X7_RELAY2

// X8 и X9 — CAN/CAN FD. Намеренно не определяются и не используются.
// X8 and X9 are CAN/CAN FD connectors and are intentionally not used here.

// X10–X13 — дискретные входы.
// X10–X13 are digital inputs.
// Контакты каждого разъёма: 1 GND, 2 Digital input.
// Each connector has pin 1 GND and pin 2 Digital input.
// По схеме вход активен при замыкании на GND: LOW = сработал.
// The input is active when shorted to GND: LOW means active.
#define X10_DI1      PC5
#define X11_DI2      PA7
#define X12_DI3      PA6
#define X13_DI4      PA4

// X14 и X15 — аналоговые входы 0...3,3 В.
// X14 and X15 are 0–3.3 V analog inputs.
// Контакты: 1 GND, 2 +3.3 V, 3 Analog input.
// Pins: 1 GND, 2 +3.3 V, 3 Analog input.
#define X14_AI1      PA0
#define X15_AI2      PA1

// X16 — I2C4 и дополнительный GPIO.
// X16 provides I2C4 and an additional GPIO.
// Контакты: 1 GND, 2 VCC 3.3/5 V, 3 SDA, 4 SCL, 5 PB0.
// Pins: 1 GND, 2 VCC 3.3/5 V, 3 SDA, 4 SCL, 5 PB0.
#define X16_I2C_SDA  PC7
#define X16_I2C_SCL  PC6
#define X16_GPIO     PB0

// Внутреннее измерение напряжения питания, делитель 1:16.
// Internal supply-voltage measurement through a 1:16 divider.
#define VIN_SENSE    PC0

// Встроенные органы управления модуля VB32G4.
// On-board controls of the VB32G4 module.
// LED1, LED2 и USR_BTN определены в VBCoreG4_arduino_system.h.
// LED1, LED2, and USR_BTN are defined in VBCoreG4_arduino_system.h.

// -----------------------------------------------------------------------------
// Параметры теста
// Test parameters
// -----------------------------------------------------------------------------

#define OUTPUT_ON_LEVEL       HIGH
#define OUTPUT_OFF_LEVEL      LOW
#define DIGITAL_INPUT_ACTIVE  LOW

const uint32_t TEST_ON_TIME_MS  = 1000;
const uint32_t TEST_PAUSE_MS    = 500;
const uint32_t INPUT_UPDATE_MS  = 300;

const float ADC_REFERENCE_V     = 3.3f;
const float ADC_MAX_CODE        = 4095.0f;  // АЦП 12 бит / 12-bit ADC
const float VIN_DIVIDER_RATIO   = 16.0f;

// Отдельный экземпляр I2C для выводов I2C4 на X16.
// Dedicated I2C instance for the I2C4 pins on X16.
// Конструктор STM32duino: TwoWire(SDA, SCL).
// STM32duino constructor: TwoWire(SDA, SCL).
TwoWire WireI2C4(X16_I2C_SDA, X16_I2C_SCL);

// -----------------------------------------------------------------------------
// Прототипы функций
// Function prototypes
// -----------------------------------------------------------------------------

void configureOutputOff(uint32_t pin);
void setAllControlledOutputsOff();
void pulseOutput(uint32_t pin, const char *name,
                 uint32_t onTimeMs = TEST_ON_TIME_MS,
                 uint32_t pauseMs = TEST_PAUSE_MS);
void printDigitalInput(const char *name, uint32_t pin);
void printAnalogInput(const char *name, uint32_t pin);

void testRelays();
void testMosfetOutputs();
void testStackLightOutputs();
void testAllControlledOutputs();
void testDigitalInputs();
void testAnalogInputs();
void testVinSense();
void testI2C4Scanner();
void testX16Gpio();
void testOnboardLeds();
void testUserButton();
void printAllInputs();

// -----------------------------------------------------------------------------
// Инициализация
// Initialization
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  // Сначала записываем LOW, затем переводим выводы в режим OUTPUT.
  // Write LOW before changing the pins to OUTPUT mode.
  // Это уменьшает вероятность короткого импульса на нагрузке при запуске.
  // This reduces the chance of a short output pulse during startup.
  configureOutputOff(X3_RED);
  configureOutputOff(X3_YELLOW);
  configureOutputOff(X3_GREEN);
  configureOutputOff(X3_BUZZER);

  configureOutputOff(X4_OUT1);
  configureOutputOff(X5_OUT2);

  configureOutputOff(X6_RELAY1);
  configureOutputOff(X7_RELAY2);

  configureOutputOff(LED1);
  configureOutputOff(LED2);

  // Дискретные входы уже имеют внешнюю схему формирования уровня.
  // The digital inputs already have external level-conditioning circuits.
  // Внутренние подтяжки STM32 не включаем.
  // Do not enable the STM32 internal pull-up resistors.
  pinMode(X10_DI1, INPUT);
  pinMode(X11_DI2, INPUT);
  pinMode(X12_DI3, INPUT);
  pinMode(X13_DI4, INPUT);

  pinMode(X14_AI1, INPUT_ANALOG);
  pinMode(X15_AI2, INPUT_ANALOG);
  pinMode(VIN_SENSE, INPUT_ANALOG);
  analogReadResolution(12);

  // Кнопка USR на модуле VB32G4 замыкает вход на GND.
  // The USR button on the VB32G4 module shorts the input to GND.
  pinMode(USR_BTN, INPUT_PULLUP);

  // PB0 и линии I2C4 здесь специально не настраиваются.
  // PB0 and the I2C4 lines are intentionally not configured here.
  // Они будут инициализированы только соответствующей тестовой функцией.
  // They are initialized only by their corresponding test functions.

  setAllControlledOutputsOff();

  Serial.println();
  Serial.println("VBCores IO Board v0.6 electrical test");
  Serial.println("CAN is not initialized");
  Serial.println("Uncomment one or more test functions in loop()");
  Serial.println();
}

// -----------------------------------------------------------------------------
// Выбор выполняемых тестов
// Select the tests to run
// -----------------------------------------------------------------------------

void loop() {
  // Раскомментируйте нужную строку.
  // Uncomment the required line.

  // testRelays();               // X6, X7
  // testMosfetOutputs();        // X4, X5 — на выходе будет VIN! / Output voltage is VIN!
  // testStackLightOutputs();    // X3: Red, Yellow, Green, Buzzer
  // testAllControlledOutputs(); // Последовательная проверка X3–X7 / Sequential X3–X7 test

  // testDigitalInputs();        // X10–X13, состояние в Serial / state output to Serial
  // testAnalogInputs();         // X14, X15, код АЦП и напряжение / ADC code and voltage
  // testVinSense();             // Измерение VIN через делитель 1:16 / VIN through 1:16 divider
  // printAllInputs();           // DI, AI, VIN и кнопка / DI, AI, VIN, and button

  // testI2C4Scanner();          // X16: поиск устройств / scan devices on SDA/SCL
  // testX16Gpio();              // X16 pin 5: импульс 3,3 В / 3.3 V pulse on PB0

  // testOnboardLeds();          // LED1 и LED2 / LED1 and LED2 on the VB32G4 module
  // testUserButton();           // Кнопка USR / USR button on the VB32G4 module

  delay(50);
}

// -----------------------------------------------------------------------------
// Общие служебные функции
// Common helper functions
// -----------------------------------------------------------------------------

void configureOutputOff(uint32_t pin) {
  digitalWrite(pin, OUTPUT_OFF_LEVEL);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, OUTPUT_OFF_LEVEL);
}

void setAllControlledOutputsOff() {
  digitalWrite(X3_RED, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_YELLOW, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_GREEN, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_BUZZER, OUTPUT_OFF_LEVEL);

  digitalWrite(X4_OUT1, OUTPUT_OFF_LEVEL);
  digitalWrite(X5_OUT2, OUTPUT_OFF_LEVEL);

  digitalWrite(X6_RELAY1, OUTPUT_OFF_LEVEL);
  digitalWrite(X7_RELAY2, OUTPUT_OFF_LEVEL);

  digitalWrite(LED1, OUTPUT_OFF_LEVEL);
  digitalWrite(LED2, OUTPUT_OFF_LEVEL);
}

void pulseOutput(uint32_t pin, const char *name,
                 uint32_t onTimeMs, uint32_t pauseMs) {
  Serial.print(name);
  Serial.println(" -> ON");
  digitalWrite(pin, OUTPUT_ON_LEVEL);
  delay(onTimeMs);

  digitalWrite(pin, OUTPUT_OFF_LEVEL);
  Serial.print(name);
  Serial.println(" -> OFF");
  delay(pauseMs);
}

void printDigitalInput(const char *name, uint32_t pin) {
  const int level = digitalRead(pin);

  Serial.print(name);
  Serial.print(" = ");
  Serial.print(level == HIGH ? "HIGH" : "LOW");
  Serial.print(" (contact ");
  Serial.print(level == DIGITAL_INPUT_ACTIVE ? "CLOSED / ACTIVE" : "OPEN");
  Serial.println(")");
}

void printAnalogInput(const char *name, uint32_t pin) {
  const uint32_t raw = analogRead(pin);
  const float voltage = (raw / ADC_MAX_CODE) * ADC_REFERENCE_V;

  Serial.print(name);
  Serial.print(" = ");
  Serial.print(raw);
  Serial.print(" ADC, ");
  Serial.print(voltage, 3);
  Serial.println(" V");
}

// -----------------------------------------------------------------------------
// Тесты выходов
// Output tests
// -----------------------------------------------------------------------------

void testRelays() {
  digitalWrite(X6_RELAY1, OUTPUT_OFF_LEVEL);
  digitalWrite(X7_RELAY2, OUTPUT_OFF_LEVEL);

  Serial.println("--- Relay test ---");
  pulseOutput(X6_RELAY1, "X6 Relay 1 / PA15");
  pulseOutput(X7_RELAY2, "X7 Relay 2 / PC10");
}

void testMosfetOutputs() {
  digitalWrite(X4_OUT1, OUTPUT_OFF_LEVEL);
  digitalWrite(X5_OUT2, OUTPUT_OFF_LEVEL);

  Serial.println("--- MOSFET output test: output voltage is VIN ---");
  pulseOutput(X4_OUT1, "X4 OUT1 / PC12");
  pulseOutput(X5_OUT2, "X5 OUT2 / PC11");
}

void testStackLightOutputs() {
  digitalWrite(X3_RED, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_YELLOW, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_GREEN, OUTPUT_OFF_LEVEL);
  digitalWrite(X3_BUZZER, OUTPUT_OFF_LEVEL);

  Serial.println("--- X3 12 V output test ---");
  pulseOutput(X3_RED, "X3 Red / PB3");
  pulseOutput(X3_YELLOW, "X3 Yellow / PB4");
  pulseOutput(X3_GREEN, "X3 Green / PB5");
  pulseOutput(X3_BUZZER, "X3 Buzzer / PB6", 500, TEST_PAUSE_MS);
}

void testAllControlledOutputs() {
  setAllControlledOutputsOff();
  Serial.println("=== All controlled outputs ===");

  testStackLightOutputs();
  testMosfetOutputs();
  testRelays();

  setAllControlledOutputsOff();
  Serial.println("=== All controlled outputs are OFF ===");
  delay(1000);
}

// -----------------------------------------------------------------------------
// Тесты входов
// Input tests
// -----------------------------------------------------------------------------

void testDigitalInputs() {
  Serial.println("--- Digital input test ---");
  printDigitalInput("X10 DI1 / PC5", X10_DI1);
  printDigitalInput("X11 DI2 / PA7", X11_DI2);
  printDigitalInput("X12 DI3 / PA6", X12_DI3);
  printDigitalInput("X13 DI4 / PA4", X13_DI4);
  Serial.println();
  delay(INPUT_UPDATE_MS);
}

void testAnalogInputs() {
  Serial.println("--- Analog input test ---");
  printAnalogInput("X14 AI1 / PA0", X14_AI1);
  printAnalogInput("X15 AI2 / PA1", X15_AI2);
  Serial.println();
  delay(INPUT_UPDATE_MS);
}

void testVinSense() {
  const uint32_t raw = analogRead(VIN_SENSE);
  const float adcVoltage = (raw / ADC_MAX_CODE) * ADC_REFERENCE_V;
  const float inputVoltage = adcVoltage * VIN_DIVIDER_RATIO;

  Serial.println("--- VIN sense test ---");
  Serial.print("PC0 = ");
  Serial.print(raw);
  Serial.print(" ADC, ADC pin = ");
  Serial.print(adcVoltage, 3);
  Serial.print(" V, calculated VIN = ");
  Serial.print(inputVoltage, 2);
  Serial.println(" V");
  Serial.println();
  delay(INPUT_UPDATE_MS);
}

void printAllInputs() {
  testDigitalInputs();
  testAnalogInputs();
  testVinSense();
  testUserButton();
  Serial.println("============================");
  delay(500);
}

// -----------------------------------------------------------------------------
// Тест X16
// X16 tests
// -----------------------------------------------------------------------------

void testI2C4Scanner() {
  Serial.println("--- X16 I2C4 scanner: SDA PC7, SCL PC6 ---");

  WireI2C4.begin();
  WireI2C4.setClock(100000);

  uint8_t found = 0;

  // Пропускаем зарезервированные адреса I2C.
  // Skip reserved I2C addresses.
  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    WireI2C4.beginTransmission(address);
    const uint8_t error = WireI2C4.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 0x10) {
        Serial.print('0');
      }
      Serial.println(address, HEX);
      ++found;
    }
  }

  if (found == 0) {
    Serial.println("No I2C devices found");
    Serial.println("Check the X16 VCC/pull-up solder jumper and wiring");
  } else {
    Serial.print("Devices found: ");
    Serial.println(found);
  }

  WireI2C4.end();
  Serial.println();
  delay(1000);
}

void testX16Gpio() {
  Serial.println("--- X16 pin 5, PB0 GPIO test ---");
  Serial.println("PB0 output level is 3.3 V");

  configureOutputOff(X16_GPIO);
  pulseOutput(X16_GPIO, "X16 GPIO / PB0");

  // После теста прекращаем активно управлять линией.
  // Stop actively driving the line after the test.
  digitalWrite(X16_GPIO, LOW);
  pinMode(X16_GPIO, INPUT);
}

// -----------------------------------------------------------------------------
// Тест элементов модуля VB32G4
// VB32G4 on-board component tests
// -----------------------------------------------------------------------------

void testOnboardLeds() {
  Serial.println("--- VB32G4 onboard LED test ---");
  pulseOutput(LED1, "LED1 / PD2", 300, 200);
  pulseOutput(LED2, "LED2 / PA5", 300, 500);
}

void testUserButton() {
  const bool pressed = (digitalRead(USR_BTN) == LOW);

  Serial.print("USR button / PC13 = ");
  Serial.println(pressed ? "PRESSED" : "released");
  delay(INPUT_UPDATE_MS);
}

