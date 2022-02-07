
// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
int main()
{

    // Allocate memory for read buffer, set size according to your needs
    char read_buf[] = "aaaaa}{bbbb";
    char *tok = strtok(read_buf, "{");
    tok = strtok(NULL, "{");
    char json_buf[128];
    char help_buf[256];
    int c = 0;
    // Normally you wouldn't do this memset() call, but since we will just receive
    // ASCII data for this example, we'll set everything to 0 so we can
    // call printf() easily.
    memset(&json_buf, '\0', sizeof(json_buf));
    memset(&help_buf, '\0', sizeof(help_buf));
    json_buf[0] = '{';
    int num_bytes;
    int exit = 0;
    while (1)
    {
        // memset(&read_buf, '\0', sizeof(read_buf));

        for (int i = 0; i < sizeof(read_buf); i++)
        {
            if (read_buf[i] == '{')
            {
                char *tok1 = strtok(read_buf, "{");
                tok1 = strtok(NULL, "{");
                
                strcat(json_buf, tok1);
                
            }
            if (read_buf[i] == '}')
            {
                char *tok2 = strtok(read_buf, "}");
                strcat(help_buf, tok2);
                
            }
        }
        if (exit)
        {
            printf("Here");
            break;
        }
        strcat(json_buf, read_buf);
    }
    strcat(json_buf, help_buf);
    printf("%s\n", json_buf);

    return 0;
}