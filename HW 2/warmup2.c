#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>

#include "cs402.h"
#include "my402list.h"

#define MAXIM_VALUE 2147483647
#define MAX_TIME_IN_MS (10000)  

typedef struct packetBundle {  
    int packetID;
    int numTokenNeeded;  
    int serviceTime;
    double packetInterArrivalTime; 
    double arrival_time; 
    double q1EnterTime;   
    double q1LeaveTime;  
    double q2EnterTime; 
    double q2LeaveTime;  
    double beginServiceTime;
    double quitTime;  
} PacketBundle;

My402List q1,q2;
pthread_mutex_t mutex;
pthread_cond_t cv;
sigset_t set;
struct timeval systemTime; 
int num, tmp_num, B, P;
int numDroppedPackets = 0; 
int numDroppedTokens = 0;
int tokenID = 0;
int numTokenInBucket = 0; 
int packetID;
double emulationTime; 
double lambda, mu, r;
pthread_t arrivalThread, tokenThread, serverOneThread, serverTwoThread, cntrlCThread;
char *tsfile;
FILE *fp = NULL;
bool isReadFile;
bool isTokenThreadWorking = false;
bool isArrivalThreadRunning = false;
int shutdown;
int numCompletedPackets = 0;
int numRemovedPackets = 0; 
int totalNumOfPackets = 0;
double averIntervalTime = 0;
double q1AvgTime = 0;
double q2AvgTime = 0;
double averServiceTime = 0; 
double totalServiceTime = 0;

double s1AvgTime = 0; 
double s2AvgTime = 0;

double averSystemTime = 0; 
double averSystemTimeSqua = 0; 
double variance;
double prevArrTime = 0; 

double totalTimeInQ1 = 0;
double totalTimeInQ2 = 0;
double totalTimeInS1 = 0;
double totalTimeInS2 = 0;


double generateTimeDiffer(struct timeval systemTime) { 
    struct timeval currTime;
    double sTime = 1000 * (systemTime.tv_sec) + (systemTime.tv_usec) / 1000.0 ;

    gettimeofday(&currTime, NULL);
    double cTime = 1000 * (currTime.tv_sec) + (currTime.tv_usec) / 1000.0 ;
    return (cTime - sTime);
}

