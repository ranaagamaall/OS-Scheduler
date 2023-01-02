#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue ReadyQueue;
struct memTree *MemoryTree;
int MemoryStart;
int pCount = 0;
int quantum = 0;
int pfinished =0;
process *CurrentProcess = NULL;
process data;
int msgid;
FILE *fptr; //For the .log file
FILE *memfptr;
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
    MemoryTree = create_memTree();
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
    memfptr = fopen("Memory_RR.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");
    printf("#At time x allocated y bytes for process z from i to j \n");


    while(pCount!=pfinished)    
    {
        do
        {
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if(rec_val == -1 && isEmpty_Queue(&ReadyQueue) == 1 && CurrentProcess == NULL)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT); // waits
            }
            if (rec_val != -1)
            {
                MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
                msg.proc.mem_start = MemoryStart;
                enqueue(&ReadyQueue, msg.proc);
                int total_size = pow(2, ceil(log2(msg.proc.memory)));
                fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
                printf("At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
            }
        } while (rec_val != -1);
        
        

        int nextTime = getClk();

        //execution every second
        if(nextTime > time)
        { 
            time = getClk();
            if(CurrentProcess != NULL && CurrentProcess->state == STOPPED)
                enqueue(&ReadyQueue, *CurrentProcess);
            
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
                CurrentProcess->waitingTime =  CurrentProcess->waitingTime + getClk() - CurrentProcess->contextSwitchTime;
                CurrentProcess->contextSwitchTime = getClk();
                CurrentProcess->state = RUNNING;
                kill(CurrentProcess->PID, SIGCONT);
                //printf("At time %d process %d continued arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d continued arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
            }

            //process execution
            if(CurrentProcess->remainingTime > quantum){
                CurrentProcess->remainingTime -= quantum;
                kill(CurrentProcess->PID, SIGTSTP);
                startclk=getClk();
                while(getClk()<startclk+quantum){}
                CurrentProcess->state = STOPPED;
                CurrentProcess->contextSwitchTime = getClk();
                //printf("At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                
            }else if(CurrentProcess->remainingTime <= quantum){
                startclk=getClk();
                while(getClk()<startclk+CurrentProcess->remainingTime){}
                CurrentProcess->remainingTime =0;
                CurrentProcess->finishTime = getClk();
                CurrentProcess->state = FINISHED;
                TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
                WTA = (float)TA / (float)CurrentProcess->runTime;
                printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime,TA, WTA);
                fprintf(fptr,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime,TA, WTA);
                pfinished++;

                deallocateProcess(MemoryTree, CurrentProcess->processId);
                int total_size = pow(2, ceil(log2(CurrentProcess->memory)));
                fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentProcess->memory, CurrentProcess->processId, CurrentProcess->mem_start, CurrentProcess->mem_start + total_size);
                printf("At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentProcess->memory, CurrentProcess->processId, CurrentProcess->mem_start, CurrentProcess->mem_start + total_size);

                sumRuntime+=CurrentProcess->runTime;
                sumWaitingtime+=CurrentProcess->waitingTime;
                sumWTA+=WTA;
                Lfinish=CurrentProcess->finishTime;
                CurrentProcess = NULL;
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
    fclose(memfptr);
    destroyClk(true);
    exit(0);
}