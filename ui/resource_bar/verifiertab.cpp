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
#include "verifiertab.h"

// Qt classes
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>

// StateS classes
#include "fsmverifier.h"
#include "truthtabledisplay.h"


VerifierTab::VerifierTab(shared_ptr<Fsm> machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    new QVBoxLayout(this);
    this->layout()->setAlignment(Qt::AlignTop);

    QLabel* title = new QLabel("<b>" + tr("Verifier tool") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    QPushButton* buttonVerify = new QPushButton(tr("Check machine"));
    connect(buttonVerify, &QPushButton::clicked, this, &VerifierTab::checkNow);
    this->layout()->addWidget(buttonVerify);
}

VerifierTab::~VerifierTab()
{
    delete this->truthTable;
}

void VerifierTab::changeMachine(shared_ptr<Fsm> machine)
{
    this->clearDisplay();

    this->verifier.reset();

    this->machine = machine;
}

void VerifierTab::checkNow()
{
    if (this->verifier == nullptr)
        this->verifier = unique_ptr<FsmVerifier>(new FsmVerifier(machine.lock()));

    QList<QString> errors = this->verifier->verifyFsm();

    this->clearDisplay();

    if (errors.count() == 0)
    {
        this->listTitle = new QLabel(tr("No errors!"));
        this->listTitle->setAlignment(Qt::AlignCenter);
        this->listTitle->setWordWrap(true);
        this->layout()->addWidget(this->listTitle);
    }
    else
    {
        this->listTitle = new QLabel();
        this->listTitle->setWordWrap(true);
        this->listTitle->setText(tr("Double-click on red items to detail error:"));
        this->layout()->addWidget(this->listTitle);


        this->list = new QListWidget();
        connect(this->list, &QListWidget::itemDoubleClicked, this, &VerifierTab::proofRequested);
        this->layout()->addWidget(this->list);

        QVector<TruthTable*> proofs = this->verifier->getProofs();

        for (int i = 0 ; i < errors.count() ; i++)
        {
            this->list->addItem(errors[i]);

            if (proofs[i] != nullptr)
                this->list->item(this->list->count()-1)->setTextColor(Qt::red);
        }
    }

    this->buttonClear = new QPushButton(tr("Clear verification"));
    this->layout()->addWidget(this->buttonClear);
    connect(this->buttonClear, &QPushButton::clicked, this, &VerifierTab::clearDisplay);
}

void VerifierTab::clearDisplay()
{
    delete this->listTitle;
    this->listTitle = nullptr;

    delete this->list;
    this->list = nullptr;

    delete this->truthTable;
    this->truthTable = nullptr;

    delete this->buttonClear;
    this->buttonClear = nullptr;
}

void VerifierTab::proofRequested(QListWidgetItem* item)
{
    QVector<TruthTable*> proofs = this->verifier->getProofs();

    if (proofs[this->list->row(item)] != nullptr)
    {
        delete this->truthTable;

        TruthTable* currentTruthTable = proofs[this->list->row(item)];

        this->truthTable = new TruthTableDisplay(currentTruthTable);
        this->layout()->addWidget(this->truthTable);

        QList<int> highlights = this->verifier->getProofsHighlight()[currentTruthTable];

        foreach(int i, highlights)
        {
            for (int j = 0 ; j < this->truthTable->columnCount() ; j++)
            {
                this->truthTable->item(i, j)->setTextColor(Qt::red);
            }
        }
    }
}


