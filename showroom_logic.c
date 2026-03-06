/*
 * showroom_logic.c
 *
 * Implements the loading and execution of the Showroom CRM.
 */

#include "showroom_logic.h"
#include "showroom_db.h"
#include "hash_analytics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ShowroomContext showrooms[NUM_SHOWROOMS];

int init_showrooms(void)
{
    char filepath_bt[128];
    char filepath_db[128];

    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        showrooms[i].id = i + 1;

        /* Available Cars */
        sprintf(filepath_bt, "data/sr%d_a_cars.idx", i+1);
        sprintf(filepath_db, "data/sr%d_a_cars.dat", i+1);
        showrooms[i].a_cars_bt = btree_open(filepath_bt);
        showrooms[i].a_cars_db = fopen(filepath_db, "ab+");

        /* Sold Cars */
        sprintf(filepath_bt, "data/sr%d_s_cars.idx", i+1);
        sprintf(filepath_db, "data/sr%d_s_cars.dat", i+1);
        showrooms[i].s_cars_bt = btree_open(filepath_bt);
        showrooms[i].s_cars_db = fopen(filepath_db, "ab+");

        /* Customers */
        sprintf(filepath_bt, "data/sr%d_customers.idx", i+1);
        sprintf(filepath_db, "data/sr%d_customers.dat", i+1);
        showrooms[i].customers_bt = btree_open(filepath_bt);
        showrooms[i].customers_db = fopen(filepath_db, "ab+");

        /* Salespersons */
        sprintf(filepath_bt, "data/sr%d_salespersons.idx", i+1);
        sprintf(filepath_db, "data/sr%d_salespersons.dat", i+1);
        showrooms[i].salespersons_bt = btree_open(filepath_bt);
        showrooms[i].salespersons_db = fopen(filepath_db, "ab+");

        if (!showrooms[i].a_cars_bt || !showrooms[i].a_cars_db ||
            !showrooms[i].s_cars_bt || !showrooms[i].s_cars_db ||
            !showrooms[i].customers_bt || !showrooms[i].customers_db ||
            !showrooms[i].salespersons_bt || !showrooms[i].salespersons_db) {
            fprintf(stderr, "Failed to initialize databases for Showroom %d.\n", i+1);
            return -1;
        }
    }
    return 0;
}

void close_showrooms(void)
{
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        if (showrooms[i].a_cars_bt) btree_close(showrooms[i].a_cars_bt);
        if (showrooms[i].a_cars_db) fclose(showrooms[i].a_cars_db);

        if (showrooms[i].s_cars_bt) btree_close(showrooms[i].s_cars_bt);
        if (showrooms[i].s_cars_db) fclose(showrooms[i].s_cars_db);

        if (showrooms[i].customers_bt) btree_close(showrooms[i].customers_bt);
        if (showrooms[i].customers_db) fclose(showrooms[i].customers_db);

        if (showrooms[i].salespersons_bt) btree_close(showrooms[i].salespersons_bt);
        if (showrooms[i].salespersons_db) fclose(showrooms[i].salespersons_db);
    }
}

/* 
 * Helper to process one line from the text file 
 */
static void parse_and_insert(ShowroomContext *sr, const char *line, int section)
{
    /* 0: Available, 1: Salespersons, 2: Customers, 3: Sold Cars */
    long offset;
    
    if (section == 0) {
        a_car car = {0};
        if (sscanf(line, "%d,%49[^,],%19[^,],%f,%19[^,],%19[^\n]", 
                   &car.VIN, car.name, car.color, &car.price, car.fuelType, car.carType) >= 5) {
            
            offset = db_append_record(sr->a_cars_db, &car, sizeof(a_car));
            if (offset >= 0) btree_insert(sr->a_cars_bt, car.VIN, offset);
        }
    } else if (section == 1) {
        Salesperson sp = {0};
        if (sscanf(line, "%d,%49[^,],%f,%f", 
                   &sp.id, sp.name, &sp.salesTarget, &sp.salesAchieved) >= 3) {
            
            offset = db_append_record(sr->salespersons_db, &sp, sizeof(Salesperson));
            if (offset >= 0) btree_insert(sr->salespersons_bt, sp.id, offset);
        }
    } else if (section == 2) {
        Customer cust = {0};
        if (sscanf(line, "%d,%49[^,],%14[^,],%99[^,],%19[^,],%9[^,],%d",
                   &cust.customerid, cust.name, cust.mobileNo, cust.address, 
                   cust.registrationNo, cust.paymentType, &cust.car_VIN) >= 6) {
            
            offset = db_append_record(sr->customers_db, &cust, sizeof(Customer));
            if (offset >= 0) btree_insert(sr->customers_bt, cust.customerid, offset);
        }
    } else if (section == 3) {
        s_car scar = {0};
        if (sscanf(line, "%d,%49[^,],%19[^,],%f,%19[^,],%19[^,],%9[^,],%d-%d-%d",
                   &scar.VIN, scar.name, scar.color, &scar.price, scar.fuelType, scar.carType, 
                   scar.paymenttype, &scar.solddate.year, &scar.solddate.month, &scar.solddate.day) >= 7) {
            
            offset = db_append_record(sr->s_cars_db, &scar, sizeof(s_car));
            if (offset >= 0) btree_insert(sr->s_cars_bt, scar.VIN, offset);
        }
    }
}

