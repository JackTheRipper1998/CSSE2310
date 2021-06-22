#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define WHO "WHO:"

// read_msg_colon(char* MSG, char* name, char* message, char* buffer) {
//     for (int i = 0; i < strlen(buffer); i++) {
        
//     }
// }
void int_length(int* num, int* numLength) {
    do
    {
        *num = *num / 10;
        (*numLength)++;
    } while (num != 0);
}

void name_reply(int* nameTakenNum, char** name) {
    int numLength = 0;
    if (*nameTakenNum == -1) {
        printf("NAME:client\n");
        strcpy(*name, "client");
    } else {
        int_length(nameTakenNum, &numLength);
        free(*name);
        *name = (char*)malloc(sizeof(char) * (numLength + 7));
        sprintf(*name, "client%d", *nameTakenNum);
        printf("NAME:client%d\n", *nameTakenNum);
    }
}

int main(int argc, char** argv) {
    int num = 101234;
    int numLength = 0;
    int_length(&num, &numLength);
    printf("length = %d\n", numLength);
    return 0;
}