#ifndef COMMANDS_H
#define COMMANDS_H

#include "command.h"
#include "librarysystem.h"
#include "book.h"
#include "librarymember.h"
#include "employee.h"
#include <string>
#include <string_view>

// Команда добавления книги
class AddBookCommand : public Command {
private:
    LibrarySystem* system;
    int bookId;
    std::string title;
    std::string author;
    std::string isbn;
    std::string genre;
    std::string coverPath;
    std::string description;
    std::string pdfPath;
    int year;
    int quantity;
    
public:
    AddBookCommand(LibrarySystem* sys, int id, std::string_view t, std::string_view a,
                   std::string_view i, int y, std::string_view g,
                   std::string_view cp, int q, std::string_view d, std::string_view pdf)
        : system(sys), bookId(id), title(t), author(a), isbn(i), genre(g),
          coverPath(cp), description(d), pdfPath(pdf), year(y), quantity(q) {}
    
    void execute() override {
        // Добавляем книгу напрямую без создания команды
        system->addBookWithId(bookId, title, author, isbn, year, genre, true, coverPath, quantity, description, pdfPath);
    }
    
    void undo() override {
        system->removeBookDirect(bookId);
    }
    
    std::string getDescription() const override {
        return "Добавить книгу: " + title;
    }
};

// Команда удаления книги
class RemoveBookCommand : public Command {
private:
    LibrarySystem* system;
    int bookId;
    std::string title;
    std::string author;
    std::string isbn;
    std::string genre;
    std::string coverPath;
    std::string description;
    std::string pdfPath;
    int year;
    int quantity;
    bool available;
    bool manuallyDisabled;
    
public:
    RemoveBookCommand(LibrarySystem* sys, int id) : system(sys), bookId(id) {
        const Book* book = sys->findBook(id);
        if (book) {
            title = book->getTitle();
            author = book->getAuthor();
            isbn = book->getIsbn();
            year = book->getYear();
            genre = book->getGenre();
            coverPath = book->getCoverPath();
            quantity = book->getQuantity();
            description = book->getDescription();
            pdfPath = book->getPdfPath();
            available = book->isAvailable();
            manuallyDisabled = book->getManuallyDisabled();
        }
    }
    
    void execute() override {
        // Получаем книгу перед удалением для удаления обложки
        const Book* book = system->findBook(bookId);
        std::string coverPathToDelete = "";
        if (book) {
            coverPathToDelete = book->getCoverPath();
        }
        
        // Удаляем книгу напрямую через контейнер (без команды, чтобы избежать рекурсии)
        // Используем прямой доступ к контейнеру через дружественный класс или публичный метод
        // Для этого нужно добавить метод removeBookDirect в LibrarySystem
        system->removeBookDirect(bookId);
        
        // Удаляем файл обложки если он существует
        if (!coverPathToDelete.empty()) {
            #ifdef _WIN32
            std::remove(coverPathToDelete.c_str());
            #else
            std::remove(coverPathToDelete.c_str());
            #endif
        }
    }
    
    void undo() override {
        // Восстанавливаем книгу
        system->addBookWithId(bookId, title, author, isbn, year, genre, available, coverPath, quantity, description, pdfPath);
        Book* book = system->findBook(bookId);
        if (book) {
            book->setManuallyDisabled(manuallyDisabled);
        }
    }
    
    std::string getDescription() const override {
        return "Удалить книгу: " + title;
    }
};

// Команда редактирования книги
class EditBookCommand : public Command {
private:
    LibrarySystem* system;
    int bookId;
    std::string oldTitle;
    std::string oldAuthor;
    std::string oldIsbn;
    std::string oldGenre;
    std::string oldCoverPath;
    std::string oldDescription;
    std::string oldPdfPath;
    std::string newTitle;
    std::string newAuthor;
    std::string newIsbn;
    std::string newGenre;
    std::string newCoverPath;
    std::string newDescription;
    std::string newPdfPath;
    int oldYear;
    int oldQuantity;
    int newYear;
    int newQuantity;
    bool oldAvailable;
    bool newAvailable;
    bool oldManuallyDisabled;
    bool newManuallyDisabled;
    
public:
    EditBookCommand(LibrarySystem* sys, int id, std::string_view t, std::string_view a,
                   std::string_view i, int y, std::string_view g,
                   std::string_view cp, int q, std::string_view d, std::string_view pdf) 
        : system(sys), bookId(id), newTitle(t), newAuthor(a), newIsbn(i), newGenre(g),
          newCoverPath(cp), newDescription(d), newPdfPath(pdf), newYear(y), newQuantity(q) {
        const Book* book = sys->findBook(id);
        if (book) {
            // Сохраняем старые значения
            oldTitle = book->getTitle();
            oldAuthor = book->getAuthor();
            oldIsbn = book->getIsbn();
            oldYear = book->getYear();
            oldGenre = book->getGenre();
            oldCoverPath = book->getCoverPath();
            oldQuantity = book->getQuantity();
            oldDescription = book->getDescription();
            oldPdfPath = book->getPdfPath();
            oldAvailable = book->isAvailable();
            oldManuallyDisabled = book->getManuallyDisabled();
            
            // Новые значения available и manuallyDisabled будут получены в execute
            // (они могут измениться через updateBookAvailability)
        }
    }
    
