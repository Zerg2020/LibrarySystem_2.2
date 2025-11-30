#ifndef LIBRARYMEMBER_H
#define LIBRARYMEMBER_H

#include "person.h"
#include <vector>
#include <string>
#include <string_view>
#include <ctime>

struct BorrowedBook {
    int bookId;
    std::string borrowDate;
    std::string returnDate;
    bool returned = false;
    int employeeId; // ID работника, который выдал книгу

    BorrowedBook(int bookId, std::string_view borrowDate, std::string_view returnDate = "", int employeeId = 0)
        : bookId(bookId), borrowDate(borrowDate), returnDate(returnDate), employeeId(employeeId) {}
};

class LibraryMember : public Person {
private:
    bool isBlocked = false;
    std::vector<BorrowedBook> borrowedBooks;

public:
    explicit LibraryMember(int pId, std::string_view pName, std::string_view pSurname, std::string_view pPhone, std::string_view pEmail = "");
    
    bool getIsBlocked() const { return isBlocked; }
    void setBlocked(bool pBlocked) { isBlocked = pBlocked; }
    
    void borrowBook(int bookId, int employeeId = 0);
    void borrowBookWithDate(int bookId, std::string_view borrowDate, std::string_view returnDate, bool returned, int employeeId = 0);
    void returnBook(int bookId);
    std::vector<BorrowedBook> getBorrowedBooks() const { return borrowedBooks; }
    std::vector<BorrowedBook> getOverdueBooks() const;
    
    std::string getInfo() const override;
    std::string getType() const override { return "LibraryMember"; }
};

#endif // LIBRARYMEMBER_H

