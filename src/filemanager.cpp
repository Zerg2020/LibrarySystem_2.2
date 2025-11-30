#include "filemanager.h"
#include "exceptions.h"
#include <sstream>
#include <iostream>

void FileManager::saveLibrarySystem(const LibrarySystem& system, std::string_view basePath) {
    try {
        std::string basePathStr(basePath);
        saveBooks(system, basePathStr + "/books.txt");
        saveMembers(system, basePathStr + "/members.txt");
        saveEmployees(system, basePathStr + "/employees.txt");
        saveMetadata(system, basePathStr + "/metadata.txt");
    } catch (const std::exception& e) {
        throw FileException("Не удалось сохранить систему библиотеки: " + std::string(e.what()));
    }
}

void FileManager::loadLibrarySystem(LibrarySystem& system, std::string_view basePath) {
    try {
        std::string basePathStr(basePath);
        // Сначала загружаем метаданные (ID счетчики)
        loadMetadata(system, basePathStr + "/metadata.txt");
        // Затем загружаем книги
        loadBooks(system, basePathStr + "/books.txt");
        // Затем загружаем абонентов (без взятых книг)
        loadMembers(system, basePathStr + "/members.txt");
        // Затем загружаем работников
        loadEmployees(system, basePathStr + "/employees.txt");
        // В конце загружаем взятые книги (нужно загружать после книг и абонентов)
        loadBorrowedBooks(system, basePathStr + "/members.txt");
        
        // Обновляем доступность всех книг после загрузки всех данных
        auto allBooks = system.getAllBooks();
        for (const auto* book : allBooks) {
            system.updateBookAvailability(book->getId());
        }
    } catch (const std::exception& e) {
        throw FileException("Не удалось загрузить систему библиотеки: " + std::string(e.what()));
    }
}

void FileManager::saveBooks(const LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ofstream file(filenameStr);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл: " + filename);
    }
    
    auto allBooks = system.getAllBooks();
    for (const auto* book : allBooks) {
        file << book->getId() << "|"
             << book->getTitle() << "|"
             << book->getAuthor() << "|"
             << book->getIsbn() << "|"
             << book->getYear() << "|"
             << book->getGenre() << "|"
             << (book->isAvailable() ? "1" : "0") << "|"
             << book->getCoverPath() << "|"
             << book->getQuantity() << "|"
             << book->getDescription() << "|"
             << book->getPdfPath() << "|"
             << (book->getManuallyDisabled() ? "1" : "0") << "\n";
    }
    file.close();
}

void FileManager::loadBooks(LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ifstream file(filenameStr);
    if (!file.is_open()) {
        return; // Файл может не существовать при первой загрузке
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto parts = split(line, '|');
        if (parts.size() >= 7) {
            int id = std::stoi(parts[0]);
            std::string title = parts[1];
            std::string author = parts[2];
            std::string isbn = parts[3];
            int year = std::stoi(parts[4]);
            std::string genre = parts[5];
            bool available = (parts[6] == "1");
            std::string coverPath = (parts.size() >= 8) ? parts[7] : "";
            int quantity = (parts.size() >= 9) ? std::stoi(parts[8]) : 1;
            std::string description = (parts.size() >= 10) ? parts[9] : "";
            std::string pdfPath = (parts.size() >= 11) ? parts[10] : "";
            bool manuallyDisabled = (parts.size() >= 12 && parts[11] == "1");
            
            system.addBookWithId(id, title, author, isbn, year, genre, available, coverPath, quantity, description, pdfPath);
            // Устанавливаем ручную блокировку после создания книги
            Book* book = system.findBook(id);
            if (book) {
                book->setManuallyDisabled(manuallyDisabled);
                // Обновляем доступность с учетом ручной блокировки
                system.updateBookAvailability(id);
            }
        }
    }
    file.close();
}

void FileManager::saveMembers(const LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ofstream file(filenameStr);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл: " + filename);
    }
    
    auto allMembers = system.getAllMembers();
    for (const auto* member : allMembers) {
        file << member->getId() << "|"
             << member->getName() << "|"
             << member->getSurname() << "|"
             << member->getPhone() << "|"
             << member->getEmail() << "|"
             << (member->getIsBlocked() ? "1" : "0") << "\n";
        
        // Сохраняем взятые книги (включая историю)
        auto borrowed = member->getBorrowedBooks();
        for (const auto& book : borrowed) {
            file << "BORROWED|" << member->getId() << "|"
                 << book.bookId << "|"
                 << book.borrowDate << "|"
                 << book.returnDate << "|"
                 << (book.returned ? "1" : "0") << "|"
                 << book.employeeId << "\n";
        }
    }
    file.close();
}

