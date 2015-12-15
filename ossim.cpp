#include "ossim.h"

ossim::PCB::PCB(){} // default constructor
ossim::PCB::PCB(int processNumber)
{
    pid = processNumber;
    burst = 0;
    state = Enter;
    operations = new list<meta>;
}
ossim::meta::meta(){} // default constructor
ossim::meta::meta(const meta& data)
{
    component = data.component;
    dcp = data.dcp;
    cycleTime = data.cycleTime;
}
bool ossim::init(const char * filename)
{
    // declarations/initializations
    ifstream fin(filename);
    string name, value;
    bool endConfiguration = false;
    bool error = false;

    current = NULL;
    pfout = NULL;
    pcout = NULL;

    // chekc if it exist
    if(!fin.good())
    {
        cout<<"Could not open the configuration file" << endl;
        fin.close();
        return false;
    }

    // input file data
    getline(fin, value);
    trimWhiteSpaces(value);
    if(value != "Start Simulator Configuration File")
    {
        cout << "Configuration must start with \"Start Simulator Configuration File\"" << endl;
        endConfiguration = true;
        error = true;
    }
    
    // start settings
    while(!endConfiguration)
    {
        getline(fin, name, ':');
        getline(fin, value);
        trimWhiteSpaces(name);
        trimWhiteSpaces(value);
     
        if(name == "Version/Phase")
        {
            settings.version = value;
        }
        else if(name == "File Path")
        {
            settings.filePath = value;
        }
        else if(name == "CPU Scheduling")
        {
            if(value == "RR" || value == "FIFO-P" || value == "SRTF-P")
            {
                settings.scheduling = value;
            }
            else
            {
                cout << "Unknown CPU Scheduling Algorithm: " << value << endl;
                cout << "Usage: RR or FIFO-P or SRTF-P" << endl;
                endConfiguration = true;
                error = true;
            }
        }
        else if(name == "Quantum time (cycles)")
        {
            settings.quantum = atoi(value.c_str());
        }
        else if(name == "Processor cycle time (msec)")
        {
            settings.CPUcycleTime = atoi(value.c_str());
        }
        else if(name == "Monitor display time (msec)")
        {
            settings.monitorDisplayTime = atoi(value.c_str());
        }
        else if(name == "Hard drive cycle time (msec)")
        {
            settings.hardDriveCycleTime = atoi(value.c_str());
        }
        else if(name == "Printer cycle time (msec)")
        {
            settings.printerCycleTime = atoi(value.c_str());
        }
        else if(name == "Keyboard cycle time (msec)")
        {
            settings.keyboardCycleTime = atoi(value.c_str());
        }
        else if(name == "Log")
        {
            if(value == "Log to Both" || value == "Log to Monitor" || value == "Log to File")
            {
                settings.logType = value;
            }
            else
            {
                cout << "Unknown Log Type: " << value << endl;
                cout << "Usage: Log to Both, Log to Monitor, or Log to File" << endl;
                endConfiguration = true;
                error = true;
            }
        }
        else if(name =="Log File Path")
        {
            settings.logPath = value;
        }
        else if(name =="End Simulator Configuration File")
        {
            endConfiguration = true;
        }
        else if(!fin.good())
        {
            cout << "Couldn't end configuration loading correctly" << endl;
            cout << "It must end with \"End Simulator Configuration File\"" << endl;
            endConfiguration = true;
            error = true;
        }
        else
        {
            cout << "Unknown Configuration Item: " << name << endl;
            endConfiguration = true;
            error = true;
        }
    }
    fin.close();

    // if there is an error, return false
    if(error)
    {
        return false;
    }

    // if load meta code does not succeed, return false
    if (!loadMetaCodes(settings.filePath.c_str()))
    {
        return false;
    }

    return true;
}
bool ossim::loadMetaCodes(const char * path)
{
    // declarations/initializations
    ifstream fin(path);
    string value;
    meta metaData;
    PCB *pcb;
    char metaStr[50];
    bool error = false;
    bool endLoadMeta = false;
    bool endProcess = false;
    int processCounter = 0;

    if(!fin.good())
    {
        cout<<"Error: could not open the program file!" << endl;
        fin.close();
        return false;
    }
    // read processes
    getline(fin, value);
    if(value != "Start Program Meta-Data Code:")
    {
        cout<<"Meta Data File must start with Start Program Meta-Data Code:" << endl;
        endLoadMeta = true;
    }

    // S - Start
    fin.getline(metaStr,50,';');
    error = !loadMeta(metaStr,metaData);
    if(metaData.component != 'S' || metaData.dcp != Start)
    {
        cout<<"First Meta Data must be S(start)0" << endl;
        endLoadMeta = true;
    }

    // load processes
    while(!endLoadMeta)
    {   
        fin >> ws;
        fin.getline(metaStr,50,';');
        endLoadMeta = !loadMeta(metaStr,metaData);

        // Load a single process
        if(metaData.component == 'A' && metaData.dcp == Start)
        {
            processCounter++;
            endProcess = false;

            pcb = new PCB(processCounter);
            pcb->operations->push_back(metaData);
            pcb->burst += getBurstTime(&metaData);

            while(!endProcess)
            {
                fin >> ws;
                fin.getline(metaStr,50,';');

                error = !loadMeta(metaStr,metaData);
                if(error)
                {
                    endProcess = true;
                    endLoadMeta = true;
                }
                else
                {
                    pcb->operations->push_back(metaData);
                    pcb->burst+= getBurstTime(&metaData);

                    if(metaData.component == 'A' && metaData.dcp == End)
                    {
                        endProcess = true;
                    }
                }
                pcb->state = Ready;
            }
            ready.push_back(*pcb);
            delete pcb;

        }
       // S - end
       else if(metaData.component == 'S' && metaData.dcp == End)
       {
           endLoadMeta = true;
       }
       else
       {
           cout<<"Last Meta Data must be S(end)0" << endl;
           error = true;
       }
    }

    // make an array of ioThreadData
    tData = new ioThreadData[ready.size()];

    // exit load
    fin.close();

    if(error)
    {
        return false;
    }
    return true;
}
int ossim::getBurstTime(meta *data)
{
    int cycleTime = 0;

    if(data->component == 'S' || data->component == 'A' || data->component == 'P')
    {
        cycleTime = settings.CPUcycleTime;
    }

    else if(data->component == 'I' || data->component == 'O')
    {
        if(data->dcp == HardDrive)
        {
            cycleTime = settings.hardDriveCycleTime;
        }
        else if(data->dcp == Keyboard)
        {
            cycleTime = settings.keyboardCycleTime;
        }
        else if(data->dcp == Monitor)
        {
            cycleTime = settings.monitorDisplayTime;
        }
    }

    return cycleTime*data->cycleTime;
}
bool ossim::loadMeta(char* input, meta &data)
{
    char *pStr = NULL;
    bool error = false;

    pStr = strtok(input, "(" );

    // component
    if(*pStr == 'S' || *pStr == 'A' || *pStr == 'P' || *pStr == 'I' || *pStr == 'O')
    {
        data.component = *pStr;
    }
    else
    {
       cout << "Unknown Meta-data component: " << pStr << endl;
       error = true;
    }

    // descriptor
    pStr = strtok (NULL, ")");

    if(data.component == 'S')
    {
        if (!strcmp(pStr,"end"))
        {
            data.dcp = End;
        }
        else if (!strcmp(pStr,"start"))
        {
            data.dcp = Start;
        }
        else
        {
            cout << "Unknown Meta-data descriptors for OS: " << pStr << endl;
            error = true;
        }
    }
    else if(data.component == 'A')
    {
        if (!strcmp(pStr,"end"))
        {
            data.dcp = End;
        }
        else if (!strcmp(pStr,"start"))
        {
            data.dcp = Start;
        }
        else
        {
            cout << "Unknown Meta-data descriptors for Application: " << pStr << endl;
            error = true;
        }
    }
    else if(data.component == 'P')
    {
        if (!strcmp(pStr,"run"))
        {
            data.dcp = Run;
        }
        else
        {
            cout << "Unknown Meta-data descriptors for Process: " << pStr << endl;
            error = true;
        }
    }
    else if(data.component == 'I')
    {
        if (!strcmp(pStr,"keyboard"))
        {
            data.dcp = Keyboard;
        }
        else if (!strcmp(pStr,"hard drive"))
        {
            data.dcp = HardDrive;
        }
        else
        {
            cout << "Unknown Meta-data descriptors for Input operation: " << pStr << endl;
            error = true;
        }
    }
    else if(data.component == 'O')
    {
        if (!strcmp(pStr,"monitor"))
        {
            data.dcp = Monitor;
        }
        else if (!strcmp(pStr, "printer"))
        {
            data.dcp = Printer;
        }
        else if (!strcmp(pStr,"hard drive"))
        {
            data.dcp = HardDrive;
        }
        else
        {
            cout << "Unknown Meta-data descriptors for Output operation: " << pStr << endl;
            error = true;
        }
    }

    if(error)
    {
        cout
        << "Meta-Data Format:"
        << "\nS – operating System, used with start and end"
        << "\nA – Program Application, used with start and end"
        << "\nP – Process, used with run"
        << "\nI – used with Input operation descriptors - hard drive, keyboard"
        << "\nO – used with Output operation descriptors - hard drive, monitor"
        << endl; 
        return false;
    }

    // cycle time
    pStr = strtok (NULL, ".");
    if(pStr!=NULL)
    {
        data.cycleTime = atoi(pStr);
    }
    else
    {
        return false;
    }

    return true;
}
void ossim::start()
{
    // declarations/initializations
    ofstream fout;
    void (ossim::*scheduler)() = NULL;
    processNum = 1;
    stringstream ss;
    ss << fixed;

    // log setting
    if(settings.logType == "Log to Both")
    {
        fout.open(settings.logPath.c_str());
        pfout = &fout;   
        pcout = &cout;  
    }
    else if(settings.logType == "Log to File")
    {
        fout.open(settings.logPath.c_str());
        pfout = &fout;  
    }
    else if(settings.logType == "Log to Monitor")
    {
        pcout = &cout; 
    }

    // scheduler setting
    if(settings.scheduling == "RR")
    {
        scheduler = &ossim::RR;
    }
    if(settings.scheduling == "FIFO-P")
    {
        scheduler = &ossim::FIFO_P;
    }
    else if(settings.scheduling == "SRTF-P")
    {
        scheduler = &ossim::SRTF_P;
    }

    // start simulation
    ss << 0.0 << " - Simulator program starting" << endl;
    log(ss, pcout , pfout);
    mainTimer.start();

    // schedules processes
    mainLoop(scheduler);

    // end simulation
    ss << elapsed() << " - Simulator program ending" << endl;
    log(ss, pcout , pfout);

    if(pfout!=NULL)
    {
        fout.close();
    }
    delete[] tData;
}
MetaEndType ossim::runMeta()
{
    // declarations/initializations
    MetaEndType end_type = EndMeta;
    stringstream ss;
    pthread_t tid;
    bool ioInt = false;
    double waitTime;

    ss << fixed;

    // run a single operation for a given metaData
    if(current->component == 'A')
    {
        if(current->dcp == Start)
        {
            ss << elapsed() << " - OS: preparing process " << processNum << endl;
            log(ss, pcout , pfout);

            ss << elapsed() << " - OS: starting process " << processNum << endl;
            log(ss, pcout , pfout);
        }
        else if(current->dcp == End)
        {
            ss << elapsed() << " - OS: ending process " << processNum << endl;
            log(ss, pcout , pfout);

            ss << elapsed() << " - OS: removing process " << processNum << endl;
            log(ss, pcout , pfout);

            end_type = EndProcess;
        }
    }
    else if(current->component == 'P')
    {
        if(current->dcp == Run)
        {
            ss << elapsed() << " - Process "<< processNum <<": start processing action" << endl;
            log(ss, pcout , pfout);

            ioInt = !IOInterrupts.empty();

            // run process one cycle at a time while watching interruption
            while(ioInt == false && quantum > 0 && current->cycleTime > 0)
            {           
                waitTime = double(settings.CPUcycleTime)/1000;
                wait(waitTime);
                quantum--;
                current->cycleTime--;
                currentPcb.burst-= waitTime;
                ioInt = !IOInterrupts.empty();
            }
            // set up return type

            if(current->cycleTime <= 0)
            {
                ss << elapsed() << " - Process "<< processNum <<": end processing action" << endl;
                log(ss, pcout , pfout);
                end_type = EndMeta;
            }
            if(quantum <= 0)
            {
                ss << elapsed() << " - OS: quantum time interrpt" << endl;
                log(ss, pcout , pfout);
                end_type = TimeSlice;
            }
            if(ioInt == true)
            {
                end_type = IOEnd;
            }
        }
    }
    else if(current->component == 'I' || current->component == 'O')
    {
        // Make the process waiting
        setUpIO();

        ss << elapsed() << " - OS: Process " << processNum << " to Blocked" << endl;
        log(ss, pcout , pfout);

        pthread_create(&tid, NULL, &IO_helper, this);
        end_type = IOStart;
        quantum = 0;
    }

    if(end_type == EndMeta || end_type == EndProcess || current->cycleTime <= 0)
    {
        currentPcb.operations->pop_front();
    }

    return end_type;
}
void ossim::RR()
{
    // Assum that the ready queue is correctly ordered
    // pop put the top one
    currentPcb = ready.front();
    ready.pop_front();
}
void ossim::putIOFinishProcessReady()
{
    int pid = IOInterrupts.front();
    bool found = false;
    list<PCB>::iterator iter;
    stringstream ss;

    ss << fixed;
    IOInterrupts.pop();
    iter = blocked.begin();

    while(!found)
    {
        if(iter->pid == pid)
        {
            iter->operations->pop_front();
            ss << elapsed() << " - OS: Process " << pid << " to Ready" << endl;
            log(ss, pcout , pfout);
            iter->state = Ready;

            ready.push_back(*iter);

            blocked.erase(iter);
            found = true;
        }
        ++iter;
    }
}
void ossim::SRTF_P()
{
    list<PCB>::iterator itr;
    list<PCB>::iterator shortestJob;

    // find the shortest remaining job
    shortestJob = ready.begin();
    for(itr = ready.begin(); itr!=ready.end(); ++itr)
    {
        if(itr->burst < shortestJob->burst)
        {
            shortestJob = itr;
        }
    }

    currentPcb = *shortestJob;
    ready.erase(shortestJob);
}
void ossim::FIFO_P()
{
    list<PCB>::iterator itr;
    list<PCB>::iterator highestPriorityJob;

    // find the highestPriorityJob
    highestPriorityJob = ready.begin();
    for(itr = ready.begin(); itr!=ready.end(); ++itr)
    {
        if(itr->pid < highestPriorityJob->pid)
        {
            highestPriorityJob = itr;
        }
    }

    currentPcb = *highestPriorityJob;
    ready.erase(highestPriorityJob);
}
void ossim::mainLoop(void (ossim::*scheduler)())
{
    // declarations/initializations
    MetaEndType endType;
    stringstream ss;
    ss << fixed;

    // simulate FIFO-P
    while(!ready.empty())
    {
        // reset quantum
        quantum = settings.quantum;

        ss << elapsed() << " - OS: selecting next process" << endl;
        log(ss, pcout , pfout);

       // schedule next process
       (this->*scheduler)();
       processNum = currentPcb.pid;
       currentPcb.state = Running;

        // run the process
        while(quantum)
        {
            current = &currentPcb.operations->front();
            endType = runMeta();


            if(endType == EndProcess)
            {
                delete currentPcb.operations;
                currentPcb.state = Exit;
                quantum = 0;
            }
            // put the process back into ready
            if(endType == TimeSlice || endType == IOEnd)
            {
                ss << elapsed() << " - OS: Process " << currentPcb.pid << " to Ready" << endl;
                log(ss, pcout , pfout);
                currentPcb.state = Ready;

                // by defualt, processes are pushed back into ready 
                // in round-robin fashion. When the process has finished
                // IO operation and interrupt another, it will go first 
                // before the interrupted one is pushed back
                if(endType == IOEnd)
                {
                    putIOFinishProcessReady();
                    quantum = 0;
                }
                ready.push_back(currentPcb);
            }
        }

        // if no ready process but a blocked process exist, 
        // wait until interruption
        if(ready.empty())
        {
            if(!blocked.empty())
            {
                while(IOInterrupts.empty());
                putIOFinishProcessReady();
            }
        }
    }
}
void ossim::setUpIO()
{
    // declarations/initializations
    int cycleTime = 0;
    int cycleIO = current->cycleTime;
    stringstream ss;
    ss << fixed;

    // bring the process into waiting
    currentPcb.state = Waiting;
    blocked.push_front(currentPcb);
    IOBounds.push(processNum);

    // set up IO thread data
    tData[processNum-1].pid = processNum;

    if(current->component =='O')
    {
        tData[processNum-1].ioType =  "output";
    }
    else if(current->component == 'I')
    {
        tData[processNum-1].ioType =  "input";
    }

    if(current->dcp == HardDrive)
    {
        cycleTime = settings.hardDriveCycleTime;
        tData[processNum-1].device = "hard drive";
    }
    else if(current->dcp == Keyboard)
    {
        cycleTime = settings.keyboardCycleTime;
        tData[processNum-1].device = "keyboard";
    }
    else if(current->dcp == Monitor)
    {
        cycleTime = settings.monitorDisplayTime;
        tData[processNum-1].device = "monitor";
    }
    else if(current->dcp == Printer)
    {
        cycleTime = settings.printerCycleTime;
        tData[processNum-1].device = "printer";
    }

    tData[processNum-1].waitTime = cycleIO*double(cycleTime)/1000;

    ss << elapsed() << " - Process " << tData[processNum-1].pid << ": start " 
    << tData[processNum-1].device << " " << tData[processNum-1].ioType << endl;
    log(ss, pcout , pfout);

    ss << elapsed() << " - OS: starting " << tData[processNum-1].device << " " 
    << tData[processNum-1].ioType << endl;
    log(ss, pcout , pfout);
}
void *ossim::IO_helper(void * param){
    return ((ossim *)param)->IO_Operation();
}
void *ossim::IO_Operation()
{
    // declarations/initializations
    int index = IOBounds.front() - 1;
    IOBounds.pop();

    stringstream ss;
    ss << fixed;

    // process IO operation
    wait(tData[index].waitTime);

    ss << elapsed() << " - Process " << tData[index].pid << ": end " 
    << tData[index].device << " " << tData[index].ioType << endl;
    log(ss, pcout , pfout);

    // interruption
    IOInterrupts.push(tData[index].pid);

    pthread_exit(0);
}
void ossim::log(stringstream &buffer, ostream* pCout, ofstream * pFout)
 {
    if(pCout!=NULL)
    {
        *pCout << buffer.str();
    }
    if(pFout!=NULL)
    {
        *pFout << buffer.str();
    }
    buffer.str("");
 }
