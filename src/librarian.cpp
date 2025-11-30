#include "librarian.h"
#include <sstream>

Librarian::Librarian(int pId, std::string_view pName, std::string_view pSurname,
                     std::string_view pPhone, double pSalary, int pWorkHours)
    : Employee(pId, pName, pSurname, pPhone, "Librarian", pSalary, pWorkHours) {
}

double Librarian::calculateMonthlySalary() const {
    // Базовая зарплата + бонус за обработанные книги
    return salary + (booksProcessed * 0.1);
}

std::string Librarian::getInfo() const {
    std::ostringstream oss;
    oss << Employee::getInfo() 
        << ", Books Processed: " << booksProcessed
        << ", Monthly Salary: " << calculateMonthlySalary();
    return oss.str();
}

