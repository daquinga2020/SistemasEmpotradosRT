#include <Thread.h>
#include <StaticThreadController.h>
#include <ThreadController.h>
#include "InfraredThread.h"
#include "DistanceThread.h"

// Pines del ultrasonido
#define TRIG_PIN 13  
#define ECHO_PIN 12 

// Pines del sensor infrarrojo
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6

#define MAX_VEL_R 70
#define MAX_VEL_L 70

ThreadController controller = ThreadController();
DistanceThread* distanceThread = new DistanceThread(TRIG_PIN,ECHO_PIN);
InfraredThread* infraredThread = new InfraredThread(PIN_ITR20001_LEFT,PIN_ITR20001_MIDDLE,PIN_ITR20001_RIGHT);
Thread motorsThread = Thread();

void callback_motors() {

  /*if (distanceThread->detected_obs) {
    // Motor se detiene, hay un obstaculo delante
  }*/
  bool line_center = false;
  bool line_right = false;
  bool line_left = false;
  int diff;
  int kp;
  int vel_r;
  int vel_l;
  // Para saber si detecta la linea
  if (infraredThread->read_middle >= 50) {
    line_center = true;
    vel_r = MAX_VEL_R;
    vel_l = MAX_VEL_L;
    if (infraredThread->read_right >= 180) {
      vel_l = MAX_VEL_L; // minimo
      vel_r = MAX_VEL_R-15;
      line_right = true;
    } else {
      line_right = false;
    }
    if (infraredThread->read_left >= 100) {
      line_left = true;
      vel_l = MAX_VEL_L-15; // minimo
      vel_r = MAX_VEL_R;
      vel_r = 60;
      vel_l = 20;
    } else {
      line_left = false;    
    }
  } else {
    line_center = false;
    //vel_r = 0;
    //vel_l = 0;
    if (infraredThread->read_right >= 180) {
      vel_l = 60; // minimo
      vel_r = 0;
      line_right = true;
    } else {
      line_right = false;
    }
    if (infraredThread->read_left >= 100) {
      line_left = true;
      vel_r = 60;
      vel_l = 0;
    } else {
      line_left = false;    
    }
  }
  /*
  // Suponiendo que habian 1024 valores, comprobar y ajustar minimos y maximos
  if (line_center || line_right || line_left) {
    // Motor avanza,
    // si la diferencia entre derecha e izquierda es menor que 10, velocidades distintas
    // sino velocidades
    Serial.println("LINEA DETECTADA");
    
    diff = abs(infraredThread->read_left - infraredThread->read_right);
    if (diff < 15) {
      // esta en el centro
      Serial.println("LINEA EN EL CENTRO");
      vel_r = MAX_VEL_R;
      vel_l = MAX_VEL_L;
    }else {
      if (line_right){
        Serial.println("GIRO DERECHA");
        vel_l = MAX_VEL_L;
        vel_r = infraredThread->read_left/1000 * MAX_VEL_R;
      } else if (line_left) {
        Serial.println("GIRO IZQUIERDA");
        vel_l = infraredThread->read_right/1000 * MAX_VEL_L;
        vel_r = MAX_VEL_R;
      }
    }
    Serial.print("VEL_R:");
    Serial.println(vel_r);
    Serial.print("VEL_L:");
    Serial.println(vel_l);
    Serial.println();
    Serial.println();
  }*/
  analogWrite(PIN_Motor_PWMA, vel_r); // Derecho
  analogWrite(PIN_Motor_PWMB, vel_l); // Izquierdo
}

void setup() {

  /*pinMode(PIN_Motor_STBY, OUTPUT);

  pinMode(PIN_Motor_AIN_1, INPUT);
  pinMode(PIN_Motor_PWMA, INPUT);
  
  pinMode(PIN_Motor_BIN_1, INPUT);
  pinMode(PIN_Motor_PWMB, INPUT);*/

  motorsThread.enabled = true;
  motorsThread.setInterval(50);
  motorsThread.onRun(callback_motors);
  digitalWrite(PIN_Motor_STBY, 1);
  digitalWrite(PIN_Motor_AIN_1,1);
  digitalWrite(PIN_Motor_BIN_1,1);
  Serial.begin(9600);

  // Arranque medidor de distancia
  distanceThread->setInterval(75);
  controller.add(distanceThread);

  infraredThread->setInterval(50);
  controller.add(infraredThread);

}

void loop() {
  
  controller.run();
  if (motorsThread.shouldRun()) {
    motorsThread.run();
  }
  Serial.print("IZQUIERDA:");
  Serial.println(infraredThread->read_left);
  Serial.print("CENTRO:");
  Serial.println(infraredThread->read_middle);
  Serial.print("DERECHA:");
  Serial.println(infraredThread->read_right);
  Serial.println();

  //delay(1000);
}
