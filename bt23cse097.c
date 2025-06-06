#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ORDER 5
#define MAX 4
#define MIN 2

typedef struct {
    int day;
    int month;
    int year;
} date;

typedef struct {
    int VIN;
    char name[50];
    char color[20];
    float price;
    char fuelType[20];
    char carType[20];
} a_car;

typedef struct a_cartag {
    int isleaf;
    int n;
    a_car cars[MAX+1];
    struct a_cartag* child[MAX+2];
    struct a_cartag* parent;
} a_car_tree;

typedef struct {
    int VIN;
    char name[50];
    char color[20];
    float price;
    char fuelType[20];
    char carType[20];
    char paymenttype[10];
    date solddate;
} s_car;

typedef struct stag {
    int isleaf;
    int n;
    s_car cars[MAX+1];
    struct stag* child[MAX+2];
    struct stag* parent;
} s_car_tree;

typedef struct {
    char name[50];
    char mobileNo[15];
    char address[100];
    int customerid;
    char registrationNo[20];
    char paymentType[10];
    s_car* car;
} Customer;

typedef struct ctag {
    int isleaf;
    int n;
    Customer customers[MAX+1];
    struct ctag* child[MAX+2];
    struct ctag* parent;
} customer_tree;

typedef struct {
    int id;
    char name[50];
    float salesTarget;
    float salesAchieved;
    customer_tree* customerList;
    float commission;
} Salesperson;

typedef struct sptag {
    int isleaf;
    int n;
    Salesperson sales[MAX+1];
    struct sptag* child[MAX+2];
    struct sptag* parent;
} sales_tree;

typedef struct {
    s_car_tree* car_tree;
    a_car_tree* car_tree1;
    sales_tree* sales_tree;
    customer_tree* customer_tree;
    int a_car_cnt;
    int s_car_cnt;
    int sales_cnt;
} showroom;


a_car_tree* createNodeacar(int isleaf);
s_car_tree* createNodescar(int isleaf);
customer_tree* createNodeCustomer(int isleaf);
sales_tree* createNodeSales(int isleaf);

void insertacar(a_car car, a_car_tree **root_ptr);
void insertscar(s_car car, s_car_tree **root_ptr);
s_car_tree* mergeSCarTrees(s_car_tree *tree1, s_car_tree *tree2);
a_car_tree* mergeACarTrees(a_car_tree *tree1, a_car_tree *tree2);
void updateCommission(Salesperson* salesperson, float carPrice);
int countSalespersons(sales_tree* root);
void insertCustomer(Customer customer, customer_tree **root_ptr);
void freetreeacar(a_car_tree *node);
void freetreescar(s_car_tree *node);


void freeTreeSales(sales_tree *node);
void freeTreeCustomer(customer_tree *node);

void loadDataFromFile(const char* filename, a_car_tree** a_car_root, s_car_tree** s_car_root, sales_tree** sales_root, customer_tree** customer_root);

void findMostPopularCar(s_car_tree* root);
void findMostSuccessfulSalesperson(sales_tree* root);
void saleCarToCustomer(int VIN, Customer customer, char* paymentType, date soldDate,
                      a_car_tree** a_car_root, s_car_tree** s_car_root,
                      customer_tree** customer_root, Salesperson* salesperson);
void searchSalespersonsBySalesRange(float min_sales, float max_sales, sales_tree* root);
float predictNextMonthSales(sales_tree* root);
void displayCarInfoByVIN(int VIN, a_car_tree* a_car_root, s_car_tree* s_car_root);
void listCustomersWithEMIPlan(customer_tree* root);
Salesperson* findSalespersonById(int id, sales_tree* root);
void freeShowroom(showroom* showroom);
void printShowroomData(showroom* showroom);



a_car_tree* createNodeacar(int isleaf) {
    a_car_tree *newNode = (a_car_tree *)malloc(sizeof(a_car_tree));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->n = 0;
    newNode->isleaf = isleaf;
    newNode->parent = NULL;
    for (int i = 0; i <= MAX; i++) {
        newNode->child[i] = NULL;
    }
    return newNode;
}

int findPositionacar(int VIN, a_car_tree *node) {
    int pos = 0;
    while (pos < node->n && VIN > node->cars[pos].VIN) {
        pos++;
    }
    return pos;
}

void findacar(int VIN, a_car_tree *root, a_car_tree **node, int *pos) {
    a_car_tree *current = root;
    *node = NULL;
    *pos = -1;

    while (current) {
        int i = 0;
        while (i < current->n && VIN > current->cars[i].VIN) {
            i++;
        }

        if (i < current->n && VIN == current->cars[i].VIN) {
            *node = current;
            *pos = i;
            return;
        }

        if (current->isleaf) {
            return;
        }

        current = current->child[i];
    }
}

void rebalanceNodeacar(a_car_tree *node);

void borrowFromRightacar(a_car_tree *node, int index) {
    a_car_tree *parent = node->parent;
    a_car_tree *rightSibling = parent->child[index + 1];

   

    node->cars[node->n] = parent->cars[index];
    node->n++;

    parent->cars[index] = rightSibling->cars[0];

    if (!node->isleaf) {
        node->child[node->n] = rightSibling->child[0];
        if (node->child[node->n]) {
            node->child[node->n]->parent = node;
        }

        for (int i = 0; i < rightSibling->n; i++) {
            rightSibling->child[i] = rightSibling->child[i + 1];
        }
    }

    for (int i = 0; i < rightSibling->n - 1; i++) {
        rightSibling->cars[i] = rightSibling->cars[i + 1];
    }
    rightSibling->n--;
}

void borrowFromLeftacar(a_car_tree *node, int index) {
    a_car_tree *parent = node->parent;
    a_car_tree *leftSibling = parent->child[index - 1];

    for (int i = node->n; i > 0; i--) {
        node->cars[i] = node->cars[i - 1];
    }

    if (!node->isleaf) {
        for (int i = node->n + 1; i > 0; i--) {
            node->child[i] = node->child[i - 1];
        }

        node->child[0] = leftSibling->child[leftSibling->n];
        if (node->child[0]) {
            node->child[0]->parent = node;
        }
    }

    node->cars[0] = parent->cars[index - 1];
    node->n++;

    parent->cars[index - 1] = leftSibling->cars[leftSibling->n - 1];
    leftSibling->n--;
}

void mergeNodesacar(a_car_tree *leftNode, a_car_tree *rightNode, int parentKeyIndex) {
    a_car_tree *parent = leftNode->parent;

    leftNode->cars[leftNode->n] = parent->cars[parentKeyIndex];
    leftNode->n++;

    for (int i = 0, j = leftNode->n; i < rightNode->n; i++, j++) {
        leftNode->cars[j] = rightNode->cars[i];
        leftNode->n++;
    }

    if (!leftNode->isleaf) {
        for (int i = 0, j = leftNode->n - rightNode->n; i <= rightNode->n; i++, j++) {
            leftNode->child[j] = rightNode->child[i];
            if (leftNode->child[j]) {
                leftNode->child[j]->parent = leftNode;
            }
        }
    }

    for (int i = parentKeyIndex; i < parent->n - 1; i++) {
        parent->cars[i] = parent->cars[i + 1];
        parent->child[i + 1] = parent->child[i + 2];
    }
    parent->n--;

    free(rightNode);
}

