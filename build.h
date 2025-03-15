#ifndef BUILD_H
#define BUILD_H

#define ARGV_LEN 64

typedef struct Cmd {
  char *argv[ARGV_LEN];
} Cmd;

void build_yourself();
void cmd_append(Cmd *cmd, ...);
void cmd_print(Cmd *cmd);
void cmd_run(Cmd *cmd);

#ifdef BUILD_IMPL

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define panic(msg)              \
  fprintf(stderr, "%s\n", msg); \
  exit(1)

void build_yourself() {
  Cmd cmd = {0};
  cmd_append(&cmd, "cc", NULL);
  cmd_append(&cmd, "-Wall", "-Werror", NULL);
  cmd_append(&cmd, "-o", "build", NULL);
  cmd_append(&cmd, "build.c", NULL);
  cmd_run(&cmd);
}

void cmd_append(Cmd *cmd, ...) {
  va_list args;
  va_start(args, cmd);
  while (true) {
    char* arg = va_arg(args, char*);
    if (arg == NULL) {
      return;
    }
    for (int i = 0; i < ARGV_LEN; i++) {
      if (cmd->argv[i] == NULL) {
        cmd->argv[i] = arg;
        goto next_arg;
      }
    }
    panic("command too long");
next_arg:
    continue;
  }
}

void cmd_print(Cmd *cmd) {
  printf("%s", cmd->argv[0]);
  for(int i = 1; i < ARGV_LEN; i++) {
    if (cmd->argv[i] == NULL) {
      break;
    }
    printf(" %s", cmd->argv[i]);
  }
  printf("\n");
}

void cmd_run(Cmd *cmd) {
  cmd_print(cmd);

  pid_t pid = fork();

  if (pid < 0) {
    panic("fork failed");
  }

  if (pid == 0) {
    execvp(cmd->argv[0], cmd->argv);
    panic("unreachable");
  }

  int status;
  waitpid(pid, &status, 0);
}

#endif // BUILD_IMPL
#endif // BUILD_H
