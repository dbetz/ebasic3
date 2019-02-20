#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "db_vm.h"

void VM_sysinit(int argc, char *argv[])
{
}

char *VM_getline(char *buf, int size)
{
#ifdef LINE_EDIT
    int i = 0;
    while (i < size - 1) {
        int ch = VM_getchar();
        if (ch == '\n') {
            buf[i++] = '\n';
#ifdef ECHO_INPUT
            VM_putchar('\n');
#endif
            break;
        }
        else if (ch == '\b' || ch == 0x7f) {
            if (i > 0) {
#ifdef ECHO_INPUT
                VM_putchar('\b');
#endif
                VM_putchar(' ');
                VM_putchar('\b');
                VM_flush();
                --i;
            }
        }
        else {
            buf[i++] = ch;
#ifdef ECHO_INPUT
            VM_putchar(ch);
            VM_flush();
#endif
        }
    }
    buf[i] = '\0';
#else
    fgets(buf, size, stdin);
#endif
    return buf;
}

void VM_vprintf(const char *fmt, va_list ap)
{
    char buf[80], *p = buf;
    vsprintf(buf, fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
}

void VM_flush(void)
{
}

int VM_getchar(void)
{
    int ch = getch();
    if (ch == '\r')
        ch = '\n';
    return ch;
}

void VM_putchar(int ch)
{
    if (ch == '\n')
        putchar('\r');
    putchar(ch);
}

void LOG_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_vprintf(fmt, ap);
    va_end(ap);
}

int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && (tolower(*s1) == tolower(*s2))) {
        ++s1;
        ++s2;
    }
    return tolower((unsigned char) *s1) - tolower((unsigned char) *s2);
}

#ifdef LOAD_SAVE

int VM_opendir(const char *path, VMDIR *dir)
{
    return -1;
}

int VM_readdir(VMDIR *dir, VMDIRENT *entry)
{
    return -1;
}

void VM_closedir(VMDIR *dir)
{
}

#endif
