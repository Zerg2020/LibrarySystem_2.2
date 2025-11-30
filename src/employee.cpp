#include "employee.h"
#include <sstream>

Employee::Employee(int pId, const std::string& pName, const std::string& pSurname,
                   const std::string& pPhone, const std::string& pPosition, double pSalary, int pWorkHours)
    : Person(pId, pName, pSurname, pPhone), position(pPosition), salary(pSalary), workHours(pWorkHours) {
}

std::string Employee::getInfo() const {
    std::ostringstream oss;
    oss << "ID: " << id << ", " << getFullName() 
        << ", Position: " << position 
        << ", Salary: " << salary 
        << ", Work Hours: " << workHours;
    return oss.str();
}

