#include "manager.h"
#include <sstream>

Manager::Manager(int pId, const std::string& pName, const std::string& pSurname,
                 const std::string& pPhone, double pSalary, int pWorkHours)
    : Employee(pId, pName, pSurname, pPhone, "Manager", pSalary, pWorkHours) {
}

double Manager::calculateMonthlySalary() const {
    // Базовая зарплата + бонус за управление сотрудниками
    return salary + (employeesManaged * 50.0);
}

std::string Manager::getInfo() const {
    std::ostringstream oss;
    oss << Employee::getInfo() 
        << ", Employees Managed: " << employeesManaged
        << ", Monthly Salary: " << calculateMonthlySalary();
    return oss.str();
}

