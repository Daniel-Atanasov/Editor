#include "Editor.hpp"

#include <QDebug>

#include <QWheelEvent>

#include "Painter.hpp"

Editor::Editor(QWidget * parent) :
    QMainWindow(parent)
{
    setupUi(this);
}
