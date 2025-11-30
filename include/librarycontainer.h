#ifndef LIBRARYCONTAINER_H
#define LIBRARYCONTAINER_H

#include "book.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "exceptions.h"
#include <iterator>
#include <string_view>

class LibraryContainer {
private:
    std::vector<std::unique_ptr<Book>> books;

public:
    class Iterator {
    private:
        std::vector<std::unique_ptr<Book>>::iterator it;

    public:
        explicit Iterator(std::vector<std::unique_ptr<Book>>::iterator pIt) : it(pIt) {}
        
        Iterator& operator++() { ++it; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        
        friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.it == rhs.it; }
        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs.it != rhs.it; }
        
        Book& operator*() { return **it; }
        Book* operator->() { return it->get(); }
    };

    Iterator begin() { return Iterator(books.begin()); }
    Iterator end() { return Iterator(books.end()); }

    void addBook(std::unique_ptr<Book> book);
    void removeBook(int id);
    Book* findBook(int id);
    const Book* findBook(int id) const;
    Book* findBookByIsbn(std::string_view isbn);
    const Book* findBookByIsbn(std::string_view isbn) const;
    std::vector<Book*> getAllBooks() const;
    std::vector<Book*> getAvailableBooks() const;
    size_t size() const { return books.size(); }
    bool empty() const { return books.empty(); }
    
    // STL алгоритмы
    template<typename Predicate>
    std::vector<Book*> findBooks(Predicate pred) const {
        std::vector<Book*> result;
        std::copy_if(books.begin(), books.end(), std::back_inserter(result),
                    [&pred](const std::unique_ptr<Book>& book) {
                        return pred(book.get());
                    });
        return result;
    }
};

#endif // LIBRARYCONTAINER_H

