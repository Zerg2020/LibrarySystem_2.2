#include "../include/mainwindow.h"
#include "ui_mainwindow.h"
#include "../include/exceptions.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCloseEvent>
#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QFileInfo>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QCompleter>
#include <QTextBrowser>
#include <QIcon>
#include <QPainter>
#include <QColor>
#include <QMap>
#include <QToolBar>
#include <QRadioButton>
#include <QDate>
#include <QList>
#include <algorithm>
#include <sstream>
#include <stdexcept>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui = new Ui::MainWindow;
    ui->setupUi(this);
    
    // Устанавливаем размер главного окна
    setMinimumSize(1200, 700);
    resize(1400, 800);
    
    // Создаем папку для данных, если её нет
    if (QDir dir; !dir.exists(dataPath)) {
        dir.mkpath(dataPath);
    }
    
    setupMenu();
    setupUI();
    
    // Загружаем данные при старте, если они есть
    try {
        FileManager::loadLibrarySystem(librarySystem, dataPath.toStdString());
        refreshBooks();
        refreshMembers();
        refreshEmployees();
        updateUndoRedoButtons();
    } catch (const FileException& e) {
        // Игнорируем ошибки при первой загрузке (файлы могут не существовать)
        (void)e; // Suppress unused variable warning
    } catch (const std::exception& e) {
        // Игнорируем другие ошибки при первой загрузке
        (void)e; // Suppress unused variable warning
    }
}

MainWindow::~MainWindow()
{
    // Автоматически сохраняем данные при закрытии
    try {
        FileManager::saveLibrarySystem(librarySystem, dataPath.toStdString());
    } catch (const std::exception& e) {
        // Игнорируем ошибки сохранения при закрытии
        (void)e; // Suppress unused variable warning
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Сохраняем данные перед закрытием
    try {
        FileManager::saveLibrarySystem(librarySystem, dataPath.toStdString());
    } catch (const LibraryException& e) {
        QMessageBox::warning(this, "Предупреждение", 
                           QString("Не удалось сохранить данные: %1\n\nПриложение все равно будет закрыто.")
                           .arg(QString::fromStdString(e.what())));
    }
    event->accept();
}

void MainWindow::setupMenu()
{
    // Скрываем меню и создаем кнопки в toolbar
    ui->menubar->setVisible(false);
    
    // Создаем toolbar для кнопок Файл и Правка
    QToolBar* fileEditToolBar = addToolBar("Файл и Правка");
    fileEditToolBar->setMovable(false);
    
    // Кнопки Файл
    auto* saveBtn = new QPushButton("Сохранить", this);
    saveBtn->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    saveBtn->setToolTip("Сохранить данные (Ctrl+S)");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
    fileEditToolBar->addWidget(saveBtn);
    
    auto* loadBtn = new QPushButton("Загрузить", this);
    loadBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    loadBtn->setToolTip("Загрузить данные (Ctrl+O)");
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoad);
    fileEditToolBar->addWidget(loadBtn);
    
    fileEditToolBar->addSeparator();
    
    // Кнопки Правка
    auto* undoBtn = new QPushButton("Назад", this);
    undoBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    undoBtn->setToolTip("Отменить действие (Ctrl+Z)");
    undoBtn->setObjectName("undoButton");
    connect(undoBtn, &QPushButton::clicked, this, &MainWindow::onUndo);
    undoBtn->setEnabled(false);
    fileEditToolBar->addWidget(undoBtn);
    
    auto* redoBtn = new QPushButton("Вперед", this);
    redoBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    redoBtn->setToolTip("Повторить действие (Ctrl+Y)");
    redoBtn->setObjectName("redoButton");
    connect(redoBtn, &QPushButton::clicked, this, &MainWindow::onRedo);
    redoBtn->setEnabled(false);
    fileEditToolBar->addWidget(redoBtn);
    
}

void MainWindow::setupUI()
{
    // Создаем универсальную панель кнопок над вкладками
    auto* toolbarWidget = new QWidget(ui->centralwidget);
    toolbarWidget->setObjectName("toolbarWidget");
    auto* toolbarLayout = new QVBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(8, 6, 8, 6);
    toolbarLayout->setSpacing(6);
    
    // Кнопка "Добавить" в панели сверху (растянута на всю ширину, меняется в зависимости от вкладки)
    auto* addButton = new QPushButton("Добавить книгу", toolbarWidget);
    addButton->setObjectName("addButton");
    addButton->setMinimumHeight(32);
    addButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    addButton->setStyleSheet("QPushButton { font-size: 11pt; font-weight: bold; padding: 6px; }");
    toolbarLayout->addWidget(addButton);
    
    // Размещаем панель в layout центрального виджета над вкладками
    if (QVBoxLayout* centralLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout()); centralLayout) {
        centralLayout->insertWidget(0, toolbarWidget);
    }
    
    setupBooksTab();
    setupMembersTab();
    setupEmployeesTab();
    setupOperationsTab();
    
    // Подключаем сигнал изменения вкладки для обновления кнопок undo/redo и текста/действия кнопки
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this, addButton](int index) {
        updateUndoRedoButtons();
        
        // Обновляем текст и действие кнопки в зависимости от вкладки
        if (index == 0) { // Книги
            addButton->setText("Добавить книгу");
            addButton->disconnect(); // Отключаем все предыдущие соединения
            connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddBook);
        } else if (index == 1) { // Абоненты
            addButton->setText("Добавить абонента");
            addButton->disconnect();
            connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddMember);
        } else if (index == 2) { // Работники
            addButton->setText("Добавить работника");
            addButton->disconnect();
            // Для работников создаем диалог с выбором типа
            connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddEmployee);
        } else { // Операции - скрываем кнопку
            addButton->setText("");
            addButton->disconnect();
            addButton->setVisible(false);
            return;
        }
        
        addButton->setVisible(true);
    });
    
    // Устанавливаем начальное состояние для вкладки "Книги"
    addButton->setText("Добавить книгу");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddBook);
}

void MainWindow::setupBooksTab()
{
    QWidget* booksTab = ui->tabWidget->widget(0);
    auto* layout = new QVBoxLayout(booksTab);
    
    // Таблица книг
    auto* booksTable = new QTableWidget(booksTab);
    booksTable->setObjectName("booksTable");
    // Устанавливаем больший шрифт для таблицы
    QFont tableFont = booksTable->font();
    tableFont.setPointSize(11);
    booksTable->setFont(tableFont);
    // Устанавливаем больший шрифт для заголовков таблицы
    QFont headerFont = booksTable->horizontalHeader()->font();
    headerFont.setPointSize(11);
    headerFont.setBold(true);
    booksTable->horizontalHeader()->setFont(headerFont);
    booksTable->setColumnCount(10);
    booksTable->setHorizontalHeaderLabels({"Обложка", "Название", "Автор", "ISBN", "Год", "Жанр", "Доступна", "Количество", "Описание", "Действия"});
    
    // Устанавливаем ширины колонок
        booksTable->setColumnWidth(0, 120);   // Обложка - вертикальная пропорция книги
        booksTable->setColumnWidth(1, 120);  // Название - уменьшено
        booksTable->setColumnWidth(2, 120);  // Автор - уменьшено
        booksTable->setColumnWidth(3, 120);  // ISBN - немного уменьшено
        booksTable->setColumnWidth(4, 70);   // Год - узкий
        booksTable->setColumnWidth(5, 110);   // Жанр - немного уменьшено
        booksTable->setColumnWidth(6, 100);   // Доступна - шире для полного текста заголовка
        booksTable->setColumnWidth(7, 100);   // Количество - шире для полного текста заголовка
        booksTable->setColumnWidth(8, 300);  // Описание - значительно шире для текста
        booksTable->setColumnWidth(9, 140); // Действия
    
    // Устанавливаем минимальные ширины для колонок, чтобы заголовки полностью помещались
        booksTable->horizontalHeader()->setMinimumSectionSize(70); // Минимальная ширина для всех колонок
    
    // Настраиваем растяжение колонок
    // Узкие колонки - фиксированные
        booksTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive); // Обложка
        booksTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive); // Название - фиксированное
        booksTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive); // Автор - фиксированное
        booksTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive); // ISBN - фиксированное
        booksTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive); // Год
        booksTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive); // Жанр
        booksTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive); // Доступна
        booksTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive); // Количество
        booksTable->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Interactive); // Действия
    
    // Широкие колонки - растягиваются
    booksTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch); // Описание
    booksTable->setSortingEnabled(false); // Отключаем стандартную сортировку для кастомной
    booksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Убираем контекстное меню, добавим кнопки действий
    // Подключаем обработчик клика на заголовок для трехсостоятельной сортировки
    connect(booksTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onBookHeaderClicked);
    connect(booksTable, &QTableWidget::itemDoubleClicked, this, [this](const QTableWidgetItem* item) {
        if (item == nullptr) return;
        
        int row = item->row();
        const QTableWidget* table = qobject_cast<const QTableWidget*>(item->tableWidget());
        if (table == nullptr) return;
        
        // Получаем ID из UserRole первой колонки (обложка) или второй (название)
        const QTableWidgetItem* rowItem = table->item(row, 0);
        if (rowItem == nullptr) return;
        
        int bookId = rowItem->data(Qt::UserRole).toInt();
        if (bookId > 0) {
            onShowBookDetails(bookId);
        }
    });
    
    // Панель фильтров (все в одну строку) - перемещена наверх под кнопку "Добавить книгу"
    auto* filtersGroup = new QGroupBox("Фильтры поиска", booksTab);
    auto* filtersLayout = new QHBoxLayout(filtersGroup);
    filtersLayout->setContentsMargins(8, 8, 8, 8);
    filtersLayout->setSpacing(6);
    
    auto* titleFilter = new QLineEdit(filtersGroup);
    titleFilter->setPlaceholderText("Название...");
    titleFilter->setObjectName("titleFilter");
    titleFilter->setMinimumWidth(220);
    connect(titleFilter, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(titleFilter);
    
    auto* authorFilter = new QLineEdit(filtersGroup);
    authorFilter->setPlaceholderText("Автор...");
    authorFilter->setObjectName("authorFilter");
    authorFilter->setMinimumWidth(220);
    connect(authorFilter, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(authorFilter);
    
    auto* genreFilter = new QLineEdit(filtersGroup);
    genreFilter->setPlaceholderText("Жанр...");
    genreFilter->setObjectName("genreFilter");
    genreFilter->setMinimumWidth(180);
    connect(genreFilter, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(genreFilter);
    
    auto* isbnFilter = new QLineEdit(filtersGroup);
    isbnFilter->setPlaceholderText("ISBN...");
    isbnFilter->setObjectName("isbnFilter");
    isbnFilter->setMinimumWidth(180);
    connect(isbnFilter, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(isbnFilter);
    
    auto* yearFromLabel = new QLabel("Год от:", filtersGroup);
    auto* yearFromFilter = new QSpinBox(filtersGroup);
    yearFromFilter->setRange(0, 9999);
    yearFromFilter->setValue(0);
    yearFromFilter->setSpecialValueText("Любой");
    yearFromFilter->setObjectName("yearFromFilter");
    yearFromFilter->setMinimumWidth(120);
    connect(yearFromFilter, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(yearFromLabel);
    filtersLayout->addWidget(yearFromFilter);
    
    auto* yearToLabel = new QLabel("до:", filtersGroup);
    auto* yearToFilter = new QSpinBox(filtersGroup);
    yearToFilter->setRange(0, 9999);
    yearToFilter->setValue(0);
    yearToFilter->setSpecialValueText("Любой");
    yearToFilter->setObjectName("yearToFilter");
    yearToFilter->setMinimumWidth(120);
    connect(yearToFilter, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(yearToLabel);
    filtersLayout->addWidget(yearToFilter);
    
    auto* availabilityLabel = new QLabel("Доступность:", filtersGroup);
    auto* availabilityFilter = new QComboBox(filtersGroup);
    availabilityFilter->addItem("Все", -1);
    availabilityFilter->addItem("Доступна", 1);
    availabilityFilter->addItem("Недоступна", 0);
    availabilityFilter->setObjectName("availabilityFilter");
    availabilityFilter->setMinimumWidth(140);
    connect(availabilityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(availabilityLabel);
    filtersLayout->addWidget(availabilityFilter);
    
    filtersLayout->addStretch();
    
    auto* clearFiltersBtn = new QPushButton("Очистить", filtersGroup);
    clearFiltersBtn->setMinimumWidth(120);
    connect(clearFiltersBtn, &QPushButton::clicked, this, &MainWindow::onClearFilters);
    filtersLayout->addWidget(clearFiltersBtn);
    
    layout->addWidget(filtersGroup);
    
    layout->addWidget(booksTable);
}

void MainWindow::setupMembersTab()
{
    QWidget* membersTab = ui->tabWidget->widget(1);
    auto* layout = new QVBoxLayout(membersTab);
    
    // Таблица абонентов
    auto* membersTable = new QTableWidget(membersTab);
    membersTable->setObjectName("membersTable");
    // Устанавливаем больший шрифт для таблицы
    QFont membersTableFont = membersTable->font();
    membersTableFont.setPointSize(11);
    membersTable->setFont(membersTableFont);
    // Устанавливаем больший шрифт для заголовков таблицы
    QFont membersHeaderFont = membersTable->horizontalHeader()->font();
    membersHeaderFont.setPointSize(11);
    membersHeaderFont.setBold(true);
    membersTable->horizontalHeader()->setFont(membersHeaderFont);
    membersTable->setColumnCount(7);
    membersTable->setHorizontalHeaderLabels({"Имя", "Фамилия", "Телефон", "Email", "Заблокирован", "Книги на руках", "Действия"});
    
    // Устанавливаем ширины колонок
    membersTable->setColumnWidth(0, 120);  // Имя - уже
    membersTable->setColumnWidth(1, 120);  // Фамилия - уже
    membersTable->setColumnWidth(2, 140);  // Телефон - шире
    membersTable->setColumnWidth(3, 220);  // Email - шире для длинных email
    membersTable->setColumnWidth(4, 130);  // Заблокирован - шире
    membersTable->setColumnWidth(5, 250);  // Книги на руках - шире
    membersTable->setColumnWidth(6, 150);  // Действия
    
    // Настраиваем растяжение колонок
        membersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive); // Имя - фиксированная
        membersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive); // Фамилия - фиксированная
        membersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive); // Телефон - фиксированная
        membersTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive); // Email - фиксированная
        membersTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive); // Заблокирован
        membersTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive); // Действия
        // Только Книги на руках - растягивается
        membersTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    membersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Контекстное меню убрано, действия вынесены в отдельную колонку
    
    // Панель фильтров для абонентов (все в одну строку) - перемещена наверх под кнопку "Добавить абонента"
    auto* memberFiltersGroup = new QGroupBox("Фильтры поиска", membersTab);
    auto* memberFiltersLayout = new QHBoxLayout(memberFiltersGroup);
    memberFiltersLayout->setContentsMargins(8, 8, 8, 8);
    memberFiltersLayout->setSpacing(6);
    
    auto* nameFilter = new QLineEdit(memberFiltersGroup);
    nameFilter->setPlaceholderText("Имя...");
    nameFilter->setObjectName("memberNameFilter");
    nameFilter->setMinimumWidth(180);
    connect(nameFilter, &QLineEdit::textChanged, this, &MainWindow::onMemberFilterChanged);
    memberFiltersLayout->addWidget(nameFilter);
    
    auto* surnameFilter = new QLineEdit(memberFiltersGroup);
    surnameFilter->setPlaceholderText("Фамилия...");
    surnameFilter->setObjectName("memberSurnameFilter");
    surnameFilter->setMinimumWidth(180);
    connect(surnameFilter, &QLineEdit::textChanged, this, &MainWindow::onMemberFilterChanged);
    memberFiltersLayout->addWidget(surnameFilter);
    
    auto* phoneFilter = new QLineEdit(memberFiltersGroup);
    phoneFilter->setPlaceholderText("Телефон...");
    phoneFilter->setObjectName("memberPhoneFilter");
    phoneFilter->setMinimumWidth(150);
    connect(phoneFilter, &QLineEdit::textChanged, this, &MainWindow::onMemberFilterChanged);
    memberFiltersLayout->addWidget(phoneFilter);
    
    auto* emailFilter = new QLineEdit(memberFiltersGroup);
    emailFilter->setPlaceholderText("Email...");
    emailFilter->setObjectName("memberEmailFilter");
    emailFilter->setMinimumWidth(200);
    connect(emailFilter, &QLineEdit::textChanged, this, &MainWindow::onMemberFilterChanged);
    memberFiltersLayout->addWidget(emailFilter);
    
    auto* blockedLabel = new QLabel("Статус:", memberFiltersGroup);
    auto* blockedFilter = new QComboBox(memberFiltersGroup);
    blockedFilter->addItem("Все", -1);
    blockedFilter->addItem("Не заблокирован", 0);
    blockedFilter->addItem("Заблокирован", 1);
    blockedFilter->setObjectName("memberBlockedFilter");
    blockedFilter->setMinimumWidth(140);
    connect(blockedFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMemberFilterChanged);
    memberFiltersLayout->addWidget(blockedLabel);
    memberFiltersLayout->addWidget(blockedFilter);
    
    memberFiltersLayout->addStretch();
    
    auto* clearMemberFiltersBtn = new QPushButton("Очистить", memberFiltersGroup);
    clearMemberFiltersBtn->setMinimumWidth(120);
    connect(clearMemberFiltersBtn, &QPushButton::clicked, this, &MainWindow::onClearMemberFilters);
    memberFiltersLayout->addWidget(clearMemberFiltersBtn);
    
    layout->addWidget(memberFiltersGroup);
    
    layout->addWidget(membersTable);
}