void rebalanceNodeacar(a_car_tree *node) {
    a_car_tree *parent = node->parent;

    int nodeIndex = 0;
    while (nodeIndex <= parent->n && parent->child[nodeIndex] != node) {
        nodeIndex++;
    }

    if (nodeIndex < parent->n) {
        a_car_tree *rightSibling = parent->child[nodeIndex + 1];
        if (rightSibling->n > MIN) {
            borrowFromRightacar(node, nodeIndex);
            return;
        }
    }

    if (nodeIndex > 0) {
        a_car_tree *leftSibling = parent->child[nodeIndex - 1];
        if (leftSibling->n > MIN) {
            borrowFromLeftacar(node, nodeIndex);
            return;
        }
    }

    if (nodeIndex < parent->n) {
        mergeNodesacar(node, parent->child[nodeIndex + 1], nodeIndex);
    } else {
        mergeNodesacar(parent->child[nodeIndex - 1], node, nodeIndex - 1);
    }
} 

void splitNodeacar(a_car_tree *node, a_car_tree **root_ptr) {
    int mid = node->n / 2;

    a_car_tree *rightNode = createNodeacar(node->isleaf);

    for (int i = mid + 1, j = 0; i < node->n; i++, j++) {
        rightNode->cars[j] = node->cars[i];
        if (!node->isleaf) {
            rightNode->child[j] = node->child[i];
            if (rightNode->child[j]) {
                rightNode->child[j]->parent = rightNode;
            }
        }
    }

    if (!node->isleaf) {
        rightNode->child[node->n - mid - 1] = node->child[node->n];
        if (rightNode->child[node->n - mid - 1]) {
            rightNode->child[node->n - mid - 1]->parent = rightNode;
        }
    }

    rightNode->n = node->n - mid - 1;

    node->n = mid;

    a_car midCar = node->cars[mid];

    if (!node->parent) {
        a_car_tree *newRoot = createNodeacar(0);
        newRoot->cars[0] = midCar;
        newRoot->child[0] = node;
        newRoot->child[1] = rightNode;
        newRoot->n = 1;

        node->parent = newRoot;
        rightNode->parent = newRoot;

        *root_ptr = newRoot;
    } else {
        a_car_tree *parent = node->parent;

        int pos = findPositionacar(midCar.VIN, parent);

        for (int i = parent->n; i > pos; i--) {
            parent->cars[i] = parent->cars[i - 1];
            parent->child[i + 1] = parent->child[i];
        }

        parent->cars[pos] = midCar;
        parent->child[pos + 1] = rightNode;
        parent->n++;

        rightNode->parent = parent;

        if (parent->n > MAX) {
            splitNodeacar(parent, root_ptr);
        }
    }
}

void deleteFromLeafacar(a_car_tree *node, int pos, a_car_tree **root_ptr) {
    for (int i = pos; i < node->n - 1; i++) {
        node->cars[i] = node->cars[i + 1];
    }
    node->n--;

    if (node == *root_ptr && node->n == 0) {
        free(*root_ptr);
        *root_ptr = NULL;
        return;
    }

    if (node->n < MIN && node != *root_ptr) {
        rebalanceNodeacar(node);
    }
}

void findInOrderSuccessoracar(a_car_tree *node, int pos, a_car_tree **successorNode, int *successorPos) {
    a_car_tree *current = node->child[pos + 1];

    while (!current->isleaf) {
        current = current->child[0];
    }

    *successorNode = current;
    *successorPos = 0;
}

void insertacar(a_car car, a_car_tree **root_ptr) {
    if (!(*root_ptr)) {
        *root_ptr = createNodeacar(1);
        (*root_ptr)->cars[0] = car;
        (*root_ptr)->n = 1;
        return;
    }

    a_car_tree *current = *root_ptr;
    while (!current->isleaf) {
        int pos = findPositionacar(car.VIN, current);
        current = current->child[pos];
    }

    int pos = findPositionacar(car.VIN, current);

    if (pos < current->n && current->cars[pos].VIN == car.VIN) {
        printf("Car with VIN %d already exists!\n", car.VIN);
        return;
    }

    for (int i = current->n; i > pos; i--) {
        current->cars[i] = current->cars[i - 1];
    }

    current->cars[pos] = car;
    current->n++;

    if (current->n > MAX) {
        splitNodeacar(current, root_ptr);
    }
}

void deleteacar(int VIN, a_car_tree **root_ptr) {
    if (!(*root_ptr)) {
        printf("Tree is empty!\n");
        return;
    }

    a_car_tree *node;
    int pos;
    findacar(VIN, *root_ptr, &node, &pos);

    if (!node) {
        printf("Car with VIN %d not found!\n", VIN);
        return;
    }

    if (node->isleaf) {
        deleteFromLeafacar(node, pos, root_ptr);
        return;
    }

    a_car_tree *successorNode;
    int successorPos;

    findInOrderSuccessoracar(node, pos, &successorNode, &successorPos);

    node->cars[pos] = successorNode->cars[successorPos];

    deleteFromLeafacar(successorNode, successorPos, root_ptr);
}



void freetreeacar(a_car_tree *node) {
    if (!node) return;

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            freetreeacar(node->child[i]);
        }
    }

    free(node);
}

s_car_tree* createNodescar(int isleaf) {
    s_car_tree *newNode = (s_car_tree *)malloc(sizeof(s_car_tree));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->n = 0;
    newNode->isleaf = isleaf;
    newNode->parent = NULL;
    for (int i = 0; i <= MAX; i++) {
        newNode->child[i] = NULL;
    }
    return newNode;
}

int findPositioncar(int VIN, s_car_tree *node) {
    int pos = 0;
    while (pos < node->n && VIN > node->cars[pos].VIN) {
        pos++;
    }
    return pos;
}

void splitNodescar(s_car_tree *node, s_car_tree **root_ptr) {
    int mid = node->n / 2;

    s_car_tree *rightNode = createNodescar(node->isleaf);

    for (int i = mid + 1, j = 0; i < node->n; i++, j++) {
        rightNode->cars[j] = node->cars[i];
        if (!node->isleaf) {
            rightNode->child[j] = node->child[i];
            if (rightNode->child[j]) {
                rightNode->child[j]->parent = rightNode;
            }
        }
    }

    if (!node->isleaf) {
        rightNode->child[node->n - mid - 1] = node->child[node->n];
        if (rightNode->child[node->n - mid - 1]) {
            rightNode->child[node->n - mid - 1]->parent = rightNode;
        }
    }

    rightNode->n = node->n - mid - 1;

    node->n = mid;

    s_car midCar = node->cars[mid];

    if (!node->parent) {
        s_car_tree *newRoot = createNodescar(0);
        newRoot->cars[0] = midCar;
        newRoot->child[0] = node;
        newRoot->child[1] = rightNode;
        newRoot->n = 1;

        node->parent = newRoot;
        rightNode->parent = newRoot;

        *root_ptr = newRoot;
    } else {
        s_car_tree *parent = node->parent;

        int pos = findPositioncar(midCar.VIN, parent);

        for (int i = parent->n; i > pos; i--) {
            parent->cars[i] = parent->cars[i - 1];
            parent->child[i + 1] = parent->child[i];
        }

        parent->cars[pos] = midCar;
        parent->child[pos + 1] = rightNode;
        parent->n++;

        rightNode->parent = parent;

        if (parent->n > MAX) {
            splitNodescar(parent, root_ptr);
        }
    }
}

