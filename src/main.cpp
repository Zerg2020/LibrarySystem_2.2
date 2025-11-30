#include "../include/mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Устанавливаем приятный шрифт для всего приложения
        // Используем Segoe UI для Windows (современный и приятный шрифт)
        // Qt автоматически выберет ближайший доступный шрифт, если Segoe UI не найден
        QFont defaultFont("Segoe UI", 10);
        QApplication::setFont(defaultFont);
    
    // Создаем папку для данных, если её нет
    if (QDir dir; !dir.exists("data")) {
        dir.mkpath("data");
    }
    
    MainWindow window;
    window.show();
    
    return QApplication::exec();
}





