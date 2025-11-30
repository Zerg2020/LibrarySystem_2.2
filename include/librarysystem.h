#ifndef LIBRARYSYSTEM_H
#define LIBRARYSYSTEM_H

#include "librarycontainer.h"
#include "membercontainer.h"
#include "commandmanager.h"
#include "book.h"
#include "librarymember.h"
#include "employee.h"
#include "librarian.h"
#include "manager.h"
#include <vector>
#include <memory>
#include <string_view>

class LibrarySystem {
private:
    LibraryContainer books;
    MemberContainer members;
    std::vector<std::unique_ptr<Employee>> employees;
    CommandManager commandManagerBooks;      // Для операций с книгами
    CommandManager commandManagerMembers;     // Для операций с абонентами
    CommandManager commandManagerEmployees;  // Для операций с работниками
    int nextBookId = 1;
    int nextEmployeeId = 1;

public:
    LibrarySystem();
    
    // Книги
    void addBook(std::string_view title, std::string_view author,
                 std::string_view isbn, int year, std::string_view genre,
                 std::string_view coverPath = "", int quantity = 1,
                 std::string_view description = "", std::string_view pdfPath = "");
    void editBook(int id, std::string_view title, std::string_view author,
                  std::string_view isbn, int year, std::string_view genre,
                  std::string_view coverPath = "", int quantity = 1,
                  std::string_view description = "", std::string_view pdfPath = "");
    void removeBook(int id);
    std::vector<Book*> getAllBooks() const;
    std::vector<Book*> getAvailableBooks() const;
    Book* findBook(int id);
    const Book* findBook(int id) const;
    int getBorrowedCount(int bookId) const; // Подсчет количества выданных экземпляров
    void updateBookAvailability(int bookId); // Обновление доступности на основе количества
    
    // Абоненты
    int addMember(std::string_view name, std::string_view surname, std::string_view phone, std::string_view email = "");
    void editMember(int id, std::string_view name, std::string_view surname, std::string_view phone, std::string_view email = "");
    void removeMember(int id);
    void blockMember(int id);
    void unblockMember(int id);
    LibraryMember* findMember(int id) const;
    std::vector<LibraryMember*> getAllMembers() const;
    std::vector<LibraryMember*> getBlockedMembers() const;
    
    // Выдача/возврат книг
    void borrowBook(int memberId, int bookId, int employeeId);
    void returnBook(int memberId, int bookId);
    std::vector<BorrowedBook> getMemberBooks(int memberId) const;
    std::vector<std::pair<const LibraryMember*, BorrowedBook>> getOverdueBooks() const;
    
    // Работники
    void addLibrarian(std::string_view name, std::string_view surname,
                      std::string_view phone, double salary, int workHours);
    void addManager(std::string_view name, std::string_view surname,
                    std::string_view phone, double salary, int workHours);
    void editEmployee(int id, std::string_view name, std::string_view surname,
                      std::string_view phone, double salary, int workHours);
    void removeEmployee(int id);
    std::vector<Employee*> getAllEmployees() const;
    
    // Undo/Redo для книг
    void undoBooks();
    void redoBooks();
    bool canUndoBooks() const;
    bool canRedoBooks() const;
    
    // Undo/Redo для абонентов
    void undoMembers();
    void redoMembers();
    bool canUndoMembers() const;
    bool canRedoMembers() const;
    
    // Undo/Redo для работников
    void undoEmployees();
    void redoEmployees();
    bool canUndoEmployees() const;
    bool canRedoEmployees() const;
    
    // Старые методы для совместимости (используют книги)
    void undo() { undoBooks(); }
    void redo() { redoBooks(); }
    bool canUndo() const { return canUndoBooks(); }
    bool canRedo() const { return canRedoBooks(); }
    
    CommandManager& getCommandManagerBooks() { return commandManagerBooks; }
    CommandManager& getCommandManagerMembers() { return commandManagerMembers; }
    CommandManager& getCommandManagerEmployees() { return commandManagerEmployees; }
    
    // Для сохранения/загрузки
    int getNextBookId() const { return nextBookId; }
    void setNextBookId(int id) { nextBookId = id; }
    int getNextEmployeeId() const { return nextEmployeeId; }
    void setNextEmployeeId(int id) { nextEmployeeId = id; }
    
    // Методы для загрузки данных и команд
    void addBookWithId(int id, std::string_view title, std::string_view author,
                       std::string_view isbn, int year, std::string_view genre, bool available,
                       std::string_view coverPath = "", int quantity = 1,
                       std::string_view description = "", std::string_view pdfPath = "");
    void removeBookDirect(int id); // Прямое удаление без команды (для команд undo/redo)
    void editBookDirect(int id, std::string_view title, std::string_view author,
                       std::string_view isbn, int year, std::string_view genre,
                       std::string_view coverPath, int quantity,
                       std::string_view description, std::string_view pdfPath); // Прямое редактирование без команды
    void addMemberWithId(int id, std::string_view name, std::string_view surname, 
                         std::string_view phone, bool blocked, std::string_view email = "");
    void removeMemberDirect(int id); // Прямое удаление без команды
    void editMemberDirect(int id, std::string_view name, std::string_view surname, std::string_view phone, std::string_view email = ""); // Прямое редактирование без команды
    void blockMemberDirect(int id); // Прямая блокировка без команды
    void unblockMemberDirect(int id); // Прямая разблокировка без команды
    void addEmployeeWithId(int id, std::string_view name, std::string_view surname,
                          std::string_view phone, double salary, int workHours, bool isLibrarian);
    void removeEmployeeDirect(int id); // Прямое удаление без команды
    void editEmployeeDirect(int id, std::string_view name, std::string_view surname,
                           std::string_view phone, double salary, int workHours); // Прямое редактирование без команды
    void addBorrowedBook(int memberId, int bookId, std::string_view borrowDate, 
                        std::string_view returnDate, bool returned, int employeeId = 0);
};

#endif // LIBRARYSYSTEM_H

