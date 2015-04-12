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
#include "signaleditortab.h"

// Qt classes
#include <QLabel>
#include <QToolBox>
#include <QVBoxLayout>
//#include <QGridLayout>
//#include <QTableWidget>
//#include <QPushButton>

// StateS classes
#include "signallisteditor.h"


SignalEditorTab::SignalEditorTab(shared_ptr<Machine> machine, QWidget* parent) :
    QWidget(parent)
{
    this->setLayout(new QVBoxLayout());

    //
    // Title

    QLabel* title = new QLabel("<b>" + tr("Signal editor")  + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    //
    // Help line

    QLabel* hint = new QLabel(tr("Switch between I/Os and local signals editors using tabs below")
                              + "<br />"
                              + tr("Signals length can not exceed 64 bits")
                              );
    hint->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(hint);

    //
    // Toolbox

    QToolBox* toolBox = new QToolBox();
    this->layout()->addWidget(toolBox);

    // Local

    QWidget* locals = new QWidget();
    locals->setLayout(new QVBoxLayout());

    QLabel* varTitle = new QLabel(tr("Variables"));
    varTitle->setAlignment(Qt::AlignCenter);
    locals->layout()->addWidget(varTitle);
    SignalListEditor* variablesToolBox = new SignalListEditor(machine, Machine::signal_type::LocalVariable);
    locals->layout()->addWidget(variablesToolBox);

    QLabel* constTitle = new QLabel(tr("Constants"));
    constTitle->setAlignment(Qt::AlignCenter);
    locals->layout()->addWidget(constTitle);
    SignalListEditor* constantsToolBox = new SignalListEditor(machine, Machine::signal_type::Constant);
    locals->layout()->addWidget(constantsToolBox);

    toolBox->addItem(locals, tr("Local signals"));

    // I/O
    QWidget* ios = new QWidget();
    ios->setLayout(new QVBoxLayout());

    QLabel* inputsTitle = new QLabel(tr("Inputs"));
    inputsTitle->setAlignment(Qt::AlignCenter);
    ios->layout()->addWidget(inputsTitle);
    SignalListEditor* inputToolBox = new SignalListEditor(machine, Machine::signal_type::Input);
    ios->layout()->addWidget(inputToolBox);

    QLabel* outputsTitle = new QLabel(tr("Outputs"));
    outputsTitle->setAlignment(Qt::AlignCenter);
    ios->layout()->addWidget(outputsTitle);
    SignalListEditor* outputToolBox = new SignalListEditor(machine, Machine::signal_type::Output);
    ios->layout()->addWidget(outputToolBox);

    toolBox->addItem(ios, tr("I/O"));

    toolBox->setCurrentIndex(1);
}

