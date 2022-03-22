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
    *re = 0;
    //duplicate stdin and stdout.
    int stdOutSave = dup(0);
    int stdInSave = dup(1);
    //call background funtion for "&" inputs.
    background(argc, argv, &bg);
    //iterate through argv commands.
    for(i=1; i<argc; i++){
        //if an argv command contains "<".
        if(strcmp(argv[i], "<") == 0){
            //increment the I/O redirection flag.
            *re += 1;
            //if less than 3 arguments, print error and return to user input.
            if(argc < 4){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            //create a copy of the process.
            pid = fork();
            //if fork fails, print error and return to user input.
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            //child process.
            else if(pid == 0){
                //open file after "<" for reading.
                in_fd = open(argv[i+1], O_RDONLY);
                //if file fails to open, print error and return to user input.
                if(in_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                //redirect stdin to open file.
                dup2(in_fd, 0);
                //close file descriptor.
                close(in_fd);
                //set "<" to NULL to avoid stderr.
                argv[i] = NULL;
                //set argv after input to NULL for exec.
                argv[argc] = NULL;
                //exec the argument inputs.
                execvp(argv[1], &argv[1]);
                //if exec fails, print error and return to user input.
                if(execvp(argv[1], &argv[1]) == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                //flush the stdin.
                fflush(stdin);
            }
        }
        //if an argv command contains ">".
        if(strcmp(argv[i], ">") == 0){
            //increment the I/O redirection flag.
            *re += 1;
            //if less than 3 arguments, print error and return to user input.
            if(argc < 4){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            //create of copy of the process.
            pid = fork();
            //if fork fails, print error and return to user input.
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            //child process.
            else if(pid == 0){
                //open file after ">" fpr writing, truncting, and creating.
                out_fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                //if file fails to open, print error and return to user input.
                if(out_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                //redirect stdout to open file.
                dup2(out_fd, 1);
                //close file descriptor.
                close(out_fd);
                //I/O redirection for dir command.
                if(strcmp(argv[1], "dir") == 0){
                    recursive_dir(".");
                }
                //I/O redirection for echo command.
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
                //I/O redirection for environ command.
                else if(strcmp(argv[1], "environ") == 0){
                    for(j=0; envp[j]; j++){
                        printf("%s\n", envp[j]);
                    }
                }
                //I/O redirection for help command.
                else if(strcmp(argv[1], "help") == 0){
                    read_file("readme_doc");
                }
                //if I/O redirection does not use built in commands.
                else{
                    //set ">" to NULL to avoid stderr.
                    argv[i] = NULL;
                    //set argv after input to NULL for exec.
                    argv[argc] = NULL;
                    //exec the argument inputs.
                    execvp(argv[1], &argv[1]);
                    //if exec fails, print error and return to user input.
                    if(execvp(argv[1], &argv[1]) == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }
                }
                //flush stdout.
                fflush(stdout);
            }
        }
        //if an argv command contains ">>".
        if(strcmp(argv[i], ">>") == 0){
            //increment the I/O redirection flag.
            *re += 1;
            //if less than 3 arguments, print error and return to user input.
            if(argc < 4){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            //create a copy of the process.
            pid = fork();
            //if fork fails, print error and return to user input.
            if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            //child process.
            else if(pid == 0){
                //open file after ">>" for writing, appending, and creating.
                out_fd = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                //if file fails to open, print error and return to user input.
                if(out_fd == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                //redurect stdout to open file.
                dup2(out_fd, 1);
                //close file descriptor.
                close(out_fd);
                //I/O redirection for dir command.
                if(strcmp(argv[1], "dir") == 0){
                    recursive_dir(".");
                }
                //I/O redirection for echo command.
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
                //I/O redirection for environ command.
                else if(strcmp(argv[1], "environ") == 0){
                    for(j=0; envp[j]; j++){
                        printf("%s\n", envp[j]);
                    }
                }
                //I/O redirection for help command.
                else if(strcmp(argv[1], "help") == 0){
                    read_file("readme_doc");
                }
                //if I/O redirection does not use built in commands.
                else{
                    //set ">>" to NULL to avoid stderr.
                    argv[i] = NULL;
                    //set argv after input to NULL for exec.
                    argv[argc] = NULL;
                    //exec the argument inputs.
                    execvp(argv[1], &argv[1]);
                    //if exec fails, print error and return to user input.
                    if(execvp(argv[1], &argv[1]) == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }
                }
                //flush stdout.
                fflush(stdout);
            }
        }
    }
    //if redirection was executed.
    if(*re > 0){
        //restore stdin and stdout.
        dup2(stdInSave, 0);
        dup2(stdOutSave, 1);
        //close duplicated stdin and stdout.
        close(stdInSave);
        close(stdOutSave);
    }
    //if there is no background and redirection was executed.
    if(bg == 0 && *re > 0){
        //wait for every redirection.
        for(i=0; i<*re; i++){
            waitpid(pid, NULL, 0);
        }
        //print success message.
        printf("myshell: I/O redirection executed.\n");
    }
    //if there is background and redirection was executed.
    if(bg > 0 && *re > 0){
        //kill child for every background process.
        for(i=0; i<bg; i++){
            kill(pid, SIGTERM);
        }
        //print success message.
        printf("myshell: process running in the background.\n");
    }
    //return back to while loop in main.
    return 1;
}

int pipe_func(int argc, char **argv, int *pi){
    const char *error_message = "myshell: an error has occured.\n";
    int fd[2];
    int pid, i, j, bg = 0;
    *pi = 0;
    //call background funtion for "&" inputs.
    background(argc, argv, &bg);
    
    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "|") == 0){
            *pi += 1;
            //if less than 2 arguments, print error and return to user input.
            if(argc<3){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }
            //if pipe fails, print error and return to user input.
            if(pipe(fd) == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
            }

            else if(pipe(fd) == 0){
                //create a copy of the process.
                pid = fork();
                //if fork fails, print error and return to user input.
                if(pid == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
	            return 1;
                }
                //child process.
                else if(pid == 0){
                    //close stdout.
                    close(1);
                    //duplicate the writing end of pipe for stdout.
		            dup2(fd[1], 1);
                    //close reading and writing end of pipe.
		            close(fd[0]);
                    close(fd[1]);
                    //set "|" to NULL to avoid stderr.
                    argv[i] = NULL;
                    //exec the argument inputs before "|" command.
		            execvp(argv[1], &argv[1]);
                }

                else{
                    //create a copy of the process.
	                pid = fork();
                    //if fork fails, print error and return to user input.
                    if(pid == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
	                    return 1;
                    }
                    //child process.
	                else if(pid == 0){
                        //close stdin.
                        close(0);
                        //duplicate reading end of pipe for stdin.
		                dup2(fd[0], 0);
                        //close writing and reading end of pipe.
		                close(fd[1]);
                        close(fd[0]);
                        //iterate through every argv before "|" to NULL;
                        for(j=1; j<=i; j++){
                            argv[j] = NULL;
                        }
                        //set argv after input to NULL for exec.
                        argv[argc] = NULL;
                        //exec the argument inputs after "|" command.
		                execvp(argv[i+1], &argv[i+1]);
                        //if exec fails, print error and return to user input.
		                if(execvp(argv[i+1], &argv[i+1]) == -1){
			                write(STDERR_FILENO, error_message, strlen(error_message));
	                        return 1;
		                }
	                }

	                else{
                        //if process is not in background wait for child process.
                        if(bg == 0){
                            close(fd[0]);
                            close(fd[1]);
                            waitpid(pid, NULL, 0);
                            printf("myshell: pipe has been executed\n");
                        }
                        //if process is in the background kill the child processs.
                        if(bg > 0){
                            kill(pid, SIGTERM);
                            printf("myshell: process running in the background.\n");
                        }
                    }
                }
            }
        }
    }
    //return back to while loop in main.
    return 1;
}

int external(int argc, char **argv){
    const char *error_message = "myshell: an error has occured.\n";
    int i, pid;
    int bg = 0;
    //call background funtion for "&" inputs.
    background(argc, argv, &bg);
    //if there is at least one argument.
    if(argc >= 2){
        //create a copy of the process.
        pid = fork();
        //if fork fails, print error and return to user input.
        if(pid == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //child process.
        else if(pid == 0){
            //set argv after input to NULL for exec.
            argv[argc] = NULL;
            //exec the argument inputs.
            execvp(argv[1], &argv[1]);
            //if exec fails, print error message and return to user input.
            if(execvp(argv[1], &argv[1]) == -1){
                for(i=1; i<argc; i++){
                    printf("myshell: \"%s\": command not found.\n", argv[i]);
                }
                return 1;
            }
        }
        //if process is not in background wait for child process.
        if(bg == 0){
            waitpid(pid, NULL, 0);
            printf("myshell: external command executed.\n");
        }
        //if process is in the background kill the child processs.
        if(bg > 0){
            kill(pid, SIGTERM);
            printf("myshell: process running in the background.\n");
        }
    }
    //return back to while loop in main.
    return 1;
}