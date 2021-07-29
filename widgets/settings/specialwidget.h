#ifndef SPECIALWIDGET_H
#define SPECIALWIDGET_H

#include <QWidget>

namespace Ui {
class SpecialWidget;
}

class SpecialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpecialWidget(QWidget *parent = nullptr);
    ~SpecialWidget();

private slots:
    void on_groupUseFriendImportanceCheck_clicked();

private:
    Ui::SpecialWidget *ui;
};

#endif // SPECIALWIDGET_H
