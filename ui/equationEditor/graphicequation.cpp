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
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDebug>

#include "equationmimedata.h"
#include "logicequation.h"
#include "contextmenu.h"
#include "inverterbar.h"

// A graphic equation can either represent
// a logic equation or a logic variable.

GraphicEquation::GraphicEquation(LogicVariable* equation, bool isTemplate, QWidget* parent) :
    QFrame(parent)
{
    this->equation   = equation;
    this->isTemplate = isTemplate;

    if (!isTemplate)
        this->setAcceptDrops(true);

    buildEquation();
}

void GraphicEquation::buildEquation()
{
    QObjectList toDelete = this->children();
    qDeleteAll(toDelete);




    // How to obtain only widget children?
 /*   QObjectList components = this->children();
    foreach(QObject* obj, components)
    {
        if (dynamic_cast<QWidget*>(obj) != nullptr)
            delete obj;
    }
*/


    QHBoxLayout* equationLayout = new QHBoxLayout();
    LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);

    if (complexEquation != nullptr)
    {
        if (this->isTemplate)
        {
            QString text;

            switch(complexEquation->getFunction())
            {
            case LogicEquation::nature::notOp:
                text += "not";
                break;
            case LogicEquation::nature::andOp:
                text += "and";
                break;
            case LogicEquation::nature::orOp:
                text += "or";
                break;
            case LogicEquation::nature::xorOp:
                text += "xor";
                break;
            case LogicEquation::nature::nandOp:
                text += "nand";
                break;
            case LogicEquation::nature::norOp:
                text += "nor";
                break;
            case LogicEquation::nature::xnorOp:
                text += "xnor";
                break;
            }

            if (complexEquation->getSize() > 1)
                text += " " + QString::number(complexEquation->getSize());

            QLabel* variableText = new QLabel(text);
            variableText->setAlignment(Qt::AlignCenter);
            equationLayout->addWidget(variableText);

            this->setLayout(equationLayout);
        }
        else
        {
    /*        if (complexEquation->isInverted())
            {
                QLabel* operatorText = nullptr;
                operatorText = new QLabel("/");
                operatorText->setAlignment(Qt::AlignCenter);
                equationLayout->addWidget(operatorText);
            }
*/
            for (int i = 0 ; i < complexEquation->getSize() ; i++)
            {
                // Add operand
                graphicOperands[i] = new GraphicEquation(complexEquation->getOperand(i), isTemplate, this);
                equationLayout->addWidget(graphicOperands[i]);

                // Add operator, except for last operand
                if (i < complexEquation->getSize() -1)
                {
                    QLabel* operatorText = nullptr;

                    switch(complexEquation->getFunction())
                    {
                    case LogicEquation::nature::andOp:
                    case LogicEquation::nature::nandOp:
                        operatorText = new QLabel("•");
                        break;
                    case LogicEquation::nature::orOp:
                    case LogicEquation::nature::norOp:
                        operatorText = new QLabel("+");
                        break;
                    case LogicEquation::nature::xorOp:
                    case LogicEquation::nature::xnorOp:
                        operatorText = new QLabel("⊕");
                        break;
                    default:
                        break;
                    }

                    if (operatorText != nullptr)
                    {
                        operatorText->setAlignment(Qt::AlignCenter);
                        equationLayout->addWidget(operatorText);

                    }
                }

                if (!complexEquation->isInverted())
                    this->setLayout(equationLayout);
                else
                {
                    QVBoxLayout* verticalLayout = new QVBoxLayout();

                    InverterBar* inverterBar = new InverterBar();

                    verticalLayout->addWidget(inverterBar);
                    verticalLayout->addLayout(equationLayout);

                    this->setLayout(verticalLayout);
                }
            }
        }
    }
    else // Equation is actually variable
    {
        QLabel* variableText = nullptr;

        if ( (equation == LogicVariable::constant1) || (equation == LogicVariable::constant0) )
        {
            variableText = new QLabel("...");
        }
        else
        {
            variableText = new QLabel(equation->getName());
        }

        variableText->setAlignment(Qt::AlignCenter);
        equationLayout->addWidget(variableText);
        this->setLayout(equationLayout);
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

    GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
    if (parentEquation != nullptr)
        parentEquation->updateEquation();
}

