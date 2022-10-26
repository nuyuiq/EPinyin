#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "ime/epinyin.h"
class QModelIndex;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_lineEdit_editingFinished();

    void on_pushButton_clicked();

    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    void updateCandidates();

    IME::EPinyin *epy;
    Ui::Widget *ui;
};

#endif // WIDGET_H
