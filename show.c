#include <stdio.h>
#include <unistd.h>     // for sleep & usleep

#include "show.h"

// --------------------------------------------
// LINE 1 = PROGRESS
// LINE 2 = TEMPORARY STEP MESSAGE
// --------------------------------------------

// Update only the PROGRESS line (line 1)
void update_main_progress(const char *msg, int percent)
{
    // Move cursor UP to line 1
    printf("\033[1A");

    // Clear entire line 1
    printf("\033[2K");

    // Print progress
    printf("\r%s %3d%%\n", msg, percent);

    fflush(stdout);
    // Cursor is now automatically on line 2 because of '\n'
}


// Show TEMPORARY message at line 2
void show(const char *msg)
{
    // Clear line 2 before printing message
    printf("\033[2K\r");
    printf("%s", msg);
    fflush(stdout);

    sleep(1);   // keep message visible

    // Clear line 2 again (remove the message)
    printf("\033[2K\r");
    fflush(stdout);
}
