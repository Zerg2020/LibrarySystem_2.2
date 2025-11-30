#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QLabel>
#include <QTabWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMap>
#include "librarysystem.h"
#include "filemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) final;

private slots:
    void onAddBook();
    void onRemoveBook();
    void onEditBook();
    void onShowBookDetails(int bookId);
    void onAddMember();
    void onRemoveMember();
    void onEditMember();
    void onBlockMember();
    void onUnblockMember();
    void onBorrowBook();
    void onReturnBook();
    void onShowMemberBooks();
    void onShowOverdueBooks();
    void onSearchMember();
    void onShowMemberDetails(int memberId);
    void onAddEmployee();
    void onAddLibrarian();
    void onAddManager();
    void onEditEmployee();
    void onShowEmployees();
    void onUndo();
    void onRedo();
    void onUndoBooks();
    void onRedoBooks();
    void onUndoMembers();
    void onRedoMembers();
    void onUndoEmployees();
    void onRedoEmployees();
    void onSave();
    void onLoad();
    void refreshBooks();
    void refreshMembers();
    void refreshEmployees();
    void onSearchBooks(const QString& text); // Устаревший метод, оставлен для совместимости
    void onFilterChanged(); // Обработчик изменения любого фильтра книг
    void onClearFilters(); // Очистка всех фильтров книг
    void onMemberFilterChanged(); // Обработчик изменения любого фильтра абонентов
    void onClearMemberFilters(); // Очистка всех фильтров абонентов
    void onBookHeaderClicked(int column);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    LibrarySystem librarySystem;
    QString dataPath = "data";
    QMap<int, int> bookSortStates; // колонка -> состояние (0=неактивна, 1=возрастание, 2=убывание)
    
    // Фильтры для книг
    struct BookFilters {
        QString title;
        QString author;
        QString genre;
        QString isbn;
        int yearFrom = 0;
        int yearTo = 0;
        int availability = -1; // -1 = все, 0 = нет, 1 = да
    };
    BookFilters bookFilters{};
    
    // Фильтры для абонентов
    struct MemberFilters {
        QString name;
        QString surname;
        QString phone;
        QString email;
        int blocked = -1; // -1 = все, 0 = не заблокирован, 1 = заблокирован
    };
    MemberFilters memberFilters{};
    
    void setupUI();
    void setupMenu();
    void setupBooksTab();
    void setupMembersTab();
    void setupEmployeesTab();
    void setupOperationsTab();
    void showError(const QString& message);
    void showInfo(const QString& message);
    void autoSave() const; // Автоматическое сохранение
    void updateUndoRedoButtons() const; // Обновление состояния кнопок undo/redo во всех вкладках
    void applyBookSorting(QTableWidget* table) const; // Применение сортировки книг
    QIcon createRedCrossIcon() const; // Создание красной иконки крестика
    
    // Вспомогательные методы для загрузки/сохранения данных (устранение дублирования кода)
    void loadDataSilently(); // Загрузка данных без сообщений об ошибках
    void saveDataSilently(); // Сохранение данных без сообщений об ошибках
    void saveDataWithWarning(); // Сохранение данных с предупреждением при ошибке
};

#endif // MAINWINDOW_H

