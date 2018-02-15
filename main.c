#define _GNU_SOURCE

#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "battery_notify.h"

extern char *program_invocation_short_name;
char path[100];

void sigHandler(int sig) {
  remove(path);
  exit(EXIT_SUCCESS);
} // TODO:error handeling

int main(argc, argv, environ)
int argc;
char *argv[];
char **environ;
{
  struct sigaction sig;
  bat_t battery_level = 0;
  char log[256];
  char *shm_true_c_addr = NULL, *progName = NULL;
  progName = basename(argv[0]);
  pid_t battery_level_pid = 0;

  if (createPidFile(basename(progName), getpid(),
#ifdef DEBUG
                    MULTI_INSTANCE
#else
                    0
#endif // DEBUG
                    ) != 0)
    err_log(1, errno, "createPidFile()\n");

  memset(&sig, '\0', sizeof(sig)); // to ensure
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = &sigHandler;
  if (sigaction(SIGTERM, &sig, NULL) == -1)
    err_log(1, errno, "sigaction()\n");

  setenv("PATH", "/sbin/:/bin/:/usr/sbin/:/usr/bin/", 1);
  shm_true_c_addr = mmap(NULL, sizeof(int), PROT_WRITE | PROT_READ,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  // TODO:delete(not safe!)

  if (shm_true_c_addr == MAP_FAILED)
    err_log(1, errno, "mmap()\n");
  *shm_true_c_addr = 0;
  for (;;) {
#ifdef DEBUG
    printf("%d\n", *shm_true_c_addr);
#endif // DEBUG
    battery_level = batteryLevel(BATTERY_UEVENT);
    if (battery_level == -1)
      err_log(1, errno, "batteryLevel() '%s'\n", BATTERY_UEVENT);
    if (*shm_true_c_addr == 1)
      sleep(REPEAT_INTERVAL);
    else
      sleep(SCRAPE_INTERVAL);
    if (!isCharging(BATTERY_UEVENT) && battery_level <= BATTERY_LIMIT_WARN &&
        battery_level > BATTERY_LIMIT_CRITICAL) {
      snprintf(log, sizeof(log), "battery level: %d", battery_level);
      switch (fork()) {
      case -1:
        err_log(1, errno, "for()\n");
      case 0:
        *shm_true_c_addr = 1;
        execlp("notify-send", "notify-send", "-u", "normal", log, (char *)NULL);
        err_log(1, errno, "execlp()\n");
      default:
        wait(NULL);
        break;
      }
    } else if (isCharging(BATTERY_UEVENT) &&
               battery_level <= BATTERY_LIMIT_CRITICAL) {
      snprintf(log, sizeof(log), "battery level: %d", battery_level);
      switch (fork()) {
      case -1:
        err_log(1, errno, "fork()\n");
      case 0:
        *shm_true_c_addr = 1;
        execlp("notify-send", "notify-send", "-u", "critical", log,
               (char *)NULL);
      default:
        wait(NULL);
        break;
      }
    } else
      *shm_true_c_addr = 0;
  }
  // TODO:delete all of them later(service std)
  if (munmap(shm_true_c_addr, sizeof(int)) == -1)
    err_log(1, errno, "munmap()\n");
  if (remove(path) == -1)
    err_log(1, errno, "remove()\n");
  exit(EXIT_SUCCESS);
}
