#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
protected:
    int id;
    std::string title;
    bool available = true;

public:
    explicit Item(int pId, const std::string& pTitle);
    virtual ~Item() = default;

    int getId() const { return id; }
    std::string getTitle() const { return title; }
    bool isAvailable() const { return available; }

    void setTitle(const std::string& pTitle) { title = pTitle; }
    void setAvailable(bool pAvailable) { available = pAvailable; }

    virtual std::string getInfo() const = 0;
    virtual std::string getType() const = 0;
};

#endif // ITEM_H

