#ifndef BATTERY_NOTIFY_DOT_H
#define BATTERY_NOTIFY_DOT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>

extern char *program_invocation_short_name;
extern char path[100];
//TODO:DELETE MAX_PATH_SZ 
#define MAX_NAME_SZ 			pathconf("/run/user",_PC_NAME_MAX)
#define BATTERY_UEVENT 			"/sys/class/power_supply/BAT0/uevent"
#define POWER_SUPPLY_CAPACITY_SZ 	strlen("POWER_SUPPLY_CAPACITY") 
#define POWER_SUPPLY_STATUS_SZ 		strlen("POWER_SUPPLY_STATUS") 
//TODO:CHANGE BATTERY_LOG TO "STANDARD DIRECTORY" e.g.(/run/user/[0-9].*)
#define BATTERY_LOG_PERM 		S_IRUSR | S_IWUSR | S_IRGRP
#define BATTERY_LIMIT_WARN 		20
#define BATTERY_LIMIT_CRITICAL 		10
#define SCRAPE_INTERVAL 		2
#define REPEAT_INTERVAL 		100
#define MULTI_INSTANCE 			1 //TODO:CHANGE TO MASK FORMAT

typedef short int bat_t;
typedef enum{False,True}bool_t;

extern bool_t isCharging(const char *uevent);
extern bat_t batteryLevel(const char *uevent);
extern int createPidFile(const char *progName,pid_t pid,int flag);
extern void err_log(int status,int error,const char *log,...);
extern char *strtime(const char *format);


#endif //BATTERY_NOTIFY_DOT_H 
