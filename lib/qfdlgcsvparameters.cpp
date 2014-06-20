#include "qfdlgcsvparameters.h"
#include <QtGui>
#include "ui_qfdlg_csvparameters.h"


QFDlgCSVParameters::QFDlgCSVParameters(QWidget* parent, QString columnSeparator, QString decimalSeparator, QString commentStart, QString headerStart):
    QDialog(parent), ui(new Ui::QFDlgCSVParameters)
{
    ui->setupUi(this);
    ui->edtColumn->setText(QString(columnSeparator));
    if (QString(columnSeparator).isEmpty()) ui->edtColumn->setEnabled(false);
    ui->edtDecimal->setText(QString(decimalSeparator));
    if (QString(decimalSeparator).isEmpty()) ui->edtDecimal->setEnabled(false);
    ui->edtComment->setText(QString(commentStart));
    if (QString(commentStart).isEmpty()) ui->edtComment->setEnabled(false);
    ui->edtHeader->setText(headerStart);
    if (headerStart.isEmpty()) ui->edtHeader->setEnabled(false);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(checkValues()));
}

QFDlgCSVParameters::~QFDlgCSVParameters()
{
    //dtor
}


void QFDlgCSVParameters::setFileContents(const QString& filename) {
    QString preview="";
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            preview=preview + in.readLine() +"\n";
        }
        file.close();
    }
    ui->txtContents->setPlainText(preview);
}

void QFDlgCSVParameters::checkValues() {
    bool ok1=(!ui->edtColumn->isEnabled()) || ( (ui->edtColumn->text().size()>0) && (ui->edtColumn->text()!=ui->edtComment->text()) && (ui->edtColumn->text()!=ui->edtDecimal->text()) );
    bool ok2=(!ui->edtDecimal->isEnabled()) || ( (ui->edtDecimal->text().size()>0) && (ui->edtDecimal->text()!=ui->edtColumn->text()) && (ui->edtDecimal->text()!=ui->edtComment->text()) );
    bool ok3=(!ui->edtComment->isEnabled()) || ( (ui->edtComment->text().size()>0) && (ui->edtComment->text()!=ui->edtColumn->text()) && (ui->edtComment->text()!=ui->edtDecimal->text()) );
    bool ok4=(!ui->edtHeader->isEnabled()) || ( ui->edtHeader->text().size()>0 );
    if (!(ok1 && ok2 && ok3 && ok4)) {
        (void) QMessageBox::critical(this, tr("CSV Import Properties"), tr("The first three properties (column, decimal and comment separator) may not be set to the same character! Please correct the indicated fields."), QMessageBox::Ok);
        if (!ok1) {
            QPalette p=ui->edtColumn->palette();
            p.setColor(QPalette::Background, QColor("salmon"));
            ui->edtColumn->setPalette(p);
        } else ui->edtColumn->setPalette(QPalette());
        if (!ok2) {
            QPalette p=ui->edtComment->palette();
            p.setColor(QPalette::Background, QColor("salmon"));
            ui->edtDecimal->setPalette(p);
        } else ui->edtDecimal->setPalette(QPalette());
        if (!ok3) {
            QPalette p=ui->edtHeader->palette();
            p.setColor(QPalette::Background, QColor("salmon"));
            ui->edtComment->setPalette(p);
        } else ui->edtComment->setPalette(QPalette());
        if (!ok4) {
            QPalette p=ui->edtHeader->palette();
            p.setColor(QPalette::Background, QColor("salmon"));
            ui->edtHeader->setPalette(p);
        } else ui->edtHeader->setPalette(QPalette());
    } else {
        QString s=ui->edtColumn->text();
        column_separator=(s.size()>0)?s[0].toAscii():',';
        s=ui->edtDecimal->text();
        decimal_separator=(s.size()>0)?s[0].toAscii():'.';
        s=ui->edtComment->text();
        comment_start=(s.size()>0)?s[0].toAscii():'#';
        header_start=ui->edtHeader->text();

        accept();
    }
}

void QFDlgCSVParameters::guessParameters()
{

}