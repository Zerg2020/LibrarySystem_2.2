#include "manager.h"
#include <sstream>

Manager::Manager(int pId, std::string_view pName, std::string_view pSurname,
                 std::string_view pPhone, double pSalary, int pWorkHours)
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

