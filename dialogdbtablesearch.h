#ifndef DIALOGDBTABLESEARCH_H
#define DIALOGDBTABLESEARCH_H

#include <QDialog>

class QLineEdit;

class DialogDBTableSearch : public QDialog
{
    Q_OBJECT

private:
    QLineEdit* textSearch;
    QString* m_string;

public:
    explicit DialogDBTableSearch(QWidget* parent,
                                 const QString &in_string,
                                 QString *out_string);

private slots:
    void slotAccept();

};

#endif // DIALOGDBTABLESEARCH_H
