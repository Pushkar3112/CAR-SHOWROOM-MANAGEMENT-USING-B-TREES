/*
 * showroom_db.c
 *
 * Implements flat binary file appending and reading for the CRM data layer.
 */

#include "showroom_db.h"
#include <stdio.h>
#include <stdlib.h>

long db_append_record(FILE *fp, const void *record, size_t record_size)
{
    if (!fp || !record) return -1;

    /* Seek to the end of the file */
    if (fseek(fp, 0, SEEK_END) != 0) return -1;

    long offset = ftell(fp);
    if (offset < 0) return -1;

    if (fwrite(record, record_size, 1, fp) != 1) return -1;
    fflush(fp);  /* Ensure data reaches disk immediately */

    return offset;
}

int db_read_record(FILE *fp, long offset, void *record, size_t record_size)
{
    if (!fp || !record || offset < 0) return 0;

    if (fseek(fp, offset, SEEK_SET) != 0) return 0;

    if (fread(record, record_size, 1, fp) != 1) return 0;

    return 1;
}

int db_update_record(FILE *fp, long offset, const void *record, size_t record_size)
{
    if (!fp || !record || offset < 0) return 0;

    if (fseek(fp, offset, SEEK_SET) != 0) return 0;

    if (fwrite(record, record_size, 1, fp) != 1) return 0;
    fflush(fp);

    return 1;
}
