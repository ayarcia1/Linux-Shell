#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "myshell.h"
void recursive_dir(char *pathname);

int main(int argc, char **argv, char **envp){
    const char *error_message = "myshell: error, please try again.\n";

    if (strcmp(argv[1], "cd")== 0){
        if(argc == 2){
            char path[100];
            printf("%s\n", getcwd(path, sizeof(path)));
        }
        else if(argc == 3){
            chdir(argv[2]);
            printf("directory has been changed to %s\n", argv[2]);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "clr")== 0){
        if(argc==2){
            printf("\e[1;1H\e[2J");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "dir")== 0){
        DIR *directory;

        if(argc==2){
            directory = opendir(".");
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            printf("[Current Directory]\n");
            recursive_dir(".");
        }
        else if(argc==3){
            directory = opendir(argv[2]);
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            printf("%s/n", argv[2]);
            recursive_dir(argv[2]);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "path")== 0){
        if(argc>=3){
            int i;
            for(i=2; i<argc; i++){
                setenv("PATH", argv[i], 1);
            }
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "environ")== 0){
        int i;
        for(i = 0; envp[i]; i++){
            printf("%s\n", envp[i]);
        }
    }

    if(strcmp(argv[1], "echo")== 0){
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
	        exit(1);
        }
    }

    if(strcmp(argv[1], "help")== 0){
        if(argc==2){
            FILE *n;
            char *txt;
            size_t size;
            n = fopen("readme_doc.txt", "r");

            if(n == NULL){
                printf("error\n");
                exit(0);
            }

            while(getline(&txt, &size, n)!= -1){
		        printf("%s",txt);
	        }

	        fclose(n);
            printf("\n");
            return 0;
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "pause")== 0){
        if(argc==2){
            printf("system has been paused, press enter to continue.\n");
            getchar();
            printf("system has been resumed.\n");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "quit")== 0){
        if(argc==2){
            exit(0);
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }
    return 0;
}

void recursive_dir(char *pathName){
    char path[1024];
    struct dirent *files;
    DIR *directory;
    directory = opendir(pathName);
    
    if(directory == NULL){
        return;
    }

    while ((files = readdir(directory)) != NULL){
        if (strcmp(files->d_name, ".") != 0 && strcmp(files->d_name, "..") != 0){
            printf("%s\n", files->d_name);

            strcpy(path, pathName);
            strcat(path, "/");
            strcat(path, files->d_name);
            
            recursive_dir(path);
        }
    }
    closedir(directory);
}