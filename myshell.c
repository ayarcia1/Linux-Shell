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
void parse_line(int *argc, char **argv, FILE *n);
int built_in(int argc, char **argv, char **envp, int *in);

int main(int argc, char **argv, char **envp){
    const char *error_message = "myshell: an error has occured.\n";
    //executing shell with no arguments executes shell in interactive mode.
    if(argc == 1){
        //prints initial myshell> and gets initial user input
        printf("myshell> ");
        parse_line(&argc, argv, stdin);
        //while user presses enter with no input, shell prints myshell> and gets user input again.
        while(argc == 1){ 
            printf("myshell> ");
            parse_line(&argc, argv, stdin);
        }
        //funtions return 1, while loop will execute all funtions in loop.
        while(1){
            //flags to determine what functions have been used.
            int re = 0;
            int pi = 0;
            int in = 0;
            //while user presses enter with no input, shell prints myshell> and gets user input again.
            while(argc == 1){
                printf("myshell> ");
                parse_line(&argc, argv, stdin);
            }
            //redirection function checks for I/O redirection in user input.
            redirection(argc, argv, envp, &re);
            //pipe function checks for any piping in user input.
            pipe_func(argc, argv, &pi);
            //builtin function checks for builtin commands in user input if there is no I/O redirection or piping.
            if(re == 0 && pi == 0){
                built_in(argc, argv, envp, &in);
            }
            //external function checks for any external commands that are not I/O redirection, piping, and builtin in user input.
            if(re == 0 && pi == 0 && in == 0){
                external(argc, argv);
            }
            //shell continues while loop by taking user input again until user exits the shell.
            printf("myshell> ");
            parse_line(&argc, argv, stdin);
        }
    }
    //if shell contains one argument of a batch file, shell will execute in batch mode.
    else if(argc == 2){
        FILE *n;
        //open batch file for reading.
        n = fopen(argv[1], "r");
        //if the batch file is bad, print error and exit out of the shell.
        if(n == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        //while the end of the batch file has not been reached the loop will execute.
        while(feof(n) == 0){
            //flags to determine what functions have been used.
            int re = 0;
            int pi = 0;
            int in = 0;
            //stores input from the batch file line by line into **argv array.
            parse_line(&argc, argv, n);
            //redirection function checks for I/O redirection in user input.
            redirection(argc, argv, envp, &re);
            //pipe function checks for any piping in user input.
            pipe_func(argc, argv, &pi);
            //builtin function checks for builtin commands in user input if there is no I/O redirection or piping.
            if(re == 0 && pi == 0){
                built_in(argc, argv, envp, &in);
            }
            //external function checks for any external commands that are not I/O redirection, piping, and builtin in user input.
            if(re == 0 && pi == 0 && in == 0){
                external(argc, argv);
            }
        }
        //closes batch file and exits out of the shell.
        fclose(n);
        exit(1);
    }
    //if there is more than one argument, shell will print an error and exit.
    else{
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}

void parse_line(int *argc, char **argv, FILE *file){
    //initialize argv[i] and argc to 1;
    int i = 1;
    *argc = 1;
    size_t size = 1024;
    char *line = (char*)malloc(size);
    char **string = &line;
    //take string as input from input stream.
    getline(string, &size, file);
    //tokenize for white spaces.
    char *token = strtok(line, " \n");
    //while there is input in the stream.
    while(token != NULL){
        //stores tokens in char **argv array for parsing.
        argv[i] = token;
        i++;
        //argc is incremented along argv[i] to stay consistent.
        *argc += 1;
        //continue tokenizing input stream.
        token = strtok(NULL, " \n");
    }
}

//function for the builtin commands of the shell.
int built_in(int argc, char **argv, char **envp, int *in){
    const char *error_message = "myshell: an error has occured.\n";
    int i;
    *in = 0;

    if(strcmp(argv[1], "cd") == 0){
        //cd with no arguments.
        if(argc == 2){
            char path[1024];
            //prints the current working directory.
            printf("%s\n", getcwd(path, sizeof(path)));
        }
        //cd with one argument.
        else if(argc == 3){
            //changes current working directory to the input in the second argument.
            chdir(argv[2]);
            //if chdir fails, print error message and return to user input.
            if(chdir(argv[2]) == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            //print success messsage.
            printf("myshell: current working directory: %s\n", argv[2]);
        }
        //if more than one argument, error will print and return back to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "clr") == 0){
        //clr by itself prints a text that clears the shell bash.
        if(argc == 2){
            printf("\e[1;1H\e[2J");
        }
        //if more than zero arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "dir") == 0){
        DIR *directory;

        if(argc == 2){
            //open the current working directory.
            directory = opendir(".");
            //if the directory fails to open, error will print and return to user input. 
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            printf("[Current Directory]\n");
            //recursively print contents of current working directory.
            recursive_dir(".");
        }
        else if(argc == 3){
            //open the directory input given in the second argument.
            directory = opendir(argv[2]);
            //if the directory fails to open, error will print and return to user input.
            if(directory == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            printf("%s\n", argv[2]);
            //recursively print contents of directory path argument.
            recursive_dir(argv[2]);
        }
        //if more than two arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment the built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "path") == 0){
        //if path takes no arguments set path to "".
        if(argc == 2){
            setenv("PATH", "", 1);
        }
        else if(argc>=3){
            char path[1024] = "\0";
            //initialize path with "/bin:"
            strcat(path, "/bin:");

            for(i=2; i<argc; i++){
                //add arguments to path.
                strcat(path, argv[i]);
                //if it is not the final argument add a ":" to path between arguments.
                if(strcmp(argv[i], argv[argc-1]) != 0){
                    strcat(path, ":");
                }
            }
            //set path array to path and print the path.
            setenv("PATH", path, 1);
            printf("myshell: PATH=%s\n", getenv("PATH"));
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "environ") == 0){
        if(argc == 2){
            //iterates through every environmental variable and prints.
            for(i=0; envp[i]; i++){
                printf("%s\n", envp[i]);
            }
        }
        //if more than zero arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "echo") == 0){
        //if echo is give no arguments print a blank line.
        if(argc == 2){
            printf("\n");
        }
        if(argc>=3){
            //iterate through every argument after echo.
            for(i=2; i<argc; i++){
                //prints every argument after echo.
                printf("%s", argv[i]);
                //seperate arguments with one space.
                printf(" ");
            }
            //prints the next line after command is finished.
            printf("\n");
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "help") == 0){
        //help with no arguments.
        if(argc == 2){
            //prints the contents of readme_doc.
            read_file("readme_doc");
        }
        //if more than zero arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }

    else if(strcmp(argv[1], "pause") == 0){
        if(argc == 2){
            //while loop goes forever until it breaks.
            while(2){
                //print pause message.
                printf("myshell: myshell has been paused, press enter to continue.\n");
                //if enter is pressed, print resume message and break while loop.
                if(getchar()){
                    printf("myshell: myshell has been resumed.\n");
                    break;
                }
                //if enter is not pressed continue the while loop.
                else{
                    continue;
                }
            }
        }
        //if more than zero arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment built in flag.
        *in += 1;
    }

    else if(strcmp(argv[1], "quit") == 0){
        //quit with no arguments exits out of the shell.
        if(argc == 2){
            exit(0);
        }
        //if more than zero arguments, error will print and return to user input.
        else{
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        //increment built in flag.
        *in += 1;
    }
    //return back to while loop in main.
    return 1;
}

char *recursive_dir(char *path_name){
    char path[1024];
    struct dirent *files;
    DIR *directory;
    //open directory for directory path call.
    directory = opendir(path_name);
    //if directory does not open return nothing.
    if(directory == NULL){
        return NULL;
    }
    //while the directory is being read
    while((files = readdir(directory)) != NULL){
        //ignores files that begin with "." or "..".
        if(strcmp(files->d_name, ".") != 0 && strcmp(files->d_name, "..") != 0){
            //prints the inital directory contents.
            printf("%s\n", files->d_name);
            //copies the directory path call to a path char.
            strcpy(path, path_name);
            //adds a "/" to the end of path to start a new directory.
            strcat(path, "/");
            //adds the inner directories to the end of the "/".
            strcat(path, files->d_name);
            //calls recursive_dir funtion again with path to print the contents recursively.
            recursive_dir(path);
        }
    }
    //closes directory.
    closedir(directory);
    //returns directory path call.
    return path_name;
}

void read_file(char *file){
    const char *error_message = "myshell: an error has occured.\n";
    FILE *n;
    char txt;
    //open file call for reading.
    n = fopen(file, "r");
    //if file does not open, print error and exit out of shell.
    if(n == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    txt = fgetc(n);
    //while txt has not reached the end of a file.
    while(txt != EOF){
        //print file characters.
		printf("%c", txt);
        txt = fgetc(n);
	}
    //close the file.
	fclose(n);
    printf("\n");
}

int background(int argc, char **argv, int *bg){
    int i;
    *bg = 0;
    //iterates through user input to find "&".
    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "&") == 0){
            //increment background flag.
            *bg += 1;
        }
    }
    //return the background flag.
    return *bg;
}