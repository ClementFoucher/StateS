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

#include "equationeditor.h"

#include <QLabel>

#include "logicvariable.h"
#include "machine.h"
#include "io.h"
#include "logicequation.h"
#include "graphicequation.h"
#include "input.h"
#include "output.h"

EquationEditor::EquationEditor(Machine* machine, LogicVariable* initialEquation, QWidget* parent) :
    QDialog(parent)
{
    this->machine = machine;

    LogicEquation* temp = dynamic_cast <LogicEquation*> (initialEquation);

    if (temp != nullptr)
        this->equation = new GraphicEquation(temp->clone(), nullptr);
    else
        this->equation = new GraphicEquation(initialEquation, nullptr);

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

    if (machine->getInputs().count() != 0)
    {
        inputListLayout = new QVBoxLayout();
        inputListLayout->setAlignment(Qt::AlignTop);

        QLabel* inputsTitle = new QLabel("<b>" + tr("Inputs") + "</b>");
        inputsTitle->setAlignment(Qt::AlignCenter);
        inputListLayout->addWidget(inputsTitle);

        //inputs = new QListWidget();
        foreach (Input* var, machine->getInputs())
        {
            //new QListWidgetItem(var->getName(), inputs);
            //inputListLayout->addWidget(inputs);
            GraphicEquation* input = new GraphicEquation(var, nullptr, true);
            inputListLayout->addWidget(input);
        }
        resourcesLayout->addLayout(inputListLayout);
    }


    if (machine->getLocalVariables().count() != 0)
    {
        variableListLayout = new QVBoxLayout();
        variableListLayout->setAlignment(Qt::AlignTop);

        QLabel* variablesTitle = new QLabel("<b>" + tr("Variables")+ "</b>");
        variablesTitle->setAlignment(Qt::AlignCenter);
        variableListLayout->addWidget(variablesTitle);

        //variables = new QListWidget();
        foreach (LogicVariable* var, machine->getLocalVariables())
        {
            //  new QListWidgetItem(var->getName(), variables);
            //variableListLayout->addWidget(variables);
            GraphicEquation* variable = new GraphicEquation(var, nullptr, true);
            variableListLayout->addWidget(variable);
        }

        resourcesLayout->addLayout(variableListLayout);
    }

    operatorListLayout = new QVBoxLayout();
    operatorListLayout->setAlignment(Qt::AlignTop);

    QLabel* operatorsTitle = new QLabel("<b>" + tr("Logic functions") + "</b>");
    operatorsTitle->setAlignment(Qt::AlignCenter);

    operatorListLayout->addWidget(operatorsTitle);

    notOperator  = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::notOp,  nullptr), nullptr, true);
    andOperator  = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::andOp,  nullptr), nullptr, true);
    orOperator   = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::orOp,   nullptr), nullptr, true);
    xorOperator  = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::xorOp,  nullptr), nullptr, true);
    nandOperator = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::nandOp, nullptr), nullptr, true);
    norOperator  = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::norOp,  nullptr), nullptr, true);
    xnorOperator = new GraphicEquation(new LogicEquation(nullptr, LogicEquation::nature::xnorOp, nullptr), nullptr, true);

    operatorListLayout->addWidget(notOperator);
    operatorListLayout->addWidget(andOperator);
    operatorListLayout->addWidget(orOperator);
    operatorListLayout->addWidget(xorOperator);
    operatorListLayout->addWidget(nandOperator);
    operatorListLayout->addWidget(norOperator);
    operatorListLayout->addWidget(xnorOperator);

    resourcesLayout->addLayout(operatorListLayout);
    mainLayout->addLayout(resourcesLayout);

    // Equation

    QLabel* equationTitle = new QLabel("<i>" + tr("... to here.") + "</i>");
    equationTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(equationTitle);

    mainLayout->addWidget(equation);

    // Buttons

    buttonsLayout = new QHBoxLayout();

    buttonOK = new QPushButton(tr("OK"));
    buttonsLayout->addWidget(buttonOK);
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(accept()));

    buttonCancel = new QPushButton(tr("Cancel"));
    buttonsLayout->addWidget(buttonCancel);
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

    mainLayout->addLayout(buttonsLayout);


}

LogicVariable* EquationEditor::getResultEquation() const
{
    // Must return a copy of the created equation,
    // as the equations are deleted on dialog deletion
    // (needed to authorize cancel)

    LogicEquation* temp = dynamic_cast <LogicEquation*> (equation->getLogicEquation());

    if (temp != nullptr)
        return temp->clone();
    else
        return equation->getLogicEquation();
}
