#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define EXACT 1
#define PREFIX 2
#define ANYWHERE 3

/* show if sort mode on */
int sortStatus;
/* show if have search mode */
int optionStatus;
/* show if have pattern */
int patternStatus;
/* which mode is on. 1 for exact, 2 for prefix, 3 for anywhere */
int optMode;
/* collect filename */
char* filename;
/* collect pattern */
char* pattern;
/* collect printed strings */
char** wordsToSort;

/** 
 * Show errors and exit.
*/
void arg_error() {
    fprintf(stderr, "Usage: search [-exact|-prefix|-anywhere] "
            "[-sort] pattern [filename]\n");
    exit(1);
}

/** 
 * Handle some default setting.
*/
void handle_default() {
    // mode default
    if (optMode == 0) {
        optMode = EXACT;
    }  

    // check if pattern collect the dictionary path
    if (patternStatus == 1 && filename == NULL) {
        FILE* fp = fopen(pattern, "r");
        if (fp != NULL) {
            arg_error();
        }
    } 

    // default filename path
    if (filename == NULL) {
        filename = malloc(sizeof(char) * strlen("/usr/share/dict/words"));
        strcpy(filename, "/usr/share/dict/words");
    }
}

/** 
 * Collect pattern and check if satisfied requirement,
 * if not then send error and exit.
 * 
 * @str: the str to be checked if satisfied requirement,
 * if it is, add it to pattern.
*/
void add_pattern(char* str) {
    for (int j = 0; j < strlen(str); j++) {
        // check if pattern only contain characters and '?'
        if ((str[j] >= 'a' && str[j] <= 'z') 
                || (str[j] >= 'A' && str[j] <= 'Z') || str[j] == '?') {

            *(pattern + j) = str[j];
        } else {
            fprintf(stderr, "search: pattern should only contain "
                    "question marks and letters\n");
            exit(1);
        }
    }
}

/** 
 * Collect pattern and check if satisfied requirement,
 * if not then send error and exit.
 * 
 * @str: the str to be checked if satisfied requirement,
 * if it is, add it to pattern.
*/
void handle_pat_path(char* str) {
    // add dictionary path
    if (patternStatus != 0) {
        if (patternStatus == 1) {
            filename = malloc(sizeof(char) * strlen(str));
            for (int a = 0; a < strlen(str); a++) {
                *(filename + a) = str[a];
            }
            patternStatus = 2;
        } else {
            arg_error();
        }
    } else {
        // add pattern
        pattern = malloc(sizeof(char) * strlen(str));
        add_pattern(str);
        patternStatus = 1;
    }
}

/** 
 * Check if the program have any output,
 * if not then exit with 1. otherwhis exit with 0.
 * 
 * @ifPrinted: the pointer to ifPrinted, show if there is any output.
*/
void check_have_output(int* ifPrinted) {
    if (*ifPrinted == 0) {
        exit(1);
    }
    exit(0);
}

/** 
 * Setting all pattern to lowercase and find all question marks in pattern.
 * record there index and counts.
 * 
 * @qMarks: the pointer to qMarks, contain the index of all question marks.
 * @qCount: the pointer to qCount, indicate the number of questions.
*/
void handle_alpha_qmarks(int* qMarks, int* qCount) {
    for (int i = 0; i < strlen(pattern); i++) {
        if (pattern[i] == '?') {
            qMarks[*qCount] = i;
            (*qCount)++;
        }
        pattern[i] = (char)tolower((int)pattern[i]);
    }
}

/** 
 * Change the str argv to lower case.
 * 
 * @str: a string with only characters.
*/
void to_lower_case(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        // turn all characters into lower case except '?'
        if (isalpha(str[i])) {
            str[i] = (char)tolower((int)str[i]);
        }
    }
}

/** 
 * Check if the index argv inputed have question mark on it.
 * if have return 1, otherwise return 0.
 * 
 * @index: the pointer to index, which is the index of the buffer
 * @qMarks: the pointer to qMarks, contain the index of all question marks.
 * @qCount: the pointer to qCount, indicate the number of questions.
*/
int if_contain_qindex(int* index, int* qMarks, int* qCount) {
    for (int i = 0; i < *qCount; i++) {
        if (*index == qMarks[i]) {
            return 1;
        }
    }
    return 0;
}

