#include "battery_notify.h"
#include <pwd.h>
#include <stdarg.h>
#include <time.h>

bat_t batteryLevel(const char *uevent) {
  errno = 0;
  char buff[1024], battery_level_c[4];
  FILE *f;
  bat_t battery_level = 0;
  char *ptr = NULL;
  f = fopen(uevent, "rb");
  if (!f)
    return -1;

  while (fgets(buff, sizeof(buff), f)) {
    if ((ptr = strstr(buff, "POWER_SUPPLY_CAPACITY")) != NULL) {
      strncpy(battery_level_c, ptr + POWER_SUPPLY_CAPACITY_SZ + 1,
              sizeof(battery_level_c) - 1);
      break;
    }
  }
  if (!ptr) {
    errno = EINVAL;
    return -1;
  }
  if (!fclose(f))
    return -1;
  battery_level_c[strlen(battery_level_c) - 1] = '\0';
  return (bat_t)strtoul(battery_level_c, NULL, 10);
}
bool_t isCharging(const char *uevent) {
  errno = 0;
  char buff[1024], battery_status[100];
  FILE *f;
  char *ptr = NULL;
  f = fopen(uevent, "rb");
  if (!f)
    return errno;

  while (fgets(buff, sizeof(buff), f)) {
    if ((ptr = strstr(buff, "POWER_SUPPLY_STATUS")) != NULL) {
      strncpy(battery_status, ptr + POWER_SUPPLY_STATUS_SZ + 1,
              sizeof(battery_status) - 1);
      break;
    }
  }
  if (!ptr)
    return EINVAL;
  if (!fclose(f))
    return -1;
  battery_status[strlen(battery_status) - 1] = '\0';
  if (strcmp(battery_status, "Charging") == 0)
    return True;
  else
    return False;
}

char *strtime(const char *format) {
  struct tm *tm;
  static char buff[128]; // TODO:change it later
  time_t time_ = time(NULL);
  tm = localtime(&time_);
  strftime(buff, sizeof(buff), format, tm);
  return buff;
}
void err_log(int status, int error, const char *log, ...) {
  int fd = -1;
  int fd_flags = O_WRONLY | O_CREAT | O_APPEND;
  char logName[MAX_NAME_SZ];
  char buff[2000]; // TODO:change it lATER
  va_list ap;
  snprintf(logName, sizeof(logName), "%s-%s.log", program_invocation_short_name,
           strtime("%F"));
  fd = open(logName, fd_flags, BATTERY_LOG_PERM);
  if (fd == -1)
    exit(EXIT_FAILURE);
  // Log Message
  snprintf(buff, sizeof(buff), "[%s] <%s> %s", strtime("%c"), strerror(errno),
           log);
  va_start(ap, log);
  vdprintf(fd, buff, ap);
  va_end(ap);
  close(fd);
  exit(status);
}
int createPidFile(const char *progName, pid_t pid, int flag) {

  int fd = -1;
  ssize_t numWrite = -1;
  char pid_c[10]; // TODO:change it later
  int fd_flags = O_CREAT | O_WRONLY;
  fd_flags += (flag == MULTI_INSTANCE) ? O_EXCL : 0;

  snprintf(path, sizeof(path), "/run/user/%d/%s.pid", getuid(), progName);
  snprintf(pid_c, sizeof(pid_c), "%d", pid);
  fd = open(path, fd_flags, S_IRUSR | S_IWUSR | S_IRGRP);
  if (fd == -1)
    return errno;
  numWrite = write(fd, pid_c, strlen(pid_c));
  if (numWrite == -1)
    return errno;
  if (close(fd) == -1)
    return errno;
  return 0;
}

