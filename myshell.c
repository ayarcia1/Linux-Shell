#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
void recursive_dir(char *pathname);
void redirection(int argc, char **argv);

int main(int argc, char **argv, char **envp){
    const char *error_message = "myshell: error, please try again.\n";
    int i;

    if(argc<2){
        write(STDERR_FILENO, error_message, strlen(error_message));
	    exit(1);
    }

    if (strcmp(argv[1], "cd")== 0){
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
            n = fopen("readme_doc", "r");

            if(n == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }

            while(getline(&txt, &size, n)!= -1){
		        printf("%s",txt);
	        }

	        fclose(n);
            printf("\n");
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
        }
    }

    if(strcmp(argv[1], "pause")== 0){
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

    if(strcmp(argv[1], "ls")== 0 && argc >= 4){
        for(i=1; i<argc; i++){
            if(strcmp(argv[i], "out")== 0 || strcmp(argv[i], "app")== 0){
                redirection(argc, argv);
            }
        }
    }
    
    if(strcmp(argv[1], "cat")== 0 && argc >= 4){
        for(i=1; i<argc; i++){
            if(strcmp(argv[i], "in")== 0){
                redirection(argc, argv);
            }
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

void redirection(int argc, char **argv){
    const char *error_message = "myshell: error, please try again.\n";
    int i, in = 0, out = 0; 
    int in_fd, out_fd;
    int stdOutSave = dup(0);
    int stdInSave = dup(1);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "in")== 0){
            in_fd = open(argv[i+1], O_RDONLY);
            if(in_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            in++;
        }
        if(strcmp(argv[i], "out")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT);
            if(out_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            out++;
        }
        if(strcmp(argv[i], "app")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT);
            if(out_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            out++;
        }
    }
    if(in == 1){
        dup2(in_fd, 0);
        close(in_fd);
        printf("myshell: input redirection executed.\n");
        fflush(stdin);
        in--;
    }
    if(out == 1){
        dup2(out_fd, 1);
        close(out_fd);
        recursive_dir(".");
        fflush(stdout);
        out--;
    }
    if(in > 1 || out > 1){
        write(STDERR_FILENO, error_message, strlen(error_message));
	    exit(1);
    }
    dup2(stdInSave, 0);
    dup2(stdOutSave, 1);
    close(stdInSave);
    close(stdOutSave);
    printf("myshell: redirection executed\n");
}