void load_initial_data(void)
{
    char filename[32];
    char line[512];

    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        /* Only load if the DB is empty (i.e. root offset is -1 in the BTree) */
        if (showrooms[i].a_cars_bt->root_offset >= 0) {
            continue; /* DB already populated from disk */
        }

        sprintf(filename, "showroom%d.txt", i+1);
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            /* Create some dummy data if file is missing */
            printf("Warning: %s not found. Skipping initial load for SR%d\n", filename, i+1);
            continue;
        }

        int section = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (line[0] == '\r' || line[0] == '\n') continue;
            
            if (strncmp(line, "---Available", 12) == 0) { section = 0; continue; }
            if (strncmp(line, "---Salespersons", 15) == 0) { section = 1; continue; }
            if (strncmp(line, "---Customers", 12) == 0) { section = 2; continue; }
            if (strncmp(line, "---Sold", 7) == 0) { section = 3; continue; }

            parse_and_insert(&showrooms[i], line, section);
        }
        fclose(fp);
    }
}

/* ------------------------------------------------------------------ */
/*  Business Logic & Query Helpers                                     */
/* ------------------------------------------------------------------ */

static void print_a_car(const a_car *c) {
    if (!c) return;
    printf("VIN: %d | Name: %s | Color: %s | Price: %.2f | Fuel: %s | Type: %s\n",
           c->VIN, c->name, c->color, c->price, c->fuelType, c->carType);
}

static void print_s_car(const s_car *c) {
    if (!c) return;
    printf("VIN: %d | Name: %s | Color: %s | Price: %.2f | Date: %02d-%02d-%04d | Pmt: %s\n",
           c->VIN, c->name, c->color, c->price, c->solddate.day, c->solddate.month, c->solddate.year, c->paymenttype);
}

static void print_customer(const Customer *c) {
    if (!c) return;
    printf("ID: %d | Name: %s | Phone: %s | PmtType: %s | CarVIN: %d\n",
           c->customerid, c->name, c->mobileNo, c->paymentType, c->car_VIN);
}

static void print_salesperson(const Salesperson *s) {
    if (!s) return;
    printf("ID: %d | Name: %s | Target: %.2f | Achieved: %.2f | Comm: %.2f\n",
           s->id, s->name, s->salesTarget, s->salesAchieved, s->commission);
}

/* Recursive helper to iterate B-Tree nodes */
static void traverse_a_cars(BTree *bt, long node_offset, FILE *db) {
    if (node_offset < 0) return;
    DiskNode node;
    dm_read_node(bt->dm, node_offset, &node);
    
    for (int i = 0; i < node.num_keys; i++) {
        if (!node.is_leaf) traverse_a_cars(bt, node.children[i], db);
        a_car c;
        if (db_read_record(db, node.values[i], &c, sizeof(a_car))) {
            print_a_car(&c);
        }
    }
    if (!node.is_leaf) traverse_a_cars(bt, node.children[node.num_keys], db);
}

static void traverse_s_cars(BTree *bt, long node_offset, FILE *db, HashTable *ht) {
    if (node_offset < 0) return;
    DiskNode node;
    dm_read_node(bt->dm, node_offset, &node);
    
    for (int i = 0; i < node.num_keys; i++) {
        if (!node.is_leaf) traverse_s_cars(bt, node.children[i], db, ht);
        s_car c;
        if (db_read_record(db, node.values[i], &c, sizeof(s_car))) {
            print_s_car(&c);
            if (ht) ht_increment_car(ht, c.name);
        }
    }
    if (!node.is_leaf) traverse_s_cars(bt, node.children[node.num_keys], db, ht);
}

static void traverse_customers(BTree *bt, long node_offset, FILE *db, int emi_only) {
    if (node_offset < 0) return;
    DiskNode node;
    dm_read_node(bt->dm, node_offset, &node);
    
    for (int i = 0; i < node.num_keys; i++) {
        if (!node.is_leaf) traverse_customers(bt, node.children[i], db, emi_only);
        Customer c;
        if (db_read_record(db, node.values[i], &c, sizeof(Customer))) {
            if (!emi_only || strcasecmp(c.paymentType, "EMI") == 0) {
                print_customer(&c);
            }
        }
    }
    if (!node.is_leaf) traverse_customers(bt, node.children[node.num_keys], db, emi_only);
}

