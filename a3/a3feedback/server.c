#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "function.h"

/* A linked list structure */
typedef struct Node {
    char* run; /* a program path */
    char* fileToRun; /* execute file path */
    char* clientName; /* client name */
    FILE* readFD; /* pipe read file pointer */
    FILE* writeFD; /* pipe write file pointer */
    struct Node* next; /* Pointer point to the next Node */
} ClientList;

/******************************************************************************
* Function Name  : print_clients(ClientList* clients)
* Description    : Print client one by one with client name and execute file
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void print_clients(ClientList* clients) {
    ClientList* current = clients;
    int i = 0;

    while (current != NULL) {
        fprintf(stderr, "%dclientname: %s\n", i, current->run);
        fprintf(stderr, "%dfileToRun: %s\n", i, current->fileToRun);
        current = current->next;
        i++;
    }
}

/******************************************************************************
* Function Name  : add_run_files(ClientList* clients, char* run, 
*                  char* fileToRun) 
* Description    : Add node to the linked list with parameters
*                  run(a program path) and fileToRun
* Input          : ClientList* clients;
*                  char* run; 
*                  char* fileToRun;
* Return         : None
******************************************************************************/
void add_run_files(ClientList* clients, char* run, char* fileToRun) {
    ClientList* current = clients;

    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = (ClientList*) calloc(1, sizeof(ClientList));
    current->next->fileToRun = 
            (char*)malloc(sizeof(char) * (strlen(fileToRun) + 1));
    current->next->run = (char*)malloc(sizeof(char) * (strlen(run) + 1));
    memset(current->next->fileToRun, '\0', (strlen(fileToRun) + 1));
    memset(current->next->run, '\0', (strlen(run) + 1));
    stpcpy(current->next->run, run);
    stpcpy(current->next->fileToRun, fileToRun);
    current->next->next = NULL;
}

/******************************************************************************
* Function Name  : remove_client_run(ClientList** clients, char* run)
* Description    : Remove node from the linked list according to
*                  run(which is a program path)
* Input          : ClientList** clients;
*                  char* run; 
* Return         : None
******************************************************************************/
void remove_client_run(ClientList** clients, char* run) {
    ClientList* current = *clients;
    ClientList* tempNode = NULL;

    if (current->next == NULL) {
        return;
    }
    /* Search until the next one is the value "n" */
    while (strcmp(current->next->run, run) != 0) {
        current = current->next;
    }
    
    tempNode = current->next;   /* get tempNode for free */
    current->next = current->next->next;
    tempNode = NULL;
    free(tempNode);
}

/******************************************************************************
* Function Name  : remove_client(ClientList** clients, char* clientName) 
* Description    : Remove node from the linked list according to client name
* Input          : ClientList** clients;
*                  char* clientName; 
* Return         : None
******************************************************************************/
void remove_client(ClientList** clients, char* clientName) {
    ClientList* current = *clients;
    ClientList* tempNode = NULL;

    if (current->next == NULL) {
        return;
    }
    /* Search until the next one is the value "n" */
    while (strcmp(current->next->clientName, clientName) != 0) {
        current = current->next;
    }
    
    tempNode = current->next;   /* get tempNode for free */
    current->next = current->next->next; 
    tempNode = NULL;
    free(tempNode);
}

/******************************************************************************
* Function Name  : args_error()
* Description    : Report client usage error with error message
*                  "Usage: server configfile" with exit code 1
* Input          : None
* Return         : None
******************************************************************************/
void args_error() {
    fprintf(stderr, "Usage: server configfile\n");
    exit(1);
}

/******************************************************************************
* Function Name  : arg_checking(int argc, char** argv)
* Description    : Check if the arguments input in command line is correct
* Input          : int argc;
*                  char** argv;
* Return         : None
******************************************************************************/
void arg_checking(int argc, char** argv) {
    FILE* fp = fopen(argv[1], "r");

    if (argc != 2 || fp == NULL) {
        args_error();
    }
}

/******************************************************************************
* Function Name  : read_fdin_dynamic(char** buffer, FILE* readFD)
* Description    : Read the pipe file pointer dynamicly and
*                  store the content in the buffer 
* Input          : char** buffer;
*                  FILE* readFD;
* Return         : None
******************************************************************************/
void read_fdin_dynamic(char** buffer, FILE* readFD) {
    char c;
    int shiftIndex = 0;

    while ((c = fgetc(readFD)) != EOF) {
        if (c == '\n') {
            (*buffer)[shiftIndex] = '\0';
            break;
        }
        (*buffer)[shiftIndex] = c;
        shiftIndex++;
        *buffer = (char*)realloc(*buffer, sizeof(char) * (shiftIndex + 1));
    }
}

