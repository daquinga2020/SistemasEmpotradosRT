#include <TimerOne.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <Thread.h>
#include <StaticThreadController.h>
#include <ThreadController.h>
#include "LedThread.h"
#include "DistanceThread.h"

#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3
#define ENTER 4
#define NONE  -1
#define DHTTYPE DHT11;

const int pinredLED = 13;
const int pingreenLED = 11;

const int pinJoyX = A0;
const int pinJoyY = A1;
const int pinJoyButton = 12;

const int pinButton = 2;

const int pinTrigger = 10;
const int pinEcho = 9;

const int pindht11 = 1;

bool detected = false;
bool init_menu = true;
bool selected_drink = false;

bool init_admin = true;
bool selected_option = false;
bool init_menu_admin = true;
bool modifying_menu = false;
bool change_price = false;

int t_begin_dht = 0;
int temphum[2];
int index_menu = 0;
int last_mvmnt = NONE;

int random_time = -1;
int counter_service = 0;
int t_begin_srv = 0;
int total_it = 0;

int index_menu_admin = 0;
int last_mvmnt_admin = NONE;

int *p_ind;

volatile bool ISRreset = false;
volatile bool ISRadmin = false;
long startTime = 0;

float prices[] = {1.00, 1.10, 1.25, 1.50, 2.00};

String *current_menu;
String menu[] = {"i. Cafe Solo","ii. Cafe Cortado","iii. Cafe Doble","iv. Cafe Premium","v. Chocolate"};
String menu_admin[] = {"i. Ver temperatura", "ii. Ver distancia sensor", "iii. Ver contador" ,"iv. Modificar precios"};

byte euro[] = {
  B00000,
  B00111,
  B01000,
  B01000,
  B11110,
  B01000,
  B00111,
  B00000
};

DHT dht(pindht11, DHT11);

//Objeto LCD con los pines correspondientes (rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

ThreadController controller = ThreadController();
LedThread* ledThread = new LedThread(pinredLED);
DistanceThread* distanceThread = new DistanceThread(pinTrigger,pinEcho);

Thread tmphumThread = Thread();
Thread menuThread = Thread();
Thread makedrinkThread = Thread();
Thread adminThread = Thread();

void callbackTempHum() {
    temphum[0] = dht.readTemperature();
    temphum[1] = dht.readHumidity();
    
    if (isnan(temphum[0]) || isnan(temphum[1])) {
      return;
    }
}

void callbackMakeDrink(){

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Preparando");
  lcd.setCursor(0,1);
  lcd.print("cafe ...");

  counter_service++;

  if (total_it == 0) {
    total_it = random_time*20;
  }

  int intensity = (254*counter_service)/total_it;
  analogWrite(pingreenLED,intensity);
  
  if (counter_service == total_it-1) {
    counter_service = 0;
    total_it = 0;
  }
}

void callbackMenu(){

  int movement = read_joystick();

  if (init_menu) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(current_menu[0]);
    lcd.setCursor(0, 1);
    lcd.print(prices[0]);
    lcd.write(byte(0));

    init_menu = false;
  }
  
  if (movement != last_mvmnt) {
    last_mvmnt = movement;

    switch (movement) {
        case UP:
          if (ISRadmin & change_price) {
            prices[index_menu] = prices[index_menu] + 0.05;
          } else {
            move_up();
          }
          break;
        case DOWN:
          if (change_price) {
            if (prices[index_menu] > 0.0) {
              prices[index_menu] = prices[index_menu] - 0.05;
            }
          } else {
            move_down();
          }
          break;
        case ENTER:
          if (!ISRadmin) {
            selected_drink = true;
          } else if (!change_price){
            change_price = true;
          } else {
            change_price = false;
          }
          break;
        case LEFT:
          if (ISRadmin) {
            current_menu = menu_admin;
            selected_option = false;
            menuThread.enabled = false;
            adminThread.enabled = true;
            modifying_menu = false;
            lcd.clear();
          }
          break;
        default: break;
    }
    lcd.clear();
      
    lcd.setCursor(0, 0);
    lcd.print(current_menu[index_menu]);
    lcd.setCursor(0, 1);
    lcd.print(prices[index_menu]);
    lcd.write(byte(0));
  }
}

