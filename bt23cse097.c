#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define SIZE 100

typedef enum { petrol, diesel, electric } fueltype;
typedef enum { sedan, hatchback, sportscar, jeep } cartype;

typedef struct struct_date {
    int date;
    int month;
    int year;
} date;

typedef struct car_tag {
    int carprice;
    int carid;
    char name[SIZE];
    char colour[SIZE];
    fueltype ft;
    cartype ct;
    struct customer_tag *c;
    struct car_tag *next;
} car;

typedef struct car_detail {
    int regno;
    int modelno;
    char colour[SIZE];
    int paid_amt;
    int emi;
    date servicing_next;
    date servicing_prev;
    date insurance_eval;
    struct car_detail *next;
} car_detail;

typedef struct customer_tag {
    char name[SIZE];
    car *carptr;
    car_detail *cardetails;
} customer;

typedef struct salesnode_tag {
    int commission;
    customer c;
    date sold;
    car cc;
    struct salesnode_tag *next;
} salesnode;

typedef struct salesperson {
    char name[SIZE];
    int id;
    int dob;
    char address[SIZE];
    int salestarget;
    int salesachieved;
    salesnode *s;
    struct salesperson *next;
} salesperson;

typedef struct showroom {
    int sold_cars;
    int available_cars;
    int req_cars;
    car *sold;
    car *available;
    struct showroom *next;
    salesperson *sp;
} showroom;


void freeShowroom(showroom *s);
showroom* readShowroomFromFile(const char *filename);
int findMostPopularCar(showroom *s1, showroom *s2, showroom *s3, car **c);
int totalLoanPending(salesperson *spList, char *salespersonName);
void Mostsuccessfulspforeachct(showroom *s1, showroom *s2, showroom *s3);
void sortCarsBySoldDate(salesperson *spList);
void setSalesTarget(salesperson *spList, char *salespersonName, int target);
void checkSalesTargetAchieved(salesperson *spList);
void generateServiceAlert(customer *c);
car *createCarNode(int carid, char *name, char *colour, fueltype ft, cartype ct, int carprice);
showroom *createShowroomNode();
car *mergeCarLists(car *list1, car *list2);
showroom *mergeShowrooms(showroom *s1, showroom *s2);
salesnode *mergeSales(salesnode *a, salesnode *b);
void splitSales(salesnode *source, salesnode **front, salesnode **back);
void mergeSortSales(salesnode **headRef);
car *createDefaultCar();
void addCarToShowroomFromInput(showroom *s);
void freeShowroom(showroom *s) {
    int cleanup = (s != NULL);
    if(cleanup) {
        car *currCar = s->available;
        while (currCar) {
            car *temp = currCar;
            currCar = currCar->next;
            free(temp);
        }

        currCar = s->sold;
        while (currCar) {
            car *tempCar = currCar;
            if (currCar->c) {
                if (currCar->c->cardetails) {
                    free(currCar->c->cardetails);
                }
                free(currCar->c);
            }
            currCar = currCar->next;
            free(tempCar);
        }

        salesperson *sp = s->sp;
        while (sp) {
            salesperson *tempSp = sp;
            salesnode *sn = sp->s;
            while (sn) {
                salesnode *tempSn = sn;
                sn = sn->next;
                free(tempSn);
            }
            sp = sp->next;
            free(tempSp);
        }
        free(s);
    }
}

car *createCarNode(int carid, char *name, char *colour, fueltype ft, cartype ct, int carprice) {
    car *result = NULL;
    car *newCar = (car *)malloc(sizeof(car));
    int success = (newCar != NULL);
    
    if(success) {
        newCar->carid = carid;
        strncpy(newCar->name, name, SIZE-1);
        newCar->name[SIZE-1] = '\0';
        strncpy(newCar->colour, colour, SIZE-1);
        newCar->colour[SIZE-1] = '\0';
        newCar->ft = ft;
        newCar->ct = ct;
        newCar->carprice = carprice;
        newCar->next = NULL;
        newCar->c = NULL;
        result = newCar;
    }
    return result;
}

