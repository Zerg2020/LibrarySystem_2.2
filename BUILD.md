# Инструкция по сборке проекта через CMake

## Требования

- CMake версии 3.16 или выше
- Qt5 или Qt6 (Core, Gui, Widgets)
- Компилятор C++17 (MSVC, GCC, Clang)

## Сборка проекта

### Windows (Visual Studio)

```bash
# Создаем папку для сборки
mkdir build
cd build

# Генерируем проект для Visual Studio
cmake .. -G "Visual Studio 17 2022" -A x64

# Или для более старой версии
cmake .. -G "Visual Studio 16 2019" -A x64

# Собираем проект
cmake --build . --config Release
```

Исполняемый файл будет находиться в `build/bin/Release/LibrarySystem.exe` (или `build/bin/Debug/LibrarySystem.exe` для Debug конфигурации).

### Windows (MinGW)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

### Linux

```bash
mkdir build
cd build
cmake ..
make
```

Исполняемый файл будет находиться в `build/bin/LibrarySystem`.

### macOS

```bash
mkdir build
cd build
cmake ..
make
```

## Настройка Qt

Если CMake не находит Qt автоматически, укажите путь к Qt:

```bash
cmake .. -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
# или для Qt5
cmake .. -DQt5_DIR=/path/to/qt5/lib/cmake/Qt5
```

## Запуск приложения

После сборки запустите приложение из папки `build/bin`. Приложение автоматически создаст папку `data` для хранения данных библиотеки.

## Структура папок после сборки

```
build/
├── bin/
│   ├── LibrarySystem.exe (или LibrarySystem)
│   └── data/
│       ├── books.txt
│       ├── members.txt
│       ├── employees.txt
│       ├── metadata.txt
│       ├── covers/
│       └── pdfs/
└── ...
```