void callbackAdminMenu() {
  
  int firstClosingBracket = current_menu[index_menu_admin].indexOf(' ');
  int secondClosingBracket = current_menu[index_menu_admin].indexOf(' ', firstClosingBracket + 1);
  
  int movement = read_joystick();

  if (init_menu_admin) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(current_menu[0].substring(0,secondClosingBracket));
    lcd.setCursor(0, 1);
    lcd.print(current_menu[0].substring(secondClosingBracket+1));
    init_menu_admin = false;
  }
  
  if (movement != last_mvmnt_admin) {
    last_mvmnt_admin = movement;
  
    switch (movement) {
        case UP:
          if (!selected_option) {
            move_up();
          }
          break;
        case DOWN:
          if (!selected_option) {
            move_down();
          }
          break;
        case ENTER:
          selected_option = true;
          break;
        case LEFT:
          current_menu = menu_admin;
          selected_option = false;
          break;
        default: break;
      }
      lcd.clear();
      
      lcd.setCursor(0, 0);
      lcd.print(current_menu[index_menu_admin].substring(0,secondClosingBracket));
      lcd.setCursor(0, 1);
      lcd.print(current_menu[index_menu_admin].substring(secondClosingBracket+1));
  }
}

int read_joystick() {

  int output = NONE;
  
  // Leer todos los valores del joystick
  int X = analogRead(pinJoyX);
  int Y = analogRead(pinJoyY);

  bool buttonValue = false;
  buttonValue = digitalRead(pinJoyButton);
  
  if (!buttonValue){
    output = ENTER;
  } else if (X >= 900) {
    output = UP;
  } else if (X <= 100) {
    output = DOWN;
  } else if (Y >= 900) {
    output = RIGHT;
  } else if (Y <= 100) {
    output = LEFT;
  }
  
  return output;
}


void move_up() {
  
  if (ISRadmin & !menuThread.enabled) {
    p_ind = &index_menu_admin;
  } else {
    p_ind = &index_menu;
  }
  
  if (*p_ind <= 0) {
    *p_ind = 0;
  } else {
    *p_ind = *p_ind - 1;
  }
}

void move_down() {
  int max_ind;
  
  if (ISRadmin & !menuThread.enabled) {
    max_ind = sizeof(menu_admin)/sizeof(String);
    p_ind = &index_menu_admin;
  } else {
    max_ind = sizeof(menu)/sizeof(String);
    p_ind = &index_menu;
  }
  
  if (*p_ind >= max_ind - 1) {
    *p_ind = max_ind - 1;
  } else {
    *p_ind = *p_ind + 1;
  }
}

void detect_button() {
  
  if (digitalRead(pinButton) == HIGH) {
    delay(50);

    if (digitalRead(pinButton) == HIGH) {
      startTime = millis();
    }
  }
  
  if (digitalRead(pinButton) == LOW) {
    delay(50);

    if (digitalRead(pinButton) == LOW) {
      if (millis() - startTime + 100 >= 2000 & millis() - startTime + 100 <= 3000 & !ISRadmin) {
        ISRreset = true;
      }
      else if (millis() - startTime + 100 >= 5000) {
        ISRadmin = !ISRadmin;
      }
    }
  }
}

void reset_srv() {
  
  lcd.clear();
  
  detected = false;
  selected_drink = false;
  init_menu = true;
  tmphumThread.enabled = false;
  menuThread.enabled = false;

  random_time = -1;
  t_begin_dht = 0;
  index_menu = 0;
  last_mvmnt = NONE;
  counter_service = 0;
  total_it = 0;
  
  analogWrite(pingreenLED,0);
}

void setup() {
  
  // Inicializacion switch del joystick
  pinMode(pinJoyButton, INPUT_PULLUP);
  
  // Arranque parpadeo de led
  ledThread->setInterval(1000);
  controller.add(ledThread);

  // Arranque medidor de distancia
  distanceThread->setInterval(400);

  // Inicializando el sensor DTH11
  dht.begin();
  tmphumThread.enabled = false;
  tmphumThread.setInterval(400);
  tmphumThread.onRun(callbackTempHum);

  // Inicializando menu
  menuThread.enabled = false;
  menuThread.setInterval(100);
  menuThread.onRun(callbackMenu);

  current_menu = menu;

  // Inicializando proceso de seleccion
  makedrinkThread.enabled = false;
  makedrinkThread.setInterval(50);
  makedrinkThread.onRun(callbackMakeDrink);

  // Inicializacion admin
  adminThread.setInterval(100);
  adminThread.onRun(callbackAdminMenu);

  // Establecemos la semilla en un pin analogico
  randomSeed(analogRead(A2));

  // Inicializar el LCD con el número de  columnas y filas del LCD
  lcd.clear();
  lcd.begin(16, 2);
  lcd.print("CARGANDO ...");
  lcd.createChar(0, euro);
  
  // Activamos pines del LED verde y boton
  pinMode(pingreenLED, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);
}

