#ifndef MANAGER_H
#define MANAGER_H

#include "employee.h"

class Manager : public Employee {
private:
    int employeesManaged = 0;

public:
    explicit Manager(int pId, const std::string& pName, const std::string& pSurname,
            const std::string& pPhone, double pSalary, int pWorkHours);
    
    int getEmployeesManaged() const { return employeesManaged; }
    void setEmployeesManaged(int pCount) { employeesManaged = pCount; }
    
    double calculateMonthlySalary() const override;
    std::string getInfo() const override;
    std::string getType() const override { return "Manager"; }
};

#endif // MANAGER_H

