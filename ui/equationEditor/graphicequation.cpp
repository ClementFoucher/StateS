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

#include "graphicequation.h"

#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QWidgetAction>

#include <QDebug>

#include "equationmimedata.h"
#include "logicequation.h"
#include "contextmenu.h"

// A graphic equation can either represent
// a logic equation or a logic variable.

GraphicEquation::GraphicEquation(LogicVariable* equation, GraphicEquation* parent, bool isTemplate)
{
    this->equation   = equation;
    this->isTemplate = isTemplate;
    this->parent     = parent;

    this->layout = new QHBoxLayout(this);

    buildEquation();
}

void GraphicEquation::updateEquation()
{
    // If equation is variable: nothing to do.
    // Else, set left/right operands based on current displays
    LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);

    if (complexEquation != nullptr)
    {
        LogicVariable* previousOperand;

        previousOperand = complexEquation->getLeftOperand();
        if (leftOperandDisplay != nullptr)
        {
            if (previousOperand != leftOperandDisplay->getLogicEquation())
            {
                complexEquation->setLeftOperand(leftOperandDisplay->getLogicEquation());

                LogicEquation* previousEquation = dynamic_cast <LogicEquation*> (previousOperand);

                if (previousEquation != nullptr)
                    delete previousOperand;
            }
        }
        else
            complexEquation->setLeftOperand(nullptr);

        previousOperand = complexEquation->getRightOperand();
        if (rightOperandDisplay != nullptr)
        {
            if (previousOperand != rightOperandDisplay->getLogicEquation())
            {
                complexEquation->setRightOperand(rightOperandDisplay->getLogicEquation());

                LogicEquation* previousEquation = dynamic_cast <LogicEquation*> (previousOperand);

                if (previousEquation != nullptr)
                    delete previousOperand;
            }
        }
        else
            complexEquation->setRightOperand(nullptr);
    }

    if (parent != nullptr)
        parent->updateEquation();
}

void GraphicEquation::buildEquation()
{
    delete leftOperandDisplay;
    delete operatorText;
    delete rightOperandDisplay;
    delete variableText;
    delete invertorText;

    leftOperandDisplay = nullptr;
    operatorText = nullptr;
    rightOperandDisplay = nullptr;
    variableText = nullptr;
    invertorText = nullptr;

    if (!isTemplate)
        this->setAcceptDrops(true);

    LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);

    if (complexEquation != nullptr)
    {
        if ( (!this->isTemplate) && (complexEquation->isInverted()) )
        {
            this->invertorText = new QLabel("/");
            layout->addWidget(this->invertorText);
        }

        if (complexEquation->getFunction() != LogicEquation::nature::notOp)
        {
            leftOperandDisplay = new GraphicEquation(complexEquation->getLeftOperand(), this, isTemplate);
            layout->addWidget(leftOperandDisplay);
        }

        if (this->isTemplate)
        {
            switch(complexEquation->getFunction())
            {
            case LogicEquation::nature::notOp:
                this->operatorText = new QLabel("not");
                break;
            case LogicEquation::nature::andOp:
                this->operatorText = new QLabel("and");
                break;
            case LogicEquation::nature::orOp:
                this->operatorText = new QLabel("or");
                break;
            case LogicEquation::nature::xorOp:
                this->operatorText = new QLabel("xor");
                break;
            case LogicEquation::nature::nandOp:
                this->operatorText = new QLabel("nand");
                break;
            case LogicEquation::nature::norOp:
                this->operatorText = new QLabel("nor");
                break;
            case LogicEquation::nature::xnorOp:
                this->operatorText = new QLabel("xnor");
                break;
            }

            layout->addWidget(operatorText);
            this->operatorText->setAlignment(Qt::AlignCenter);
        }
        else
        {
            switch(complexEquation->getFunction())
            {
            case LogicEquation::nature::andOp:
            case LogicEquation::nature::nandOp:
                this->operatorText = new QLabel("•");
                this->operatorText->setAlignment(Qt::AlignCenter);
                layout->addWidget(operatorText);
                break;
            case LogicEquation::nature::orOp:
            case LogicEquation::nature::norOp:
                this->operatorText = new QLabel("+");
                this->operatorText->setAlignment(Qt::AlignCenter);
                layout->addWidget(operatorText);
                break;
            case LogicEquation::nature::xorOp:
            case LogicEquation::nature::xnorOp:
                this->operatorText = new QLabel("⊕");
                this->operatorText->setAlignment(Qt::AlignCenter);
                layout->addWidget(operatorText);
                break;
            default:
                break;
            }
        }

        rightOperandDisplay = new GraphicEquation(complexEquation->getRightOperand(), this, isTemplate);
        layout->addWidget(rightOperandDisplay);
    }
    else // Equation is actually variable
    {
        if ( (equation == LogicVariable::constant1) || (equation == LogicVariable::constant0) )
        {
            variableText = new QLabel("...");
        }
        else
            variableText = new QLabel(equation->getName());

        variableText->setAlignment(Qt::AlignCenter);

        layout->addWidget(variableText);
    }

    if (this->isTemplate)
        this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
    else
    {
        if ( (this->equation == LogicVariable::constant0) || (this->equation == LogicVariable::constant1) )
        {
            this->setStyleSheet("GraphicEquation {border: 1px solid red; border-radius: 10px}");
        }
        else
            this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
    }


}

LogicVariable* GraphicEquation::getLogicEquation() const
{
    return equation;
}

