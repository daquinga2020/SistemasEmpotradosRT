class LedThread: public Thread {
public:
  int pin;
  int counter;
  bool state;

  LedThread(int _pin): Thread() {

    pin = _pin;
    state = true;
    counter = 0;
    pinMode(pin, OUTPUT);
  }

  bool shouldRun(unsigned long time) {
    return Thread::shouldRun(time);
  }

  void run() {

    Thread::run();
    
    digitalWrite(pin, state ? HIGH : LOW);
    state = !state;
    counter++;
  }
};