# ossim
OS Simulator with multi-programming as CS course project

## Description
This program is a multiprogramming OS simulator. 
The machine takes one of the five states (Enter, Ready, Waiting, Running, or Exit). 
Given a meta-data file for some programs (i.e., potentially unlimited number), 
it run the programs concurrently using a multi-programming strategy, and then ends the simulation.

## Specification
### Meta file
Program meta-data components:
* S – operating System, used with start and end
* A – Program Application, used with start and end
* P – Process, used with run
* I – used with Input operation descriptors such as hard drive, keyboard 
* O – used with Output operation descriptors such as hard drive, monitor

Program meta-data descriptors:
* end, hard drive, keyboard, monitor, run, start

Meta-data operation:
* \<component letter\>(\<operation\>)\<cycle time\>; \<successive meta-data operation\>; . . . \<last operation\>.

### Configuration file
* Start Simulator Configuration File
* Version/Phase: <number>
* File Path: \<complete file path and name of program file\>
* CPU Scheduling: \<scheduling code\>
* Quantum time (cycles): \<number of cycles for quantum\> 
* Processor cycle time (msec/cycle): \<time\>
* Monitor display time (msec/cycle): \<time\>
* Hard drive cycle time (msec/cycle): \<time\>
* Printer cycle time (msec/cycle): \<time\>
* Keyboard cycle time (msec/cycle): \<time\>
* Log: \<Log to Both\> OR \<Log to Monitor\> OR \<Log to File\> 
* Log File Path: \<complete file path and name of log file name\> 
* End Simulator Configuration File

#### \<Scheduling code\>
* \<RR\>: the Round-Robin fashion with specified Quantum time
* \<FIFO-P\>: the First In – First Out with pre-emption
* \<SRTF-P\>: the Shortest Remaining Time First – Preemptive

## Usage
1. Generate a meta file
2. Make a configuration file
3. Compile and execute
```
> make
> ./ossim config
```

## Testcase
[![asciicast](https://asciinema.org/a/e0uincumck1oovv3952vjtbpg.png)](https://asciinema.org/a/e0uincumck1oovv3952vjtbpg)

## Licence
* MIT License

## Author
* Yuta Miyake
