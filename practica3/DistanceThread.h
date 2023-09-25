class DistanceThread: public Thread {
public:
  int pinTrigger;
  int pinEcho;
  long dist;
  long t;

  DistanceThread(int _pin1, int _pin2): Thread() {

    pinTrigger = _pin1;
    pinEcho = _pin2;
    dist = 101;
    pinMode(pinTrigger, OUTPUT);
    pinMode(pinEcho, INPUT);
    digitalWrite(pinTrigger, LOW);
  }

  bool shouldRun(unsigned long time) {

    return Thread::shouldRun(time);
  }

  void run() {

    Thread::run();

    digitalWrite(pinTrigger, HIGH);
    delayMicroseconds(10);  // Enviamos un pulso de 10us
    digitalWrite(pinTrigger, LOW);
  
    t = pulseIn(pinEcho, HIGH); // Obtenemos el ancho del pulso
  
    dist = t/59; // Escalamos el tiempo a una distancia en cm
  }

  bool detected_person() {

    return dist <= 100;
  }
};
