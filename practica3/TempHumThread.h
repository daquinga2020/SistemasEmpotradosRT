class MenuThread: public Thread {
public:
  int index;
  int last_mvmnt = NONE;

  String *option_menu;
  String menu[] = {"i. Cafe Solo 1 e","ii. Cafe Cortado 1.10 e","iii. Cafe Doble 1.25 e","iv. Cafe Premium 1.50 e","v. Chocolate 2.00 e"};

  Menu(): Thread() {
    
  }

  bool shouldRun(unsigned long time){
    return Thread::shouldRun(time);
  }

  void run(){
    Thread::run();
  }

};
