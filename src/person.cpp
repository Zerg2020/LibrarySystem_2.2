#include "person.h"
#include <sstream>

Person::Person(int pId, const std::string& pName, const std::string& pSurname, const std::string& pPhone, const std::string& pEmail)
    : name(pName), surname(pSurname), phone(pPhone), email(pEmail), id(pId) {
}

std::string Person::getFullName() const {
    return name + " " + surname;
}

