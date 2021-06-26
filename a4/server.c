#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "function.h"

/* A linked list structure */
typedef struct Node {
    // Client node have
    char* clientName; /* client name */
    FILE* readFD; /* pipe read file pointer */
    FILE* writeFD; /* pipe write file pointer */
    int* say; /* "SAY:" command receive times counter */
    int* kick; /* "KICK:" command receive times counter */
    int* list; /* "LIST:" command receive times counter */

    // Head node only
    char* authentication;
    int* auth; /* "AUTH:" command receive times counter */
    int* name; /* "NAME:" command receive times counter */
    int* leave; /* "LEAVE:" command receive times counter */
    struct Node* clients; /* Pointer point to the head node */

    // Thread using
    sem_t* guard; /* Thread lock */
    sigset_t signalSet; /* A data type is used to represent a signal set */

    // linking
    struct Node* next; /* Pointer point to the next Node */
} ClientList;

/******************************************************************************
* Function Name  : thread_communication_error()
* Description    : Report communication error in thread and sent message 
*                  "Communications error" to stderr, exit thread with code 2
* Input          : None
* Return         : None
******************************************************************************/
void thread_communication_error() {
    fprintf(stderr, "Communications error\n");
    fflush(stderr);
    pthread_exit((void*)2);
}

/******************************************************************************
* Function Name  : init_lock(sem_t* l)
* Description    : Initialize lock
* Input          : sem_t* l
* Return         : None
******************************************************************************/
void init_lock(sem_t* l) {
    sem_init(l, 0, 1);
}

/******************************************************************************
* Function Name  : take_lock(sem_t* l)
* Description    : start locking thread
* Input          : sem_t* l
* Return         : None
******************************************************************************/
void take_lock(sem_t* l) {
    sem_wait(l);
}

/******************************************************************************
* Function Name  : release_lock(sem_t* l)
* Description    : end locking thread
* Input          : sem_t* l
* Return         : None
******************************************************************************/
void release_lock(sem_t* l) {
    sem_post(l);
}

/******************************************************************************
* Function Name  : add_pipe(ClientList* clients, FILE* send, FILE* receive) 
* Description    : Add node to the linked list with parameters
*                  send and receive file pointer
* Input          : ClientList* clients;
*                  FILE* send; 
*                  FILE* receive;
* Return         : ClientList*
******************************************************************************/
ClientList* add_pipe(ClientList* clients, FILE* send, FILE* receive) {
    ClientList* current = clients;

    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = (ClientList*) calloc(1, sizeof(ClientList));
    current->next->writeFD = (FILE*)malloc(sizeof(FILE));
    current->next->readFD = (FILE*)malloc(sizeof(FILE));
    current->next->say = (int*)malloc(sizeof(int));
    current->next->kick = (int*)malloc(sizeof(int));
    current->next->list = (int*)malloc(sizeof(int));

    *(current->next->say) = 0;
    *(current->next->kick) = 0;
    *(current->next->list) = 0;
    current->next->writeFD = send;
    current->next->readFD = receive;
    current->next->clients = clients;
    current->next->next = NULL;

    return current->next;
}

/******************************************************************************
* Function Name  : remove_unnamed_client(ClientList* clients, 
*                  ClientList* currentClients)
* Description    : Remove node from the linked list according to
*                  currentClients memory address
* Input          : ClientList** clients;
*                  ClientList* currentClients; 
* Return         : None
******************************************************************************/
void remove_unnamed_client(ClientList* clients, ClientList* currentClients) {
    ClientList* current = clients;
    ClientList* tempNode = NULL;

    take_lock(clients->guard);
    if (current->next == NULL) {
        return;
    }
    
    while (current->next != currentClients) {
        if (current->next->next == NULL) {
            return;
        }
        current = current->next;
    }

    tempNode = current->next;   /* get tempNode for free */
    current->next = current->next->next; 
    tempNode = NULL;
    free(tempNode);
    release_lock(clients->guard);
}

/******************************************************************************
* Function Name  : remove_client(ClientList* clients, char* clientName)
* Description    : Remove node from the linked list according to
*                  client name provided
* Input          : ClientList** clients;
*                  char* clientName;
* Return         : None
******************************************************************************/
void remove_client(ClientList* clients, char* clientName) {
    ClientList* current = clients;
    ClientList* tempNode = NULL;

    take_lock(clients->guard);
    if (current->next == NULL) {
        return;
    }
    
    while (strcmp(current->next->clientName, clientName) != 0) {
        if (current->next->next == NULL) {
            return;
        }
        current = current->next;

    }

    tempNode = current->next;   /* get tempNode for free */
    current->next = current->next->next; 
    tempNode = NULL;
    free(tempNode);
    release_lock(clients->guard);
}

