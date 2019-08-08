#include <stdio.h>
#include <propeller.h>

int getch(void)
{
    int ch = getchar();
    if (ch == '\r')
        putchar('\n');
    return ch;
}

void sleep(int secs)
{
    _waitx(_clockfreq() * secs);
}