void FileManager::loadMembers(LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ifstream file(filenameStr);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Пропускаем строки с взятыми книгами - они загружаются отдельно
        if (line.find("BORROWED|") == 0) {
            continue;
        }
        
        auto parts = split(line, '|');
        if (parts.size() >= 5) {
            int id = std::stoi(parts[0]);
            std::string name = parts[1];
            std::string surname = parts[2];
            std::string phone = parts[3];
            std::string email = (parts.size() >= 6) ? parts[4] : "";
            bool blocked = (parts.size() >= 6) ? (parts[5] == "1") : (parts[4] == "1");
            
            system.addMemberWithId(id, name, surname, phone, blocked, email);
        }
    }
    file.close();
}

void FileManager::loadBorrowedBooks(LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ifstream file(filenameStr);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line.find("BORROWED|") == 0) {
            // Загрузка информации о взятых книгах (включая историю)
            auto parts = split(line, '|');
            if (parts.size() >= 6) {
                int memberId = std::stoi(parts[1]);
                int bookId = std::stoi(parts[2]);
                std::string borrowDate = parts[3];
                std::string returnDate = parts[4];
                bool returned = (parts[5] == "1");
                int employeeId = (parts.size() >= 7) ? std::stoi(parts[6]) : 0;
                
                try {
                    system.addBorrowedBook(memberId, bookId, borrowDate, returnDate, returned, employeeId);
                } catch (const LibraryException& e) {
                    // Игнорируем ошибки при загрузке взятых книг
                    (void)e; // Suppress unused variable warning
                }
            }
        }
    }
    file.close();
}

void FileManager::saveEmployees(const LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ofstream file(filenameStr);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл: " + filename);
    }
    
    auto allEmployees = system.getAllEmployees();
    for (const auto* emp : allEmployees) {
        file << emp->getId() << "|"
             << emp->getName() << "|"
             << emp->getSurname() << "|"
             << emp->getPhone() << "|"
             << emp->getPosition() << "|"
             << emp->getSalary() << "|"
             << emp->getWorkHours() << "|";
        
        if (emp->getType() == "Librarian") {
            const auto* lib = dynamic_cast<const Librarian*>(emp);
            if (lib) {
                file << lib->getBooksProcessed() << "\n";
            }
        } else if (emp->getType() == "Manager") {
            const auto* mgr = dynamic_cast<const Manager*>(emp);
            if (mgr) {
                file << mgr->getEmployeesManaged() << "\n";
            }
        }
    }
    file.close();
}

void FileManager::loadEmployees(LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ifstream file(filenameStr);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto parts = split(line, '|');
        if (parts.size() >= 8) {
            int id = std::stoi(parts[0]);
            std::string name = parts[1];
            std::string surname = parts[2];
            std::string phone = parts[3];
            std::string position = parts[4];
            double salary = std::stod(parts[5]);
            int workHours = std::stoi(parts[6]);
            
            bool isLibrarian = (position == "Librarian");
            system.addEmployeeWithId(id, name, surname, phone, salary, workHours, isLibrarian);
        }
    }
    file.close();
}

void FileManager::saveMetadata(const LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ofstream file(filenameStr);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл: " + filename);
    }
    
    file << "nextBookId=" << system.getNextBookId() << "\n";
    file << "nextEmployeeId=" << system.getNextEmployeeId() << "\n";
    file.close();
}

void FileManager::loadMetadata(LibrarySystem& system, std::string_view filename) {
    std::string filenameStr(filename);
    std::ifstream file(filenameStr);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            int value = std::stoi(line.substr(pos + 1));
            
            if (key == "nextBookId") {
                system.setNextBookId(value);
            } else if (key == "nextEmployeeId") {
                system.setNextEmployeeId(value);
            }
        }
    }
    file.close();
}

std::vector<std::string> FileManager::split(std::string_view str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::string strStr(str);
    std::istringstream tokenStream(strStr);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

