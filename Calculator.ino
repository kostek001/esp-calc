/* @file Calculator
|| @version 1.0
|| @author Kostek001
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>

#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

/* =============== KEYPAD =============== */

#define I2CADDR 0x20

const byte ROWS = 5; //four rows
const byte COLS = 6; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'4','?','?','1','8','C'}, // 4   x   x   1   8   CE
  {'R','=','%','0','7','?'}, // MRC =   %   0   7   x
  {'5','/','?','2','9','W'}, // 5   /   x   2   9   OFF
  {'6','*','+','3','-','?'}, // 6   *   +   3   -   x
  {'.','?','?','P','?','?'}  // .   x   x   √   x   x
};
byte rowPins[ROWS] = {10, 8, 7, 4, 3};
byte colPins[COLS] = {9, 6, 5, 2, 1, 0};

Keypad_I2C keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR); 

/* =============== KEYPAD =============== */

/* =============== LCD =============== */

#define TFT_CS 5
#define TFT_RST 3
#define TFT_DC 4
#define TFT_BL 2

Adafruit_ST7789 lcd = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

/* =============== LCD =============== */

/* =============== WebServer =============== */ 

WebServer server(80);

struct settings {
  char message[1024];
  char kod_in[30];
  char kod_web[30];
} memory = {};

/* =============== WebServer =============== */ 

double firstNumber = 0;
double secondNumber = 0;
double total = 0;

double oldFirst = 0;
double oldSecond = 0;
char oldSign = '+';

bool isDecimal1 = false;
float decimals1 = 10.0;
bool isDecimal2 = false;
float decimals2 = 10.0;

int nextOperation = 0;

void setup(){
  //Serial.begin(115200);

  // START KEYPAD
  keypad.begin();
  Wire.setClock(200000);

  // START LCD
  lcd.init(240, 320);
  lcd.setSPISpeed(40000000);
  lcd.setRotation(1);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  //START WEBSERVER
  WiFi.mode(WIFI_AP);
  WiFi.softAP("The Counter", "kostek4321");
  server.on("/",  webControlPortal);
  server.begin();
  WiFi.mode(WIFI_OFF);

  //READ EEPROM (text)
  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, memory);

  // PRINT START
  lcd.fillScreen(ST77XX_BLACK);
  printStart();
}
  
