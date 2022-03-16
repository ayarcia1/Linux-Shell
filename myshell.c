//Name: Arif Ayarci
//Lab 2: Shell Program
//Date: 3/23/2022
//Section: CIS-3207-01
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include "myshell.h"
int built_in(int argc, char **argv, char **envp);
void parse_line(int *argc, char **argv);

int main(int argc, char **argv, char **envp){
    printf("Welcome to MyShell!\n");
    printf("MyShell: ");
    parse_line(&argc, argv);

    while(1){
        built_in(argc, argv, envp);
        redirection(argc, argv);
        pipe_func(argc, argv);
        printf("MyShell: ");
        parse_line(&argc, argv);
    }
}

void parse_line(int *argc, char **argv){
    int i = 1;
    *argc = 1;
    char *line;
    size_t size = 100;
    line = (char*) malloc (size);
    char **string = &line;

    getline(string, &size, stdin);
    char *token = strtok(line, " \n");

    while(token != NULL){
        argv[i] = token;
        i++;
        *argc+=1;
        token = strtok(NULL, " \n");
    }
}


int built_in(int argc, char **argv, char **envp){
    const char *error_message = "myshell: an error has occured.\n";

    if (strcmp(argv[1], "cd") == 0){
        if(argc == 2){
            char path[100];
            printf("%s\n", getcwd(path, sizeof(path)));
        }
        else if(argc == 3){
            chdir(argv[2]);
            printf("myshell: directory has been changed to %s\n", argv[2]);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "clr") == 0){
        if(argc==2){
            printf("\e[1;1H\e[2J");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "dir") == 0){
        DIR *directory;

        if(argc==2){
            directory = opendir(".");
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            printf("[Current Directory]\n");
            recursive_dir(argv, ".");
        }
        else if(argc==3){
            directory = opendir(argv[2]);
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            printf("%s\n", argv[2]);
            recursive_dir(argv, argv[2]);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "path") == 0){
        if(argc>=3){
            int i;
            for(i=2; i<argc; i++){
                setenv("PATH", argv[i], 1);
            }
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "environ") == 0){
        int i;
        for(i=0; envp[i]; i++){
            printf("%s\n", envp[i]);
        }
    }

    if(strcmp(argv[1], "echo") == 0){
        if(argc>=3){
            int i;
            for(i=2; i<argc; i++){
                printf("%s", argv[i]);
                printf(" ");
            }
            printf("\n");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "help") == 0){
        if(argc==2){
            read_file("readme_doc");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "pause") == 0){
        if(argc==2){
            while(1){
                printf("myshell: system has been paused, press enter to continue.\n");
                if(getchar()){
                    printf("myshell: system has been resumed.\n");
                    break;
                }
                else{
                    continue;
                }
            }
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    if(strcmp(argv[1], "exit") == 0){
        if(argc==2){
            exit(0);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }
    return 1;
}