#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    epy = new IME::EPinyin(":/ime/dict_pinyin.dat");
}

Widget::~Widget()
{
    delete epy;
    delete ui;
}

void Widget::on_lineEdit_editingFinished()
{
    ui->listWidget->clear();
    epy->search(ui->lineEdit->text());
    updateCandidates();
}

void Widget::on_pushButton_clicked()
{
    updateCandidates();
}

void Widget::on_listWidget_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        epy->choose(index.row());
        ui->listWidget->clear();
        ui->label_2->setText(epy->getFixedStr());
        updateCandidates();
    }
}

void Widget::updateCandidates()
{
    const QStringList ls = epy->getCandidate(ui->listWidget->count(), 10);
    if (!ls.isEmpty())
    {
        ui->listWidget->addItems(ls);
    }
}

void Widget::on_pushButton_2_clicked()
{
    on_listWidget_doubleClicked(ui->listWidget->currentIndex());
}

void Widget::on_pushButton_3_clicked()
{
    ui->listWidget->clear();
    epy->cancelLastChoice();
    ui->label_2->setText(epy->getFixedStr());
    updateCandidates();
}
