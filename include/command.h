#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <string>

class LibrarySystem;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
};

#endif // COMMAND_H

