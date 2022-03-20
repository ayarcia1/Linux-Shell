#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

char *recursive_dir(char **argv, char *path_name){
    char path[1024];
    struct dirent *files;
    DIR *directory;
    directory = opendir(path_name);
    
    if(directory == NULL){
        return NULL;
    }

    while((files = readdir(directory)) != NULL){
        if(strcmp(files->d_name, ".") != 0 && strcmp(files->d_name, "..") != 0 && strcmp(argv[1], "dir") == 0){
            printf("%s\n", files->d_name);

            strcpy(path, path_name);
            strcat(path, "/");
            strcat(path, files->d_name);
            
            recursive_dir(argv, path);
        }
        if(strcmp(files->d_name, ".") != 0 && strcmp(files->d_name, "..") != 0 && strcmp(argv[1], "dir") != 0){
            strcpy(path, path_name);
            strcat(path, "/");
            strcat(path, files->d_name);
            
            recursive_dir(argv, path);
        }
    }
    closedir(directory);
    return path_name;
}

void read_file(char *file){
    const char *error_message = "myshell: an error has occured.\n";
    FILE *n;
    char txt;

    n = fopen(file, "r");

    if(n == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
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
        if(strcmp(argv[i], "&") == 0){
            *bg += 1;
        }
    }
    return *bg;
}

int redirection(int argc, char **argv, char **envp, int *red){
    const char *error_message = "myshell: an error has occured.\n";
    int i, j, pid, bg = 0;
    int in_fd, out_fd;
    int stdOutSave = dup(0);
    int stdInSave = dup(1);
    *red = 0;

    background(argc, argv, &bg);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "<") == 0){
            *red += 1;
            if(argc<4){
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

                argv[i] = NULL;
                argv[argc] = NULL;

                execvp(argv[1], &argv[1]);
                if(execvp(argv[1], &argv[1]) == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                fflush(stdin);
            }
        }

        if(strcmp(argv[i], ">") == 0){
            *red += 1;
            if(argc<4){
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
                    recursive_dir(argv, ".");
                    *red += 1;
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
                    if(strcmp(argv[1], "ls") == 0){
                        argv[argc-1] = recursive_dir(argv, ".");
                    }
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
            *red += 1;
            if(argc<4){
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
                    recursive_dir(argv, ".");
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
                    if(strcmp(argv[1], "ls") == 0){
                        argv[argc-1] = recursive_dir(argv, ".");
                    }

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

    if(bg == 0 && *red > 0){
        dup2(stdInSave, 0);
        dup2(stdOutSave, 1);
        close(stdInSave);
        close(stdOutSave);
        waitpid(pid, NULL, 0);
        printf("myshell: redirection executed.\n");
    }
    if(bg > 0 && *red > 0){
        dup2(stdInSave, 0);
        dup2(stdOutSave, 1);
        close(stdInSave);
        close(stdOutSave);
        printf("myshell: process running in the background.\n");
    }
    return 1;
}

int pipe_func(int argc, char **argv, int *pipes){
    const char *error_message = "myshell: an error has occured.\n";
    int fd[2];
    int pid, i, bg = 0;
    int fd_err;
    *pipes = 0;
    
    background(argc, argv, &bg);

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "|") == 0){
            *pipes += 1;
            if(argc<4){
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
                    fd_err = open("stderr.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                    if(fd_err == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }

                    close(1);
		            dup2(fd[1], 1);
                    dup2(fd_err, 2);
		            close(fd[0]);
                    close(fd[1]);

                    argv[argc] = NULL;
		            execvp(argv[1], &argv[1]);
                }

                else{
	                pid = fork();

                    if(pid == -1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
	                    return 1;
                    }

	                else if(pid == 0){
                        fd_err = open("stderr.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                        if(fd_err == -1){
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 1;
                        }

                        close(0);
		                dup2(fd[0], 0);
                        dup2(fd_err, 2);
		                close(fd[1]);
                        close(fd[0]);

                        argv[argc] = NULL;
		                execvp(argv[1], &argv[1]);
		                if(execvp(argv[1], &argv[1]) == -1){
			                write(STDERR_FILENO, error_message, strlen(error_message));
	                        return 1;
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
                            printf("myshell: process running in the background.\n");
                        }
                        fflush(stderr);
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

    if(argc>=2){
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
        if(bg==0){
            waitpid(pid, NULL, 0);
            printf("myshell: external command executed.\n");
        }
        if(bg>0){
            waitpid(pid, NULL, 0);
            printf("myshell: process running in the background.\n");
        }
    }
    return 1;
}