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
#include "graphicequation.h"

// Qt classes
#include <QDrag>
#include <QMouseEvent>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVariant>

// Debug
#include <QDebug>

// StateS classes
#include "equationmimedata.h"
#include "equation.h"
#include "contextmenu.h"
#include "inverterbar.h"
#include "equationeditor.h"


// A graphic equation can either represent
// a logic equation or a logic signal

GraphicEquation::GraphicEquation(shared_ptr<Signal> equation, bool isTemplate, QWidget* parent) :
    QFrame(parent)
{
    this->equation   = equation;
    this->isTemplate = isTemplate;

    if (!isTemplate)
        this->setAcceptDrops(true);

    if (this->parentEquation() == nullptr)
        // We are top level
        this->rootEquation = equation;

    buildEquation();
}

// Builds graphical representation of equation
// based on current logic equation pointer
void GraphicEquation::buildEquation()
{
    // Clear previous content
    QObjectList toDelete = this->children();
    qDeleteAll(toDelete);

    // Main layout (will be embedded in a vertical layout if equation is inverted)
    QHBoxLayout* equationLayout = new QHBoxLayout();

    shared_ptr<Signal> equationAsSignal = this->equation.lock();

    if (equationAsSignal != nullptr)
    {
        shared_ptr<Equation> equationAsEquation = dynamic_pointer_cast<Equation> (equationAsSignal);

        if (equationAsEquation != nullptr)
        {
            if (this->isTemplate)
            {
                QString text;

                switch(equationAsEquation->getFunction())
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
                case Equation::nature::identity:
                    // Nothing
                    break;
                }

                if ( (equationAsEquation->getFunction() != Equation::nature::notOp)   &&
                     (equationAsEquation->getFunction() != Equation::nature::equalOp) &&
                     (equationAsEquation->getFunction() != Equation::nature::diffOp)
                     )
                {
                    text += " " + QString::number(equationAsEquation->getOperandCount());
                }

                QLabel* signalText = new QLabel(text);
                signalText->setAlignment(Qt::AlignCenter);
                equationLayout->addWidget(signalText);

                this->setLayout(equationLayout);
            }
            else
            {
                for (uint i = 0 ; i < equationAsEquation->getOperandCount() ; i++)
                {
                    // Add operand
                    equationLayout->addWidget(new GraphicEquation(equationAsEquation->getOperand(i), false, this));

                    // Add operator, except for last operand
                    if (i < equationAsEquation->getOperandCount() -1)
                    {
                        QLabel* operatorText = nullptr;

                        switch(equationAsEquation->getFunction())
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
                        case Equation::nature::identity:
                            // No intermediate sign
                            break;
                        }

                        if (operatorText != nullptr)
                        {
                            operatorText->setAlignment(Qt::AlignCenter);
                            equationLayout->addWidget(operatorText);
                        }
                    }

                    if (!equationAsEquation->isInverted())
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

                this->setToolTip(tr("This equation") + " " + tr("is size") + " " + QString::number(equationAsEquation->getSize()));
            }
        }
        else // Equation is actually signal
        {
            QLabel* signalText = nullptr;

            signalText = new QLabel(equationAsSignal->getName());
            this->setToolTip(tr("Signal") + " " + equationAsSignal->getName() + " " + tr("is size") + " " + QString::number(equationAsSignal->getSize()));

            signalText->setAlignment(Qt::AlignCenter);
            equationLayout->addWidget(signalText);
            this->setLayout(equationLayout);
        }
    }
    else
    {
        // Empty equation
        QLabel* emptyText = new QLabel("...");

        emptyText->setAlignment(Qt::AlignCenter);
        equationLayout->addWidget(emptyText);
        this->setLayout(equationLayout);
    }

    this->setDefaultBorderColor();
}

// Set passive border color:
// neutral unless current equation is incorrect
void GraphicEquation::setDefaultBorderColor()
{
    if (this->isTemplate)
        this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
    else
    {
        shared_ptr<Signal> equationAsSignal = this->equation.lock();

        if ( ( equationAsSignal == nullptr ) || (equationAsSignal->getCurrentValue().isNull()) )
        {
            this->setStyleSheet("GraphicEquation {border: 1px solid red; border-radius: 10px}");

            if ( equationAsSignal == nullptr )
                this->setToolTip(tr("Empty operand"));
            else
            {
                shared_ptr<Equation> equationAsEquation = dynamic_pointer_cast<Equation>(equationAsSignal);

                if (equationAsEquation != nullptr)
                {
                    if (equationAsEquation->getComputationFailureCause() == Equation::computationFailureCause::nullOperand)
                        this->setToolTip(tr("Empty operand"));
                    else if (equationAsEquation->getComputationFailureCause() == Equation::computationFailureCause::incompleteOperand)
                        this->setToolTip(tr("One operand is not correct"));
                    else if (equationAsEquation->getComputationFailureCause() == Equation::computationFailureCause::sizeMismatch)
                        this->setToolTip(tr("The sizes of operands does not match between each other"));
                }
            }
        }
        else
        {
            this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
        }
    }
}

