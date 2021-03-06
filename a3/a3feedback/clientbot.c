#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "function.h"

/* A linked list structure */
typedef struct Node {
    char* stimulus; /* stimulus */
    char* response; /* response */
    struct Node* next; /* Pointer point to the next Node */
} Dictionary;

/******************************************************************************
* Function Name  : print_head(Dictionary* head)
* Description    : Print the linked list node by node with 
*                  stiumulus and response
* Input          : Dictionary* head;
* Return         : None
******************************************************************************/
void print_head(Dictionary* head) {
    Dictionary* current = head;
    int i = 0;

    while (current != NULL) {
        printf("%dstimulus: %s\n", i, current->stimulus);
        printf("%dresponse: %s\n", i, current->response);
        current = current->next;
        i++;
    }
}

/******************************************************************************
* Function Name  : print_reply(Dictionary* reply)
* Description    : Print the linked list node by node with  
*                  all response clientbot need to reply
* Input          : Dictionary* reply;
* Return         : None
******************************************************************************/
void print_reply(Dictionary* reply) {
    Dictionary* current = NULL;
    current = (Dictionary*) calloc(1, sizeof(Dictionary));
    current = reply;

    while (current != NULL) {
        if (current->response != NULL) {
            printf("CHAT:%s\n", current->response);
            fflush(stdout);
        }
        current = current->next;
    }
}

/******************************************************************************
* Function Name  : add_node(Dictionary* head, char* stimulus, char* response)
* Description    : Add node to the linked list with parameters
*                  stimulus and response
* Input          : Dictionary* head;
*                  char* stimulus; 
*                  char* response;
* Return         : None
******************************************************************************/
void add_node(Dictionary* head, char* stimulus, char* response) {
    Dictionary* current = head;

    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = (Dictionary*) malloc(sizeof(Dictionary));
    current->next->response = 
            (char*)malloc(sizeof(char) * (strlen(response) + 1));
    current->next->stimulus = 
            (char*)malloc(sizeof(char) * (strlen(stimulus) + 1));
    memset(current->next->response, '\0', (strlen(response) + 1));
    memset(current->next->stimulus, '\0', (strlen(stimulus) + 1));
    stpcpy(current->next->stimulus, stimulus);
    stpcpy(current->next->response, response);
    current->next->next = NULL;
}

/******************************************************************************
* Function Name  : add_head(Dictionary* head, char* stimulus, char* response)
* Description    : Add head node with parameters stimulus and response
* Input          : Dictionary* head;
*                  char* stimulus; 
*                  char* response;
* Return         : None
******************************************************************************/
void add_head(Dictionary* head, char* stimulus, char* response) {
    head->response = (char*)malloc(sizeof(char) * (strlen(response) + 1));
    head->stimulus = (char*)malloc(sizeof(char) * (strlen(stimulus) + 1));
    memset(head->response, '\0', (strlen(response) + 1));
    memset(head->stimulus, '\0', (strlen(stimulus) + 1));
    stpcpy(head->stimulus, stimulus);
    stpcpy(head->response, response);
    head->next = NULL;
}

/******************************************************************************
* Function Name  : free_linked_list(Dictionary* head)
* Description    : Free a linked list  
* Input          : Dictionary* head;
* Return         : None
******************************************************************************/
void free_linked_list(Dictionary* head) {
    Dictionary* current = head;
    Dictionary* temp = NULL;
    // head = head->next;
    while (current != NULL) {
        temp = current->next;
        free(current);
        current = temp;
    }
}

/******************************************************************************
* Function Name  : to_lower_case(char* string)
* Description    : replace all characters in the string to lower case
* Input          : char* string;
* Return         : None
******************************************************************************/
void to_lower_case(char* string) {
    for (int i = 0; i < strlen(string); i++) {
        // turn all characters into lower case except '?'
        if (isalpha(string[i])) {
            string[i] = (char)tolower((int)string[i]);
        }
    }
}

