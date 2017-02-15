//Matt Noblett
//Project 2 - Streamed Vector Processing 
//Wolffe GVSU - Winter 2017


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define READ 0
#define WRITE 1

void assignment(int num);

void assignment(int num){
    printf("\n%d is beinging assigned to a process.\n", getpid());
}

int main(){
    int i;
    //First pipe between process 0 and 1
    int pipe1[2];

    //Second Pipe between process 1 and 2
    int pipe2[2];

    //Create File
    FILE *file, *file2;
    
    //Creates an array of childpids that are all originally set to null.
    pid_t childpid[3] = {NULL, NULL, NULL};


    //This is used to check if either of the pipes are bad
    pipe(pipe1);
    pipe(pipe2);
    //if(pipe(pipe1) < 0 && pipe(pipe2) < 0){
    //    printf("One of the pipes are bad!!!\n");
    //    exit(0);
    //}

    printf("Parent PID: %d\n", getpid());

    //For loop to run through and create the required child processes. 
    for(i = 0; i <= 2; i++){
        if((childpid[i] = fork()) == 0){
            printf("Child %d spawned: %d\n", i, getpid());

            switch (i){

                //Reads in each line from a file, and then performs the ones compliment on it
                //After that, it will write it to the first pipe.
                case 0:{       
                           signal(SIGINT, assignment);
                           pause();
                           dup2(pipe1[1], STDOUT_FILENO);
                           close(pipe1[1]);
                           close(pipe1[0]);
                           close(pipe2[1]);
                           close(pipe2[0]);
                           file = fopen("input vector A.txt", "r");
                           char line[256];
                           while(fgets(line, sizeof(line), file) != NULL){
                               char onescomp[256];
                               bzero(onescomp, sizeof(onescomp));
                               int count = 0;
                               for(count = 0; count < strlen(line); count++){
                                   if(line[count] == '1'){
                                       onescomp[count] = '0';
                                   }else if(line[count] == '0'){
                                       onescomp[count] = '1';
                                   }else{
                                   }

                               }
                               onescomp[count +1] = '\n';
                               //Completes the One comp, and then prints it out, and puts it to the pipe.
                               //printf("Ones Comp: %s\n",onescomp);
                               write(STDOUT_FILENO, onescomp, sizeof(onescomp));
                               bzero(onescomp, sizeof(onescomp));
                           }
                           exit(0);
                       }
                       //Process 2 - Does the increment on 1's complimented number. 
                case 1:{
                           signal(SIGINT, assignment);
                           pause();
                           dup2(pipe1[0], STDIN_FILENO);
                           dup2(pipe2[1], STDOUT_FILENO);
                           close(pipe1[1]);
                           close(pipe1[0]);
                           close(pipe2[1]);
                           close(pipe2[0]);
                           char line[256];
                           //Used to increment by 1 regardless
                           int carry;
                           int count;
                           while(read(STDIN_FILENO, line, sizeof(line)) != EOF){
                               //read(STDIN_FILENO, line, sizeof(line));
                               carry = 1;
                               count = sizeof(line)-1;
                               //Starting at the right most bit every time.
                               //Carry of 1 to start the +1 addition.
                               while(count >= 0 && carry == 1){
                                   //Check to see if the character I are looking at is 1, 
                                   //if it is then flip it to 0, and maintain the carry of 1/
                                   if(line[count] == '1'){
                                       line[count] = '0';
                                       carry = 1;
                                    //Else, if the character is 0, then the carry of one just gets dropped into its place
                                    //and the 0 becomes a 1, and the carry is changed to 0.
                                   }else if(line[count] == '0' && carry == 1){
                                       line[count] = '1';
                                       carry = 0;
                                   }
                                   count--;
                               }
                               //Write to the last pipe, and then zero out the line to get ready for the next iteration through the loop. 
                               line[1 + sizeof(line)] = '\n';
                               write(STDOUT_FILENO, line, sizeof(line));
                               bzero(line, sizeof(line));
                           }
                           //exit the process
                           exit(0);
                       }
                    //Adds two binary values together, and writes to a file.
                case 2:{
                           signal(SIGINT, assignment);
                           pause();
                           dup2(pipe2[0], STDOUT_FILENO);
                           close(pipe1[1]);
                           close(pipe1[0]);
                           close(pipe2[1]);
                           close(pipe2[0]);
                           file2 = fopen("input vector B.txt","r");
                           char line[256];
                           char pipeline[256];
                           int i;
                           int carry;
                           int count;
                           char addednum[256];
                           FILE *endingfile;
                           endingfile = fopen("Outputfile.txt", "w");
                           //Generic while loop to get us rolling. 
                           while(1){
                               fgets(line, sizeof(line), file2);
                               //This is used to check and see if I am at the end of the file that I am reading in to "add" together
                               if(feof(line)){
                                   break;
                               }
                               //This will read in from the pipe
                               read(STDIN_FILENO, pipeline, sizeof(pipeline));
                               //Here there is no carry initialized.
                               carry = 0;
                               count = sizeof(line) - 1;
                               //Starts with the right most bit first, and then moves on keeping track of the carry.
                               for(i = sizeof(line)-1; i >= 0; i--){
                                   //If they are both equal to one, we generate a carry and flip
                                   //the 1 to a zero
                                   if(line[count] == '1' && (line[count] == pipeline[count] && carry == 0)){
                                       addednum[count] = '0';
                                       carry = 1;
                                    //If it is 1, and both positions in each array equal each other, and the carry is 1
                                    //then the position in addednum array becomes 1, and there is still a carry generated.
                                   }else if(line[count] == '1' && (line[count] == pipeline[count] && carry == 1)){
                                       addednum[count] = '1';
                                       carry = 1;
                                    //This checks to see if the position is a 0, and if it is equal in both array's, 
                                    //and the carry is 1, then the 0 gets flipped to 1, thanks to the carry, and 
                                    //then the carry is set to 0. 
                                   }else if(line[count] == '0' && (line[count] == pipeline[count] && carry == 1)){
                                       addednum[count] = '1';
                                       carry = 0;
                                    //Checks the position, if it is a 0, and the carry is a 0, then nothing happens,
                                    //the position stays a 0 and the carry remains 0.
                                   }else if(line[count] == '0' && (line[count] == pipeline[count] && carry == 0)){
                                       addednum[count] = '0';
                                       carry = 0;
                                    //Checking to see if either position is a 1 or 0, and if the carry is a 1, 
                                    //then the position in the addednum is updated to a 0, and the carry remains 1.
                                   }else if((line[count] != pipeline[count]) && carry == 1){
                                       addednum[count] = '0';
                                       carry = 1;
                                    //Similar condition as above, but if the carry is 0, then the position in addednum gets
                                    //set to 1, and the carry remains 0.
                                   }else if((line[count] != pipeline[count]) && carry == 0){
                                       addednum[count] = '1';
                                       carry = 0;
                                   }else{
                                       fprintf(stderr, "Massively Broken\n");
                                   }
                               }
                               //Write the line into the outputfile, and then bzero out the line for the next iteration throguh. 
                               write(endingfile, addednum, sizeof(addednum));
                               bzero(addednum, sizeof(addednum));
                           }
                           //end of the while, close out the file, and exit the process. 
                           fclose(endingfile);
                           exit(0);
                       }
                default:{
                    break;
                        }
            }

        }
    }

    //Used to wait for all three children to return after finishing their process
    for(i = 0; i < 3; i++){
        wait(NULL);
    }
}

