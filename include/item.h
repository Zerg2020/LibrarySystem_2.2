#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <string_view>

class Item {
protected:
    int id;
    std::string title;
    bool available = true;

public:
    explicit Item(int pId, std::string_view pTitle);
    virtual ~Item() = default;

    int getId() const { return id; }
    std::string getTitle() const { return title; }
    bool isAvailable() const { return available; }

    void setTitle(std::string_view pTitle) { title = pTitle; }
    void setAvailable(bool pAvailable) { available = pAvailable; }

    virtual std::string getInfo() const = 0;
    virtual std::string getType() const = 0;
};

#endif // ITEM_H

