#include <QApplication>

#include "Clipboard.hpp"
#include "Editor.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Clipboard::Instance();
    Editor w;
    w.show();

    return a.exec();
}