    void execute() override {
        system->editBookDirect(bookId, newTitle, newAuthor, newIsbn, newYear, newGenre, newCoverPath, newQuantity, newDescription, newPdfPath);
        const Book* book = system->findBook(bookId);
        if (book) {
            // Обновляем доступность (available может измениться автоматически)
            system->updateBookAvailability(bookId);
            // Сохраняем новые значения для undo (manuallyDisabled не меняется в editBookDirect)
            newAvailable = book->isAvailable();
            newManuallyDisabled = book->getManuallyDisabled();
        }
    }
    
    void undo() override {
        system->editBookDirect(bookId, oldTitle, oldAuthor, oldIsbn, oldYear, oldGenre, oldCoverPath, oldQuantity, oldDescription, oldPdfPath);
        Book* book = system->findBook(bookId);
        if (book) {
            book->setAvailable(oldAvailable);
            book->setManuallyDisabled(oldManuallyDisabled);
            system->updateBookAvailability(bookId);
        }
    }
    
    std::string getDescription() const override {
        return "Редактировать книгу: " + newTitle;
    }
};

// Команда добавления абонента
class AddMemberCommand : public Command {
private:
    LibrarySystem* system;
    int memberId;
    std::string name;
    std::string surname;
    std::string phone;
    std::string email;
    
public:
    AddMemberCommand(LibrarySystem* sys, int id, std::string_view n, std::string_view s, std::string_view p, std::string_view e = "")
        : system(sys), memberId(id), name(n), surname(s), phone(p), email(e) {}
    
    void execute() override {
        system->addMemberWithId(memberId, name, surname, phone, false, email);
    }
    
    void undo() override {
        system->removeMemberDirect(memberId);
    }
    
    std::string getDescription() const override {
        return "Добавить абонента: " + name + " " + surname;
    }
};

// Команда удаления абонента
class RemoveMemberCommand : public Command {
private:
    LibrarySystem* system;
    int memberId;
    std::string name;
    std::string surname;
    std::string phone;
    std::string email;
    bool blocked;
    
public:
    RemoveMemberCommand(LibrarySystem* sys, int id) : system(sys), memberId(id) {
        const LibraryMember* member = sys->findMember(id);
        if (member) {
            name = member->getName();
            surname = member->getSurname();
            phone = member->getPhone();
            email = member->getEmail();
            blocked = member->getIsBlocked();
        }
    }
    
    void execute() override {
        system->removeMemberDirect(memberId);
    }
    
    void undo() override {
        system->addMemberWithId(memberId, name, surname, phone, blocked, email);
    }
    
    std::string getDescription() const override {
        return "Удалить абонента: " + name + " " + surname;
    }
};

// Команда редактирования абонента
class EditMemberCommand : public Command {
private:
    LibrarySystem* system;
    int memberId;
    std::string oldName;
    std::string oldSurname;
    std::string oldPhone;
    std::string oldEmail;
    std::string newName;
    std::string newSurname;
    std::string newPhone;
    std::string newEmail;
    
public:
    EditMemberCommand(LibrarySystem* sys, int id, std::string_view n, std::string_view s, std::string_view p, std::string_view e = "")
        : system(sys), memberId(id), newName(n), newSurname(s), newPhone(p), newEmail(e) {
        const LibraryMember* member = sys->findMember(id);
        if (member) {
            oldName = member->getName();
            oldSurname = member->getSurname();
            oldPhone = member->getPhone();
            oldEmail = member->getEmail();
        }
    }
    
    void execute() override {
        system->editMemberDirect(memberId, newName, newSurname, newPhone, newEmail);
    }
    
    void undo() override {
        system->editMemberDirect(memberId, oldName, oldSurname, oldPhone, oldEmail);
    }
    