void MainWindow::setupEmployeesTab()
{
    QWidget* employeesTab = ui->tabWidget->widget(2);
    auto* layout = new QVBoxLayout(employeesTab);
    
    // Таблица работников
    auto* employeesTable = new QTableWidget(employeesTab);
    employeesTable->setObjectName("employeesTable");
    // Устанавливаем больший шрифт для таблицы
    QFont employeesTableFont = employeesTable->font();
    employeesTableFont.setPointSize(11);
    employeesTable->setFont(employeesTableFont);
    // Устанавливаем больший шрифт для заголовков таблицы
    QFont employeesHeaderFont = employeesTable->horizontalHeader()->font();
    employeesHeaderFont.setPointSize(11);
    employeesHeaderFont.setBold(true);
    employeesTable->horizontalHeader()->setFont(employeesHeaderFont);
    employeesTable->setColumnCount(6);
    employeesTable->setHorizontalHeaderLabels({"Имя", "Фамилия", "Должность", "Зарплата", "Часы работы", "Действия"});
    
    // Устанавливаем ширины колонок
    employeesTable->setColumnWidth(0, 200);  // Имя - шире
    employeesTable->setColumnWidth(1, 200);  // Фамилия - шире
    employeesTable->setColumnWidth(2, 200);  // Должность - шире для длинных названий
    employeesTable->setColumnWidth(3, 120);  // Зарплата - шире для больших чисел
    employeesTable->setColumnWidth(4, 120);  // Часы работы
    employeesTable->setColumnWidth(5, 130);  // Действия
    
    // Настраиваем растяжение колонок
        employeesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive); // Должность
        employeesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive); // Зарплата
        employeesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive); // Часы работы
        employeesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch); // Действия
        // Имя и Фамилия - растягиваются
        employeesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    // Контекстное меню убрано, действия вынесены в отдельную колонку
    
    layout->addWidget(employeesTable);
}

void MainWindow::setupOperationsTab()
{
    QWidget* operationsTab = ui->tabWidget->widget(3);
    auto* layout = new QVBoxLayout(operationsTab);
    
    // Кнопки операций
    auto* borrowBtn = new QPushButton("Выдать книгу", operationsTab);
    auto* returnBtn = new QPushButton("Вернуть книгу", operationsTab);
    auto* overdueBtn = new QPushButton("Показать задолженности", operationsTab);
    
    connect(borrowBtn, &QPushButton::clicked, this, &MainWindow::onBorrowBook);
    connect(returnBtn, &QPushButton::clicked, this, &MainWindow::onReturnBook);
    connect(overdueBtn, &QPushButton::clicked, this, &MainWindow::onShowOverdueBooks);
    
    layout->addWidget(borrowBtn);
    layout->addWidget(returnBtn);
    layout->addWidget(overdueBtn);
    layout->addStretch();
    
    // Текстовое поле для вывода информации
    auto* infoText = new QTextEdit(operationsTab);
    infoText->setObjectName("infoText");
    infoText->setReadOnly(true);
    layout->addWidget(infoText);
}