void loop() {
  
  controller.run();

  // Arranque
  if (ledThread->counter != -1 & ledThread->counter > 5) {
    controller.remove(ledThread);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ESPERANDO");

    lcd.setCursor(0,1);
    lcd.print("CLIENTE");
    ledThread->counter = -1;
  }
  /////////////////////////////

  // Servicio (a)
  if (controller.size() == 0) {
    attachInterrupt(digitalPinToInterrupt(pinButton), detect_button, CHANGE);
    controller.add(distanceThread);
  }
  
  if (ISRreset) {
    reset_srv();
    ISRreset = false;
  }

  if (distanceThread->detected_person() & !detected) {
    detected = true;
    lcd.clear();
    tmphumThread.enabled = true;
    menuThread.enabled = true;
  }

  // Servicio (b)
  if (detected & !ISRadmin) {
    
    if (tmphumThread.shouldRun()) {
      tmphumThread.run();
    }
    
    if (t_begin_dht == 0) {
      t_begin_dht = millis()/1000;
    }
    
    if (millis()/1000 - t_begin_dht <= 5) {
      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.print(temphum[0]);
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0,1);
      lcd.print("Hum: ");
      lcd.print(temphum[1]);
      lcd.print("%");
    } else {
      if (!selected_drink) {
        if (menuThread.shouldRun()) {
          menuThread.run();
        }

      } else {
        if (random_time == -1) {
          makedrinkThread.enabled = true;
          random_time = random(4,8);
          t_begin_srv = millis()/1000;
        }

        if (millis()/1000 - t_begin_srv > random_time) {
          makedrinkThread.enabled = false;

          if (millis()/1000 - t_begin_srv <= random_time+3) {
            lcd.setCursor(0, 0);
            lcd.print("RETIRE BEBIDA");

            lcd.setCursor(0, 1);
            lcd.print("                ");
          } else {
            selected_drink = false;
            random_time = -1;
            counter_service = 0;
            total_it = 0;
            analogWrite(pingreenLED,0);
          }
        }
        if (makedrinkThread.shouldRun()) {
          makedrinkThread.run();
        }
      }
    }
  }
  /////////////////////////////////////////

  // Modo Admin
  if (ISRadmin){
    if (init_admin) {
      digitalWrite(pinredLED, HIGH);
      digitalWrite(pingreenLED,HIGH);
      current_menu = menu_admin;
      init_admin = false;
      init_menu_admin = true;
      index_menu = 0;
      adminThread.enabled = true;
      menuThread.enabled = false;
    }
    
    if (adminThread.shouldRun()){
      adminThread.run();
    }
    
    if (selected_option) {

      switch (index_menu_admin) {
        case 0:
          // Temperatura y humedad
          lcd.setCursor(0,0);
          lcd.print("Temp: ");
          lcd.print(temphum[0]);
          lcd.print((char)223);
          lcd.print("C           ");
    
          lcd.setCursor(0,1);
          lcd.print("Hum: ");
          lcd.print(temphum[1]);
          lcd.print("%             ");
          break;
        case 1:
          // Distancia sensor
          lcd.setCursor(0,0);
          lcd.print("Distancia:       ");
          
          lcd.setCursor(0,1);
          lcd.print(distanceThread->dist);
          lcd.print(" cm           ");
          break;
        case 2:
          // Contador de tiempo
          lcd.setCursor(0,0);
          lcd.print("Tiempo:          ");
          
          lcd.setCursor(0,1);
          lcd.print(millis()/1000);
          lcd.print(" seg            ");
          break;
        case 3:
          // Menú de modificacion de precios
          if (!modifying_menu) {
            last_mvmnt = NONE;
            index_menu = 0;
            current_menu = menu;
            init_menu = true;
            modifying_menu = true;
            change_price = true;
            adminThread.enabled = false;
            menuThread.enabled = true;
          }
          
          if(menuThread.shouldRun()){
            menuThread.run();
          }
          
          break;
        default: break;
      }
      
    }
  }
  else {
    if (!init_admin) {
      digitalWrite(pinredLED, LOW);
      digitalWrite(pingreenLED,LOW);
      current_menu = menu;
      index_menu = 0;
      index_menu_admin = 0;
      init_admin = true;
      init_menu = true;
      change_price = false;
      adminThread.enabled = false;
      menuThread.enabled = true;
    }
  }
}