void insertscar(s_car car, s_car_tree **root_ptr) {
    if (!(*root_ptr)) {
        *root_ptr = createNodescar(1);
        (*root_ptr)->cars[0] = car;
        (*root_ptr)->n = 1;
        return;
    }

    s_car_tree *current = *root_ptr;
    while (!current->isleaf) {
        int pos = findPositioncar(car.VIN, current);
        current = current->child[pos];
    }

    int pos = findPositioncar(car.VIN, current);

    if (pos < current->n && current->cars[pos].VIN == car.VIN) {
        printf("Car with VIN %d already exists!\n", car.VIN);
        return;
    }

    for (int i = current->n; i > pos; i--) {
        current->cars[i] = current->cars[i - 1];
    }

    current->cars[pos] = car;
    current->n++;

    if (current->n > MAX) {
        splitNodescar(current, root_ptr);
    }
}



void freetreescar(s_car_tree *node) {
    if (!node) return;

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            freetreescar(node->child[i]);
        }
    }

    free(node);
}

sales_tree* createNodeSales(int isleaf) {
    sales_tree *newNode = (sales_tree *)malloc(sizeof(sales_tree));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->n = 0;
    newNode->isleaf = isleaf;
    newNode->parent = NULL;
    for (int i = 0; i <= MAX; i++) {
        newNode->child[i] = NULL;
    }
    return newNode;
}

int findPositionSales(int id, sales_tree *node) {
    int pos = 0;
    while (pos < node->n && id > node->sales[pos].id) {
        pos++;
    }
    return pos;
}

void splitNodeSales(sales_tree *node, sales_tree **root_ptr) {
    int mid = node->n / 2;

    sales_tree *rightNode = createNodeSales(node->isleaf);

    for (int i = mid + 1, j = 0; i < node->n; i++, j++) {
        rightNode->sales[j] = node->sales[i];
        if (!node->isleaf) {
            rightNode->child[j] = node->child[i];
            if (rightNode->child[j]) {
                rightNode->child[j]->parent = rightNode;
            }
        }
    }

    if (!node->isleaf) {
        rightNode->child[node->n - mid - 1] = node->child[node->n];
        if (rightNode->child[node->n - mid - 1]) {
            rightNode->child[node->n - mid - 1]->parent = rightNode;
        }
    }

    rightNode->n = node->n - mid - 1;

    node->n = mid;

    Salesperson midSalesperson = node->sales[mid];

    if (!node->parent) {
        sales_tree *newRoot = createNodeSales(0);
        newRoot->sales[0] = midSalesperson;
        newRoot->child[0] = node;
        newRoot->child[1] = rightNode;
        newRoot->n = 1;

        node->parent = newRoot;
        rightNode->parent = newRoot;

        *root_ptr = newRoot;
    } else {
        sales_tree *parent = node->parent;

        int pos = findPositionSales(midSalesperson.id, parent);

        for (int i = parent->n; i > pos; i--) {
            parent->sales[i] = parent->sales[i - 1];
            parent->child[i + 1] = parent->child[i];
        }

        parent->sales[pos] = midSalesperson;
        parent->child[pos + 1] = rightNode;
        parent->n++;

        rightNode->parent = parent;

        if (parent->n > MAX) {
            splitNodeSales(parent, root_ptr);
        }
    }
}

void insertSales(Salesperson salesperson, sales_tree **root_ptr) {
    if (!(*root_ptr)) {
        *root_ptr = createNodeSales(1);
        (*root_ptr)->sales[0] = salesperson;
        (*root_ptr)->n = 1;
        return;
    }

    sales_tree *current = *root_ptr;
    while (!current->isleaf) {
        int pos = findPositionSales(salesperson.id, current);
        current = current->child[pos];
    }

    int pos = findPositionSales(salesperson.id, current);

    if (pos < current->n && current->sales[pos].id == salesperson.id) {
        printf("Salesperson with ID %d already exists!\n", salesperson.id);
        return;
    }

    for (int i = current->n; i > pos; i--) {
        current->sales[i] = current->sales[i - 1];
    }

    current->sales[pos] = salesperson;
    current->n++;

    if (current->n > MAX) {
        splitNodeSales(current, root_ptr);
    }
}



void freeTreeSales(sales_tree *node) {
    if (!node) return;

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            freeTreeSales(node->child[i]);
        }
    }

    free(node);
}

customer_tree* createNodeCustomer(int isleaf) {
    customer_tree *newNode = (customer_tree *)malloc(sizeof(customer_tree));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->n = 0;
    newNode->isleaf = isleaf;
    newNode->parent = NULL;
    for (int i = 0; i <= MAX; i++) {
        newNode->child[i] = NULL;
    }
    return newNode;
}

int findPositionCustomer(int customerid, customer_tree *node) {
    int pos = 0;
    while (pos < node->n && customerid > node->customers[pos].customerid) {
        pos++;
    }
    return pos;
}

void splitNodeCustomer(customer_tree *node, customer_tree **root_ptr) {
    int mid = node->n / 2;

    customer_tree *rightNode = createNodeCustomer(node->isleaf);

    for (int i = mid + 1, j = 0; i < node->n; i++, j++) {
        rightNode->customers[j] = node->customers[i];
        if (!node->isleaf) {
            rightNode->child[j] = node->child[i];
            if (rightNode->child[j]) {
                rightNode->child[j]->parent = rightNode;
            }
        }
    }

    if (!node->isleaf) {
        rightNode->child[node->n - mid - 1] = node->child[node->n];
        if (rightNode->child[node->n - mid - 1]) {
            rightNode->child[node->n - mid - 1]->parent = rightNode;
        }
    }

    rightNode->n = node->n - mid - 1;

    node->n = mid;

    Customer midCustomer = node->customers[mid];

    if (!node->parent) {
        customer_tree *newRoot = createNodeCustomer(0);
        newRoot->customers[0] = midCustomer;
        newRoot->child[0] = node;
        newRoot->child[1] = rightNode;
        newRoot->n = 1;

        node->parent = newRoot;
        rightNode->parent = newRoot;

        *root_ptr = newRoot;
    } else {
        customer_tree *parent = node->parent;

        int pos = findPositionCustomer(midCustomer.customerid, parent);

        for (int i = parent->n; i > pos; i--) {
            parent->customers[i] = parent->customers[i - 1];
            parent->child[i + 1] = parent->child[i];
        }

        parent->customers[pos] = midCustomer;
        parent->child[pos + 1] = rightNode;
        parent->n++;

        rightNode->parent = parent;

        if (parent->n > MAX) {
            splitNodeCustomer(parent, root_ptr);
        }
    }
}

