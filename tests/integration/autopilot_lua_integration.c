/* End-to-end Lua administration of live BTech autopilot objects. */

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "mux/support/checked_storage.h"

enum {
  TEST_TIMEOUT_MS = 15000,
  READY_TIMEOUT_MS = 5000,
  PROCESS_TIMEOUT_SECONDS = 60,
};

static void *buffer_suffix(void *buffer, size_t capacity, size_t offset) {
  return checked_storage_region(buffer, capacity, offset, capacity - offset);
}

static const void *constant_buffer_suffix(const void *buffer, size_t capacity,
                                          size_t offset) {
  return checked_storage_region_const(buffer, capacity, offset,
                                      capacity - offset);
}

static int choose_port(void) {
  struct sockaddr_in address = {.sin_family = AF_INET,
                                .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
  socklen_t length = sizeof(address);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0 ||
      bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
      getsockname(socket_fd, (struct sockaddr *)&address, &length) < 0) {
    if (socket_fd >= 0)
      close(socket_fd);
    return -1;
  }
  close(socket_fd);
  return ntohs(address.sin_port);
}

static pid_t start_server(const char *server, const char *directory) {
  char descriptor[32];
  char signal_value;
  int ready_pipe[2];
  if (pipe(ready_pipe) < 0)
    return -1;
  pid_t child = fork();
  if (child < 0) {
    close(ready_pipe[0]);
    close(ready_pipe[1]);
    return -1;
  }
  if (child == 0) {
    close(ready_pipe[0]);
    (void)snprintf(descriptor, sizeof(descriptor), "%d", ready_pipe[1]);
    if (chdir(directory) < 0 ||
        setenv("BTECH_TEST_READY_FD", descriptor, 1) < 0 ||
        setenv("BTECH_TEST_GOD_PASSWORD", "btmuxr0x", 1) < 0)
      _exit(127);
    execl(server, server, "stompymux.toml", nullptr);
    _exit(127);
  }
  close(ready_pipe[1]);
  struct pollfd ready = {.fd = ready_pipe[0], .events = POLLIN};
  for (int elapsed = 0; elapsed < READY_TIMEOUT_MS; elapsed += 25) {
    int status;
    pid_t waited = waitpid(child, &status, WNOHANG);
    if (waited != 0) {
      close(ready_pipe[0]);
      return -1;
    }
    int result = poll(&ready, 1, 25);
    if (result < 0 && errno == EINTR)
      continue;
    if (result == 1 && (ready.revents & POLLIN) &&
        read(ready_pipe[0], &signal_value, sizeof(signal_value)) ==
            sizeof(signal_value)) {
      close(ready_pipe[0]);
      return child;
    }
    if (result < 0 ||
        (result == 1 && (ready.revents & (POLLERR | POLLHUP | POLLNVAL))))
      break;
  }
  close(ready_pipe[0]);
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  return -1;
}

static int stop_server(pid_t child) {
  if (kill(child, SIGTERM) < 0)
    return -1;
  for (int attempt = 0; attempt < 100; attempt++) {
    int status;
    pid_t waited = waitpid(child, &status, WNOHANG);
    if (waited == child)
      return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
    if (waited < 0)
      return -1;
    struct timespec delay = {.tv_nsec = 50000000};
    nanosleep(&delay, nullptr);
  }
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  return -1;
}

static int connect_when_ready(int port) {
  struct sockaddr_in address = {.sin_family = AF_INET,
                                .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
                                .sin_port = htons((uint16_t)port)};
  for (int attempt = 0; attempt < 100; attempt++) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd >= 0 &&
        connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) == 0)
      return socket_fd;
    if (socket_fd >= 0)
      close(socket_fd);
    struct timespec delay = {.tv_nsec = 50000000};
    nanosleep(&delay, nullptr);
  }
  return -1;
}

static int send_text(int socket_fd, const char *text) {
  size_t sent = 0;
  const size_t LENGTH = strlen(text);
  while (sent < LENGTH) {
    ssize_t written =
        write(socket_fd, constant_buffer_suffix(text, LENGTH + 1, sent),
              LENGTH - sent);
    if (written <= 0)
      return -1;
    sent += (size_t)written;
  }
  return 0;
}

static bool contains(const char *buffer, size_t size, const char *expected) {
  const size_t EXPECTED_SIZE = strlen(expected);
  if (EXPECTED_SIZE > size)
    return false;
  for (size_t offset = 0; offset <= size - EXPECTED_SIZE; offset++)
    if (memcmp(checked_storage_at_const(buffer, size, sizeof(*buffer), offset),
               expected, EXPECTED_SIZE) == 0)
      return true;
  return false;
}