void MainWindow::refreshBooks()
{
    auto* table = findChild<QTableWidget*>("booksTable");
    if (table == nullptr) return;
    
    table->setRowCount(0);
    auto books = librarySystem.getAllBooks();
    
    // Применяем все фильтры одновременно
    for (const auto* book : books) {
        bool matches = true;
        
        // Фильтр по названию
        if (!bookFilters.title.isEmpty()) {
            QString title = QString::fromStdString(book->getTitle()).toLower();
            if (!title.contains(bookFilters.title.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по автору
        if (matches && !bookFilters.author.isEmpty()) {
            QString author = QString::fromStdString(book->getAuthor()).toLower();
            if (!author.contains(bookFilters.author.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по жанру
        if (matches && !bookFilters.genre.isEmpty()) {
            QString genre = QString::fromStdString(book->getGenre()).toLower();
            if (!genre.contains(bookFilters.genre.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по ISBN
        if (matches && !bookFilters.isbn.isEmpty()) {
            QString isbn = QString::fromStdString(book->getIsbn()).toLower();
            if (!isbn.contains(bookFilters.isbn.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по году (от)
        if (matches && bookFilters.yearFrom > 0 && book->getYear() < bookFilters.yearFrom) {
            matches = false;
        }
        
        // Фильтр по году (до)
        if (matches && bookFilters.yearTo > 0 && book->getYear() > bookFilters.yearTo) {
            matches = false;
        }
        
        // Фильтр по доступности
        if (matches && bookFilters.availability != -1) {
            const bool isAvailable = book->isAvailable();
            if ((bookFilters.availability == 1 && !isAvailable) ||
                (bookFilters.availability == 0 && isAvailable)) {
                matches = false;
            }
        }
        
        // Пропускаем книгу, если она не соответствует фильтрам
        if (!matches) {
            continue;
        }
        int row = table->rowCount();
        table->insertRow(row);
        
        // Колонка 0 - Обложка
        auto* coverItem = new QTableWidgetItem();
        if (QString coverPath = QString::fromStdString(book->getCoverPath()); !coverPath.isEmpty() && QFile::exists(coverPath)) {
            if (QPixmap pixmap(coverPath); !pixmap.isNull()) {
                // Обложка в вертикальной пропорции книги (высота строки 180px, ширина колонки 120px)
                // Масштабируем с сохранением пропорций, максимальная высота 170px, ширина 110px
                QPixmap scaled = pixmap.scaled(110, 170, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverItem->setData(Qt::DecorationRole, scaled);
            } else {
                coverItem->setText("Нет\nобложки");
                coverItem->setTextAlignment(Qt::AlignCenter);
            }
        } else {
            coverItem->setText("Нет\nобложки");
            coverItem->setTextAlignment(Qt::AlignCenter);
        }
        // Сохраняем ID в UserRole обложки
        coverItem->setData(Qt::UserRole, book->getId());
        table->setItem(row, 0, coverItem);
        // Высота строки уменьшена для размещения большего количества книг на экране
        table->setRowHeight(row, 180);
        
        // Остальные колонки (ID убран, все индексы сдвинуты на -1)
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(book->getTitle())));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(book->getAuthor())));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(book->getIsbn())));
        
        // Год (колонка 4) - числовая
        auto* yearItem = new QTableWidgetItem(QString::number(book->getYear()));
        yearItem->setData(Qt::UserRole, book->getYear());
        table->setItem(row, 4, yearItem);
        
        table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(book->getGenre())));
        table->setItem(row, 6, new QTableWidgetItem(book->isAvailable() ? "Да" : "Нет"));
        
        // Количество (колонка 7) - числовая, выровнено по центру
        auto* quantityItem = new QTableWidgetItem(QString::number(book->getQuantity()));
        quantityItem->setData(Qt::UserRole, book->getQuantity());
        quantityItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 7, quantityItem);
        
        // Колонка описания - многострочное описание
        QString description = QString::fromStdString(book->getDescription());
        auto* descriptionLabel = new QLabel(description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setTextInteractionFlags(Qt::NoTextInteraction);
        descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        descriptionLabel->setStyleSheet("QLabel { padding: 4px; }");
        table->setCellWidget(row, 8, descriptionLabel);
        
        QString pdfPath = QString::fromStdString(book->getPdfPath());

        // Колонка 9 - Действия (кнопки)
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->setSpacing(2);

        // Кнопка "Инфо"
        auto* infoBtn = new QPushButton();
        infoBtn->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
        infoBtn->setIconSize(QSize(22, 22));
        infoBtn->setToolTip("Информация о книге");
        infoBtn->setProperty("row", row);
        connect(infoBtn, &QPushButton::clicked, this, [this, book]() { onShowBookDetails(book->getId()); });
        actionLayout->addWidget(infoBtn);

        // Кнопка "Редактировать"
        auto* editBtn = new QPushButton();
        editBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
        editBtn->setIconSize(QSize(22, 22));
        editBtn->setToolTip("Редактировать книгу");
        connect(editBtn, &QPushButton::clicked, this, [this, book]() {
            // Выделяем строку и вызываем редактирование
            auto* table = findChild<QTableWidget*>("booksTable");
            if (table == nullptr) {
                onEditBook();
                return;
            }
            
            for (int r = 0; r < table->rowCount(); ++r) {
                if (const auto* item = table->item(r, 0); item && item->data(Qt::UserRole).toInt() == book->getId()) {
                    table->selectRow(r);
                    break;
                }
            }
            onEditBook();
        });
        actionLayout->addWidget(editBtn);

        // Кнопка "Удалить"
        auto* delBtn = new QPushButton();
        delBtn->setIcon(createRedCrossIcon());
        delBtn->setIconSize(QSize(22, 22));
        delBtn->setToolTip("Удалить книгу");
        connect(delBtn, &QPushButton::clicked, this, [this, book]() {
            int ret = QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить книгу '%1'?").arg(QString::fromStdString(book->getTitle())), QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                try {
                    librarySystem.removeBook(book->getId());
                    refreshBooks();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Книга успешно удалена");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            }
        });
        actionLayout->addWidget(delBtn);

        // Кнопка "PDF"
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            auto* pdfBtn = new QPushButton();
            pdfBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
            pdfBtn->setIconSize(QSize(22, 22));
            pdfBtn->setToolTip("Открыть PDF");
            connect(pdfBtn, &QPushButton::clicked, this, [pdfPath, this]() {
                QUrl url = QUrl::fromLocalFile(pdfPath);
                if (!QDesktopServices::openUrl(url)) {
                    QMessageBox::warning(this, "Ошибка", QString("Не удалось открыть PDF файл.\n\nФайл: %1\n\nУбедитесь, что на вашем компьютере установлен просмотрщик PDF.").arg(pdfPath));
                }
            });
            actionLayout->addWidget(pdfBtn);
        }

        // Кнопка "Выдать"
        auto* borrowBtn = new QPushButton();
        borrowBtn->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        borrowBtn->setIconSize(QSize(22, 22));
        borrowBtn->setToolTip("Выдать книгу");
        connect(borrowBtn, &QPushButton::clicked, this, [this, book]() {
            QDialog dialog(this);
            dialog.setWindowTitle("Выдать книгу");
            QFormLayout form(&dialog);
            // Поиск по пользователям
            auto* memberCombo = new QComboBox(&dialog);
            memberCombo->setEditable(true);
            QStringList memberList;
            auto members = librarySystem.getAllMembers();
            for (const auto* member : members) {
                QString memberText = QString("%1 %2").arg(QString::fromStdString(member->getName())).arg(QString::fromStdString(member->getSurname()));
                memberCombo->addItem(memberText, member->getId());
                memberList << memberText;
            }
            auto* memberCompleter = new QCompleter(memberList, memberCombo);
            memberCompleter->setCaseSensitivity(Qt::CaseInsensitive);
            memberCombo->setCompleter(memberCompleter);
            form.addRow("Абонент:", memberCombo);

            // Поиск по книгам
            auto* bookCombo = new QComboBox(&dialog);
            bookCombo->setEditable(true);
            QStringList bookList;
            auto books = librarySystem.getAllBooks();
            for (const auto* b : books) {
                QString bookText = QString::fromStdString(b->getTitle());
                bookCombo->addItem(bookText, b->getId());
                bookList << bookText;
            }
            // По умолчанию выбранная книга — та, по которой нажата кнопка
            if (int idx = bookCombo->findData(book->getId()); idx >= 0) {
                bookCombo->setCurrentIndex(idx);
            }
            auto* bookCompleter = new QCompleter(bookList, bookCombo);
            bookCompleter->setCaseSensitivity(Qt::CaseInsensitive);
            bookCombo->setCompleter(bookCompleter);
            form.addRow("Книга:", bookCombo);

            // Поиск по работникам
            auto* employeeCombo = new QComboBox(&dialog);
            employeeCombo->setEditable(true);
            QStringList employeeList;
            auto employees = librarySystem.getAllEmployees();
            for (const auto* emp : employees) {
                QString empText = QString("%1 %2").arg(QString::fromStdString(emp->getName())).arg(QString::fromStdString(emp->getSurname()));
                employeeCombo->addItem(empText, emp->getId());
                employeeList << empText;
            }
            auto* empCompleter = new QCompleter(employeeList, employeeCombo);
            empCompleter->setCaseSensitivity(Qt::CaseInsensitive);
            employeeCombo->setCompleter(empCompleter);
            form.addRow("Работник:", employeeCombo);

            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            if (dialog.exec() == QDialog::Accepted) {
                try {
                    int memberId = memberCombo->currentData().toInt();
                    int bookId = bookCombo->currentData().toInt();
                    int employeeId = employeeCombo->currentData().toInt();
                    librarySystem.borrowBook(memberId, bookId, employeeId);
                    refreshBooks();
                    refreshMembers();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Книга успешно выдана");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            }
        });
        actionLayout->addWidget(borrowBtn);

        actionWidget->setLayout(actionLayout);
        table->setCellWidget(row, 9, actionWidget);
    }
    
    // Применяем сортировку после добавления всех книг
    applyBookSorting(table);
}

void MainWindow::onSearchBooks(const QString& text)
{
    // Устаревший метод, оставлен для совместимости
    bookFilters.title = text;
    refreshBooks();
}

void MainWindow::onFilterChanged()
{
    // Обновляем фильтры из полей ввода
    auto* titleFilter = findChild<QLineEdit*>("titleFilter");
    auto* authorFilter = findChild<QLineEdit*>("authorFilter");
    auto* genreFilter = findChild<QLineEdit*>("genreFilter");
    auto* isbnFilter = findChild<QLineEdit*>("isbnFilter");
    auto* yearFromFilter = findChild<QSpinBox*>("yearFromFilter");
    auto* yearToFilter = findChild<QSpinBox*>("yearToFilter");
    auto* availabilityFilter = findChild<QComboBox*>("availabilityFilter");
    
    if (titleFilter) bookFilters.title = titleFilter->text();
    if (authorFilter) bookFilters.author = authorFilter->text();
    if (genreFilter) bookFilters.genre = genreFilter->text();
    if (isbnFilter) bookFilters.isbn = isbnFilter->text();
    if (yearFromFilter) bookFilters.yearFrom = yearFromFilter->value();
    if (yearToFilter) bookFilters.yearTo = yearToFilter->value();
    if (availabilityFilter) bookFilters.availability = availabilityFilter->currentData().toInt();
    
    refreshBooks();
}

void MainWindow::onClearFilters()
{
    // Очищаем все фильтры
    bookFilters = BookFilters();
    
    // Очищаем поля ввода
    const auto* titleFilter = findChild<QLineEdit*>("titleFilter");
    const auto* authorFilter = findChild<QLineEdit*>("authorFilter");
    const auto* genreFilter = findChild<QLineEdit*>("genreFilter");
    const auto* isbnFilter = findChild<QLineEdit*>("isbnFilter");
    auto* yearFromFilter = findChild<QSpinBox*>("yearFromFilter");
    auto* yearToFilter = findChild<QSpinBox*>("yearToFilter");
    auto* availabilityFilter = findChild<QComboBox*>("availabilityFilter");
    
    if (titleFilter) titleFilter->clear();
    if (authorFilter) authorFilter->clear();
    if (genreFilter) genreFilter->clear();
    if (isbnFilter) isbnFilter->clear();
    if (yearFromFilter) yearFromFilter->setValue(0);
    if (yearToFilter) yearToFilter->setValue(0);
    if (availabilityFilter) availabilityFilter->setCurrentIndex(0);
    
    refreshBooks();
}

void MainWindow::onBookHeaderClicked(int column)
{
    // Переключаем состояние сортировки для колонки
    // 0 -> 1 (неактивна -> возрастание)
    // 1 -> 2 (возрастание -> убывание)
    // 2 -> 0 (убывание -> неактивна)
    
    int currentState = bookSortStates.value(column, 0);
    int newState = (currentState + 1) % 3;
    
    // Сбрасываем все остальные колонки в неактивное состояние
    for (auto it = bookSortStates.begin(); it != bookSortStates.end(); ++it) {
        if (it.key() != column) {
            it.value() = 0;
        }
    }
    
    // Устанавливаем новое состояние для выбранной колонки
    bookSortStates[column] = newState;
    
    // Обновляем отображение заголовков
    auto* table = findChild<QTableWidget*>("booksTable");
    if (table == nullptr) return;
    
    QHeaderView* header = table->horizontalHeader();
    
    // Убираем индикатор со всех колонок
    header->setSortIndicatorShown(false);
    
    // Показываем индикатор только для активной колонки (если есть активная сортировка)
    if (newState != 0) {
        header->setSortIndicatorShown(true);
        header->setSortIndicator(column, newState == 1 ? Qt::AscendingOrder : Qt::DescendingOrder);
    }
    
    // Применяем сортировку
    applyBookSorting(table);
}

void MainWindow::applyBookSorting(QTableWidget* table) const
{
    if (table == nullptr) return;
    
    // Находим активную колонку для сортировки
    int sortColumn = -1;
    int sortOrder = 0; // 0 = неактивна, 1 = возрастание, 2 = убывание
    
    for (auto it = bookSortStates.begin(); it != bookSortStates.end(); ++it) {
        if (it.value() != 0) {
            sortColumn = it.key();
            sortOrder = it.value();
            break;
        }
    }
    
    // Если нет активной сортировки, просто возвращаемся
    if (sortColumn == -1 || sortOrder == 0) {
        return;
    }
    
    // Сортируем таблицу
    Qt::SortOrder order = (sortOrder == 1) ? Qt::AscendingOrder : Qt::DescendingOrder;
    
    // Сортируем таблицу (sortItems автоматически использует UserRole для числовых колонок, если данные установлены)
    table->sortItems(sortColumn, order);
}

void MainWindow::refreshMembers()
{
    QTableWidget* table = findChild<QTableWidget*>("membersTable");
    if (table == nullptr) return;
    
    table->setRowCount(0);
    auto members = librarySystem.getAllMembers();
    
    auto allBooks = librarySystem.getAllBooks();
    
    // Применяем все фильтры одновременно
    for (const auto* member : members) {
        bool matches = true;
        
        // Фильтр по имени
        if (!memberFilters.name.isEmpty()) {
            QString name = QString::fromStdString(member->getName()).toLower();
            if (!name.contains(memberFilters.name.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по фамилии
        if (matches && !memberFilters.surname.isEmpty()) {
            QString surname = QString::fromStdString(member->getSurname()).toLower();
            if (!surname.contains(memberFilters.surname.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по телефону
        if (matches && !memberFilters.phone.isEmpty()) {
            QString phone = QString::fromStdString(member->getPhone());
            if (!phone.contains(memberFilters.phone)) {
                matches = false;
            }
        }
        
        // Фильтр по email
        if (matches && !memberFilters.email.isEmpty()) {
            QString email = QString::fromStdString(member->getEmail()).toLower();
            if (!email.contains(memberFilters.email.toLower())) {
                matches = false;
            }
        }
        
        // Фильтр по статусу блокировки
        if (matches && memberFilters.blocked != -1) {
            bool isBlocked = member->getIsBlocked();
            if ((memberFilters.blocked == 1 && !isBlocked) ||
                (memberFilters.blocked == 0 && isBlocked)) {
                matches = false;
            }
        }
        
        // Пропускаем абонента, если он не соответствует фильтрам
        if (!matches) {
            continue;
        }
        
        int row = table->rowCount();
        table->insertRow(row);
        
        // Сохраняем ID в UserRole первой колонки (Имя)
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(member->getName()));
        nameItem->setData(Qt::UserRole, member->getId());
        table->setItem(row, 0, nameItem);
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(member->getSurname())));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(member->getPhone())));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(member->getEmail())));
        // Колонка "Заблокирован" - выровнена по центру
        auto* blockedItem = new QTableWidgetItem(member->getIsBlocked() ? "Да" : "Нет");
        blockedItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 4, blockedItem);
        
        // Колонка 5 - Книги на руках
        auto borrowedBooks = member->getBorrowedBooks();
        int notReturnedCount = 0;
        QStringList bookTitles;
        
        for (const auto& borrowed : borrowedBooks) {
            if (!borrowed.returned) {
                notReturnedCount++;
                // Находим название книги
                QString bookTitle = QString::number(borrowed.bookId);
                for (const auto* book : allBooks) {
                    if (book->getId() == borrowed.bookId) {
                        bookTitle = QString::fromStdString(book->getTitle());
                        break;
                    }
                }
                bookTitles << bookTitle;
            }
        }
        
        QString booksInfo;
        if (notReturnedCount == 0) {
            booksInfo = "Нет книг";
        } else {
            booksInfo = QString("Книг: %1").arg(notReturnedCount);
            if (notReturnedCount <= 3) {
                // Если книг немного, показываем названия
                booksInfo += " (" + bookTitles.join(", ") + ")";
            } else {
                // Если много книг, показываем только первые 2
                booksInfo += " (" + bookTitles.mid(0, 2).join(", ") + "...)";
            }
        }
        
        auto* booksItem = new QTableWidgetItem(booksInfo);
        // Выделяем цветом, если есть книги на руках
        if (notReturnedCount > 0) {
            booksItem->setForeground(QBrush(QColor(255, 140, 0))); // Оранжевый цвет
        }
        table->setItem(row, 5, booksItem);

        // Колонка 6 - Действия (кнопки)
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->setSpacing(2);

        // Кнопка "Инфо"
        auto* infoBtn = new QPushButton();
        infoBtn->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
        infoBtn->setIconSize(QSize(22, 22));
        infoBtn->setToolTip("Информация об абоненте");
        connect(infoBtn, &QPushButton::clicked, this, [this, member]() { onShowMemberDetails(member->getId()); });
        actionLayout->addWidget(infoBtn);

        // Кнопка "Редактировать"
        auto* editBtn = new QPushButton();
        editBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
        editBtn->setIconSize(QSize(22, 22));
        editBtn->setToolTip("Редактировать абонента");
        connect(editBtn, &QPushButton::clicked, this, [this, member]() {
            if (auto* table = findChild<QTableWidget*>("membersTable"); table != nullptr) {
                for (int r = 0; r < table->rowCount(); ++r) {
                    const auto* item = table->item(r, 0);
                    if (item && item->data(Qt::UserRole).toInt() == member->getId()) {
                        table->selectRow(r);
                        break;
                    }
                }
            }
            onEditMember();
        });
        actionLayout->addWidget(editBtn);

        // Кнопка "Блок/Разблок"
        auto* blockBtn = new QPushButton();
        blockBtn->setIconSize(QSize(22, 22));
        if (member->getIsBlocked()) {
            blockBtn->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
            blockBtn->setToolTip("Разблокировать абонента");
            connect(blockBtn, &QPushButton::clicked, this, [this, member]() {
                try {
                    librarySystem.unblockMember(member->getId());
                    refreshMembers();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Абонент успешно разблокирован");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            });
        } else {
            blockBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
            blockBtn->setToolTip("Заблокировать абонента");
            connect(blockBtn, &QPushButton::clicked, this, [this, member]() {
                try {
                    librarySystem.blockMember(member->getId());
                    refreshMembers();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Абонент успешно заблокирован");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            });
        }
        actionLayout->addWidget(blockBtn);

        // Кнопка "Удалить"
        auto* delBtn = new QPushButton();
        delBtn->setIcon(createRedCrossIcon());
        delBtn->setIconSize(QSize(22, 22));
        delBtn->setToolTip("Удалить абонента");
        connect(delBtn, &QPushButton::clicked, this, [this, member]() {
            int ret = QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить абонента %1?").arg(member->getId()), QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                try {
                    librarySystem.removeMember(member->getId());
                    refreshMembers();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Абонент успешно удален");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            }
        });
        actionLayout->addWidget(delBtn);

        actionWidget->setLayout(actionLayout);
        table->setCellWidget(row, 6, actionWidget);
    }
}

void MainWindow::refreshEmployees()
{
    QTableWidget* table = findChild<QTableWidget*>("employeesTable");
    if (table == nullptr) return;
    
    table->setRowCount(0);
    auto employees = librarySystem.getAllEmployees();
    
    for (const auto* emp : employees) {
        int row = table->rowCount();
        table->insertRow(row);
        
        // Сохраняем ID в UserRole первой колонки (Имя)
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(emp->getName()));
        nameItem->setData(Qt::UserRole, emp->getId());
        table->setItem(row, 0, nameItem);
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(emp->getSurname())));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(emp->getPosition())));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(emp->getSalary())));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(emp->getWorkHours())));

        // Колонка 5 - Действия (кнопки)
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->setSpacing(2);

        // Кнопка "Редактировать"
        auto* editBtn = new QPushButton();
        editBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
        editBtn->setIconSize(QSize(22, 22));
        editBtn->setToolTip("Редактировать работника");
        connect(editBtn, &QPushButton::clicked, this, [this, emp]() {
            if (auto* table = findChild<QTableWidget*>("employeesTable"); table != nullptr) {
                for (int r = 0; r < table->rowCount(); ++r) {
                    const auto* item = table->item(r, 0);
                    if (item && item->data(Qt::UserRole).toInt() == emp->getId()) {
                        table->selectRow(r);
                        break;
                    }
                }
            }
            onEditEmployee();
        });
        actionLayout->addWidget(editBtn);

        // Кнопка "Удалить"
        auto* delBtn = new QPushButton();
        delBtn->setIcon(createRedCrossIcon());
        delBtn->setIconSize(QSize(22, 22));
        delBtn->setToolTip("Удалить работника");
        connect(delBtn, &QPushButton::clicked, this, [this, emp]() {
            int ret = QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить работника '%1 %2'?").arg(QString::fromStdString(emp->getName())).arg(QString::fromStdString(emp->getSurname())), QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                try {
                    librarySystem.removeEmployee(emp->getId());
                    refreshEmployees();
                    autoSave();
                    updateUndoRedoButtons();
                    showInfo("Работник успешно удален");
                } catch (const LibraryException& e) {
                    showError(QString::fromStdString(e.what()));
                }
            }
        });
        actionLayout->addWidget(delBtn);

        actionWidget->setLayout(actionLayout);
        table->setCellWidget(row, 5, actionWidget);
    }
}