/******************************************************************************
* Function Name  : args_error()
* Description    : Report client usage error with error message
*                  "Usage: clientbot responsefile" with exit code 1
* Input          : None
* Return         : None
******************************************************************************/
void args_error() {
    fprintf(stderr, "Usage: clientbot responsefile\n");
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
* Function Name  : name_reply(int* nameTakenNum, char** name) 
* Description    : According to the nameTakenNum, reply clientbot 
*                  name to server 
* Input          : int* nameTakenNum;
*                  char** name;
* Return         : None
******************************************************************************/
void name_reply(int* nameTakenNum, char** name) {
    int numLength = 0;

    if (*nameTakenNum == -1) {
        printf("NAME:clientbot\n");
        fflush(stdout);
        strcpy(*name, "clientbot");
    } else {
        printf("NAME:clientbot%d\n", *nameTakenNum);
        fflush(stdout);
        int_length(nameTakenNum, &numLength);
        free(*name);
        *name = (char*)malloc(sizeof(char) * (numLength + 7));
        sprintf(*name, "clientbot%d", *nameTakenNum);
    }
}

/******************************************************************************
* Function Name  : collect_stimulus_response
*                  (char* buffer, int* ifhead, Dictionary* head)
* Description    : Collect stimulus and response and add them to linked list
* Input          : char* buffer;
*                  int* ifhead;
*                  Dictionary* head;
* Return         : None
******************************************************************************/
void collect_stimulus_response(char* buffer, int* ifhead, Dictionary* head) {
    char* response = (char*)malloc(sizeof(char) * (strlen(buffer)));
    char* stimulus = (char*)malloc(sizeof(char) * (strlen(buffer)));
    memset(response, '\0', strlen(buffer));
    memset(stimulus, '\0', strlen(buffer));

    if (check_contain_colon(buffer)) {
        sscanf(buffer, "%[^:]:%[^\n]", stimulus, response);
        if ((*ifhead) == 0) {
            add_head(head, stimulus, response);
        } else {
            add_node(head, stimulus, response);
        }
        (*ifhead)++;
    }
    
}

/******************************************************************************
* Function Name  : read_script(FILE* fp, Dictionary* head)
* Description    : Read the file run by clientbot and accoding to 
*                  the content of each line in the file if line is valid,
*                  collect the stimulus and response
* Input          : FILE* fp;
*                  Dictionary* head;
* Return         : None
******************************************************************************/
void read_script(FILE* fp, Dictionary* head) {
    char* buffer = (char*)malloc(sizeof(char));
    read_file_dynamic(&buffer, fp);
    int ifhead = 0;

    while (!feof(fp)) {
        if (strncmp(buffer, "#", 1) != 0) {
            collect_stimulus_response(buffer, &ifhead, head);
        }
        free(buffer);
        buffer = malloc(sizeof(char) + 1);
        read_file_dynamic(&buffer, fp);
    }
    collect_stimulus_response(buffer, &ifhead, head);
}

/******************************************************************************
* Function Name  : check_contain_stimulus(char* message, Dictionary* head,
*                  Dictionary* reply, int* ifhead)
* Description    : Check if the message receive from server contain   
*                  stimulus in head linked list, if contain and the 
*                  message is not sent by current clientbot 
8                  then store it to reply linked list
* Input          : char* message;
*                  Dictionary* head;
*                  Dictionary* reply;
*                  int* ifhead;
* Return         : None
******************************************************************************/
void check_contain_stimulus(char* message, Dictionary* head, Dictionary* reply,
        int* ifhead) {
    char* msgLowerCase = (char*)malloc(sizeof(char) * (strlen(message) + 1));
    memset(msgLowerCase, '\0', (strlen(message) + 1));
    char* currentLowerCase;
    int equalFlag = 1;
    Dictionary* current = head;
    //change all characters in message to lower case
    strcpy(msgLowerCase, message);
    to_lower_case(msgLowerCase);

    while (current != NULL) {
        //change all characters in current stimulus to lower case
        currentLowerCase = 
                (char*)malloc(sizeof(char) * (strlen(current->stimulus) + 1));
        memset(currentLowerCase, '\0', (strlen(current->stimulus) + 1));
        strcpy(currentLowerCase, current->stimulus);
        to_lower_case(currentLowerCase);
        //check if message contain stimulus
        for (int j = 0; j <= 
                (strlen(msgLowerCase) - strlen(currentLowerCase)); j++) {
            if (strlen(currentLowerCase) > strlen(msgLowerCase)) {
                break;
            }

            for (int i = 0; i < strlen(msgLowerCase); i++) {
                if (j <= i && i < (strlen(currentLowerCase) + j)) {
                    if (msgLowerCase[i] != currentLowerCase[i - j]) {
                        equalFlag = 0;
                        break;
                    }
                }    
            }
            if (equalFlag) {
                if ((*ifhead) == 0) {
                    add_head(reply, current->stimulus, current->response);
                } else {
                    add_node(reply, current->stimulus, current->response);
                }
                (*ifhead)++;
            }
            equalFlag = 1;
        }
        current = current->next;
        free(currentLowerCase);
    }
}

/******************************************************************************
* Function Name  : check_contain_two_colon(char* buffer) 
* Description    : Check if there is two colons in the buffer. 
* Input          : char* buffer;
* Return         : Return 1 if have two colons
*                  Return 0 if do not have colon
******************************************************************************/
int check_contain_two_colon(char* buffer) {
    int haveColon = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == 58) {
            haveColon++;
            
        }
    }
    
    if (haveColon == 2) {
        return 1;
    }
    return 0;
}

