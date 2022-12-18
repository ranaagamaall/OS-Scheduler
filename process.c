#include "headers.h"

/* Modify this file as needed*/
int remainingTime;
int startclk;
int sleep_time = 0;
int runTime;
int wait_time = 0; 

// Overwriting the SIGTSTP
void stopProcess(int signum){
    sleep_time = getClk();
    raise(SIGSTOP);
    signal(SIGTSTP,stopProcess);
}

// Overwriting the SIGCONT
void resumeProcess(int signum){
    wait_time += getClk() - sleep_time;
    signal(SIGCONT,resumeProcess);
}

int main(int agrc, char *argv[])
{
    initClk();
    
    signal(SIGTSTP,stopProcess);
    signal(SIGCONT,resumeProcess);
    runTime = atoi(argv[1]);
    remainingTime = runTime;
    
    startclk=getClk();
    while (remainingTime > 0)
    {
        remainingTime = runTime - (getClk() - startclk - wait_time);
    }
    //destroyClk(false);

    return 0;
}
