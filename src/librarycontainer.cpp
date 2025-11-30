#include "librarycontainer.h"
#include "exceptions.h"

void LibraryContainer::addBook(std::unique_ptr<Book> book) {
    if (findBookByIsbn(book->getIsbn()) != nullptr) {
        throw DuplicateException("Книга с ISBN " + book->getIsbn() + " уже существует");
    }
    books.push_back(std::move(book));
}

void LibraryContainer::removeBook(int id) {
    auto it = std::find_if(books.begin(), books.end(),
                          [id](const std::unique_ptr<Book>& book) {
                              return book->getId() == id;
                          });
    if (it != books.end()) {
        books.erase(it);
    } else {
        throw NotFoundException("Книга с ID " + std::to_string(id));
    }
}

Book* LibraryContainer::findBook(int id) {
    auto it = std::find_if(books.begin(), books.end(),
                          [id](const std::unique_ptr<Book>& book) {
                              return book->getId() == id;
                          });
    return (it != books.end()) ? it->get() : nullptr;
}

const Book* LibraryContainer::findBook(int id) const {
    auto it = std::find_if(books.begin(), books.end(),
                          [id](const std::unique_ptr<Book>& book) {
                              return book->getId() == id;
                          });
    return (it != books.end()) ? it->get() : nullptr;
}

Book* LibraryContainer::findBookByIsbn(const std::string& isbn) {
    auto it = std::find_if(books.begin(), books.end(),
                          [&isbn](const std::unique_ptr<Book>& book) {
                              return book->getIsbn() == isbn;
                          });
    return (it != books.end()) ? it->get() : nullptr;
}

const Book* LibraryContainer::findBookByIsbn(const std::string& isbn) const {
    auto it = std::find_if(books.begin(), books.end(),
                          [&isbn](const std::unique_ptr<Book>& book) {
                              return book->getIsbn() == isbn;
                          });
    return (it != books.end()) ? it->get() : nullptr;
}

std::vector<Book*> LibraryContainer::getAllBooks() const {
    std::vector<Book*> result;
    for (const auto& book : books) {
        result.push_back(book.get());
    }
    return result;
}

std::vector<Book*> LibraryContainer::getAvailableBooks() const {
    std::vector<Book*> result;
    for (const auto& book : books) {
        if (book->isAvailable()) {
            result.push_back(book.get());
        }
    }
    return result;
}