/******************************************************************************
* Function Name  : collect_information(char* buffer, ClientList* clients)
* Description    : Split the message store in buffer, get the information of
*                  run and fileToRun and store these to the clients linked list
* Input          : char* buffer;
*                  ClientList* clients; 
* Return         : None
******************************************************************************/
void collect_information(char* buffer, ClientList* clients) {
    char* run = (char*)malloc(sizeof(char) * (strlen(buffer)));
    char* fileToRun = (char*)malloc(sizeof(char) * (strlen(buffer)));
    memset(run, '\0', strlen(buffer));
    memset(fileToRun, '\0', strlen(buffer));

    if (check_contain_colon(buffer)) {
        sscanf(buffer, "%[^:]:%[^\n]", run, fileToRun);
        add_run_files(clients, run, fileToRun);
    }
}

/******************************************************************************
* Function Name  : read_file(char* filePath, ClientList* clients)
* Description    : Read the execute file ignore lines with "#"
*                  or with no colon in the line, according to
*                  the number of valid line creat client node
*                  in the clients linked list stimulus and response
* Input          : char* filePath;
*                  ClientList* clients; 
* Return         : None
******************************************************************************/
void read_file(char* filePath, ClientList* clients) {
    ClientList* current = clients;
    char* buffer = (char*)malloc(sizeof(char));
    int validLine = 0;
    FILE* fp = fopen(filePath, "r");
    read_file_dynamic(&buffer, fp);

    while (!feof(fp)) {
        if (strncmp(buffer, "#", 1) != 0) {
            collect_information(buffer, current);
            validLine++;
        }
        free(buffer);
        buffer = malloc(sizeof(char) + 1);
        read_file_dynamic(&buffer, fp);
    }

    if (strncmp(buffer, "#", 1) != 0) {
        collect_information(buffer, current);
        validLine++;
    }
    
    if (validLine == 0) {
        exit(0);
    }
}

/******************************************************************************
* Function Name  : open_socket(ClientList* clients) 
* Description    : Create child process according to the clients linked
*                  list pipe between parent process and child process,
*                  store the read and write file pointer to 
*                  the clients linked list
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void open_socket(ClientList* clients) {
    ClientList* current = clients;
    int fdOne[2]; //send msg to child
    int fdTwo[2]; //receive msg from child

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sa, 0);
    sigaction(SIGCHLD, &sa, 0);

    current = current->next;
    while (current != NULL) {

        pipe(fdOne); /* read from 0 write in 1 */
        pipe(fdTwo);
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "create process fail\n");

        } else if (pid == 0) {
            // child read from stdin
            close(fdOne[1]);
            dup2(fdOne[0], 0); // 0 -> stdin's file pointer
            close(fdOne[0]);
            // child write in stdout
            close(fdTwo[0]);
            dup2(fdTwo[1], 1); // 1 -> stdout's file pointer
            close(fdTwo[1]);
            
            int fdError = open("/dev/null", O_WRONLY);
            dup2(fdError, 2);
            execlp(current->run, current->run, current->fileToRun, NULL);

        } else {
            // parent send msg to child on fdOne
            close(fdOne[0]);
            current->writeFD = fdopen(fdOne[1], "w");
            // parent read msg from child on fdTwo
            close(fdTwo[1]);
            current->readFD = fdopen(fdTwo[0], "r");
        }

        current = current->next;
    }
}

/******************************************************************************
* Function Name  : check_name_taken(char* name, ClientList* clients)
* Description    : Check if the name sent by client has taken,
*                  if it has taken then return nameTaken = 1,
*                  if not return 0
* Input          : char* name;
*                  ClientList* clients;
* Return         : nameTaken;
******************************************************************************/
int check_name_taken(char* name, ClientList* clients) {
    int nameTaken = 0;
    ClientList* current = clients;
    current = current->next;
    while (current != NULL) {

        if (current->clientName == NULL) {
            break;
        }

        if (strcmp(current->clientName, name) == 0) {
            nameTaken = 1;
            break;
        }

        current = current->next;
    }
    return nameTaken;
}

/******************************************************************************
* Function Name  : split_buffer(char* buffer, char** content) 
* Description    : Split the text in buffer and store the useful
*                  message which is after colon into content
*                  stimulus and response
* Input          : char* buffer;
*                  char** content;
* Return         : None
******************************************************************************/
void split_buffer(char* buffer, char** content) {
    char* rubbish = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
    *content = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
    memset((*content), '\0', (strlen(buffer) + 1));
    if (check_contain_colon(buffer)) {
        
        sscanf(buffer, "%[^:]:%[^\n]", rubbish, *content);
    }   
}

