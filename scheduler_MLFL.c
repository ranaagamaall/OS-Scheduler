#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue ReadyQueue;
struct Queue priorityLevel[11];
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


int main(int argc, char *argv[])
{
    initClk();
    ReadyQueue=createQueue();
    for (int i = 0; i < 5; i++)
    {
        priorityLevel[i]=createQueue();
    }
    
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
    quantum = atoi(argv[2]);

    int time;
    time = getClk();
    int rec_val;

    fptr = fopen("Scheduler_MLFL.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    for (int i = 0; i <= 10; i++)
    {
        while(pCount!=pfinished)    
        {
            do{
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
                if(rec_val == -1 && isEmpty_Queue(&priorityLevel[0]) && isEmpty_Queue(&priorityLevel[1]) 
                 && isEmpty_Queue(&priorityLevel[2]) && isEmpty_Queue(&priorityLevel[3]) && isEmpty_Queue(&priorityLevel[4]) 
                 && isEmpty_Queue(&priorityLevel[5]) && isEmpty_Queue(&priorityLevel[6]) && isEmpty_Queue(&priorityLevel[7]) 
                 && isEmpty_Queue(&priorityLevel[8]) && isEmpty_Queue(&priorityLevel[9]) && isEmpty_Queue(&priorityLevel[10]) ){
                    rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT); 
                    // waits if rec_val =-1 & all priority levels queues are empty
                }
                if (rec_val != -1)
                {
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
                    else if (msg.proc.priority=4)
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
            }while(rec_val != -1);       //handle if more than 1 message arrive at the same time


            printf("recieved a messege");
            int nextTime = getClk();

            //execution every second
            if(nextTime > time){ 
                printf("da5alt\n");
                time = getClk();

                //peek 
                if(isEmpty_Queue(&priorityLevel[i]) == 0){
                    data = peek_Queue(&priorityLevel[i]);
                    CurrentProcess = &data;
                    dequeue(&priorityLevel[i]);
                }


                int pid, status;

                if (data.state == WAITING)  //never executed before ==> check status = waiting instead of null
                {
                    CurrentProcess->state = RUNNING;
                    CurrentProcess->startTime = getClk();
                    CurrentProcess->waitingTime = getClk() - CurrentProcess->arrivalTime;
                    CurrentProcess->remainingTime = CurrentProcess->runTime;
        
                    pid = fork();
                    CurrentProcess->PID = pid;
                    if (pid == 0)
                    {
                        char buffer[20];
                        sprintf(buffer, "%d", CurrentProcess->remainingTime);

                        char *argv[] = {"./process.out", buffer, NULL, 0};
                        execve(argv[0], &argv[0], NULL);
                    }
                    else
                    {
                        printf("At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                        fprintf(fptr,"At time %d process %d started arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->remainingTime, CurrentProcess->waitingTime);
                    }
                
                }
                //if forked before ==> do not fork ==> signal continue
                else if(data.state == STOPPED){ 
                    CurrentProcess = &data;
                    CurrentProcess->state = RUNNING;
                    kill(CurrentProcess->PID, SIGCONT);
                    printf("At time %d process %d continued arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime);

                }
            }

            //process execution
            if(CurrentProcess->remainingTime > quantum){
                CurrentProcess->remainingTime -= quantum;
                printf("mark\n");
                kill(CurrentProcess->PID, SIGSTOP);
                CurrentProcess->state = STOPPED;
                printf("At time %d process %d stopped arr %d total %d remain %d wait %d \n", CurrentProcess->startTime, CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->runTime, CurrentProcess->waitingTime);

            }else if(CurrentProcess->remainingTime <= quantum){
                CurrentProcess->remainingTime =0;
                sleep(CurrentProcess->remainingTime);
                CurrentProcess->state = FINISHED;
                pfinished++;
                enqueue(&ReadyQueue, *CurrentProcess);
            }

        } 
    }
    

            

    utilization=((float)sumRuntime/(float)Lfinish)*100;
    perfptr = fopen("Scheduler.perf", "w");
    fprintf(perfptr,"CPU utilization = %0.2f%%Avg\n",utilization);
    fprintf(perfptr,"WTA=%.2f\n",(float)sumWTA/(float)pCount);
    fprintf(perfptr,"Average waiting=%.2f\n",(float)sumWaitingtime/(float)pCount);

    //whileloop(PCount)
        //2a2ra mn elmessege queue (Weslet f ma3adha) 
        //enque priority 
        //peek => Fork elproccess(execv remaining time/quantum lw feeh(quantum=remaining time)) => dequeue   
    //destroyClk(true);

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