// common.c -- Defines some global functions.
//             From JamesM's kernel development tutorials.

#include "common.h"

/*
// Write a byte out to the specified port.
void outb(u16int port, u8int value) {
  asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

u8int inb(u16int port) {
  u8int ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}

u16int inw(u16int port) {
  u16int ret;
  asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}
*/

// Copy len bytes from src to dest.
void memcpy(u8int *dest, const u8int *src, u32int len) {
  const u8int *sp = (const u8int *)src;
  u8int *dp = (u8int *)dest;
  for (; len != 0; len--)
    *dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(u8int *dest, u8int val, u32int len) {

  u8int *temp = (u8int *)dest;
  for (; len != 0; len--) {
    *temp++ = val;
  }
}

// Compare two strings. Should return -1 if
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(char *str1, char *str2) {
  int i = 0;
  int failed = 0;
  while (str1[i] != '\0' && str2[i] != '\0') {
    if (str1[i] != str2[i]) {
      failed = 1;
      break;
    }
    i++;
  }
  // why did the loop exit?
  if ((str1[i] == '\0' && str2[i] != '\0') ||
      (str1[i] != '\0' && str2[i] == '\0'))
    failed = 1;

  return failed;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src) {
  do {
    *dest++ = *src++;
  } while (*src != 0);
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src) {
  while (*dest != 0) {
    *dest = *dest++;
  }

  do {
    *dest++ = *src++;
  } while (*src != 0);
  return dest;
}

/* Clear the screen and initialize VIDEO, XPOS and YPOS. */
void cls(void) {

  int i;

  video = (unsigned char *)VIDEO;

  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  xpos = 0;
  ypos = 0;
}

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal. */
void itoa(char *buf, int base, int d) {

  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0. */
  do {
    int remainder = ud % divisor;

    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  /* Terminate BUF. */
  *p = 0;

  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }
}

/* Put the character C on the screen. */
void putchar(int c) {

  if (c == '\n' || c == '\r') {
  newline:
    xpos = 0;
    ypos++;
    if (ypos >= LINES)
      ypos = 0;
    return;
  }

  *(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
  *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

  xpos++;
  if (xpos >= COLUMNS)
    goto newline;
}

/* Format a string and print it on the screen, just like the libc
   function printf. */
void printf(const char *format, ...) {

  vga_index += 80;

  char **arg = (char **)&format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {

    if (c != '%') {

      putchar(c);

    } else {

      char *p;

      // read next character
      c = *format++;

      switch (c) {

      // all numerics
      case 'd':
      case 'u':
      case 'x':

        // encode the current argument into the buffer
        itoa(buf, c, *((int *)arg++));
        p = buf;
        goto string;
        break;

      case 's':
        p = *arg++;
        if (!p)
          p = "(null)";

      string:
        // output a string
        while (*p) {
          putchar(*p++);
        }
        break;

      default:
        putchar(*((int *)arg++));
        break;
      }
    }
  }
}

void clear_screen(void) {

  int index = 0;
  /* there are 25 lines each of 80 columns; each element takes 2 bytes */
  while (index < 80 * 25 * 2) {
    terminal_buffer[index] = ' ';
    index += 2;
  }
}

void print_string(char *str, unsigned char color) {

  int index = 0;
  while (str[index]) {
    terminal_buffer[vga_index] =
        (unsigned short)str[index] | (unsigned short)color << 8;
    index++;
    vga_index++;
  }
}