/** 
 * Initializing sortarray and count the number of string be printed. 
 * 
 * @printStrNumIndex: the pointer to printStrNumIndex, 
 * the number of string be printed start with 0.
 * @buffer: the pointer to buffer, 
 * the string going to be printed in dictionary. 
*/
void sortarray_initial_and_copy(int* printStrNumIndex, char* buffer) {
    if (*printStrNumIndex > 0) {
        wordsToSort = 
                realloc(wordsToSort, sizeof(char*) * (*printStrNumIndex + 1));
        wordsToSort[*printStrNumIndex] = (char*) malloc(sizeof(char) * 41);
    }
    strcpy(wordsToSort[*printStrNumIndex], buffer);
    *printStrNumIndex += 1;
}

/** 
 * Sorting the strings in wordsToSort.
 * 
 * @printStrNumIndex: the pointer to printStrNumIndex, 
 * the number of string be printed start with 0.
*/
void sort_function(int* printStrNumIndex) {
    char temp[41];
    
    // sorting
    for (int i = 0; i <= *printStrNumIndex; i++) {
        for (int j = i + 1; j <= *printStrNumIndex; j++) {

            if (strcasecmp(wordsToSort[i], wordsToSort[j]) > 0) {
                memset(temp, '\0', 41);
                strcpy(temp, wordsToSort[i]);
                memset(wordsToSort[i], '\0', 41); 
                strcpy(wordsToSort[i], wordsToSort[j]);
                memset(wordsToSort[j], '\0', 41); 
                strcpy(wordsToSort[j], temp);
            }
        }
    }

    // printing
    for (int a = 0; a <= *printStrNumIndex; a++) {
        printf("%s\n", wordsToSort[a]);
    }  
}

/** 
 * Deciding if print the word directly or put it in wordToSort
 * 
 * @printStrNumIndex: the pointer to printStrNumIndex, 
 * the number of string be printed start with 0.
 * @buffer: the pointer to buffer, 
 * the string going to be printed in dictionary. 
 * @equalFlag: the pointer to equalFlag,
 * 1 indicates equal, 0 indicates not equal.
 * @ifPrinted: the pointer to ifPrinted,
 * 1 indicates printed, 0 indicates have not printed yet.
 * 
*/
void if_printed_word(int* equalFlag, int* printStrNumIndex,
        char* buffer, int* ifPrinted) {
    if (*equalFlag) {
        // check if need sort
        if (sortStatus == 1) {
            sortarray_initial_and_copy(printStrNumIndex, buffer);
        } else {
            printf("%s\n", buffer);
        }
        *ifPrinted = 1;  
    }
    *equalFlag = 1;
}

/** 
 * Checking if sort mode on, if on, initializing wordsToSort.
*/
void check_sort_on() {
    if (sortStatus == 1) {
        wordsToSort = (char**) malloc(sizeof(char*) * 1);
        wordsToSort[0] = (char*) malloc(sizeof(char) * 41);
    }
}

