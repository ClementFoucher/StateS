/*
 * Copyright © 2014-2025 Clément Foucher
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

// Qt classes
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>

// StateS classes
#include "machinemanager.h"
#include "truthtabledisplay.h"
#include "collapsiblewidgetwithtitle.h"
#include "truthtable.h"
#include "fsmverifier.h"


VerifierTab::VerifierTab(QWidget* parent) :
    QWidget(parent)
{
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &VerifierTab::clearDisplay);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);

	QLabel* title = new QLabel("<b>" + tr("Verifier tool") + "</b>", this);
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	QCheckBox* checkVhdlExport = new QCheckBox(tr("Check for VHDL export restrictions"), this);
	connect(checkVhdlExport, &QCheckBox::clicked, this, &VerifierTab::setCheckVhdl);
	layout->addWidget(checkVhdlExport);

	QPushButton* buttonVerify = new QPushButton(tr("Check machine"), this);
	connect(buttonVerify, &QPushButton::clicked, this, &VerifierTab::checkNow);
	layout->addWidget(buttonVerify);
}

void VerifierTab::checkNow()
{
	this->clearDisplay();

	this->verifier = unique_ptr<FsmVerifier>(new FsmVerifier());
	const QList<shared_ptr<FsmVerifier::Issue>>& issues = this->verifier->verifyFsm(this->checkVhdl);

	if (issues.count() == 0)
	{
		this->listTitle = new QLabel(tr("No errors!"), this);
		this->listTitle->setAlignment(Qt::AlignCenter);
		this->listTitle->setWordWrap(true);
		this->layout()->addWidget(this->listTitle);
	}
	else
	{
		this->listTitle = new QLabel(this);
		this->listTitle->setWordWrap(true);
		this->listTitle->setText(tr("The following issues were found:"));
		this->layout()->addWidget(this->listTitle);


		this->list = new QListWidget(this);
		connect(this->list, &QListWidget::itemDoubleClicked, this, &VerifierTab::proofRequested);
		this->list->setWordWrap(true);
		this->layout()->addWidget(this->list);

		bool hasProofs = false;
		bool hasRed = false;
		bool hasBlue = false;
		bool hasGreen = false;
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

		if (hasRed)
			hint += tr("Issues in red are blocking for the machine to work.") + "<br />";
		if (hasBlue)
			hint += tr("Issues in blue won't block machine, but are structural errors that will lead to impredictible behavior at some point and must be corrected.") + "<br />";
		if (hasGreen)
			hint += tr("Issues in green are not machine errors but have restriction in StateS.") + "<br />";
		if (hasProofs)
			hint += tr("Yellow highlighted issues can be double-clicked for more details on the error.");

		this->hintBox = new CollapsibleWidgetWithTitle(this);
		this->layout()->addWidget(this->hintBox);

		this->hintBox->setContent(tr("Hint"), hint, true);
	}

	this->buttonClear = new QPushButton(tr("Clear verification"), this);
	this->layout()->addWidget(this->buttonClear);
	connect(this->buttonClear, &QPushButton::clicked, this, &VerifierTab::clearDisplay);
}

void VerifierTab::clearDisplay()
{
	delete this->listTitle;
	delete this->list;
	delete this->truthTableDisplay;
	delete this->buttonClear;
	delete this->hintBox;

	this->listTitle         = nullptr;
	this->list              = nullptr;
	this->truthTableDisplay = nullptr;
	this->buttonClear       = nullptr;
	this->hintBox           = nullptr;

	this->verifier.reset();
}

void VerifierTab::setCheckVhdl(bool doCheck)
{
	this->checkVhdl = doCheck;
}

void VerifierTab::proofRequested(QListWidgetItem* item)
{
	if (this->verifier == nullptr) return;

	const QList<shared_ptr<FsmVerifier::Issue>>&  issues = this->verifier->getIssues();
	if (issues[this->list->row(item)]->proof != nullptr)
	{
		delete this->truthTableDisplay;

		shared_ptr<TruthTable> currentTruthTable = issues[this->list->row(item)]->proof;
		QList<int> highlights                    = issues[this->list->row(item)]->proofsHighlight;

		this->truthTableDisplay = new TruthTableDisplay(currentTruthTable, highlights);
		((QVBoxLayout*)this->layout())->insertWidget(5, this->truthTableDisplay);

		QString text = tr("Lines highlighted in red in the truth table are conflicts resulting in multiple simultaneous transitions being activated.");

		this->hintBox->setContent(tr("Details on error"), text, true);
	}
}
