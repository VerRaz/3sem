#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<stdbool.h>

typedef char *word;

char symbol;

bool _write_to_file, _add_to_file, _from_file; 
bool _background_mode; 
int ERROR;
int i, j, l;
int file;
int pid;
int com_count; //n
int status;
int current_stdout;
int current_stdin;
word filename;
word *command;

void clear() {
    for(int k = 0; k <= i; k++) {
        free(command[i]);
    }
    free(command);
    free(filename);

    _write_to_file = false; 
    _add_to_file = false; 
    _from_file = false; 
    _background_mode = false;
    ERROR = 1;
    i = 0;
    j = 0;
    l = 0;
    file = 0;
    pid = 0;
    status = 0;

    dup2(current_stdin, 0);
    dup2(current_stdout, 1);

    filename = malloc(sizeof(char));
    command = malloc(sizeof(char*));//чтобы можно было чистить для каждой команды
    command[0]= malloc(sizeof(char));
}

int main()
{
    int exit_code;
    current_stdout = dup(1);
    current_stdin = dup(0);
    filename = malloc(sizeof(char));
    command = malloc(sizeof(char*));
    command[0]= malloc(sizeof(char));
    while (true) {
        printf(">");
        clear();
        symbol = getchar();
        while ((symbol != EOF) && (symbol != '\n')) {
            switch(symbol) {
                case ' ':
                    symbol = getchar();
                    break;
                case '\n':
                    break;
                case EOF:
                    break;
                case '<':
                    _from_file = true;
                    symbol = getchar();
                    break;
                case '>':
                    if ((symbol = getchar()) == '>') {
                        _add_to_file = true;
                        symbol = getchar();
                    }
                    else _write_to_file = true;
                    break;
                case ';':
                    command = realloc(command, sizeof(char*)*(i+1));
                    command[i] = NULL;
                    if((pid = fork()) == 0){
                        execvp(command[0], command);
                        exit(1);
                    }
                    wait(NULL);
                    clear();
                    symbol = getchar();
                    break;
                case '|':
                    if ((symbol = getchar()) == '|') {
                        command = realloc(command, sizeof(char*)*(i+1));
                        command[i] = NULL;
                        if((pid = fork()) == 0){
                            execvp(command[0], command);
                            exit(1);
                        }
                        waitpid(pid, &status, 0);
                        ERROR = WEXITSTATUS(status);
                        symbol = getchar();
                        if (ERROR == 0) {
                            while ((symbol != EOF) && (symbol != '\n')) {
                                symbol = getchar();
                            }
                        }
                        clear();
                    }
                    else {
                        int fd[2];
                        pipe(fd);
                        dup2(fd[1],1);
                        close(fd[1]);
                        command = realloc(command, sizeof(char*)*(i+1));
                        command[i] = NULL;
                        if(!fork()){
                            execvp(command[0], command);
                            exit(1);
                        }
                        wait(NULL);
                        clear();
                        //symbol = getchar();
                        dup2(fd[0],0);
                    }
                    break;
                case '&':
                    if ((symbol = getchar()) == '&') {
                        command = realloc(command, sizeof(char*)*(i+1));
                        command[i] = NULL;
                        if((pid = fork()) == 0){
                            execvp(command[0], command);
                            exit(1);
                        }
                        waitpid(pid, &status, 0);
                        ERROR = WEXITSTATUS(status);
                        symbol = getchar();
                        if (ERROR != 0) {
                            while ((symbol != EOF) && (symbol != '\n')) {
                                symbol = getchar();
                            }
                        }
                    }
                    else { 
                        command = realloc(command, sizeof(char*)*(i+1));
                        command[i] = NULL;
                        if((pid = fork()) == 0){
                            signal(SIGINT,SIG_IGN);
                            execvp(command[0], command);
                            exit(1);
                        }
                    }
        //         if(!(fork())){
        //             command = realloc(command, sizeof(char*)*(i+1));
        //             command[i] = NULL;
        //             if((pid = fork()) == 0){
        //                 printf("COM %s\n", command[0]);
        //                 execvp(command[0], command);
        //                 exit(1);
        //             }
        //         }
        //     }

                    clear();
                    break;
                default:
                    if ((!_write_to_file) && (!_add_to_file) && (!_from_file)) {
                        command = realloc(command, sizeof(char*)*(i+1));
                        command[i] = malloc(sizeof(char));
                        while ((symbol !=';')&&(symbol !=' ')&&(symbol !='\n')&&(symbol!=EOF)
                            &&(symbol!='|')&&(symbol!='>')&&(symbol!='<')&&(symbol!='&')){
                            command[i] = realloc(command[i], sizeof(char)*(j+1));
                            command[i][j] = symbol;
                            j++;
                            symbol = getchar();
                        }
                        command[i] = realloc(command[i], sizeof(char)*(j+1));
                        command[i][j] = '\0';
                        com_count++;
                        j = 0;
                        i++;
                    }
                    else {
                        while ((symbol!=';')&&(symbol!=' ')&&(symbol!='\n')&&
                        (symbol!=EOF)&&(symbol!='|')&&(symbol!='>')&&(symbol!='<')&&(symbol!='&')) {
                            filename = realloc(filename, sizeof(char)*(l+1));
                            filename[l] = symbol;
                            l++;
                            symbol = getchar();
                        }
                        filename = realloc(filename, sizeof(char)*(l+1));
                        filename[l] = '\0';
                        l = 0;
                    }
                    if (_write_to_file) {
                        if (file == -1) {
                            return(1);
                        }
                        file = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                        dup2(file, 1);
                        close(file);
                    }
                    if (_add_to_file){
                        file = open(filename, O_APPEND | O_CREAT | O_WRONLY, 0777);
                        dup2(file, 1);
                        close(file);
                        if (file == -1){ 
                            return(1);
                        }
                    }
                    if (_from_file){
                        file = open(filename, O_RDONLY);
                        dup2(file, 0);
                        close(file);
                        if(file == -1) {
                            return(1);
                        }
                    }
                    break;
            }
        }
        if (!_background_mode){
            command = realloc(command, sizeof(char*)*(i+1));
            command[i] = NULL;
            if((pid = fork()) == 0){
                execvp(command[0], command);
                exit(1);
            }
            wait(NULL);
        }
        //     if(_background_mode){
        //         if(!(fork())){
        //             command = realloc(command, sizeof(char*)*(i+1));
        //             command[i] = NULL;
        //             if((pid = fork()) == 0){
        //                 printf("COM %s\n", command[0]);
        //                 execvp(command[0], command);
        //                 exit(1);
        //             }
        //         }
        //     }
        if (symbol == EOF) return 0; 
        printf("here");    
    }
    return 0;
}
