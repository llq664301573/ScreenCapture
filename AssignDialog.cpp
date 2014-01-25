#include "AssignDialog.h"

AssignDialog::AssignDialog(QWidget *parent/* =0 */): QDialog(parent)
{
    alreadyRegister = false;
    identifier = 0x0000;

    QFormLayout *form = new QFormLayout();
    form->setSizeConstraint( QLayout::SetFixedSize );

    shiftModifier = new QCheckBox("ShiftModifier");
    controlModifier = new QCheckBox("ControlModifier");
    altModifier = new QCheckBox("AltModifier");
    form->addRow(shiftModifier);
    form->addRow(controlModifier);
    form->addRow(altModifier);

    virtualKey = new QLineEdit();
    virtualKey->setReadOnly(true);
    form->addRow("Virtual-Key", virtualKey);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    form->addRow(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(form);

    show();
}

AssignDialog::~AssignDialog()
{
    if(alreadyRegister)
        ::UnregisterHotKey(NULL, identifier);
}

void AssignDialog::accept()
{
    if(virtualKey->text().isEmpty())
    {
        QMessageBox::information(this, "",  QString::fromLocal8Bit("ÐéÄâ¼üÂë²»ÄÜÎª¿Õ£¡"));
        return ;
    }

    int modifiers = 0;
    if(shiftModifier->isChecked())
        modifiers |= MOD_SHIFT;
    if(controlModifier->isChecked())
        modifiers |= MOD_CONTROL;
    if(altModifier->isChecked())
        modifiers |= MOD_ALT;

    if(alreadyRegister)
    {
        ::UnregisterHotKey(NULL, identifier);
        alreadyRegister = false;
    }

    if(!::RegisterHotKey(NULL, identifier, modifiers, virtualKey->text().toInt(0, 16)))
    {
        QString text;
        text.sprintf("RegisterHotKey failed with error %d:", ::GetLastError());
        QMessageBox::information(this, "",  text);
    }
    else
        alreadyRegister = true;

    QDialog::accept();
}

void AssignDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Shift:
        shiftModifier->setChecked(true);
        break;
    case Qt::Key_Control:
        controlModifier->setChecked(true);
        break;
    case Qt::Key_Alt:
        altModifier->setChecked(true);
        break;
    default:
        QString key;
        key.sprintf("0x%x", event->key());
        virtualKey->setText(key);
        break;
    }

    QDialog::keyPressEvent(event);
}

void AssignDialog::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Shift:
        shiftModifier->setChecked(false);
        break;
    case Qt::Key_Control:
        controlModifier->setChecked(false);
        break;
    case Qt::Key_Alt:
        altModifier->setChecked(false);
        break;
    default:
        break;
    }

    QDialog::keyReleaseEvent(event);
}