car *createDefaultCar() {
    car *result = NULL;
    car *newCar = (car *)malloc(sizeof(car));
    int success = (newCar != NULL);

    if (success) {
        newCar->carid = 0;
        newCar->carprice = 0;
        newCar->ft = petrol;
        newCar->ct = sedan;

        strncpy(newCar->name, "Unknown", SIZE - 1);
        newCar->name[SIZE - 1] = '\0';

        strncpy(newCar->colour, "Unspecified", SIZE - 1);
        newCar->colour[SIZE - 1] = '\0';

        newCar->next = NULL;
        newCar->c = NULL;
        result = newCar;
    }
    return result;
}

showroom *createShowroomNode() {
    showroom *result = NULL;
    showroom *newShowroom = (showroom *)malloc(sizeof(showroom));
    int success = (newShowroom != NULL);

    if (success) {
        newShowroom->sold_cars = 0;
        newShowroom->available_cars = 0;
        newShowroom->req_cars = 0;
        newShowroom->sold = NULL;
        newShowroom->available = NULL;
        newShowroom->sp = NULL;
        newShowroom->next = NULL;
        result = newShowroom;
    }
    return result;
}

salesnode *copySalesData(salesnode *original) {
    salesnode *result = NULL;
    int proceed = (original != NULL);
    
    if(proceed) {
        salesnode *newHead = NULL, *tail = NULL;
        while (original) {
            salesnode *newNode = (salesnode *)malloc(sizeof(salesnode));
            int success = (newNode != NULL);
            
            if(!success) {
                while(newHead) {
                    salesnode *temp = newHead;
                    newHead = newHead->next;
                    free(temp);
                }
                break;
            }

            *newNode = *original;
            newNode->next = NULL;

            if (!newHead) {
                newHead = newNode;
                tail = newNode;
            } else {
                tail->next = newNode;
                tail = newNode;
            }
            original = original->next;
        }
        result = newHead;
    }
    return result;
}

void splitCarList(car *source, car **front, car **back) {
    car *slow = source, *fast = source->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    *front = source;
    *back = slow->next;
    slow->next = NULL;
}

car *sortedMerge(car *list1, car *list2) {
    car *result = NULL, **tail = &result;
    while (list1 && list2) {
        if (list1->carid <= list2->carid) {
            *tail = list1;
            list1 = list1->next;
        } else {
            *tail = list2;
            list2 = list2->next;
        }
        tail = &((*tail)->next);
    }
    *tail = list1 ? list1 : list2;
    return result;
}

void mergeSortCars(car **headRef) {
    car *head = *headRef, *a, *b;
    if (!head || !(head->next)) return;
    splitCarList(head, &a, &b);
    mergeSortCars(&a);
    mergeSortCars(&b);
    *headRef = sortedMerge(a, b);
}

showroom *mergeShowrooms(showroom *s1, showroom *s2) {
    showroom *merged = NULL;
    if (s1 && s2) {
        merged = createShowroomNode();
        if (merged) {
            merged->sold_cars = s1->sold_cars + s2->sold_cars;
            merged->available_cars = s1->available_cars + s2->available_cars;
            merged->req_cars = s1->req_cars + s2->req_cars;
            merged->sold = sortedMerge(s1->sold, s2->sold);
            merged->available = sortedMerge(s1->available, s2->available);
            mergeSortCars(&(merged->sold));
            mergeSortCars(&(merged->available));

            salesperson *sp1 = s1->sp, *sp2 = s2->sp, *tail = NULL;
            while (sp1) {
                salesperson *newSp = (salesperson *)malloc(sizeof(salesperson));
                *newSp = *sp1;
                newSp->s = copySalesData(newSp->s);
                newSp->next = merged->sp;
                merged->sp = newSp;
                sp1 = sp1->next;
            }
            
            while (sp2) {
                salesperson *newSp = (salesperson *)malloc(sizeof(salesperson));
                *newSp = *sp2;
                newSp->s = copySalesData(newSp->s);
                newSp->next = merged->sp;
                merged->sp = newSp;
                sp2 = sp2->next;
            }
            
        }
    }
    return merged ;
}

