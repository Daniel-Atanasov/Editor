#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <QString>

#include <QKeyEvent>

#include "String32.hpp"
#include "Vector.hpp"

#include "Cursor.hpp"

#include "Painter.hpp"

#include "Lexer.hpp"

class Buffer
{
private:
    Lexer * lexer;

    Vector<Cursor> cursors;
    Vector<String32> lines;
    Vector<Vector<int>> styles;

    QFont font;
    QSize cell_size;

    Position style_pos;

protected:
    void SetText(QString const& text);

    Position FirstPosition();
    Position LastPosition();

    Position DeleteAdjustedPosition(Position start, Position stop, Position pos);
    Position NewlineAdjustedPosition(Position insertion_pos, Position pos);

    int DistanceToPrevBorder(Position pos);
    int DistanceToNextBorder(Position pos);

    void RemoveText(Cursor & cursor);

    void MoveLeft(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void MoveUp(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void MoveRight(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void MoveDown(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void MoveHome(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void MoveEnd(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void SelectAllOld(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void DeleteNext(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void DeletePrev(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    void InsertNewLine(Cursor & cursor, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void Insert(Cursor & cursor, char32_t ch, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void Insert(Cursor & cursor, String32 const& text, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    void HandleCursorInput(QKeyEvent * event);
    void HandleKeyboardInput(QKeyEvent * event);

    void PaintTextMargin(Painter & painter, int scroll);
    void PaintLineNumberMargin(Painter & painter, int scroll);

    void UpdateCellSize();

public:
    Buffer();

    char32_t CharacterAt(Position pos);

    int TextSize();
    int TextSize(Position start);
    int TextSize(Position start, Position stop);

    String32 Text();
    String32 Text(Position start);
    String32 Text(Position start, Position stop);

    int LineCount();
    int LineLength(int line_idx);
    int MaximumLineLength();

    Position LineStart(int line_idx);
    Position LineEnd(int line_idx);

    void SelectionClear();

    void CursorAdjustToMinPosition();
    void CursorAdjustToMaxPosition();

    void CursorAdjustLeft(int count = 1);
    void CursorAdjustUp(int count = 1);
    void CursorAdjustRight(int count = 1);
    void CursorAdjustDown(int count = 1);

    void CursorAdjustToPrevBorder();
    void CursorAdjustToNextBorder();

    void CursorAdjustToLineStart();
    void CursorAdjustToLineEnd();

    void CursorAdjustToBufferStart();
    void CursorAdjustToBufferEnd();

    void CursorMoveLeft(int count = 1);
    void CursorMoveUp(int count = 1);
    void CursorMoveRight(int count = 1);
    void CursorMoveDown(int count = 1);

    void CursorMoveToPrevBorder();
    void CursorMoveToNextBorder();

    void CursorMoveToLineStart();
    void CursorMoveToLineEnd();

    void CursorMoveToBufferStart();
    void CursorMoveToBufferEnd();

    void CursorSelectAll();

    void CursorCloneUp();
    void CursorCloneDown();

    void CursorDeletePrev();
    void CursorDeleteNext();

    void CursorDeleteToPrevBorder();
    void CursorDeleteToNextBorder();

    void CursorReplaceText(String32 const& text);
    void CursorReplaceText(Vector<String32> const& text);

    void CursorInsertText(String32 const& text);
    void CursorInsertText(Vector<String32> const& text);

    void CursorDeleteSelection();

    void ConsolidateCursors();

    bool IsOnBorder(Position pos);

    Position PrevPosition(Position pos, int count = 1);
    Position NextPosition(Position pos, int count = 1);

    int StyleAt(Position pos);

    void StartStyling(Position pos);
    void SetStyle(int count, int style);

    void EnsureStyled(Position pos);
    void EnsureStyled(int line_idx);

    void InvalidateStyles();
    void InvalidateStyles(Position pos);
    void InvalidateStyles(int line_idx);

    void SetLexer(Lexer * new_lexer);

    QSize const& CellSize();

    int LineNumberMarginWidth();

    void ClearCursors();
    void AddCursor(const Cursor &cursor);

    Cursor const& FirstCursor();
    Cursor const& LastCursor();

    void AddLineSelection(int line_idx);
    void AddWordSelection(Position pos);

    void SetPointSize(int size);
    void SetFontName(QString const& name);

    void DoPaste();
    void DoCopy();

    void ZoomIn(int amount = 1);
    void ZoomOut(int amount = 1);

    void KeyPress(QKeyEvent * event);
    void Paint(Painter & painter, int scroll);
};

#endif // BUFFER_HPP