void insertCustomer(Customer customer, customer_tree **root_ptr) {
    if (!(*root_ptr)) {
        *root_ptr = createNodeCustomer(1);
        (*root_ptr)->customers[0] = customer;
        (*root_ptr)->n = 1;
        return;
    }

    customer_tree *current = *root_ptr;
    while (!current->isleaf) {
        int pos = findPositionCustomer(customer.customerid, current);
        current = current->child[pos];
    }

    int pos = findPositionCustomer(customer.customerid, current);

    if (pos < current->n && current->customers[pos].customerid == customer.customerid) {
        printf("Customer with ID %d already exists!\n", customer.customerid);
        return;
    }

    for (int i = current->n; i > pos; i--) {
        current->customers[i] = current->customers[i - 1];
    }

    current->customers[pos] = customer;
    current->n++;

    if (current->n > MAX) {
        splitNodeCustomer(current, root_ptr);
    }
}



void freeTreeCustomer(customer_tree *node) {
    if (!node) return;

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            freeTreeCustomer(node->child[i]);
        }
    }

    free(node);
}

s_car* copyCar(s_car* original) {
    if (!original) return NULL;
    s_car* copy = (s_car*)malloc(sizeof(s_car));
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    *copy = *original;
    return copy;
}

void traverseAndInsert(customer_tree *node, customer_tree **newTree) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        Customer customer = node->customers[i];
        customer.car = copyCar(customer.car);
        insertCustomer(customer, newTree);
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            traverseAndInsert(node->child[i], newTree);
        }
    }
}

customer_tree* mergeCustomerTrees(customer_tree *tree1, customer_tree *tree2) {
    customer_tree *newTree = NULL;

    traverseAndInsert(tree1, &newTree);
    traverseAndInsert(tree2, &newTree);

    return newTree;
}

customer_tree* copyCustomerTree(customer_tree *original) {
    if (!original) return NULL;
    customer_tree *copy = createNodeCustomer(original->isleaf);
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    copy->n = original->n;
    for (int i = 0; i < original->n; i++) {
        copy->customers[i] = original->customers[i];
        copy->customers[i].car = copyCar(original->customers[i].car); 
    }
    if (!original->isleaf) {
        for (int i = 0; i <= original->n; i++) {
            copy->child[i] = copyCustomerTree(original->child[i]);
            if (copy->child[i]) {
                copy->child[i]->parent = copy;
            }
        }
    }
    return copy;
}

void traverseAndInsertSales(sales_tree *node, sales_tree **newTree) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        Salesperson salesperson = node->sales[i];
        salesperson.customerList = copyCustomerTree(salesperson.customerList); 
        insertSales(salesperson, newTree);
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            traverseAndInsertSales(node->child[i], newTree);
        }
    }
}

sales_tree* mergeSalesTrees(sales_tree *tree1, sales_tree *tree2) {
    sales_tree *newTree = NULL;

    
    traverseAndInsertSales(tree1, &newTree);

   
    traverseAndInsertSales(tree2, &newTree);

    return newTree;
}

a_car* copyACar(a_car* original) {
    if (!original) return NULL;
    a_car* copy = (a_car*)malloc(sizeof(a_car));
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    *copy = *original;
    return copy;
}

s_car* copySCar(s_car* original) {
    if (!original) return NULL;
    s_car* copy = (s_car*)malloc(sizeof(s_car));
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    *copy = *original;
    return copy;
}

void traverseAndInsertACar(a_car_tree *node, a_car_tree **newTree) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        a_car car = node->cars[i];
        insertacar(car, newTree);
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            traverseAndInsertACar(node->child[i], newTree);
        }
    }
}

void traverseAndInsertSCar(s_car_tree *node, s_car_tree **newTree) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        s_car car = node->cars[i];
        insertscar(car, newTree);
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            traverseAndInsertSCar(node->child[i], newTree);
        }
    }
}

a_car_tree* copyACarTree(a_car_tree *original) {
    if (!original) return NULL;
    a_car_tree *copy = createNodeacar(original->isleaf);
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    copy->n = original->n;
    for (int i = 0; i < original->n; i++) {
        copy->cars[i] = original->cars[i];
    }
    if (!original->isleaf) {
        for (int i = 0; i <= original->n; i++) {
            copy->child[i] = copyACarTree(original->child[i]);
            if (copy->child[i]) {
                copy->child[i]->parent = copy;
            }
        }
    }
    return copy;
}

s_car_tree* copySCarTree(s_car_tree *original) {
    if (!original) return NULL;
    s_car_tree *copy = createNodescar(original->isleaf);
    if (!copy) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    copy->n = original->n;
    for (int i = 0; i < original->n; i++) {
        copy->cars[i] = original->cars[i];
    }
    if (!original->isleaf) {
        for (int i = 0; i <= original->n; i++) {
            copy->child[i] = copySCarTree(original->child[i]);
            if (copy->child[i]) {
                copy->child[i]->parent = copy;
            }
        }
    }
    return copy;
}

a_car_tree* mergeACarTrees(a_car_tree *tree1, a_car_tree *tree2) {
    a_car_tree *newTree = NULL;

    if (tree1) {
        for (int i = 0; i < tree1->n; i++) {
            insertacar(tree1->cars[i], &newTree);
        }
        if (!tree1->isleaf) {
            for (int i = 0; i <= tree1->n; i++) {
                if (tree1->child[i]) {
                    a_car_tree* result = mergeACarTrees(tree1->child[i], NULL);
                    if (result) {
                        for (int j = 0; j < result->n; j++) {
                            insertacar(result->cars[j], &newTree);
                        }
                        freetreeacar(result);
                    }
                }
            }
        }
    }

    if (tree2) {
        for (int i = 0; i < tree2->n; i++) {
            insertacar(tree2->cars[i], &newTree);
        }
        if (!tree2->isleaf) {
            for (int i = 0; i <= tree2->n; i++) {
                if (tree2->child[i]) {
                    a_car_tree* result = mergeACarTrees(tree2->child[i], NULL);
                    if (result) {
                        for (int j = 0; j < result->n; j++) {
                            insertacar(result->cars[j], &newTree);
                        }
                        freetreeacar(result);
                    }
                }
            }
        }
    }

    return newTree;
}

s_car_tree* mergeSCarTrees(s_car_tree *tree1, s_car_tree *tree2) {
    s_car_tree *newTree = NULL;

    if (tree1) {
        for (int i = 0; i < tree1->n; i++) {
            insertscar(tree1->cars[i], &newTree);
        }
        if (!tree1->isleaf) {
            for (int i = 0; i <= tree1->n; i++) {
                if (tree1->child[i]) {
                    s_car_tree* result = mergeSCarTrees(tree1->child[i], NULL);
                    if (result) {
                        for (int j = 0; j < result->n; j++) {
                            insertscar(result->cars[j], &newTree);
                        }
                        freetreescar(result);
                    }
                }
            }
        }
    }

    if (tree2) {
        for (int i = 0; i < tree2->n; i++) {
            insertscar(tree2->cars[i], &newTree);
        }
        if (!tree2->isleaf) {
            for (int i = 0; i <= tree2->n; i++) {
                if (tree2->child[i]) {
                    s_car_tree* result = mergeSCarTrees(tree2->child[i], NULL);
                    if (result) {
                        for (int j = 0; j < result->n; j++) {
                            insertscar(result->cars[j], &newTree);
                        }
                        freetreescar(result);
                    }
                }
            }
        }
    }

    return newTree;
}

