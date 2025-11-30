#ifndef PERSON_H
#define PERSON_H

#include <string>
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

    void setName(const std::string& pName) { name = pName; }
    void setSurname(const std::string& pSurname) { surname = pSurname; }
    void setPhone(const std::string& pPhone) { phone = pPhone; }
    void setEmail(const std::string& pEmail) { email = pEmail; }

    virtual std::string getInfo() const = 0;
    virtual std::string getType() const = 0;
};

#endif // PERSON_H

