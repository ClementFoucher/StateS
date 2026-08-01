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
#include "verifiertab.h"

// Qt
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>

// StateS
#include "machinemanager.h"
#include "truthtabledisplay.h"
#include "hintwidget.h"
#include "truthtable.h"
#include "fsmverifier.h"


VerifierTab::VerifierTab(QWidget* parent) :
	QWidget(parent)
{
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &VerifierTab::resetContent);

	//
	// Header

	auto title = new QLabel("<b>" + tr("Verifier tool") + "</b>", this);
	title->setAlignment(Qt::AlignCenter);

	//
	// Content

	this->checkVhdlExport = new QCheckBox(tr("Check for VHDL export restrictions"), this);

	this->buttonVerify = new QPushButton(tr("Check machine"), this);
	connect(this->buttonVerify, &QPushButton::clicked, this, &VerifierTab::beginCheck);

	//
	// Build complete rendering

	this->mainLayout = new QVBoxLayout(this);
	this->mainLayout->setAlignment(Qt::AlignTop);

	this->mainLayout->addWidget(title);
	this->mainLayout->addWidget(this->checkVhdlExport);
	this->mainLayout->addWidget(this->buttonVerify);
}

void VerifierTab::beginCheck()
{
	// Reset any content from a previous verification
	this->resetContent();

	// Hide initial widgets
	this->checkVhdlExport->setVisible(false);
	this->buttonVerify   ->setVisible(false);

	// Add verification in progress widgets
	this->verificationRunningLabel = new QLabel(tr("Verification in progress..."), this);
	this->verificationRunningLabel->setWordWrap(true);
	this->verificationRunningLabel->setAlignment(Qt::AlignCenter);
	this->mainLayout->addWidget(this->verificationRunningLabel);

	this->buttonCancel = new QPushButton(tr("Cancel verification"), this);
	connect(this->buttonCancel, &QPushButton::clicked, this, &VerifierTab::cancelCheck);
	this->mainLayout->addWidget(this->buttonCancel);

	// Build verifier
	this->verifier = make_unique<FsmVerifier>();
	if (this->checkVhdlExport->isChecked() == true)
	{
		this->verifier->setCheckVhdl();
	}
	connect(this->verifier.get(), &FsmVerifier::verificationOver, this, &VerifierTab::checkFinished);

	// Build timer
	this->timer = new QTimer(this);
	this->timer->setInterval(10000);
	connect(this->timer, &QTimer::timeout, this, &VerifierTab::timeout);

	// Lock interface
	machineManager->setInterfaceMode(InterfaceMode_t::verifyMode);

	// Launch verification
	this->verifier->start();
	this->timer->start();
}

void VerifierTab::cancelCheck()
{
	// Suspend timer
	this->timer->stop();

	// Cancel verification thread
	this->verifier->abort();
	this->verifier->wait();

	// Reset content
	this->resetContent();

	// Unlock interface
	machineManager->setInterfaceMode(InterfaceMode_t::editMode);
}