showroom mergeShowrooms(showroom showroom1, showroom showroom2) {
    showroom newShowroom;

    // Merge the car trees
    newShowroom.car_tree = mergeSCarTrees(showroom1.car_tree, showroom2.car_tree);
    newShowroom.car_tree1 = mergeACarTrees(showroom1.car_tree1, showroom2.car_tree1);

    // Merge the sales trees
    newShowroom.sales_tree = mergeSalesTrees(showroom1.sales_tree, showroom2.sales_tree);

    // Merge the customer trees
    newShowroom.customer_tree = mergeCustomerTrees(showroom1.customer_tree, showroom2.customer_tree);

    // Update the counts
    newShowroom.a_car_cnt = showroom1.a_car_cnt + showroom2.a_car_cnt;
    newShowroom.s_car_cnt = showroom1.s_car_cnt + showroom2.s_car_cnt;
    newShowroom.sales_cnt = showroom1.sales_cnt + showroom2.sales_cnt;

    return newShowroom;
}

typedef struct {
    char model[50];
    int count;
} CarModelCount;

#define HASH_SIZE 208

typedef struct CarModelNode {
    char model[50];
    int count;
    struct CarModelNode* next;
} CarModelNode;

CarModelNode* hashTable[HASH_SIZE];

unsigned int hash(const char* model) {
    unsigned int hashVal = 0;
    for (int i = 0; model[i] != '\0'; i++) {
        hashVal = (hashVal * 31 + model[i]) % HASH_SIZE;
    }
    return hashVal;
}

void updateCarModelHash(const char* model) {
    unsigned int index = hash(model);
    CarModelNode* curr = hashTable[index];
    while (curr) {
        if (strcmp(curr->model, model) == 0) {
            curr->count++;
            return;
        }
        curr = curr->next;
    }
    CarModelNode* newNode = (CarModelNode*)malloc(sizeof(CarModelNode));
    strcpy(newNode->model, model);
    newNode->count = 1;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

void findMostPopularCarHelper(s_car_tree* node) {
    if (!node) return;
    for (int i = 0; i < node->n; i++) {
        updateCarModelHash(node->cars[i].name);
    }
    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            findMostPopularCarHelper(node->child[i]);
        }
    }
}

void findMostPopularCar(s_car_tree* root) {
    for (int i = 0; i < HASH_SIZE; i++) {
        hashTable[i] = NULL;
    }
    findMostPopularCarHelper(root);
    char mostPopular[50] = "";
    int maxCount = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        CarModelNode* curr = hashTable[i];
        while (curr) {
            if (curr->count > maxCount) {
                maxCount = curr->count;
                strcpy(mostPopular, curr->model);
            }
            curr = curr->next;
        }
    }
    printf("Most Popular Car: %s with %d occurrences\n", mostPopular, maxCount);
    for (int i = 0; i < HASH_SIZE; i++) {
        CarModelNode* curr = hashTable[i];
        while (curr) {
            CarModelNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

void findMostSuccessfulSalespersonHelper(sales_tree* node, Salesperson** mostSuccessful) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        if (!(*mostSuccessful) || node->sales[i].salesAchieved > (*mostSuccessful)->salesAchieved) {
            *mostSuccessful = &node->sales[i];
        }
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            findMostSuccessfulSalespersonHelper(node->child[i], mostSuccessful);
        }
    }
}

void findMostSuccessfulSalesperson(sales_tree* root) {
    Salesperson* mostSuccessful = NULL;
    findMostSuccessfulSalespersonHelper(root, &mostSuccessful);

    if (mostSuccessful) {
        mostSuccessful->commission += 0.01 * mostSuccessful->salesAchieved;
        printf("Most Successful Salesperson: %s with Sales Achieved: %.2f and Commission: %.2f\n",
               mostSuccessful->name, mostSuccessful->salesAchieved, mostSuccessful->commission);
    }
}

void saleCarToCustomer(int VIN, Customer customer, char* paymentType, date soldDate,
                      a_car_tree** a_car_root, s_car_tree** s_car_root,
                      customer_tree** customer_root, Salesperson* salesperson) {
   
    a_car_tree* node = *a_car_root;
    a_car* found_car = NULL;

    
    while (node) {
        for (int i = 0; i < node->n; i++) {
            if (node->cars[i].VIN == VIN) {
                found_car = &node->cars[i];
                break;
            }
        }
        if (found_car) break;
        node = node->isleaf ? NULL : node->child[0];
    }

    if (!found_car) {
        printf("Car with VIN %d not found in available cars!\n", VIN);
        return;
    }

    
    s_car sold_car;
    sold_car.VIN = found_car->VIN;
    strcpy(sold_car.name, found_car->name);
    strcpy(sold_car.color, found_car->color);
    sold_car.price = found_car->price;
    strcpy(sold_car.fuelType, found_car->fuelType);
    strcpy(sold_car.carType, found_car->carType);
    strcpy(sold_car.paymenttype, paymentType);
    sold_car.solddate = soldDate;


    customer.car = (s_car*)malloc(sizeof(s_car));
    if (customer.car) {
        *customer.car = sold_car;
    }

   
    insertscar(sold_car, s_car_root);
    insertCustomer(customer, customer_root);
    insertCustomer(customer, &salesperson->customerList);
    updateCommission(salesperson, sold_car.price);

   
    deleteacar(VIN, a_car_root);

    printf("Car with VIN %d sold to customer %s (ID: %d) by salesperson %s (ID: %d)\n",
           VIN, customer.name, customer.customerid, salesperson->name, salesperson->id);
}

void updateCommission(Salesperson* salesperson, float carPrice) {
    salesperson->commission += 0.02 * carPrice;
    salesperson->salesAchieved += carPrice;
}

void searchSalespersonsBySalesRangeHelper(float min_sales, float max_sales, sales_tree* node) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        if (node->sales[i].salesAchieved >= min_sales && node->sales[i].salesAchieved <= max_sales) {
            printf("Salesperson: %s, Sales Achieved: %.2f\n", node->sales[i].name, node->sales[i].salesAchieved);
        }
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            searchSalespersonsBySalesRangeHelper(min_sales, max_sales, node->child[i]);
        }
    }
}

void searchSalespersonsBySalesRange(float min_sales, float max_sales, sales_tree* root) {
    searchSalespersonsBySalesRangeHelper(min_sales, max_sales, root);
}

float calculateAverageSales(sales_tree* root) {
    float totalSales = 0.0;
    int salespersonCount = 0;

    if (!root) return 0.0;

   
    for (int i = 0; i < root->n; i++) {
        totalSales += root->sales[i].salesAchieved;
        salespersonCount++;
    }

    if (!root->isleaf) {
        for (int i = 0; i <= root->n; i++) {
            totalSales += calculateAverageSales(root->child[i]);
            salespersonCount += countSalespersons(root->child[i]);
        }
    }

    return totalSales / salespersonCount;
}

int countSalespersons(sales_tree* root) {
    if (!root) return 0;

    int count = root->n;

    if (!root->isleaf) {
        for (int i = 0; i <= root->n; i++) {
            count += countSalespersons(root->child[i]);
        }
    }

    return count;
}

float predictNextMonthSales(sales_tree* root) {
    float averageSales = calculateAverageSales(root);
    return averageSales; 
}

