#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue ReadyQueue;
int pCount = 0;
int quantum = 0;
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
int startclk;


int main(int argc, char *argv[])
{
    initClk();
    ReadyQueue=createQueue();
    // signal(SIGCHLD, childHandler);
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
    quantum = atoi(argv[2]);

    int time;
    time = getClk();
    int rec_val;

    fptr = fopen("Scheduler_RR.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    while(pCount!=pfinished)    
    {
        
        rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
        if(rec_val == -1 && isEmpty_Queue(&ReadyQueue) == 1)
        {
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT); // waits
        }
        if (rec_val != -1)
        {
            enqueue(&ReadyQueue, msg.proc);
        }

        int nextTime = getClk();

        //execution every second
        if(nextTime > time){ 
            time = getClk();

            //peek 
            if(isEmpty_Queue(&ReadyQueue) == 0){
                data = peek_Queue(&ReadyQueue);
                CurrentProcess = &data;
                dequeue(&ReadyQueue);
            }


            int pid, status;

            if (data.state == WAITING)  //never executed before ==> check status = waiting instead of null
            {
                char buffer1[5];
                sprintf(buffer1, "%d", CurrentProcess->runTime); // pass the remaining time
                CurrentProcess->startTime = getClk(); // start time of the process added to the node
                CurrentProcess->contextSwitchTime = getClk();
                int pid = fork();                     // fork the process
                CurrentProcess->state = RUNNING;
                if (pid == 0)
                {
                    char *ar[] = {"./process.out", buffer1, NULL, 0};
                    execve(ar[0], &ar[0], NULL);
                }
                else
                {
                    CurrentProcess->waitingTime = CurrentProcess->startTime - CurrentProcess->arrivalTime;
                    printf("At time %d process %d started arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                    fprintf(fptr,"At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                }
            
            }
            //if forked before ==> do not fork ==> signal continue
            else if(data.state == STOPPED){ 
                CurrentProcess->state = RUNNING;
                kill(CurrentProcess->PID, SIGCONT);
                printf("At time %d process %d continued arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d continued arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
            }

            //process execution
            if(CurrentProcess->remainingTime > quantum){
                CurrentProcess->remainingTime -= quantum;
                kill(CurrentProcess->PID, SIGTSTP);
                startclk=getClk();
                while(getClk()<startclk+quantum){}
                CurrentProcess->state = STOPPED;
                printf("At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                enqueue(&ReadyQueue, *CurrentProcess);
            }else if(CurrentProcess->remainingTime <= quantum){
                CurrentProcess->remainingTime =0;
                startclk=getClk();
                CurrentProcess->finishTime = getClk();
                while(getClk()<startclk+quantum){}
                CurrentProcess->state = FINISHED;
                printf("At time %d process %d finished arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
                WTA = (float)TA / (float)CurrentProcess->runTime;
                fprintf(fptr,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime,TA, WTA);
                pfinished++;

                sumRuntime+=CurrentProcess->runTime;
                sumWaitingtime+=CurrentProcess->waitingTime;
                sumWTA+=WTA;
                Lfinish=CurrentProcess->finishTime;
            }
        }
    } 

            

    utilization=((float)sumRuntime/(float)Lfinish)*100;
    perfptr = fopen("Scheduler_RR.perf", "w");
    fprintf(perfptr,"CPU utilization = %0.2f%%Avg\n",utilization);
    fprintf(perfptr,"WTA=%.2f\n",(float)sumWTA/(float)pCount);
    fprintf(perfptr,"Average waiting=%.2f\n",(float)sumWaitingtime/(float)pCount);

    destroyClk(true);
    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
    
}


// void childHandler(int signum)
// {
//     CurrentProcess->finishTime = getClk();
//     //For the .log file 
//     TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
//     WTA = (float)TA / (float)CurrentProcess->runTime;
//     fprintf(fptr,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", CurrentProcess->finishTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime, TA, WTA);
//     CurrentProcess->state = FINISHED;
//     pfinished++;

//     //For the .perf file 
//     sumRuntime+=CurrentProcess->runTime;
//     sumWaitingtime+=CurrentProcess->waitingTime;
//     sumWTA+=WTA;
//     Lfinish=CurrentProcess->finishTime;
    
    
//     CurrentProcess = NULL;
//     signal(SIGCHLD, childHandler);
// }

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    msgctl(msgid, IPC_RMID, NULL);
    fclose(perfptr);
    fclose(fptr);
    destroyClk(true);
    exit(0);
}