/******************************************************************************
* Function Name  : collect_client_name(ClientList* clients)
* Description    : Ask each client name by sending "WHO:" and
*                  check the name send back from client whether 
*                  has taken or not, if not store client name
*                  in the corresponding client node
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void collect_client_name(ClientList* clients) {
    ClientList* current = clients;
    char* buffer;
    char* name;
    
    current = current->next;
    while (current != NULL) {
        while (1) {
            // ask client name by sending WHO:
            buffer = (char*)malloc(sizeof(char));
            fprintf(current->writeFD, "WHO:\n");
            fflush(current->writeFD);
            // receive client response
            read_fdin_dynamic(&buffer, current->readFD);
            split_buffer(buffer, &name);
            // check if name has taken
            if (strlen(name) == 0) {
                break;
            }
            if (check_name_taken(name, clients) == 0) {
                break;
            }
            fprintf(current->writeFD, "NAME_TAKEN:\n");
            fflush(current->writeFD);
            free(buffer);
            free(name);
        }
        //handle command like cat, ls
        if (strlen(name) == 0) {
            remove_client_run(&clients, current->run);
        } else {
            //store name in client node
            current->clientName = 
                    (char*)malloc(sizeof(char) * (strlen(name) + 1));
            strcpy(current->clientName, name);
            printf("(%s has entered the chat)\n", current->clientName);
            free(name);
        }
        
        free(buffer);
        current = current->next;
    }
}

/******************************************************************************
* Function Name  : kick_notification(ClientList* clients, char* kickName)
* Description    : Send the "KICK:" message to the client who is 
*                  kicked by other client
* Input          : ClientList* clients;
*                  char* kickName;
* Return         : None
******************************************************************************/
void kick_notification(ClientList* clients, char* kickName) {
    ClientList* current = clients;
    current = current->next;

    while (strcmp(current->clientName, kickName) != 0) {
        current = current->next;
    }

    fprintf(current->writeFD, "KICK:\n");
    fflush(current->writeFD);
}

/******************************************************************************
* Function Name  : send_msg_to_clients(ClientList* clients, char* sender,
*                  char* message)
* Description    : Send broadcast message to all clients
* Input          : ClientList* clients;
*                  char* sender; 
*                  char* message;
* Return         : None
******************************************************************************/
void send_msg_to_clients(ClientList* clients, char* sender, char* message) {
    ClientList* current = clients;
    current = current->next;

    char* wholeMsg = 
            (char*)malloc(sizeof(char) * 
            (strlen(sender) + strlen(message) + 7));

    sprintf(wholeMsg, "MSG:%s:%s\n", sender, message);

    while (current != NULL) {
        fprintf(current->writeFD, wholeMsg);
        fflush(current->writeFD);
        current = current->next;
    }
}

/******************************************************************************
* Function Name  : take_action(ClientList* clients, ClientList* current)
* Description    : Receive messages from clients and take corresponding actions
* Input          : ClientList* clients;
*                  ClientList* current;
* Return         : None
******************************************************************************/
void take_action(ClientList* clients, ClientList* current){
    char* buffer; //receive message send from client
    char* message; //pure message without "CHAT:"
    char* kickName; //name going to be kicked

    while (1) {
        buffer = (char*)malloc(sizeof(char));
        memset(buffer, '\0', 1);
        //reading pipe read port
        read_fdin_dynamic(&buffer, current->readFD);
        //remove error client or client executing command like cat, ls
        if (strlen(buffer) == 0) {
            printf("(%s has left the chat)\n", current->clientName);
            remove_client(&clients, current->clientName);
            break;
        }
        //handle CHAT:
        if (strncmp(buffer, "CHAT:", 5) == 0) {
            split_buffer(buffer, &message);
            printf("(%s) %s\n", current->clientName, message);
            send_msg_to_clients(clients, current->clientName, message);
            free(message);
        //handle KICK:
        } else if (strncmp(buffer, "KICK:", 5) == 0) {
            split_buffer(buffer, &kickName);
            printf("(%s has left the chat)\n", kickName);
            kick_notification(clients, kickName);
            remove_client(&clients, kickName);
            free(kickName);
        //handle DONE:
        } else if (strcmp(buffer, "DONE:") == 0) {
            break;
        //handle QUIT:
        } else if (strcmp(buffer, "QUIT:") == 0) {
            printf("(%s has left the chat)\n", current->clientName);
            remove_client(&clients, current->clientName);
            break;
        //handle error client
        } else {
            printf("(%s has left the chat)\n", current->clientName);
            remove_client(&clients, current->clientName);
            break;
        }
        free(buffer);
    }   
}

/******************************************************************************
* Function Name  : chating_time(ClientList* clients)
* Description    : Send "YT:" to each client
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void chating_time(ClientList* clients) {
    ClientList* current = clients;

    while (1) {
        current = current->next;

        //send "YT:" to each client
        while (current != NULL) {
            fprintf(current->writeFD, "YT:\n");
            fflush(current->writeFD);
            
            //take different action according to different response from client
            take_action(clients, current);

            current = current->next;
        }
    
        current = clients;

        //check if all client quit
        if (clients->next == NULL) {
            break;
        }
    }
}

int main(int argc, char** argv) {
    ClientList* clients = NULL;
    clients = (ClientList*) calloc(1, sizeof(ClientList));
    
    // argument checking 
    arg_checking(argc, argv);
    
    // file reading and information collecting
    read_file(argv[1], clients);

    // create multi-progress
    open_socket(clients);

    // handshaking
    collect_client_name(clients);
    chating_time(clients);

    //valgrind -s --track-origins=yes ./server config.txt
    return 0;
}
