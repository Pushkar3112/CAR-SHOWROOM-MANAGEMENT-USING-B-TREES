/*
 * showroom_logic.h
 *
 * Implements the Car Showroom CRM features, data loading from text files, and CLI menu.
 */

#ifndef SHOWROOM_LOGIC_H
#define SHOWROOM_LOGIC_H

#include "btree.h"
#include <stdio.h>

/* Number of distinct showrooms */
#define NUM_SHOWROOMS 3

/* Structure representing a single showroom's databases (4 B-Trees + 4 Data Files) */
typedef struct {
    int id;

    BTree *a_cars_bt;
    FILE  *a_cars_db;

    BTree *s_cars_bt;
    FILE  *s_cars_db;

    BTree *customers_bt;
    FILE  *customers_db;

    BTree *salespersons_bt;
    FILE  *salespersons_db;
} ShowroomContext;

/* Global array of 3 showrooms */
extern ShowroomContext showrooms[NUM_SHOWROOMS];

/*
 * Initialize the B-Trees and open data files for all 3 showrooms.
 * Creates "data/" directory if needed.
 */
int init_showrooms(void);

/*
 * Closes all B-Trees and Data Files.
 */
void close_showrooms(void);

/*
 * Reads `showroom1.txt`, `showroom2.txt` etc. line-by-line and populate DBs
 * if they are currently empty.
 */
void load_initial_data(void);

/*
 * Launches the interactive CLI menu for the user.
 */
void run_showroom_menu(void);

#endif /* SHOWROOM_LOGIC_H */
