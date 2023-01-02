#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue priorityLevel[11];
struct memTree *MemoryTree;
int MemoryStart;
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
FILE *memfptr;
float utilization;
int sumRuntime=0;
int sumWaitingtime=0;
float sumWTA;
int Lfinish;
int startclk;
int oldPriorityLevel = 10;

//TODO: lama process tegy mn bara tehotaha abl el process el 3aleha el dor
int main(int argc, char *argv[])
{
    initClk();
    MemoryTree = create_memTree();
    for (int i = 0; i <= 10; i++)
    {
        priorityLevel[i]=createQueue();
    }
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

    fptr = fopen("Scheduler_MLFL.log", "w");
    memfptr = fopen("Memory_MLFL.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");
    fprintf(memfptr, "#At time x allocated y bytes for process z from i to j \n");
    printf("#At time x allocated y bytes for process z from i to j \n");

    //2 processes of priority 0, 2 processes of priority 1, 2 processes of priority 4 (scenario)
    int i=10;  //Must be initialized by 10 not 0 bec it is set later by the highest priority(a small value) 
    while(pCount!=pfinished)    
    {
        do{
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if( rec_val == -1 && isEmpty_Queue(&priorityLevel[0])==1 && isEmpty_Queue(&priorityLevel[1])==1 
                && isEmpty_Queue(&priorityLevel[2])==1 && isEmpty_Queue(&priorityLevel[3])==1 && isEmpty_Queue(&priorityLevel[4])==1 
                && isEmpty_Queue(&priorityLevel[5])==1 && isEmpty_Queue(&priorityLevel[6])==1 && isEmpty_Queue(&priorityLevel[7])==1 
                && isEmpty_Queue(&priorityLevel[8])==1 && isEmpty_Queue(&priorityLevel[9])==1 && isEmpty_Queue(&priorityLevel[10])==1
                && CurrentProcess == NULL)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT); 
                // waits if rec_val =-1 & all priority levels queues are empty
            }
            if (rec_val != -1)
            {
                MemoryStart = allocateProcess(MemoryTree, msg.proc.memory, msg.proc.processId);
                msg.proc.mem_start = MemoryStart;
                int total_size = pow(2, ceil(log2(msg.proc.memory)));
                fprintf(memfptr, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
                printf("At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), msg.proc.memory, msg.proc.processId, MemoryStart, MemoryStart + total_size);
                if (msg.proc.priority < i)
                {
                    i=msg.proc.priority;    //we must set the i by the highest priority to start with
                }
                //printf("%d hena in time %d\n",msg.proc.processId,getClk());
                if (msg.proc.priority==0)
                {
                    enqueue(&priorityLevel[0], msg.proc);
                }
                else if (msg.proc.priority==1)
                {
                    enqueue(&priorityLevel[1], msg.proc);
                }
                else if (msg.proc.priority==2)
                {
                    enqueue(&priorityLevel[2], msg.proc);
                }
                else if (msg.proc.priority==3)
                {
                    enqueue(&priorityLevel[3], msg.proc);
                }
                else if (msg.proc.priority==4)
                {
                    enqueue(&priorityLevel[4], msg.proc);
                }
                else if (msg.proc.priority==5)
                {
                    enqueue(&priorityLevel[5], msg.proc);
                }
                else if (msg.proc.priority==6)
                {
                    enqueue(&priorityLevel[6], msg.proc);
                }
                else if (msg.proc.priority==7)
                {
                    enqueue(&priorityLevel[7], msg.proc);
                }
                else if (msg.proc.priority==8)
                {
                    enqueue(&priorityLevel[8], msg.proc);
                }
                else if (msg.proc.priority==9)
                {
                    enqueue(&priorityLevel[9], msg.proc);
                }
                else if (msg.proc.priority==10)
                {
                    enqueue(&priorityLevel[10], msg.proc);
                }
            }

        }while(rec_val != -1);
        

        int nextTime = getClk();

        //execution every second
        if(nextTime > time){ 
            time = getClk();
            if(CurrentProcess != NULL && CurrentProcess->state == STOPPED)
            {
                if (oldPriorityLevel < 10)
                {
                    //printf("Current process is: %d and priority level is: %d and real priority is: %d\n", CurrentProcess->processId, i, CurrentProcess->priority);
                    enqueue(&priorityLevel[oldPriorityLevel + 1], *CurrentProcess);
                }   
                else
                    enqueue(&priorityLevel[oldPriorityLevel], *CurrentProcess);
            }

            for (int j = 0; j < 11; j++)
            {
                if (isEmpty_Queue(&priorityLevel[j])!=1) //if not empty
                {
                    i=j;
                    break;   
                }
            }
            // if (isEmpty_Queue(&priorityLevel[oldPriorityLevel])==1 && i<10)
            // {
            //     for (int j = 0; j < 11; j++)
            //     {
            //         if (isEmpty_Queue(&priorityLevel[j])!=1) //if not empty
            //         {
            //             i=j;
            //             break;   
            //         }
            //     }
            // }
                
            //peek 
            if(isEmpty_Queue(&priorityLevel[i]) == 0){
                data = peek_Queue(&priorityLevel[i]);
                CurrentProcess = &data;
                dequeue(&priorityLevel[i]);
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
            if(CurrentProcess->remainingTime > quantum)
            {
                CurrentProcess->remainingTime -= quantum;
                kill(CurrentProcess->PID, SIGTSTP);
                startclk=getClk();
                while(getClk()<startclk+quantum){}  //Sleep
                CurrentProcess->state = STOPPED;
                 oldPriorityLevel = i;
                CurrentProcess->contextSwitchTime = getClk();
                //printf("At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                fprintf(fptr,"At time %d process %d stopped arr %d total %d remain %d wait %d \n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                
                    //i++;   //move to the next priority level queue and perform RR on it
                
            }else if(CurrentProcess->remainingTime <= quantum){
                startclk=getClk();
                while(getClk()<startclk+CurrentProcess->remainingTime){}  //Sleep
                CurrentProcess->remainingTime =0;
                CurrentProcess->state = FINISHED;
                CurrentProcess->finishTime = getClk();
                TA = CurrentProcess->finishTime - CurrentProcess->arrivalTime;
                WTA = (float)TA / (float)CurrentProcess->runTime;
                printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime,TA, WTA);
                fprintf(fptr,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime,TA, WTA);
                pfinished++;

                deallocateProcess(MemoryTree, CurrentProcess->processId);
                int total_size = pow(2, ceil(log2(CurrentProcess->memory)));
                fprintf(memfptr, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentProcess->memory, CurrentProcess->processId, CurrentProcess->mem_start, CurrentProcess->mem_start + total_size);
                printf("At time %d freed %d bytes for process %d from %d to %d\n", getClk(), CurrentProcess->memory, CurrentProcess->processId, CurrentProcess->mem_start, CurrentProcess->mem_start + total_size);
                //printf("Priority level is: %d\n", i);
                //printqueue(&priorityLevel[i]);
                if (isEmpty_Queue(&priorityLevel[i])==1 && i<10)
                {
                    for (int j = 0; j < 11; j++)
                    {
                        if (isEmpty_Queue(&priorityLevel[j])!=1) //if not empty
                        {
                            i=j;
                            //printf("el i = %d\n",i);
                            break;   
                        }
                    }
                }
                    //i++; //Corner case, when a process finishes and the queue is empty we must proceed to the next priority level queue
                        // note: if process finishes but queue is not empty we should continue in the same queue normally to perform RR on the rest processes in this queue
                
                sumRuntime+=CurrentProcess->runTime;
                sumWaitingtime+=CurrentProcess->waitingTime;
                sumWTA+=WTA;
                Lfinish=CurrentProcess->finishTime;
                CurrentProcess = NULL; 
            }
            
        }
    } 
    

    utilization=((float)sumRuntime/(float)Lfinish)*100;
    perfptr = fopen("Scheduler_MLFL.perf", "w");
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
    fclose(perfptr);
    fclose(fptr);
    fclose(memfptr);
    destroyClk(true);
    exit(0);
}