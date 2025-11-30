#include "book.h"
#include <sstream>

Book::Book(int pId, std::string_view pTitle, std::string_view pAuthor,
           std::string_view pIsbn, int pYear, std::string_view pGenre, 
           std::string_view pCoverPath, int pQuantity,
           std::string_view pDescription, std::string_view pPdfPath)
    : Item(pId, pTitle), author(pAuthor), isbn(pIsbn), year(pYear), genre(pGenre), 
      coverPath(pCoverPath), quantity(pQuantity), description(pDescription), 
      pdfPath(pPdfPath) {
}

std::string Book::getInfo() const {
    std::ostringstream oss;
    oss << "ID: " << id << ", " << title << " by " << author 
        << ", ISBN: " << isbn << ", Year: " << year 
        << ", Genre: " << genre 
        << ", Available: " << (available ? "Yes" : "No");
    return oss.str();
}

