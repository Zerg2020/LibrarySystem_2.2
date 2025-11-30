#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "person.h"
#include <string>
#include <string_view>

class Employee : public Person {
protected:
    std::string position;
    double salary;
    int workHours;

public:
    explicit Employee(int pId, const std::string& pName, const std::string& pSurname, 
             const std::string& pPhone, const std::string& pPosition, double pSalary, int pWorkHours);
    
    std::string getPosition() const { return position; }
    double getSalary() const { return salary; }
    int getWorkHours() const { return workHours; }
    
    void setPosition(std::string_view pPosition) { position = pPosition; }
    void setSalary(double pSalary) { salary = pSalary; }
    void setWorkHours(int pWorkHours) { workHours = pWorkHours; }
    
    virtual double calculateMonthlySalary() const = 0;
    std::string getInfo() const override;
    std::string getType() const override { return "Employee"; }
};

#endif // EMPLOYEE_H