void MainWindow::onAddBook()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить книгу");
    QFormLayout form(&dialog);
    
    auto* titleEdit = new QLineEdit(&dialog);
    auto* authorEdit = new QLineEdit(&dialog);
    auto* isbnEdit = new QLineEdit(&dialog);
    auto* yearEdit = new QSpinBox(&dialog);
    yearEdit->setRange(1000, 9999);
    yearEdit->setValue(2024);
    auto* genreEdit = new QLineEdit(&dialog);
    auto* quantityEdit = new QSpinBox(&dialog);
    quantityEdit->setRange(1, 10000);
    quantityEdit->setValue(1);
    auto* descriptionEdit = new QTextEdit(&dialog);
    descriptionEdit->setMaximumHeight(100);
    
    // Обложка книги
    auto* coverLabel = new QLabel(&dialog);
    coverLabel->setMinimumSize(200, 300);
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    coverLabel->setText("Обложка не выбрана");
    auto* selectCoverBtn = new QPushButton("Выбрать обложку", &dialog);
    auto* clearCoverBtn = new QPushButton("Удалить обложку", &dialog);
    QString coverPath;
    
    auto* coverLayout = new QHBoxLayout();
    coverLayout->addWidget(selectCoverBtn);
    coverLayout->addWidget(clearCoverBtn);
    
    auto* coverBoxLayout = new QVBoxLayout();
    coverBoxLayout->addWidget(coverLabel);
    coverBoxLayout->addLayout(coverLayout);
    
    connect(selectCoverBtn, &QPushButton::clicked, [&dialog, &coverPath, coverLabel]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog, "Выбрать обложку", "", 
                                                        "Изображения (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!fileName.isEmpty()) {
            coverPath = fileName;
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                QPixmap scaled = pixmap.scaled(coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverLabel->setPixmap(scaled);
                coverLabel->setText("");
            }
        }
    });
    
    connect(clearCoverBtn, &QPushButton::clicked, [&coverPath, coverLabel]() {
        coverPath = "";
        coverLabel->clear();
        coverLabel->setText("Обложка не выбрана");
    });
    
    // PDF файл книги
    auto* pdfLabel = new QLabel(&dialog);
    pdfLabel->setText("PDF не выбран");
    auto* selectPdfBtn = new QPushButton("Выбрать PDF", &dialog);
    auto* clearPdfBtn = new QPushButton("Удалить PDF", &dialog);
    QString pdfPath;
    
    auto* pdfLayout = new QHBoxLayout();
    pdfLayout->addWidget(selectPdfBtn);
    pdfLayout->addWidget(clearPdfBtn);
    
    auto* pdfBoxLayout = new QVBoxLayout();
    pdfBoxLayout->addWidget(pdfLabel);
    pdfBoxLayout->addLayout(pdfLayout);
    
    connect(selectPdfBtn, &QPushButton::clicked, [&dialog, &pdfPath, pdfLabel]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog, "Выбрать PDF файл", "", "PDF файлы (*.pdf)");
        if (!fileName.isEmpty()) {
            pdfPath = fileName;
            QFileInfo fileInfo(fileName);
            pdfLabel->setText("PDF: " + fileInfo.fileName());
        }
    });
    
    connect(clearPdfBtn, &QPushButton::clicked, [&pdfPath, pdfLabel]() {
        pdfPath = "";
        pdfLabel->setText("PDF не выбран");
    });
    
    form.addRow("Название:", titleEdit);
    form.addRow("Автор:", authorEdit);
    form.addRow("ISBN:", isbnEdit);
    form.addRow("Год:", yearEdit);
    form.addRow("Жанр:", genreEdit);
    form.addRow("Количество экземпляров:", quantityEdit);
    form.addRow("Описание:", descriptionEdit);
    form.addRow("Обложка:", coverBoxLayout);
    form.addRow("PDF файл:", pdfBoxLayout);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            // Копируем обложку в папку data/covers/ если выбрана
            QString savedCoverPath = "";
            if (!coverPath.isEmpty()) {
                QDir dir;
                QString coversDir = dataPath + "/covers";
                if (!dir.exists(coversDir)) {
                    dir.mkpath(coversDir);
                }
                
                QFileInfo fileInfo(coverPath);
                QString newFileName = coversDir + "/book_" + QString::number(librarySystem.getNextBookId()) + "." + fileInfo.suffix();
                QFile::copy(coverPath, newFileName);
                savedCoverPath = newFileName;
            }
            
            // Копируем PDF в папку data/pdfs/ если выбран
            QString savedPdfPath = "";
            if (!pdfPath.isEmpty()) {
                QDir dir;
                QString pdfsDir = dataPath + "/pdfs";
                if (!dir.exists(pdfsDir)) {
                    dir.mkpath(pdfsDir);
                }
                
                QFileInfo fileInfo(pdfPath);
                QString newFileName = pdfsDir + "/book_" + QString::number(librarySystem.getNextBookId()) + ".pdf";
                QFile::copy(pdfPath, newFileName);
                savedPdfPath = newFileName;
            }
            
            librarySystem.addBook(
                titleEdit->text().toStdString(),
                authorEdit->text().toStdString(),
                isbnEdit->text().toStdString(),
                yearEdit->value(),
                genreEdit->text().toStdString(),
                savedCoverPath.toStdString(),
                quantityEdit->value(),
                descriptionEdit->toPlainText().toStdString(),
                savedPdfPath.toStdString()
            );
            refreshBooks();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Книга успешно добавлена");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onEditBook()
{
    const auto* booksTable = findChild<QTableWidget*>("booksTable");
    if (booksTable == nullptr) {
        showError("Таблица книг не найдена");
        return;
    }
    
    int currentRow = booksTable->currentRow();
    if (currentRow < 0) {
        showError("Выберите книгу для редактирования");
        return;
    }
    
    // Получаем ID из UserRole первой колонки (обложка)
    const auto* item = booksTable->item(currentRow, 0);
    if (item == nullptr) return;
    
    int bookId = item->data(Qt::UserRole).toInt();
    if (bookId <= 0) {
        showError("Неверный ID книги");
        return;
    }
    
    Book* bookPtr = librarySystem.findBook(bookId);
    if (bookPtr == nullptr) {
        showError("Книга не найдена");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать книгу");
    QFormLayout form(&dialog);
    
    auto* titleEdit = new QLineEdit(&dialog);
    titleEdit->setText(QString::fromStdString(bookPtr->getTitle()));
    auto* authorEdit = new QLineEdit(&dialog);
    authorEdit->setText(QString::fromStdString(bookPtr->getAuthor()));
    auto* isbnEdit = new QLineEdit(&dialog);
    isbnEdit->setText(QString::fromStdString(bookPtr->getIsbn()));
    auto* yearEdit = new QSpinBox(&dialog);
    yearEdit->setRange(1000, 9999);
    yearEdit->setValue(bookPtr->getYear());
    auto* genreEdit = new QLineEdit(&dialog);
    genreEdit->setText(QString::fromStdString(bookPtr->getGenre()));
    auto* quantityEdit = new QSpinBox(&dialog);
    quantityEdit->setRange(1, 10000);
    quantityEdit->setValue(bookPtr->getQuantity());
    auto* descriptionEdit = new QTextEdit(&dialog);
    descriptionEdit->setMaximumHeight(100);
    descriptionEdit->setPlainText(QString::fromStdString(bookPtr->getDescription()));
    
    // Обложка книги
    auto* coverLabel = new QLabel(&dialog);
    coverLabel->setMinimumSize(200, 300);
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    QString currentCoverPath = QString::fromStdString(bookPtr->getCoverPath());
    QString coverPath = currentCoverPath;
    
    if (!currentCoverPath.isEmpty() && QFile::exists(currentCoverPath)) {
        QPixmap pixmap(currentCoverPath);
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            coverLabel->setPixmap(scaled);
        } else {
            coverLabel->setText("Обложка не выбрана");
        }
    } else {
        coverLabel->setText("Обложка не выбрана");
    }
    
    auto* selectCoverBtn = new QPushButton("Выбрать обложку", &dialog);
    auto* clearCoverBtn = new QPushButton("Удалить обложку", &dialog);
    
    auto* coverLayout = new QHBoxLayout();
    coverLayout->addWidget(selectCoverBtn);
    coverLayout->addWidget(clearCoverBtn);
    
    auto* coverBoxLayout = new QVBoxLayout();
    coverBoxLayout->addWidget(coverLabel);
    coverBoxLayout->addLayout(coverLayout);
    
    connect(selectCoverBtn, &QPushButton::clicked, [&dialog, &coverPath, coverLabel]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog, "Выбрать обложку", "", 
                                                        "Изображения (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!fileName.isEmpty()) {
            coverPath = fileName;
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                QPixmap scaled = pixmap.scaled(coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverLabel->setPixmap(scaled);
                coverLabel->setText("");
            }
        }
    });
    
    connect(clearCoverBtn, &QPushButton::clicked, [&coverPath, coverLabel]() {
        coverPath = "";
        coverLabel->clear();
        coverLabel->setText("Обложка не выбрана");
    });
    
    // PDF файл книги
    QString currentPdfPath = QString::fromStdString(bookPtr->getPdfPath());
    QString pdfPath = currentPdfPath;
    auto* pdfLabel = new QLabel(&dialog);
    if (!currentPdfPath.isEmpty() && QFile::exists(currentPdfPath)) {
        QFileInfo fileInfo(currentPdfPath);
        pdfLabel->setText("PDF: " + fileInfo.fileName());
    } else {
        pdfLabel->setText("PDF не выбран");
    }
    auto* selectPdfBtn = new QPushButton("Выбрать PDF", &dialog);
    auto* clearPdfBtn = new QPushButton("Удалить PDF", &dialog);
    
    auto* pdfLayout = new QHBoxLayout();
    pdfLayout->addWidget(selectPdfBtn);
    pdfLayout->addWidget(clearPdfBtn);
    
    auto* pdfBoxLayout = new QVBoxLayout();
    pdfBoxLayout->addWidget(pdfLabel);
    pdfBoxLayout->addLayout(pdfLayout);
    
    connect(selectPdfBtn, &QPushButton::clicked, [&dialog, &pdfPath, pdfLabel]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog, "Выбрать PDF файл", "", "PDF файлы (*.pdf)");
        if (!fileName.isEmpty()) {
            pdfPath = fileName;
            QFileInfo fileInfo(fileName);
            pdfLabel->setText("PDF: " + fileInfo.fileName());
        }
    });
    
    connect(clearPdfBtn, &QPushButton::clicked, [&pdfPath, pdfLabel]() {
        pdfPath = "";
        pdfLabel->setText("PDF не выбран");
    });
    
    auto* availableCheckBox = new QCheckBox(&dialog);
    // Проверяем реальную доступность (с учетом количества и ручной блокировки)
    int borrowedCount = librarySystem.getBorrowedCount(bookId);
    int availableCount = bookPtr->getQuantity() - borrowedCount;
    bool actuallyAvailable = !bookPtr->getManuallyDisabled() && availableCount > 0;
    availableCheckBox->setChecked(actuallyAvailable);
    // Добавляем подсказку о количестве
    auto* availabilityHint = new QLabel(&dialog);
    availabilityHint->setText(QString("Доступно экземпляров: %1 из %2").arg(availableCount).arg(bookPtr->getQuantity()));
    if (bookPtr->getManuallyDisabled()) {
        availabilityHint->setText(availabilityHint->text() + " (заблокировано вручную)");
    }
    
    form.addRow("Название:", titleEdit);
    form.addRow("Автор:", authorEdit);
    form.addRow("ISBN:", isbnEdit);
    form.addRow("Год:", yearEdit);
    form.addRow("Жанр:", genreEdit);
    form.addRow("Количество экземпляров:", quantityEdit);
    form.addRow("Доступна:", availableCheckBox);
    form.addRow("", availabilityHint);
    form.addRow("Описание:", descriptionEdit);
    form.addRow("Обложка:", coverBoxLayout);
    form.addRow("PDF файл:", pdfBoxLayout);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            // Копируем обложку в папку data/covers/ если выбрана новая
            QString savedCoverPath = currentCoverPath;
            if (!coverPath.isEmpty() && coverPath != currentCoverPath) {
                QDir dir;
                QString coversDir = dataPath + "/covers";
                if (!dir.exists(coversDir)) {
                    dir.mkpath(coversDir);
                }
                
                // Удаляем старую обложку, если она была
                if (!currentCoverPath.isEmpty() && QFile::exists(currentCoverPath)) {
                    QFile::remove(currentCoverPath);
                }
                
                QFileInfo fileInfo(coverPath);
                QString newFileName = coversDir + "/book_" + QString::number(bookId) + "." + fileInfo.suffix();
                if (QFile::exists(newFileName)) {
                    QFile::remove(newFileName);
                }
                QFile::copy(coverPath, newFileName);
                savedCoverPath = newFileName;
            } else if (coverPath.isEmpty() && !currentCoverPath.isEmpty()) {
                // Удаляем обложку, если пользователь выбрал "Удалить"
                if (QFile::exists(currentCoverPath)) {
                    QFile::remove(currentCoverPath);
                }
                savedCoverPath = "";
            }
            
            // Копируем PDF в папку data/pdfs/ если выбрана новая
            QString savedPdfPath = currentPdfPath;
            if (!pdfPath.isEmpty() && pdfPath != currentPdfPath) {
                QDir dir;
                QString pdfsDir = dataPath + "/pdfs";
                if (!dir.exists(pdfsDir)) {
                    dir.mkpath(pdfsDir);
                }
                
                // Удаляем старый PDF, если он был
                if (!currentPdfPath.isEmpty() && QFile::exists(currentPdfPath)) {
                    QFile::remove(currentPdfPath);
                }
                
                QFileInfo fileInfo(pdfPath);
                QString newFileName = pdfsDir + "/book_" + QString::number(bookId) + ".pdf";
                if (QFile::exists(newFileName)) {
                    QFile::remove(newFileName);
                }
                QFile::copy(pdfPath, newFileName);
                savedPdfPath = newFileName;
            } else if (pdfPath.isEmpty() && !currentPdfPath.isEmpty()) {
                // Удаляем PDF, если пользователь выбрал "Удалить"
                if (QFile::exists(currentPdfPath)) {
                    QFile::remove(currentPdfPath);
                }
                savedPdfPath = "";
            }
            
            librarySystem.editBook(
                bookId,
                titleEdit->text().toStdString(),
                authorEdit->text().toStdString(),
                isbnEdit->text().toStdString(),
                yearEdit->value(),
                genreEdit->text().toStdString(),
                savedCoverPath.toStdString(),
                quantityEdit->value(),
                descriptionEdit->toPlainText().toStdString(),
                savedPdfPath.toStdString()
            );
            
            // Управление доступностью: если пользователь снял галочку "Доступна",
            // значит он хочет вручную заблокировать книгу
            // Если пользователь хочет сделать книгу недоступной вручную
            if (bool shouldBeAvailable = availableCheckBox->isChecked(); !shouldBeAvailable) {
                bookPtr->setManuallyDisabled(true);
                bookPtr->setAvailable(false);
            } else {
                // Если пользователь хочет сделать доступной, снимаем ручную блокировку
                bookPtr->setManuallyDisabled(false);
            }
            // Обновляем доступность на основе текущего количества (editBook уже обновляет автоматически)
            librarySystem.updateBookAvailability(bookId);
            
            refreshBooks();
            autoSave();
            updateUndoRedoButtons();
            showInfo("Книга успешно отредактирована");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onRemoveBook()
{
        bool okFlag = false;
        int bookId = QInputDialog::getInt(this, "Удалить книгу", "Введите ID книги:", 1, 1, 100000, 1, &okFlag);
        if (okFlag) {
            try {
                librarySystem.removeBook(bookId);
                refreshBooks();
                autoSave(); // Автосохранение после изменения
                updateUndoRedoButtons();
                showInfo("Книга успешно удалена");
            } catch (const LibraryException& e) {
                showError(QString::fromStdString(e.what()));
            }
        }
}