/******************************************************************************
* Function Name  : send_kick_msg(ClientList* clients, char* clientName)
* Description    : Sending "KICK:" command to the client with the client name
*                  provided
* Input          : ClientList** clients;
*                  char* clientName;
* Return         : None
******************************************************************************/
void send_kick_msg(ClientList* clients, char* clientName) {
    ClientList* current = clients;

    if (current->next == NULL) {
        return;
    }

    while (strcmp(current->next->clientName, clientName) != 0) {
        if (current->next->next == NULL) {
            return;
        }
        current = current->next;
    }

    fprintf(current->next->writeFD, "KICK:\n");
    fflush(current->next->writeFD);
}

/******************************************************************************
* Function Name  : args_error()
* Description    : Report server usage error with error message
*                  "Usage: server authfile [port]" with exit code 1
* Input          : None
* Return         : None
******************************************************************************/
void args_error() {
    fprintf(stderr, "Usage: server authfile [port]\n");
    fflush(stderr);
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

    if (argc != 2) {
        if (argc != 3) {
            args_error();
        }
    } else if (fp == NULL) {
        args_error();
    }
}

/******************************************************************************
* Function Name  : get_authentication(char** buffer, char* filePath)
* Description    : Get authentication from the file(file path provided)
* Input          : char** buffer;
*                  char* filePath;
* Return         : None
******************************************************************************/
void get_authentication(char** buffer, char* filePath) {
    FILE* fp = fopen(filePath, "r");
    read_file_dynamic(buffer, fp);
}

/******************************************************************************
* Function Name  : port_num_checking(char* port)
* Description    : Check if the port provided is valid
* Input          : char* port;
* Return         : None
******************************************************************************/
void port_num_checking(char* port) {
    int intPort = atoi(port);

    if (intPort < 1024 || intPort > 65535) {
        communication_error();
    }
}

/******************************************************************************
* Function Name  : create_socket(int argc, char** argv)
* Description    : Create TCP socket and print out port for client to connect
* Input          : int argc;
*                  char** argv;
* Return         : None
******************************************************************************/
int create_socket(int argc, char** argv) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    char* port;

    // See if a portnum was specified, otherwise use zero (ephemeral)
    if (argc == 2) {
        port = "0";
    } else {
        port_num_checking(argv[2]);
        port = argv[2];
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // Because we want to bind with it on all
                                // of our interfaces (if first argument
                                // to getaddrinfo() is NULL)
    int err;
    if ((err = getaddrinfo("localhost", port, &hints, &ai))) {
        // freeaddrinfo(ai);
        communication_error();
        // fprintf(stderr, "%s\n", gai_strerror(err));
        // return 1;   // could not work out the address
    }
        // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        communication_error();
    }

    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        communication_error();
    }

    fprintf(stderr, "%u\n", ntohs(ad.sin_port));
    fflush(stderr);

    if (listen(serv, 128)) {     // allow up to 10 connection requests to queue
        communication_error();
        // perror("Listen");
        // return 4;
    }

    return serv;
}

/******************************************************************************
* Function Name  : authentication_checking(FILE* send, FILE* receive, 
*                  ClientList* clients)
* Description    : Check if the authentication send from client is matching
*                  the authentication in server's authfile
* Input          : ClientList** clients;
*                  FILE* send;
*                  FILE* receive;
* Return         : authenticate sucess: return 0;
*                  authenticate fail: return 1;
******************************************************************************/
int authentication_checking(FILE* send, FILE* receive, ClientList* clients) {
    char* buffer = (char*)malloc(sizeof(char));
    char* auth;

    fprintf(send, "AUTH:\n");
    fflush(send);
    
    if (read_fdin_dynamic(&buffer, receive)) {
        return 1;
    }

    *(clients->auth) += 1;

    split_buffer(buffer, &auth);
    if (strcmp(auth, clients->authentication) != 0) {
        communication_error();
        return 1;
    }

    return 0;
}

