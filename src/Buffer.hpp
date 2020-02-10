#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "TextContainer.hpp"
#include "TextView.hpp"

#include "String32.hpp"
#include "Vector.hpp"

#include "Cursor.hpp"

#include "Painter.hpp"

#include "Lexer.hpp"

enum WhitespaceFlag
{
    EXPAND_TABS
};

class Buffer : public TextContainer<Buffer>
{
private:
    friend class TextContainer<Buffer>;

    Lexer * lexer;

    Vector<Cursor> cursors;
    Vector<String32> lines;
    Vector<Vector<int>> styles;

    // TODO@Daniel:
    //  Cleanup font stuff
    QFont font;
    QFontMetrics metrics;
    QSize cell_size;
    int baseline;

    Position style_pos;

    int flags = EXPAND_TABS;

protected:
    void SetText(TextView const& text);

    Position DeleteAdjustedPosition(Position start, Position stop, Position pos);
    Position NewlineAdjustedPosition(Position insertion_pos, Position pos);

    void PaintTextMargin(Painter & painter, int scroll);
    void PaintLineNumberMargin(Painter & painter, int scroll);

    void UpdateFontMetrics();

public:
    Buffer();

    bool Flag(int flag);
    void SetFlag(int flag, bool value = true);

    int SelectionSizeTotal();

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

    int CursorDeletePrev();
    int CursorDeleteNext();

    int CursorDeleteToPrevBorder();
    int CursorDeleteToNextBorder();

    int CursorReplaceText(TextView const& text);

    int CursorInsertText(TextView const& text);

    int CursorDeleteSelection();

    int ConvertTabsToSpaces();

    void ConsolidateCursors();

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
    void AddCursor(Cursor cursor);

    int CursorCount();

    Cursor FirstCursor();
    Cursor LastCursor();

    void AddLineSelection(int line_idx);
    void AddWordSelection(Position pos);

    void SetPointSize(int size);
    void SetFontName(QString const& name);

    int DoPaste();
    int DoCut();
    void DoCopy();

    void ZoomIn(int amount = 1);
    void ZoomOut(int amount = 1);

    void Paint(Painter & painter, int scroll);
};

#endif // BUFFER_HPP
