#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class LibraryException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class FileException : public LibraryException {
public:
    explicit FileException(const std::string& message) 
        : LibraryException("Ошибка файла: " + message) {}
};

class DataException : public LibraryException {
public:
    explicit DataException(const std::string& message) 
        : LibraryException("Ошибка данных: " + message) {}
};

class NotFoundException : public DataException {
public:
    explicit NotFoundException(const std::string& item) 
        : DataException("Элемент не найден: " + item) {}
};

class DuplicateException : public DataException {
public:
    explicit DuplicateException(const std::string& item) 
        : DataException("Дублирующийся элемент: " + item) {}
};

class CommandException : public LibraryException {
public:
    explicit CommandException(const std::string& message) 
        : LibraryException("Ошибка команды: " + message) {}
};

#endif // EXCEPTIONS_H