/******************************************************************************
* Function Name  : check_name_taken(char* name, ClientList* clients)
* Description    : Check if name provided by client has taken
* Input          : ClientList** clients;
*                  char* name;
* Return         : Has taken: return 1;
*                  Has not taken: return 0;
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
* Function Name  : broadcast_client_enter(ClientList* currentClient)
* Description    : Broadcast the entered client name to all exist client
* Input          : ClientList* currentClient;
* Return         : None
******************************************************************************/
void broadcast_client_enter(ClientList* currentClient) {
    ClientList* current = currentClient->clients;
    current = current->next;

    take_lock(currentClient->clients->guard);

    while (current != NULL) {
        fprintf(current->writeFD, "ENTER:%s\n", currentClient->clientName);
        fflush(current->writeFD);
        current = current->next;
    }

    release_lock(currentClient->clients->guard);
}

/******************************************************************************
* Function Name  : collecting_client_name(ClientList* currentClient)
* Description    : Asking connected client name
* Input          : ClientList* currentClient;
* Return         : None
******************************************************************************/
void collecting_client_name(ClientList* currentClient) {
    char* buffer = (char*)malloc(sizeof(char));
    char* name;
    
    while (1) {
        usleep(100000);
        // ask client name by sending WHO:
        fprintf(currentClient->writeFD, "WHO:\n");
        fflush(currentClient->writeFD);
        // receive client response
        if (read_fdin_dynamic(&buffer, currentClient->readFD)) {
            fclose(currentClient->readFD);
            fclose(currentClient->writeFD);
            remove_unnamed_client(currentClient->clients, currentClient);
            thread_communication_error();
        }
        if (strncmp(buffer, "NAME:", 5) == 0) {
            split_buffer(buffer, &name);
            *(currentClient->clients->name) += 1;
        }
        // check if name has taken
        if (strlen(name) == 0) {
            fprintf(currentClient->writeFD, "NAME_TAKEN:\n");
            fflush(currentClient->writeFD);
            free_malloc_char_pointer(&buffer);
            free(name);
            continue;
        }
        if (check_name_taken(name, currentClient->clients) == 0) {
            break;
        }

        fprintf(currentClient->writeFD, "NAME_TAKEN:\n");
        fflush(currentClient->writeFD);
        free_malloc_char_pointer(&buffer);
        free(name);
    }
    //store name in client node
    take_lock(currentClient->clients->guard);
    currentClient->clientName = 
            (char*)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(currentClient->clientName, name);
    release_lock(currentClient->clients->guard);

    fprintf(currentClient->writeFD, "OK:\n");
    fflush(currentClient->writeFD);
    printf("(%s has entered the chat)\n", currentClient->clientName);
    fflush(stdout);
    broadcast_client_enter(currentClient);
}