void GraphicEquation::enterEvent(QEvent*)
{
    if (this->isTemplate)
    {
        if ((this->equation != LogicEquation::constant0) && (this->equation != LogicEquation::constant1))
            this->setStyleSheet("GraphicEquation {border: 1px solid #FF9900; border-radius: 10px}");
    }
}


void GraphicEquation::leaveEvent(QEvent*)
{
    if (this->isTemplate)
        this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
}

void GraphicEquation::mousePressEvent(QMouseEvent* event)
{
    if ( (this->equation != nullptr) && (isTemplate) && (this->equation != LogicEquation::constant0) && (this->equation != LogicEquation::constant1))
    {
        if (event->button() == Qt::LeftButton)
        {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new EquationMimeData(this);

            drag->setMimeData(mimeData);
            drag->setPixmap(this->grab());

            drag->exec();
        }
    }
    else if (this->parent != nullptr)
        parent->mousePressEvent(event);
}

void GraphicEquation::dragEnterEvent(QDragEnterEvent* event)
{
    const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData *>(event->mimeData());
    if (mimeData != nullptr)
    {
        event->acceptProposedAction();
    }

}

void GraphicEquation::contextMenuEvent(QContextMenuEvent* event)
{
    if ( (!isTemplate) && (equation != LogicVariable::constant0) && (equation != LogicVariable::constant1) )
    {
        ContextMenu* menu = new ContextMenu();
        menu->addTitle(tr("Equation: ") +  "<i>" + equation->getText() + "</i>");

        menu->addAction(tr("Delete"));

        menu->addAction(tr("Cancel"));

        menu->popup(this->mapToGlobal(event->pos()));

        connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(treatMenu(QAction*)));
    }
}

void GraphicEquation::dropEvent(QDropEvent* event)
{
    const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData*>(event->mimeData());

    if (mimeData != nullptr)
    {
        // Obtain new equation
        droppedEquation = mimeData->getEquation()->getLogicEquation();

        // Automatically replace constants:
        if ( (this->equation == LogicVariable::constant0) || (this->equation == LogicVariable::constant1) )
        {
            replaceEquation(droppedEquation);
        }
        else
        {
            // Ask what to do
            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("What should I do?"));
            menu->addSubTitle(tr("Existing equation: ") +  "<i>" + equation->getText() + "</i>");
            menu->addSubTitle(tr("Dropped equation: ") + "<i>" + droppedEquation->getText() + "</i>");

            QWidgetAction* a = new QWidgetAction(menu);
            a->setText(tr("Replace existing equation by dropped equation"));
            a->setToolTip(tr("New equation would be: ") + "<i>" + droppedEquation->getText() + "</i>");
            menu->addAction(a);

            LogicEquation* droppedComplexEquation = dynamic_cast <LogicEquation*> (droppedEquation);
            if (droppedComplexEquation != nullptr)
            {
                LogicEquation* currentEquation = dynamic_cast<LogicEquation*> (this->equation);

                if (droppedComplexEquation->getFunction() != LogicEquation::nature::notOp)
                {
                    a = new QWidgetAction(menu);
                    a->setText(tr("Set existing equation as left operand of dropped equation"));

                    LogicEquation* newEquation = droppedComplexEquation->clone();

                    if (currentEquation != nullptr)
                        newEquation->setLeftOperand(currentEquation->clone());
                    else
                        newEquation->setLeftOperand(this->equation);

                    a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");

                    delete newEquation;

                    menu->addAction(a);
                }


                a = new QWidgetAction(menu);
                a->setText(tr("Set existing equation as right operand of dropped equation"));

                LogicEquation* newEquation = droppedComplexEquation->clone();

                if (currentEquation != nullptr)
                    newEquation->setRightOperand(currentEquation->clone());
                else
                    newEquation->setRightOperand(this->equation);

                a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");

                delete newEquation;

                menu->addAction(a);
            }
            menu->addAction(tr("Cancel"));

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(treatMenu(QAction*)));
        }

        event->acceptProposedAction();
    }
}

void GraphicEquation::replaceEquation(LogicVariable* newVariable)
{
    LogicEquation* newEquation = dynamic_cast <LogicEquation*> (newVariable);

    // Delete current equation if it is not a single variable
    LogicEquation* currentEquation = dynamic_cast <LogicEquation*> (equation);
    if (currentEquation != nullptr)
        delete currentEquation;

    this->equation = nullptr;

    // Test if equation is actually equation or just single variable


    if (newEquation != nullptr)
        this->equation = newEquation->clone();
    else
        this->equation = newVariable;

    buildEquation();

    if (parent != nullptr)
        parent->updateEquation();
}

void GraphicEquation::treatMenu(QAction* action)
{

    if (action->text() == (tr("Replace existing equation by dropped equation")))
    {
        replaceEquation(droppedEquation);
    }
    else if (action->text() == tr("Set existing equation as left operand of dropped equation"))
    {
        LogicVariable* oldEquation = this->equation;

        LogicEquation* newEquation = dynamic_cast <LogicEquation*> (droppedEquation)->clone();
        newEquation->setLeftOperand(oldEquation);

        this->equation = newEquation;

        buildEquation();
    }
    else if (action->text() == tr("Set existing equation as right operand of dropped equation"))
    {
        LogicVariable* oldEquation = this->equation;

        LogicEquation* newEquation = dynamic_cast <LogicEquation*> (droppedEquation)->clone();
        newEquation->setRightOperand(oldEquation);

        this->equation = newEquation;

        buildEquation();
    }
    else if (action->text() == tr("Delete"))
    {
        replaceEquation(LogicVariable::constant0);
    }

    droppedEquation = nullptr;

}