/** 
 * Searching pattern in dictionary under EXACT MODE.
*/
void search_exact() {
    FILE* fp = fopen(filename, "r"); // open dictionary
    char buffer[41]; // a string to contain each line in dictionary
    char lowerCaseCopy[41]; // a string to put lowercase buffer
    memset(buffer, '\0', 41); // initialize
    memset(lowerCaseCopy, '\0', 41); // initialize
    int ifPrinted = 0; // a flag to check if there is any output
    int qMarks[40]; // a int array contain each '?' index in pattern
    int qCount = 0; // number of '?' in pattern
    int equalFlag = 1; // a flag to check if two strings are equal
    int printStrNumIndex = 0; // the number of printed string start with 0

    check_sort_on();
    // get qmarks and qcount
    handle_alpha_qmarks(qMarks, &qCount);

    // read dictionary
    while (fgets(buffer, 41, fp) != NULL) {
        if (buffer[strlen(buffer) - 1] == 10) {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(lowerCaseCopy, buffer);
        to_lower_case(lowerCaseCopy);

        if (strlen(buffer) == strlen(pattern)) {
            for (int i = 0; i < strlen(buffer); i++) {
                // check if satisfy requirement
                if (if_contain_qindex(&i, qMarks, &qCount)) {
                    if (lowerCaseCopy[i] < 'a' || lowerCaseCopy[i] > 'z') {
                        equalFlag = 0;
                        break;
                    } 
                } else {
                    if (lowerCaseCopy[i] != pattern[i]) {
                        equalFlag = 0;
                        break;
                    }
                }
            }
            // decide if print string directly or put it into array
            if_printed_word(&equalFlag, &printStrNumIndex, buffer, &ifPrinted);
        } 
    }
    // sort printing
    if (sortStatus == 1 && ifPrinted) {
        printStrNumIndex -= 1;
        sort_function(&printStrNumIndex);
    }
    // check if have any output
    check_have_output(&ifPrinted); 
}

/** 
 * Searching pattern in dictionary under PREFIX MODE.
*/
void search_prefix() {
    FILE* fp = fopen(filename, "r"); // open dictionary
    char buffer[41]; // a string to contain each line in dictionary
    char lowerCaseCopy[41]; // a string to put lowercase buffer
    memset(buffer, '\0', 41); // initialize
    memset(lowerCaseCopy, '\0', 41); // initialize
    int ifPrinted = 0; // a flag to check if there is any output
    int qMarks[40]; // a int array contain each '?' index in pattern
    int qCount = 0; // number of '?' in pattern
    int equalFlag = 1; // a flag to check if two strings are equal
    int printStrNumIndex = 0; // the number of printed string start with 0
    check_sort_on();
    handle_alpha_qmarks(qMarks, &qCount); // get qmarks and qcount
    
    while (fgets(buffer, 41, fp) != NULL) { // read dictionary
        if (buffer[strlen(buffer) - 1] == 10) {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(lowerCaseCopy, buffer);
        to_lower_case(lowerCaseCopy);
        
        if (strlen(buffer) >= strlen(pattern)) {
            for (int i = 0; i < strlen(buffer); i++) {
                if (i < strlen(pattern)) { // check if satisfy requirement
                    if (if_contain_qindex(&i, qMarks, &qCount)) {
                        if (lowerCaseCopy[i] < 'a' || lowerCaseCopy[i] > 'z') {
                            equalFlag = 0;
                            break;
                        } 
                    } else {
                        if (lowerCaseCopy[i] != pattern[i]) {
                            equalFlag = 0;
                            break;
                        }
                    }
                } else {
                    if (!isalpha(buffer[i])) {
                        equalFlag = 0;
                        break;
                    }
                }     
            }
            if_printed_word(&equalFlag, &printStrNumIndex, buffer, &ifPrinted);
        } 
    }
    if (sortStatus == 1 && ifPrinted) { // sort printing
        printStrNumIndex -= 1;
        sort_function(&printStrNumIndex);
    }
    check_have_output(&ifPrinted);  // check if have any output
}

/** 
 * Moving the index in qMarks for ANYWHERE MODE
 * 
 * @qMarks: the pointer to qMarks, contain the index of all question marks.
 * @j: the pointer to j, which is the number of index moved.
 * @qCount: the pointer to qCount, indicate the number of questions.
 * @qMarksCopy: the pointer to qMarksCopy, a copy of qMarks
*/
void change_qmarks_index(int* qMarks, int* j, int* qCount, int* qMarksCopy) {
    for (int i = 0; i < *qCount; i++) {
        qMarksCopy[i] = qMarks[i] + *j;
    }
}

/** 
 * Searching pattern in dictionary under ANYWHERE MODE.
*/
void search_anywhere() {
    FILE* fp = fopen(filename, "r"); // open dictionary
    char buffer[41]; // a string to contain each line in dictionary
    char lowerCaseCopy[41]; // a string to put lowercase buffer
    memset(buffer, '\0', 41); // initialize
    memset(lowerCaseCopy, '\0', 41); // initialize
    int ifPrinted = 0; // a flag to check if there is any output
    int qMarks[40]; // a int array contain each '?' index in pattern
    int qMarksCopy[40]; // a copy of qMarks
    int qCount = 0; // number of '?' in pattern
    int equalFlag = 1; // a flag to check if two strings are equal
    int strPrinted = 0; // a flag to check if this string has printed before
    int printStrNumIndex = 0; // the number of printed string start with 0

    check_sort_on();

    // get qmarks and qcount
    handle_alpha_qmarks(qMarks, &qCount);

    // read file
    while (fgets(buffer, 41, fp) != NULL) {
        if (buffer[strlen(buffer) - 1] == 10) {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(lowerCaseCopy, buffer);
        to_lower_case(lowerCaseCopy);
        // reset strPrinted, because here enter a new line in file
        strPrinted = 0;

        // moving index
        for (int j = 0; j <= (strlen(buffer) - strlen(pattern)); j++) {
            change_qmarks_index(qMarks, &j, &qCount, qMarksCopy);
            if (strPrinted) {
                break;
            }
            if (strlen(pattern) > strlen(buffer)) {
                break;
            }
            // read buffer
            for (int i = 0; i < strlen(buffer); i++) {
                if (j <= i && i < (strlen(pattern) + j)) {
                    if (if_contain_qindex(&i, qMarksCopy, &qCount)) {
                        if (lowerCaseCopy[i] < 'a' || lowerCaseCopy[i] > 'z') {
                            equalFlag = 0;
                            break;
                        } 
                    } else {
                        if (lowerCaseCopy[i] != pattern[i - j]) {
                            equalFlag = 0;
                            break;
                        }
                    }
                } else {
                    if (!isalpha(buffer[i])) {
                        equalFlag = 0;
                        break;
                    }
                }     
            }

            // decide if print string directly or put it into array
            if (equalFlag) {
                if (sortStatus == 1) {
                    sortarray_initial_and_copy(&printStrNumIndex, buffer);
                } else {
                    printf("%s\n", buffer);
                }
                strPrinted = 1;
                ifPrinted = 1;
            }
            equalFlag = 1;
        }
    }
    // sort printing
    if (sortStatus == 1 && ifPrinted) {
        printStrNumIndex -= 1;
        sort_function(&printStrNumIndex);
    }

    // check if have any output
    check_have_output(&ifPrinted);    
}

/** 
 * Checking if the arguments input satisfy the requirements,
 * if not sent error message and exit by 1.
 * 
 * @argc: the number of argvs.
 * @argv: the arguments inputed in cmd line.
*/
void arg_checking(int argc, char** argv) {
    // none argument in cmd lind
    if (argc == 1) {
        arg_error();
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-exact") == 0) {
            if (optionStatus != 0) {
                arg_error();
            }
            optionStatus = 1;
            optMode = EXACT;
        } else if (strcmp(argv[i], "-prefix") == 0) {
            if (optionStatus != 0) {
                arg_error();
            }
            optionStatus = 1;
            optMode = PREFIX;
        } else if (strcmp(argv[i], "-anywhere") == 0) {
            if (optionStatus != 0) {
                arg_error();
            }
            optionStatus = 1;
            optMode = ANYWHERE;
        } else if (strcmp(argv[i], "-sort") == 0) {
            if (sortStatus != 0) {
                arg_error();
            }
            sortStatus = 1;
        } else if (argv[i][0] != '-') { // check if it is a pattern or path
            handle_pat_path(argv[i]);
        } else {
            arg_error();
        }
    }

    if (pattern == NULL) {
        arg_error();
    }

    handle_default();
}

/** 
 * Checking if filename path valid and choose search mode.
*/
void search_func() {
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "search: file \"%s\" can not be opened\n", filename);
        exit(1);
    }

    if (optMode == EXACT) {
        search_exact();
    } 
    
    if (optMode == PREFIX) {
        search_prefix();
    } 
    
    if (optMode == ANYWHERE) {
        search_anywhere();
    }

}

int main(int argc, char** argv) {
    // argument checking 
    arg_checking(argc, argv);

    // search keyword
    search_func();

    return 0;
}
