#ifndef __OSSIM__H__
#define __OSSIM__H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <list>
#include <ctime>    
#include <pthread.h> 
#include <cstdlib>
#include <queue>
#include "Timer.h"

using namespace std;

namespace enums{
    enum State{Enter, Ready, Waiting, Running, Exit};
    enum Descriptor{End, HardDrive, Keyboard, Monitor, Printer, Run, Start};
    enum MetaEndType{EndProcess, EndMeta, TimeSlice, IOStart, IOEnd};
}
using namespace enums;

class ossim{
public:
    bool init(const char * filename);
    void config();
    void start();

private:
    struct meta{
        meta();
        meta(const meta& data);
        char component;
        Descriptor dcp;
        int cycleTime;
    };
    struct PCB{
        PCB();
        PCB(int processNumber);
        int pid;
        int burst;
        State state;
        list<meta> *operations;
    };
    struct configuration{
        string version;
        string filePath;
        string logPath;
        string scheduling;
        string logType;
        int quantum;
        int CPUcycleTime;
        int monitorDisplayTime;
        int hardDriveCycleTime;
        int printerCycleTime;
        int keyboardCycleTime;
    };
    struct ioThreadData
    {
        string device;
        string ioType;
        double waitTime;
        int pid;
    };
    void mainLoop(void (ossim::*scheduler)());
    void FIFO_P();
    void SRTF_P();
    void RR();
    void putIOFinishProcessReady();
    int getBurstTime(meta *data);
    bool loadMetaCodes(const char * path);
    bool loadMeta(char* input, meta &data);
    MetaEndType runMeta();
    void log(stringstream &buffer, ostream* pCout, ofstream * pFout);
    static void * IO_helper(void * param);
    void *IO_Operation();
    void setUpIO();
    void wait(double num);
    double elapsed();
    void trimWhiteSpaces(string &str);

    configuration settings;
    meta * current;
    int processNum;
    PCB currentPcb;
    Timer mainTimer;
    int quantum;
    ofstream *pfout;
    ostream *pcout;
    ioThreadData *tData;
    list<PCB> ready;
    list<PCB> blocked;
    queue<int> IOBounds;
    queue<int> IOInterrupts;

};

#endif