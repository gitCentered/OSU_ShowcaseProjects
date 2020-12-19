/********************************************************************
 * Program : smallsh
 * Author  : Will Geller
 * Due Date: 7/21/2020
 * Description: Source code for smallsh
 *******************************************************************/

// SOURCE: https://brennan.io/2015/01/16/write-a-shell-in-c/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_BUFF_SIZE 2048
#define ARG_BUFF_SIZE 512

// Global variable for signal handler
int foregroundMode = 0;

// Struct for shell variables
struct shell{
    int argCount;
    int childCount;
    int shellStatus;
    int exitStatus;
    int background;
    int foreground;
    int redirectIn;
    int redirectOut;
    int* childProc;
    char* inFile;
    char* outFile;
};

/********************************************************************
 * Add child process number to child process array
 *******************************************************************/
void addChildProc(int childPID, struct shell* vars){
    vars->childProc[vars->childCount++] = childPID;
}

/********************************************************************
 * Remove child process number from child process array
 *******************************************************************/
void removeChildProc(int childPID, struct shell* vars){
    int i;
    int found = 0;
    int index = 0;
    while(!found && index < vars->childCount){
        if(vars->childProc[i] == childPID){
            found = 1;
        }
        else{
            index++;
        }
    }
    for(i = index; i < vars->childCount - 1; i++){
        vars->childProc[i] = vars->childProc[i + 1];
    }
    vars->childCount--;
}

/********************************************************************
 * Kill all child processes and free allocated memory of child
 * process array
 *******************************************************************/
void killChildProc(struct shell* vars){
    pid_t wpid;
    int status;
    int i;
    for(i = 0; i < vars->childCount; i++){
        kill(vars->childProc[i], SIGTERM);
    }
    free(vars->childProc);
}

/********************************************************************
 * Frees allocated memory and resets shell vars
 *******************************************************************/
void cleanUp(char* buffer, char** args, struct shell* vars){
    
    int i;
    for(i = 0; i < vars->argCount; i++){
        args[i] = NULL;
    }    
    free(args);
    free(buffer);
    
    vars->argCount = 0;
    vars->background = 0;
    vars->redirectIn = 0;
    vars->redirectOut = 0;
    vars->inFile = NULL;
    vars->outFile = NULL;
}

/********************************************************************
 * Handle SIGSTP Signal to toggle foregroundMode
 *******************************************************************/
void catchSIGTSTP(int signo){
    if(foregroundMode){
        printf("\nExiting foreground-only mode\n");
        write(STDOUT_FILENO, ": ", 2);
        foregroundMode = 0;
    }
    else{
        printf("\nEntering foreground-only mode (& is ignored)\n");
        write(STDOUT_FILENO, ": ", 2);
        foregroundMode = 1;
    }
}

/********************************************************************
 * Checks for terminated background processes and display message
 *******************************************************************/
void checkBackground(struct shell* vars){
    int childStatus;
    
    // Check for terminated child processes
    pid_t cpid = waitpid(WAIT_ANY, &childStatus, WNOHANG);
    
    // Display appropriate prompt for terminated child process
    while(cpid > 0){
        if(WIFEXITED(childStatus)){
            vars->exitStatus = WEXITSTATUS(childStatus);
            printf("background pid %d is done: exit value %d\n", cpid, WEXITSTATUS(vars->exitStatus));
        }
        else{
            vars->exitStatus = WTERMSIG(childStatus);
            printf("background pid %d is done: terminated by signal %d\n", cpid, WTERMSIG(vars->exitStatus));
        }
       
        // Remove child process from child process array
        removeChildProc(cpid, vars);

        // Check for other child processes that have terminated
        cpid = waitpid(WAIT_ANY, &childStatus, WNOHANG);
    }
}

/********************************************************************
 * Returns false if blank or comment, otherwise true
 *******************************************************************/
int isArgument(char* arg){
    if(arg == NULL || *arg == '#'){
        return 0;
    }
    else{
        return 1;
    }
}

/********************************************************************
 * Returns true if built in function, otherwise false
 *******************************************************************/
int isBuiltIn(char* arg){
    if(strcmp(arg, "cd") == 0 || strcmp(arg, "status") == 0 || strcmp(arg, "exit") == 0){
        return 1;
    }
    else{
        return 0;
    }
}

/********************************************************************
 * Change directories
 *******************************************************************/
