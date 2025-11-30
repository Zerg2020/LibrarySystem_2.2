#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "command.h"
#include <stack>
#include <memory>

class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> undoStack;
    std::stack<std::unique_ptr<Command>> redoStack;
    static const size_t MAX_HISTORY = 50;

public:
    void executeCommand(std::unique_ptr<Command> command);
    void undo();
    void redo();
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    void clear();
};

#endif // COMMANDMANAGER_H

