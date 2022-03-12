#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>

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
    char txt;

    n = fopen(file, "r");

    if(n == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    txt = fgetc(n);

    while(txt != EOF){
		printf("%c", txt);
        txt = fgetc(n);
	}

	fclose(n);
    printf("\n");
}

int background(int argc, char **argv, int *bg){
    int i;
    *bg = 0;

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "and") == 0){
            *bg += 1;
        }
    }
    return *bg;
}

void redirection(int argc, char **argv){
    const char *error_message = "myshell: error, please try again.\n";
    int i;
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
            dup2(in_fd, 0);
            close(in_fd);
            read_file(argv[i+1]);
            fflush(stdin);
        }

        if(strcmp(argv[i], "out")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if(out_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            dup2(out_fd, 1);
            close(out_fd);
            recursive_dir(".");
            fflush(stdout);
        }

        if(strcmp(argv[i], "app")== 0){
            out_fd = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT, 0777);
            if(out_fd == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }
            dup2(out_fd, 1);
            close(out_fd);
            recursive_dir(".");
            fflush(stdout);
        }
    }
    
    dup2(stdInSave, 0);
    dup2(stdOutSave, 1);
    close(stdInSave);
    close(stdOutSave);
    printf("myshell: redirection executed.\n");
}

void pipe_func(int argc, char **argv){
    const char *error_message = "myshell: error, please try again.\n";
    int fd[2];
    int pid, i, bg = 0;
    
    background(argc, argv, &bg);

    for(i=0; i<argc; i++){
        if(strcmp(argv[i], "pipe") == 0){
            if(pipe(fd) == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
            }

            if(pipe(fd) == 0){
                pid = fork();

                if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            exit(1);
                }

                else if(pid == 0){
		            close(1);
		            dup2(fd[1], 1);
		            close(fd[0]);
                    close(fd[1]);

		            char *args[] = {"ls", "-la", NULL};
		            execvp(args[0], args);
                }
                else{
	                pid = fork();

                    if(pid == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
	                    exit(1);
                    }

	                else if(pid == 0){
		                close(0);
		                dup2(fd[0], 0);
		                close(fd[1]);
                        close(fd[0]);

		                char *args[] = {"grep", argv[i+2], NULL};
		                if(execvp(args[0], args)==-1){
			                write(STDERR_FILENO, error_message, strlen(error_message));
	                        exit(1);
		                }
	                }
	                else{
                        if(bg == 0){
                            close(fd[0]);
                            close(fd[1]);
                            waitpid(pid, NULL, 0);
                            printf("myshell: pipe has been executed\n");
                        }
                        if(bg > 0){
                            close(fd[0]);
                            close(fd[1]);
                            printf("myshell: process running in the background.\n");
                        }
                    }
                }
            }
        }
    }
}