static void traverse_salespersons(BTree *bt, long node_offset, FILE *db, Salesperson *best_sp, float *total_sales, int *sp_count, float min_s, float max_s) {
    if (node_offset < 0) return;
    DiskNode node;
    dm_read_node(bt->dm, node_offset, &node);
    
    for (int i = 0; i < node.num_keys; i++) {
        if (!node.is_leaf) traverse_salespersons(bt, node.children[i], db, best_sp, total_sales, sp_count, min_s, max_s);
        Salesperson sp;
        if (db_read_record(db, node.values[i], &sp, sizeof(Salesperson))) {
            print_salesperson(&sp);
            
            if (best_sp && sp.salesAchieved > best_sp->salesAchieved) {
                *best_sp = sp;
            }
            if (total_sales) *total_sales += sp.salesAchieved;
            if (sp_count) (*sp_count)++;
            
            if (min_s >= 0 && sp.salesAchieved >= min_s && sp.salesAchieved <= max_s) {
                printf("  -> IN RANGE: %s (%.2f)\n", sp.name, sp.salesAchieved);
            }
        }
    }
    if (!node.is_leaf) traverse_salespersons(bt, node.children[node.num_keys], db, best_sp, total_sales, sp_count, min_s, max_s);
}

/* ------------------------------------------------------------------ */
/*  Menu Operations                                                    */
/* ------------------------------------------------------------------ */

void print_showroom_data(int sr_idx)
{
    printf("\n=== Showroom %d Data ===\n", sr_idx + 1);
    ShowroomContext *sr = &showrooms[sr_idx];
    
    printf("\n-- Available Cars --\n");
    traverse_a_cars(sr->a_cars_bt, sr->a_cars_bt->root_offset, sr->a_cars_db);
    
    printf("\n-- Sold Cars --\n");
    traverse_s_cars(sr->s_cars_bt, sr->s_cars_bt->root_offset, sr->s_cars_db, NULL);
    
    printf("\n-- Customers --\n");
    traverse_customers(sr->customers_bt, sr->customers_bt->root_offset, sr->customers_db, 0);
    
    printf("\n-- Salespersons --\n");
    traverse_salespersons(sr->salespersons_bt, sr->salespersons_bt->root_offset, sr->salespersons_db, NULL, NULL, NULL, -1, -1);
}

void find_most_popular_car_all(void)
{
    HashTable *ht = ht_create();
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        traverse_s_cars(showrooms[i].s_cars_bt, showrooms[i].s_cars_bt->root_offset, showrooms[i].s_cars_db, ht);
    }
    
    char pop_name[50];
    int count = ht_get_most_popular(ht, pop_name);
    if (count > 0) {
        printf("\nMost Popular Car: %s with %d sales.\n", pop_name, count);
    } else {
        printf("\nNo cars have been sold yet.\n");
    }
    ht_destroy(ht);
}

void find_best_salesperson_all(void)
{
    Salesperson best_sp = {0};
    best_sp.salesAchieved = -1;
    
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        traverse_salespersons(showrooms[i].salespersons_bt, showrooms[i].salespersons_bt->root_offset, showrooms[i].salespersons_db, &best_sp, NULL, NULL, -1, -1);
    }
    
    if (best_sp.salesAchieved >= 0) {
        best_sp.commission += 0.01 * best_sp.salesAchieved;
        printf("\nMost Successful Salesperson: %s with Sales: %.2f and Comm: %.2f\n", 
               best_sp.name, best_sp.salesAchieved, best_sp.commission);
    }
}

void predict_sales(void)
{
    float total = 0;
    int count = 0;
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        traverse_salespersons(showrooms[i].salespersons_bt, showrooms[i].salespersons_bt->root_offset, showrooms[i].salespersons_db, NULL, &total, &count, -1, -1);
    }
    if (count > 0) {
        printf("\nPredicted Next Month Total Sales: %.2f\n", total / count);
    }
}

void display_car_info(int vin)
{
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        long offset = btree_search(showrooms[i].a_cars_bt, vin);
        if (offset >= 0) {
            a_car c;
            db_read_record(showrooms[i].a_cars_db, offset, &c, sizeof(a_car));
            printf("\nFOUND AVAILABLE CAR in SR%d: \n", i+1);
            print_a_car(&c);
            return;
        }
        
        offset = btree_search(showrooms[i].s_cars_bt, vin);
        if (offset >= 0) {
            s_car c;
            db_read_record(showrooms[i].s_cars_db, offset, &c, sizeof(s_car));
            printf("\nFOUND SOLD CAR in SR%d: \n", i+1);
            print_s_car(&c);
            return;
        }
    }
    printf("\nCar with VIN %d NOT FOUND in any showroom.\n", vin);
}

