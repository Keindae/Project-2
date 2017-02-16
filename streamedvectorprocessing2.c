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

int main(int argc, char **argv){
    int i;
    //First pipe between process 0 and 1
    int pipe1[2];

    //Second Pipe between process 1 and 2
    int pipe2[2];

    //Create File
    char *file;
    file = argv[1];
    char *file2;
    file2 = argv[2];

    //The input files getting from the user from the command l;

    //Create a buffer, with the user inputting the size. Atoi converts it to an interger.
    int linebuffer = atoi(argv[3]);
    
    //Checks to make sure that the line buffer was created correctly. This allows for my project to be scalabe, sort of?
    fprintf(stderr, "Line Buffer Size: %d\n", linebuffer);

    //Creates an array of childpids that are all originally set to null.
    pid_t childpid[3];// = {NULL, NULL, NULL};


    //This is used to check if either of the pipes are bad
    pipe(pipe1);
    pipe(pipe2);

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
                           close(pipe1[0]);
                           close(pipe2[1]);
                           close(pipe2[0]);
                           FILE *inputFileA;
                           inputFileA = fopen(file, "r");
                           char line[linebuffer + 1];
                           line[linebuffer] = '\0';
                           char onescomp[linebuffer +1];
                           onescomp[linebuffer] = '\0';
                           bzero(onescomp, strlen(onescomp));
                           while(1){
                               //linebuffer +3 because was out of ideas and was on verge of insanity.
                               fgets(line, linebuffer + 3, inputFileA);
                               if(feof(inputFileA)){
                                   break;
                               }
                               //This strips the extra ending characters before doing any of the work
                               //and writing it to the pipe. 
                               line[strcspn(line, "\n\r")] = 0;
                               fprintf(stderr, "Comp Original Line Read: %s\n", line);
                               int count;
                               for(count = 0; count < linebuffer + 1; count++){
                                   if(line[count] == '1'){
                                       onescomp[count] = '0';
                                   }else if(line[count] == '0'){
                                       onescomp[count] = '1';
                                   }else{
                                   }
                               }
                               //onescomp[count +1] = '\n';
                               //Completes the One comp, and then prints it out, and puts it to the pipe.
                               fprintf(stderr, "Comp completion of Ones Comp: %s\n",onescomp);  
                               write(pipe1[1], onescomp, sizeof(onescomp)+1);
                           }
                           fclose(inputFileA);
                           close(pipe1[1]);
                           exit(0);
                       }
                       //Process 2 - Does the increment on 1's complimented number. 
                case 1:{
                           signal(SIGINT, assignment);
                           pause();
                           close(pipe1[1]);
                           close(pipe2[0]);
                           char line[linebuffer +1];
                           while(1){
                               int reading = read(pipe1[0],line,sizeof(line)+1);
                               if(reading == 0){
                                   break;
                               }
                               fprintf(stderr, "Incrementor Reading from Pipe 1, after Complement: %s\n", line);
                               //Starting at the right most bit every time.
                               //Carry of 1 to start the +1 addition.
                               for(i = linebuffer -1; i >= 0; i--){
                                   //Check to see if the character I are looking at is 1, 
                                   //if it is then flip it to 0, and maintain the carry of 1/
                                   if(line[i] == '0'){
                                       line[i] = '1';
                                       break;
                                    //Else, if the character is 0, then the carry of one just gets dropped into its place
                                    //and the 0 becomes a 1, and the carry is changed to 0.
                                   }else if(line[i] == '1'){
                                       line[i] = '0';
                                   }
                               }
                               //Write to the last pipe, and then zero out the line to get ready for the next iteration through the loop. 
                               //line[1 + linebuffer] = '\n';
                               fprintf(stderr, "Incrementor Writing to the Pipe after Increment: %s\n", line);
                               write(pipe2[1], line, sizeof(line)+1);
                           }
                           //exit the process
                           close(pipe1[0]);
                           close(pipe2[1]);
                           exit(0);
                       }
                    //Adds two binary values together, and writes to a file.
                case 2:{
                           signal(SIGINT, assignment);
                           pause();
                           close(pipe1[1]);
                           close(pipe1[0]);
                           close(pipe2[1]);
                           FILE *inputFileB;
                           inputFileB = fopen(file2,"r");
                           char line[linebuffer +1];
                           char pipeline[linebuffer +1];
                           int i;
                           int carry;
                           FILE *endingfile;
                           endingfile = fopen("Outputfile.txt", "w");
                           //Generic while loop to get us rolling. 
                           while(1){
                               fgets(line, linebuffer + 3, inputFileB);
                               if(feof(inputFileB)){
                                       break;
                               }
                               fprintf(stderr, "Binary Adder Reading from the Text File: %s\n", line);
                               //This will read in from the pipe
                               read(pipe2[0], pipeline, sizeof(pipeline)+1);
                               fprintf(stderr, "Binary Adder Reading from the Pipe: %s\n", pipeline);
                               //Here there is no carry initialized.
                               carry = 0;
                               //Starts with the right most bit first, and then moves on keeping track of the carry.
                               for(i = linebuffer -1; i >= 0; i--){
                                   //If they are both equal to one, we generate a carry and flip
                                   //the 1 to a zero
                                   if(line[i] == '1' && (line[i] == pipeline[i] && carry == 0)){
                                       pipeline[i] = '0';
                                       carry = 1;
                                    //If it is 1, and both positions in each array equal each other, and the carry is 1
                                    //then the position in addednum array becomes 1, and there is still a carry generated.
                                   }else if(line[i] == '1' && (line[i] == pipeline[i] && carry == 1)){
                                       pipeline[i] = '1';
                                       carry = 1;
                                    //This checks to see if the position is a 0, and if it is equal in both array's, 
                                    //and the carry is 1, then the 0 gets flipped to 1, thanks to the carry, and 
                                    //then the carry is set to 0. 
                                   }else if(line[i] == '0' && (line[i] == pipeline[i] && carry == 1)){
                                       pipeline[i] = '1';
                                       carry = 0;
                                    //Checks the position, if it is a 0, and the carry is a 0, then nothing happens,
                                    //the position stays a 0 and the carry remains 0.
                                   }else if(line[i] == '0' && (line[i] == pipeline[i] && carry == 0)){
                                       pipeline[i] = '0';
                                       carry = 0;
                                    //Checking to see if either position is a 1 or 0, and if the carry is a 1, 
                                    //then the position in the addednum is updated to a 0, and the carry remains 1.
                                   }else if((line[i] != pipeline[i]) && carry == 1){
                                       pipeline[i] = '0';
                                       carry = 1;
                                    //Similar condition as above, but if the carry is 0, then the position in addednum gets
                                    //set to 1, and the carry remains 0.
                                   }else if((line[i] != pipeline[i]) && carry == 0){
                                       pipeline[i] = '1';
                                       carry = 0;
                                   }else{
                                   }
                               }
                               //Write the line into the outputfile, and then bzero out the line for the next iteration throguh. 
                               
                               fprintf(stderr, "Binary Adder Addednum Total: %s\n", pipeline);
                               fprintf(endingfile, "%s\n", pipeline);
                               bzero(pipeline, sizeof(pipeline));
                           }
                           //end of the while, close out the file, and exit the process. 
                           fclose(endingfile);
                           fclose(inputFileB);
                           close(pipe2[0]);
                           exit(0);
                       }
                default:{
                    break;
                        }
            }

        }
    }
    //Used to wait for all three children to return after finishing their process
    for(i = 0; i <= 2; i++){
        wait(NULL);
    }
    return 0;
}

