#ifndef BOOK_H
#define BOOK_H

#include "item.h"
#include <string>

class Book : public Item {
private:
    std::string author;
    std::string isbn;
    int year;
    std::string genre;
    std::string coverPath; // Путь к обложке книги
    int quantity; // Количество экземпляров книги
    std::string description; // Описание книги
    std::string pdfPath; // Путь к PDF файлу книги
    bool manuallyDisabled = false; // Ручная блокировка доступности (переопределяет автоматическую логику)

public:
    explicit Book(int pId, const std::string& pTitle, const std::string& pAuthor, 
         const std::string& pIsbn, int pYear, const std::string& pGenre, 
         const std::string& pCoverPath = "", int pQuantity = 1,
         const std::string& pDescription = "", const std::string& pPdfPath = "");
    
    std::string getAuthor() const { return author; }
    std::string getIsbn() const { return isbn; }
    int getYear() const { return year; }
    std::string getGenre() const { return genre; }
    std::string getCoverPath() const { return coverPath; }
    int getQuantity() const { return quantity; }
    std::string getDescription() const { return description; }
    std::string getPdfPath() const { return pdfPath; }
    
    void setAuthor(const std::string& pAuthor) { author = pAuthor; }
    void setIsbn(const std::string& pIsbn) { isbn = pIsbn; }
    void setYear(int pYear) { year = pYear; }
    void setGenre(const std::string& pGenre) { genre = pGenre; }
    void setCoverPath(const std::string& pCoverPath) { coverPath = pCoverPath; }
    void setQuantity(int pQuantity) { quantity = pQuantity; }
    void setDescription(const std::string& pDescription) { description = pDescription; }
    void setPdfPath(const std::string& pPdfPath) { pdfPath = pPdfPath; }
    bool getManuallyDisabled() const { return manuallyDisabled; }
    void setManuallyDisabled(bool disabled) { this->manuallyDisabled = disabled; }
    
    std::string getInfo() const override;
    std::string getType() const override { return "Book"; }
};

#endif // BOOK_H