void changeDirectory(char** args){
    // If there is not an argument, go to home directory
    if(args[1] == NULL){
        chdir(getenv("HOME"));
    }
    // Otherwise change directory
    else {
        if(chdir(args[1]) != 0){
            printf("%s: directory not found\n", args[1]);
        }
    }
}

/********************************************************************
 * Kills all child processes and sets shellStatus to 0 for exit
 *******************************************************************/
void exitShell(struct shell* vars){ 
    killChildProc(vars);
    vars->shellStatus = 0;
}

/********************************************************************
 * Display status of last terminated system process
 *******************************************************************/
void getStatus(struct shell* vars){ 
    if(WIFEXITED(vars->exitStatus)){
        printf("exit value %d\n", WEXITSTATUS(vars->exitStatus));
    }
    else{
        printf("terminated by signal %d\n", WTERMSIG(vars->exitStatus));
    }
}

/********************************************************************
 * Executes built in functions and returns status
 *******************************************************************/
void builtInFx(char** args, struct shell* vars){
    if(strcmp(args[0], "cd") == 0){
        changeDirectory(args);
    }
    else if(strcmp(args[0], "status") == 0){
        getStatus(vars);
    }
    else if(strcmp(args[0], "exit") == 0){
        exitShell(vars);
    }
}

/********************************************************************
 * Redirect Foreground input/output
 *******************************************************************/
