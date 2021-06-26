#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include "function.h"

/******************************************************************************
* Function Name  : args_error() 
* Description    : Report client usage error with error message
*                  "Usage: client chatscript"
* Input          : None
* Return         : None
******************************************************************************/
void args_error() {
    fprintf(stderr, "Usage: client name authfile port\n");
    exit(1);
}

/******************************************************************************
* Function Name  : arg_checking(int argc, char** argv) 
* Description    : Check if the arguments input in command line is correct
* Input          : int argc;
*                  char** argv;
* Output         : None
* Return         : None
******************************************************************************/
void arg_checking(int argc, char** argv) {
    FILE* fp = fopen(argv[2], "r");

    if (argc != 4 || fp == NULL) {
        args_error();
    }
}

/******************************************************************************
* Function Name  : name_reply(int* nameTakenNum, char* name, FILE** pipes) 
* Description    : According to the nameTakenNum, reply client name to server 
* Input          : int* nameTakenNum;
*                  char** name;
*                  FILE** pipes;
* Return         : None
******************************************************************************/
void name_reply(int* nameTakenNum, char* name, FILE** pipes) {
    // int numLength = 0;
    FILE* send = pipes[0];

    if (*nameTakenNum == -1) {
        fprintf(send, "NAME:%s\n", name);
        fflush(send);
    } else {
        fprintf(send, "NAME:%s%d\n", name, *nameTakenNum);
        fflush(send);
    }
}

/******************************************************************************
* Function Name  : check_over_two_colon(char* buffer)
* Description    : Check if there is over two colons in the buffer. 
* Input          : char* buffer;
* Return         : Return 1 if over two colons
*                  Return 0 if do not
******************************************************************************/
int check_over_two_colon(char* buffer) {
    int haveColon = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == 58) {
            haveColon++;
            
        }
    }
    
    if (haveColon >= 2) {
        return 1;
    }
    return 0;
}

/******************************************************************************
* Function Name  : handle_msg_cmd(char* buffer) 
* Description    : After receive "MSG:" from server, client send  
*                  this text to stdout with a specific format
* Input          : char* buffer;
* Return         : None
******************************************************************************/
void handle_msg_cmd(char* buffer) {
    char cmd[4];
    memset(cmd, '\0', 4);
    char name[strlen(buffer)];
    memset(name, '\0', strlen(buffer));
    char message[strlen(buffer)];
    memset(message, '\0', strlen(buffer));

    sscanf(buffer, "%[^:]:%[^:]:%[^\n]\n", cmd, name, message);
     
    fprintf(stdout, "%s: %s\n", name, message);
    
}

