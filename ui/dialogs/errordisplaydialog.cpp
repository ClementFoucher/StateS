/*
 * Copyright © 2014-2015 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.txt.
 *
 *
 * This file is part of StateS.
 *
 * StateS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * StateS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "errordisplaydialog.h"

// Qt classes
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>


ErrorDisplayDialog::ErrorDisplayDialog(const QString& errorTitle, const QList<QString>& errors, QWidget* parent) :
    QDialog(parent)
{
    this->build(errorTitle, errors);
}

ErrorDisplayDialog::ErrorDisplayDialog(const QString& errorTitle, const QString& error, QWidget* parent) :
    QDialog(parent)
{
    QList<QString> singleErrorList = QList<QString>();
    singleErrorList.append(error);

    this->build(errorTitle, singleErrorList);
}

void ErrorDisplayDialog::build(const QString& errorTitle, const QList<QString>& errors)
{
    this->setWindowTitle(tr("Errors encountred"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel(errorTitle);
    layout->addWidget(titleLabel);

    QLabel* errorsLabel = new QLabel(tr("List of errors:"));
    layout->addWidget(errorsLabel);

    QListWidget* errorList = new QListWidget();
    errorList->setWordWrap(true);

    for (QString s : errors)
    {
        errorList->addItem(s);
    }

    layout->addWidget(errorList);

    QPushButton* buttonOk = new QPushButton(tr("OK"));
    connect(buttonOk, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(buttonOk);
}
