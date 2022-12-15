#include "headers.h"

void childHandler(int signum);
void schedulerHandler(int signum);

struct Queue ReadyQueue;
int pCount = 0;
int pfinished =0;
process *CurrentProcess = NULL;
process data;
int msgid;

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

    int time;
    time = getClk();
    int rec_val;

    while(pCount!=pfinished)
    {
        do
        {
            /* receive all types of messages */
            rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, IPC_NOWAIT); // shouldn't wait for msg
            if (rec_val == -1 && isEmpty_Queue(&ReadyQueue) == 1)
            {
                rec_val = msgrcv(msgid, &msg, sizeof(msg.proc), 0, !IPC_NOWAIT);
            }
            if (rec_val != -1)
            {
                enqueue(&ReadyQueue, msg.proc);
            }
        } while (rec_val != -1);


        int pid, status;

        if (CurrentProcess == NULL && isEmpty_Queue(&ReadyQueue) == 0)
        {
            data = peek_Queue(&ReadyQueue);
            CurrentProcess = &data;
            CurrentProcess->state = RUNNING;
            dequeue(&ReadyQueue);
            printf("\nProcess %d Running: %d %d %d %d\n", CurrentProcess->processId,CurrentProcess->processId, CurrentProcess->arrivalTime, CurrentProcess->runTime, CurrentProcess->priority);
            CurrentProcess->startTime = getClk();
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
            }
        }
    }




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
    printf("The proccess has finished: %d start: %d run: %d\n", getClk(), CurrentProcess->startTime, CurrentProcess->runTime);
    pfinished++;
    CurrentProcess = NULL;
    signal(SIGCHLD, childHandler);
}

void schedulerHandler(int signum)
{
    printf("The Scheduler has stopped\n");
    msgctl(msgid, IPC_RMID, NULL);
    destroyClk(true);
    exit(0);
}