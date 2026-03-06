/*
 * showroom_db.h
 *
 * Defines the core data structures for the Car Showroom CRM.
 * Manages appending and reading flat fixed-size records from binary heap files
 * (so the B-Tree only needs to store the file offsets).
 */

#ifndef SHOWROOM_DB_H
#define SHOWROOM_DB_H

#include <stdio.h>

/* --- Core Data Structures (from original project) --- */

typedef struct {
    int day;
    int month;
    int year;
} date;

typedef struct {
    int VIN;                /* Primary Key */
    char name[50];
    char color[20];
    float price;
    char fuelType[20];
    char carType[20];
} a_car;

typedef struct {
    int VIN;                /* Primary Key */
    char name[50];
    char color[20];
    float price;
    char fuelType[20];
    char carType[20];
    char paymenttype[10];
    date solddate;
} s_car;

typedef struct {
    int customerid;         /* Primary Key */
    char name[50];
    char mobileNo[15];
    char address[100];
    char registrationNo[20];
    char paymentType[10];
    int car_VIN;            /* Foreign Key to s_car (replaces pointer) */
} Customer;

typedef struct {
    int id;                 /* Primary Key */
    char name[50];
    float salesTarget;
    float salesAchieved;
    float commission;
} Salesperson;

/* --- CRM Database I/O Layer --- */

/* Appends the struct memory buffer to the end of the file. Returns the offset. */
long db_append_record(FILE *fp, const void *record, size_t record_size);

/* Reads a record of `record_size` from `offset`. Returns 1 on success, 0 on failure. */
int db_read_record(FILE *fp, long offset, void *record, size_t record_size);

/* Overwrites a record at `offset` (e.g. to update commission/sales achieving). */
int db_update_record(FILE *fp, long offset, const void *record, size_t record_size);

#endif /* SHOWROOM_DB_H */
