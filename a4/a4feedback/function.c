#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"

/******************************************************************************
* Function Name  : communication_error() 
* Description    : Report communication error and sent message 
*                  "Communications error" to stderr, exit program with code 2
* Input          : None
* Return         : None
******************************************************************************/
void communication_error() {
    fprintf(stderr, "Communications error\n");
    exit(2);
}

/******************************************************************************
* Function Name  : client_kicked() 
* Description    : The client is kicked by server and send message 
*                  "Kicked" to stderr, exit program with code 3
* Input          : None
* Return         : None
******************************************************************************/
void client_kicked() {
    fprintf(stderr, "Kicked\n");
    exit(3);
}

/******************************************************************************
* Function Name  : int_length(int* nameTakenNum, int* numLength) 
* Description    : Caculate the nameTakeNum length, and assigned to numLength 
* Input          : int* nameTakenNum;
*                  int* numLength;
* Return         : None
******************************************************************************/
void int_length(int* nameTakenNum, int* numLength) {
    int num = *nameTakenNum;

    do {
        num = num / 10;
        (*numLength)++;
    } while (num != 0);
}

/******************************************************************************
* Function Name  : check_contain_colon(char* buffer) 
* Description    : Check if there is a colon in the buffer. 
* Input          : char* buffer;
* Return         : Return 1 if have colon
*                  Return 0 if do not have colon
******************************************************************************/
int check_contain_colon(char* buffer) {
    int haveColon = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == 58) {
            haveColon++;
        }
    }
    
    if (haveColon == 1) {
        return 1;
    }

    return 0;
}

/******************************************************************************
* Function Name  : read_stdin_dynamic(char** buffer) 
* Description    : Read stdin and store content in buffer dynamicly
* Input          : char** buffer;
* Return         : return 1 if get eof, else return 0
******************************************************************************/
int read_stdin_dynamic(char** buffer) {
    char c;
    int shiftIndex = 0;

    while ((c = fgetc(stdin)) != EOF) {
        if (c == 10) {
            (*buffer)[shiftIndex] = '\0';
            return 0;
        }
        (*buffer)[shiftIndex] = c;
        shiftIndex++;
        *buffer = (char*)realloc(*buffer, sizeof(char) * (shiftIndex + 1));
    }
    return 1;
}

/******************************************************************************
* Function Name  : read_file_dynamic(char** buffer, FILE* fp)
* Description    : Read file line by line and store content 
*                  in buffer dynamicly 
* Input          : char** buffer;
*                  FILE* fp;
* Return         : None
******************************************************************************/
void read_file_dynamic(char** buffer, FILE* fp) {
    char c;
    int shiftIndex = 0;

    while (1) {
        if ((c = fgetc(fp)) == EOF) {
            (*buffer)[shiftIndex] = '\0';
            break;
        }

        if (c == 10) {
            (*buffer)[shiftIndex] = '\0';
            break;
        }
        (*buffer)[shiftIndex] = c;
        shiftIndex++;
        *buffer = (char*)realloc(*buffer, sizeof(char) * (shiftIndex + 1));
    }
}

/******************************************************************************
* Function Name  : handle_left_cmd(char* buffer)
* Description    : Handle "LEFT:" command from server, and send this
*                  text to stderr with a specific format
* Input          : char* buffer;
* Return         : None
******************************************************************************/
void handle_left_cmd(char* buffer) {
    char cmd[5];
    memset(cmd, '\0', 5);
    char name[strlen(buffer)];
    memset(name, '\0', 5);
    // const char s[2] = ":";
    // char *token;
    // int part = 1;

    sscanf(buffer, "%[^:]:%[^\n]\n", cmd, name);
    // token = strtok(buffer, s);

    // while( token != NULL ) {
    //     if (part == 1) {
    //         strcpy(cmd, token);
    //     } 

    //     if (part == 2) {
    //         strcpy(name, token);
    //     }
    //     part++;
    
    //     token = strtok(NULL, s);
    // }

    fprintf(stderr, "(%s has left the chat)\n", name);
}

/******************************************************************************
* Function Name  : read_fdin_dynamic(char** buffer, FILE* readFD)
* Description    : Read the pipe file pointer dynamicly and
*                  store the content in the buffer 
* Input          : char** buffer;
*                  FILE* readFD;
* Return         : return 1 if get eof, else return 0
******************************************************************************/
int read_fdin_dynamic(char** buffer, FILE* readFD) {
    char c;
    int shiftIndex = 0;
    while ((c = fgetc(readFD)) != EOF) {
        if (c == '\n') {
            (*buffer)[shiftIndex] = '\0';
            return 0;
        }
        (*buffer)[shiftIndex] = c;
        shiftIndex++;
        *buffer = (char*)realloc(*buffer, sizeof(char) * (shiftIndex + 1));
    }
    return 1;
}

/******************************************************************************
* Function Name  : free_malloc_char_pointer(char** buffer)
* Description    : Free buffer, and malloc buffer with one char size
* Input          : char** buffer;
* Return         : None
******************************************************************************/
void free_malloc_char_pointer(char** buffer) {
    free(*buffer);
    (*buffer) = (char*)malloc(sizeof(char));
    memset(*buffer, '\0', 1);
}

/******************************************************************************
* Function Name  : split_buffer(char* buffer, char** content)
* Description    : Split buffer by colon, and put useful value on content
* Input          : char* buffer;
*                  char** content
* Return         : None
******************************************************************************/
void split_buffer(char* buffer, char** content) {
    char* rubbish = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
    *content = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
    memset((*content), '\0', (strlen(buffer) + 1));
    
    sscanf(buffer, "%[^:]:%[^\n]", rubbish, *content);
}