// Set active border color
void GraphicEquation::setHilightedBorderColor()
{
    this->setStyleSheet("GraphicEquation {border: 1px solid blue; border-radius: 10px}");
}

// Scans QWidget hierarchy to find closest GraphicEquation
// parent. Returns nullptr if we are top level in this hierarchy.
GraphicEquation* GraphicEquation::parentEquation() const
{
    GraphicEquation* parentEquation = nullptr;

    QWidget* parentWidget = this->parentWidget();
    while (parentWidget != nullptr)
    {
        parentEquation = dynamic_cast<GraphicEquation*>(parentWidget);
        if (parentEquation != nullptr)
            break; // Found

        // No need to go beyond equation editor
        EquationEditor* editor = dynamic_cast<EquationEditor*>(parentWidget);
        if (editor != nullptr)
            break; // Not found

        parentWidget = parentWidget->parentWidget();
    }

    return parentEquation;
}

// Handle user input to replace equation.
// This is done by telling parent to replace ourself by
// new equation, unless we are top level.
// Graphic equation is rebuilt by parent, which destoys
// current object.
void GraphicEquation::replaceEquation(shared_ptr<Signal> newEquation)
{
    // Find equation parent
    GraphicEquation* parentEquation = this->parentEquation();

    if (parentEquation == nullptr)
    {
        // We are top level
        this->equation = newEquation;
        this->rootEquation = newEquation;

        buildEquation();
    }
    else
    {
        // Parent will handle replacement
        parentEquation->updateEquation(this->equation.lock(), newEquation);
    }
}

// Replaces one operand with the given one in logic equation.
// This function is typically called by children operand
// to replace themselves.
// Graphic equation is rebuild after logice equation update.
void GraphicEquation::updateEquation(shared_ptr<Signal> oldOperand, shared_ptr<Signal> newOperand)
{
    shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (this->equation.lock());

    if (complexEquation != nullptr) // This IS true, because we are called by an operand of ours. Anyway...
    {
        for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
        {
            if (complexEquation->getOperand(i) == oldOperand)
            {
                complexEquation->setOperand(i, newOperand);
                break;
            }
        }

        this->buildEquation();
    }
    else
        qDebug() << "(Graphic equation) Error! Update equation called on something not an equation.";
}

// Returns the equation currently represented by the graphic object.
// This is typically a pointer to a part of a logic equation,
// unless we are top level, then the whole equation is returned.
shared_ptr<Signal> GraphicEquation::getLogicEquation() const
{
    if (rootEquation != nullptr)
        return rootEquation;
    else
        return equation.lock();
}

// Triggered when mouse enters widget
void GraphicEquation::enterEvent(QEvent*)
{
    GraphicEquation* parentEquation = this->parentEquation();

    if (parentEquation != nullptr)
        parentEquation->enterChildrenEventHandler();

    this->setHilightedBorderColor();
}

// Triggered when mouse leaves widget
// (not trigerred when children is entered)
void GraphicEquation::leaveEvent(QEvent*)
{
    GraphicEquation* parentEquation = this->parentEquation();

    if (parentEquation != nullptr)
        parentEquation->leaveChildrenEventHandler();

    this->setDefaultBorderColor();
}

// Generate drag-n-drop image when mouse is pressed
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

    QFrame::mousePressEvent(event);
}

// Triggered when mouse enters widget while dragging an object
void GraphicEquation::dragEnterEvent(QDragEnterEvent* event)
{
    if (!this->isTemplate)
    {
        this->setHilightedBorderColor();

        // Check if object is actually a graphic equation
        const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData *>(event->mimeData());
        if (mimeData != nullptr)
        {
            event->acceptProposedAction();
        }
    }
}

// Trigerred when mouse leaves widget while dragging an object
// (also trigerred when child is entered)
void GraphicEquation::dragLeaveEvent(QDragLeaveEvent*)
{
    this->setDefaultBorderColor();
}

// Triggerd by right-click
void GraphicEquation::contextMenuEvent(QContextMenuEvent* event)
{
    if (!isTemplate)
    {
        shared_ptr<Signal> equation = this->equation.lock();

        if (equation != nullptr)
        {
            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("Equation:") +  " <i>" + equation->getText() + "</i>");

            QVariant data;
            data.convert(QVariant::Int);

            shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (equation);
            if (complexEquation != nullptr)
            {
                if ( (complexEquation->getFunction() != Equation::nature::notOp) &&
                     (complexEquation->getFunction() != Equation::nature::equalOp) &&
                     (complexEquation->getFunction() != Equation::nature::diffOp)
                   )
                {
                    QAction* actionAddOperand = menu->addAction(tr("Add one operand to that operator"));
                    data.setValue((int)ContextAction::IncrementOperandCount);
                    actionAddOperand->setData(data);

                    if (complexEquation->getOperandCount() > 2)
                    {
                        QAction* actionRemoveOperand = menu->addAction(tr("Remove one operand from that operator"));
                        data.setValue((int)ContextAction::DecrementOperandCount);
                        actionRemoveOperand->setData(data);
                    }
                }
            }

            QAction* actionDelete = menu->addAction(tr("Delete"));
            data.setValue((int)ContextAction::DeleteEquation);
            actionDelete->setData(data);

            QAction* actionCancel = menu->addAction(tr("Cancel"));
            data.setValue((int)CommonAction::Cancel);
            actionCancel->setData(data);

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenuEventHandler);
        }
    }
}

