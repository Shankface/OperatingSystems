#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int lastStatus = 0;

void doRedirect(char * redirects){
    char * token;
    token = strtok(redirects, " ");
    char * target;
    int original;
    int flags;
    int fd;

    while(token != NULL){
        flags = 0;
        target = (char*)malloc(strlen(token));
        switch(token[0]){
            case '<':
                original = STDIN_FILENO;
                strcpy(target,token + 1);
                flags = O_RDONLY;
                break;

            case '>':
                original = STDOUT_FILENO;
                if(token[1] == '>'){
                    flags = O_APPEND|O_CREAT|O_WRONLY;
                    strcpy(target,token + 2);
                }
                else {
                    flags = O_TRUNC|O_CREAT|O_WRONLY;
                    strcpy(target,token + 1);
                }
                break;

            case '2':
                original = STDERR_FILENO;
                if(token[2] == '>'){
                    flags = O_APPEND|O_CREAT|O_WRONLY;
                    strcpy(target,token + 3);
                }
                else {
                    flags = O_TRUNC|O_CREAT|O_WRONLY;
                    strcpy(target,token + 2);
                }
                break;

            default: fprintf(stderr,"Unknown symbol for I/O redirection\n"); exit(-1);
        }

        if((fd = open(target,flags,0666)) < 0){
            fprintf(stderr, "Error opening file '%s' during I/O redirection: %s\n", target, strerror(errno)); 
            exit(errno);
        }

        if(dup2(fd,original) < 0){
            perror("Error during dup2 during I/O redirection");
            exit(errno);
        }

        close(fd);
        free(target);
        token = strtok(NULL, " ");
    }
}


void cd(char * dir){
    char * direct;
    direct = strlen(dir) == 0 ? getenv("HOME") : dir;
    if(chdir(direct) != 0){
        perror("Error changing directory");
        lastStatus = errno;
    }
} 

void pwd(){
    char buf[PATH_MAX];
    if(getcwd(buf, PATH_MAX) != NULL) printf("%s\n", buf);
    else{ perror("Error printing directory"); lastStatus = errno;}
}

void exitS(char * args){
    char * status;
    if(strlen(args) != 0) status = args;
    else exit(lastStatus);
    int i;
    if(status[0] == '-'){fprintf(stderr, "Exit return code not valid integer\n"); return;}

    for(i = 0; i < strlen(status); i++){
        if(isdigit(status[i]) == 0){
            fprintf(stderr, "Exit return code not valid integer\n"); 
            return;
        }
    }
    exit(atoi(status));
}


void otherCommand(char* command, char* args, char* redirects, bool redirectExist){
    int pid;
    struct timeval stop,start;
    gettimeofday(&start,NULL);

    switch(pid = fork()){
        case -1:
            perror("Error with fork");
            exit(errno);

        case 0:
            if(redirectExist) doRedirect(redirects);
            
            char ** argv = (char**)malloc(strlen(command) + strlen(args)); 
            char * token;
            token = strtok(args, " ");
            argv[0] = command;
            int i = 1;
            while(token != NULL){
                argv[i] = token;
                i++;
                token = strtok(NULL, " ");
            }
            argv[i] = NULL;
            execvp(command, argv);
            perror("Error in execvp");
            exit(127);
            break;

        default: {
            struct rusage ru;
            int cpid;
            int status;

            if((cpid = wait3(&status,0,&ru)) < 0){
                perror("Error in wait3");
                exit(errno);
            }

            gettimeofday(&stop,NULL);
            suseconds_t real_micro = (stop.tv_sec - start.tv_sec)*1000000L + stop.tv_usec - start.tv_usec;
            time_t real_sec = real_micro/1000000;

            if (status!=0){
                if (WIFSIGNALED(status)){
                    printf("Child process %d exited with signal %d\n", cpid, WTERMSIG(status));
                    lastStatus = WTERMSIG(status);
                }
                else{
                    printf("Child process %d exited with return value %d\n", cpid, WEXITSTATUS(status));
                    lastStatus = WEXITSTATUS(status);
                }
            }
            else{
                lastStatus = 0;
                printf("Child process %d exited normally\nReal:%ld.%.6ds User:%ld.%.6ds Sys:%ld.%.6ds\n", pid, real_sec, real_micro, ru.ru_utime.tv_sec,  ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
            }
        }
    }
}

void parse(FILE * fp){
    char line[ARG_MAX];
    char * command;
    char * args;
    char * redirects;
    bool redirectExist = true;

    while(fgets(line, ARG_MAX, fp) != NULL){
        if(line[0] == '#' || line[0] == '\n') continue;
        int length = strlen(line);
        redirectExist = true;

        //isolates command
        int command_len = strchr(line, ' ') != NULL ? strchr(line, ' ') - line : length -1;
        command = (char *)malloc(command_len+1); 
        strncpy(command, line, command_len+1);
        if(command[command_len] == ' ' || command[command_len] == '\n') command[command_len] = '\0';


        //isolates arguments and redirects
        int io_pos;
        int stdin_pos = strchr(line, '<') != NULL ? strchr(line, '<') - line : length;
        int stdout_pos = strchr(line, '>') != NULL ? strchr(line, '>') - line : length;
        if((stdout_pos < strlen(line) - 1) && (line[stdout_pos - 1] == '2')) stdout_pos--;
        io_pos = stdin_pos < stdout_pos ? stdin_pos : stdout_pos;

        if(io_pos != length){
            redirects = (char *)malloc(length - io_pos -1);
            strncpy(redirects, line + io_pos, length - io_pos -1);
            args = (char *)malloc(io_pos-1 - command_len);
            strncpy(args, line + command_len + 1, io_pos-1 - command_len);
            args[io_pos-1 - command_len -1] = '\0';
        }
        else{
            args = (char *)malloc(io_pos-1 - command_len);
            strncpy(args, line + command_len + 1, io_pos-1 - command_len);
            args[io_pos-1 - command_len -1] = '\0';
            redirectExist = false;
        }

        if(strcmp(command, "cd") == 0) cd(args);
        else if(strcmp(command, "pwd") == 0) pwd(args);
        else if(strcmp(command, "exit") == 0) exitS(args);
        else otherCommand(command, args, redirects, redirectExist);

        free(command);
        if(io_pos != length){
            free(redirects);
            free(args);
        }
        else free(args);
    }
}

int main(int argc, char* argv[]){
    FILE * fp = (argc > 1) ? fopen(argv[1], "r") : stdin;
    if(fp == NULL){
        perror("Error opening file");
        exit(errno);
    }
    parse(fp);
    return lastStatus;