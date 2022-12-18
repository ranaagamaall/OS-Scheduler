#include "headers.h"

void clearResources(int);
int msgid;
int send_msg;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    //extracting command line parametes
    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        printf("could not read file");
        exit(1);
    }
    fscanf(file, "%*[^\n]\n");      //skipping the first commented line


    //travesing though the file to count # of processes
    int pCount = 0;
    char buffer[60];
    
    while((fscanf(file, "%*[^\n]\n", buffer)) != EOF){    
        pCount ++;
    }

    int id[pCount], arrTime[pCount], runTime[pCount], priority[pCount];
    fclose(file);


    //populating data from file
    file = fopen(argv[1], "r");
    fscanf(file, "%*[^\n]\n");      
    int buf;

    for (int i=0; i<pCount; i++){
        fscanf(file, "%d", &buf);
        id[i] = buf;

        fscanf(file, "%d", &buf);
        arrTime[i] = buf;

        fscanf(file, "%d", &buf);
        runTime[i] = buf;

        fscanf(file, "%d", &buf);
        priority[i] = buf;
    }

    //For Checking the input file 
    // for (int i=0; i<pCount; i++){
    //     printf( "The id:%d arrival_time=%d runtime=%d priority%d \n", id[i], arrTime[i], runTime[i], priority[i]);
    // }

    fclose(file);

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algorithm = atoi(argv[3]);
    int quantum = atoi(argv[5]);
    // printf("The algorithm chosen:%d whith quantum=%d \n",algorithm,quantum); Printing the input

    // 3. Initiate and create the scheduler and clock processes.
    //clock forking
    int clock_pid = fork();
    if(clock_pid == 0){
        char * argv[] = {"./clk.out", 0};
        execve(argv[0], &argv[0], NULL);
    }

    //scheduler forking (Conditions of the different algorithms)
    int scheduler_pid = fork();
    if (scheduler_pid == 0){
        if (algorithm == SJF)
        {
            char buffer1[20];
            sprintf(buffer1, "%d", pCount);
            argv[1] = buffer1;

            if (execv("./scheduler_SJF.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithm == HPF)
        {
            char buffer1[20];
            sprintf(buffer1, "%d", pCount);
            argv[1] = buffer1;

            if (execv("./scheduler_HPF.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithm == RR)
        {
            char buffer1[20];
            sprintf(buffer1, "%d", pCount);
            argv[1] = buffer1;

            if (execv("./scheduler_RR.out", argv) == -1)
                perror("failed to execv");
        }
        else if (algorithm == MLFL)
        {
            char buffer1[20];
            sprintf(buffer1, "%d", pCount);
            argv[1] = buffer1;

            if (execv("./scheduler_MLFL.out", argv) == -1)
                perror("failed to execv");
        }
        
    }

    key_t key = ftok("./clk.c", 'Z');
    msgid = msgget(key, IPC_CREAT | 0666);
    struct msgbuffer msg;
    if (msgid == -1)
    {
        perror("Error in msgget");
        return 1;
    }
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    int currentP= 0;
    int x;
    x = getClk();
    while (currentP < pCount)
    {
        sleep(1);
        x = getClk();
        //printf("Current Time is %d\n", x); For printing the timer
        while (arrTime[currentP] == x)
        {

            msg.mtype = 1;
            msg.proc.processId = id[currentP];
            msg.proc.arrivalTime = arrTime[currentP];
            msg.proc.runTime = runTime[currentP];
            msg.proc.priority = priority[currentP];
            msg.proc.state = WAITING;
            msg.proc.remainingTime = runTime[currentP];

            
            send_msg = msgsnd(msgid, &msg, sizeof(msg.proc), !IPC_NOWAIT);

            if (send_msg == 0)
            {
                //printf ("At time %d Sent messege: Id:%d arrival_time=%d run_time=%d priority=%d\n",x,id[currentP],arrTime[currentP], runTime[currentP],priority[currentP] );
            }
            // 2. Increment the current process index.
            currentP++;
        }
    }
    // 5. Create a data structure for processes and provide it with its parameters.
    
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    //destroyClk(true);

    int status;
    scheduler_pid = wait(&status);
    if (WIFEXITED(status))
    {
        int msgq_del;
        msgq_del = msgctl(msgid, IPC_RMID, 0);
        destroyClk(true);
        exit(0);
    }

    return 0;
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    printf("The process generator has stopped\n");
    int msgq_del;
    msgq_del = msgctl(msgid, IPC_RMID, 0);
    destroyClk(true);
    exit(0);
}