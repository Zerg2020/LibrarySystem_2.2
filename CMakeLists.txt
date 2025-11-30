cmake_minimum_required(VERSION 3.16)

project(LibrarySystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Генерация compile_commands.json для анализаторов кода (SonarCloud, clangd и т.д.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Настройка путей
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Поиск Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Пробуем найти Qt6, если не найдено - ищем Qt5
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    set(QT_VERSION_MAJOR 5)
endif()

# Включаемые директории
include_directories(${CMAKE_SOURCE_DIR}/include)

# Настройка путей поиска для AUTOUIC
# CMake будет искать UI файлы в этих директориях при обработке #include "ui_*.h"
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/forms)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/person.cpp
    src/librarymember.cpp
    src/employee.cpp
    src/librarian.cpp
    src/manager.cpp
    src/item.cpp
    src/book.cpp
    src/librarycontainer.cpp
    src/membercontainer.cpp
    src/librarysystem.cpp
    src/filemanager.cpp
    src/commandmanager.cpp
)

# Заголовочные файлы
set(HEADERS
    include/mainwindow.h
    include/person.h
    include/librarymember.h
    include/employee.h
    include/librarian.h
    include/manager.h
    include/item.h
    include/book.h
    include/librarycontainer.h
    include/membercontainer.h
    include/librarysystem.h
    include/filemanager.h
    include/command.h
    include/commandmanager.h
    include/exceptions.h
    include/commands.h
)

# UI файлы (относительные пути)
set(UI_FORMS
    forms/mainwindow.ui
)

# Создание исполняемого файла
add_executable(LibrarySystem ${SOURCES} ${HEADERS} ${UI_FORMS})

# Подключение Qt библиотек
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(LibrarySystem Qt6::Core Qt6::Gui Qt6::Widgets)
else()
    target_link_libraries(LibrarySystem Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

# Копирование папки data в bin после сборки
add_custom_command(TARGET LibrarySystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/data/covers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/data/pdfs
    COMMENT "Создание папок для данных"
)

# Копируем существующие данные, если они есть
if(EXISTS ${CMAKE_SOURCE_DIR}/bin/data)
    add_custom_command(TARGET LibrarySystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/bin/data
            ${CMAKE_BINARY_DIR}/bin/data
        COMMENT "Копирование данных из bin/data"
    )
elseif(EXISTS ${CMAKE_SOURCE_DIR}/data)
    add_custom_command(TARGET LibrarySystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data
            ${CMAKE_BINARY_DIR}/bin/data
        COMMENT "Копирование данных из data"
    )
endif()

# Настройка для Windows
if(WIN32)
    set_target_properties(LibrarySystem PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    # Указываем Visual Studio, что это startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LibrarySystem)
endif()

