# Car Showroom Management System Using B-Trees

A robust C project for managing multiple car showrooms, utilizing B-Trees to efficiently store and retrieve data about available cars, sold cars, customers, and salespersons. The project demonstrates advanced data structures, file-driven initialization, and provides a menu-driven interface for typical showroom operations.

---

## Features

- **Multiple Showroom Support:** Handles three showrooms, each with its own inventory, customers, and salespersons. Data can be merged and queried across all showrooms.
- **B-Tree Based Storage:** Uses B-Trees for fast insert/search/delete operations for cars, customers, and salespersons.
- **File Input:** Loads initial data from text files for available cars, sold cars, customers, and salespersons for each showroom.
- **Sales and Customer Management:** Supports car sales with payment tracking (cash, EMI), commission calculation, and customer assignments.
- **Statistics & Queries:** Find the most popular car, the most successful salesperson, search salespersons by sales range, and predict next month’s sales.
- **Comprehensive Printing:** Pretty-prints all data in tabular format for available cars, sold cars, customers, and salespersons.
- **Merge & Analytics:** Merge all showroom data and run analytics across combined data.

---

## File Structure

- `bt23cse097.c` – Main source code implementing all logic, data structures, and menu interface.
- `showroom1.txt`, `showroom2.txt`, `showroom3.txt` – Sample files containing data for each showroom (cars, salespersons, customers, sold cars).

---

## Getting Started

### Prerequisites

- GCC compiler (or any C99-compliant compiler)
- Standard C library

### Compilation

```sh
gcc bt23cse097.c -o showroom
```

### Running the Program

Make sure `showroom1.txt`, `showroom2.txt`, and `showroom3.txt` are present in the same directory as the executable.

```sh
./showroom
```

You will be prompted to select a showroom and presented with a menu of management operations.

---

## Data File Format

Each `showroomX.txt` file is divided into four sections:

- **Available Cars:**  
  `VIN,Name,Color,Price,FuelType,CarType`
- **Salespersons:**  
  `ID,Name,SalesTarget,SalesAchieved`
- **Customers:**  
  `CustomerID,Name,MobileNo,Address,RegistrationNo,PaymentType,CarVIN,SalespersonID`
- **Sold Cars:**  
  `VIN,Name,Color,Price,FuelType,CarType,PaymentType,SoldDate,SalespersonID`

Sample files are included for reference.

---

## Main Menu Options

- **Print Showroom Data:** View the current inventory, customers, salespersons, and sales.
- **Find Most Popular Car/Salesperson (All Showrooms):** Analytics across all showrooms.
- **Sale Car to Customer:** Register a car sale, assign to customer and salesperson, update inventories.
- **Search Salespersons by Sales Range:** Query sales staff by total sales achieved.
- **Predict Next Month Sales:** Simple prediction based on average sales.
- **Display Car Info by VIN Across All Showrooms:** Lookup car details in all showrooms.
- **List Customers with EMI Plan:** List customers with EMI payment plans (with >36 and ≤48 months).
- **Merge All Showrooms and Print:** Combine all showroom data and print merged statistics.
- **Add New Salesperson / Insert a Car:** Add new entries to the system.

---

## Highlights of Implementation

- **Efficient B-Tree Operations:** Fast lookup, insertion, deletion, and balancing for all major entities.
- **Dynamic Memory Management:** All data structures are dynamically allocated. Clean-up routines included.
- **Comprehensive Data Handling:** Handles multiple relationships (e.g., cars sold to customers, salespersons assigned to customers).
- **Hash Table for Popularity:** Quickly finds the most popular car model using a hash table.

---

## Authors

- [Pushkar3112](https://github.com/Pushkar3112)

---