void displayAvailableCarByVIN(int VIN, a_car_tree* node) {
    if (!node) return;

    int i = 0;
    while (i < node->n && VIN > node->cars[i].VIN) {
        i++;
    }

    if (i < node->n && node->cars[i].VIN == VIN) {
        printf("Available Car Found:\n");
        printf("VIN: %d, Name: %s, Color: %s, Price: %.2f, Fuel Type: %s, Car Type: %s\n",
               node->cars[i].VIN, node->cars[i].name, node->cars[i].color,
               node->cars[i].price, node->cars[i].fuelType, node->cars[i].carType);
        return;
    }

    if (node->isleaf) return;

    displayAvailableCarByVIN(VIN, node->child[i]);
}

void displaySoldCarByVIN(int VIN, s_car_tree* node) {
    if (!node) return;

    int i = 0;
    while (i < node->n && VIN > node->cars[i].VIN) {
        i++;
    }

    if (i < node->n && node->cars[i].VIN == VIN) {
        printf("Sold Car Found:\n");
        printf("VIN: %d, Name: %s, Color: %s, Price: %.2f, Fuel Type: %s, Car Type: %s, Payment Type: %s, Date of Sale: %02d-%02d-%04d\n",
               node->cars[i].VIN, node->cars[i].name, node->cars[i].color,
               node->cars[i].price, node->cars[i].fuelType, node->cars[i].carType,
               node->cars[i].paymenttype, node->cars[i].solddate.day,
               node->cars[i].solddate.month, node->cars[i].solddate.year);
        return;
    }

    if (node->isleaf) return;

    displaySoldCarByVIN(VIN, node->child[i]);
}


void displayCarInfoByVIN(int VIN, a_car_tree* a_car_root, s_car_tree* s_car_root) {
    displayAvailableCarByVIN(VIN, a_car_root);
    displaySoldCarByVIN(VIN, s_car_root);
}

void listCustomersWithEMIPlanHelper(customer_tree* node) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        if (node->customers[i].car && strncmp(node->customers[i].paymentType, "emi", 3) == 0) {
            int emiMonths = atoi(node->customers[i].paymentType + 4);
            if (emiMonths > 36 && emiMonths <= 48) {
                printf("Customer: %s, Car VIN: %d, EMI Duration: %d months\n",
                       node->customers[i].name, node->customers[i].car->VIN, emiMonths);
            }
        }
    }

    if (!node->isleaf) {
        for (int i = 0; i <= node->n; i++) {
            listCustomersWithEMIPlanHelper(node->child[i]);
        }
    }
}

void listCustomersWithEMIPlan(customer_tree* root) {
    listCustomersWithEMIPlanHelper(root);
}

void* checkMalloc(void* ptr) {
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed! Exiting program.\n");
        exit(1);
    }
    return ptr;
}