void foreground(char** args, struct shell* vars){
    // Redirect input to file
    if(vars->redirectIn == 1){
        int srcFD = open(vars->inFile, O_RDONLY);
        if(srcFD == -1){
            printf("cannot open %s for input\n", vars->inFile);
            exit(EXIT_FAILURE);
        }

        else{
            if(dup2(srcFD, STDIN_FILENO) == -1){
                perror("smallsh");
                exit(EXIT_FAILURE);
            }
            close(srcFD);
        }
    }
    // Redirect output to file
    if(vars->redirectOut == 1){
        int targetFD = open(vars->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(targetFD == -1){
            printf("cannot open %s for output\n", vars->outFile);
            exit(EXIT_FAILURE);
        }

        else{
            if(dup2(targetFD, STDOUT_FILENO) == -1){
                perror("smallsh");
                exit(EXIT_FAILURE);
            }
            close(targetFD);
        }
    }
}

/********************************************************************
 * Redirect Background input/output
 *******************************************************************/
void background(char** args, struct shell* vars){
    int srcFD;
    int targetFD;
    
    // Redirect input to file
    if(vars->redirectIn == 1){
        srcFD = open(vars->inFile, O_RDONLY);
        if(srcFD == -1){
            printf("cannot open %s for input\n", vars->inFile);
            exit(EXIT_FAILURE);
        }
    }
    // Redirect input to /dev/null
    else{
        srcFD = open("/dev/null", O_RDONLY);
        if(srcFD == -1){
            printf("cannot open /dev/null for input\n");
            exit(EXIT_FAILURE);
        }
    }

    if(dup2(srcFD, STDIN_FILENO) == -1){
        perror("smallsh");
        exit(EXIT_FAILURE);
    }
    close(srcFD);

    // Redirect output to file
    if(vars->redirectOut == 1){
        targetFD = open(vars->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(targetFD == -1){
            printf("cannot open %s for output\n", vars->outFile);
            exit(EXIT_FAILURE);
        }
    }
    // Redirect output to /dev/null
    else{
        targetFD = open("/dev/null", O_WRONLY);
        if(targetFD == -1){
            printf("cannot open /dev/null for output\n");
            exit(EXIT_FAILURE);
        }
    }

    if(dup2(targetFD, STDOUT_FILENO) == -1){
        perror("smallsh");
        exit(EXIT_FAILURE);
    }
    close(targetFD);
}

/********************************************************************
 * Execute command line input via child process
 *******************************************************************/
void execute(char** args, struct shell* vars, struct sigaction sigintAction){
    int childStatus;
    pid_t wpid;
    pid_t cpid = fork();

    if(cpid < 0){
        perror("smallsh");
    }
    // Child process...
    else if(cpid == 0) {
        
        // Foreground-only mode is on
        if(foregroundMode == 1){
            sigintAction.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sigintAction, NULL);
            
            foreground(args, vars);
        }
        // Child process executes in background
        else if(vars->background == 1){
            background(args, vars);
        }
        // Child process executes in foreground
        else{
            sigintAction.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sigintAction, NULL);
            
            foreground(args, vars);           
        }
        
        // Execute the command line arguments
        if(execvp(args[0], args) == -1) {
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    // Parent process...
    else{
        if(foregroundMode == 0 && vars->background == 1){
            addChildProc(cpid, vars);
            printf("background pid is %d\n", cpid);
        }
        else{
            wpid = waitpid(cpid, &childStatus, WUNTRACED);
            vars->exitStatus = childStatus;
 
            if(WIFSIGNALED(vars->exitStatus)){
                printf("terminated by signal %d\n", WTERMSIG(vars->exitStatus));
            }
        }
    }
}

/********************************************************************
 * Get arguments from command line input
 *******************************************************************/
char** getArgs(char* buffer, struct shell* vars){
    char* token;
    char** args = malloc(ARG_BUFF_SIZE * sizeof(char*));

    if(buffer == NULL){
        return NULL;
    }

    // Loop until all arguments are gathered
    token = strtok(buffer, " \n\r\t\a");
    while(token != NULL){
        if(strcmp(token, "<") == 0){
            vars->redirectIn = 1;
            token = strtok(NULL, " \n\r\t\a");
            if(token == NULL){
                printf("smallsh: missing input filename\n");
                return NULL;
            }
            else{
                vars->inFile = token;
            }
        }
        else if(strcmp(token, ">") == 0){
            vars->redirectOut = 1;
            token = strtok(NULL, " \n\r\t\a");
            if(token == NULL){
                printf("smallsh: missing output filename\n");
                return NULL;
            }
            else{
                vars->outFile = token;
            }
        }
        else{
            args[vars->argCount++] = token;
        }

        if(vars->argCount > ARG_BUFF_SIZE){
            printf("smallsh: number of arguments exceeded buffer\n");
            return NULL;
        }
        token = strtok(NULL, " \n\r\t\a");
    }

    // Check for &, the background process argument
    if(vars->argCount > 1 && strcmp(args[vars->argCount - 1], "&") == 0){
        vars->background = 1;
        args[vars->argCount - 1] = NULL;
    }
    return args;
}

/********************************************************************
 * Get command line input by character
 *******************************************************************/
char* getCmdLine(){
    size_t buffSize = INPUT_BUFF_SIZE;
    char* buffer = malloc(INPUT_BUFF_SIZE * sizeof(char));
    int characters = getline(&buffer, &buffSize, stdin);
    
    if(characters == -1){
        printf("smallsh: error reading input\n");
        return NULL;
    }
    else if(characters > INPUT_BUFF_SIZE){
        printf("smallsh: input exceeded buffer size\n");
        return NULL;
    }
    else{
        buffer[characters - 1] = '\0';
        
        // Replace "$$" with pid
        while(strstr(buffer, "$$")){
            sprintf(strstr(buffer, "$$"), "%d", getpid());
        }
    }
    return buffer;
}

/********************************************************************
 * Initialize shell buffers, signal handlers, and status vars. Run 
 * loop for shell.
 *******************************************************************/
int main(){
    // Flush buffers
    fflush(stdin);
    fflush(stdout);
    
    // Create buffers and initialize shell vars
    char* buffer;
    char** args;
    struct shell* vars = malloc(sizeof(struct shell));
   
    // Set to ignore SIGINT
    struct sigaction sigintAction = {0};
    sigintAction.sa_handler = SIG_IGN;
    sigfillset(&sigintAction.sa_mask);
    sigintAction.sa_flags = 0;
    sigaction(SIGINT, &sigintAction, NULL);

    // Set to send SIGSTP to handler
    struct sigaction sigtstpAction = {0};
    sigtstpAction.sa_handler = catchSIGTSTP;
    sigfillset(&sigtstpAction.sa_mask);
    sigtstpAction.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sigtstpAction, NULL);

    // Initialize shell variables
    vars->argCount = 0;
    vars->childCount = 0;
    vars->shellStatus = 1;
    vars->exitStatus = 0;
    vars->background = 0;
    vars->redirectIn = 0;
    vars->redirectOut = 0;
    vars->childProc = malloc(20 * sizeof(int));
    vars->inFile = NULL;
    vars->outFile = NULL;
    
    while(vars->shellStatus){
        fflush(stdin);
        fflush(stdout);
        printf(": ");

        buffer = getCmdLine();
        args = getArgs(buffer, vars);

        if(isArgument(args[0])){
            if(isBuiltIn(args[0])){
                builtInFx(args, vars);
            }
            else{
                execute(args, vars, sigintAction);
            }
        }
        cleanUp(buffer, args, vars);
        checkBackground(vars);
    }
    free(vars);
    return EXIT_SUCCESS;
}