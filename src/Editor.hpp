#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "ui_Editor.h"

#include "Buffer.hpp"

class Editor : public QMainWindow, private Ui::Editor
{
    Q_OBJECT

private:
    Buffer buffer;

public:
    explicit Editor(QWidget * parent = nullptr);
};

#endif // EDITOR_HPP
