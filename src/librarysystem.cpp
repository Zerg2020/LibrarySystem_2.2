#include "librarysystem.h"
#include "exceptions.h"
#include "commands.h"
#include <algorithm>

LibrarySystem::LibrarySystem() = default;

void LibrarySystem::addBook(std::string_view title, std::string_view author,
                            std::string_view isbn, int year, std::string_view genre,
                            std::string_view coverPath, int quantity,
                            std::string_view description, std::string_view pdfPath) {
    try {
        int id = nextBookId;
        nextBookId++;
        auto command = std::make_unique<AddBookCommand>(this, id, title, author, isbn, year, genre, coverPath, quantity, description, pdfPath);
        commandManagerBooks.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::editBook(int id, std::string_view title, std::string_view author,
                             std::string_view isbn, int year, std::string_view genre,
                             std::string_view coverPath, int quantity,
                             std::string_view description, std::string_view pdfPath) {
    const Book* book = findBook(id);
    if (!book) {
        throw NotFoundException("Книга с ID " + std::to_string(id));
    }
    
    // Проверяем, не дублируется ли ISBN у другой книги
    if (const Book* existingBook = books.findBookByIsbn(isbn); existingBook && existingBook->getId() != id) {
        throw DuplicateException("Книга с ISBN " + std::string(isbn) + " уже существует");
    }
    
    try {
        auto command = std::make_unique<EditBookCommand>(this, id, title, author, isbn, year, genre, coverPath, quantity, description, pdfPath);
        commandManagerBooks.executeCommand(std::move(command));
        
        // Обновляем доступность после изменения количества (если не заблокировано вручную)
        if (!book->getManuallyDisabled()) {
            updateBookAvailability(id);
        }
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::removeBook(int id) {
    try {
        auto command = std::make_unique<RemoveBookCommand>(this, id);
        commandManagerBooks.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

std::vector<Book*> LibrarySystem::getAllBooks() const {
    return books.getAllBooks();
}

std::vector<Book*> LibrarySystem::getAvailableBooks() const {
    std::vector<Book*> result;
    for (auto* book : books.getAllBooks()) {
        // Проверяем доступность с учетом количества экземпляров и ручной блокировки
        if (book->getManuallyDisabled()) {
            continue; // Пропускаем книги, заблокированные вручную
        }
        // Проверяем, есть ли доступные экземпляры
        if (book->getQuantity() > 0) {
            result.push_back(book);
        }
    }
    return result;
}

Book* LibrarySystem::findBook(int id) {
    return books.findBook(id);
}

const Book* LibrarySystem::findBook(int id) const {
    return books.findBook(id);
}

int LibrarySystem::addMember(std::string_view name, std::string_view surname, std::string_view phone, std::string_view email) {
    try {
        int id = members.generateId();
        auto command = std::make_unique<AddMemberCommand>(this, id, name, surname, phone, email);
        commandManagerMembers.executeCommand(std::move(command));
        return id;
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::editMember(int id, std::string_view name, std::string_view surname, std::string_view phone, std::string_view email) {
    if (const LibraryMember* member = findMember(id); !member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    
    try {
        auto command = std::make_unique<EditMemberCommand>(this, id, name, surname, phone, email);
        commandManagerMembers.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::removeMember(int id) {
    try {
        auto command = std::make_unique<RemoveMemberCommand>(this, id);
        commandManagerMembers.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::blockMember(int id) {
    if (const LibraryMember* member = findMember(id); !member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    try {
        auto command = std::make_unique<BlockMemberCommand>(this, id, true);
        commandManagerMembers.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::unblockMember(int id) {
    if (const LibraryMember* member = findMember(id); !member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    try {
        auto command = std::make_unique<BlockMemberCommand>(this, id, false);
        commandManagerMembers.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

LibraryMember* LibrarySystem::findMember(int id) const {
    return members.findMember(id);
}

std::vector<LibraryMember*> LibrarySystem::getAllMembers() const {
    return members.getAllMembers();
}

std::vector<LibraryMember*> LibrarySystem::getBlockedMembers() const {
    return members.getBlockedMembers();
}

void LibrarySystem::borrowBook(int memberId, int bookId, int employeeId) {
    LibraryMember* member = findMember(memberId);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(memberId));
    }
    
    Book* book = findBook(bookId);
    if (!book) {
        throw NotFoundException("Книга с ID " + std::to_string(bookId));
    }
    
    // Проверяем доступность на основе количества экземпляров
    if (book->getManuallyDisabled() || book->getQuantity() <= 0) {
        throw LibraryException("Book is not available");
    }
    
    // Проверяем существование работника
    const Employee* employee = nullptr;
    for (const auto* emp : getAllEmployees()) {
        if (emp->getId() == employeeId) {
            employee = emp;
            break;
        }
    }
    if (!employee) {
        throw NotFoundException("Работник с ID " + std::to_string(employeeId));
    }
    
    try {
        member->borrowBook(bookId, employeeId);
        // Уменьшаем количество экземпляров при выдаче
        book->setQuantity(book->getQuantity() - 1);
        // Обновляем доступность после выдачи
        updateBookAvailability(bookId);
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::returnBook(int memberId, int bookId) {
    LibraryMember* member = findMember(memberId);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(memberId));
    }
    
    Book* book = findBook(bookId);
    if (!book) {
        throw NotFoundException("Книга с ID " + std::to_string(bookId));
    }
    
    try {
        member->returnBook(bookId);
        // Увеличиваем количество экземпляров при возврате
        book->setQuantity(book->getQuantity() + 1);
        // Обновляем доступность после возврата
        updateBookAvailability(bookId);
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

std::vector<BorrowedBook> LibrarySystem::getMemberBooks(int memberId) const {
    const LibraryMember* member = members.findMember(memberId);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(memberId));
    }
    return member->getBorrowedBooks();
}

std::vector<std::pair<const LibraryMember*, BorrowedBook>> LibrarySystem::getOverdueBooks() const {
    std::vector<std::pair<const LibraryMember*, BorrowedBook>> overdue;
    for (const auto* member : getAllMembers()) {
        auto memberOverdue = member->getOverdueBooks();
        for (const auto& book : memberOverdue) {
            overdue.emplace_back(member, book);
        }
    }
    return overdue;
}

void LibrarySystem::addLibrarian(std::string_view name, std::string_view surname,
                                  std::string_view phone, double salary, int workHours) {
    try {
        int id = nextEmployeeId;
        nextEmployeeId++;
        auto command = std::make_unique<AddEmployeeCommand>(this, id, name, surname, phone, salary, workHours, true);
        commandManagerEmployees.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::addManager(std::string_view name, std::string_view surname,
                                std::string_view phone, double salary, int workHours) {
    try {
        int id = nextEmployeeId;
        nextEmployeeId++;
        auto command = std::make_unique<AddEmployeeCommand>(this, id, name, surname, phone, salary, workHours, false);
        commandManagerEmployees.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::editEmployee(int id, std::string_view name, std::string_view surname,
                                 std::string_view phone, double salary, int workHours) {
    const Employee* emp = nullptr;
    for (const auto* e : getAllEmployees()) {
        if (e->getId() == id) {
            emp = e;
            break;
        }
    }
    if (!emp) {
        throw NotFoundException("Работник с ID " + std::to_string(id));
    }
    
    try {
        auto command = std::make_unique<EditEmployeeCommand>(this, id, name, surname, phone, salary, workHours);
        commandManagerEmployees.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

void LibrarySystem::removeEmployee(int id) {
    const Employee* emp = nullptr;
    for (const auto* e : getAllEmployees()) {
        if (e->getId() == id) {
            emp = e;
            break;
        }
    }
    if (!emp) {
        throw NotFoundException("Работник с ID " + std::to_string(id));
    }
    
    try {
        auto command = std::make_unique<RemoveEmployeeCommand>(this, id);
        commandManagerEmployees.executeCommand(std::move(command));
    } catch (const LibraryException&) {
        // Re-throw to let caller handle the exception
        throw;
    }
}

std::vector<Employee*> LibrarySystem::getAllEmployees() const {
    std::vector<Employee*> result;
    for (const auto& emp : employees) {
        result.push_back(emp.get());
    }
    return result;
}

// Undo/Redo для книг
void LibrarySystem::undoBooks() {
    commandManagerBooks.undo();
}

void LibrarySystem::redoBooks() {
    commandManagerBooks.redo();
}

bool LibrarySystem::canUndoBooks() const {
    return commandManagerBooks.canUndo();
}

bool LibrarySystem::canRedoBooks() const {
    return commandManagerBooks.canRedo();
}

// Undo/Redo для абонентов
void LibrarySystem::undoMembers() {
    commandManagerMembers.undo();
}

void LibrarySystem::redoMembers() {
    commandManagerMembers.redo();
}

bool LibrarySystem::canUndoMembers() const {
    return commandManagerMembers.canUndo();
}

bool LibrarySystem::canRedoMembers() const {
    return commandManagerMembers.canRedo();
}

// Undo/Redo для работников
void LibrarySystem::undoEmployees() {
    commandManagerEmployees.undo();
}

void LibrarySystem::redoEmployees() {
    commandManagerEmployees.redo();
}

bool LibrarySystem::canUndoEmployees() const {
    return commandManagerEmployees.canUndo();
}

bool LibrarySystem::canRedoEmployees() const {
    return commandManagerEmployees.canRedo();
}

void LibrarySystem::addBookWithId(int id, std::string_view title, std::string_view author,
                                   std::string_view isbn, int year, std::string_view genre, bool available,
                                   std::string_view coverPath, int quantity,
                                   std::string_view description, std::string_view pdfPath) {
    auto book = std::make_unique<Book>(id, title, author, isbn, year, genre, coverPath, quantity, description, pdfPath);
    book->setAvailable(available);
    if (id >= nextBookId) {
        nextBookId = id + 1;
    }
    books.addBook(std::move(book));
}

void LibrarySystem::removeBookDirect(int id) {
    books.removeBook(id);
}

void LibrarySystem::editBookDirect(int id, std::string_view title, std::string_view author,
                                   std::string_view isbn, int year, std::string_view genre,
                                   std::string_view coverPath, int quantity,
                                   std::string_view description, std::string_view pdfPath) {
    Book* book = findBook(id);
    if (!book) {
        throw NotFoundException("Книга с ID " + std::to_string(id));
    }
    
    book->setTitle(title);
    book->setAuthor(author);
    book->setIsbn(isbn);
    book->setYear(year);
    book->setGenre(genre);
    book->setCoverPath(coverPath);
    book->setQuantity(quantity);
    book->setDescription(description);
    book->setPdfPath(pdfPath);
}

void LibrarySystem::addMemberWithId(int id, std::string_view name, std::string_view surname, 
                                     std::string_view phone, bool blocked, std::string_view email) {
    auto member = std::make_unique<LibraryMember>(id, name, surname, phone, email);
    member->setBlocked(blocked);
    if (id >= members.getNextId()) {
        members.setNextId(id + 1);
    }
    members.addMember(std::move(member));
}

void LibrarySystem::removeMemberDirect(int id) {
    members.removeMember(id);
}

void LibrarySystem::editMemberDirect(int id, std::string_view name, std::string_view surname, std::string_view phone, std::string_view email) const {
    LibraryMember* member = findMember(id);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    member->setName(name);
    member->setSurname(surname);
    member->setPhone(phone);
    member->setEmail(email);
}

void LibrarySystem::blockMemberDirect(int id) const {
    LibraryMember* member = findMember(id);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    member->setBlocked(true);
}

void LibrarySystem::unblockMemberDirect(int id) const {
    LibraryMember* member = findMember(id);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
    member->setBlocked(false);
}

void LibrarySystem::addEmployeeWithId(int id, std::string_view name, std::string_view surname,
                                      std::string_view phone, double salary, int workHours, bool isLibrarian) {
    if (isLibrarian) {
        employees.emplace_back(std::make_unique<Librarian>(id, name, surname, phone, salary, workHours));
    } else {
        employees.emplace_back(std::make_unique<Manager>(id, name, surname, phone, salary, workHours));
    }
    if (id >= nextEmployeeId) {
        nextEmployeeId = id + 1;
    }
}

void LibrarySystem::removeEmployeeDirect(int id) {
    auto it = std::find_if(employees.begin(), employees.end(),
                          [id](const std::unique_ptr<Employee>& emp) {
                              return emp->getId() == id;
                          });
    if (it != employees.end()) {
        employees.erase(it);
    } else {
        throw NotFoundException("Работник с ID " + std::to_string(id));
    }
}

void LibrarySystem::editEmployeeDirect(int id, std::string_view name, std::string_view surname,
                                       std::string_view phone, double salary, int workHours) const {
    Employee* emp = nullptr;
    for (const auto& e : employees) {
        if (e->getId() == id) {
            emp = e.get();
            break;
        }
    }
    if (!emp) {
        throw NotFoundException("Работник с ID " + std::to_string(id));
    }
    
    emp->setName(name);
    emp->setSurname(surname);
    emp->setPhone(phone);
    emp->setSalary(salary);
    emp->setWorkHours(workHours);
}

void LibrarySystem::addBorrowedBook(int memberId, int bookId, std::string_view borrowDate, 
                                    std::string_view returnDate, bool returned, int employeeId) {
    LibraryMember* member = findMember(memberId);
    if (!member) {
        throw NotFoundException("Абонент с ID " + std::to_string(memberId));
    }
    
    if (const Book* book = findBook(bookId); !book) {
        throw NotFoundException("Книга с ID " + std::to_string(bookId));
    }
    
    member->borrowBookWithDate(bookId, borrowDate, returnDate, returned, employeeId);
    // Обновляем доступность после загрузки
    updateBookAvailability(bookId);
}

int LibrarySystem::getBorrowedCount(int bookId) const {
    int count = 0;
    for (const auto* member : getAllMembers()) {
        auto borrowedBooks = member->getBorrowedBooks();
        for (const auto& borrowed : borrowedBooks) {
            if (borrowed.bookId == bookId && !borrowed.returned) {
                count++;
            }
        }
    }
    return count;
}

void LibrarySystem::updateBookAvailability(int bookId) {
    Book* book = findBook(bookId);
    if (!book) return;
    
    // Если книга заблокирована вручную, она недоступна
    if (book->getManuallyDisabled()) {
        book->setAvailable(false);
        return;
    }
    
    // Иначе проверяем количество доступных экземпляров
    book->setAvailable(book->getQuantity() > 0);
}

