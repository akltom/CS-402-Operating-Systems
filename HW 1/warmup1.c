#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "cs402.h"
#include "my402list.h"

typedef struct TransactionFields
{
    char type;
    time_t time;
    int amount;
    char *description;
}   My402TransactionFields;

int MAXIMUM_LINE_LENGTH = 1026;

void printList(My402List *aList, int balance) {
    My402ListElem * elem = NULL;
    My402TransactionFields * tran = NULL;
    if(My402ListEmpty(aList)){
        fprintf(stderr, "This is the end of tfile. \n"); 
        exit(1);
	}
    
    int thisBalance = balance;
    for (elem = My402ListFirst(aList); elem != NULL; elem = My402ListNext(aList, elem)) {
        printf("%s","| ");

		tran = (My402TransactionFields*)elem->obj;  

		char * thisTime;
		thisTime = ctime(&(tran->time));
		fprintf(stdout, "%.*s", 11, thisTime);
		
		fprintf(stdout, "%.*s", 4, (thisTime+20));  // the + 20 thing is to convert the date to year, must use
		printf("%s"," | ");

		char * thisDesc;
		thisDesc = tran->description;
		int numChar = 0;
		int numDesChar = 24;
		int k,p,r;

		thisDesc[24] = '\0';
		

		if (strlen(thisDesc) > numDesChar) {
			for (p=0; p<numDesChar; ++p){
				fprintf(stdout, "%c", thisDesc[p]);
			}
		}
		else {
			for (k = 0; thisDesc[k]!= '\0'; ++k){
				fprintf(stdout, "%c", thisDesc[k]);
				numChar++;
			}
		}
		
		int numEmptySpace = 0;
		numEmptySpace = numDesChar - numChar;
		for (r=1; r<=numEmptySpace; r++) {
			fprintf(stdout, " ");
		}
		printf("%s"," | ");
		
		int thisAmount = 0; 
		thisAmount = (tran->amount);

		char numNum[15];
 
		memset(numNum, ' ', sizeof(numNum)); 
		numNum[14] = '\0';

		char thisType;
		thisType = tran->type;
		if (thisType == '-') {
			numNum[0] = '(';
			numNum[13] = ')';
		}

        int numElem = 0;
		int tempAmount = thisAmount;
		while (tempAmount !=0) {
			tempAmount = tempAmount/10;
			numElem = numElem +1;
		}

		int tempAmountTwo = thisAmount;
		if (numElem <= 9) { 
            numNum[10] = '.';
            numNum[12] = tempAmountTwo % 10 + '0';
            tempAmountTwo = tempAmountTwo/10;
			numNum[11] = tempAmountTwo % 10 + '0';
			tempAmountTwo = tempAmountTwo/10;
			int degit = 9;
			if(tempAmountTwo == 0) {
                numNum[9] = '0';
			}
			while (tempAmountTwo > 0) {
				numNum[degit]= tempAmountTwo % 10 + '0';
				tempAmountTwo = tempAmountTwo/10;
				degit = degit - 1;
				
				if (tempAmountTwo > 0 && degit == 6) {
                    numNum[degit] = ',';
                    degit = degit - 1;
				}
				if (tempAmountTwo > 0 && degit == 2) {
					numNum[degit] = ',';
					degit = degit - 1;
				}
			}
			
			printf("%s", numNum);
            
		}
		else {
			if (thisType == '+') {
				printf("%s"," ?,???,???.?? ");
				
			}
			if (thisType == '-') {
				printf("%s"," (?,???,???.??) ");

			}
		}
		printf("%s"," | ");

		if (thisType == '+') {
			thisBalance = thisBalance + thisAmount;
		}
		if (thisType == '-') {
			thisBalance = thisBalance - thisAmount;
		}
        
		char numBal[15];
		int numElemBal = 0;
 
		memset(numBal, ' ', sizeof(numBal)); 
		numBal[14] = '\0';

		if (thisBalance < 0) {
			numBal[0] = '(';
			numBal[13] = ')';
		}

		int tempBalance = thisBalance;
		while (tempBalance != 0) {
			tempBalance = abs(tempBalance);
			if (thisBalance > 0) {
                tempBalance = tempBalance/10;
                numElemBal = numElemBal + 1;
			}
			if (thisBalance < 0) {
                tempBalance = tempBalance/10;
                numElemBal = numElemBal + 1;
			}
		}


		int tempBalanceTwo = abs(thisBalance);
		if (numElemBal <= 9) {  
					
			numBal[10]='.';
			numBal[12]= tempBalanceTwo % 10 + '0';
			tempBalanceTwo = tempBalanceTwo/10;
			numBal[11]= tempBalanceTwo % 10 + '0';
			tempBalanceTwo = tempBalanceTwo/10;
			int degitBal =9;
			if(tempBalanceTwo == 0) {
				numBal[9] = '0';
			}
            
			while (tempBalanceTwo > 0) {
				numBal[degitBal]= tempBalanceTwo % 10 + '0';
				tempBalanceTwo = tempBalanceTwo/10;
				degitBal = degitBal - 1;
				
				if (tempBalanceTwo > 0 && degitBal == 6){
					numBal[degitBal] = ',';
					degitBal = degitBal - 1;
				}
				if (tempBalanceTwo  >0 && degitBal == 2){
					numBal[degitBal]=',';
					degitBal = degitBal - 1;
				}
			}
			
			printf("%s", numBal);
			
		}
		else {
			if (thisType == '+') {
				printf("%s"," ?,???,???.?? ");
			}
			if (thisType == '-') {
				printf("%s"," (?,???,???.??) ");
			}
		}
        
		printf("%s"," |");
    
    printf("\n");
    
    }
    
}

