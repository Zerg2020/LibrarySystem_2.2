# Library Management System

Система управления библиотекой на C++ и Qt.

## Структура проекта

```
LibrarySystem_1/
├── include/          # Заголовочные файлы (.h)
│   ├── book.h
│   ├── command.h
│   ├── commandmanager.h
│   ├── employee.h
│   ├── exceptions.h
│   ├── filemanager.h
│   ├── item.h
│   ├── librarian.h
│   ├── librarycontainer.h
│   ├── librarymember.h
│   ├── librarysystem.h
│   ├── mainwindow.h
│   ├── manager.h
│   ├── membercontainer.h
│   └── person.h
├── src/              # Исходные файлы (.cpp)
│   ├── book.cpp
│   ├── commandmanager.cpp
│   ├── employee.cpp
│   ├── filemanager.cpp
│   ├── item.cpp
│   ├── librarian.cpp
│   ├── librarycontainer.cpp
│   ├── librarymember.cpp
│   ├── librarysystem.cpp
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── manager.cpp
│   ├── membercontainer.cpp
│   └── person.cpp
├── forms/            # UI формы Qt (.ui)
│   └── mainwindow.ui
├── data/             # Данные библиотеки (создается автоматически)
│   ├── books.txt
│   ├── members.txt
│   ├── employees.txt
│   └── metadata.txt
├── LibrarySystem.pro # Файл проекта Qt (qmake)
├── CMakeLists.txt    # Файл проекта CMake
└── README.md
```

## Иерархия классов

### Иерархия Person (3 уровня наследования)
1. **Person** (базовый класс)
   - **LibraryMember** (абонент библиотеки)
   - **Employee** (работник)
     - **Librarian** (библиотекарь)
     - **Manager** (менеджер)

### Иерархия Item
1. **Item** (базовый класс)
   - **Book** (книга)

## Основные возможности

- ✅ Просмотр доступных книг
- ✅ Управление книгами (добавление, удаление)
- ✅ Список абонентов библиотеки
- ✅ Просмотр книг на абоненте с датой взятия
- ✅ Просмотр задолженностей по возврату книги
- ✅ Временная блокировка абонента или полное удаление
- ✅ Добавление нового абонента с автоматической генерацией уникального номера
- ✅ Организация труда работников
- ✅ Отмена последних действий (Undo/Redo)
- ✅ Сохранение и загрузка данных из файлов
- ✅ Обработка исключительных ситуаций

## Использованные технологии

- **C++17** с STL контейнерами и алгоритмами
- **Qt Framework** для пользовательского интерфейса
- **Паттерн Command** для реализации Undo/Redo
- **Собственные контейнеры и итераторы** (LibraryContainer, MemberContainer)
- **Обработка исключений** через иерархию исключений

## Сборка проекта

### Сборка через CMake (рекомендуется)

1. Убедитесь, что установлены:
   - CMake версии 3.16 или выше
   - Qt5 или Qt6 (Core, Gui, Widgets)
   - Компилятор C++17

2. Создайте папку для сборки и сгенерируйте проект:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Соберите проект:
   ```bash
   # Windows (Visual Studio)
   cmake --build . --config Release
   
   # Linux/macOS
   make
   ```

4. Исполняемый файл будет находиться в `build/bin/`

Подробные инструкции по сборке см. в файле [BUILD.md](BUILD.md)

### Сборка через qmake (альтернативный способ)

1. Убедитесь, что установлен Qt (версия 5.x или 6.x)
2. Откройте `LibrarySystem.pro` в Qt Creator
3. Выберите подходящий компилятор
4. Нажмите "Build" или используйте qmake:
   ```bash
   qmake LibrarySystem.pro
   make
   ```

## Использование

1. Запустите приложение
2. Данные автоматически загружаются из папки `data/` при старте
3. Используйте вкладки для навигации:
   - **Books** - управление книгами
   - **Members** - управление абонентами
   - **Employees** - управление работниками
   - **Operations** - выдача/возврат книг и просмотр задолженностей

## Горячие клавиши

- `Ctrl+S` - Сохранить данные
- `Ctrl+O` - Загрузить данные
- `Ctrl+Z` - Отменить действие
- `Ctrl+Y` - Повторить действие

## CI/CD

Проект настроен для автоматической сборки и анализа кода:

- **GitHub Actions**: Автоматическая сборка на Ubuntu и Windows при каждом push и pull request
- **SonarCloud**: Автоматический анализ качества кода и поиск уязвимостей

### Настройка SonarCloud

1. Создайте проект на [SonarCloud](https://sonarcloud.io/)
2. Получите `SONAR_TOKEN` из настроек проекта
3. Добавьте `SONAR_TOKEN` в секреты GitHub репозитория (Settings → Secrets → Actions)
4. Обновите `sonar-project.properties` с вашими ключами проекта и организации

## Загрузка в GitHub

Для загрузки проекта в GitHub через Visual Studio см. [GITHUB_UPLOAD_VS.md](GITHUB_UPLOAD_VS.md) - подробная инструкция.

## Настройка SonarCloud

Для настройки автоматического анализа кода через SonarCloud:

1. **Быстрый старт**: См. [QUICK_START.md](QUICK_START.md) - минимальные шаги для запуска
2. **Подробная инструкция**: См. [SONARCLOUD_SETUP.md](SONARCLOUD_SETUP.md) - полное руководство с решением проблем

## Лицензия

Этот проект распространяется под лицензией MIT. См. файл [LICENSE](LICENSE) для подробностей.




