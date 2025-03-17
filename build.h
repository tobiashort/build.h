#ifndef BUILD_H
#define BUILD_H

#define ARGV_LEN 64

typedef struct Cmd {
  char *argv[ARGV_LEN];
} Cmd;

int build_yourself(void);
void cmd_append(Cmd *cmd, ...);
void cmd_print(Cmd *cmd);
int cmd_run(Cmd *cmd);

#ifdef BUILD_IMPL

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define panic(msg)              \
  fprintf(stderr, "%s\n", msg); \
  exit(1)

static bool has_changed(void) {
  struct stat source = {0};
  struct stat binary = {0};

  if (stat("build.c", &source) < 0) {
    perror("could not stat build.c");
    exit(1);
  };

  if (stat("build", &binary) < 0) {
    perror("could not stat build.c");
    exit(1);
  };

  printf("build:   mtime: %ld\n", binary.st_mtime);
  printf("build.c: mtime: %ld --> ", source.st_mtime);
  bool changed = source.st_mtime > binary.st_mtime;
  changed ? printf("changed\n") : printf("no changes\n");
  return changed;
}

int build_yourself(void) {
  if (!has_changed()) {
    return 0;
  }
  Cmd cmd = {0};
  cmd_append(&cmd, "cc", NULL);
  cmd_append(&cmd, "-Wall", "-Werror", NULL);
  cmd_append(&cmd, "-o", "build", NULL);
  cmd_append(&cmd, "build.c", NULL);
  int status = cmd_run(&cmd);
  if (status != 0) {
    return status;
  }
  execvp("./build", NULL);
  panic("unreachable");
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

int cmd_run(Cmd *cmd) {
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

  return status;
}

#endif // BUILD_IMPL
#endif // BUILD_H
