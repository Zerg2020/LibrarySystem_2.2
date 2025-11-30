#ifndef LIBRARIAN_H
#define LIBRARIAN_H

#include "employee.h"

class Librarian : public Employee {
private:
    int booksProcessed = 0;

public:
    explicit Librarian(int pId, const std::string& pName, const std::string& pSurname,
              const std::string& pPhone, double pSalary, int pWorkHours);
    
    int getBooksProcessed() const { return booksProcessed; }
    void incrementBooksProcessed() { booksProcessed++; }
    
    double calculateMonthlySalary() const override;
    std::string getInfo() const override;
    std::string getType() const override { return "Librarian"; }
};

#endif // LIBRARIAN_H