static int receive_until(int socket_fd, const char *expected, char *buffer,
                         size_t capacity) {
  size_t used = 0;
  for (int elapsed = 0; elapsed < TEST_TIMEOUT_MS; elapsed += 100) {
    struct pollfd readable = {.fd = socket_fd, .events = POLLIN};
    int result = poll(&readable, 1, 100);
    if (result < 0 && errno == EINTR)
      continue;
    if (result < 0)
      return -1;
    if (result == 0)
      continue;
    ssize_t count = read(socket_fd, buffer_suffix(buffer, capacity, used),
                         capacity - used - 1);
    if (count <= 0)
      return -1;
    used += (size_t)count;
    *(char *)checked_storage_at(buffer, capacity, sizeof(*buffer), used) = '\0';
    if (contains(buffer, used, expected))
      return 0;
    if (used == capacity - 1)
      break;
  }
  fprintf(stderr, "expected '%s', received '%s'\n", expected, buffer);
  return -1;
}

static int expect_text(int socket_fd, const char *expected) {
  char buffer[32768] = {};
  return receive_until(socket_fd, expected, buffer, sizeof(buffer));
}

static int login_as_god(int socket_fd) {
  return expect_text(socket_fd, "Who are you?") < 0 ||
                 send_text(socket_fd, "GOD\r\n") < 0 ||
                 expect_text(socket_fd, "Password:") < 0 ||
                 send_text(socket_fd, "btmuxr0x\r\n") < 0 ||
                 expect_text(socket_fd, "Connected.") < 0
             ? -1
             : 0;
}

static int create_object(int socket_fd, const char *name, long *dbref) {
  char command[256];
  char expected[256];
  char received[32768] = {};
  if (snprintf(command, sizeof(command), "@create %s\r\n", name) >=
          (int)sizeof(command) ||
      snprintf(expected, sizeof(expected), "%s created as object #", name) >=
          (int)sizeof(expected) ||
      send_text(socket_fd, command) < 0 ||
      receive_until(socket_fd, expected, received, sizeof(received)) < 0)
    return -1;
  const char *match = strstr(received, expected);
  if (match == nullptr)
    return -1;
  const char *number = checked_string_suffix(match, strlen(expected));
  char *end;
  *dbref = strtol(number, &end, 10);
  return end != number && *dbref >= 0 ? 0 : -1;
}

static int register_object(int socket_fd, long dbref, const char *type) {
  char command[128];
  if (snprintf(command, sizeof(command), "@btech/register #%ld=%s\r\n", dbref,
               type) >= (int)sizeof(command) ||
      send_text(socket_fd, command) < 0)
    return -1;
  return expect_text(socket_fd, "as BTech type") < 0 ? -1 : 0;
}

static int write_config(const char *path, int port) {
  FILE *file = fopen(path, "w");
  if (file == nullptr)
    return -1;
  const int RESULT = fprintf(file,
                             "[database]\n"
                             "game_database = \"data/stompymux.db\"\n"
                             "[server]\n"
                             "port = %d\n",
                             port) < 0 ||
                             fclose(file) != 0
                         ? -1
                         : 0;
  return RESULT;
}

int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  alarm(PROCESS_TIMEOUT_SECONDS);
  const char *server = *(const char *const *)checked_storage_at_const(
      argv, (size_t)argc, sizeof(*argv), 1);
  const char *directory = *(const char *const *)checked_storage_at_const(
      argv, (size_t)argc, sizeof(*argv), 2);
  char config[PATH_MAX];
  char database[PATH_MAX];
  int port = choose_port();
  if (port < 0 ||
      snprintf(config, sizeof(config), "%s/stompymux.toml", directory) >=
          (int)sizeof(config) ||
      snprintf(database, sizeof(database), "%s/data/stompymux.db", directory) >=
          (int)sizeof(database) ||
      (unlink(database) < 0 && errno != ENOENT) ||
      write_config(config, port) < 0)
    return 1;

  pid_t child = start_server(server, directory);
  int socket_fd = child > 0 ? connect_when_ready(port) : -1;
  long autopilot_one;
  long autopilot_two;
  long unit_one;
  long unit_two;
  int result = 1;
  if (socket_fd < 0 || login_as_god(socket_fd) < 0 ||
      create_object(socket_fd, "LuaAutopilotOne", &autopilot_one) < 0 ||
      register_object(socket_fd, autopilot_one, "AUTOPILOT") < 0 ||
      create_object(socket_fd, "LuaAutopilotTwo", &autopilot_two) < 0 ||
      register_object(socket_fd, autopilot_two, "AUTOPILOT") < 0 ||
      create_object(socket_fd, "LuaAutopilotUnitOne", &unit_one) < 0 ||
      register_object(socket_fd, unit_one, "MECH") < 0 ||
      create_object(socket_fd, "LuaAutopilotUnitTwo", &unit_two) < 0 ||
      register_object(socket_fd, unit_two, "MECH") < 0)
    goto done;
  char command[256];
  if (snprintf(command, sizeof(command), "autopilottest %ld %ld %ld %ld\r\n",
               autopilot_one, autopilot_two, unit_one,
               unit_two) >= (int)sizeof(command) ||
      send_text(socket_fd, command) < 0 ||
      expect_text(socket_fd, "Autopilot Lua passed") < 0)
    goto done;
  result = 0;
done:
  if (socket_fd >= 0)
    close(socket_fd);
  if (child > 0 && stop_server(child) < 0)
    result = 1;
  return result;
}
