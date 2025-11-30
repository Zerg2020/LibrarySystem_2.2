#include "person.h"
#include <sstream>

Person::Person(int pId, std::string_view pName, std::string_view pSurname, std::string_view pPhone, std::string_view pEmail)
    : name(pName), surname(pSurname), phone(pPhone), email(pEmail), id(pId) {
}

std::string Person::getFullName() const {
    return name + " " + surname;
}

