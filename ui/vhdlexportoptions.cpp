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
#include "vhdlexportoptions.h"

// Qt classes
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>


VhdlExportOptions::VhdlExportOptions(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* title = new QLabel("<b>" + tr("Choose export options:") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout* formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    resetLogicSelectionBox = new QComboBox();
    resetLogicSelectionBox->addItem(tr("Positive"));
    resetLogicSelectionBox->addItem(tr("Negative"));
    formLayout->addRow(tr("Reset logic:"), resetLogicSelectionBox);

    addPrefixSelectionBox = new QComboBox();
    addPrefixSelectionBox->addItem(tr("No"));
    addPrefixSelectionBox->addItem(tr("Yes"));
    formLayout->addRow(tr("Prefix inputs and outputs with 'I_' and 'O_' respectively:"), addPrefixSelectionBox);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    QPushButton* buttonOK = new QPushButton(tr("OK"));
    connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
    buttonsLayout->addWidget(buttonOK);

    QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    buttonsLayout->addWidget(buttonCancel);
}

bool VhdlExportOptions::isResetPositive()
{
    if (resetLogicSelectionBox->currentIndex() == 0)
        return true;
    else
        return false;
}

bool VhdlExportOptions::prefixIOs()
{
    if (addPrefixSelectionBox->currentIndex() == 0)
        return false;
    else
        return true;
}


