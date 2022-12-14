#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[])
{

    //extracting command line parametes
    FILE *file = fopen(argv[1], "r");
    int algorithm = atoi(argv[3]);
    int quantum = atoi(argv[5]);


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

    int id[pCount], arrTime[pCount], runTime[pCount], priority[pCount], memory[pCount];
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

        fscanf(file, "%d", &buf);
        memory[i] = buf;
    }

    fclose(file);





    //clock forking
    int clock_pid = fork();
    if(clock_pid == 0){
        char * argv[] = {"./clk.out", 0};
        execve(argv[0], &argv[0], NULL);
    }


    //scheduler forking
    int scheduler_pid = fork();
    if(scheduler_pid == 0){

    }


    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
