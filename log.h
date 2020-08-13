#ifndef LOGTOOL
#define LOGTOOL
#include <stdio.h>
#include <time.h>
#include <string>
#include <cstring>

using namespace std;

#ifndef LOG_PARA
#define LOG_PARA
FILE *log_file = NULL;
const char *LOG_FILE = "./result/log";
const char LEVEL[3][20] = {"DEBUG", "INFO", "ERROR"};
const int CURRENT_LEVEL = 1;
const int PRINT_INFO = 1;
#define LOG_ON 1
#endif

#if LOG_ON
#define LogWrite(level, s, arg...)                   \
    if (PRINT_INFO)                                  \
    {                                                \
        fprintf(stdout, s, ##arg);                   \
        printf("\n");                                \
    }                                                \
    if (level >= CURRENT_LEVEL)                      \
    {                                                \
        log_file = fopen(LOG_FILE, "a");             \
        fprintf(log_file, GET_MSG(level, s), ##arg); \
        fflush(log_file);                            \
        fclose(log_file);                            \
    }
#else
#endif

char *GET_MSG(int level, string s)
{
    static char log_msg[1000];
    time_t t = time(0);
    struct tm *local = localtime(&t);
    char time_buf[128];
    strftime(time_buf, 64, "%Y/%m/%d %H:%M:%S", local);
    strcpy(log_msg, "[");
    strcat(log_msg, time_buf);
    strcat(log_msg, "] [");
    strcat(log_msg, LEVEL[level]);
    strcat(log_msg, "] ");
    strcat(log_msg, s.c_str());
    strcat(log_msg, "\n");
    return log_msg;
}

void LogClear()
{
    log_file = fopen(LOG_FILE, "w");
    fclose(log_file);
}

/*
void test(){
    LogWrite(0,"%d\n",123);
    LogClear();
    return;
}
*/
#endif