void loop(){
  char key = keypad.getKey();
  
  /*
      if(firstNumber != 0) firstNumber = log(firstNumber);
      else firstNumber = -999999999999999; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber,4);
  
      if(firstNumber != 0) firstNumber = sin(firstNumber);
      else firstNumber = 0; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
  
      if(firstNumber != 0) firstNumber = cos(firstNumber);
      else firstNumber = 1; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
  
      if(firstNumber != 0) firstNumber = tan(firstNumber);
      else firstNumber = 0; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
      
      if(firstNumber != 0) firstNumber = asin(firstNumber);
      else firstNumber = 0; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
  
      if(firstNumber != 0) firstNumber = acos(firstNumber);
      else firstNumber = 1.57; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
  
      if(firstNumber != 0) firstNumber = atan(firstNumber);
      else firstNumber = 0; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
  */
  
  if(key != NO_KEY){
    
    switch(key){
    case '0' ... '9':
      if(isDecimal1 == false){
        lcd.setCursor(20, 125);
        firstNumber = firstNumber * 10 + (key - '0');
        lcd.print(firstNumber, 4);
      }
      else{
        lcd.setCursor(20, 125);
        firstNumber = firstNumber + (key - '0') / decimals1;
        decimals1 = decimals1 * 10;
        lcd.print(firstNumber, 4);
      }
    break;
      
    case '.':
      isDecimal1 = true;
    break;
    
    case '+':
      firstNumber = (total != 0 ? total : firstNumber);
      PLUS:
      lcd.setCursor(20, 145);
      lcd.print("+");
      secondNumber = SecondNumber();

      if(secondNumber){
        total = firstNumber + secondNumber;
      
        oldFirst = firstNumber;
        oldSecond = secondNumber;
        oldSign = '+';
      
        firstNumber = total, secondNumber = 0;
      
        total = 0;
      }
      printStart();
    break;

    case '-':
      firstNumber = (total != 0 ? total : firstNumber);
      MINUS:
      lcd.setCursor(20, 145);
      lcd.print("-");
      secondNumber = SecondNumber();

      if(secondNumber){
        total = firstNumber - secondNumber;

        oldFirst = firstNumber;
        oldSecond = secondNumber;
        oldSign = '-';

        firstNumber = total, secondNumber = 0;
       
        total = 0;
      }
      printStart();
    break;

    case '*':
      firstNumber = (total != 0 ? total : firstNumber);
      MULTIPLY:
      lcd.setCursor(20, 145);
      lcd.print("x");
      secondNumber = SecondNumber();
      
      if(secondNumber){
        total = firstNumber * secondNumber;
      
        oldFirst = firstNumber;
        oldSecond = secondNumber;
        oldSign = 'x';

        firstNumber = total, secondNumber = 0;
      
        total = 0;
      }
      printStart();
    break;

    case '/':
      firstNumber = (total != 0 ? total : firstNumber);
      DIVIDE:
      lcd.setCursor(20, 145);
      lcd.print("/");
      secondNumber = SecondNumber();

      if(secondNumber){
        if(secondNumber == 0){
          lcd.setCursor(20, 185);
          lcd.print("Invalid");\
          delay(500);
        }
        else{
          total = (float)firstNumber / (float)secondNumber;
          oldFirst = firstNumber;
          oldSecond = secondNumber;
          oldSign = '/';
        
          firstNumber = total, secondNumber = 0;
        
          total = 0; 
        }
      }
      printStart();
    break;

    case 'C':
      total = 0;
      firstNumber = 0;
      secondNumber = 0;
      isDecimal1 = false;
      isDecimal2 = false;
      decimals1 = 10;
      decimals2 = 10;
      printStart();
    break;

    case 'P':
      if(firstNumber != 0) firstNumber = sqrt(firstNumber);
      else firstNumber = 0; 
      lcd.setCursor(20, 125);
      lcd.print(firstNumber, 4);
    break;

    case 'W':
      if(firstNumber == 1234) webControl(); //atoi(memory.kod_web)
      else if(firstNumber == atoi(memory.kod_in)) textMode();
    break;
    
    }
  } 
  
  switch(nextOperation){
    case 1:
      goto PLUS;
    break;
    case 2:
      goto MINUS;
    break;
    case 3:
      goto MULTIPLY;
    break;
    case 4:
      goto DIVIDE;
    break;
  }
}

double SecondNumber(){
  double second;

  while(true){
    // ESP.wdtFeed(); // WATCHDOG FEED
    
    char key = keypad.getKey();
    
    if(key >= '0' && key <= '9'){
      if(isDecimal2 == false){
        lcd.setCursor(20, 165);
        second = second * 10 + (key - '0');
        lcd.print(second, 4);
      }
      else{
        lcd.setCursor(20, 165);
        second = second + (key - '0') / decimals2;
        decimals2 = decimals2 * 10;
        lcd.print(second, 4);
      }
    }

    if(key == '.') isDecimal2 = true;
    
    if(key == '='){ 
      isDecimal1 = false;
      isDecimal2 = false;
      decimals1 = 10;
      decimals2 = 10;
      nextOperation = 0;
      break;  //return second; 
    }
    
    else if(key == '+'){
      nextOperation = 1;
      break;
    }
    
    else if(key == '-'){
      nextOperation = 2;
      break;
    } 
    
    else if(key == '*'){
      nextOperation = 3;
      break;
    } 
    
    else if(key == '/'){
      nextOperation = 4;
      break;
    }

    else if(key == 'C'){
      total = 0;
      second = 0;
      second;
      isDecimal1 = false;
      isDecimal2 = false;
      decimals1 = 10;
      decimals2 = 10;
      nextOperation = 0;
      break;
    }
  } 
  return second;
}

void printStart(){
  lcd.fillScreen(ST77XX_BLACK);
  
  lcd.setTextSize(1);
  lcd.setCursor(20, 220);
  lcd.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  lcd.print("Previous---------------------------");
  lcd.setCursor(20, 230);
  lcd.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  lcd.print(oldFirst, 4);
  lcd.print(" ");
  lcd.print(oldSign);
  lcd.print(" ");
  lcd.print(oldSecond, 4);
  
  lcd.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(20, 125);
  lcd.print(firstNumber, 4);
}