void MainWindow::onShowBookDetails(int bookId)
{
    try {
        const Book* bookPtr = librarySystem.findBook(bookId);
        if (bookPtr == nullptr) {
            showError("Книга не найдена");
            return;
        }
        // Создаем красивое окно с информацией о книге
        auto* detailDialogPtr = new QDialog(this);
        detailDialogPtr->setWindowTitle(QString("Информация о книге: %1").arg(QString::fromStdString(bookPtr->getTitle())));
        detailDialogPtr->setMinimumSize(600, 500);
        auto* mainLayoutPtr = new QHBoxLayout(detailDialogPtr);
        // Левая часть - обложка
        auto* leftLayoutPtr = new QVBoxLayout();
        auto* coverLabelPtr = new QLabel(detailDialogPtr);
        coverLabelPtr->setMinimumSize(200, 300);
        coverLabelPtr->setAlignment(Qt::AlignCenter);
        coverLabelPtr->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
        if (QString coverPath = QString::fromStdString(bookPtr->getCoverPath()); !coverPath.isEmpty() && QFile::exists(coverPath)) {
            if (QPixmap pixmap(coverPath); !pixmap.isNull()) {
                QPixmap scaled = pixmap.scaled(coverLabelPtr->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverLabelPtr->setPixmap(scaled);
            } else {
                coverLabelPtr->setText("Обложка\nне найдена");
            }
        } else {
            coverLabelPtr->setText("Обложка\nне выбрана");
        }
        leftLayoutPtr->addWidget(coverLabelPtr);
        leftLayoutPtr->addStretch();
        // Правая часть - информация
        auto* rightLayoutPtr = new QVBoxLayout();
        auto* infoLayoutPtr = new QFormLayout();
        auto* idLabel = new QLabel(QString::number(bookPtr->getId()));
        auto* titleLabel = new QLabel(QString::fromStdString(bookPtr->getTitle()));
        titleLabel->setWordWrap(true);
        auto* authorLabel = new QLabel(QString::fromStdString(bookPtr->getAuthor()));
        auto* isbnLabel = new QLabel(QString::fromStdString(bookPtr->getIsbn()));
        auto* yearLabel = new QLabel(QString::number(bookPtr->getYear()));
        auto* genreLabel = new QLabel(QString::fromStdString(bookPtr->getGenre()));
        auto* availableLabel = new QLabel(bookPtr->isAvailable() ? "Да" : "Нет");
        auto* quantityLabel = new QLabel(QString::number(bookPtr->getQuantity()));
        auto* descriptionLabel = new QLabel(QString::fromStdString(bookPtr->getDescription()));
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setMaximumWidth(300);
        infoLayoutPtr->addRow("ID:", idLabel);
        infoLayoutPtr->addRow("Название:", titleLabel);
        infoLayoutPtr->addRow("Автор:", authorLabel);
        infoLayoutPtr->addRow("ISBN:", isbnLabel);
        infoLayoutPtr->addRow("Год:", yearLabel);
        infoLayoutPtr->addRow("Жанр:", genreLabel);
        infoLayoutPtr->addRow("Количество:", quantityLabel);
        infoLayoutPtr->addRow("Доступна:", availableLabel);
        infoLayoutPtr->addRow("Описание:", descriptionLabel);
        auto* infoBoxPtr = new QGroupBox("Информация о книге", detailDialogPtr);
        infoBoxPtr->setLayout(infoLayoutPtr);
        rightLayoutPtr->addWidget(infoBoxPtr);
        // Кнопка открытия PDF
        QString pdfPath = QString::fromStdString(bookPtr->getPdfPath());
        auto* openPdfBtnPtr = static_cast<QPushButton*>(nullptr);
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            openPdfBtnPtr = new QPushButton("Открыть PDF", detailDialogPtr);
            connect(openPdfBtnPtr, &QPushButton::clicked, [pdfPath]() {
                QUrl url = QUrl::fromLocalFile(pdfPath);
                QDesktopServices::openUrl(url);
            });
            rightLayoutPtr->addWidget(openPdfBtnPtr);
        }
        rightLayoutPtr->addStretch();
        // Кнопка закрытия
        auto* closeBtnPtr = new QPushButton("Закрыть", detailDialogPtr);
        connect(closeBtnPtr, &QPushButton::clicked, detailDialogPtr, &QDialog::accept);
        rightLayoutPtr->addWidget(closeBtnPtr);
        mainLayoutPtr->addLayout(leftLayoutPtr);
        mainLayoutPtr->addLayout(rightLayoutPtr);
        detailDialogPtr->exec();
        delete detailDialogPtr;
        
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onAddMember()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить абонента");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    auto* surnameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* emailEdit = new QLineEdit(&dialog);
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Email:", emailEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            int id = librarySystem.addMember(
                nameEdit->text().toStdString(),
                surnameEdit->text().toStdString(),
                phoneEdit->text().toStdString(),
                emailEdit->text().toStdString()
            );
            refreshMembers();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo(QString("Абонент успешно добавлен с ID: %1").arg(id));
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onEditMember()
{
    const auto* membersTable = findChild<QTableWidget*>("membersTable");
    if (!membersTable) {
        showError("Таблица абонентов не найдена");
        return;
    }
    
    int currentRow = membersTable->currentRow();
    if (currentRow < 0) {
        showError("Выберите абонента для редактирования");
        return;
    }
    
    // Получаем ID из UserRole первой колонки (Имя)
    const auto* nameItem = membersTable->item(currentRow, 0);
    if (!nameItem) return;
    
    int id = nameItem->data(Qt::UserRole).toInt();
    if (id <= 0) {
        showError("Неверный ID абонента");
        return;
    }
    
    const LibraryMember* member = librarySystem.findMember(id);
    if (!member) {
        showError("Абонент не найден");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать абонента");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(QString::fromStdString(member->getName()));
    auto* surnameEdit = new QLineEdit(&dialog);
    surnameEdit->setText(QString::fromStdString(member->getSurname()));
    auto* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setText(QString::fromStdString(member->getPhone()));
    auto* emailEdit = new QLineEdit(&dialog);
    emailEdit->setText(QString::fromStdString(member->getEmail()));
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Email:", emailEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            librarySystem.editMember(
                id,
                nameEdit->text().toStdString(),
                surnameEdit->text().toStdString(),
                phoneEdit->text().toStdString(),
                emailEdit->text().toStdString()
            );
            refreshMembers();
            autoSave();
            updateUndoRedoButtons();
            showInfo("Абонент успешно отредактирован");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onRemoveMember()
{
    bool ok;
    int id = QInputDialog::getInt(this, "Удалить абонента", "Введите ID абонента:", 1, 1, 100000, 1, &ok);
    if (ok) {
        try {
            librarySystem.removeMember(id);
            refreshMembers();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Абонент успешно удален");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onBlockMember()
{
    bool ok;
    int id = QInputDialog::getInt(this, "Заблокировать абонента", "Введите ID абонента:", 1, 1, 100000, 1, &ok);
    if (ok) {
        try {
            librarySystem.blockMember(id);
            refreshMembers();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Абонент успешно заблокирован");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onUnblockMember()
{
    bool ok;
    int id = QInputDialog::getInt(this, "Разблокировать абонента", "Введите ID абонента:", 1, 1, 100000, 1, &ok);
    if (ok) {
        try {
            librarySystem.unblockMember(id);
            refreshMembers();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Абонент успешно разблокирован");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onBorrowBook()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Выдать книгу");
    QFormLayout form(&dialog);

    // Абонент с поиском
    auto* memberCombo = new QComboBox(&dialog);
    memberCombo->setEditable(true);
    QStringList memberList;
    auto members = librarySystem.getAllMembers();
    for (const auto* member : members) {
        QString memberText = QString("%1 %2").arg(QString::fromStdString(member->getName())).arg(QString::fromStdString(member->getSurname()));
        memberCombo->addItem(memberText, member->getId());
        memberList << memberText;
    }
    auto* memberCompleter = new QCompleter(memberList, memberCombo);
    memberCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    memberCombo->setCompleter(memberCompleter);
    form.addRow("Абонент:", memberCombo);

    // Книга с поиском
    auto* bookCombo = new QComboBox(&dialog);
    bookCombo->setEditable(true);
    QStringList bookList;
    auto books = librarySystem.getAllBooks();
    for (const auto* book : books) {
        QString bookText = QString::fromStdString(book->getTitle());
        bookCombo->addItem(bookText, book->getId());
        bookList << bookText;
    }
    auto* bookCompleter = new QCompleter(bookList, bookCombo);
    bookCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    bookCombo->setCompleter(bookCompleter);
    form.addRow("Книга:", bookCombo);

    // Работник с поиском
    auto* employeeCombo = new QComboBox(&dialog);
    employeeCombo->setEditable(true);
    QStringList employeeList;
    auto employees = librarySystem.getAllEmployees();
    for (const auto* emp : employees) {
        QString empText = QString("%1 %2").arg(QString::fromStdString(emp->getName())).arg(QString::fromStdString(emp->getSurname()));
        employeeCombo->addItem(empText, emp->getId());
        employeeList << empText;
    }
    auto* empCompleter = new QCompleter(employeeList, employeeCombo);
    empCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    employeeCombo->setCompleter(empCompleter);
    form.addRow("Работник:", employeeCombo);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            int memberId = memberCombo->currentData().toInt();
            int bookId = bookCombo->currentData().toInt();
            int employeeId = employeeCombo->currentData().toInt();
            librarySystem.borrowBook(memberId, bookId, employeeId);
            refreshBooks();
            refreshMembers();
            autoSave();
            showInfo("Книга успешно выдана");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onReturnBook()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Вернуть книгу");
    QFormLayout form(&dialog);
    
    // Абонент с поиском
    auto* memberCombo = new QComboBox(&dialog);
    memberCombo->setEditable(true);
    QStringList memberList;
    auto members = librarySystem.getAllMembers();
    for (const auto* member : members) {
        QString memberText = QString("%1 %2").arg(QString::fromStdString(member->getName())).arg(QString::fromStdString(member->getSurname()));
        memberCombo->addItem(memberText, member->getId());
        memberList << memberText;
    }
    auto* memberCompleter = new QCompleter(memberList, memberCombo);
    memberCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    memberCombo->setCompleter(memberCompleter);
    form.addRow("Абонент:", memberCombo);
    
    // Книга с поиском (только взятые книги выбранного абонента)
    auto* bookCombo = new QComboBox(&dialog);
    bookCombo->setEditable(true);
    bookCombo->setEnabled(false); // Будет активирован после выбора абонента
    
    // Обновляем список книг при выборе абонента
    connect(memberCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, bookCombo, memberCombo]() {
        bookCombo->clear();
        int memberId = memberCombo->currentData().toInt();
        if (memberId > 0) {
            try {
                auto borrowedBooks = librarySystem.getMemberBooks(memberId);
                auto allBooks = librarySystem.getAllBooks();
                QStringList bookList;
                
                for (const auto& borrowed : borrowedBooks) {
                    if (!borrowed.returned) {
                        // Находим название книги
                        QString bookTitle = QString::number(borrowed.bookId);
                        for (const auto* book : allBooks) {
                            if (book->getId() == borrowed.bookId) {
                                bookTitle = QString::fromStdString(book->getTitle());
                                break;
                            }
                        }
                        bookCombo->addItem(bookTitle, borrowed.bookId);
                        bookList << bookTitle;
                    }
                }
                
                if (bookCombo->count() > 0) {
                    bookCombo->setEnabled(true);
                    auto* bookCompleter = new QCompleter(bookList, bookCombo);
                    bookCompleter->setCaseSensitivity(Qt::CaseInsensitive);
                    bookCombo->setCompleter(bookCompleter);
                } else {
                    bookCombo->setEnabled(false);
                    bookCombo->addItem("Нет взятых книг", -1);
                }
            } catch (const std::exception&) {
                bookCombo->setEnabled(false);
                bookCombo->addItem("Ошибка загрузки", -1);
            }
        }
    });
    
    form.addRow("Книга:", bookCombo);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            int memberId = memberCombo->currentData().toInt();
            int bookId = bookCombo->currentData().toInt();
            
            if (memberId <= 0 || bookId <= 0) {
                showError("Пожалуйста, выберите абонента и книгу");
                return;
            }
            
            librarySystem.returnBook(memberId, bookId);
            refreshBooks();
            refreshMembers();
            autoSave(); // Автосохранение после изменения
            showInfo("Книга успешно возвращена");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onShowMemberBooks()
{
    bool ok;
    int id = QInputDialog::getInt(this, "Показать книги абонента", "Введите ID абонента:", 1, 1, 100000, 1, &ok);
    if (ok) {
        onShowMemberDetails(id);
    }
}

void MainWindow::onSearchMember()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Поиск абонента");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    auto* surnameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* emailEdit = new QLineEdit(&dialog);
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Email:", emailEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            std::vector<LibraryMember*> foundMembers;
            auto allMembers = librarySystem.getAllMembers();
            
            for (auto* member : allMembers) {
                bool match = true;
                
                if (!nameEdit->text().isEmpty() && 
                    QString::fromStdString(member->getName()).toLower().indexOf(nameEdit->text().toLower()) == -1) {
                    match = false;
                }
                if (!surnameEdit->text().isEmpty() && 
                    QString::fromStdString(member->getSurname()).toLower().indexOf(surnameEdit->text().toLower()) == -1) {
                    match = false;
                }
                if (!phoneEdit->text().isEmpty() && 
                    QString::fromStdString(member->getPhone()).indexOf(phoneEdit->text()) == -1) {
                    match = false;
                }
                if (!emailEdit->text().isEmpty() && 
                    QString::fromStdString(member->getEmail()).toLower().indexOf(emailEdit->text().toLower()) == -1) {
                    match = false;
                }
                
                if (match) {
                    foundMembers.push_back(member);
                }
            }
            
            if (foundMembers.empty()) {
                showInfo("Абоненты не найдены");
            } else if (foundMembers.size() == 1) {
                onShowMemberDetails(foundMembers[0]->getId());
            } else {
                // Показываем список найденных абонентов
                QStringList items;
                QMap<QString, int> itemToIdMap;
                for (const auto* member : foundMembers) {
                    QString itemText = QString("%1 %2 (%3)")
                        .arg(QString::fromStdString(member->getName()))
                        .arg(QString::fromStdString(member->getSurname()))
                        .arg(QString::fromStdString(member->getPhone()));
                    if (!member->getEmail().empty()) {
                        itemText += " - " + QString::fromStdString(member->getEmail());
                    }
                    items << itemText;
                    itemToIdMap[itemText] = member->getId();
                }
                bool ok;
                QString selected = QInputDialog::getItem(this, "Выберите абонента", 
                                                       "Найдено несколько абонентов:", items, 0, false, &ok);
                if (ok && itemToIdMap.contains(selected)) {
                    int id = itemToIdMap[selected];
                    onShowMemberDetails(id);
                }
            }
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onShowMemberDetails(int memberId)
{
    try {
        const LibraryMember* member = librarySystem.findMember(memberId);
        if (!member) {
            showError("Абонент не найден");
            return;
        }
        
        auto books = librarySystem.getMemberBooks(memberId);
        auto allBooks = librarySystem.getAllBooks();
        auto allEmployees = librarySystem.getAllEmployees();
        
        // Создаем красивое окно с информацией
        auto* detailDialog = new QDialog(this);
        detailDialog->setWindowTitle(QString("Информация об абоненте: %1").arg(memberId));
        detailDialog->setMinimumSize(1100, 600);
        detailDialog->resize(1200, 700);
        
        auto* mainLayout = new QVBoxLayout(detailDialog);
        
        // Информация об абоненте
        auto* memberInfoBox = new QGroupBox("Информация об абоненте", detailDialog);
        auto* memberLayout = new QFormLayout(memberInfoBox);
        
        auto* idLabel = new QLabel(QString::number(member->getId()));
        auto* nameLabel = new QLabel(QString::fromStdString(member->getFullName()));
        auto* phoneLabel = new QLabel(QString::fromStdString(member->getPhone()));
        auto* emailLabel = new QLabel(QString::fromStdString(member->getEmail()));
        auto* blockedLabel = new QLabel(member->getIsBlocked() ? "Да" : "Нет");
        
        memberLayout->addRow("ID:", idLabel);
        memberLayout->addRow("ФИО:", nameLabel);
        memberLayout->addRow("Телефон:", phoneLabel);
        memberLayout->addRow("Email:", emailLabel);
        memberLayout->addRow("Заблокирован:", blockedLabel);
        
        mainLayout->addWidget(memberInfoBox);
        
        // Таблица взятых книг
        auto* booksBox = new QGroupBox("Взятые книги", detailDialog);
        auto* booksLayout = new QVBoxLayout(booksBox);
        
        auto* booksTable = new QTableWidget(detailDialog);
        booksTable->setColumnCount(6);
        booksTable->setHorizontalHeaderLabels({"Название", "Дата взятия", "Дата возврата", "Возвращена", "Выдал работник", "Действие"});
        
        // Настраиваем ширины колонок
        booksTable->setColumnWidth(0, 250);  // Название
        booksTable->setColumnWidth(1, 120);  // Дата взятия
        booksTable->setColumnWidth(2, 120);  // Дата возврата
        booksTable->setColumnWidth(3, 100);  // Возвращена
        booksTable->setColumnWidth(4, 180);  // Выдал работник
        booksTable->setColumnWidth(5, 100);  // Действие
        
        booksTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Название растягивается
        booksTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); // Выдал работник растягивается
        booksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        
        booksTable->setRowCount(static_cast<int>(books.size()));
        for (size_t i = 0; i < books.size(); i++) {
            const auto& borrowedBook = books[i];
            
            // Находим название книги
            QString bookTitle = QString::number(borrowedBook.bookId);
            for (const auto* b : allBooks) {
                if (b->getId() == borrowedBook.bookId) {
                    bookTitle = QString::fromStdString(b->getTitle());
                    break;
                }
            }
            
            // Находим имя работника
            QString employeeName = "Неизвестно";
            if (borrowedBook.employeeId > 0) {
                for (const auto* emp : allEmployees) {
                    if (emp->getId() == borrowedBook.employeeId) {
                        employeeName = QString::fromStdString(emp->getFullName());
                        break;
                    }
                }
            }
            
            booksTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(bookTitle));
            booksTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(borrowedBook.borrowDate)));
            booksTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(borrowedBook.returnDate)));
            booksTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(borrowedBook.returned ? "Да" : "Нет"));
            booksTable->setItem(static_cast<int>(i), 4, new QTableWidgetItem(employeeName));
            
            // Добавляем кнопку возврата для не возвращенных книг
            if (!borrowedBook.returned) {
                auto* returnBtn = new QPushButton("Вернуть", detailDialog);
                returnBtn->setProperty("memberId", memberId);
                returnBtn->setProperty("bookId", borrowedBook.bookId);
                connect(returnBtn, &QPushButton::clicked, [this, detailDialog, memberId, bookId = borrowedBook.bookId]() {
                    try {
                        librarySystem.returnBook(memberId, bookId);
                        refreshBooks();
                        refreshMembers();
                        autoSave();
                        updateUndoRedoButtons();
                        showInfo("Книга успешно возвращена");
                        // Закрываем диалог и открываем заново с обновленными данными
                        detailDialog->accept();
                        onShowMemberDetails(memberId);
                    } catch (const LibraryException& e) {
                        showError(QString::fromStdString(e.what()));
                    }
                });
                booksTable->setCellWidget(static_cast<int>(i), 5, returnBtn);
            } else {
                // Для возвращенных книг просто пустая ячейка
                booksTable->setItem(static_cast<int>(i), 5, new QTableWidgetItem("-"));
            }
        }
        
        booksLayout->addWidget(booksTable);
        mainLayout->addWidget(booksBox);
        
        // Кнопка закрытия
        auto* closeBtn = new QPushButton("Закрыть", detailDialog);
        connect(closeBtn, &QPushButton::clicked, detailDialog, &QDialog::accept);
        mainLayout->addWidget(closeBtn);
        
        detailDialog->exec();
        delete detailDialog;
        
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onShowOverdueBooks()
{
    try {
        auto overdue = librarySystem.getOverdueBooks();
        auto allBooks = librarySystem.getAllBooks();
        
        // Создаем простое диалоговое окно
        auto* overdueDialog = new QDialog(this);
        overdueDialog->setWindowTitle("Отчет по задолженностям");
        overdueDialog->setMinimumSize(800, 400);
        overdueDialog->resize(1000, 500);
        
        auto* mainLayout = new QVBoxLayout(overdueDialog);
        
        // Простой заголовок
        auto* headerLabel = new QLabel(overdueDialog);
        if (overdue.empty()) {
            headerLabel->setText(QString("<h3>Нет задолженностей по книгам</h3>"));
        } else {
            headerLabel->setText(QString("<h3>Задолженности по книгам: %1 шт.</h3>").arg(overdue.size()));
        }
        mainLayout->addWidget(headerLabel);
        
        if (!overdue.empty()) {
            // Таблица задолженностей
            auto* overdueTable = new QTableWidget(overdueDialog);
            overdueTable->setColumnCount(6);
            overdueTable->setHorizontalHeaderLabels({"Абонент", "Телефон", "Название книги", "Дата взятия", "Дата возврата", "Дней просрочки"});
            
            // Настраиваем ширины колонок с правильным распределением
            overdueTable->setColumnWidth(0, 180);  // Абонент
            overdueTable->setColumnWidth(1, 120);  // Телефон
            overdueTable->setColumnWidth(2, 250);  // Название книги
            overdueTable->setColumnWidth(3, 110);  // Дата взятия
            overdueTable->setColumnWidth(4, 110);  // Дата возврата
            overdueTable->setColumnWidth(5, 120);  // Дней просрочки
            
            // Настраиваем растяжение колонок
            overdueTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
            overdueTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
            overdueTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // Название книги растягивается
            overdueTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
            overdueTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
            overdueTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
            overdueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
            overdueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
            overdueTable->setAlternatingRowColors(true);
            
            // Вычисляем количество дней просрочки и сортируем
            QDate currentDate = QDate::currentDate();
            QList<QPair<int, std::pair<const LibraryMember*, BorrowedBook>>> overdueWithDays;
            
            for (const auto& pair : overdue) {
                BorrowedBook book = pair.second;
                
                // Парсим дату возврата
                QDate returnDate = QDate::fromString(QString::fromStdString(book.returnDate), "yyyy-MM-dd");
                int daysOverdue = static_cast<int>(returnDate.daysTo(currentDate));
                
                overdueWithDays.append({daysOverdue, pair});
            }
            
            // Сортируем по количеству дней просрочки (по убыванию)
            std::sort(overdueWithDays.begin(), overdueWithDays.end(),
                     [](const QPair<int, std::pair<const LibraryMember*, BorrowedBook>>& a,
                        const QPair<int, std::pair<const LibraryMember*, BorrowedBook>>& b) {
                         return a.first > b.first;
                     });
            
            overdueTable->setRowCount(static_cast<int>(overdueWithDays.size()));
            
            // Заполняем таблицу
            for (int i = 0; i < static_cast<int>(overdueWithDays.size()); ++i) {
                const auto& [daysOverdue, memberBookPair] = overdueWithDays[i];
                const LibraryMember* member = memberBookPair.first;
                const BorrowedBook& book = memberBookPair.second;
                
                // Находим название книги
                QString bookTitle = QString("ID: %1").arg(book.bookId);
                for (const auto* b : allBooks) {
                    if (b->getId() == book.bookId) {
                        bookTitle = QString::fromStdString(b->getTitle());
                        break;
                    }
                }
                
                // Заполняем строку
                auto* memberItem = new QTableWidgetItem(QString("%1 (ID: %2)")
                    .arg(QString::fromStdString(member->getFullName()))
                    .arg(member->getId()));
                overdueTable->setItem(i, 0, memberItem);
                
                auto* phoneItem = new QTableWidgetItem(QString::fromStdString(member->getPhone()));
                overdueTable->setItem(i, 1, phoneItem);
                
                auto* bookItem = new QTableWidgetItem(bookTitle);
                overdueTable->setItem(i, 2, bookItem);
                
                auto* borrowDateItem = new QTableWidgetItem(QString::fromStdString(book.borrowDate));
                overdueTable->setItem(i, 3, borrowDateItem);
                
                auto* returnDateItem = new QTableWidgetItem(QString::fromStdString(book.returnDate));
                overdueTable->setItem(i, 4, returnDateItem);
                
                // Количество дней просрочки
                auto* daysItem = new QTableWidgetItem(QString::number(daysOverdue));
                daysItem->setTextAlignment(Qt::AlignCenter);
                overdueTable->setItem(i, 5, daysItem);
            }
            
            mainLayout->addWidget(overdueTable);
        }
        
        // Кнопка закрытия
        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, overdueDialog);
        connect(buttonBox, &QDialogButtonBox::rejected, overdueDialog, &QDialog::reject);
        mainLayout->addWidget(buttonBox);
        
        overdueDialog->exec();
        
        // Убрано обновление текстового поля в операциях
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onAddEmployee()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить работника");
    auto* mainLayout = new QVBoxLayout(&dialog);
    
    // Выбор типа работника
    auto* typeGroup = new QGroupBox("Тип работника", &dialog);
    auto* typeLayout = new QVBoxLayout(typeGroup);
    auto* librarianRadio = new QRadioButton("Библиотекарь", typeGroup);
    auto* managerRadio = new QRadioButton("Менеджер", typeGroup);
    librarianRadio->setChecked(true); // По умолчанию выбран библиотекарь
    typeLayout->addWidget(librarianRadio);
    typeLayout->addWidget(managerRadio);
    mainLayout->addWidget(typeGroup);
    
    // Форма с полями
    auto* form = new QFormLayout();
    auto* nameEdit = new QLineEdit(&dialog);
    auto* surnameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* salaryEdit = new QDoubleSpinBox(&dialog);
    salaryEdit->setRange(0, 1000000);
    salaryEdit->setValue(50000);
    auto* workHoursEdit = new QSpinBox(&dialog);
    workHoursEdit->setRange(1, 100);
    workHoursEdit->setValue(40);
    
    form->addRow("Имя:", nameEdit);
    form->addRow("Фамилия:", surnameEdit);
    form->addRow("Телефон:", phoneEdit);
    form->addRow("Зарплата:", salaryEdit);
    form->addRow("Часы работы:", workHoursEdit);
    mainLayout->addLayout(form);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    mainLayout->addWidget(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            if (librarianRadio->isChecked()) {
                librarySystem.addLibrarian(
                    nameEdit->text().toStdString(),
                    surnameEdit->text().toStdString(),
                    phoneEdit->text().toStdString(),
                    salaryEdit->value(),
                    workHoursEdit->value()
                );
                showInfo("Библиотекарь успешно добавлен");
            } else {
                librarySystem.addManager(
                    nameEdit->text().toStdString(),
                    surnameEdit->text().toStdString(),
                    phoneEdit->text().toStdString(),
                    salaryEdit->value(),
                    workHoursEdit->value()
                );
                showInfo("Менеджер успешно добавлен");
            }
            refreshEmployees();
            autoSave();
            updateUndoRedoButtons();
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onAddLibrarian()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить библиотекаря");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    auto* surnameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* salaryEdit = new QDoubleSpinBox(&dialog);
    salaryEdit->setRange(0, 1000000);
    salaryEdit->setValue(50000);
    auto* workHoursEdit = new QSpinBox(&dialog);
    workHoursEdit->setRange(1, 100);
    workHoursEdit->setValue(40);
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Зарплата:", salaryEdit);
    form.addRow("Часы работы:", workHoursEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            librarySystem.addLibrarian(
                nameEdit->text().toStdString(),
                surnameEdit->text().toStdString(),
                phoneEdit->text().toStdString(),
                salaryEdit->value(),
                workHoursEdit->value()
            );
            refreshEmployees();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Библиотекарь успешно добавлен");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onAddManager()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить менеджера");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    auto* surnameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* salaryEdit = new QDoubleSpinBox(&dialog);
    salaryEdit->setRange(0, 1000000);
    salaryEdit->setValue(80000);
    auto* workHoursEdit = new QSpinBox(&dialog);
    workHoursEdit->setRange(1, 100);
    workHoursEdit->setValue(40);
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Зарплата:", salaryEdit);
    form.addRow("Часы работы:", workHoursEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            librarySystem.addManager(
                nameEdit->text().toStdString(),
                surnameEdit->text().toStdString(),
                phoneEdit->text().toStdString(),
                salaryEdit->value(),
                workHoursEdit->value()
            );
            refreshEmployees();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Менеджер успешно добавлен");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onEditEmployee()
{
    const auto* employeesTable = findChild<QTableWidget*>("employeesTable");
    if (!employeesTable) return;
    
    int currentRow = employeesTable->currentRow();
    if (currentRow < 0) {
        showError("Выберите работника для редактирования");
        return;
    }
    
    // Получаем ID из UserRole первой колонки (Имя)
    const auto* item = employeesTable->item(currentRow, 0);
    if (!item) return;
    
    int employeeId = item->data(Qt::UserRole).toInt();
    if (employeeId <= 0) return;
    
    const Employee* emp = nullptr;
    for (const auto* e : librarySystem.getAllEmployees()) {
        if (e->getId() == employeeId) {
            emp = e;
            break;
        }
    }
    
    if (!emp) {
        showError("Работник не найден");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать работника");
    QFormLayout form(&dialog);
    
    auto* nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(QString::fromStdString(emp->getName()));
    auto* surnameEdit = new QLineEdit(&dialog);
    surnameEdit->setText(QString::fromStdString(emp->getSurname()));
    auto* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setText(QString::fromStdString(emp->getPhone()));
    auto* salaryEdit = new QDoubleSpinBox(&dialog);
    salaryEdit->setRange(0, 1000000);
    salaryEdit->setValue(emp->getSalary());
    auto* workHoursEdit = new QSpinBox(&dialog);
    workHoursEdit->setRange(1, 100);
    workHoursEdit->setValue(emp->getWorkHours());
    
    form.addRow("Имя:", nameEdit);
    form.addRow("Фамилия:", surnameEdit);
    form.addRow("Телефон:", phoneEdit);
    form.addRow("Зарплата:", salaryEdit);
    form.addRow("Часы работы:", workHoursEdit);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        try {
            librarySystem.editEmployee(
                employeeId,
                nameEdit->text().toStdString(),
                surnameEdit->text().toStdString(),
                phoneEdit->text().toStdString(),
                salaryEdit->value(),
                workHoursEdit->value()
            );
            refreshEmployees();
            autoSave(); // Автосохранение после изменения
            updateUndoRedoButtons();
            showInfo("Работник успешно отредактирован");
        } catch (const LibraryException& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onShowEmployees()
{
    refreshEmployees();
}

void MainWindow::onUndo()
{
    // Определяем текущую активную вкладку
    int currentTab = ui->tabWidget->currentIndex();
    
    try {
        if (currentTab == 0) { // Книги
            if (!librarySystem.canUndoBooks()) {
                showError("Нет действий для отмены");
                return;
            }
            librarySystem.undoBooks();
            refreshBooks();
        } else if (currentTab == 1) { // Абоненты
            if (!librarySystem.canUndoMembers()) {
                showError("Нет действий для отмены");
                return;
            }
            librarySystem.undoMembers();
            refreshMembers();
        } else if (currentTab == 2) { // Работники
            if (!librarySystem.canUndoEmployees()) {
                showError("Нет действий для отмены");
                return;
            }
            librarySystem.undoEmployees();
            refreshEmployees();
        } else {
            showError("Нет действий для отмены в этой вкладке");
            return;
        }
        
        updateUndoRedoButtons();
        showInfo("Действие отменено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onRedo()
{
    // Определяем текущую активную вкладку
    int currentTab = ui->tabWidget->currentIndex();
    
    try {
        if (currentTab == 0) { // Книги
            if (!librarySystem.canRedoBooks()) {
                showError("Нет действий для повторения");
                return;
            }
            librarySystem.redoBooks();
            refreshBooks();
        } else if (currentTab == 1) { // Абоненты
            if (!librarySystem.canRedoMembers()) {
                showError("Нет действий для повторения");
                return;
            }
            librarySystem.redoMembers();
            refreshMembers();
        } else if (currentTab == 2) { // Работники
            if (!librarySystem.canRedoEmployees()) {
                showError("Нет действий для повторения");
                return;
            }
            librarySystem.redoEmployees();
            refreshEmployees();
        } else {
            showError("Нет действий для повторения в этой вкладке");
            return;
        }
        
        updateUndoRedoButtons();
        showInfo("Действие повторено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onUndoBooks()
{
    if (!librarySystem.canUndoBooks()) {
        showError("Нет действий для отмены");
        return;
    }
    
    try {
        librarySystem.undoBooks();
        refreshBooks();
        updateUndoRedoButtons();
        showInfo("Действие отменено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onRedoBooks()
{
    if (!librarySystem.canRedoBooks()) {
        showError("Нет действий для повторения");
        return;
    }
    
    try {
        librarySystem.redoBooks();
        refreshBooks();
        updateUndoRedoButtons();
        showInfo("Действие повторено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onUndoMembers()
{
    if (!librarySystem.canUndoMembers()) {
        showError("Нет действий для отмены");
        return;
    }
    
    try {
        librarySystem.undoMembers();
        refreshMembers();
        updateUndoRedoButtons();
        showInfo("Действие отменено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onRedoMembers()
{
    if (!librarySystem.canRedoMembers()) {
        showError("Нет действий для повторения");
        return;
    }
    
    try {
        librarySystem.redoMembers();
        refreshMembers();
        updateUndoRedoButtons();
        showInfo("Действие повторено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onUndoEmployees()
{
    if (!librarySystem.canUndoEmployees()) {
        showError("Нет действий для отмены");
        return;
    }
    
    try {
        librarySystem.undoEmployees();
        refreshEmployees();
        updateUndoRedoButtons();
        showInfo("Действие отменено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onRedoEmployees()
{
    if (!librarySystem.canRedoEmployees()) {
        showError("Нет действий для повторения");
        return;
    }
    
    try {
        librarySystem.redoEmployees();
        refreshEmployees();
        updateUndoRedoButtons();
        showInfo("Действие повторено");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onSave()
{
    try {
        FileManager::saveLibrarySystem(librarySystem, dataPath.toStdString());
        showInfo("Данные успешно сохранены");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onLoad()
{
    try {
        FileManager::loadLibrarySystem(librarySystem, dataPath.toStdString());
        refreshBooks();
        refreshMembers();
        refreshEmployees();
        showInfo("Данные успешно загружены");
    } catch (const LibraryException& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showInfo(const QString& message)
{
    QMessageBox::information(this, "Информация", message);
}

// Контекстное меню для книг больше не используется, действия вынесены в отдельную колонку

void MainWindow::autoSave()
{
    // Автоматическое сохранение без сообщений
    try {
        FileManager::saveLibrarySystem(librarySystem, dataPath.toStdString());
    } catch (const std::exception& e) {
        // Игнорируем ошибки автосохранения, чтобы не мешать работе пользователя
        (void)e; // Suppress unused variable warning
    }
}

void MainWindow::updateUndoRedoButtons() const
{
    // Определяем текущую активную вкладку и обновляем кнопки соответственно
    int currentTab = ui->tabWidget->currentIndex();
    
    QPushButton* undoBtn = findChild<QPushButton*>("undoButton");
    QPushButton* redoBtn = findChild<QPushButton*>("redoButton");
    
    if (currentTab == 0) { // Книги
        if (undoBtn) undoBtn->setEnabled(librarySystem.canUndoBooks());
        if (redoBtn) redoBtn->setEnabled(librarySystem.canRedoBooks());
    } else if (currentTab == 1) { // Абоненты
        if (undoBtn) undoBtn->setEnabled(librarySystem.canUndoMembers());
        if (redoBtn) redoBtn->setEnabled(librarySystem.canRedoMembers());
    } else if (currentTab == 2) { // Работники
        if (undoBtn) undoBtn->setEnabled(librarySystem.canUndoEmployees());
        if (redoBtn) redoBtn->setEnabled(librarySystem.canRedoEmployees());
    } else {
        if (undoBtn) undoBtn->setEnabled(false);
        if (redoBtn) redoBtn->setEnabled(false);
    }
}

QIcon MainWindow::createRedCrossIcon()
{
    // Используем стандартную иконку крестика и делаем её красной
    QIcon standardIcon = style()->standardIcon(QStyle::SP_DialogCloseButton);
    
    if (QPixmap pixmap = standardIcon.pixmap(22, 22); !pixmap.isNull()) {
        QPixmap redPixmap = pixmap;
        QPainter painter(&redPixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(redPixmap.rect(), QColor(255, 0, 0));
        return QIcon(redPixmap);
    }
    
    // Если стандартная иконка недоступна, создаём свою
    QPixmap customPixmap(22, 22);
    customPixmap.fill(Qt::transparent);
    QPainter customPainter(&customPixmap);
    customPainter.setRenderHint(QPainter::Antialiasing);
    customPainter.setPen(QPen(QColor(255, 0, 0), 2.5));
    customPainter.drawLine(5, 5, 17, 17);
    customPainter.drawLine(17, 5, 5, 17);
    return QIcon(customPixmap);
}

// Контекстное меню для абонентов больше не используется, действия вынесены в отдельную колонку

// Контекстное меню для сотрудников больше не используется, действия вынесены в отдельную колонку

void MainWindow::onMemberFilterChanged()
{
    // Обновляем фильтры из полей ввода
    QLineEdit* nameFilter = findChild<QLineEdit*>("memberNameFilter");
    QLineEdit* surnameFilter = findChild<QLineEdit*>("memberSurnameFilter");
    QLineEdit* phoneFilter = findChild<QLineEdit*>("memberPhoneFilter");
    QLineEdit* emailFilter = findChild<QLineEdit*>("memberEmailFilter");
    QComboBox* blockedFilter = findChild<QComboBox*>("memberBlockedFilter");
    
    if (nameFilter) memberFilters.name = nameFilter->text();
    if (surnameFilter) memberFilters.surname = surnameFilter->text();
    if (phoneFilter) memberFilters.phone = phoneFilter->text();
    if (emailFilter) memberFilters.email = emailFilter->text();
    if (blockedFilter) memberFilters.blocked = blockedFilter->currentData().toInt();
    
    refreshMembers();
}

void MainWindow::onClearMemberFilters()
{
    // Очищаем все фильтры
    memberFilters = MemberFilters();
    
    // Очищаем поля ввода
    QLineEdit* nameFilter = findChild<QLineEdit*>("memberNameFilter");
    QLineEdit* surnameFilter = findChild<QLineEdit*>("memberSurnameFilter");
    QLineEdit* phoneFilter = findChild<QLineEdit*>("memberPhoneFilter");
    QLineEdit* emailFilter = findChild<QLineEdit*>("memberEmailFilter");
    QComboBox* blockedFilter = findChild<QComboBox*>("memberBlockedFilter");
    
    if (nameFilter) nameFilter->clear();
    if (surnameFilter) surnameFilter->clear();
    if (phoneFilter) phoneFilter->clear();
    if (emailFilter) emailFilter->clear();
    if (blockedFilter) blockedFilter->setCurrentIndex(0);
    
    refreshMembers();
}

