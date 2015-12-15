#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <sys/time.h> 
#include <stdexcept>
#include <iostream>

using namespace std;


class Timer {
  public:
    Timer();
    void start() throw (runtime_error);
    void stop() throw (logic_error);
    double getElapsedTime() const throw (logic_error);

  private:
    timeval beginTime;
    double duration;
    bool timerWasStarted;
};

#endif	// ifndef TIMER_H
