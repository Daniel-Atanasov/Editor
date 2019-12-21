#include "Editor.hpp"

#include <QDebug>

#include <QWheelEvent>

#include "Painter.hpp"

Editor::Editor(QWidget * parent) :
    QMainWindow(parent)
{
    setupUi(this);

//    setContentsMargins(0, 0, 0, 0);
//    layout()->setContentsMargins(0, 0, 0, 0);
//    layout()->setSpacing(0);
}