void GraphicEquation::enterChildrenEventHandler()
{
    this->setDefaultBorderColor();
}


void GraphicEquation::leaveChildrenEventHandler()
{
    this->setHilightedBorderColor();
}


void GraphicEquation::dropEvent(QDropEvent* event)
{
    const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData*>(event->mimeData());

    if (mimeData != nullptr)
    {
        // Obtain new equation
        this->droppedEquation = mimeData->getEquation()->getLogicEquation();

        // Automatically replace empty operands
        if (this->equation.lock() == nullptr)
        {
            replaceEquation(droppedEquation);
        }
        else
        {
            // Ask what to do
            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("What should I do?"));
            menu->addSubTitle(tr("Existing equation:") +  " <i>"  + equation.lock()->getText() + "</i>");
            menu->addSubTitle(tr("Dropped equation:") + " <i> " + droppedEquation->getText() + "</i>");

            QVariant data;
            data.convert(QVariant::Int);

            QAction* a = new QAction(menu);
            a->setText(tr("Replace existing equation by dropped equation"));
            a->setToolTip(tr("New equation would be:") + " <i> " + droppedEquation->getText() + "</i>");
            data.setValue((int)DropAction::ReplaceExisting);
            a->setData(data);
            menu->addAction(a);

            shared_ptr<Equation> droppedComplexEquation = dynamic_pointer_cast<Equation> (droppedEquation);
            if (droppedComplexEquation != nullptr)
            {
                QString actionText = tr("Set existing equation as operand of dropped equation");

                a = new QAction(menu);
                a->setText(actionText);
                data.setValue((int)DropAction::ExistingAsOperand);
                a->setData(data);

                // Build tooltip
                shared_ptr<Equation> newEquation = droppedComplexEquation->clone();
                newEquation->setOperand(0, this->equation.lock());

                a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");
                newEquation.reset();

                menu->addAction(a);

            }

            a = menu->addAction(tr("Cancel"));
            data.setValue((int)CommonAction::Cancel);
            a->setData(data);

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenuEventHandler);
        }

        event->acceptProposedAction();
    }
}


void GraphicEquation::treatMenuEventHandler(QAction* action)
{
    shared_ptr<Equation> newEquation;
    shared_ptr<Equation> complexEquation;
    shared_ptr<Signal> signalEquation;
    bool valid;

    QVariant data = action->data();
    int dataValue = data.toInt();
    switch (dataValue)
    {
    case CommonAction::Cancel:
        break;

    case DropAction::ReplaceExisting:

        // Nothing should be done after that line because it
        // will cause parent to rebuild, deleting this
        replaceEquation(this->droppedEquation);

        break;

    case DropAction::ExistingAsOperand:

        complexEquation = dynamic_pointer_cast<Equation>(this->droppedEquation);
        newEquation = complexEquation->clone();
        newEquation->setOperand(0, this->equation.lock());

        // Nothing should be done after that line because it
        // will cause parent to rebuild, deleting this
        this->replaceEquation(newEquation);

        break;

    case ContextAction::DeleteEquation:

        valid = false;

        signalEquation = this->equation.lock();

        if (signalEquation != nullptr)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete equation?") + "<br />" + tr("Content is:") + " " + signalEquation->getText(), QMessageBox::Ok | QMessageBox::Cancel);
            if (reply == QMessageBox::StandardButton::Ok)
                valid = true;
        }
        else
            valid = true;

        if (valid)
        {
            replaceEquation(nullptr);
        }

        break;

    case ContextAction::IncrementOperandCount:

        complexEquation = dynamic_pointer_cast<Equation>(this->equation.lock());

        complexEquation->increaseOperandCount();

        this->buildEquation();

        break;

    case ContextAction::DecrementOperandCount:

        valid = false;

        complexEquation = dynamic_pointer_cast<Equation>(this->equation.lock());
        signalEquation = complexEquation->getOperand(complexEquation->getOperandCount() - 1);

        if (signalEquation != nullptr)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete last oprand?") + "<br />" + tr("Content is:") + " " + signalEquation->getText(), QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::StandardButton::Ok)
                valid = true;
        }
        else
        {
            valid = true;
        }

        if (valid)
        {
            complexEquation->decreaseOperandCount();

            this->buildEquation();
        }

        break;

    }
}