/******************************************************************************
* Function Name  : handle_msg_cmd(char* buffer, Dictionary* head, 
*                  Dictionary* reply, char* myname, int* ifhead)
* Description    : Handle "MSG:" command from server, if the message 
*                  contain stimulus in head linked list if contain 
*                  and the message is not sent by current clientbot 
*                  then store it to reply linked list
* Input          : char* buffer;
*                  Dictionary* head;
*                  Dictionary* reply;
*                  char* myname;
*                  int* ifhead;
* Output         : None
* Return         : None
******************************************************************************/
void handle_msg_cmd(char* buffer, Dictionary* head, Dictionary* reply, 
        char* myname, int* ifhead) {
    char cmd[4];
    memset(cmd, '\0', 4);
    char name[strlen(buffer)];
    memset(name, '\0', strlen(buffer));
    char message[strlen(buffer)];
    memset(message, '\0', strlen(buffer));

    if (check_contain_two_colon(buffer)) {
        sscanf(buffer, "%[^:]:%[^:]:%[^\n]\n", cmd, name, message);
        fprintf(stderr, "(%s) %s\n", name, message);

        if (strcmp(myname, name) != 0) {
            check_contain_stimulus(message, head, reply, ifhead);
        }
    }
    
    
}

/******************************************************************************
* Function Name  : handshaking(char** argv)
* Description    : Read messages send from server, accoding to 
*                  different command make corresponding actions
* Input          : char** argv;
* Return         : None
******************************************************************************/
void handshaking(char** argv) {
    char* buffer = (char*)malloc(sizeof(char));
    char* name = (char*)malloc(sizeof(char) * 10);
    memset(name, '\0', 10);
    int nameTakenNum = -1;
    FILE* fp = fopen(argv[1], "r");
    Dictionary* head = NULL; //linked list store all stimules and response
    head = (Dictionary*) calloc(1, sizeof(Dictionary));
    Dictionary* reply = NULL; // linked list store all response to reply
    reply = (Dictionary*) calloc(1, sizeof(Dictionary));  
    int ifhead = 0;
    read_script(fp, head);
    
    while (1) {
        read_stdin_dynamic(&buffer);

        if (strcmp(buffer, "WHO:") == 0) {
            //send client name to server
            name_reply(&nameTakenNum, &name);
        } else if (strcmp(buffer, "NAME_TAKEN:") == 0) {
            //change client name according to nameTakenNum
            nameTakenNum++;
        } else if (strcmp(buffer, "YT:") == 0) {
            //send the responses collect from the received message
            print_reply(reply);
            printf("DONE:\n");
            fflush(stdout);
            ifhead = 0;
            free_linked_list(reply);
            reply = NULL;
            reply = (Dictionary*) calloc(1, sizeof(Dictionary));
        } else if (strncmp(buffer, "MSG:", 4) == 0) {
            //collect response
            handle_msg_cmd(buffer, head, reply, name, &ifhead);            
        } else if (strncmp(buffer, "LEFT:", 5) == 0) {
            //print LEFT:client name to stderr
            handle_left_cmd(buffer);
        } else if (strcmp(buffer, "KICK:") == 0) {
            //current client is kicked by server
            client_kicked();
        } else {
            communication_error();
        }

        free(buffer);
        buffer = (char*)malloc(sizeof(char));
        memset(buffer, '\0', 1);
    }

}

int main(int argc, char** argv) {
    // argument checking 
    arg_checking(argc, argv);
    
    // handshaking
    handshaking(argv);

    return 0;
}