static
void process(FILE *fp, My402List *aList) {
    char buf[MAXIMUM_LINE_LENGTH]; 
    int coun = 0;
        while(fgets(buf, sizeof(buf), fp) != NULL) {
            if (buf[strlen(buf)-1] == '\n') { 
                buf[strlen(buf)-1] = '\0';
            }
            My402TransactionFields* transac = (My402TransactionFields*)malloc(sizeof(My402TransactionFields));
            if(strlen(buf) > (MAXIMUM_LINE_LENGTH - 2)) { 
                fprintf(stderr, "Error: There are at most 1024 characters in each line in the tfile. \n");
                exit(1);
            }
            int numTab = 0;
            int i;
            for (i = 0; i < strlen(buf); i++) {
                if(buf[i] == '\t') 
                    numTab++;
            }
            if (numTab > 3) {
                fprintf(stderr, "Error: The input file is not in the right format, the tfile can not have more than 3 tabs, should have exactly 3 tabs. \n");
                exit(1);
            }
            if (numTab < 3) {
                fprintf(stderr, "Error: The input file is not in the right format, the tfile can not have fewer than 3 tabs, should have exactly 3 tabs. \n");
                exit(1);
            }
            char *start_ptr = buf;
            char *tab_ptr = strchr(start_ptr, '\t');  

            if (tab_ptr != NULL) {
                *tab_ptr++ = '\0';  
            }
            else {
                fprintf(stderr, "Error: The second (timestamp) argument is empty, wrong tfile format. \n");
                exit(1);
            }
            
            if (start_ptr[0] != '+' && start_ptr[0] != '-') {
                fprintf(stderr, "Error: The transaction type should be either + or - in the tfile. \n");
                exit(1);
            }
            else {
                if (start_ptr[0] == '+') {
                    transac->type = '+';   
                }
                else if (start_ptr[0] == '-') {
                    transac->type = '-';
                }
            }
            
            start_ptr = tab_ptr;   
            tab_ptr = strchr(start_ptr, '\t');
            
            if (tab_ptr != NULL) { 
                *tab_ptr++ = '\0';
            }
            else {
                fprintf(stderr, "Error: The third (transaction amount) argument is empty, wrong tfile format. \n");
                exit(1);
            }
            
            int maxTimeStamp = 11;
            if(strlen(start_ptr) >= maxTimeStamp) {
                fprintf(stderr, "Error: The value of the timestamp must be between 0 to 4,294,967,295; the length can not be more than or equal to 11. \n");
                exit(1);
            }
            
            int intTime = atoi(start_ptr);
            time_t transTime = intTime; 
            time_t currTime = time(NULL);
            
            if(transTime > currTime) {
                fprintf(stderr, "Error: The value of the transcation time can not be greater than current time. \n");
                exit(1);
            }
            
            if (transTime < 0) {
                fprintf(stderr, "Error: The value of the transcation time can not be negative. \n");
                exit(1);
            }
            
            transac->time = transTime;
            
            start_ptr = tab_ptr;
            tab_ptr = strchr(start_ptr, '\t');
            
            if (tab_ptr != NULL) {
                *tab_ptr++ = '\0';
            }
            else {
                fprintf(stderr, "Error: The fourth (transcation description) argument is empty, wrong tfile format. \n");
                exit(1);
            }
            
            int numDigit = 0;
            int numDecimal = 0;
            int p;
            int stopCount = 0;
            
            for (p = 0; p < strlen(start_ptr); p++) {
                if (stopCount == 1) {
                    numDecimal += 1;
                }
                if (start_ptr[p] == '.') {
                    stopCount = 1;
                }
                if (stopCount == 0) {
                    numDigit += 1;
                }
            }
            
            if (numDigit > 7) {
                fprintf(stderr, "Error: The number to the left of the decimal point can be at most 7 digits (i.e., < 10,000,000). \n");
                exit(1);
            }
            
            if (numDecimal != 2) {
                fprintf(stderr, "Error: The number to the right of the decimal point can only be exactly 2 digits. \n");
                exit(1);
            }
            
            char * tempPeriodToken = strtok(start_ptr, "."); 
            int tranAmountInteger = atoi(tempPeriodToken);
            
            if (tranAmountInteger < 0) {
                fprintf(stderr, "Error: The transaction amount must have a positive value. \n");
                exit(1);
            }
            
            tempPeriodToken = strtok(NULL, ".");
            int tranAmountDecimal = atoi(tempPeriodToken);
            transac->amount = (100 * tranAmountInteger + tranAmountDecimal);
            
            start_ptr = tab_ptr;
            tab_ptr = strchr(start_ptr, '\t');
            
            if (tab_ptr != NULL) {
                fprintf(stderr, "Error: Can not have more than 4 arguments, must have exactly 4 arguments. \n");
                exit(1);
            }
            
            if (start_ptr == NULL) {
                fprintf(stderr, "Error: The transcation description cannot be empty. \n");
                exit(1);
            }
            
            char * numChar = NULL;
            size_t length = 0;
            size_t maxLength = 25;   
            numChar = malloc(maxLength);
            strcpy(numChar, start_ptr);
            
            while(isspace(*numChar)) {
                numChar++;
            }
            
            length = strlen(numChar);
            
            if (length == 0 ) { 
                fprintf(stderr, "Error: The transaction description can not be empty after removing leading space characters. \n");
                exit(1);
            }
            
            transac->description = strdup(numChar);
            My402ListAppend(aList, transac);
            coun = coun + 1;
        }
        if (coun == 0) {
            fprintf(stderr, "Error: The file is empty. \n");
            exit(1);
        } 
    
}


