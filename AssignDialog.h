#pragma once

#include <QtWidgets>

class AssignDialog: public QDialog
{
    Q_OBJECT
public:
    explicit AssignDialog(QWidget *parent = 0);
    ~AssignDialog();

public Q_SLOTS:
    virtual void accept();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

private:
    QCheckBox *shiftModifier;
    QCheckBox *controlModifier;
    QCheckBox *altModifier;
    QLineEdit *virtualKey;

    bool alreadyRegister;
    int identifier;
};