/******************************************************************************
* Function Name  : server_conn(char** argv, FILE** pipes)
* Description    : Connect to server port using IPv4 protocol
* Input          : char** argv;
*                  FILE** pipes;
* Return         : None
******************************************************************************/
void server_conn(char** argv, FILE** pipes) {
    const char* port = argv[3];
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    int err;

    if ((err = getaddrinfo("localhost", port, &hints, &ai))) {
        freeaddrinfo(ai);
        // fprintf(stderr, "%s\n", gai_strerror(err));
        communication_error();
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (connect(fd, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        communication_error();
    }
    // fd is now connected
    // we want separate streams (which we can close independently)
    
    int fd2 = dup(fd);
    FILE* send = fdopen(fd, "w");
    FILE* receive = fdopen(fd2, "r");

    pipes[0] = send;
    pipes[1] = receive;
}

/******************************************************************************
* Function Name  : stdin_handling(void* v)
* Description    : A thread function, handle the messages typed by users
*                  in stdin and send them to the server
* Input          : void* v;
* Return         : None
******************************************************************************/
void* stdin_handling(void* v) {
    FILE** pipes = (FILE**)v;
    char* buffer = (char*)malloc(sizeof(char));
    FILE* send = pipes[0];

    while (1) {
        usleep(100000);
        if (read_stdin_dynamic(&buffer)) {
            free_malloc_char_pointer(&buffer);
            exit(0);
        }
        
        if (strcmp(buffer, "*LEAVE:") == 0) {
            exit(0);
        }
        // send msg as CMD to server
        if (strncmp(buffer, "*", 1) == 0) {
            // remove asterisk
            char* bufferNew = 
                    (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
            // add \n
            sprintf(bufferNew, "%s\n", buffer + 1);
            // send
            fprintf(send, bufferNew);
            fflush(send);
        // send msg as SAY to server
        } else {
            char* message = 
                    (char*)malloc(sizeof(char) * (strlen(buffer) + 6));
            sprintf(message, "SAY:%s\n", buffer);
            // send
            fprintf(send, message);
            fflush(send);
        }

        free(buffer);
        buffer = (char*)malloc(sizeof(char));
        memset(buffer, '\0', 1);
    }
}

/******************************************************************************
* Function Name  : get_auth(char** buffer, char** argv)
* Description    : Get the authentication from the authfile provided by
*                  user, the file path is in the arguments
* Input          : char** buffer;
*                  char** argv
* Return         : None
******************************************************************************/
void get_auth(char** buffer, char** argv) {
    FILE* fp = fopen(argv[2], "r");
    read_file_dynamic(buffer, fp);
}

/******************************************************************************
* Function Name  : authentication_checking(FILE** pipes, char** argv)
* Description    : Send the authentication to server, wait for server checking,
*                  if replied with OK: then enter the chating group
* Input          : FILE** pipes;
*                  char** argv;
* Return         : None
******************************************************************************/
void authentication_checking(FILE** pipes, char** argv) {
    char* authentication = (char*)malloc(sizeof(char));
    char* buffer = (char*)malloc(sizeof(char));

    while (1) {
        usleep(100000);
        if (read_fdin_dynamic(&buffer, pipes[1])) {
            fprintf(stderr, "Authentication error\n");
            exit(4);
        }
        
        if (strcmp(buffer, "AUTH:") == 0) {
            get_auth(&authentication, argv);
            fprintf(pipes[0], "AUTH:%s\n", authentication);
            fflush(pipes[0]);

        } else if (strcmp(buffer, "OK:") == 0) {
            break;
        }
        free_malloc_char_pointer(&buffer);
    }

}

/******************************************************************************
* Function Name  : registering_username(FILE** pipes, char** argv)
* Description    : Registering username, give server name which provided by 
*                  user, if not valid then add number after the name, until
*                  name valid
* Input          : FILE** pipes;
*                  char** argv;
* Return         : None
******************************************************************************/
void registering_username(FILE** pipes, char** argv) {
    char* buffer = (char*)malloc(sizeof(char));
    char* name = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 1));
    int nameTakenNum = -1;
    strcpy(name, argv[1]);

    while (1) {
        usleep(100000);
        if (read_fdin_dynamic(&buffer, pipes[1])) {
            communication_error();
        }

        if (strcmp(buffer, "WHO:") == 0) {
            name_reply(&nameTakenNum, name, pipes);

        } else if (strcmp(buffer, "NAME_TAKEN:") == 0) {
            nameTakenNum++;
            
        } else if (strcmp(buffer, "OK:") == 0) {
            
            break;
        }
        free_malloc_char_pointer(&buffer);
    }
}

/******************************************************************************
* Function Name  : handle_enter(char* buffer)
* Description    : Change the ENTER: message format from server and print it  
*                  out in stdout
* Input          : char* buffer;
* Return         : None
******************************************************************************/
void handle_enter(char* buffer) {
    char* broadcastMsg;
    split_buffer(buffer, &broadcastMsg);
    fprintf(stdout, "(%s has entered the chat)\n", broadcastMsg);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : handle_leave(char* buffer)
* Description    : Change the LEAVE: message format from server and print it  
*                  out in stdout
* Input          : char* buffer;
* Return         : None
******************************************************************************/
void handle_leave(char* buffer) {
    char* broadcastMsg;
    split_buffer(buffer, &broadcastMsg);
    fprintf(stdout, "(%s has left the chat)\n", broadcastMsg);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : handle_list(char* buffer)
* Description    : Change the LIST: message format from server and print it  
*                  out in stdout
* Input          : char* buffer;
* Return         : None
******************************************************************************/
void handle_list(char* buffer) {
    char* broadcastMsg;
    split_buffer(buffer, &broadcastMsg);
    fprintf(stdout, "(current chatters: %s)\n", broadcastMsg);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : message_handling(char** argv, FILE** pipes)
* Description    : Receive message from server and according to different   
*                  message take different actions
* Input          : char** argv;
*                  FILE** pipes
* Return         : None
******************************************************************************/
void message_handling(char** argv, FILE** pipes) {
    char* buffer = (char*)malloc(sizeof(char));

    while (1) {
        usleep(100000);
        if (read_fdin_dynamic(&buffer, pipes[1])) {
            communication_error();
        }
        //Server acknowledgement of a good NAME: or AUTH: response).
        if (strcmp(buffer, "OK:") == 0) {
            free_malloc_char_pointer(&buffer);
            continue;
        //broadcast name
        } else if (strncmp(buffer, "ENTER:", 6) == 0) {
            handle_enter(buffer);
        //send message to stderr
        } else if (strncmp(buffer, "MSG:", 4) == 0) {
            handle_msg_cmd(buffer);
        //send left client name to stderr    
        } else if (strncmp(buffer, "LEAVE:", 6) == 0) {
            handle_leave(buffer);

        //kicked by server and exit
        } else if (strcmp(buffer, "KICK:") == 0) {
            client_kicked();
        //print chatter list
        } else if (strncmp(buffer, "LIST:", 5) == 0) {
            handle_list(buffer);

        }
        free_malloc_char_pointer(&buffer);
    }
}

/******************************************************************************
* Function Name  : signal_handling()
* Description    : Ignore sigpipe signal. 
* Input          : None
* Return         : None
******************************************************************************/
void signal_handling() {
    // handle sigpipe
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sa, 0);
}

int main(int argc, char** argv) {
    FILE** pipes = (FILE**)malloc(sizeof(FILE*) * 2);
    pthread_t tid;
    // argument checking 
    arg_checking(argc, argv);
    
    // server connecting
    server_conn(argv, pipes);

    // handle sigpipe
    signal_handling();

    // authentication checking
    authentication_checking(pipes, argv);

    // username registration
    registering_username(pipes, argv);

    // stdin handling
    pthread_create(&tid, 0, stdin_handling, pipes);
    pthread_detach(tid);

    // communicate with server
    message_handling(argv, pipes);

    return 0;
}
