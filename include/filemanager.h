#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "librarysystem.h"
#include <string>
#include <string_view>
#include <fstream>

class FileManager {
public:
    static void saveLibrarySystem(const LibrarySystem& system, std::string_view basePath);
    static void loadLibrarySystem(LibrarySystem& system, std::string_view basePath);
    
private:
    static void saveBooks(const LibrarySystem& system, std::string_view filename);
    static void loadBooks(LibrarySystem& system, std::string_view filename);
    
    static void saveMembers(const LibrarySystem& system, std::string_view filename);
    static void loadMembers(LibrarySystem& system, std::string_view filename);
    
    static void saveEmployees(const LibrarySystem& system, std::string_view filename);
    static void loadEmployees(LibrarySystem& system, std::string_view filename);
    
    static void saveMetadata(const LibrarySystem& system, std::string_view filename);
    static void loadMetadata(LibrarySystem& system, std::string_view filename);
    
    static void loadBorrowedBooks(LibrarySystem& system, std::string_view filename);
    
    static std::vector<std::string> split(std::string_view str, char delimiter);
};

#endif // FILEMANAGER_H

