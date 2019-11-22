// common.c -- Defines some global functions.
//             From JamesM's kernel development tutorials.

#include "common.h"

// write a byte out to the specified port.
void outb(u8int value, u16int port) {

  __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(value));
}

// write a word (16 bit) out to the specified port.
void outw(u16int value, u16int port) {

  __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(value));
}

// "l" meaning long (32bits)
void outl(u32int value, u16int port) {

  __asm__ __volatile__("outl %1, %0" : : "dN"(port), "a"(value));
}

// read a byte from the specified port.
u8int inb(u16int port) {

  u8int ret;
  __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));

  return ret;
}

// read a word from the specified port.
u16int inw(u16int port) {

  u16int ret;
  __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "dN"(port));

  return ret;
}

// read a word from the specified port.
u32int inl(u16int port) {

  u32int ret;
  __asm__ __volatile__("inl %1, %0" : "=a"(ret) : "dN"(port));

  return ret;
}

// copy len bytes from src to dest.
void k_memcpy(u8int *dest, const u8int *src, u32int len) {

  const u8int *sp = (const u8int *)src;
  u8int *dp = (u8int *)dest;
  for (; len != 0; len--) {
    *dp++ = *sp++;
  }
}

// write len copies of val into dest.
void k_memset(u8int *dest, u8int val, u32int len) {

  u8int *temp = (u8int *)dest;
  for (; len != 0; len--) {
    *temp++ = val;
  }
}

// compare two strings. Should return -1 if
// str1 < str2, 0 if they are equal or 1 otherwise.
int k_strcmp(char *str1, char *str2) {

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
      (str1[i] != '\0' && str2[i] == '\0')) {
    failed = 1;
  }

  return failed;
}

// copy the NULL-terminated string src into dest, and
// return dest.
char *k_strcpy(char *dest, const char *src) {

  do {
    *dest++ = *src++;
  } while (*src != 0);
}

// concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *k_strcat(char *dest, const char *src) {

  while (*dest != 0) {
    *dest = *dest++;
  }

  do {
    *dest++ = *src++;
  } while (*src != 0);

  return dest;
}

// clear the screen and initialize VIDEO, XPOS and YPOS.
void k_cls(void) {

  int i;

  video = (unsigned char *)VIDEO;

  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  xpos = 0;
  ypos = 0;
}

// Convert the integer D to a string and save the string in BUF. If
// BASE is equal to 'd', interpret that D is decimal, and if BASE is
// equal to 'x', interpret that D is hexadecimal.
void k_itoa(char *buf, int base, int d) {

  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  // If %d is specified and D is minus, put `-' in the head.
  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x') {
    divisor = 16;
  }

  // Divide UD by DIVISOR until UD == 0.
  do {
    int remainder = ud % divisor;

    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  // Terminate BUF.
  *p = 0;

  // Reverse BUF.
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

// Put the character C on the screen.
void k_putchar(int c) {

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

// Format a string and print it on the screen, just like the libc
// function printf.
//
//   Does not support float or double.
//
//   Is buggy when using more than one format specifier in the same format
//   string.
//
void k_printf(const char *format, ...) {

  vga_index += 80;

  char **arg = (char **)&format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {

    if (c != '%') {

      k_putchar(c);

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
        k_itoa(buf, c, *((int *)arg++));
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
          k_putchar(*p++);
        }
        break;

      default:
        k_putchar(*((int *)arg++));
        break;
      }
    }
  }
}

void k_clear_screen(void) {

  int index = 0;
  // there are 25 lines each of 80 columns; each element takes 2 bytes
  while (index < 80 * 25 * 2) {
    terminal_buffer[index] = ' ';
    index += 2;
  }
}

void k_print_string(char *str, unsigned char color) {

  int index = 0;
  while (str[index]) {
    terminal_buffer[vga_index] =
        (unsigned short)str[index] | (unsigned short)color << 8;
    index++;
    vga_index++;
  }
}

void k_print_float(float data) {

  char int_part_buf[20];
  for (int i = 0; i < 20; i++) {
    int_part_buf[i] = 0;
  }

  char fraction_part_buf[20];
  for (int i = 0; i < 20; i++) {
    fraction_part_buf[i] = 0;
  }

  int int_part = (int)data;

  float temp = data - (float)int_part;

  unsigned int max_fractions = 3;
  while ((temp - (int)temp) > 0.0f && max_fractions > 0) {
    temp *= 10.0f;
    max_fractions--;
  }

  int float_part = temp;

  // printf("int %d frac %d\n", int_part, float_part);

  k_itoa(int_part_buf, 10, int_part);
  k_itoa(fraction_part_buf, 10, float_part);

  // printf("int_part_buf %s\n", int_part_buf);
  // printf("fraction_part_buf %s\n", fraction_part_buf);

  char total_buf[41];
  int total_buf_index = 0;
  for (int i = 0; i < 41; i++) {
    total_buf[i] = 0;
  }

  // printf("inserting int part\n");

  // insert the integral part of the floating point number into the full array
  for (int i = 0; i < 20; i++) {

    if (int_part_buf[i] == 0) {
      continue;
    }

    total_buf[total_buf_index] = int_part_buf[i];
    total_buf_index++;
  }

  // printf("inserting dot\n");

  total_buf[total_buf_index] = '.';
  total_buf_index++;

  // printf("inserting frac part\n");

  // insert the fractional part of the floating point number into the full array
  for (int i = 0; i < 20; i++) {

    if (fraction_part_buf[i] == 0) {
      continue;
    }

    total_buf[total_buf_index] = fraction_part_buf[i];
    total_buf_index++;
  }

  // printf("output total_buf_index %d\n", total_buf_index);

  // printf("float %s\n", total_buf);

  // k_putchar segfaults during mockito tests because it accesses video memory
  // which is off limits for cmockito user space applications on modern linux
  // systems with paging enabled!

  for (int i = 0; i <= total_buf_index; i++) {
    k_putchar(total_buf[i]);
  }
}

extern void panic(const char *message, const char *file, u32int line) {

  // We encountered a massive problem and have to stop.
  __asm__ __volatile__("cli"); // Disable interrupts.

  k_printf("PANIC(");
  k_printf("%s", message);
  k_printf(") at ");
  k_printf("%s", file);
  k_printf(":");
  k_printf("%d", line);
  k_printf("\n");

  // Halt by going into an infinite loop.
  for (;;)
    ;
}