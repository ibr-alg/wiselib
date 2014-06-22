#include <stdio.h>
#include "integer.h"

int main() {
const char* path = "  /hello/world.txt";
printf("%s\n", path);
    while (*path == ' ') path++;		/* Skip leading spaces */
	if (*path == '/') path++;			/* Strip heading separator */
    if ((BYTE)*path <= ' ')
        printf("Heya");
    printf("%s\n", path);
    return 0;
}
