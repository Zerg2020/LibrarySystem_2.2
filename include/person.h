#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <string_view>
#include <memory>

class Person {
protected:
    std::string name;
    std::string surname;
    std::string phone;
    std::string email;
    int id;

public:
    explicit Person(int pId, const std::string& pName, const std::string& pSurname, const std::string& pPhone, const std::string& pEmail = "");
    virtual ~Person() = default;

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getSurname() const { return surname; }
    std::string getPhone() const { return phone; }
    std::string getEmail() const { return email; }
    std::string getFullName() const;

    void setName(std::string_view pName) { name = pName; }
    void setSurname(std::string_view pSurname) { surname = pSurname; }
    void setPhone(std::string_view pPhone) { phone = pPhone; }
    void setEmail(std::string_view pEmail) { email = pEmail; }

    virtual std::string getInfo() const = 0;
    virtual std::string getType() const = 0;
};

#endif // PERSON_H

