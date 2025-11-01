/* gpiotime.c - CHIP program . */

/* This work is dedicated to the public domain under CC0 1.0 Universal:
 * http://creativecommons.org/publicdomain/zero/1.0/
 * 
 * To the extent possible under law, Steven Ford has waived all copyright
 * and related or neighboring rights to this work. In other words, you can 
 * use this code for any purpose without any restrictions.
 * This work is published from: United States.
 * Project home: https://github.com/fordsfords/gpiotime
 */

/* Allow compilation under c99 and POSIX 1995 / 2004. */
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* See http://wiki.geeky-boy.com/w/index.php?title=Internal_error_handling */
#define WCHK(cond_expr) do { \
  if (!(cond_expr)) { \
    fprintf(stderr, "%s:%d, Warning, expected '%s' to be true (%s)\n", \
      __FILE__, __LINE__, #cond_expr, strerror(errno)); \
  }  \
} while (0)

#define ECHK(cond_expr) do { \
  if (!(cond_expr)) { \
    fprintf(stderr, "%s:%d, Error, expected '%s' to be true (%s)\n", \
      __FILE__, __LINE__, #cond_expr, strerror(errno)); \
    abort(); \
  }  \
} while (0)

/* See http://blog.geeky-boy.com/2014/04/strtoul-preferred-over-atoi.html */
#define SAFE_ATOL(a,l) do { \
  char *in_a = a;  char *temp = NULL;  long result;  errno = 0; \
  if (*in_a == '0' && *(in_a+1) == 'x') \
    result = strtol(in_a+2, &temp, 16); \
  else \
    result = strtol(in_a, &temp, 10); \
  if (errno != 0 || temp == in_a || temp == NULL || *temp != '\0') { \
    fprintf(stderr, "%s:%d, Error, invalid numeric value for %s: '%s'\n", \
       __FILE__, __LINE__, #l, in_a); \
    exit(1); \
  } \
  l = result; /* "return" value of macro */ \
} while (0)

 
double tv_diff(struct timespec *first, struct timespec *second)
{
  double rtn = (second->tv_sec - first->tv_sec) + (second->tv_nsec - first->tv_nsec) * 1e-9;
  return rtn;
}  /* tv_diff */


volatile int vol_int;  /* volatile to prevent compiler optimizations during CPU burn loop */

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: gpiotime gpionum\nWhere: gpionum can be 132-139 for CSID0-CSID7, 408-415 for XIOP0-XIOP7, etc.\n");
    return 0;
  }
  long port;  SAFE_ATOL(argv[1], port);
  char port_asc[80];  memset(port_asc, '\0', sizeof(port_asc));
  snprintf(port_asc, sizeof(port_asc), "%ld", port);
  ECHK(port_asc[sizeof(port_asc)-2] == '\0');  /* make sure snprintf didn't truncate */

  /* Enable GPIO line. */
  int export_fd = open("/sys/class/gpio/export", O_WRONLY);  ECHK(export_fd != -1);
  /* If port is already exported, the following will fail. That's OK, but warn. */
  int wlen = write(export_fd, port_asc, strlen(port_asc)+1);  WCHK(wlen == strlen(port_asc)+1);
  close(export_fd);

  /* Set direction initially to "input" */
  char direction_fn[80];  memset(direction_fn, '\0', sizeof(direction_fn));
  snprintf(direction_fn, sizeof(direction_fn), "/sys/class/gpio/gpio%ld/direction", port);
  ECHK(direction_fn[sizeof(direction_fn)-2] == '\0');  /* make sure snprintf didn't truncate */
  int dir_fd = open(direction_fn, O_WRONLY);  ECHK(dir_fd != -1);
  wlen = write(dir_fd, "in", 3);  ECHK(wlen == 3);

  /* Get fd to read/write the value of the line */
  char value_fn[80];  memset(value_fn, '\0', sizeof(value_fn));
  snprintf(value_fn, sizeof(value_fn), "/sys/class/gpio/gpio%ld/value", port);
  ECHK(value_fn[sizeof(value_fn)-2] == '\0');  /* make sure snprintf didn't truncate */
  int value_fd = open(value_fn, O_RDWR);  ECHK(value_fd != -1);

  sleep(1);  /* allow startup transients to settle. */

  /* Get chip out of low-power mode */
  for (int j = 0; j < 100000000; j++) {
    ++vol_int;
  }

#define NUM_LOOPS 5
  struct timespec first[NUM_LOOPS], second[NUM_LOOPS], third[NUM_LOOPS], fourth[NUM_LOOPS], fifth[NUM_LOOPS];

  for (int i = 0; i < NUM_LOOPS; ++i) {
    char readbuf[99] = "";
    int st;

    /* Time input operations */

    wlen = write(dir_fd, "in", 3);  ECHK(wlen == 3);

    st = clock_gettime(CLOCK_MONOTONIC, &first[i]);  ECHK(st == 0);

    for (int j = 0; j < 10000; ++j) {
      st = lseek(value_fd, 0, SEEK_SET);  ECHK(st == 0);
    }

    st = clock_gettime(CLOCK_MONOTONIC, &second[i]);  ECHK(st == 0);

    for (int j = 0; j < 10000; ++j) {
      st = lseek(value_fd, 0, SEEK_SET);  ECHK(st == 0);
      int rlen = read(value_fd, readbuf, sizeof(readbuf));  ECHK(rlen > 0);
    }

    st = clock_gettime(CLOCK_MONOTONIC, &third[i]);  ECHK(st == 0);

    /* Time output operations */

    wlen = write(dir_fd, "out", 4);  ECHK(wlen == 4);

    st = clock_gettime(CLOCK_MONOTONIC, &fourth[i]);  ECHK(st == 0);

    for (int j = 0; j < 5000; ++j) {
      wlen = write(value_fd, "1", 2);  ECHK(wlen == 2);
      wlen = write(value_fd, "0", 2);  ECHK(wlen == 2);
    }

    st = clock_gettime(CLOCK_MONOTONIC, &fifth[i]);  ECHK(st == 0);
  }  /* for i */

  /* Wrap up */
  close(value_fd);

  wlen = write(dir_fd, "in", 3);  ECHK(wlen == 3);
  close(dir_fd);

  /* Release GPIO */
  export_fd = open("/sys/class/gpio/unexport", O_WRONLY);  ECHK(export_fd != -1);
  wlen = write(export_fd, port_asc, strlen(port_asc)+1);  WCHK(wlen == strlen(port_asc)+1);
  close(export_fd);

  /* Print results */
  for (int i = 0; i < NUM_LOOPS; i++) {
    printf("Sample: %d\n", i);
    printf("lseek in: %lf us\n", 1000000/10000 * tv_diff(&first[i], &second[i]));
    printf("lseek/read in: %lf us\n", 1000000/10000 * tv_diff(&second[i], &third[i]));
    printf("write out: %lf us\n", 1000000/10000 * tv_diff(&fourth[i], &fifth[i]));
  }

  return 0;
}  /* main */
