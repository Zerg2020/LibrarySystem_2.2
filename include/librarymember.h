#ifndef LIBRARYMEMBER_H
#define LIBRARYMEMBER_H

#include "person.h"
#include <vector>
#include <string>
#include <ctime>

struct BorrowedBook {
    int bookId;
    std::string borrowDate;
    std::string returnDate;
    bool returned = false;
    int employeeId; // ID работника, который выдал книгу

    BorrowedBook(int bookId, const std::string& borrowDate, const std::string& returnDate = "", int employeeId = 0)
        : bookId(bookId), borrowDate(borrowDate), returnDate(returnDate), employeeId(employeeId) {}
};

class LibraryMember : public Person {
private:
    bool isBlocked = false;
    std::vector<BorrowedBook> borrowedBooks;

public:
    explicit LibraryMember(int pId, const std::string& pName, const std::string& pSurname, const std::string& pPhone, const std::string& pEmail = "");
    
    bool getIsBlocked() const { return isBlocked; }
    void setBlocked(bool pBlocked) { isBlocked = pBlocked; }
    
    void borrowBook(int bookId, int employeeId = 0);
    void borrowBookWithDate(int bookId, const std::string& borrowDate, const std::string& returnDate, bool returned, int employeeId = 0);
    void returnBook(int bookId);
    std::vector<BorrowedBook> getBorrowedBooks() const { return borrowedBooks; }
    std::vector<BorrowedBook> getOverdueBooks() const;
    
    std::string getInfo() const override;
    std::string getType() const override { return "LibraryMember"; }
};

#endif // LIBRARYMEMBER_H

