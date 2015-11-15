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
#include "vhdlexportdialog.h"

// Qt classes
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>


VhdlExportDialog::VhdlExportDialog(const QString& baseFileName, const QString& searchPath, QWidget *parent) :
    QDialog(parent)
{
    this->baseFileName = baseFileName;
    this->searchPath   = searchPath;

    QVBoxLayout* layout = new QVBoxLayout(this);


    QLabel* warning = new QLabel("<font color=red>" + tr("Warning! Experimental feature. Please consult known_bugs.txt.") + "</font>");
    warning->setAlignment(Qt::AlignCenter);
    layout->addWidget(warning);

    QLabel* title = new QLabel("<b>" + tr("Choose export options:") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout* formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    this->resetLogicSelectionBox = new QComboBox();
    this->resetLogicSelectionBox->addItem(tr("Positive"));
    this->resetLogicSelectionBox->addItem(tr("Negative"));
    formLayout->addRow(tr("Reset logic:"), this->resetLogicSelectionBox);

    this->addPrefixSelectionBox = new QComboBox();
    this->addPrefixSelectionBox->addItem(tr("No"));
    this->addPrefixSelectionBox->addItem(tr("Yes"));
    formLayout->addRow(tr("Prefix inputs and outputs with 'I_' and 'O_' respectively:"), this->addPrefixSelectionBox);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    QPushButton* buttonOK = new QPushButton(tr("OK"));
    connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
    buttonsLayout->addWidget(buttonOK);

    QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    buttonsLayout->addWidget(buttonCancel);
}

bool VhdlExportDialog::isResetPositive()
{
    if (this->resetLogicSelectionBox->currentIndex() == 0)
        return true;
    else
        return false;
}

bool VhdlExportDialog::prefixIOs()
{
    if (this->addPrefixSelectionBox->currentIndex() == 0)
        return false;
    else
        return true;
}

QString VhdlExportDialog::getFilePath()
{
    return this->filePath;
}

void VhdlExportDialog::accept()
{
    QString defaultFilePath;

    if (! this->searchPath.isEmpty())
    {
        defaultFilePath += this->searchPath;
        defaultFilePath += "/"; // TODO: check if environment dependant!
    }

    defaultFilePath += this->baseFileName;

    this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to VHDL"), defaultFilePath + ".vhdl", "*.vhdl");

    if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".vhdl", Qt::CaseInsensitive)) )
        this->filePath += ".vhdl";

    if (! this->filePath.isEmpty())
        QDialog::accept();
}
