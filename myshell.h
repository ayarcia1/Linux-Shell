#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

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

void read_file(char *file){
    const char *error_message = "myshell: error, please try again.\n";
    FILE *n;
    char *txt;
    size_t size;
    n = fopen(file, "r");

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

void redirection(int argc, char **argv){
    const char *error_message = "myshell: error, please try again.\n";
    int i, in = 0, out = 0; 
    int in_fd, out_fd;
    char file[100];
    int stdOutSave = dup(0);
    int stdInSave = dup(1);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "in")== 0){
            strcpy(file, argv[i+1]);
            in_fd = open(argv[i+1], O_RDONLY);
            if(in_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            in++;
        }
        if(strcmp(argv[i], "out")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if(out_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            out++;
        }
        if(strcmp(argv[i], "app")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT, 0777);
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
        read_file(file);
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
    printf("myshell: redirection executed.\n");
}