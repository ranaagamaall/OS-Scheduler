#include "headers.h"

/* Modify this file as needed*/
int remainingTime;
int startclk;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    remainingTime = atoi(argv[1]);
    printf("I am the proccess file & The running time is %d\n", remainingTime);
    
    startclk=getClk();
    while (getClk() < startclk + remainingTime);
    destroyClk(false);

    return 0;
}
