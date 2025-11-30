#ifndef LIBRARIAN_H
#define LIBRARIAN_H

#include "employee.h"

class Librarian : public Employee {
private:
    int booksProcessed = 0;

public:
    explicit Librarian(int pId, std::string_view pName, std::string_view pSurname,
              std::string_view pPhone, double pSalary, int pWorkHours);
    
    int getBooksProcessed() const { return booksProcessed; }
    void incrementBooksProcessed() { booksProcessed++; }
    
    double calculateMonthlySalary() const override;
    std::string getInfo() const override;
    std::string getType() const override { return "Librarian"; }
};

#endif // LIBRARIAN_H

