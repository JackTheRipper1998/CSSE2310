#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"

/******************************************************************************
* Function Name  : args_error() 
* Description    : Report client usage error with error message
*                  "Usage: client chatscript"
* Input          : None
* Return         : None
******************************************************************************/
void args_error() {
    fprintf(stderr, "Usage: client chatscript\n");
    exit(1);
}

/******************************************************************************
* Function Name  : arg_checking(int argc, char** argv) 
* Description    : Check if the arguments input in command line 
*                  is correct
* Input          : int argc;
*                  char** argv;
* Output         : None
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
* Description    : According to the nameTakenNum, reply client name to server 
* Input          : int* nameTakenNum;
*                  char** name;
* Return         : None
******************************************************************************/
void name_reply(int* nameTakenNum, char** name) {
    int numLength = 0;

    if (*nameTakenNum == -1) {
        printf("NAME:client\n");
        fflush(stdout);
        strcpy(*name, "client");
    } else {
        printf("NAME:client%d\n", *nameTakenNum);
        fflush(stdout);
        int_length(nameTakenNum, &numLength);
        free(*name);
        *name = (char*)malloc(sizeof(char) * (numLength + 7));
        sprintf(*name, "client%d", *nameTakenNum);
    }
}

/******************************************************************************
* Function Name  : read_script(FILE* fp) 
* Description    : After receive "YT:" from server, read the file run 
*                  by client and accoding to the content of each line  
*                  in the file make corresponding actions
* Input          : FILE* fp;
* Return         : None
******************************************************************************/
void read_script(FILE* fp) {
    char* buffer = (char*)malloc(sizeof(char));
    read_file_dynamic(&buffer, fp);

    while (strcmp(buffer, "DONE:") != 0) {
        if (strcmp(buffer, "QUIT:") == 0) {
            printf("%s\n", buffer);
            fflush(stdout);
            exit(0);
        }
        if (strncmp(buffer, "CHAT:", 4) == 0 || 
                strncmp(buffer, "KICK:", 4) == 0) {
            printf("%s\n", buffer);
            fflush(stdout);
        }
        free(buffer);
        buffer = malloc(sizeof(char) + 1);
        read_file_dynamic(&buffer, fp);
    }
    printf("%s\n", buffer);
    fflush(stdout);
}

/******************************************************************************
* Function Name  : handle_msg_cmd(char* buffer) 
* Description    : After receive "MSG:" from server, client send  
*                  this text to stderr with a specific format
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
     
    fprintf(stderr, "(%s) %s\n", name, message);
    
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
    char* name = (char*)malloc(sizeof(char) * 7);
    int nameTakenNum = -1;
    FILE* fp = fopen(argv[1], "r");

    while (1) {
        read_stdin_dynamic(&buffer);
        //reply name
        if (strcmp(buffer, "WHO:") == 0) {
            name_reply(&nameTakenNum, &name);
        //change name
        } else if (strcmp(buffer, "NAME_TAKEN:") == 0) {
            nameTakenNum++;
        //read script
        } else if (strcmp(buffer, "YT:") == 0) {
            read_script(fp);
        //send message to stderr
        } else if (strncmp(buffer, "MSG:", 4) == 0) {
            handle_msg_cmd(buffer);
        //send left client name to stderr    
        } else if (strncmp(buffer, "LEFT:", 5) == 0) {
            handle_left_cmd(buffer);
        //kicked by server and exit
        } else if (strcmp(buffer, "KICK:") == 0) {
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