void sorting(My402List *aList){

    My402TransactionFields *dummy = NULL;
	My402ListElem *location, *currTransac, *nextTransac = NULL;
	
    for (currTransac = My402ListFirst(aList); currTransac != My402ListLast(aList); currTransac = My402ListNext(aList, currTransac))  {
        location = currTransac;
        for (nextTransac = My402ListNext(aList, currTransac); nextTransac != NULL; nextTransac = My402ListNext(aList, nextTransac))  {
            if (((My402TransactionFields *)nextTransac->obj)->time == ((My402TransactionFields*)location->obj)->time) {
                printf("Error: Two timestamp can not have the same value. \n");
                exit(1);
            }
            if (((My402TransactionFields *)nextTransac->obj)->time < ((My402TransactionFields *)location->obj)->time) {
                location = nextTransac;
            }
        }
		
        dummy = (currTransac->obj);
        (currTransac->obj) = (location->obj);
        (location->obj) = dummy;
    }
	return;
}
 
int main(int argc, char *argv[]) {
    struct stat goThrough;
    if (argc > 3) { 
        fprintf(stderr, "Error: Malformed command, too many arguments, the currect syntax is: warmup1 sort [tfile], tfile is optional. \n");
        exit(1);
    }
    else if (argc < 2) {
        fprintf(stderr, "Error: Malformed command, too few arguments, the currect syntax is: warmup1 sort [tfile], tfile is optional. \n");
        exit(1);
    }
    else {  
        My402List* aList = (My402List*)malloc(sizeof(My402List)); 
        if (strcmp(argv[1], "sort") != 0) { 
            fprintf(stderr, "Error: Malformed command, the second argument should be: sort. \n");
            exit(1);   
        }
        FILE *fp = NULL;
        if (argc == 2) {   
            fp = stdin;          
        }
        else { 
            stat(argv[2], &goThrough);
            fp = fopen(argv[2] , "r");
            if (S_ISDIR(goThrough.st_mode)) {
                fprintf(stderr, "Error: The file ");
                fprintf(stderr, "%s", argv[2]);
                fprintf(stderr, " is a directory. \n");
                exit(1);			 
			}
            if (fp == NULL) { 
                fprintf(stderr, "%s", strerror(errno));
                fprintf(stderr, "%s", argv[2]);
                fprintf(stderr, "%s", "\n");
                exit(1);
            }
            
        }
        process(fp,aList);
        fclose(fp);
        sorting(aList);
		printf("+-----------------+--------------------------+----------------+----------------+\n");
		printf("|       Date      | Description              |         Amount |        Balance |\n");
		printf("+-----------------+--------------------------+----------------+----------------+\n");
        int balance = 0;
        printList(aList,balance);     
        fprintf(stdout,"+-----------------+--------------------------+----------------+----------------+\n");   
		free(aList);
    }
    return 0;    
}
