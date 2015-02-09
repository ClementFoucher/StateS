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
#include "equation.h"
#include "contextmenu.h"
#include "inverterbar.h"

// A graphic equation can either represent
// a logic equation or a logic signal

GraphicEquation::GraphicEquation(Signal* equation, bool isTemplate, QWidget* parent) :
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

    delete this->layout();

    QHBoxLayout* equationLayout = new QHBoxLayout();
    Equation* complexEquation = dynamic_cast<Equation*> (equation);

    if (complexEquation != nullptr)
    {
        if (this->isTemplate)
        {
            QString text;

            switch(complexEquation->getFunction())
            {
            case Equation::nature::notOp:
                text += "not";
                break;
            case Equation::nature::andOp:
                text += "and";
                break;
            case Equation::nature::orOp:
                text += "or";
                break;
            case Equation::nature::xorOp:
                text += "xor";
                break;
            case Equation::nature::nandOp:
                text += "nand";
                break;
            case Equation::nature::norOp:
                text += "nor";
                break;
            case Equation::nature::xnorOp:
                text += "xnor";
                break;
            case Equation::nature::equalOp:
                text += "equality";
                break;
            case Equation::nature::diffOp:
                text += "difference";
                break;
            }

            if ( (complexEquation->getFunction() != Equation::nature::notOp)   &&
                 (complexEquation->getFunction() != Equation::nature::equalOp) &&
                 (complexEquation->getFunction() != Equation::nature::diffOp)
               )
            {
                text += " " + QString::number(complexEquation->getOperandCount());
            }

            QLabel* signalText = new QLabel(text);
            signalText->setAlignment(Qt::AlignCenter);
            equationLayout->addWidget(signalText);

            this->setLayout(equationLayout);
        }
        else
        {
            for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
            {
                // Add operand
                graphicOperands[i] = new GraphicEquation(complexEquation->getOperand(i), isTemplate, this);
                equationLayout->addWidget(graphicOperands[i]);

                // Add operator, except for last operand
                if (i < complexEquation->getOperandCount() -1)
                {
                    QLabel* operatorText = nullptr;

                    switch(complexEquation->getFunction())
                    {
                    case Equation::nature::andOp:
                    case Equation::nature::nandOp:
                        operatorText = new QLabel("•");
                        break;
                    case Equation::nature::orOp:
                    case Equation::nature::norOp:
                        operatorText = new QLabel("+");
                        break;
                    case Equation::nature::xorOp:
                    case Equation::nature::xnorOp:
                        operatorText = new QLabel("⊕");
                        break;
                    case Equation::nature::equalOp:
                        operatorText = new QLabel("=");
                        break;
                    case Equation::nature::diffOp:
                        operatorText = new QLabel("≠");
                        break;
                    case Equation::nature::notOp:
                        // No intermediate sign
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
    else // Equation is actually signal (or empty)
    {
        QLabel* signalText = nullptr;

        if (equation != nullptr)
        {
            signalText = new QLabel(equation->getName());
        }
        else
        {
            signalText = new QLabel("...");
        }

        signalText->setAlignment(Qt::AlignCenter);
        equationLayout->addWidget(signalText);
        this->setLayout(equationLayout);
    }

    if (this->isTemplate)
        this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
    else
    {
        if ( this->equation == nullptr )
        {
            this->setStyleSheet("GraphicEquation {border: 1px solid red; border-radius: 10px}");
        }
        else
            this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
    }
}

void GraphicEquation::childLeavedEvent()
{
    this->setStyleSheet("GraphicEquation {border: 1px solid blue; border-radius: 10px}");
}

void GraphicEquation::childEnteredEvent()
{
    this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
}

void GraphicEquation::replaceEquation(Signal* newSignal)
{
    // Delete current equation if it is not a single variable
    Equation* currentEquation = dynamic_cast <Equation*> (equation);
    if (currentEquation != nullptr)
        delete currentEquation;
    else if ( (equation != nullptr) ) //&& (equation->isSystemConstant()) )
        delete equation;

    this->equation = nullptr;

    // Test if equation is actually equation or just single variable
    if (newSignal != nullptr)
    {
        Equation* equation = dynamic_cast <Equation*> (newSignal);
        if (equation != nullptr)
            this->equation = equation->clone();
        else
            this->equation = newSignal;
    }
    else
        this->equation = nullptr;

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
    Equation* complexEquation = dynamic_cast<Equation*> (equation);

    if (complexEquation != nullptr)
    {
        for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
        {
            if (graphicOperands[i]->getLogicEquation() != complexEquation->getOperand(i))
            {
                complexEquation->setOperand(i, graphicOperands[i]->getLogicEquation());
            }
        }
    }

    GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
    if (parentEquation != nullptr)
        parentEquation->updateEquation();
}

Signal* GraphicEquation::getLogicEquation() const
{
    return equation;
}

void GraphicEquation::enterEvent(QEvent* event)
{
    if (this->isTemplate)
    {
        this->setStyleSheet("GraphicEquation {border: 1px solid #FF9900; border-radius: 10px}");
    }
    else
    {
        this->setStyleSheet("GraphicEquation {border: 1px solid blue; border-radius: 10px}");

        GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
        if (parentEquation != nullptr)
        {
            parentEquation->childEnteredEvent();
        }
    }

    QFrame::enterEvent(event);
}


void GraphicEquation::leaveEvent(QEvent* event)
{
    this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");

    if (!(this->isTemplate))
    {
        GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
        if (parentEquation != nullptr)
            parentEquation->childLeavedEvent();
    }

    QFrame::leaveEvent(event);
}

void GraphicEquation::mousePressEvent(QMouseEvent* event)
{
    if (isTemplate)
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
    if (!isTemplate)
    {
        ContextMenu* menu = new ContextMenu();
        menu->addTitle(tr("Equation:") +  " <i>" + equation->getText() + "</i>");

        Equation* complexEquation = dynamic_cast<Equation*> (equation);
        if (complexEquation != nullptr)
        {
            if (complexEquation->getFunction() != Equation::nature::notOp)
            {
                menu->addAction(tr("Add one operand to that operator"));

                if (complexEquation->getOperandCount() > 2)
                {
                    menu->addAction(tr("Remove one operand from that operator"));
                }
            }
        }

        menu->addAction(tr("Delete"));

        menu->addAction(tr("Cancel"));

        menu->popup(this->mapToGlobal(event->pos()));

        connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenu);
    }
}

void GraphicEquation::dropEvent(QDropEvent* event)
{
    const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData*>(event->mimeData());

    if (mimeData != nullptr)
    {
        // Obtain new equation
        droppedEquation = mimeData->getEquation()->getLogicEquation();

        // Check operand size for adequation with parent
        if ( /*(!droppedEquation->isSystemConstant()) &&*/ (droppedEquation->getSize() != 0) )
        {
            GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
            if (parentEquation != nullptr)
            {
                if ( (((Equation*)parentEquation->getLogicEquation())->getFunction() != Equation::nature::equalOp) &&
                     (((Equation*)parentEquation->getLogicEquation())->getFunction() != Equation::nature::diffOp)
                     )
                {
                    if ( (parentEquation->getLogicEquation()->getSize() != 0) && (parentEquation->getLogicEquation()->getSize() != droppedEquation->getSize()) )
                    {
                        ContextMenu* menu = ContextMenu::createErrorMenu(tr("Error: equation size is") + " " + QString::number(parentEquation->getLogicEquation()->getSize()) + " " +
                                                                         tr("while dropped operand size is") + " " + QString::number(droppedEquation->getSize())
                                                                         );
                        menu->popup(mapToGlobal(event->pos()));
                        return;
                    }
                }
            }
        }

        // Automatically replace empty operands
        if (this->equation == nullptr)
        {
            replaceEquation(droppedEquation);
        }
        else
        {
            // Ask what to do
            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("What should I do?"));
            menu->addSubTitle(tr("Existing equation:") +  " <i>"  + equation->getText() + "</i>");
            menu->addSubTitle(tr("Dropped equation:") + " <i> " + droppedEquation->getText() + "</i>");

            QWidgetAction* a = new QWidgetAction(menu);
            a->setText(tr("Replace existing equation by dropped equation"));
            a->setToolTip(tr("New equation would be:") + " <i> " + droppedEquation->getText() + "</i>");
            menu->addAction(a);

            Equation* droppedComplexEquation = dynamic_cast <Equation*> (droppedEquation);
            if (droppedComplexEquation != nullptr)
            {
                for (int i = 0 ; i < droppedComplexEquation->getOperands().count() ; i++)
                {
                    QString actionText = tr("Set existing equation as") + " ";
                    actionText += QString::number(i+1);

                    if (i == 0)
                        actionText += tr("st");
                    else if (i == 1)
                        actionText += tr("nd");
                    else if (i == 1)
                        actionText += tr("rd");
                    else
                        actionText += tr("th");

                    actionText += " " + tr("operand of dropped equation");

                    a = new QWidgetAction(menu);
                    a->setText(actionText);

                    // Build tooltip
                    Equation* newEquation = (Equation*)droppedComplexEquation->clone();
                    newEquation->setOperand(i, this->equation);

                    a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");
                    delete newEquation;

                    menu->addAction(a);
                }
            }
            menu->addAction(tr("Cancel"));

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenu);
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
        replaceEquation(nullptr);
    }
    else if (action->text() == tr("Add one operand to that operator"))
    {
        Equation* complexEquation = dynamic_cast<Equation*>(this->equation);

        complexEquation->increaseOperandCount();

        this->buildEquation();

        GraphicEquation* parentEquation = dynamic_cast<GraphicEquation*>(this->parentWidget());
        if (parentEquation != nullptr)
            parentEquation->updateEquation();
    }
    else if (action->text() == tr("Remove one operand from that operator"))
    {
        bool valid = false;

        Equation* complexEquation = dynamic_cast<Equation*>(this->equation);
        Signal* operandToRemove = complexEquation->getOperand(complexEquation->getOperandCount() - 1);

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete last oprand?") + "<br />" + tr("Content is:") + " " + operandToRemove->getText(), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
            valid = true;

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
        Equation* droppedComplexEquation = dynamic_cast <Equation*> (droppedEquation);
        for (int i = 0 ; i < droppedComplexEquation->getOperands().count() ; i++)
        {
            QString actionText = tr("Set existing equation as") + " ";
            actionText += QString::number(i+1);

            if (i == 0)
                actionText += tr("st");
            else if (i == 1)
                actionText += tr("nd");
            else if (i == 1)
                actionText += tr("rd");
            else
                actionText += tr("th");

            actionText += " " + tr("operand of dropped equation");

            if (action->text() == actionText)
            {
                Equation* newEquation = droppedComplexEquation->clone();
                newEquation->setOperand(i, this->equation);

                this->equation = newEquation;

                buildEquation();
                break;
            }
        }
    }

    droppedEquation = nullptr;

}
