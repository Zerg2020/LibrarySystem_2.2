#include "employee.h"
#include <sstream>

Employee::Employee(int pId, std::string_view pName, std::string_view pSurname,
                   std::string_view pPhone, std::string_view pPosition, double pSalary, int pWorkHours)
    : Person(pId, pName, pSurname, pPhone), position(pPosition), salary(pSalary), workHours(pWorkHours) {
}

std::string Employee::getInfo() const {
    std::ostringstream oss;
    oss << "ID: " << getId() << ", " << getFullName() 
        << ", Position: " << getPosition() 
        << ", Salary: " << getSalary() 
        << ", Work Hours: " << getWorkHours();
    return oss.str();
}

