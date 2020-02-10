#include "Clipboard.hpp"

#include <QApplication>
#include <QClipboard>

Clipboard::Clipboard()
{
    QObject::connect(
        QApplication::clipboard(),
        &QClipboard::dataChanged,
        [this]()
        {
            text.clear();
            text.push_back(Text());
        }
    );
}

Clipboard & Clipboard::Instance()
{
    static Clipboard clipboard;
    return clipboard;
}

String32 Clipboard::Text() const
{
    return QApplication::clipboard()->text().toStdU32String();
}

Vector<String32> const& Clipboard::MultiText() const
{
    return text;
}

void Clipboard::SetText(StringView32 txt)
{
    QApplication::clipboard()->setText(QString::fromUcs4(txt.data(), txt.size()));
}

void Clipboard::SetMultiText(Vector<String32> txt)
{
    QString qtext;
    for (StringView32 t : txt)
    {
        qtext += QString::fromUcs4(t.data(), t.size());
        qtext += '\n';
    }

    QApplication::clipboard()->setText(qtext);

    text = std::move(txt);
}
