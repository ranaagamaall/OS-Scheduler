typedef struct process
{
    int runTime;
    int priority;
    int processId;
    int arrivalTime;
    int size;
    pid_t PID;

    int state;

    int waitingTime;
    int remainingTime;
    int contextSwitchTime;

    int startTime;
    int finishTime;
    int memory;
    int mem_start;
} process;

process initializeProcess(
    int runtime, int priority,
    int processId, int arrivalTime, int size)
{
    process p;
    p.runTime = runtime;
    p.priority = priority;
    p.processId = processId;
    p.arrivalTime = arrivalTime;
    p.remainingTime = runtime; //we will decrement from it later
    p.size = size;

    p.state=0;

    p.waitingTime = 0;
    p.contextSwitchTime = -1;
    p.finishTime = -1;
    p.startTime = -1;
    return p;
}

void initializeProcessPointer(
    process *p,
    int runtime, int priority,
    int processId, int arrivalTime, int size)
{
    p->runTime = runtime;
    p->priority = priority;
    p->processId = processId;
    p->arrivalTime = arrivalTime;
    p->remainingTime = runtime;
    p->size = size;

    p->state=0;
    p->waitingTime = 0;
    p->contextSwitchTime = -1;
    p->finishTime = -1;
    p->startTime = -1;
}
process copyProcess(process input)
{
    return input;
};