void list_emi_all(void)
{
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        printf("\n== EMI Customers in Showroom %d ==\n", i+1);
        traverse_customers(showrooms[i].customers_bt, showrooms[i].customers_bt->root_offset, showrooms[i].customers_db, 1);
    }
}

void sell_car_interactive(void)
{
    int sr_idx, vin, sp_id, cust_id;
    printf("Enter Showroom ID (1-3): ");
    scanf("%d", &sr_idx); sr_idx--;

    if (sr_idx < 0 || sr_idx >= NUM_SHOWROOMS) { printf("Invalid SR.\n"); return; }
    
    printf("Enter Car VIN: "); scanf("%d", &vin);
    long car_offset = btree_search(showrooms[sr_idx].a_cars_bt, vin);
    if (car_offset < 0) { printf("Car not available!\n"); return; }
    
    printf("Enter Salesperson ID: "); scanf("%d", &sp_id);
    long sp_offset = btree_search(showrooms[sr_idx].salespersons_bt, sp_id);
    if (sp_offset < 0) { printf("Salesperson not found!\n"); return; }
    
    printf("Enter new Customer ID: "); scanf("%d", &cust_id);
    Customer cust = {0};
    cust.customerid = cust_id;
    cust.car_VIN = vin;
    strcpy(cust.name, "New Cust");
    strcpy(cust.paymentType, "Cash");
    
    a_car available_car;
    db_read_record(showrooms[sr_idx].a_cars_db, car_offset, &available_car, sizeof(a_car));
    
    /* Convert available to sold */
    s_car sold_car = {0};
    sold_car.VIN = available_car.VIN;
    strcpy(sold_car.name, available_car.name);
    strcpy(sold_car.color, available_car.color);
    sold_car.price = available_car.price;
    strcpy(sold_car.fuelType, available_car.fuelType);
    strcpy(sold_car.carType, available_car.carType);
    strcpy(sold_car.paymenttype, "Cash");
    sold_car.solddate.day = 1; sold_car.solddate.month = 1; sold_car.solddate.year = 2024;
    
    /* Update Salesperson */
    Salesperson sp;
    db_read_record(showrooms[sr_idx].salespersons_db, sp_offset, &sp, sizeof(Salesperson));
    sp.salesAchieved += sold_car.price;
    sp.commission += 0.02 * sold_car.price;
    db_update_record(showrooms[sr_idx].salespersons_db, sp_offset, &sp, sizeof(Salesperson));
    
    /* Save Sold Car & Customer */
    long s_offset = db_append_record(showrooms[sr_idx].s_cars_db, &sold_car, sizeof(s_car));
    btree_insert(showrooms[sr_idx].s_cars_bt, sold_car.VIN, s_offset);
    
    long c_offset = db_append_record(showrooms[sr_idx].customers_db, &cust, sizeof(Customer));
    btree_insert(showrooms[sr_idx].customers_bt, cust.customerid, c_offset);
    
    /* Remove from available */
    btree_delete(showrooms[sr_idx].a_cars_bt, vin);
    
    printf("\nSUCCESS: Car %d sold! Commission added to SP %d.\n", vin, sp_id);
}

void search_sp_range(void)
{
    float min, max;
    printf("Enter Min Sales: "); scanf("%f", &min);
    printf("Enter Max Sales: "); scanf("%f", &max);
    
    for (int i = 0; i < NUM_SHOWROOMS; i++) {
        traverse_salespersons(showrooms[i].salespersons_bt, showrooms[i].salespersons_bt->root_offset, showrooms[i].salespersons_db, NULL, NULL, NULL, min, max);
    }
}

void run_showroom_menu(void)
{
    int choice;
    do {
        printf("\n============================================\n");
        printf("       Car Showroom Management CRM\n");
        printf("============================================\n");
        printf("1. Print Showroom Data\n");
        printf("2. Find Most Popular Car (Hash Table)\n");
        printf("3. Find Most Successful Salesperson\n");
        printf("4. Sale Car to Customer\n");
        printf("5. Search Salespersons by Sales Range\n");
        printf("6. Predict Next Month Sales\n");
        printf("7. Display Car Info by VIN (All Showrooms)\n");
        printf("8. List Customers with EMI Plan\n");
        printf("9. Merge All Showrooms and Print\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                for(int i=0; i<NUM_SHOWROOMS; i++) print_showroom_data(i);
                break;
            case 2: find_most_popular_car_all(); break;
            case 3: find_best_salesperson_all(); break;
            case 4: sell_car_interactive(); break;
            case 5: search_sp_range(); break;
            case 6: predict_sales(); break;
            case 7: {
                int v; printf("Enter VIN: "); scanf("%d", &v);
                display_car_info(v);
                break;
            }
            case 8: list_emi_all(); break;
            case 9:
                for(int i=0; i<NUM_SHOWROOMS; i++) print_showroom_data(i);
                break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}