// This function is used to update the logic equation
// based on graphical changes made by user
void GraphicEquation::updateEquation()
{
    // If equation is variable: nothing to do.
    // Else, set operands based on current displays
    LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);

    if (complexEquation != nullptr)
    {
        LogicVariable* previousOperand;

        for (int i = 0 ; i < complexEquation->getSize() ; i++)
        {
            previousOperand = complexEquation->getOperand(i);

            if (graphicOperands[i]->getLogicEquation() != previousOperand)
            {
                complexEquation->setOperand(i, graphicOperands[i]->getLogicEquation());

                LogicEquation* previousEquation = dynamic_cast <LogicEquation*> (previousOperand);

                if (previousEquation != nullptr)
                    delete previousOperand;
            }
        }
    }

    GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
    if (parentEquation != nullptr)
        parentEquation->updateEquation();
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
    else
    {
        QFrame::mousePressEvent(event);
    }
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

        LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);
        if (complexEquation != nullptr)
        {
            if (complexEquation->getFunction() != LogicEquation::nature::notOp)
            {
                menu->addAction(tr("Add one operand to that operator"));

                if (complexEquation->getSize() > 2)
                {
                    menu->addAction(tr("Remove one operand from that operator"));
                }
            }
        }

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

                for (int i = 0 ; i < droppedComplexEquation->getOperands().count() ; i++)
                {
                    QString actionText = tr("Set existing equation as ");
                    actionText += QString::number(i+1);

                    if (i == 0)
                        actionText += tr("st");
                    else if (i == 1)
                        actionText += tr("nd");
                    else if (i == 1)
                        actionText += tr("rd");
                    else
                        actionText += tr("th");

                    actionText += tr(" operand of dropped equation");

                    a = new QWidgetAction(menu);
                    a->setText(actionText);

                    // Build tooltip
                    LogicEquation* newEquation = droppedComplexEquation->clone();
                    if (currentEquation != nullptr)
                        newEquation->setOperand(i, currentEquation->clone());
                    else
                        newEquation->setOperand(i, this->equation);
                    a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");
                    delete newEquation;

                    menu->addAction(a);
                }
            }
            menu->addAction(tr("Cancel"));

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(treatMenu(QAction*)));
        }

        event->acceptProposedAction();
    }
}

void GraphicEquation::treatMenu(QAction* action)
{

    if (action->text() == (tr("Replace existing equation by dropped equation")))
    {
        replaceEquation(droppedEquation);
    }
    else if (action->text() == tr("Delete"))
    {
        replaceEquation(LogicVariable::constant0);
    }
    else if (action->text() == tr("Add one operand to that operator"))
    {
        LogicEquation* complexEquation = dynamic_cast<LogicEquation*>(this->equation);

        complexEquation->increaseOperandCount();

        this->buildEquation();

        GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
        if (parentEquation != nullptr)
            parentEquation->updateEquation();
    }
    else if (action->text() == tr("Remove one operand from that operator"))
    {
        bool valid = false;

        LogicEquation* complexEquation = dynamic_cast<LogicEquation*>(this->equation);
        LogicVariable* operandToRemove = complexEquation->getOperand(complexEquation->getSize() - 1);
        if ( (operandToRemove == LogicVariable::constant0) || (operandToRemove == LogicVariable::constant1) )
            valid = true;
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete last oprand?") + "<br />" + tr("Content is: ") + operandToRemove->getText(), QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::StandardButton::Ok)
                valid = true;
        }

        if (valid)
        {
            complexEquation->decreaseOperandCount();

            this->buildEquation();

            GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
            if (parentEquation != nullptr)
                parentEquation->updateEquation();
        }
    }
    else
    {
        LogicEquation* droppedComplexEquation = dynamic_cast <LogicEquation*> (droppedEquation);
        for (int i = 0 ; i < droppedComplexEquation->getOperands().count() ; i++)
        {
            QString actionText = tr("Set existing equation as ");
            actionText += QString::number(i+1);

            if (i == 0)
                actionText += tr("st");
            else if (i == 1)
                actionText += tr("nd");
            else if (i == 1)
                actionText += tr("rd");
            else
                actionText += tr("th");

            actionText += tr(" operand of dropped equation");

            if (action->text() == actionText)
            {
                LogicVariable* oldEquation = this->equation;

                LogicEquation* newEquation = dynamic_cast <LogicEquation*> (droppedEquation)->clone();
                newEquation->setOperand(i, oldEquation);

                this->equation = newEquation;

                buildEquation();
                break;
            }
        }
    }

    droppedEquation = nullptr;

}