showroom *readShowroomFromFile(const char *filename) {
    showroom *s = NULL;
    FILE *file = NULL;
    int success = 1;
    int spCount = 0;
    int salesCount = 0;
    int i, j;

    file = fopen(filename, "r");
    success = (file != NULL);

    if (success) {
        s = (showroom *)malloc(sizeof(showroom));
        success = (s != NULL);
    }

    if (success) {
        success = (fscanf(file, "%d %d %d", &s->sold_cars, &s->available_cars, &s->req_cars) == 3);
    }

    if (success) {
        s->available = NULL;
        s->sold = NULL;
        s->sp = NULL;

      
        for (i = 0; success && i < s->available_cars; i++) {
            car *newCar = (car *)malloc(sizeof(car));
            success = (newCar != NULL);

            if (success) {
                success = (fscanf(file, "%d %99s %99s %d %d %d",
                    &newCar->carid, newCar->name, newCar->colour, 
                    (int *)&newCar->ft, (int *)&newCar->ct, &newCar->carprice) == 6);
            }

            if (success) {
                newCar->next = s->available;
                s->available = newCar;
            } else {
                free(newCar);
            }
        }

       
        for (i = 0; success && i < s->sold_cars; i++) {
            car *newCar = (car *)malloc(sizeof(car));
            customer *cust = NULL;
            int allocOK = 1;

            success = (newCar != NULL);
            if (success) {
                cust = (customer *)malloc(sizeof(customer));
                allocOK = (cust != NULL);
            }
            if (success && allocOK) {
                cust->cardetails = (car_detail *)calloc(1, sizeof(car_detail));
                allocOK = (cust->cardetails != NULL);
            }

            if (!allocOK) {
                free(newCar);
                free(cust);
                success = 0;
                continue;
            }

            if (success) {
                success = (fscanf(file, "%d %99s %99s %d %d %d %99s %d %d %d %d %d %d %d %d %d %d %d",
                    &newCar->carid, newCar->name, newCar->colour, (int *)&newCar->ft, (int *)&newCar->ct,
                    &newCar->carprice, cust->name, &cust->cardetails->paid_amt, &cust->cardetails->emi,
                    &cust->cardetails->servicing_prev.date, &cust->cardetails->servicing_prev.month,
                    &cust->cardetails->servicing_prev.year, &cust->cardetails->servicing_next.date,
                    &cust->cardetails->servicing_next.month, &cust->cardetails->servicing_next.year,
                    &cust->cardetails->insurance_eval.date, &cust->cardetails->insurance_eval.month,
                    &cust->cardetails->insurance_eval.year) == 18);
            }

            if (success) {
                newCar->c = cust;
                newCar->next = s->sold;
                s->sold = newCar;
            } else {
                free(newCar);
                if (cust) {
                    free(cust->cardetails);
                    free(cust);
                }
            }
        }

       
        if (success) {
            success = (fscanf(file, "%d", &spCount) == 1);
        }

        for (i = 0; success && i < spCount; i++) {
            salesperson *sp = (salesperson *)malloc(sizeof(salesperson));
            success = (sp != NULL);

            if (success) {
                success = (fscanf(file, "%99s %d %d \"%99[^\"]\" %d %d",
                    sp->name, &sp->id, &sp->dob, sp->address,
                    &sp->salestarget, &sp->salesachieved) == 6);
            }

            if (success) {
                sp->s = NULL;
                sp->next = s->sp;
                s->sp = sp;

                success = (fscanf(file, "%d", &salesCount) == 1);
                for (j = 0; success && j < salesCount; j++) {
                    salesnode *newSale = (salesnode *)malloc(sizeof(salesnode));
                    success = (newSale != NULL);

                 
                if (success) {
                success = (fscanf(file, "%d %d %d %d %99s %d",
                &newSale->sold.date, 
                &newSale->sold.month, 
                &newSale->sold.year,
                &newSale->commission,
                newSale->cc.name,
                (int *)&newSale->cc.ct) == 6); 
                }


                    if (success) {
                        newSale->next = sp->s;
                        sp->s = newSale;
                    } else {
                        free(newSale);
                    }
                }
            } else {
                free(sp);
            }
        }
    }

    if (file) fclose(file);
    if (!success && s) {
        freeShowroom(s);
        s = NULL;
    }
    return s;
}

