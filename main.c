#include "showroom_logic.h"
#include <stdio.h>

int main(void)
{
    printf("\nInitializing Car Showroom CRM (Disk-Backed B-Trees)...\n");

    if (init_showrooms() != 0) {
        return -1;
    }

    /* Load text data if databases are currently empty */
    load_initial_data();

    /* Launch interactive user menu */
    run_showroom_menu();

    /* Ensure all modified disk nodes are flushed cleanly */
    close_showrooms();

    return 0;
}
