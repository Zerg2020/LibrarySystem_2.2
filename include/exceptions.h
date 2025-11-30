#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class LibraryException : public std::runtime_error {
public:
    explicit LibraryException(const std::string& message) 
        : std::runtime_error(message) {}
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

#endif // EXCEPTIONS_H

