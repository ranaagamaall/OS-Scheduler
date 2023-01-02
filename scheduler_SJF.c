#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue ReadyQueue;
int pCount = 0;
int pfinished =0;
process *CurrentProcess = NULL;
process data;
int msgid;
FILE *fptr; //For the .log file
int TA;
float WTA;
FILE *perfptr; //For the .perf file
float utilization;
int sumRuntime=0;
int sumWaitingtime=0;
float sumWTA;
int Lfinish;


int main(int argc, char *argv[])
{
    initClk();
    ReadyQueue=createQueue();
    signal(SIGCHLD, childHandler);
    signal(SIGINT, schedulerHandler);

    key_t key = ftok("./clk.c", 'Z');
    msgid = msgget(key, IPC_CREAT | 0666);
    struct msgbuffer msg;
    if (msgid == -1)
    {
        perror("Error in msgget");
        return 1;
    }

    pCount = atoi(argv[1]);
    int rec_val;
    

    fptr = fopen("Scheduler_SJF.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    printf("#At time x process y state arr w total z remain y wait k \n");

    while(pCount!=pfinished)
    {
        rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT); // shouldn't wait for msg
        if (rec_val != -1)
        {
            enqueue(&ReadyQueue, msg.proc);
        }

        //execution every second
    
        int pid, status;

        if (CurrentProcess == NULL && isEmpty_Queue(&ReadyQueue) == 0)
        {
            data = peek_Queue(&ReadyQueue);
            CurrentProcess = &data;
            CurrentProcess->startTime = getClk();
            CurrentProcess->state = RUNNING;
            dequeue(&ReadyQueue);
            
            CurrentProcess->waitingTime = getClk() - CurrentProcess->arrivalTime;
            pid = fork();
            if (pid == 0)
            {
                char buffer[20];
                sprintf(buffer, "%d", CurrentProcess->runTime);
                char *argv[] = {"./process.out", buffer, NULL, 0};
                execve(argv[0], &argv[0], NULL);
            }
            else
            {
                printf("At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime);
            }
        }
    }

    utilization=((float)sumRuntime/(float)Lfinish)*100;
    perfptr = fopen("Scheduler_SJF.perf", "w");
    fprintf(perfptr,"CPU utilization = %0.2f%%Avg\n",utilization);
    fprintf(perfptr,"WTA=%.2f\n",(float)sumWTA/(float)pCount);
    fprintf(perfptr,"Average waiting=%.2f\n",(float)sumWaitingtime/(float)pCount);

    destroyClk(true);
    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
}


void childHandler(int signum)
{
    CurrentProcess->finishTime = getClk();
    //For the .log file 
    TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
    WTA = (float)TA / (float)CurrentProcess->runTime;
    fprintf(fptr,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", CurrentProcess->finishTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime, TA, WTA);
    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", CurrentProcess->finishTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime, TA, WTA);
    CurrentProcess->state = FINISHED;
    pfinished++;

    //For the .perf file 
    sumRuntime+=CurrentProcess->runTime;
    sumWaitingtime+=CurrentProcess->waitingTime;
    sumWTA+=WTA;
    Lfinish=CurrentProcess->finishTime;
    
    
    CurrentProcess = NULL;
    signal(SIGCHLD, childHandler);
}

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    msgctl(msgid, IPC_RMID, NULL);
    fclose(perfptr);
    fclose(fptr);
    destroyClk(true);
    exit(0);
}