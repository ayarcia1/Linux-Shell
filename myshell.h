#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
char *recursive_dir(char *path_name);
void read_file(char *file);
int background(int argc, char **argv, int *bg);

int redirection(int argc, char **argv, char **envp, int *re){
    const char *error_message = "myshell: an error has occured.\n";
    int i, j, pid, bg = 0;
    int in_fd, out_fd;
    int stdOutSave = dup(0);
    int stdInSave = dup(1);
    *re = 0;

    background(argc, argv, &bg);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "<") == 0){
            *re += 1;

            if(argc < 4){
	            return 1;
            }

            pid = fork();
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }

            else if(pid == 0){
                in_fd = open(argv[i+1], O_RDONLY);
                if(in_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }

                dup2(in_fd, 0);
                close(in_fd);

                for(j=1; j<argc; j++){
                    if(strcmp(argv[j], ">") == 0 || strcmp(argv[j], ">>") == 0){
                        argv[j] = NULL;
                        break;
                    }
                }

                argv[i] = NULL;
                argv[argc] = NULL;

                execvp(argv[1], &argv[1]);
                if(execvp(argv[1], &argv[1]) == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                if(argv[j] != NULL){
                    fflush(stdin);
                }
            }
        }

        if(strcmp(argv[i], ">") == 0){
            *re += 1;

            if(argc < 4){
	            return 1;
            }

            pid = fork();
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }

            else if(pid == 0){
                out_fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                if(out_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }

                dup2(out_fd, 1);
                close(out_fd);

                if(strcmp(argv[1], "dir") == 0){
                    recursive_dir(".");
                }

                else if(strcmp(argv[1], "echo") == 0){
                    if(strcmp(argv[2], ">") == 0){
                        printf("\n");
                    }
                    if(strcmp(argv[2], ">") != 0){
                        for(j=2; j<argc-2; j++){
                            printf("%s",argv[j]);
                            printf(" ");
                        }
                        printf("\n");
                    }
                }

                else if(strcmp(argv[1], "environ") == 0){
                    for(j=0; envp[j]; j++){
                        printf("%s\n", envp[j]);
                    }
                }

                else if(strcmp(argv[1], "help") == 0){
                    read_file("readme_doc");
                }

                else{
                    argv[i] = NULL;
                    argv[argc] = NULL;

                    execvp(argv[1], &argv[1]);
                    if(execvp(argv[1], &argv[1]) == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }
                }
                fflush(stdout);
            }
        }

        if(strcmp(argv[i], ">>") == 0){
            *re += 1;

            if(argc < 4){
	            return 1;
            }

            pid = fork();
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            
            else if(pid == 0){
                out_fd = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                if(out_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }

                dup2(out_fd, 1);
                close(out_fd);

                if(strcmp(argv[1], "dir") == 0){
                    recursive_dir(".");
                }

                else if(strcmp(argv[1], "echo") == 0){
                    if(strcmp(argv[2], ">>") == 0){
                        printf("\n");
                    }
                    if(strcmp(argv[2], ">>") != 0){
                        for(j=2; j<argc-2; j++){
                            printf("%s",argv[j]);
                            printf(" ");
                        }
                        printf("\n");
                    }
                }

                else if(strcmp(argv[1], "environ") == 0){
                    for(j=0; envp[j]; j++){
                        printf("%s\n", envp[j]);
                    }
                }

                else if(strcmp(argv[1], "help") == 0){
                    read_file("readme_doc");
                }

                else{
                    argv[i] = NULL;
                    argv[argc] = NULL;

                    execvp(argv[1], &argv[1]);
                    if(execvp(argv[1], &argv[1]) == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }
                }
                fflush(stdout);
            }
        }
    }
    if(*re > 0){
        dup2(stdInSave, 0);
        dup2(stdOutSave, 1);
        close(stdInSave);
        close(stdOutSave);
    }

    if(bg == 0 && *re > 0){
        for(i=0; i<*re; i++){
            wait(&pid);
        }
        printf("myshell: redirection executed.\n");
    }

    if(bg > 0 && *re > 0){
        printf("myshell: process running in the background.\n");
        kill(pid, SIGTERM);
    }
    return 1;
}

int pipe_func(int argc, char **argv, int *pi){
    const char *error_message = "myshell: an error has occured.\n";
    int fd[2];
    int pid, i, j, bg = 0;
    *pi = 0;

    background(argc, argv, &bg);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "|") == 0){
            *pi += 1;

            if(argc<3){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }

            if(pipe(fd) == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }

            if(pipe(fd) == 0){
                pid = fork();

                if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
                }

                else if(pid == 0){

                    close(1);
		            dup2(fd[1], 1);
		            close(fd[0]);
                    close(fd[1]);

                    argv[i] = NULL;

		            execvp(argv[1], &argv[1]);
                }

                else{
	                pid = fork();

                    if(pid == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
	                    return 1;
                    }

	                else if(pid == 0){

                        close(0);
		                dup2(fd[0], 0);
		                close(fd[1]);
                        close(fd[0]);

                        for(j=1; j<=i; j++){
                            argv[j] = NULL;
                        }
                        argv[argc] = NULL;

		                execvp(argv[i+1], &argv[i+1]);
		                if(execvp(argv[i+1], &argv[i+1]) == -1){
			                write(STDERR_FILENO, error_message, strlen(error_message));
	                        return 1;
		                }
	                }

	                else{
                        if(bg == 0){
                            wait(&pid);
                            printf("myshell: pipe has been executed\n");
                        }
                        
                        if(bg > 0){
                            kill(pid, SIGTERM);
                            printf("myshell: process running in the background.\n");
                        }
                    }
                }
            }
        }
    }
    return 1;
}

int external(int argc, char **argv){
    const char *error_message = "myshell: an error has occured.\n";
    int i, pid;
    int bg = 0;
    
    background(argc, argv, &bg);

    if(argc >= 2){
        pid = fork();
        if(pid == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }

        else if(pid == 0){
            argv[argc] = NULL;
            execvp(argv[1], &argv[1]);
            if(execvp(argv[1], &argv[1]) == -1){
                for(i=1; i<argc; i++){
                    printf("myshell: \"%s\": command not found.\n", argv[i]);
                }
                return 1;
            }
        }

        if(bg == 0){
            wait(&pid);
            printf("myshell: external command executed.\n");
        }

        if(bg > 0){
            kill(pid, SIGTERM);
            printf("myshell: process running in the background.\n");
        }
    }
    return 1;
}