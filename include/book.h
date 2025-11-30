#ifndef BOOK_H
#define BOOK_H

#include "item.h"
#include <string>
#include <string_view>

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
    explicit Book(int pId, std::string_view pTitle, std::string_view pAuthor, 
         std::string_view pIsbn, int pYear, std::string_view pGenre, 
         std::string_view pCoverPath = "", int pQuantity = 1,
         std::string_view pDescription = "", std::string_view pPdfPath = "");
    
    std::string getAuthor() const { return author; }
    std::string getIsbn() const { return isbn; }
    int getYear() const { return year; }
    std::string getGenre() const { return genre; }
    std::string getCoverPath() const { return coverPath; }
    int getQuantity() const { return quantity; }
    std::string getDescription() const { return description; }
    std::string getPdfPath() const { return pdfPath; }
    
    void setAuthor(std::string_view pAuthor) { author = pAuthor; }
    void setIsbn(std::string_view pIsbn) { isbn = pIsbn; }
    void setYear(int pYear) { year = pYear; }
    void setGenre(std::string_view pGenre) { genre = pGenre; }
    void setCoverPath(std::string_view pCoverPath) { coverPath = pCoverPath; }
    void setQuantity(int pQuantity) { quantity = pQuantity; }
    void setDescription(std::string_view pDescription) { description = pDescription; }
    void setPdfPath(std::string_view pPdfPath) { pdfPath = pPdfPath; }
    bool getManuallyDisabled() const { return manuallyDisabled; }
    void setManuallyDisabled(bool disabled) { this->manuallyDisabled = disabled; }
    
    std::string getInfo() const override;
    std::string getType() const override { return "Book"; }
};

#endif // BOOK_H