void printText(String text[], int line){
  lcd.fillScreen(ST77XX_BLACK);
  lcd.setCursor(0, 125);
  lcd.println(text[line]);
  lcd.println(text[line + 1]);
  lcd.println(text[line + 2]);
  lcd.println(text[line + 3]);
  lcd.println(text[line + 4]);
  lcd.println(text[line + 5]);
  lcd.println(text[line + 6]);
}

void textMode(){
  String message = String(memory.message);
  int messageLength = ceil(message.length() / 26);
  String text[messageLength];
  int line = 0;

  for ( int i = 0; i < messageLength; i++ ){ //
    text[i] = message.substring((i * 26), ((i + 1) * 26));
    //ESP.wdtFeed();
  }

  printText(text, line);
  delay(1000);
  
  while(true){
    //ESP.wdtFeed(); // WATCHDOG FEED
    

    char key = keypad.getKey();
    if(key != NO_KEY){
      if(key == '8'){
        if(line > 0) line--;
        printText(text, line);
      }
      else if(key == '2'){
        if(line < messageLength) line++;
        printText(text, line);
      }
      else break;
    }
  }

  firstNumber = 0;
  printStart();
}

void webControl(){
  WiFi.mode(WIFI_AP);
  WiFi.softAP("The Counter", "kostek4321");
    
  lcd.fillScreen(ST77XX_BLACK);
  lcd.setCursor(20, 125);
  lcd.print("Web Control mode");
  
  while(true){
    // ESP.wdtFeed(); // WATCHDOG FEED
    server.handleClient();

    char key = keypad.getKey();
    if(key != NO_KEY){
      break;
    }
  }

  WiFi.mode(WIFI_OFF);
  firstNumber = 0;
  printStart();
}

void webControlPortal() {
  if (server.method() == HTTP_POST) {
    Serial.println(server.arg("text"));
    strncpy(memory.message, server.arg("text").c_str(), sizeof(memory.message) );
    strncpy(memory.kod_in, server.arg("kod_in").c_str(), sizeof(memory.kod_in) );
    strncpy(memory.kod_web, server.arg("kod_web").c_str(), sizeof(memory.kod_web) );
    memory.message[server.arg("text").length()] = memory.kod_in[server.arg("text").length()] = memory.kod_web[server.arg("text").length()] = '\0';
    EEPROM.put(0, memory);
    EEPROM.commit();
  }
  
  server.send(200,   "text/html", "\
  <!doctype html>\
  <html lang='en'>\
    <head>\
      <meta charset='utf-8'>\
      <meta name='viewport' content='width=device-width, initial-scale=1'>\
      <title>Wifi Setup</title>\
      <style>\
      *,::after,::before{box-sizing:border-box;}\
      body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}\
      .form-textarea{display:block;width:100%;border:1px solid #ced4da;min-height:8em;resize:none;overflow:hidden;}\
      .form-control{display:block;width:100%;border:1px solid #ced4da;min-height:2em;max-height:50vh;resize:none;overflow:hidden;}\
      button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}\
      .form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}\
      </style>\
      <script>\
        function auto_height(elem){\
          elem.style.height = '1px';\
          elem.style.height = (elem.scrollHeight)+'px';\
        }\
      </script>\
    </head>\
    <body>\
      <main class='form-signin'>\
        <form action='/' method='post'>\
          <h1 class=''>NO LEARNING</h1>\
          <br/>\
          <div class='form-floating'>\
            <label>TEXT</label>\
            <textarea name='text' class='form-textarea' onload='auto_height(this)' oninput='auto_height(this)'>" + String(memory.message) + "</textarea>\
          </div>\
          <div class='form-floating'>\
            <label>KOD IN</label>\
            <input type='text' class='form-control' name='kod_in' value='" + String(memory.kod_in) +"'>\
          </div>\
          <div class='form-floating'>\
            <label>KOD WEB</label>\
            <input type='text' class='form-control' name='kod_web' value='" + String(memory.kod_web) +"'>\
          </div>\
          <br/><br/>\
          <button type='submit'>Zapisz</button>\
          <p style='text-align: right'>\
            by Kostek001\
          </p>\
        </form>\
      </main>\
    </body>\
  </html>\
  " );
}