void ossim::wait(double num){
    Timer timer;
    double duration = 0;

    timer.start();
    do{
        timer.stop();
        duration = timer.getElapsedTime();
    }while(duration < num);
 }
double ossim::elapsed(){
    mainTimer.stop();
    return mainTimer.getElapsedTime();
}
void ossim::trimWhiteSpaces(string &str)
{
    size_t first, end, range; 
    first = str.find_first_not_of(" \t\n");
    end = str.find_last_not_of(" \t\n");
    if( first != string::npos && end != string::npos)
    {
        range = end - first + 1;
        str = str.substr(first,range);
    } 
}
void ossim::config()
{
    // display configuration for debugging
    cout
         << "Configuration --------------------------------------------" 
         << "\nVersion/Phase                    : " << settings.version
         << "\nFile Path                        : " << settings.filePath 
         << "\nCPU Scheduling                   : " << settings.scheduling
         << "\nProcessor cycle time (ms/cycle)  : " << settings.CPUcycleTime
         << "\nQuantum time (cycles)            : " << settings.quantum
         << "\nMonitor display time (ms/cycle)  : " << settings.monitorDisplayTime 
         << "\nHard drive cycle time (ms/cycle) : " << settings.hardDriveCycleTime
         << "\nPrinter cycle time (ms/cycle)    : " << settings.printerCycleTime 
         << "\nKeyboard cycle time (ms/cycle)   : " << settings.keyboardCycleTime
         << "\nLog                              : " << settings.logType
         << "\nLog File Path                    : " << settings.logPath
         << "\n---------------------------------------------------------" 
         << endl;
}
