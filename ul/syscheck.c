#include <stdio.h>
#include <stdlib.h>

int main() {
    // Example of using system() to execute a shell command
    printf("Running ls command:\n");
    system("ls");  // This will list files and directories in the current directory

    return 0;
}
