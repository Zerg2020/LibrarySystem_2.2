#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "librarysystem.h"
#include <string>
#include <fstream>

class FileManager {
public:
    static void saveLibrarySystem(LibrarySystem& system, const std::string& basePath);
    static void loadLibrarySystem(LibrarySystem& system, const std::string& basePath);
    
private:
    static void saveBooks(LibrarySystem& system, const std::string& filename);
    static void loadBooks(LibrarySystem& system, const std::string& filename);
    
    static void saveMembers(LibrarySystem& system, const std::string& filename);
    static void loadMembers(LibrarySystem& system, const std::string& filename);
    
    static void saveEmployees(LibrarySystem& system, const std::string& filename);
    static void loadEmployees(LibrarySystem& system, const std::string& filename);
    
    static void saveMetadata(LibrarySystem& system, const std::string& filename);
    static void loadMetadata(LibrarySystem& system, const std::string& filename);
    
    static void loadBorrowedBooks(LibrarySystem& system, const std::string& filename);
    
    static std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif // FILEMANAGER_H