void loadDataFromFile(const char* filename, a_car_tree** a_car_root, s_car_tree** s_car_root,
    sales_tree** sales_root, customer_tree** customer_root) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    char line[4096];
    char section[50] = "";
    int dataCount = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        if (strlen(line) == 0) {
            continue;
        }

        if (strstr(line, "Available Cars")) {
            strcpy(section, "available_cars");
            continue;
        } else if (strstr(line, "Salespersons")) {
            strcpy(section, "salespersons");
            continue;
        } else if (strstr(line, "Customers")) {
            strcpy(section, "customers");
            continue;
        } else if (strstr(line, "Sold Cars")) {
            strcpy(section, "sold_cars");
            continue;
        }

        if (strstr(line, "VIN,") || strstr(line, "ID,") || strstr(line, "CustomerID,")) {
            continue;
        }

        char* token = strtok(line, ",");
        if (!token) continue;

        if (strcmp(section, "available_cars") == 0) {
            a_car car;
            car.VIN = atoi(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.name, token, sizeof(car.name) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.color, token, sizeof(car.color) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            car.price = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.fuelType, token, sizeof(car.fuelType) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.carType, token, sizeof(car.carType) - 1);

            insertacar(car, a_car_root);
            dataCount++;
        } else if (strcmp(section, "salespersons") == 0) {
            Salesperson salesperson = {0};
            salesperson.id = atoi(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(salesperson.name, token, sizeof(salesperson.name) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            salesperson.salesTarget = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            salesperson.salesAchieved = atof(token);

            salesperson.customerList = (customer_tree*)checkMalloc(malloc(sizeof(customer_tree)));
            salesperson.customerList->n = 0;
            salesperson.customerList->isleaf = 1;
            salesperson.customerList->parent = NULL;
            for (int i = 0; i <= MAX; i++) {
                salesperson.customerList->child[i] = NULL;
            }

            salesperson.commission = 0.0;

            insertSales(salesperson, sales_root);
            dataCount++;
        } else if (strcmp(section, "customers") == 0) {
            Customer customer;
            customer.customerid = atoi(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(customer.name, token, sizeof(customer.name) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(customer.mobileNo, token, sizeof(customer.mobileNo) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(customer.address, token, sizeof(customer.address) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(customer.registrationNo, token, sizeof(customer.registrationNo) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(customer.paymentType, token, sizeof(customer.paymentType) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            customer.car = (s_car*)malloc(sizeof(s_car));
            if (customer.car) {
                customer.car->VIN = atoi(token);
            }

            token = strtok(NULL, ",");
            if (!token) continue;
            int salespersonID = atoi(token);

            Salesperson* salesperson = findSalespersonById(salespersonID, *sales_root);
            if (salesperson) {
                insertCustomer(customer, &salesperson->customerList);
            }

            insertCustomer(customer, customer_root);
            dataCount++;
        } else if (strcmp(section, "sold_cars") == 0) {
            s_car car = {0};
            char dateStr[11];

            car.VIN = atoi(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.name, token, sizeof(car.name) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.color, token, sizeof(car.color) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            car.price = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.fuelType, token, sizeof(car.fuelType) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.carType, token, sizeof(car.carType) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(car.paymenttype, token, sizeof(car.paymenttype) - 1);

            token = strtok(NULL, ",");
            if (!token) continue;
            strncpy(dateStr, token, sizeof(dateStr) - 1);
            sscanf(dateStr, "%d-%d-%d", &car.solddate.year, &car.solddate.month, &car.solddate.day);

            token = strtok(NULL, ",");
            if (!token) continue;
            int salespersonID = atoi(token);

            Salesperson* salesperson = findSalespersonById(salespersonID, *sales_root);
            if (salesperson) {
                updateCommission(salesperson, car.price);
            }

            insertscar(car, s_car_root);
            dataCount++;
        }
    }

    fclose(file);
    printf("File reading completed. Processed %d records successfully.\n", dataCount);
}




int countCustomers(customer_tree* root) {
    if (!root) return 0;

    int count = root->n;

    if (!root->isleaf) {
        for (int i = 0; i <= root->n; i++) {
            count += countCustomers(root->child[i]);
        }
    }

    return count;
}





void printACarTreeHelper(a_car_tree* node) {
    if (!node) return;
    for (int i = 0; i < node->n; i++) {
        if (!node->isleaf) {
            printACarTreeHelper(node->child[i]);
        }
        printf("|%4d| %-14s| %-13s|%10.2f| %-9s |\n",
               node->cars[i].VIN,
               node->cars[i].name,
               node->cars[i].color,
               node->cars[i].price,
               node->cars[i].carType);
    }
    if (!node->isleaf) {
        printACarTreeHelper(node->child[node->n]);
    }
}

void printSCarTreeHelper(s_car_tree* node) {
    if (!node) return;
    for (int i = 0; i < node->n; i++) {
        if (!node->isleaf) {
            printSCarTreeHelper(node->child[i]);
        }
        printf("|%4d| %-14s| %-13s|%10.2f| %-8s|%02d-%02d-%04d|\n",
               node->cars[i].VIN,
               node->cars[i].name,
               node->cars[i].color,
               node->cars[i].price,
               node->cars[i].paymenttype,
               node->cars[i].solddate.day,
               node->cars[i].solddate.month,
               node->cars[i].solddate.year);
    }
    if (!node->isleaf) {
        printSCarTreeHelper(node->child[node->n]);
    }
}

void printSalesTreeHelper(sales_tree* node) {
    if (!node) return;
    for (int i = 0; i < node->n; i++) {
        if (!node->isleaf) {
            printSalesTreeHelper(node->child[i]);
        }
        printf("|%4d| %-14s|%13.2f|%10.2f|%10.2f|%8d |\n",
               node->sales[i].id,
               node->sales[i].name,
               node->sales[i].salesTarget/100000,
               node->sales[i].salesAchieved/100000,
               node->sales[i].commission,
               countCustomers(node->sales[i].customerList));
    }
    if (!node->isleaf) {
        printSalesTreeHelper(node->child[node->n]);
    }
}

void printCustomerTreeHelper(customer_tree* node) {
    if (!node) return;
    for (int i = 0; i < node->n; i++) {
        if (!node->isleaf) {
            printCustomerTreeHelper(node->child[i]);
        }
        printf("|%4d| %-14s| %-10s | %-12s | %-7s | %-6s|\n",
               node->customers[i].customerid,
               node->customers[i].name,
               node->customers[i].mobileNo,
               node->customers[i].address,
               node->customers[i].registrationNo,
               node->customers[i].paymentType);
    }
    if (!node->isleaf) {
        printCustomerTreeHelper(node->child[node->n]);
    }
}


void printShowroomData(showroom* showroom) {
    printf("\n+================================================+\n");
    printf("|                 SHOWROOM DATA                    |\n");
    printf("+================================================+\n\n");

    // Available Cars Section
    printf("+================================================+\n");
    printf("|                AVAILABLE CARS                    |\n");
    printf("+====+================+===============+==========+===========+\n");
    printf("| VIN|     Name      |    Color      |  Price   |   Type    |\n");
    printf("+====+================+===============+==========+===========+\n");
    printACarTreeHelper(showroom->car_tree1);
    printf("+====+================+===============+==========+===========+\n\n");

    // Sold Cars Section
    printf("+=========================================================+\n");
    printf("|                      SOLD CARS                           |\n");
    printf("+====+================+===============+==========+==========+==========+\n");
    printf("| VIN|     Name      |    Color      |  Price   | Payment | SoldDate |\n");
    printf("+====+================+===============+==========+==========+==========+\n");
    printSCarTreeHelper(showroom->car_tree);
    printf("+====+================+===============+==========+==========+==========+\n\n");

    // Salespersons Section
    printf("+=========================================================+\n");
    printf("|                     SALESPERSONS                         |\n");
    printf("+====+================+===============+==========+==========+=========+\n");
    printf("| ID |     Name      |Target(Lakhs)  |Achieved  |Commission|Clients |\n");
    printf("+====+================+===============+==========+==========+=========+\n");
    printSalesTreeHelper(showroom->sales_tree);
    printf("+====+================+===============+==========+==========+=========+\n\n");

    // Customers Section
    printf("+================================================================+\n");
    printf("|                         CUSTOMERS                                |\n");
    printf("+====+================+============+==============+=========+========+\n");
    printf("| ID |     Name      | Mobile No  |   Address    | Reg No  | Type  |\n");
    printf("+====+================+============+==============+=========+========+\n");
    printCustomerTreeHelper(showroom->customer_tree);
    printf("+====+================+============+==============+=========+========+\n");
}
showroom mergeAllShowrooms(showroom showroom1, showroom showroom2, showroom showroom3) {
    showroom newShowroom;

    // Merge the car trees
    newShowroom.car_tree = mergeSCarTrees(mergeSCarTrees(showroom1.car_tree, showroom2.car_tree), showroom3.car_tree);
    newShowroom.car_tree1 = mergeACarTrees(mergeACarTrees(showroom1.car_tree1, showroom2.car_tree1), showroom3.car_tree1);

    // Merge the sales trees
    newShowroom.sales_tree = mergeSalesTrees(mergeSalesTrees(showroom1.sales_tree, showroom2.sales_tree), showroom3.sales_tree);

    // Merge the customer trees
    newShowroom.customer_tree = mergeCustomerTrees(mergeCustomerTrees(showroom1.customer_tree, showroom2.customer_tree), showroom3.customer_tree);

    // Update the counts
    newShowroom.a_car_cnt = showroom1.a_car_cnt + showroom2.a_car_cnt + showroom3.a_car_cnt;
    newShowroom.s_car_cnt = showroom1.s_car_cnt + showroom2.s_car_cnt + showroom3.s_car_cnt;
    newShowroom.sales_cnt = showroom1.sales_cnt + showroom2.sales_cnt + showroom3.sales_cnt;

    return newShowroom;
}
void addSalesperson(Salesperson salesperson, sales_tree** root_ptr) {
    insertSales(salesperson, root_ptr);
}
Salesperson* findSalespersonById(int id, sales_tree* root) {
    if (!root) return NULL;

    int i = 0;

    
    while (i < root->n && id > root->sales[i].id) {
        i++;
    }

    
    if (i < root->n && root->sales[i].id == id) {
        return &root->sales[i];
    }

   
    if (root->isleaf) {
        return NULL;
    }

   
    return findSalespersonById(id, root->child[i]);
}

void findMostPopularCarAcrossShowrooms(showroom showroom1, showroom showroom2, showroom showroom3) {
    for (int i = 0; i < HASH_SIZE; i++) {
        hashTable[i] = NULL;
    }

    findMostPopularCarHelper(showroom1.car_tree);
    findMostPopularCarHelper(showroom2.car_tree);
    findMostPopularCarHelper(showroom3.car_tree);

    char mostPopular[50] = "";
    int maxCount = 0;

    for (int i = 0; i < HASH_SIZE; i++) {
        CarModelNode* curr = hashTable[i];
        while (curr) {
            if (curr->count > maxCount) {
                maxCount = curr->count;
                strcpy(mostPopular, curr->model);
            }
            curr = curr->next;
        }
    }

    printf("Most Popular Car Across All Showrooms: %s with %d occurrences\n", mostPopular, maxCount);

    for (int i = 0; i < HASH_SIZE; i++) {
        CarModelNode* curr = hashTable[i];
        while (curr) {
            CarModelNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}
 
void findMostSuccessfulSalespersonAcrossShowrooms(showroom showroom1, showroom showroom2, showroom showroom3) {
    Salesperson* mostSuccessful = NULL;

    findMostSuccessfulSalespersonHelper(showroom1.sales_tree, &mostSuccessful);
    findMostSuccessfulSalespersonHelper(showroom2.sales_tree, &mostSuccessful);
    findMostSuccessfulSalespersonHelper(showroom3.sales_tree, &mostSuccessful);

    if (mostSuccessful) {
        mostSuccessful->commission += 0.01 * mostSuccessful->salesAchieved;
        printf("Most Successful Salesperson Across All Showrooms: %s with Sales Achieved: %.2f and Commission: %.2f\n",
               mostSuccessful->name, mostSuccessful->salesAchieved, mostSuccessful->commission);
    }
}
void displayCarInfoByVINAcrossShowrooms(int VIN, showroom showroom1, showroom showroom2, showroom showroom3) {
    displayCarInfoByVIN(VIN, showroom1.car_tree1, showroom1.car_tree);
    displayCarInfoByVIN(VIN, showroom2.car_tree1, showroom2.car_tree);
    displayCarInfoByVIN(VIN, showroom3.car_tree1, showroom3.car_tree);
}
void freeshowroom(showroom* showroom) {
    freetreeacar(showroom->car_tree1);
    freetreescar(showroom->car_tree);
    freeTreeSales(showroom->sales_tree);
    freeTreeCustomer(showroom->customer_tree);
}
int main() {
    showroom showrooms[3];
    const char* filenames[] = {
        "showroom1.txt",
        "showroom2.txt",
        "showroom3.txt"
    };

    // Initialize showrooms
    for (int i = 0; i < 3; i++) {
        showrooms[i].car_tree = NULL;
        showrooms[i].car_tree1 = NULL;
        showrooms[i].sales_tree = NULL;
        showrooms[i].customer_tree = NULL;
        showrooms[i].a_car_cnt = 0;
        showrooms[i].s_car_cnt = 0;
        showrooms[i].sales_cnt = 0;

        FILE* test = fopen(filenames[i], "r");
        if (!test) {
            printf("Error: Cannot open file %s\n", filenames[i]);
            printf("Please ensure the file exists and you have proper permissions.\n");
            return 1;
        }
        fclose(test);

        loadDataFromFile(filenames[i], &showrooms[i].car_tree1, &showrooms[i].car_tree, &showrooms[i].sales_tree, &showrooms[i].customer_tree);
    }

    int showroomChoice, choice;
    while (1) {
        printf("\nSelect Showroom (1, 2, or 3) or 0 to Exit: ");
        scanf("%d", &showroomChoice);

        if (showroomChoice == 0) {
           
            for (int i = 0; i < 3; i++) {
                freetreeacar(showrooms[i].car_tree1);
                freetreescar(showrooms[i].car_tree);
                freeTreeSales(showrooms[i].sales_tree);
                freeTreeCustomer(showrooms[i].customer_tree);
            }
            printf("\nProgram completed successfully.\n");
            return 0;
        }

        if (showroomChoice < 1 || showroomChoice > 3) {
            printf("Invalid showroom choice! Please try again.\n");
            continue;
        }

        showroom* selectedShowroom = &showrooms[showroomChoice - 1];

        printf("\nShowroom %d Management System\n", showroomChoice);
        printf("1. Print Showroom Data\n");
        printf("2. Find Most Popular Car Across All Showrooms\n");
        printf("3. Find Most Successful Salesperson Across All Showrooms\n");
        printf("4. Sale Car to Customer\n");
        printf("5. Search Salespersons by Sales Range\n");
        printf("6. Predict Next Month Sales\n");
        printf("7. Display Car Info by VIN Across All Showrooms\n");
        printf("8. List Customers with EMI Plan\n");
        printf("9. Merge All Showrooms and Print\n");
        printf("10. Add New Salesperson\n");
        printf("11.insert a car\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printShowroomData(selectedShowroom);
                break;
            case 2:
                findMostPopularCarAcrossShowrooms(showrooms[0], showrooms[1], showrooms[2]);
                break;
            case 3:
                findMostSuccessfulSalespersonAcrossShowrooms(showrooms[0], showrooms[1], showrooms[2]);
                break;
            case 4: {
                int VIN;
                Customer customer;
                date soldDate;
                char paymentType[10];

                printf("Enter VIN of car to be sold: ");
                scanf("%d", &VIN);

                printf("Enter customer details (ID Name MobileNo Address RegistrationNo): ");
                scanf("%d %s %s %s %s",
                      &customer.customerid,
                      customer.name,
                      customer.mobileNo,
                      customer.address,
                      customer.registrationNo);

                printf("Enter payment type (cash/emi-XX): ");
                scanf("%s", paymentType);
                strcpy(customer.paymentType, paymentType);

                printf("Enter sold date (DD MM YYYY): ");
                scanf("%d %d %d",
                      &soldDate.day,
                      &soldDate.month,
                      &soldDate.year);

                printf("Enter salesperson ID: ");
                int salespersonID;
                scanf("%d", &salespersonID);

                Salesperson* salesperson = findSalespersonById(salespersonID,
                                                             selectedShowroom->sales_tree);
                if (salesperson) {
                    saleCarToCustomer(VIN, customer, paymentType, soldDate,
                                     &selectedShowroom->car_tree1,
                                     &selectedShowroom->car_tree,
                                     &selectedShowroom->customer_tree,
                                     salesperson);
                } else {
                    printf("Salesperson not found!\n");
                }
                break;
            }
            case 5: {
                float min_sales, max_sales;
                printf("Enter minimum and maximum sales range: ");
                scanf("%f %f", &min_sales, &max_sales);
                searchSalespersonsBySalesRange(min_sales, max_sales, selectedShowroom->sales_tree);
                break;
            }
            case 6:
                printf("Predicted Next Month Sales: %.2f\n", predictNextMonthSales(selectedShowroom->sales_tree));
                break;
            case 7: {
                int VIN;
                printf("Enter VIN to display car info: ");
                scanf("%d", &VIN);
                displayCarInfoByVINAcrossShowrooms(VIN, showrooms[0], showrooms[1], showrooms[2]);
                break;
            }
            case 8:
                listCustomersWithEMIPlan(selectedShowroom->customer_tree);
                break;
            case 9: {
                showroom mergedShowroom = mergeAllShowrooms(showrooms[0], showrooms[1], showrooms[2]);
                printShowroomData(&mergedShowroom);
                break;
            }
            case 10: {
                Salesperson newSalesperson;
                printf("Enter new salesperson details (ID Name SalesTarget SalesAchieved): ");
                scanf("%d %s %f %f", &newSalesperson.id, newSalesperson.name, &newSalesperson.salesTarget, &newSalesperson.salesAchieved);
                newSalesperson.customerList = createNodeCustomer(1);
                newSalesperson.commission = 0.0;
                addSalesperson(newSalesperson, &selectedShowroom->sales_tree);
                break;
            }
            case 11: {
                a_car newCar;
                printf("Enter new car details (VIN Name Color Price FuelType CarType): ");
                scanf("%d %s %s %f %s %s", &newCar.VIN, newCar.name, newCar.color, &newCar.price, newCar.fuelType, newCar.carType);
                insertacar(newCar, &selectedShowroom->car_tree1);
                break;
            }
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
    freeShowroom(&showrooms[0]);
    freeShowroom(&showrooms[1]);
    freeShowroom(&showrooms[2]);

   
    
    


}
