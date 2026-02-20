#include "jsonrpc2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int read_input_line_pc(char *buffer, int bufferSize, int timeoutMillis) {

  // Create a file descriptor set containing stdin
  fd_set inputSet;
  FD_ZERO(&inputSet);
  FD_SET(fileno(stdin), &inputSet);

  // Set the timeout using timeval structure
  struct timeval timeout;
  timeout.tv_sec = timeoutMillis / 1000;
  timeout.tv_usec = (timeoutMillis % 1000) * 1000;

  // Wait for data to be available on stdin or timeout
  int result = select(fileno(stdin) + 1, &inputSet, NULL, NULL, &timeout);

  if (result == -1) {
    perror("select");
    return -1; // Error occurred
  } else if (result == 0) {
    return 1; // Timeout occurred
  }

  // Data is available, read a line from stdin
  if (fgets(buffer, bufferSize, stdin) == NULL) {
    return -1; // Error or EOF
  }

  // Remove the trailing newline character, if present
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
  }

  return 0; // Line read successfully
}

int read_input_line_arduino(char *buffer, int bufferSize, int timeoutMillis) {
#ifndef SIMULATOR
  SERIAL_PORT.setTimeout(timeoutMillis);
  if (!SERIAL_PORT.available()) {
    return -1;
  }
  // Read data from Serial until a newline character is encountered
  // LV_LOG_USER("buffer data before reading on serial:%s and length is %d\n", buffer,
  // strlen(buffer));
  size_t len = SERIAL_PORT.readBytesUntil('\n', buffer, bufferSize);
  SERIAL_PORT.flush();
  // LV_LOG_USER("read data on serial:%s and length is %d\n", buffer, len);
  // myprint(buffer);

  return 0;
#endif
}

// Function to read a line from stdin with a timeout in milliseconds
// Returns 0 if successful, 1 if timeout, and -1 on error
int read_input_line(char *buffer, int bufferSize, int timeoutMillis) {
  memset(buffer, 0, bufferSize);
#ifdef SIMULATOR
  return read_input_line_pc(buffer, bufferSize, timeoutMillis);
#else
  return read_input_line_arduino(buffer, bufferSize, timeoutMillis);
#endif
}

void myprint(const char *str) {
#ifdef SIMULATOR
  printf("%s\n", str);
#else
  SERIAL_PORT.println(str);
#endif
}
