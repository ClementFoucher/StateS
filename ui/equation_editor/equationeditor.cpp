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
#include "equationeditor.h"

// Qt classes
#include <QLabel>
#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>

// StateS classes
#include "machine.h"
#include "equation.h"
#include "graphicequation.h"
#include "input.h"
#include "states.h"


EquationEditor::EquationEditor(shared_ptr<Machine> machine, shared_ptr<Signal> initialEquation, QWidget* parent) :
    QDialog(parent)
{
    this->machine = machine;

    this->setWindowIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/StateS"))));
    this->setWindowTitle(tr("StateS equation editor"));

    mainLayout = new QVBoxLayout(this);

    //
    // Title
    QLabel* title = new QLabel("<b>" + tr("Equation editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    //
    // Resources
    QLabel* resourcesTitle = new QLabel("<i>" + tr("Drag and drop equation components from here...") + "</i>");
    resourcesTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(resourcesTitle);

    QWidget* resourcesWidget = new QWidget();
    resourcesLayout = new QHBoxLayout(resourcesWidget);

    // Display signals
    if (machine->getInputs().count() != 0)
    {
        inputListLayout = new QVBoxLayout();
        inputListLayout->setAlignment(Qt::AlignTop);
        resourcesLayout->addLayout(inputListLayout);

        QLabel* inputsTitle = new QLabel("<b>" + tr("Inputs") + "</b>");
        inputsTitle->setAlignment(Qt::AlignCenter);
        inputListLayout->addWidget(inputsTitle);

        foreach (shared_ptr<Signal> input, machine->getInputs())
        {
            inputListLayout->addWidget(new GraphicEquation(input, true));
        }
    }


    if (machine->getLocalVariables().count() != 0)
    {
        variableListLayout = new QVBoxLayout();
        variableListLayout->setAlignment(Qt::AlignTop);
        resourcesLayout->addLayout(variableListLayout);

        QLabel* variablesTitle = new QLabel("<b>" + tr("Variables")+ "</b>");
        variablesTitle->setAlignment(Qt::AlignCenter);
        variableListLayout->addWidget(variablesTitle);

        foreach (shared_ptr<Signal> variable, machine->getLocalVariables())
        {
            variableListLayout->addWidget(new GraphicEquation(variable, true));
        }

    }

    if (machine->getConstants().count() != 0)
    {
        constantListLayout = new QVBoxLayout();
        constantListLayout->setAlignment(Qt::AlignTop);
        resourcesLayout->addLayout(constantListLayout);

        QLabel* constantsTitle = new QLabel("<b>" + tr("Constants")+ "</b>");
        constantsTitle->setAlignment(Qt::AlignCenter);
        constantListLayout->addWidget(constantsTitle);

        foreach (shared_ptr<Signal> constant, machine->getConstants())
        {
            constantListLayout->addWidget(new GraphicEquation(constant, true));
        }

    }

    // Display operators
    operatorListLayout = new QGridLayout();
    operatorListLayout->setAlignment(Qt::AlignTop);
    resourcesLayout->addLayout(operatorListLayout);

    QLabel* operatorsTitle = new QLabel("<b>" + tr("Logic functions") + "</b>");
    operatorsTitle->setAlignment(Qt::AlignCenter);
    operatorListLayout->addWidget(operatorsTitle, 0, 0, 1, 2);

    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::notOp,   1)),  true),  1, 0, 1, 2);

    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::equalOp, 2)), true), 2, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::diffOp,  2)), true), 2, 1, 1, 1);

    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::andOp,   2)), true), 3, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::orOp,    2)), true), 4, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::xorOp,   2)), true), 5, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::nandOp,  2)), true), 6, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::norOp,   2)), true), 7, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::xnorOp,  2)), true), 8, 0, 1, 1);

    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::andOp,   3)), true), 3, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::orOp,    3)), true), 4, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::xorOp,   3)), true), 5, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::nandOp,  3)), true), 6, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::norOp,   3)), true), 7, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(Equation::nature::xnorOp,  3)), true), 8, 1, 1, 1);

    // Add resources in a scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setAlignment(Qt::AlignHCenter);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(resourcesWidget);
    mainLayout->addWidget(scrollArea);


    //
    // Equation
    QLabel* equationTitle = new QLabel("<i>" + tr("... to here.") + "</i>");
    equationTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(equationTitle);

    if (initialEquation != nullptr)
    {
        shared_ptr<Equation> equation = dynamic_pointer_cast<Equation>(initialEquation);
        if (equation != nullptr)
            this->equation = new GraphicEquation(equation->clone(), false);
        else
            this->equation = new GraphicEquation(initialEquation, false);
    }
    else
    {
        this->equation = new GraphicEquation(nullptr, false);
    }

    mainLayout->addWidget(equation, 0, Qt::AlignHCenter);

    QLabel* equationInfo= new QLabel(tr("You can also use right-click on equation members to edit"));
    equationInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(equationInfo);

    //
    // Buttons
    buttonsLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonsLayout);

    buttonOK = new QPushButton(tr("OK"));
    buttonsLayout->addWidget(buttonOK);
    connect(buttonOK, &QAbstractButton::clicked, this, &EquationEditor::accept);

    buttonCancel = new QPushButton(tr("Cancel"));
    buttonsLayout->addWidget(buttonCancel);
    connect(buttonCancel, &QAbstractButton::clicked, this, &EquationEditor::reject);
}

shared_ptr<Signal> EquationEditor::getResultEquation() const
{
    return equation->getLogicEquation();
}