salesnode *mergeSales(salesnode *a, salesnode *b) {
    salesnode dummy;
    salesnode *result = NULL;
    salesnode *tail = &dummy;
    dummy.next = NULL;

    while (a && b) {
        if ((a->sold.year < b->sold.year) ||
            (a->sold.year == b->sold.year && a->sold.month < b->sold.month) ||
            (a->sold.year == b->sold.year && a->sold.month == b->sold.month && 
             a->sold.date < b->sold.date)) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? a : b;
    result = dummy.next;
    return result;
}

void splitSales(salesnode *source, salesnode **front, salesnode **back) {
    salesnode *fast = source->next;
    salesnode *slow = source;
    *front = source;
    *back = NULL;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    if(slow) {
        *back = slow->next;
        slow->next = NULL;
    }
}

void mergeSortSales(salesnode **headRef) {
    salesnode *head = *headRef;
    int proceed = (head != NULL && head->next != NULL);

    if(proceed) {
        salesnode *a, *b;
        splitSales(head, &a, &b);
        mergeSortSales(&a);
        mergeSortSales(&b);
        *headRef = mergeSales(a, b);
    }
}
void sortCarsBySoldDate(salesperson *spList) {
    salesperson *sp = spList;
    while (sp) {
        mergeSortSales(&(sp->s));
        sp = sp->next;
    }
}

int findMostPopularCar(showroom *s1, showroom *s2, showroom *s3, car **c) {
    int result = 0;
    int carCount[1000] = {0};
    int maxCount = 0;
    *c = NULL;

    showroom *showrooms[] = {s1, s2, s3};
    for (int i = 0; i < 3; i++) {
        
        
        car *current = showrooms[i]->sold;
        while (current) {
            if (current->carid >= 0 && current->carid < 1000) {
                carCount[current->carid]++;
                if (carCount[current->carid] > maxCount) {
                    maxCount = carCount[current->carid];
                    *c = current;
                }
            }
            current = current->next;
        }
    }
    result = maxCount;
    return result;
}

int totalLoanPending(salesperson *spList, char *salespersonName) {
    int result = 0;
    int found = 0;
    salesperson *sp = spList;

    while (sp && !found) {
        if (strcmp(sp->name, salespersonName) == 0) {
            salesnode *sales = sp->s;
            while (sales) {
                car_detail *details = sales->c.cardetails;
                while (details) {
                    result += (sales->cc.carprice - details->paid_amt);
                    details = details->next;
                }
                sales = sales->next;
            }
            found = 1;
        }
        sp = sp->next;
    }
    return result;
}

void Mostsuccessfulspforeachct(showroom *s1, showroom *s2, showroom *s3) {
    showroom *sw[] = {s1, s2, s3};
    char *types[] = {"sedan", "hatchback", "sportscar", "jeep"};

    for (int i = 0; i < 3; i++) {
        salesperson *spp = sw[i]->sp;
        if (!spp) continue;

        int maxi[4] = {0, 0, 0, 0}; 
        salesperson *ans[4] = {NULL}; 

        while (spp) {
            int cnt[4] = {0}; 
            salesnode *sn = spp->s;

           
            while (sn) {
                switch (sn->cc.ct) {
                    case sedan: cnt[0]++; break;
                    case hatchback: cnt[1]++; break;
                    case sportscar: cnt[2]++; break;
                    case jeep: cnt[3]++; break;
                    default: break;
                }
                sn = sn->next;
            }

            
            for (int j = 0; j < 4; j++) {
                if (cnt[j] > maxi[j]) {
                    maxi[j] = cnt[j];
                    ans[j] = spp;
                }
            }
            spp = spp->next;
        }

        printf("\nFor showroom %d:\n", i + 1);
        for (int j = 0; j < 4; j++) {
            if (maxi[j] > 0) { 
                printf("Best salesperson for %-9s: %s (%d sales)\n",
                       types[j], ans[j] ? ans[j]->name : "none", maxi[j]);
            } else {
                printf("No sales for %-9s\n", types[j]);
            }
        }
    }
}



void setSalesTarget(salesperson *spList, char *salespersonName, int target) {
    int found = 0;
    salesperson *sp = spList;

    while (sp && !found) {
        if (strcmp(sp->name, salespersonName) == 0) {
            sp->salestarget = target;
            found = 1;
        }
        sp = sp->next;
    }
}

void checkSalesTargetAchieved(salesperson *spList) {
    salesperson *sp = spList;

    while (sp) {
        if (sp->salesachieved >= sp->salestarget) {
            printf("%s achieved target\n", sp->name);
        } else {
            printf("%s missed target\n", sp->name);
        }
        sp = sp->next;
    }
}





void printLine(int length) {
    for (int i = 0; i < length; i++)
        printf("-");
    printf("\n");
}


void displayShowroomDetails(showroom *s) {
    if (!s) {
        printf("No showroom data available.\n");
        return;
    }

    printf("\n=== SHOWROOM DETAILS ===\n");
    printf("Sold Cars: %d | Available Cars: %d | Required Cars: %d\n", 
           s->sold_cars, s->available_cars, s->req_cars);

    // Available Cars
    printf("\n--- AVAILABLE CARS ---\n");
    printLine(75);
    printf("| %-5s | %-20s | %-6s | %-4s | %-4s | %-10s |\n",
           "ID", "Name", "Color", "Fuel", "Type", "Price");
    printLine(75);

    for (car *c = s->available; c; c = c->next) {
        printf("| %-5d | %-20.20s | %-6s | %-4d | %-4d | %-10d |\n",
               c->carid, c->name, c->colour, c->ft, c->ct, c->carprice);
    }
    printLine(75);

    // Sold Cars
    printf("\n--- SOLD CARS ---\n");
    printLine(95);
    printf("| %-5s | %-20s | %-6s | %-10s | %-12s | %-7s | %-6s |\n",
           "ID", "Name", "Color", "Price", "Customer", "Paid", "EMI");
    printLine(95);

    for (car *c = s->sold; c; c = c->next) {
        printf("| %-5d | %-20.20s | %-6s | %-10d | %-12.12s | %-7d | %-6d |\n",
               c->carid, c->name, c->colour, c->carprice,
               (c->c ? c->c->name : "N/A"),
               (c->c && c->c->cardetails ? c->c->cardetails->paid_amt : 0),
               (c->c && c->c->cardetails ? c->c->cardetails->emi : 0));
    }
    printLine(95);
}


void generateServiceAlert(customer *c) {
    if (!c || !c->cardetails) return;
    date currentDate = {1, 2, 2024};
    car_detail *detail = c->cardetails;

    while (detail) {
        int monthsDiff = (currentDate.year - detail->servicing_prev.year) * 12 +
                        (currentDate.month - detail->servicing_prev.month);

        if (monthsDiff >= 6) {
            printf("Service alert for %s:\n", c->name);
            printf("Last service: %d/%d/%d\n", 
                   detail->servicing_prev.date,
                   detail->servicing_prev.month,
                   detail->servicing_prev.year);
            printf("Service due!\n");
        }
        detail = detail->next;
    }
}

int predictNextMonthSales(showroom *s1, showroom *s2, showroom *s3, cartype ct) {
    int carTypeSales = 0;
    date currentDate = {1, 2, 2024};
    showroom *showrooms[] = {s1, s2, s3};
    
    for (int i = 0; i < 3; i++) {
        if (!showrooms[i]) continue;
        salesperson *sp = showrooms[i]->sp;
        while (sp) {
            salesnode *sale = sp->s;
            while (sale) {
                if (sale->cc.ct == ct) {
                    int monthDiff = (currentDate.year - sale->sold.year) * 12 + 
                                  (currentDate.month - sale->sold.month);
                    if (monthDiff >= 0 && monthDiff <= 12) {
                        carTypeSales++;
                    }
                }
                sale = sale->next;
            }
            sp = sp->next;
        }
    }
    return carTypeSales/11 ;
}

int getModelSalesFigures(showroom *s1, showroom *s2, showroom *s3, 
                        cartype ct, date startDate, date endDate) {
    int salesCount = 0;
    showroom *showrooms[] = {s1, s2, s3};
    
    for (int i = 0; i < 3; i++) {
        if (!showrooms[i]) continue;
        salesperson *sp = showrooms[i]->sp;
        while (sp) {
            salesnode *sale = sp->s;
            while (sale) {
                if (sale->cc.ct == ct) {
                    int afterStart = (sale->sold.year > startDate.year) || 
                                   (sale->sold.year == startDate.year && 
                                    sale->sold.month > startDate.month) ||
                                   (sale->sold.year == startDate.year && 
                                    sale->sold.month == startDate.month && 
                                    sale->sold.date >= startDate.date);
                    
                    int beforeEnd = (sale->sold.year < endDate.year) || 
                                  (sale->sold.year == endDate.year && 
                                   sale->sold.month < endDate.month) ||
                                  (sale->sold.year == endDate.year && 
                                   sale->sold.month == endDate.month && 
                                   sale->sold.date <= endDate.date);
                    
                    if (afterStart && beforeEnd) {
                        salesCount++;
                    }
                }
                sale = sale->next;
            }
            sp = sp->next;
        }
    }
    return salesCount;
}

float calculateServiceBill(car_detail *carDetails, int laborHours, float laborRate, 
                         float *partsCosts, int numParts) {
    float totalBill = 0.0;
    date currentDate = {1, 2, 2024};
    
    totalBill += laborHours * laborRate;
    
    for (int i = 0; i < numParts; i++) {
        totalBill += partsCosts[i];
    }
    
    
    return totalBill;
}
void addCarToShowroomFromInput(showroom *s) {
    if (!s) return;

    int carid, carprice, ft, ct;
    char name[20], colour[20];

    printf("Enter Car ID: ");
    scanf("%d", &carid);
    
    printf("Enter Car Name: ");
    scanf(" %[^\n]", name);

    printf("Enter Car Colour: ");
    scanf(" %[^\n]", colour);

    printf("Enter Fuel Type (0: Petrol, 1: Diesel, 2: Electric): ");
    scanf("%d", &ft);

    printf("Enter Car Type (0: Sedan, 1: Hatchback, 2: Sportscar, 3: Jeep): ");
    scanf("%d", &ct);

    printf("Enter Car Price: ");
    scanf("%d", &carprice);

    car *newCar = createCarNode(carid, name, colour, (fueltype)ft, (cartype)ct, carprice);
    if (!newCar) {
        printf("Failed to create car node.\n");
        return;
    }

    newCar->next = s->available;
    s->available = newCar;
    s->available_cars++;
}
void displaySalespersons(showroom *s) {
    if (!s || !s->sp) {
        printf("No salespersons in this showroom.\n");
        return;
    }

    salesperson *sp = s->sp;
    printf("\n--- Salespersons in Showroom ---\n");

    while (sp) {
        printf("\nSalesperson: %s (ID: %d)\n", sp->name, sp->id);
        printf("DOB: %d | Address: %s\n", sp->dob, sp->address);
        printf("Sales Target: %d | Sales Achieved: %d\n", sp->salestarget, sp->salesachieved);
        
        if (!sp->s) {
            printf("No sales records available.\n");
        } else {
            printf("\nSales History:\n");
            salesnode *sales = sp->s;
            while (sales) {
                printf("  Sale Date: %02d/%02d/%04d\n", sales->sold.date, sales->sold.month, sales->sold.year);
                printf("  Commission Earned: %d\n", sales->commission);
                
                
               
                
                
                printf("  Car Sold: %s \n", sales->cc.name);
                printf("  Fuel Type: %s | Car Type: %s\n",
                       
                       (sales->cc.ft == petrol) ? "Petrol" : (sales->cc.ft == diesel) ? "Diesel" : "Electric",
                       (sales->cc.ct == sedan) ? "Sedan" : (sales->cc.ct == hatchback) ? "Hatchback" : 
                       (sales->cc.ct == sportscar) ? "Sportscar" : "Jeep");
                
                sales = sales->next;
            }
        }
        
        printf("\n-------------------------------------\n");
        sp = sp->next;
    }
}
void sellCar(showroom *shop, char *carName, char *buyerName, char *sellerName, 
    int paid, int emi, date nextService, date lastService, 
    date insurance, int bonus) {

        car *prev = NULL;
        car *curr = shop->available;
        car *sale = NULL;

        while (curr) {
        if (strcmp(curr->name, carName) == 0) {
        sale = curr;
        if (prev == NULL) {
            shop->available = curr->next;
        } else {
            prev->next = curr->next;
        }
        shop->available_cars--;
        break;
        }
        prev = curr;
        curr = curr->next;
        }

        if (!sale) {
        printf("Car not found!\n");
        return;
        }

        customer *buyer = (customer *)malloc(sizeof(customer));
        

        strncpy(buyer->name, buyerName, SIZE-1);
        buyer->name[SIZE-1] = '\0';

        car_detail *info = (car_detail *)malloc(sizeof(car_detail));
        

        info->paid_amt = paid;
        info->emi = emi;
        info->servicing_next = nextService;
        info->servicing_prev = lastService;
        info->insurance_eval = insurance;
        info->next = NULL;

        buyer->cardetails = info;
        sale->c = buyer;

        sale->next = shop->sold;
        shop->sold = sale;
        shop->sold_cars++;

        salesperson *seller = shop->sp;
        while (seller) {
        if (strcmp(seller->name, sellerName) == 0) {
        salesnode *record = (salesnode *)malloc(sizeof(salesnode));
       
        
        
        record->sold.date = 1;
        record->sold.month = 2;
        record->sold.year = 2025;
        
        record->commission = bonus;
        record->cc = *sale;
        
        record->next = seller->s;
        seller->s = record;
        
        seller->salesachieved++;
        break;
        }
        seller = seller->next;
        }

        if (!seller) {
        printf("Seller not found!\n");
        return;
        }

        printf("Car sold successfully!\n");
}
int main() {
    showroom *s1 = readShowroomFromFile("showroom1.txt");
    showroom *s2 = readShowroomFromFile("showroom2.txt");
    showroom *s3 = readShowroomFromFile("showroom3.txt");
    showroom *merged = NULL;  // Only merge when user selects it

    if (!s1 || !s2 || !s3) {
        printf("Error loading showrooms\n");
        if (s1) freeShowroom(s1);
        if (s2) freeShowroom(s2);
        if (s3) freeShowroom(s3);
        return 1;
    }

    int choice;
    char name[SIZE];
    int target;

    do {
        printf("\n=== Car Showroom Management System ===\n");
        printf("1. Display Showroom Details\n");
        printf("2. Find Most Popular Car\n");
        printf("3. Check Total Pending Loan\n");
        printf("4. View Best Salespeople\n");
        printf("5. Sort Sales by Date\n");
        printf("6. Set Sales Target\n");
        printf("7. Check Target Achievement\n");
        printf("8. Generate Service Alerts\n");
        printf("9. Predict Next Month Sales\n");
        printf("10. Get Model Sales Figures\n");
        printf("11. Calculate Service Bill\n");
        printf("12. Merge Showrooms\n");
        printf("13. Add Car to Showroom\n");
        printf("14. Display Salespersons\n");
        printf("15. Exit\n");
        printf("16. Sell Car\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: {
                int showroomChoice;
                printf("Choose Showroom (1, 2, or 3): ");
                scanf("%d", &showroomChoice);
                displayShowroomDetails((showroomChoice == 1) ? s1 : (showroomChoice == 2) ? s2 : s3);
                break;
            }
            case 2: {
                car *result = NULL;
                int count = findMostPopularCar(s1, s2, s3, &result);
                if (result) printf("Most popular: %s (%d sales)\n", result->name, count);
                break;
            }
            case 3: {
                printf("Enter salesperson name: ");
                scanf("%99s", name);
                int total = totalLoanPending(s1->sp, name);
                printf("Total pending: %d\n", total);
                break;
            }
            case 4: {
                Mostsuccessfulspforeachct(s1, s2, s3);
                break;
            }
            case 5: {
                sortCarsBySoldDate(s1->sp);
                printf("Sales data sorted by date\n");
                break;
            }
            case 6: {
                printf("Enter salesperson name and target: ");
                scanf("%99s %d", name, &target);
                setSalesTarget(s1->sp, name, target);
                printf("Target set successfully\n");
                break;
            }
            case 7: {
                checkSalesTargetAchieved(s1->sp);
                break;
            }
            case 8: {
                char custName[SIZE];
                printf("Enter customer name: ");
                scanf("%99s", custName);
                customer *c = (customer *)malloc(sizeof(customer));
                strncpy(c->name, custName, SIZE - 1);
                c->cardetails = (car_detail *)malloc(sizeof(car_detail));
                c->cardetails->servicing_prev.date = 1;
                c->cardetails->servicing_prev.month = 7;
                c->cardetails->servicing_prev.year = 2023;
                generateServiceAlert(c);
                free(c->cardetails);
                free(c);
                break;
            }
            case 9: {
                int carTypeChoice;
                printf("Enter car type (0-sedan, 1-hatchback, 2-sportscar, 3-jeep): ");
                scanf("%d", &carTypeChoice);
                cartype ct = (cartype)carTypeChoice;
                int prediction = predictNextMonthSales(s1, s2, s3, ct);
                printf("Predicted sales for car type %d next month: %d units\n", 
                       carTypeChoice, prediction);
                break;
            }
            case 10: {
                int carTypeChoice;
                date startDate, endDate;
                printf("Enter car type (0-sedan, 1-hatchback, 2-sportscar, 3-jeep): ");
                scanf("%d", &carTypeChoice);
                cartype ct = (cartype)carTypeChoice;
                printf("Enter start date (dd mm yyyy): ");
                scanf("%d %d %d", &startDate.date, &startDate.month, &startDate.year);
                printf("Enter end date (dd mm yyyy): ");
                scanf("%d %d %d", &endDate.date, &endDate.month, &endDate.year);
                int sales = getModelSalesFigures(s1, s2, s3, ct, startDate, endDate);
                printf("Sales figures for car type %d between %d/%d/%d and %d/%d/%d: %d units\n",
                       carTypeChoice, startDate.date, startDate.month, startDate.year,
                       endDate.date, endDate.month, endDate.year, sales);
                break;
            }
            case 11: {
                int laborHours;
                float laborRate;
                int numParts;
                printf("Enter labor hours and rate: ");
                scanf("%d %f", &laborHours, &laborRate);
                printf("Enter number of parts: ");
                scanf("%d", &numParts);
                float *partsCosts = (float *)malloc(numParts * sizeof(float));
                printf("Enter cost for each part:\n");
                for (int i = 0; i < numParts; i++) {
                    scanf("%f", &partsCosts[i]);
                }
                car_detail *sampleDetail = (car_detail *)malloc(sizeof(car_detail));
                sampleDetail->servicing_prev.year = 2020;
                float bill = calculateServiceBill(sampleDetail, laborHours, laborRate, 
                                                partsCosts, numParts);
                printf("Total service bill: %.2f\n", bill);
                free(partsCosts);
                free(sampleDetail);
                break;
            }
            case 12: { 
                if (!merged) {
                    showroom *temp_merged = mergeShowrooms(s1, s2);
                    merged = mergeShowrooms(temp_merged, s3);
                    freeShowroom(temp_merged);
                    printf("Showrooms merged successfully!\n");
                } else {
                    printf("Showrooms already merged!\n");
                }
                break;
            }
            case 13: {  
                int showroomChoice;
                printf("Choose Showroom (1, 2, or 3): ");
                scanf("%d", &showroomChoice);
                addCarToShowroomFromInput((showroomChoice == 1) ? s1 : (showroomChoice == 2) ? s2 : s3);
                break;
            }
            case 14: {  
                int showroomChoice;
                printf("Choose Showroom (1, 2, or 3): ");
                scanf("%d", &showroomChoice);
                displaySalespersons((showroomChoice == 1) ? s1 : (showroomChoice == 2) ? s2 : s3);
                break;
            }
            case 15:{

                printf("Exiting program...\n");
                break;
            }
                case 16: {
                    char car[SIZE], buyer[SIZE], seller[SIZE];
                    int paid, emi, bonus;
                    date next, last, insure;
                    
                    printf("Car name: ");
                    scanf(" %[^\n]", car);
                    
                    printf("Buyer name: ");
                    scanf(" %[^\n]", buyer);
                    
                    printf("Seller name: ");
                    scanf(" %[^\n]", seller);
                    
                    printf("Amount paid: ");
                    scanf("%d", &paid);
                    
                    printf("EMI amount: ");
                    scanf("%d", &emi);
                    
                    printf("Next service (dd mm yyyy): ");
                    scanf("%d %d %d", &next.date, &next.month, &next.year);
                    
                    printf("Last service (dd mm yyyy): ");
                    scanf("%d %d %d", &last.date, &last.month, &last.year);
                    
                    printf("Insurance date (dd mm yyyy): ");
                    scanf("%d %d %d", &insure.date, &insure.month, &insure.year);
                    
                    printf("Seller bonus: ");
                    scanf("%d", &bonus);
                    
                    int shop;
                    printf("Which showroom (1, 2, or 3): ");
                    scanf("%d", &shop);
                    
                    sellCar((shop == 1) ? s1 : (shop == 2) ? s2 : s3,
                            car, buyer, seller, paid, emi,
                            next, last, insure, bonus);
                    break;
                }

            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 15);
    
    if(merged)
    {
        displayShowroomDetails(merged);
        displaySalespersons(merged);
    }
    freeShowroom(s1);
    freeShowroom(s2);
    freeShowroom(s3);
    if (merged) freeShowroom(merged);

    return 0;
}


               