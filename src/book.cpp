#include "book.h"
#include <sstream>

Book::Book(int pId, const std::string& pTitle, const std::string& pAuthor,
           const std::string& pIsbn, int pYear, const std::string& pGenre, 
           const std::string& pCoverPath, int pQuantity,
           const std::string& pDescription, const std::string& pPdfPath)
    : Item(pId, pTitle), author(pAuthor), isbn(pIsbn), year(pYear), genre(pGenre), 
      coverPath(pCoverPath), quantity(pQuantity), description(pDescription), 
      pdfPath(pPdfPath), manuallyDisabled(false) {
}

std::string Book::getInfo() const {
    std::ostringstream oss;
    oss << "ID: " << id << ", " << title << " by " << author 
        << ", ISBN: " << isbn << ", Year: " << year 
        << ", Genre: " << genre 
        << ", Available: " << (available ? "Yes" : "No");
    return oss.str();
}

