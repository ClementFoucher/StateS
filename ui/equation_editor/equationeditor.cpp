/*
 * Copyright © 2014 Clément Foucher
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

// StateS classes
#include "machine.h"
#include "equation.h"
#include "graphicequation.h"
#include "input.h"


EquationEditor::EquationEditor(Machine* machine, Signal* initialEquation, QWidget* parent) :
    QDialog(parent)
{
    this->machine = machine;

    mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* title = new QLabel("<b>" + tr("Equation editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Resources
    QLabel* resourcesTitle = new QLabel("<i>" + tr("Drag and drop equation components from here...") + "</i>");
    resourcesTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(resourcesTitle);

    resourcesLayout = new QHBoxLayout();
    mainLayout->addLayout(resourcesLayout);


    if (machine->getInputs().count() != 0)
    {
        inputListLayout = new QVBoxLayout();
        inputListLayout->setAlignment(Qt::AlignTop);
        resourcesLayout->addLayout(inputListLayout);

        QLabel* inputsTitle = new QLabel("<b>" + tr("Inputs") + "</b>");
        inputsTitle->setAlignment(Qt::AlignCenter);
        inputListLayout->addWidget(inputsTitle);

        foreach (Signal* input, machine->getInputs())
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

        foreach (Signal* variable, machine->getLocalVariables())
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

        foreach (Signal* constant, machine->getConstants())
        {
            constantListLayout->addWidget(new GraphicEquation(constant, true));
        }

    }

    operatorListLayout = new QGridLayout();
    operatorListLayout->setAlignment(Qt::AlignTop);
    resourcesLayout->addLayout(operatorListLayout);

    QLabel* systemConstantsTitle = new QLabel("<b>" + tr("Constants") + "</b>");
    systemConstantsTitle->setAlignment(Qt::AlignCenter);
    operatorListLayout->addWidget(systemConstantsTitle, 0, 0, 1, 2);

    QLabel* operatorsTitle = new QLabel("<b>" + tr("Logic functions") + "</b>");
    operatorsTitle->setAlignment(Qt::AlignCenter);
    operatorListLayout->addWidget(operatorsTitle, 2, 0, 1, 2);

    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::notOp, 1),  true),  3, 0, 1, 2);

    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::equalOp, 2), true), 4, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::diffOp,  2), true), 4, 1, 1, 1);

    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::andOp,  2), true), 5, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::orOp,   2), true), 6, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::xorOp,  2), true), 7, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::nandOp, 2), true), 8, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::norOp,  2), true), 9, 0, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::xnorOp, 2), true), 10, 0, 1, 1);

    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::andOp,  3), true), 5, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::orOp,   3), true), 6, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::xorOp,  3), true), 7, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::nandOp, 3), true), 8, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::norOp,  3), true), 9, 1, 1, 1);
    operatorListLayout->addWidget(new GraphicEquation(new Equation(Equation::nature::xnorOp, 3), true), 10, 1, 1, 1);

    // Equation
    QLabel* equationTitle = new QLabel("<i>" + tr("... to here.") + "</i>");
    equationTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(equationTitle);

    if (initialEquation != nullptr)
    {
        Equation* equation = dynamic_cast <Equation*> (initialEquation);
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

Signal* EquationEditor::getResultEquation() const
{
    // Must return a copy of the created equation,
    // as the equations are deleted on dialog deletion
    // (needed to authorize cancel)

    Equation* temp = dynamic_cast <Equation*> (equation->getLogicEquation());

    if (temp != nullptr)
        return temp->clone();
    else
        return equation->getLogicEquation();
}