void *signalHandler(void *arg) {
    while(1){
        int sig; 
        sigwait(&set, &sig); 
        pthread_mutex_lock(&mutex);
        fprintf(stdout, "SIGINT caught, no new packets or tokens will be allowed\n");
        shutdown = 1;
        pthread_cancel(arrivalThread);
        pthread_cancel(tokenThread);
        pthread_cond_broadcast(&cv);
        while (!My402ListEmpty(&q1)) {
            PacketBundle *currQ1Packet = (PacketBundle *)(My402ListFirst(&q1)->obj);
            My402ListUnlink(&q1, My402ListFirst(&q1));
            fprintf(stdout, "%012.3fms: p%d removed from Q1\n", generateTimeDiffer(systemTime), currQ1Packet->packetID);
        }
        while (!My402ListEmpty(&q2)) {
            PacketBundle *currQ2Packet = (PacketBundle *)(My402ListFirst(&q2)->obj);
            My402ListUnlink(&q2, My402ListFirst(&q2));
            fprintf(stdout, "%012.3fms: p%d removed from Q2\n", generateTimeDiffer(systemTime), currQ2Packet->packetID);
        }
        numRemovedPackets = numRemovedPackets + 1;
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

void *doServerThread(void *arg) {
    int serverID = (int)arg;
    int sleepTime;
    
    while (isTokenThreadWorking || !My402ListEmpty(&q2)) {
        pthread_mutex_lock(&mutex);
        while ( !shutdown && My402ListEmpty(&q2) && isTokenThreadWorking) {
            pthread_cond_wait(&cv, &mutex);
        }
  
        PacketBundle *currPacket = NULL;
        if (!My402ListEmpty(&q2)) {
            
            My402ListElem *topElem=My402ListFirst(&q2);
            currPacket=(PacketBundle*)topElem->obj;
            My402ListUnlink(&q2, topElem);
    
            currPacket->q2LeaveTime = generateTimeDiffer(systemTime);
            fprintf(stdout, "%012.3fms: p%d leaves Q2, time in Q2 = %.3fms\n", 
            currPacket->q2LeaveTime, currPacket->packetID, currPacket->q2LeaveTime - currPacket->q2EnterTime);
            currPacket->beginServiceTime = generateTimeDiffer(systemTime);
            fprintf(stdout, "%012.3fms: p%d begins service at S%d, requesting %dms of service\n",
            currPacket->beginServiceTime, currPacket->packetID, serverID, currPacket->serviceTime);
        }
        else {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        sleepTime = 1000 * currPacket->serviceTime;
        usleep(sleepTime);
        currPacket->quitTime = generateTimeDiffer(systemTime);
        fprintf(stdout, "%012.3fms: p%d departs from S%d, service time = %.3fms, time in system = %.3fms\n", currPacket->quitTime, currPacket->packetID, serverID, currPacket->quitTime - currPacket->beginServiceTime, currPacket->quitTime - currPacket->arrival_time);
        
        averServiceTime = averServiceTime + (currPacket->quitTime - currPacket->beginServiceTime);
        totalServiceTime = totalServiceTime + (currPacket->quitTime - currPacket->beginServiceTime);
        
        
        q1AvgTime = q1AvgTime + (currPacket->q1LeaveTime - currPacket->q1EnterTime);
        totalTimeInQ1 = totalTimeInQ1 + (currPacket->q1LeaveTime - currPacket->q1EnterTime);
        q2AvgTime = q2AvgTime + (currPacket->q2LeaveTime - currPacket->q2EnterTime);
        totalTimeInQ2 = totalTimeInQ2 + (currPacket->q2LeaveTime - currPacket->q2EnterTime);
        if(serverID == 1){
            s1AvgTime += currPacket->quitTime - currPacket->beginServiceTime;
            totalTimeInS1 += currPacket->quitTime - currPacket->beginServiceTime;
        }
        else{
            s2AvgTime += currPacket->quitTime - currPacket->beginServiceTime;
            totalTimeInS2 += currPacket->quitTime - currPacket->beginServiceTime;
        }
        averSystemTime += currPacket->quitTime - currPacket->arrival_time;
        averSystemTimeSqua += pow(currPacket->quitTime - currPacket->arrival_time, 2);
        numCompletedPackets = numCompletedPackets + 1;
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cv);
    shutdown = 1;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *doTokenThread(void *arg) {
    int sleepTime;
    double interArrivalTime = 1000.0 * (1/r);
    while (!My402ListEmpty(&q1) || isArrivalThreadRunning) {
        sleepTime = 1000 * interArrivalTime;
        usleep(sleepTime);
        pthread_mutex_lock(&mutex);
        pthread_cleanup_push((void *)pthread_mutex_unlock, &mutex);
        tokenID = tokenID+1;
        if(numTokenInBucket < B){
            numTokenInBucket++;
            fprintf(stdout, "%012.3fms: token t%d arrives, token bucket now has %d token(s)\n", generateTimeDiffer(systemTime), tokenID, numTokenInBucket);
        }
        else{
            numDroppedTokens++;
            fprintf(stdout, "%012.3fms: token t%d arrives, dropped\n", generateTimeDiffer(systemTime), tokenID);
        }
        
        PacketBundle *currPacket = NULL;
        while (!My402ListEmpty(&q1)) {
            My402ListElem *topElem=My402ListFirst(&q1);
            currPacket = (PacketBundle*)topElem->obj;
            if(numTokenInBucket < currPacket->numTokenNeeded) {
                break;
            }
            My402ListUnlink(&q1, topElem);
            numTokenInBucket -= currPacket->numTokenNeeded;
            currPacket->q1LeaveTime = generateTimeDiffer(systemTime);
            fprintf(stdout, "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token(s)\n", currPacket->q1LeaveTime, currPacket->packetID, currPacket->q1LeaveTime - currPacket->q1EnterTime, numTokenInBucket);
            My402ListAppend(&q2, currPacket);
            currPacket->q2EnterTime = generateTimeDiffer(systemTime);
            fprintf(stdout, "%012.3fms: p%d enters Q2\n", currPacket->q2EnterTime, currPacket->packetID);
            if(My402ListLength(&q2) == 1 ) {
                pthread_cond_broadcast(&cv);
            }
        }
        
        pthread_cleanup_pop(0);
        pthread_mutex_unlock(&mutex);
    }
   
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cv);
    
    isTokenThreadWorking = false; 
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *doArrivalThread (void *arg) {
    PacketBundle* aPacket;
    int sleepTime;
    double prevArrTime = 0;
    int packetOrderNum = 1;
    int i;
    for (i = 0; i < num; i++) { 
        
        if(isReadFile){
            char buf[1024];
            if(fgets(buf, sizeof(buf), fp) != NULL) { 
                if (buf[strlen(buf)-1] == '\n') { 
                    buf[strlen(buf)-1] = '\0';
                }
                if(strlen(buf) > (1024 - 2)) { 
                    fprintf(stderr, "Error: There are at most 1024 characters in each line in the tfile. \n");
                    exit(1);
                }

                aPacket = (PacketBundle*)malloc(sizeof(PacketBundle)); 
                aPacket->packetID = packetOrderNum;
                char* tmp;
                tmp = strtok(buf, " \n\t");
                aPacket->packetInterArrivalTime = atof(tmp);  
                tmp = strtok(NULL, " \n\t");
                aPacket->numTokenNeeded = atoi(tmp);
                tmp = strtok(NULL, " \n\t");
                aPacket->serviceTime = atof(tmp);  
                    
                sleepTime = aPacket->packetInterArrivalTime * 1000;
             
                usleep(sleepTime);
                pthread_mutex_lock(&mutex);
                    
                totalNumOfPackets = totalNumOfPackets + 1;
                pthread_cleanup_push((void *)pthread_mutex_unlock, &mutex);
                aPacket->arrival_time = generateTimeDiffer(systemTime);
                    
                    
                if(aPacket->numTokenNeeded <= B) {    
                    fprintf(stdout, "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n", aPacket->arrival_time, aPacket->packetID, aPacket->numTokenNeeded, aPacket->arrival_time - prevArrTime);
                            
                    averIntervalTime = averIntervalTime + (aPacket->arrival_time - prevArrTime);
                            
                    prevArrTime = aPacket->arrival_time;
                    My402ListAppend(&q1, aPacket);
                    aPacket->q1EnterTime = generateTimeDiffer(systemTime);
                    fprintf(stdout, "%012.3fms: p%d enters Q1\n", aPacket->q1EnterTime, aPacket->packetID);
                    if(My402ListLength(&q1) == 1) {
                        My402ListElem * firstElem = My402ListFirst(&q1);
                        if(numTokenInBucket >= aPacket->numTokenNeeded) {
                            My402ListUnlink(&q1, firstElem);
                            numTokenInBucket -= aPacket->numTokenNeeded;
                            aPacket->q1LeaveTime = generateTimeDiffer(systemTime);
                            fprintf(stdout, "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token(s)\n", aPacket->q1LeaveTime, aPacket->packetID, aPacket->q1LeaveTime - aPacket->q1EnterTime, numTokenInBucket);
                            My402ListAppend(&q2, aPacket);
                            aPacket->q2EnterTime = generateTimeDiffer(systemTime);
                            fprintf(stdout, "%012.3fms: p%d enters Q2\n", aPacket->q2EnterTime, aPacket->packetID);
                            if(My402ListLength(&q2) == 1){
                                pthread_cond_broadcast(&cv);
                            }  
                        }
                    } 
                }
                else {
                    fprintf(stdout, "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms, dropped\n", 
                    aPacket->arrival_time, aPacket->packetID, aPacket->numTokenNeeded, aPacket->arrival_time - prevArrTime);
                            
                    averIntervalTime = averIntervalTime + aPacket->arrival_time - prevArrTime;
                    prevArrTime = aPacket->arrival_time;
                            
                    numDroppedPackets = numDroppedPackets+1;
                }
                pthread_cleanup_pop(1);
            }
        }
        else {  
            
            PacketBundle *aPacket = (PacketBundle*)malloc(sizeof(PacketBundle));
            aPacket->packetID = packetOrderNum;
            aPacket->numTokenNeeded = P;
            aPacket->packetInterArrivalTime = min(MAX_TIME_IN_MS, 1000.0/lambda);  
            aPacket->serviceTime = min(MAX_TIME_IN_MS, 1000.0/mu); 
            
            sleepTime = aPacket->packetInterArrivalTime * 1000;
            usleep(sleepTime);
            pthread_mutex_lock(&mutex);
                    
            totalNumOfPackets++;
            pthread_cleanup_push((void *)pthread_mutex_unlock, &mutex);
            aPacket->arrival_time = generateTimeDiffer(systemTime);
                    
                    
            if(aPacket->numTokenNeeded <= B) {     
                fprintf(stdout, "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n", 
                aPacket->arrival_time, aPacket->packetID, aPacket->numTokenNeeded, aPacket->arrival_time - prevArrTime);
                            
                averIntervalTime = averIntervalTime + aPacket->arrival_time - prevArrTime;
                prevArrTime = aPacket->arrival_time;
                My402ListAppend(&q1, aPacket);
                aPacket->q1EnterTime = generateTimeDiffer(systemTime);
                fprintf(stdout, "%012.3fms: p%d enters Q1\n", aPacket->q1EnterTime, aPacket->packetID);
                    if(My402ListLength(&q1) == 1) {
                        My402ListElem * firstElem = My402ListFirst(&q1);
                        if(numTokenInBucket >= aPacket->numTokenNeeded) {
                            My402ListUnlink(&q1, firstElem);
                            numTokenInBucket -= aPacket->numTokenNeeded;
                            aPacket->q1LeaveTime = generateTimeDiffer(systemTime);
                            fprintf(stdout, "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token(s)\n", aPacket->q1LeaveTime, aPacket->packetID, aPacket->q1LeaveTime - aPacket->q1EnterTime, numTokenInBucket);
                            My402ListAppend(&q2, aPacket);
                            aPacket->q2EnterTime = generateTimeDiffer(systemTime);
                            fprintf(stdout, "%012.3fms: p%d enters Q2\n", aPacket->q2EnterTime, aPacket->packetID);
                            if(My402ListLength(&q2) == 1){
                                pthread_cond_broadcast(&cv);
                            }  
                        }
                    } 
                }
                else {
                    fprintf(stdout, "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms, dropped\n", aPacket->arrival_time, aPacket->packetID, aPacket->numTokenNeeded, aPacket->arrival_time - prevArrTime);
                    prevArrTime = aPacket->arrival_time;
                            
                    numDroppedPackets++;
                }
                pthread_cleanup_pop(1);
            }
    
        packetOrderNum++;
        }
    if(isReadFile) {
        fclose(fp);
    }
    
    isArrivalThreadRunning = false;
    pthread_exit(NULL);
   
}


void processFile(char *tsfile ) { 
    
    struct stat goThrough;
    stat(tsfile, &goThrough);
    fp = fopen(tsfile , "r");  
    if (S_ISDIR(goThrough.st_mode)) {
        fprintf(stderr, "Error: The file ");
        fprintf(stderr, "%s", tsfile);
        fprintf(stderr, " is a directory. \n");
        exit(1);			 
    }
    if (fp == NULL) { 
        fprintf(stderr, "%s", strerror(errno));
        fprintf(stderr, "%s", tsfile);
        fprintf(stderr, "%s", "\n");
        exit(1);
    }
    char buf[1024]; 
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fprintf(stderr, "Error: The file is empty \n");
        exit(1);
	}
    else { 
        char *start_ptr = buf; 
        char *tab_ptr = strchr(start_ptr, '\t');  
        if (tab_ptr != NULL) { 
            fprintf(stderr, "Error: In the first line, there should be only num element \n");  
            exit(1);
        }
        if (start_ptr[0] == '-') {
            fprintf(stderr, "Error: %s is not in the right format\n", tsfile);
            exit(1);
        }
        num = atoi(start_ptr); 
	    if (num == 0) {
            fprintf(stderr, "Error: %s is not in the right format \n", tsfile);
            exit(1);
        }  
    }   
}

void emulateParame() {
    fprintf(stdout, "Emulation Parameters:\n");
    fprintf(stdout, "\tnumber to arrive = %i\n", num);
    if (!isReadFile) { 
        fprintf(stdout, "\tlambda = %.6g\n", lambda);
        fprintf(stdout, "\tmu = %.6g\n", mu);
        fprintf(stdout, "\tr = %.6g\n", r);
        fprintf(stdout, "\tB = %i\n", B);
        fprintf(stdout, "\tP = %i\n", P);
    }
    if (isReadFile) { 
        fprintf(stdout, "\tr = %.6g\n", r); 
        fprintf(stdout, "\tB = %i\n", B);
        fprintf(stdout, "\ttsfile = %s\n", tsfile);
    }
} 
void readCommandLine(int argc, char *argv[]){
    if (argc > 15) { 
        fprintf(stderr, "Error: Malformed command: you should type warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
        exit(1);
    }
    if(argc%2 == 0) {
        fprintf(stderr, "Error: Malformed command: you should type warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
        exit(1);
    }
    int index;
    for(index = 1; index < argc; index = index + 2) {      
        if (strcmp (argv[index], "-lambda") == 0 ) { 
            if (index+1 < argc) {
                if (argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%lf", &lambda) != 1) {    
                        fprintf(stderr, "Error: Malformed command, the amount of lambda %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%lf", &lambda); 
                    }
                }
                else {
                    fprintf(stderr, "Error: Malformed command, the amount of lambda %s can not be negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing some arguments. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else if (strcmp (argv[index], "-mu") == 0 ) {
            if (index+1 < argc) {
                if(argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%lf", &mu) != 1) { 
                        fprintf(stderr, "Error: Malformed command, the amount of mu %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%lf", &mu); 
                    }
				}
				else {
                    fprintf(stderr, "Error: Malformed command, the amount of mu %s can not be negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                } 
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing some arguments. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else if (strcmp (argv[index], "-r") == 0 ) {
            if (index+1 < argc) {
                if(argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%lf", &r) != 1) { 
                        fprintf(stderr, "Error: Malformed command, the amount of r %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index+1]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%lf", &r); 
                    }
				}
				else {
                    fprintf(stderr, "Error: Malformed command, the amount of r %s can not be negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing some arguments. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        
        else if (strcmp (argv[index], "-B") == 0 ) {
            if (index+1 < argc) {
                if(argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%i", &B) != 1) {     
                        fprintf(stderr, "Error: Malformed command, the amount of B %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index+1]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%i", &B); 
                    }
				}
				else {
                    fprintf(stderr, "Error: Malformed command, the amount of B %s is negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                }
                if (B > MAXIM_VALUE) { 
                    fprintf(stderr, "Error: Malformed command, B can not be greater than 2147483647. \n");
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing some arguments. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else if (strcmp (argv[index], "-P") == 0 ) {
            if (index+1 < argc) {
                if(argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%i", &P) != 1) {     
                        fprintf(stderr, "Error: Malformed command, the amount of P %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%i", &P); 
                    }
				}
				else {
                    fprintf(stderr, "Error: Malformed command, the amount of P %s can not be negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                }
                if (P > MAXIM_VALUE) { 
                    fprintf(stderr, "Error: Malformed command, P can not be greater than 2147483647. \n");
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing some arguments. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else if (strcmp (argv[index], "-n") == 0 ) {    
            if (index+1 < argc) {
                if(argv[index+1][0] != '-') {
                    if (sscanf(argv[index+1], "%i", &num) != 1) { 
                        fprintf(stderr, "Error: Malformed command, the amount of num %s is incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                        exit(1);
                    } 
                    else {
                        sscanf(argv[index+1], "%i", &num); 
                    }
                }
                else {
                    fprintf(stderr, "Error: Malformed command, the amount of num %s can not be negative. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[index]);
                    exit(1);
                }
                if (num > MAXIM_VALUE) { 
                    fprintf(stderr, "Error: Malformed command, num can not be greater than 2147483647. \n");
                    exit(1);
                }
            }
            else {
                fprintf(stderr, "Error: Malformed command, you are missing num. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else if (strcmp (argv[index], "-t") == 0 ) {
            if (index+1 < argc) { 
                isReadFile = true;
                tsfile= argv[index+1];
            }
            else { 
                fprintf(stderr, "Error: Malformed command, you are missing tsfile name. You should type: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
                exit(1);
            }
        }
        else {
            fprintf(stderr, "Error: Malformed command, your line arguments are incorrect. You should type: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
            exit(1);
        }
    }
    
}


void doStatistics() {
    fprintf(stdout, "Statistics:\n");
    fprintf(stdout, "\n");
    
    averIntervalTime = averIntervalTime/totalNumOfPackets;
    if(totalNumOfPackets == 0) {
		fprintf(stdout, "\taverage packet inter-arrival time = %.6g, no packet arrived at this facility\n", averIntervalTime/1000.0);
	}
    else {
        
        fprintf(stdout, "\taverage packet inter-arrival time = %.6g\n", averIntervalTime/1000.0);
    }
    
    
    averServiceTime = averServiceTime/numCompletedPackets;
    if (numCompletedPackets == 0) {  
        fprintf(stdout, "\taverage packet service time = %.6g, no completed packets at this facility\n", averServiceTime/1000.0);
        fprintf(stdout, "\n");
    }
    else {
        fprintf(stdout, "\taverage packet service time = %.6g\n", averServiceTime/1000.0);
        fprintf(stdout, "\n");
    }
    
    fprintf(stdout, "\taverage number of packets in Q1 = %.6g\n", totalTimeInQ1/emulationTime);
    fprintf(stdout, "\taverage number of packets in Q2 = %.6g\n", totalTimeInQ2/emulationTime);
    fprintf(stdout, "\taverage number of packets in S1 = %.6g\n", totalTimeInS1/emulationTime);
    fprintf(stdout, "\taverage number of packets in S2 = %.6g\n", totalTimeInS2/emulationTime);
    fprintf(stdout, "\n");
    
    
    averSystemTime = averSystemTime/numCompletedPackets;
    averSystemTimeSqua = averSystemTimeSqua/numCompletedPackets;
    variance = averSystemTimeSqua - pow(averSystemTime, 2);
    double std;
    std = sqrt(variance);
    if (numCompletedPackets == 0) {  
        fprintf(stdout, "\taverage time a packet spent in system = %.6g, no completed packets at this facility\n", averSystemTime/1000.0);
        fprintf(stdout, "\tstandard deviation for time spent in system = %.6g, no completed packets at this facility\n", std/1000.0);
        fprintf(stdout, "\n");
    }
    else {
        fprintf(stdout, "\taverage time a packet spent in system = %.6g\n", averSystemTime/1000.0);
        fprintf(stdout, "\tstandard deviation for time spent in system = %.6g\n", std/1000.0);
        fprintf(stdout, "\n");
    }
    
    
    double tokenDropProb;
    tokenDropProb = 1.0 * numDroppedTokens / tokenID;
    if(tokenID == 0) { 
        fprintf(stdout, "\ttoken drop probability = %.6g, no tokens arrived at this facility\n", tokenDropProb);
    }
    else {
        fprintf(stdout, "\ttoken drop probability = %.6g\n", tokenDropProb);
    }
    
    
    double packDropProb;
    packDropProb = (1.0 * numDroppedPackets ) / totalNumOfPackets;
    
    if (totalNumOfPackets == 0) {
        fprintf(stdout, "\tpacket drop probability = %.6g, no packets arrived at this facility\n", packDropProb);
    }
    else {
        fprintf(stdout, "\tpacket drop probability = %.6g\n", packDropProb);
    }
    
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[]) {  
    
    isReadFile = false;
    lambda = (double)1;   
    mu = (double)0.35;
    r = (double)1.5;
    B = 10;         
    P = 3;
    num = 20;
    
    readCommandLine(argc, argv);
    
    shutdown = 0;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, 0);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv, NULL);
    memset(&q1, 0, sizeof(My402List));
    memset(&q2, 0, sizeof(My402List));
    My402ListInit(&q1);
    My402ListInit(&q2);
    if(isReadFile) {
        processFile(tsfile);
    }
    
    emulateParame();
   
    if(!isReadFile) {
        if(lambda < 0.1) {
			lambda = 0.1;
		}
		if(mu<0.1) {
			mu=0.1;
		}
		if (r<0.1) {
			r=0.1;
		}
    }
    
    gettimeofday(&systemTime, NULL);
    fprintf(stdout, "%012.3fms: emulation begins\n", generateTimeDiffer(systemTime));
    pthread_create(&cntrlCThread, 0, signalHandler , NULL);
    pthread_create(&arrivalThread, 0, doArrivalThread , NULL);
    isArrivalThreadRunning = true;
    pthread_create(&tokenThread, 0, doTokenThread, NULL);
    isTokenThreadWorking = true;
    
    pthread_create(&serverOneThread, 0, doServerThread, (void *)1);
    pthread_create(&serverTwoThread, 0, doServerThread, (void *)2);
    pthread_join(arrivalThread, NULL);
    pthread_join(tokenThread, NULL);
    pthread_join(serverOneThread, NULL);
    pthread_join(serverTwoThread, NULL);
    
    pthread_cancel(cntrlCThread);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cv);
    emulationTime = generateTimeDiffer(systemTime);
    fprintf(stdout, "%012.3fms: emulation ends\n", emulationTime);
    fprintf(stdout, "\n");
    doStatistics(); 
    return 0;
}

