#include <Adafruit_ST7789.h>
#include <Arduino.h>
#include <Keypad_I2C.h>
#include <tinyexpr.h>

// KEYPAD
#define I2CADDR 0x20

const byte ROWS = 5;  // four rows
const byte COLS = 6;  // four columns
// define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
    {'4', ')', '(', '1', '8', 'C'},  // 4   M+   M-   1   8   CE
    {'R', '=', '%', '0', '7', '?'},  // MRC =   %   0   7   x
    {'5', '/', '?', '2', '9', 'F'},  // 5   /   x   2   9   OFF
    {'6', '*', '+', '3', '-', '?'},  // 6   *   +   3   -   x
    {'.', '?', '?', 0xE8, '?', '?'}  // .   x   x   √   x   x
};
byte rowPins[ROWS] = {10, 8, 7, 4, 3};
byte colPins[COLS] = {9, 6, 5, 2, 1, 0};

Keypad_I2C keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);

// LCD
#define TFT_CS 5
#define TFT_RST 3
#define TFT_DC 4
#define TFT_BL 2

Adafruit_ST7789 lcd = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// MATH PARSER
te_parser tep;

// CLASS DECLARATIONS
class LcdLine {
 public:
  String text;
  int cursor_y;
  int cursor_x;
  int size;
  uint16_t color;
  uint16_t background;

  void print() {
    lcd.setCursor(cursor_x, cursor_y);
    lcd.setTextSize(size);
    lcd.setTextColor(color, background);
    lcd.print(text);
  };
};

// FUNCTION DECLARATIONS
void printMenu(const int, const String[], const bool = false);
LcdLine toLcdLine(const String, const int, const int, const int = 2,
                  const uint16_t = ST77XX_WHITE, const uint16_t = ST77XX_BLACK);
bool checkIfSymbol(const char, const char[]);

int mode = 0;  // 0 - Calc | 1 - GPT
String equasion = "0";

void setup() {
  // START KEYPAD
  keypad.begin();
  Wire.setClock(200000);

  // START LCD
  lcd.init(240, 320);
  lcd.setSPISpeed(40000000);
  lcd.setRotation(1);
  // BACKLIGHT
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  const String values[] = {String(equasion), "0.00"};
  printMenu(0, values, true);
}

void loop() {
  char key = keypad.getKey();
  if (mode == 0) {
    if (key) switch (key) {
        case '=':
          break;

        default:
          if (key == 'C')
            equasion = "0";
          else if (key == 'F') {
            equasion.remove(equasion.length() - 1);
            if (equasion == "") equasion = "0";
          } else {
            if (equasion == "0" && ((key >= '0' && key <= '9') || key == '+' ||
                                        key == '-' || key == 0xE8 || key == '('))
              equasion = key;
            else
              equasion += key;
          }

          String values[] = {String(equasion), ""};

          int error;
          if (equasion != "") {
            String tempEquasion = equasion;
            tempEquasion.replace("\xE8", "sqrt");

            int str_len = tempEquasion.length() + 1;
            char char_array[str_len];
            tempEquasion.toCharArray(char_array, str_len);

            values[1] = String(tep.evaluate(char_array));
          }

          printMenu(mode, values, true);
          break;
      }
  }
}

void printMenu(const int mode, const String values[], const bool clear) {
  const int margin = 70;
  switch (mode) {
    case 0:  // Math
      if (clear) lcd.fillScreen(ST77XX_BLACK);
      LcdLine out[] = {
          toLcdLine(values[0], margin, 0),
          toLcdLine("\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3",
                    margin + 60, 0),
          toLcdLine(values[1], margin + 80, 0)};
      // PRINT ON LCD
      for (int i = 0; i < (sizeof(out) / sizeof(out[0])); i++) {
        out[i].print();
      }
      break;
  }
}

LcdLine toLcdLine(const String text, const int cursor_y, const int cursor_x,
                  const int size, const uint16_t color,
                  const uint16_t background) {
  LcdLine object;
  object.text = text;
  object.cursor_y = cursor_y;
  object.cursor_x = cursor_x;
  object.size = size;
  object.color = color;
  object.background = background;
  return object;
}

bool checkIfSymbol(const char symbol, const char array[]) {
  for (int i = 0; i < strlen(array); i++) {
    if (symbol == array[i]) return true;
  }
  return false;
}