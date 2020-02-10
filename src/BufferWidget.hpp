#ifndef BUFFERWIDGET_HPP
#define BUFFERWIDGET_HPP

#include "ui_BufferWidget.h"

#include <QWidget>
#include <QTimer>

#include "HashMap.hpp"

#include "Buffer.hpp"
#include "LexerJass.hpp"

#include "KeyMap.hpp"

class BufferWidget : public QWidget, private Ui::BufferWidget
{
    Q_OBJECT

private:
    KeyMap keymap;

    Buffer buffer;
    LexerJass lexer;

    QTimer timer;

    int CellWidth();
    int CellHeight();

    QSize CellArea();

    int FirstVisibleLine();
    int LastVisibleLine();

    Position FirstVisiblePosition();
    Position LastVisiblePosition();

    void EnsureCursorIsVisible();

    void EnsureVisibleAreaIsStyled();

    int VScroll();
    void SetVScroll(int value);
    void ScrollUp(int amount = 1);
    void ScrollDown(int amount = 1);

    Position ScreenToCell(QPoint pt, bool round = true);

private slots:
    void UpdateScrollbar();

public:
    explicit BufferWidget(QWidget * parent = nullptr);

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);
    void keyPressEvent(QKeyEvent * event);
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent * event);
};

#endif // BUFFERWIDGET_HPP