void VerifierTab::checkFinished()
{
	// Suspend and delete timer
	this->timer->stop();

	delete this->timer;
	this->timer = nullptr;

	// Delete verification in progress widgets
	delete this->verificationRunningLabel;
	delete this->buttonCancel;
	delete this->timingWarning;

	this->verificationRunningLabel = nullptr;
	this->buttonCancel             = nullptr;
	this->timingWarning            = nullptr;

	// Unlock interface
	machineManager->setInterfaceMode(InterfaceMode_t::editMode);

	// Restore initial widgets visibility
	this->checkVhdlExport->setVisible(true);
	this->buttonVerify   ->setVisible(true);

	// Add clear button
	this->buttonClear = new QPushButton(tr("Clear verification results"), this);
	this->mainLayout->addWidget(this->buttonClear);
	connect(this->buttonClear, &QPushButton::clicked, this, &VerifierTab::resetContent);

	// Display issues
	auto issues = this->verifier->getIssues();

	if (issues.count() == 0)
	{
		this->listTitle = new QLabel(tr("No errors!"), this);
		this->listTitle->setAlignment(Qt::AlignCenter);
		this->listTitle->setWordWrap(true);
		this->mainLayout->addWidget(this->listTitle);
	}
	else
	{
		this->listTitle = new QLabel(this);
		this->listTitle->setWordWrap(true);
		this->listTitle->setText(tr("The following issues were found:"));
		this->mainLayout->addWidget(this->listTitle);


		this->list = new QListWidget(this);
		connect(this->list, &QListWidget::itemDoubleClicked, this, &VerifierTab::proofRequested);
		this->list->setWordWrap(true);
		this->mainLayout->addWidget(this->list);

		bool hasProofs = false;
		bool hasRed    = false;
		bool hasBlue   = false;
		bool hasGreen  = false;
		for (int i = 0 ; i < issues.count() ; i++)
		{
			this->list->addItem(issues[i]->text);

			QBrush brush;

			switch (issues[i]->type)
			{
			case VerifierSeverityLevel_t::blocking:
				brush.setColor(Qt::red);
				hasRed = true;
				break;
			case VerifierSeverityLevel_t::structure:
				brush.setColor(Qt::blue);
				hasBlue = true;
				break;
			case VerifierSeverityLevel_t::tool:
				brush.setColor(Qt::darkGreen);
				hasGreen = true;
				break;
			case VerifierSeverityLevel_t::hint:
				break;
			}

			this->list->item(this->list->count()-1)->setForeground(brush);

			if(issues[i]->proof != nullptr)
			{
				brush.setColor(Qt::yellow);
				brush.setStyle(Qt::Dense4Pattern);
				this->list->item(this->list->count()-1)->setBackground(brush);
				hasProofs = true;
			}
		}

		QString hint;

		if (hasRed == true)
		{
			hint += tr("Issues in red are blocking for the machine to work.") + "<br />";
		}
		if (hasBlue == true)
		{
			hint += tr("Issues in blue won't block machine, but are structural errors that will lead to impredictible behavior at some point and must be corrected.") + "<br />";
		}
		if (hasGreen == true)
		{
			hint += tr("Issues in green are not machine errors but have restriction in StateS.") + "<br />";
		}
		if (hasProofs == true)
		{
			hint += tr("Yellow highlighted issues can be double-clicked for more details on the error.");
		}

		this->hintBox = new HintWidget(tr("Hint"), hint, this);
		this->mainLayout->addWidget(this->hintBox);
	}
}

void VerifierTab::resetContent()
{
	// Delete verification in progress
	// and verification results widgets
	delete this->verificationRunningLabel;
	delete this->buttonCancel;
	delete this->timingWarning;
	delete this->buttonClear;
	delete this->listTitle;
	delete this->list;
	delete this->truthTableDisplay;
	delete this->hintBox;

	this->verificationRunningLabel = nullptr;
	this->buttonCancel             = nullptr;
	this->timingWarning            = nullptr;
	this->buttonClear              = nullptr;
	this->listTitle                = nullptr;
	this->list                     = nullptr;
	this->truthTableDisplay        = nullptr;
	this->hintBox                  = nullptr;

	// Delete other members
	this->verifier.reset();

	delete this->timer;
	this->timer = nullptr;

	// Restore initial widgets visibility
	this->checkVhdlExport->setVisible(true);
	this->buttonVerify   ->setVisible(true);
}

void VerifierTab::timeout()
{
	QString timingWarningText;
	timingWarningText += tr("Warning: The verification has been running for an anormal duration.");
	timingWarningText += "<br>";
	timingWarningText += tr("This looks like the number of combinations to test is very high.");
	timingWarningText += "<br>";
	timingWarningText += tr("Consider canceling the verification.");

	this->timingWarning = new QLabel(timingWarningText, this);
	this->timingWarning->setWordWrap(true);
	this->timingWarning->setAlignment(Qt::AlignCenter);
	this->mainLayout->addWidget(this->timingWarning);
}

void VerifierTab::proofRequested(QListWidgetItem* item)
{
	if (this->verifier == nullptr) return;

	auto issues = this->verifier->getIssues();
	int row = this->list->row(item);
	auto issue = issues[row];
	if (issue->proof == nullptr) return;


	auto currentTruthTable = issue->proof;
	auto highlights        = issue->proofsHighlight;

	delete this->truthTableDisplay;
	this->truthTableDisplay = new TruthTableDisplay(currentTruthTable, highlights, this);
	this->mainLayout->insertWidget(this->mainLayout->count()-1, this->truthTableDisplay);

	QString text = tr("Lines highlighted in red in the truth table are conflicts resulting in multiple simultaneous transitions being activated.");

	this->hintBox->setContent(tr("Details on error"), text);
}
