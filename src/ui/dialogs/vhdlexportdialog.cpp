/*
 * Copyright © 2014-2026 Clément Foucher
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

// StateS classes
#include "fsmvhdlexport.h"
#include "savefiledialog.h"


VhdlExportDialog::VhdlExportDialog(const QString& baseFileName, const QString& searchPath, QWidget* parent) :
	StatesDialog(parent)
{
	this->baseFileName  = baseFileName;
	this->searchPath    = searchPath;
	this->fsmVhdlExport = make_shared<FsmVhdlExport>();

	this->setWindowTitle(tr("VHDL export"));

	auto compatChecker = fsmVhdlExport->checkCompatibility();

	//
	// Build widgets

	QLabel* warning = nullptr;
	if (compatChecker->isCompatible() == false)
	{
		warning = new QLabel("<span style=\"color:red;\">"
		                     + tr("Warning! Experimental feature.") + "<br />"
		                     + tr("Output generation has strong restrictions:") + "<br />"
		                     + tr("some of the current machine's outputs won't be correctly handled.") + "<br />"
		                     + tr("Please run machine verifier to identify affected variables.") + "</span>");
		warning->setAlignment(Qt::AlignCenter);
	}

	auto title = new QLabel("<b>" + tr("Choose export options:") + "</b>");
	title->setAlignment(Qt::AlignCenter);

	this->resetLogicSelectionBox = new QComboBox();
	this->resetLogicSelectionBox->addItem(tr("Positive"));
	this->resetLogicSelectionBox->addItem(tr("Negative"));

	this->addPrefixSelectionBox = new QComboBox();
	this->addPrefixSelectionBox->addItem(tr("No"));
	this->addPrefixSelectionBox->addItem(tr("Yes"));

	auto buttonOK     = new QPushButton(tr("OK"));
	auto buttonCancel = new QPushButton(tr("Cancel"));

	//
	// Build layouts

	auto formLayout = new QFormLayout();
	formLayout->addRow(tr("Reset logic:"), this->resetLogicSelectionBox);
	formLayout->addRow(tr("Prefix inputs and outputs with 'I_' and 'O_' respectively:"), this->addPrefixSelectionBox);

	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(buttonOK);
	buttonsLayout->addWidget(buttonCancel);

	auto mainLayout = new QVBoxLayout(this);
	if (warning != nullptr)
	{
		mainLayout->addWidget(warning);
	}
	mainLayout->addWidget(title);
	mainLayout->addLayout(formLayout);
	mainLayout->addLayout(buttonsLayout);

	//
	// Connect signals

	connect(buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
	connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}

bool VhdlExportDialog::isResetPositive() const
{
	if (this->resetLogicSelectionBox->currentIndex() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool VhdlExportDialog::prefixIOs() const
{
	if (this->addPrefixSelectionBox->currentIndex() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

QString VhdlExportDialog::getFilePath() const
{
	return this->filePath;
}

shared_ptr<FsmVhdlExport> VhdlExportDialog::getFsmVhdlExport() const
{
	return this->fsmVhdlExport;
}

void VhdlExportDialog::accept()
{
	this->filePath = SaveFileDialog::getSaveFileName(this, tr("Export machine to VHDL"), this->searchPath, this->baseFileName, "vhdl");

	if (this->filePath.isEmpty() == false)
	{
		QDialog::accept();
	}
}