/******************************************************************************
* Function Name  : broadcast_client_say(ClientList* currentClient, 
*                  char* content)
* Description    : Broadcast the client "SAY:" message to all exist client
* Input          : ClientList* currentClient;
*                  char* content;
* Return         : None
******************************************************************************/
void broadcast_client_say(ClientList* currentClient, char* content) {
    ClientList* current = currentClient->clients;
    current = current->next;

    take_lock(currentClient->clients->guard);
    while (current != NULL) {
        fprintf(current->writeFD, "MSG:%s:%s\n", currentClient->clientName,
                content);
        fflush(current->writeFD);
        current = current->next;
    }
    release_lock(currentClient->clients->guard);
    fprintf(stdout, "%s: %s\n", currentClient->clientName, content);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : handle_say_cmd(char* buffer, ClientList* currentClient)
* Description    : Handle the client "SAY:" message and take action
* Input          : ClientList* currentClient;
*                  char* buffer;
* Return         : None
******************************************************************************/
void handle_say_cmd(char* buffer, ClientList* currentClient) {
    char* content;

    split_buffer(buffer, &content);

    broadcast_client_say(currentClient, content);
}

/******************************************************************************
* Function Name  : cmp(const void * a,const void *b)
* Description    : Broadcast the client "SAY:" message to all exist client
* Input          : const void * a;
*                  const void *b;
* Return         : int
* Reference      : Assignment One solution search.c
******************************************************************************/
int cmp(const void* a, const void* b) {
    const char* str1 = *((char**)a);
    const char* str2 = *((char**)b);

    /* While characters remain in both strings, keep iterating. We
    ** do this until we run out characters in one string - or
    ** terminate prematurely and return if a difference is found
    ** between the strings. We could have used strcasecmp() here, 
    ** e.g. return strcasecmp(str1, str2);
    */
    while (*str1 && *str2) {
	/* Get the next character (as an int) of the two strings 
	*/
	int c1 = tolower((int)*str1);
	int c2 = tolower((int)*str2);
	if (c1 != c2) {
	    /* Next characters are different. Return negative or positive
	    ** number as required - we do this just with a subtraction.
	    */
	    return c1 - c2;
	} else {
	    /* Move on to next characters in each string.
	    */
	    str1++;
	    str2++;
	}
    } 

    /* If we get here, then we have exhausted one or both strings.
    */
    if (*str1) {
	return 1;	// Ran out of string 2 - string 1 is greater
    } else if (*str2) {
	return -1;	// Ran out of string 1 - string 2 is greater
    } else {
	return 0;	// Ran out of both strings - they are the same.
    }
}

/******************************************************************************
* Function Name  : collect_client_name(char** *clientName, 
*                  ClientList* currentClient)
* Description    : Collect client names from client linked list to a Char**
* Input          : char** *clientName;
*                  ClientList* currentClient
* Return         : int*
******************************************************************************/
int* collect_client_name(char*** clientName, ClientList* currentClient) {
    ClientList* current = currentClient->clients;

    if (current->next == NULL) {
        int* data = (int*)malloc(sizeof(int) * 2);
        data[0] = 0;
        data[1] = 0;
        return data;
    }

    current = current->next;
    *clientName = (char**)malloc(sizeof(char*));
    int arrayIndex = 0;
    int nameStrLength = 0;

    take_lock(currentClient->clients->guard);

    while (current != NULL) {
        (*clientName)[arrayIndex] = (char*)malloc(sizeof(char) * 
                (strlen(current->clientName) + 1));

        memset((*clientName)[arrayIndex], '\0', 
                (strlen(current->clientName) + 1));

        strcpy((*clientName)[arrayIndex], current->clientName);
        arrayIndex++;
        nameStrLength += strlen(current->clientName);

        *clientName = (char**)realloc((*clientName), 
                sizeof(char*) * (arrayIndex + 1));

        current = current->next;
    }

    release_lock(currentClient->clients->guard);

    int* data = (int*)malloc(sizeof(int) * 2);
    data[0] = nameStrLength;
    data[1] = arrayIndex;

    return data;
}

/******************************************************************************
* Function Name  : handle_list_cmd(ClientList* currentClient)
* Description    : Reply client request by a comma seperated list containing 
*                  the names of all current chatters, in lexicographical order
* Input          : ClientList* currentClient;
* Return         : None
******************************************************************************/
void handle_list_cmd(ClientList* currentClient) {
    char** clientName;
    int* data = (int*)malloc(sizeof(int) * 2);

    data = collect_client_name(&clientName, currentClient);

    qsort(clientName, data[1], sizeof(char*), cmp);

    char* strToPrint = 
            (char*)malloc(sizeof(char) * (data[0] + data[1] + 1));

    memset(strToPrint, '\0', data[0] + data[1] + 1);

    char* commaName = (char*)malloc(sizeof(char));

    for (int i = 0; i < data[1]; i++) {
        if (i == 0) {
            sprintf(strToPrint, "%s", clientName[i]);
        } else {
            commaName = (char*)realloc(commaName, sizeof(char) * 
                    (strlen(clientName[i]) + 2));
            sprintf(commaName, ",%s", clientName[i]);
            strcat(strToPrint, commaName);
        }
    }
    
    ClientList* current = currentClient->clients;
    current = current->next;

    take_lock(currentClient->clients->guard);
    while (current != NULL) {
        fprintf(current->writeFD, "LIST:%s\n", strToPrint);
        fflush(current->writeFD);
        current = current->next;
    }
    release_lock(currentClient->clients->guard);
}

/******************************************************************************
* Function Name  : broadcast_client_leave(ClientList* currentClient,
*                  char* leaveName)
* Description    : Broadcast the client "LEAVE:" message to all exist client
* Input          : ClientList* currentClient;
*                  char* leaveName;
* Return         : None
******************************************************************************/
void broadcast_client_leave(ClientList* currentClient, char* leaveName) {
    ClientList* current = currentClient->clients;
    current = current->next;

    take_lock(currentClient->clients->guard);

    while (current != NULL) {
        if (strcmp(current->clientName, leaveName) != 0) {
            fprintf(current->writeFD, "LEAVE:%s\n", leaveName);
            fflush(current->writeFD);
        }
        current = current->next;
    }

    release_lock(currentClient->clients->guard);
    
    printf("(%s has left the chat)\n", leaveName);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : handle_kick_cmd(char* buffer, ClientList* currentClient)
* Description    : Send "KICK:" command to the kick target client
* Input          : ClientList* currentClient;
*                  char* buffer;
* Return         : None
******************************************************************************/
void handle_kick_cmd(char* buffer, ClientList* currentClient) {
    char* kickedName;
    split_buffer(buffer, &kickedName);
    ClientList* clients = currentClient->clients;

    take_lock(clients->guard);

    send_kick_msg(currentClient->clients, kickedName);

    release_lock(clients->guard);
    // remove_client(clients, kickedName);
    // broadcast_client_leave(clients, kickedName);
}

/******************************************************************************
* Function Name  : msg_cmd_accepting(ClientList* currentClient)
* Description    : Receive message from client and take corresponding actions
* Input          : ClientList* currentClient;
* Return         : None
******************************************************************************/
void msg_cmd_accepting(ClientList* currentClient) {
    char* buffer = (char*)malloc(sizeof(char));

    while (1) {
        usleep(100000);
        // read message
        if (read_fdin_dynamic(&buffer, currentClient->readFD)) {
            broadcast_client_leave(currentClient, currentClient->clientName);
            remove_client(currentClient->clients, currentClient->clientName);
            pthread_exit((void*)2);  
        }
        // handle SAY:
        if (strncmp(buffer, "SAY:", 4) == 0) {
            handle_say_cmd(buffer, currentClient);
            *(currentClient->say) += 1;
            *(currentClient->clients->say) += 1;
        // handle LIST:
        } else if (strcmp(buffer, "LIST:") == 0) {
            handle_list_cmd(currentClient);
            *(currentClient->list) += 1;
            *(currentClient->clients->list) += 1;
        // handle KICK:
        } else if (strncmp(buffer, "KICK:", 5) == 0) {
            handle_kick_cmd(buffer, currentClient);
            *(currentClient->kick) += 1;
            *(currentClient->clients->kick) += 1;
        // handle LEAVE:
        } else if (strcmp(buffer, "LEAVE:") == 0) {
            *(currentClient->clients->leave) += 1;
            broadcast_client_leave(currentClient, currentClient->clientName);
            remove_client(currentClient->clients, currentClient->clientName);
            pthread_exit(0);
        }

        free_malloc_char_pointer(&buffer);
        
    }
}

/******************************************************************************
* Function Name  : chating_time(void* v)
* Description    : Server and client communication
* Input          : void* v;
* Return         : None
******************************************************************************/
void* chating_time(void* v) {
    ClientList* currentClient = (ClientList*)v;
    
    collecting_client_name(currentClient);

    msg_cmd_accepting(currentClient);

    return 0;
}

/******************************************************************************
* Function Name  : client_listener(int socket, ClientList* clients)
* Description    : Listening client which trying to connect this server
* Input          : int socket;
*                  ClientList* clients;
* Return         : None
******************************************************************************/
void client_listener(int socket, ClientList* clients) {
    int connFd;
    
    // change 0, 0 to get info about other end
    while (connFd = accept(socket, 0, 0), connFd >= 0) {   
        int fd2 = dup(connFd);
        FILE* send = fdopen(connFd, "w");
        FILE* receive = fdopen(fd2, "r");
        // Authentication check
        if (authentication_checking(send, receive, clients)) {
            // fail authentication
            fclose(send);
            fclose(receive);
            continue;
        } else {
            // pass authentication
            fprintf(send, "OK:\n");
            fflush(send);
        }
        
        // store client information to node
        ClientList* currentClient = NULL;
        currentClient = (ClientList*) calloc(1, sizeof(ClientList));
        currentClient = add_pipe(clients, send, receive);

        // begin communicating
        pthread_t tid;
        pthread_create(&tid, 0, chating_time, currentClient);
        pthread_detach(tid);
    }
}

/******************************************************************************
* Function Name  : find_client_node(ClientList* clients, char* clientName)
* Description    : Find the client node from linked list according to the given
*                  client name
* Input          : char* clientName;
*                  ClientList* clients;
* Return         : ClientList*
******************************************************************************/
ClientList* find_client_node(ClientList* clients, char* clientName) {
    ClientList* current = clients;

    while (strcmp(current->next->clientName, clientName) != 0) {
        current = current->next;
    }
    
    return current->next;
}

/******************************************************************************
* Function Name  : signal_handler(ClientList* clients)
* Description    : Emit to stderr a table of statistics relating to the 
*                  current session
* Input          : char* clientName;
* Return         : None
******************************************************************************/
void signal_handler(ClientList* clients) {
    char** clientName;
    int* data = (int*)malloc(sizeof(int) * 2);

    data = collect_client_name(&clientName, clients);
    qsort(clientName, data[1], sizeof(char*), cmp);

    fprintf(stderr, "@CLIENTS@\n");
    fflush(stderr);
    for (int i = 0; i < data[1]; i++) {

        ClientList* currentClient = NULL;
        currentClient = (ClientList*) calloc(1, sizeof(ClientList));
        currentClient = find_client_node(clients, clientName[i]);

        fprintf(stderr, "%s:SAY:%d:KICK:%d:LIST:%d\n", 
                currentClient->clientName, *(currentClient->say), 
                *(currentClient->kick), *(currentClient->list));

        fflush(stderr);
    }

    fprintf(stderr, "@SERVER@\n");
    fflush(stderr);

    fprintf(stderr, "server:AUTH:%d:NAME:%d:SAY:%d:KICK:%d:LIST:%d:LEAVE:%d\n",
            *(clients->auth), *(clients->name), *(clients->say), 
            *(clients->kick), *(clients->list), *(clients->leave));   
    fflush(stderr);
}

/******************************************************************************
* Function Name  : signal_waiter (void* v)
* Description    : Receiving signal and taking corresponding actions
* Input          : void* v;
* Return         : None
******************************************************************************/
void* signal_waiter(void* v) {
    ClientList* clients = (ClientList*) v;
    int sigNum;

    while (1) {
        sigwait(&(clients->signalSet), &sigNum);
        switch (sigNum) {
            case SIGPIPE:
                break;
            case SIGHUP: 
                signal_handler(clients);
                break;
        }
    }

}

/******************************************************************************
* Function Name  : sighup_sigpipe_handling(ClientList* clients)
* Description    : Block SIGPIPE and SIGHUP, and take corresponding actions
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void sighup_sigpipe_handling(ClientList* clients) {
    pthread_t tid;      /* signal handler thread ID */

    sigemptyset(&(clients->signalSet));
    sigaddset(&(clients->signalSet), SIGPIPE);
    sigaddset(&(clients->signalSet), SIGHUP);
    pthread_sigmask(SIG_BLOCK, &(clients->signalSet), NULL);

    pthread_create(&tid, NULL, signal_waiter, clients);
    pthread_detach(tid);
}

/******************************************************************************
* Function Name  : linked_list_head_initialization(ClientList* clients)
* Description    : Initializing client linked list head node
* Input          : ClientList* clients;
* Return         : None
******************************************************************************/
void linked_list_head_initialization(ClientList* clients) {
    clients->authentication = (char*)malloc(sizeof(char));
    clients->say = (int*)malloc(sizeof(int));
    clients->kick = (int*)malloc(sizeof(int));
    clients->list = (int*)malloc(sizeof(int));
    clients->auth = (int*)malloc(sizeof(int));
    clients->name = (int*)malloc(sizeof(int));
    clients->leave = (int*)malloc(sizeof(int));
    *(clients->say) = 0;
    *(clients->kick) = 0;
    *(clients->list) = 0;
    *(clients->auth) = 0;
    *(clients->name) = 0;
    *(clients->leave) = 0;
    sem_t l;
    init_lock(&l);
    clients->guard = (sem_t*)malloc(sizeof(sem_t));
    clients->guard = &l;
    clients->clients = clients;
}

int main(int argc, char** argv) {
    int socket;
    ClientList* clients = NULL;
    clients = (ClientList*) calloc(1, sizeof(ClientList));

    // initialize head node of the clients linked list
    linked_list_head_initialization(clients);

    // handle sighup and sigpipe
    sighup_sigpipe_handling(clients);

    // argument checking 
    arg_checking(argc, argv);

    // print out port
    socket = create_socket(argc, argv);

    // get authentication content
    get_authentication(&(clients->authentication), argv[1]);

    // listen client
    client_listener(socket, clients);

    return 0;
}
