#include "commandmanager.h"
#include "exceptions.h"

void CommandManager::executeCommand(std::unique_ptr<Command> command) {
    try {
        command->execute();
        undoStack.push(std::move(command));
        
        // Очищаем redo стек при новом действии
        while (!redoStack.empty()) {
            redoStack.pop();
        }
        
        // Ограничиваем размер истории
        if (undoStack.size() > MAX_HISTORY) {
            std::stack<std::unique_ptr<Command>> temp;
            while (undoStack.size() > MAX_HISTORY - 1) {
                undoStack.pop();
            }
        }
    } catch (const LibraryException& e) {
        // Если команда не может выполниться, не добавляем её в стек
        throw; // Re-throw to let caller handle the exception
    }
}

void CommandManager::undo() {
    if (undoStack.empty()) {
        throw LibraryException("Нет действий для отмены");
    }
    auto command = std::move(undoStack.top());
    undoStack.pop();
    command->undo();
    redoStack.push(std::move(command));
}

void CommandManager::redo() {
    if (redoStack.empty()) {
        throw LibraryException("Нет действий для повторения");
    }
    auto command = std::move(redoStack.top());
    redoStack.pop();
    command->execute();
    undoStack.push(std::move(command));
}

void CommandManager::clear() {
    while (!undoStack.empty()) {
        undoStack.pop();
    }
    while (!redoStack.empty()) {
        redoStack.pop();
    }
}

