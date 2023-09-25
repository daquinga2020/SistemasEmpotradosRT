class InfraredThread: public Thread {
public:
  int pinLEFT;
  int pinMIDDLE;
  int pinRIGHT;

  int read_left;
  int read_middle;
  int read_right;

  InfraredThread(int _pin1, int _pin2, int _pin3): Thread() {

    pinLEFT = _pin1;
    pinMIDDLE = _pin2;
    pinRIGHT = _pin3;

    pinMode(pinLEFT, INPUT);
    pinMode(pinMIDDLE, INPUT);
    pinMode(pinRIGHT, INPUT);
  }

  bool shouldRun(unsigned long time) {

    return Thread::shouldRun(time);
  }

  void run() {

    Thread::run();

    read_left = analogRead(pinLEFT);
    read_middle = analogRead(pinMIDDLE);
    read_right = analogRead(pinRIGHT);
  }
};