    std::string getDescription() const override {
        return "Редактировать абонента: " + newName + " " + newSurname;
    }
};

// Команда блокировки/разблокировки абонента
class BlockMemberCommand : public Command {
private:
    LibrarySystem* system;
    int memberId;
    bool block;
    
public:
    BlockMemberCommand(LibrarySystem* sys, int id, bool b) : system(sys), memberId(id), block(b) {}
    
    void execute() override {
        if (block) {
            system->blockMemberDirect(memberId);
        } else {
            system->unblockMemberDirect(memberId);
        }
    }
    
    void undo() override {
        if (block) {
            system->unblockMemberDirect(memberId);
        } else {
            system->blockMemberDirect(memberId);
        }
    }
    
    std::string getDescription() const override {
        return (block ? "Заблокировать" : "Разблокировать") + std::string(" абонента");
    }
};

// Команда добавления работника
class AddEmployeeCommand : public Command {
private:
    LibrarySystem* system;
    int employeeId;
    std::string name;
    std::string surname;
    std::string phone;
    double salary;
    int workHours;
    bool isLibrarian; // true для библиотекаря, false для менеджера
    
public:
    AddEmployeeCommand(LibrarySystem* sys, int id, std::string_view n, std::string_view s,
                      std::string_view p, double sal, int hours, bool librarian)
        : system(sys), employeeId(id), name(n), surname(s), phone(p),
          salary(sal), workHours(hours), isLibrarian(librarian) {}
    
    void execute() override {
        system->addEmployeeWithId(employeeId, name, surname, phone, salary, workHours, isLibrarian);
    }
    
    void undo() override {
        system->removeEmployeeDirect(employeeId);
    }
    
    std::string getDescription() const override {
        return "Добавить " + std::string(isLibrarian ? "библиотекаря" : "менеджера") + ": " + name + " " + surname;
    }
};

// Команда удаления работника
class RemoveEmployeeCommand : public Command {
private:
    LibrarySystem* system;
    int employeeId;
    std::string name;
    std::string surname;
    std::string phone;
    double salary;
    int workHours;
    bool isLibrarian;
    
public:
    RemoveEmployeeCommand(LibrarySystem* sys, int id) : system(sys), employeeId(id) {
        const Employee* emp = nullptr;
        for (const auto* e : sys->getAllEmployees()) {
            if (e->getId() == id) {
                emp = e;
                break;
            }
        }
        if (emp) {
            name = emp->getName();
            surname = emp->getSurname();
            phone = emp->getPhone();
            salary = emp->getSalary();
            workHours = emp->getWorkHours();
            isLibrarian = (emp->getType() == "Librarian");
        }
    }
    
    void execute() override {
        system->removeEmployeeDirect(employeeId);
    }
    
    void undo() override {
        system->addEmployeeWithId(employeeId, name, surname, phone, salary, workHours, isLibrarian);
    }
    
    std::string getDescription() const override {
        return "Удалить работника: " + name + " " + surname;
    }
};

// Команда редактирования работника
class EditEmployeeCommand : public Command {
private:
    LibrarySystem* system;
    int employeeId;
    std::string oldName;
    std::string oldSurname;
    std::string oldPhone;
    std::string newName;
    std::string newSurname;
    std::string newPhone;
    double oldSalary;
    double newSalary;
    int oldWorkHours;
    int newWorkHours;
    bool isLibrarian;
    
public:
    EditEmployeeCommand(LibrarySystem* sys, int id, std::string_view n, std::string_view s,
                        std::string_view p, double sal, int hours)
        : system(sys), employeeId(id), newName(n), newSurname(s), newPhone(p),
          newSalary(sal), newWorkHours(hours) {
        const Employee* emp = nullptr;
        for (const auto* e : sys->getAllEmployees()) {
            if (e->getId() == id) {
                emp = e;
                break;
            }
        }
        if (emp) {
            oldName = emp->getName();
            oldSurname = emp->getSurname();
            oldPhone = emp->getPhone();
            oldSalary = emp->getSalary();
            oldWorkHours = emp->getWorkHours();
            isLibrarian = (emp->getType() == "Librarian");
        }
    }
    
    void execute() override {
        system->editEmployeeDirect(employeeId, newName, newSurname, newPhone, newSalary, newWorkHours);
    }
    
    void undo() override {
        system->editEmployeeDirect(employeeId, oldName, oldSurname, oldPhone, oldSalary, oldWorkHours);
    }
    
    std::string getDescription() const override {
        return "Редактировать работника: " + newName + " " + newSurname;
    }
};

#endif // COMMANDS_H

