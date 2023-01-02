#include "headers.h"
#include <signal.h>

void schedulerHandler(int signum);
struct Queue ReadyQueue;
struct memTree *MemoryTree;

process *CurrentRunning = NULL;

int MemoryStart;
int msgid;
int TA=0;
float WTA=0;
FILE *fptr;
FILE *perfptr;
FILE *memfptr;
process data;

int pfinished=0;
int pCount=0;
int sumRuntime=0;
int sumWaitingtime=0;
float sumWTA=0;
int Lfinish=0;

int main(int argc, char *argv[])
{
    initClk();
    MemoryTree = create_memTree();
    signal(SIGINT, schedulerHandler);
    ReadyQueue = createQueue();

    int rec_val;
    key_t key_id;
    pCount = atoi(argv[1]);

    key_id = ftok("./clk.c", 'Z');            // create unique key
    msgid = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    struct msgbuffer msg;
    if (msgid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    fptr = fopen("Scheduler_HPF.log", "w"); // For Files
    perfptr = fopen("Scheduler_HPF.perf", "w");
    memfptr = fopen("Memory_HPF.log", "w"); // For Files
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    //printf("#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");
    printf("#At time x allocated y bytes for process z from i to j \n");
    fflush(fptr);

    while (pCount!=pfinished)
    {
        /* receive all types of messages */
        rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
        if (rec_val == -1 && pfinished == pCount - 1)   
            goto Here;
        if (rec_val == -1 && isEmpty_Queue(&ReadyQueue) == 1)
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);
        
        if (rec_val != -1)
        {
            MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
            msg.proc.mem_start = MemoryStart;
            enqueue(&ReadyQueue, msg.proc);
            int total_size = pow(2, ceil(log2(msg.proc.memory)));
            //printf("process: %d received at: %d\n", msg.proc.processId, getClk());
            printf("At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
            fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
        }

        int pid, status;
        Here:
        if (CurrentRunning == NULL)
        {
            data = peek_Queue(&ReadyQueue);
            dequeue(&ReadyQueue); // get the process and dequeue it
            CurrentRunning = &data;
        }
        if (CurrentRunning->state == WAITING)
        {
            char buffer1[5];
            sprintf(buffer1, "%d", CurrentRunning->runTime); // pass the remaining time
            CurrentRunning->startTime = getClk(); // start time of the process added to the node
            CurrentRunning->contextSwitchTime = getClk();
            int pid = fork();                     // fork the process
            CurrentRunning->state = RUNNING;
            if (pid == 0)
            {
                char *ar[] = {"./process.out", buffer1, NULL, 0};
                execve(ar[0], &ar[0], NULL);
            }
            else // it is the first time for the process to run
            {
                //printf("It is the first time to run\n");
                CurrentRunning->waitingTime = CurrentRunning->startTime - CurrentRunning->arrivalTime;
                fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
                printf("At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
                fflush(fptr);
                CurrentRunning->PID = pid;
            }
        }
        else if (CurrentRunning->state == PREEMPTED)
        {
            CurrentRunning->waitingTime =  CurrentRunning->waitingTime + getClk() - CurrentRunning->contextSwitchTime;
            CurrentRunning->contextSwitchTime = getClk();
            fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
            //printf("At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
            fflush(fptr);
            CurrentRunning->state = RUNNING;
            kill(CurrentRunning->PID, SIGCONT);
        }
        
        if (isEmpty_Queue(&ReadyQueue) == 0 && CurrentRunning->priority > peek_Queue(&ReadyQueue).priority)
        {

            CurrentRunning->contextSwitchTime = getClk();
            CurrentRunning->remainingTime -= (getClk() - CurrentRunning->startTime);
            if (CurrentRunning->remainingTime == 0)
            {
                goto checkfinish;
            }
            kill(CurrentRunning->PID, SIGTSTP);
            fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", CurrentRunning->contextSwitchTime, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
            //printf("At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", CurrentRunning->contextSwitchTime, CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime);
            fflush(fptr);
            CurrentRunning->state = PREEMPTED;
            enqueue(&ReadyQueue, *CurrentRunning);
            CurrentRunning = NULL;
            goto Here;
        }
    checkfinish:
        if (getClk() == (CurrentRunning->contextSwitchTime + CurrentRunning->remainingTime))
        {
            CurrentRunning->remainingTime = 0;
            CurrentRunning->state = FINISHED;
            CurrentRunning->finishTime = getClk();
            pfinished++;

            //for the Scheduler_HPF.log file
            TA = getClk() - CurrentRunning->arrivalTime;
            WTA = (float)TA / (float)CurrentRunning->runTime;
        
            fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime, TA, WTA);
            printf("At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), CurrentRunning->processId, CurrentRunning->arrivalTime, CurrentRunning->runTime, CurrentRunning->remainingTime, CurrentRunning->waitingTime, TA, WTA);
            fflush(fptr);

            deallocateProcess(MemoryTree, CurrentRunning->processId);
            int total_size = pow(2, ceil(log2(CurrentRunning->memory)));
            fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);
            printf("At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentRunning->memory, CurrentRunning->processId, CurrentRunning->mem_start, CurrentRunning->mem_start + total_size);

            //For the Scheduler_HPF.perf file 
            sumRuntime+=CurrentRunning->runTime;
            sumWaitingtime+=CurrentRunning->waitingTime;
            sumWTA+=WTA;
            Lfinish=CurrentRunning->finishTime;
            
            CurrentRunning = NULL;
            //printqueue(&ReadyQueue);
        }
    }

    float utilization=((float)sumRuntime/(float)Lfinish)*100;
    perfptr = fopen("Scheduler_HPF.perf", "w");
    fprintf(perfptr,"CPU utilization = %0.2f%%Avg\n",utilization);
    fprintf(perfptr,"WTA=%.2f\n",(float)sumWTA/(float)pCount);
    fprintf(perfptr,"Average waiting=%.2f\n",(float)sumWaitingtime/(float)pCount);
    
    destroyClk(true);
    msgctl(msgid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
}

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    msgctl(msgid, IPC_RMID, NULL);
    fclose(fptr);
    fclose(perfptr);
    fclose(memfptr);
    destroyClk(true);
    exit(0);
}
