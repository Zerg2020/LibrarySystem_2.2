#include "person.h"
#include <sstream>

Person::Person(int pId, const std::string& pName, const std::string& pSurname, const std::string& pPhone, const std::string& pEmail)
    : id(pId), name(pName), surname(pSurname), phone(pPhone), email(pEmail) {
}

std::string Person::getFullName() const {
    return name + " " + surname;
}

