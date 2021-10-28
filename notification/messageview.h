#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextBrowser>
#include <QLabel>
#include "msgbean.h"

#define MESSAGE_LABEL

#ifdef MESSAGE_LABEL
class MessageView : public QLabel
#else
class MessageView : public QTextBrowser
#endif
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg);
    static QString simpleMessage(const MsgBean &msg);

    QSize adjustSizeByTextWidth(int w);

    void setTextColor(QColor c);

    void markDeleted();

    void markSelected();

    QSize sizeHint() const override;

private:
    void updateStyleSheet();

signals:
    void needMemberNames();
    void keepShowing();
    void restoreTimerIfNecessary();
    void replyText(const QString& text);
    void sendText(const QString& text);
    void focusMessage(const qint64 messageId);
    void heightChanged();

public slots:
    void replaceGroupAt();

    void showMenu();

private:
    MsgBean msg;
    int fixedWidth = 0;
    QColor textColor = Qt::black;

    QString filePath;
    QPixmap filePixmap;

    bool deleted = false;
    bool selected = false;
};

#endif // MESSAGEEDIT_H
