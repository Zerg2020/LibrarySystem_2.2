#include "librarymember.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "exceptions.h"

LibraryMember::LibraryMember(int pId, const std::string& pName, const std::string& pSurname, const std::string& pPhone, const std::string& pEmail)
    : Person(pId, pName, pSurname, pPhone, pEmail) {
}

void LibraryMember::borrowBook(int bookId, int employeeId) {
    if (isBlocked) {
        throw LibraryException("Абонент заблокирован");
    }
    
    // Проверка на дубликаты
    for (const auto& book : borrowedBooks) {
        if (book.bookId == bookId && !book.returned) {
            throw DuplicateException("Книга уже взята");
        }
    }
    
    time_t now = time(0);
    struct tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    localtime_r(&now, &timeinfo);
#endif
    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d");
    
    // Рассчитываем дату возврата (30 дней)
    time_t returnTime = now + (30 * 24 * 60 * 60);
    struct tm returnTimeinfo;
#ifdef _WIN32
    localtime_s(&returnTimeinfo, &returnTime);
#else
    localtime_r(&returnTime, &returnTimeinfo);
#endif
    std::ostringstream returnOss;
    returnOss << std::put_time(&returnTimeinfo, "%Y-%m-%d");
    
    borrowedBooks.emplace_back(bookId, oss.str(), returnOss.str(), employeeId);
}

void LibraryMember::borrowBookWithDate(int bookId, const std::string& borrowDate, const std::string& returnDate, bool returned, int employeeId) {
    BorrowedBook book(bookId, borrowDate, returnDate, employeeId);
    book.returned = returned;
    borrowedBooks.push_back(book);
}

void LibraryMember::returnBook(int bookId) {
    bool found = false;
    for (auto& book : borrowedBooks) {
        if (book.bookId == bookId && !book.returned) {
            time_t now = time(0);
            struct tm timeinfo;
#ifdef _WIN32
            localtime_s(&timeinfo, &now);
#else
            localtime_r(&now, &timeinfo);
#endif
            std::ostringstream oss;
            oss << std::put_time(&timeinfo, "%Y-%m-%d");
            book.returnDate = oss.str();
            book.returned = true;
            found = true;
            break;
        }
    }
    if (!found) {
        throw NotFoundException("Взятая книга не найдена");
    }
}

std::vector<BorrowedBook> LibraryMember::getOverdueBooks() const {
    std::vector<BorrowedBook> overdue;
    time_t now = time(0);
    
    for (const auto& book : borrowedBooks) {
        if (!book.returned && !book.returnDate.empty()) {
            // Парсим дату возврата
            struct tm returnTime = {};
            std::istringstream iss(book.returnDate);
            iss >> std::get_time(&returnTime, "%Y-%m-%d");
            time_t returnTimeT = mktime(&returnTime);
            
            if (now > returnTimeT) {
                overdue.push_back(book);
            }
        }
    }
    return overdue;
}

std::string LibraryMember::getInfo() const {
    std::ostringstream oss;
    oss << "ID: " << id << ", " << getFullName() 
        << ", Телефон: " << phone 
        << ", Заблокирован: " << (isBlocked ? "Да" : "Нет")
        << ", Взято книг: " << borrowedBooks.size();
    return oss.str();
}

