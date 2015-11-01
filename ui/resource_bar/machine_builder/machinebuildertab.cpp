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
#include "machinebuildertab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// Debug
#include <QDebug>

// StateS classes
#include "fsmtoolspanel.h"
#include "machinecomponentvisualizer.h"
#include "collapsiblewidgetwithtitle.h"
#include "dynamiclineedit.h"
#include "fsm.h"


MachineBuilderTab::MachineBuilderTab(shared_ptr<Machine> machine, shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent) :
    QWidget(parent)
{
    this->machineComponentView = machineComponentView;
    this->machine              = machine;

    if (machine != nullptr)
    {
        shared_ptr<MachineBuilder> machineBuilder = machine->getMachineBuilder();

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);

        // Tools

        connect(machineBuilder.get(), &MachineBuilder::changedToolEvent,      this, &MachineBuilderTab::toolChangedEventHandler);
        connect(machineBuilder.get(), &MachineBuilder::singleUseToolSelected, this, &MachineBuilderTab::singleUsetoolChangedEventHandler);

        shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(machine);

        if (fsm != nullptr)
        {
            QLabel* title = new QLabel("<b>" + tr("FSM editor") + "</b>");
            title->setAlignment(Qt::AlignCenter);
            layout->addWidget(title);

            layout->addWidget(new FsmToolsPanel(machineBuilder));

            QHBoxLayout* nameLayout = new QHBoxLayout();

            QLabel* machineNameLabel = new QLabel(tr("Component name:"));
            this->stateName = new DynamicLineEdit(fsm->getName(), true);

            connect(this->stateName, &DynamicLineEdit::newTextAvailableEvent, this, &MachineBuilderTab::nameTextChangedEventHandler);
            connect(this->stateName, &DynamicLineEdit::userCancelEvent,       this, &MachineBuilderTab::updateContent);

            connect(machine.get(), &Machine::nameChangedEvent, this, &MachineBuilderTab::updateContent);

            nameLayout->addWidget(machineNameLabel);
            nameLayout->addWidget(this->stateName);

            layout->addLayout(nameLayout);

        }
        else
        {
            qDebug() << "(Drawing tool bar:) Error, unknown machine type";
        }

        // Machine visualization
        this->machineDisplay = new CollapsibleWidgetWithTitle(tr("Component visualization"), machineComponentView.get());
        layout->addWidget(this->machineDisplay);

        // Hints


        this->hintDisplay = new CollapsibleWidgetWithTitle();
        layout->addWidget(this->hintDisplay);

        this->updateHint(MachineBuilder::tool::none);
    }
}

void MachineBuilderTab::showEvent(QShowEvent* e)
{
    // Ensure we get the view back
    shared_ptr<MachineComponentVisualizer> machineComponentView = this->machineComponentView.lock();

    if (machineComponentView != nullptr)
    {
        this->machineDisplay->setContent(tr("Component visualization"), machineComponentView.get());
    }

    QWidget::showEvent(e);
}

/**
 * @brief MachineBuilderTab::mousePressEvent
 * Used to allow validation of name wherever we click,
 * otherwise clicks inside this widget won't validate input.
 */
void MachineBuilderTab::mousePressEvent(QMouseEvent *e)
{
    this->stateName->clearFocus();

    QWidget::mousePressEvent(e);
}

void MachineBuilderTab::toolChangedEventHandler(MachineBuilder::tool newTool)
{
    this->updateHint(newTool);
}

void MachineBuilderTab::singleUsetoolChangedEventHandler(MachineBuilder::singleUseTool tempTool)
{
    if (tempTool == MachineBuilder::singleUseTool::none)
    {
        shared_ptr<Machine> l_machine = this->machine.lock();

        if (l_machine!= nullptr)
            this->updateHint(l_machine->getMachineBuilder()->getTool());
        else
            this->updateHint(MachineBuilder::tool::none);
    }
    else
    {
        QString title;
        QString hint;

        title = tr("Hint:") + " ";

        switch(tempTool)
        {
        case MachineBuilder::singleUseTool::drawTransitionFromScene:
            title +=  tr("Drawing a transition");

            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state") + " " + tr("to make it") + " " + tr("the target") + " " + tr("of this transition") + ".";
            hint += "<br />";
            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition insertion") + ".";
            hint += "<br />";

            break;
        case MachineBuilder::singleUseTool::beginDrawTransitionFromTool:
            title +=  tr("Drawing a transition");

            hint += "<br />";
            hint += tr("Release") + " <i>" + tr("left-click") + "</i> " + tr("on a state") + " " + tr("to make it") + " " + tr("the target") + " " + tr("of this transition") + ".";
            hint += "<br />";
            hint += "<br />";
            hint += tr("Release") + " <i>" + tr("left-click") + "</i> " + tr("anywhere else") + " " + tr("to cancel") + " " + tr("transition insertion") + ".";
            hint += "<br />";

            break;
        case MachineBuilder::singleUseTool::editTransitionSource:
            title +=  tr("Editing a transition");

            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("to make it") + " " + tr("the source") + " " + tr("of this transition") + ".";
            hint += "<br />";
            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition editing") + ".";
            hint += "<br />";

            break;
        case MachineBuilder::singleUseTool::editTransitionTarget:
            title +=  tr("Editing a transition");

            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("to make it") + " " + tr("the target") + " " + tr("of this transition") + ".";
            hint += "<br />";
            hint += "<br />";
            hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition editing") + ".";
            hint += "<br />";

            break;
        default:
            break;
        }

        QLabel* hintText = new QLabel(hint);
        hintText->setAlignment(Qt::AlignCenter);
        hintText->setWordWrap(true);

        this->hintDisplay->setContent(title, hintText, true);
    }
}

void MachineBuilderTab::nameTextChangedEventHandler(const QString& name)
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        if (name != l_machine->getName())
        {
            bool accepted = l_machine->setName(name);

            if (!accepted)
                this->stateName->markAsErroneous();
        }
    }
}

void MachineBuilderTab::updateContent()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->stateName->setText(l_machine->getName());
    }
    else
    {
        this->stateName->setText("<i>(" + tr("No machine") + ")</i>");
        this->stateName->setEnabled(false);
    }
}

void MachineBuilderTab::updateHint(MachineBuilder::tool newTool)
{
    QString title;
    QString hint;

    title = tr("Hint:") + " ";

    switch(newTool)
    {
    case MachineBuilder::tool::none:
    case MachineBuilder::tool::quittingTool:
        title +=  tr("Navigation");

        hint += "<br />";
        hint += tr("Use <i>mouse center button (wheel)</i> to move scene:");
        hint += "<br />";
        hint += tr("Press and hold") + " " + tr("wheel") + " " + tr("for free move") + ",";
        hint += "<br />";
        hint += tr("Scroll") + " " + tr("wheel") + " " + tr("to move vertically") + ",";
        hint += "<br />";
        hint += tr("Scroll") + " " + tr("wheel") + " " + tr("while holding") + " <i>" + tr("shift") + "</i> " + tr("to move horizontally") + ".";
        hint += "<br />";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("ctrl") + "+" + tr("mouse wheel") + "</i> " + tr("to zoom in/out") + ".";
        hint += "<br />";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("or") + " " + tr("a transition") + " " + tr("to select it") + ",";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to display context menu") + ".";
        hint += "<br />";

        break;
    case MachineBuilder::tool::initial_state:
        title +=  tr("Adding an initial state");

        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("to add") + " " + tr("a state") + " " + tr("at mouse position") + ".";
        hint += "<br />";
        hint += tr("This state will be set as the initial state of this machine") + ".";
        hint += "<br />";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to unselect current tool") + ".";
        hint += "<br />";

        break;
    case MachineBuilder::tool::state:
        title +=  tr("Adding a state");

        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("to add") + " " + tr("a state") + " " + tr("at mouse position") + ".";
        hint += "<br />";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to unselect current tool") + ".";
        hint += "<br />";

        break;
    case MachineBuilder::tool::transition:
        title +=  tr("Drawing a transition");

        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("and maintain button") + " " + tr("to begin") + " " + tr("drawing a transition") + " "  + tr("from this state") + ".";
        hint += "<br />";
        hint += "<br />";
        hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to unselect current tool") + ".";
        hint += "<br />";

        break;
    }

    QLabel* hintText = new QLabel(hint);
    hintText->setAlignment(Qt::AlignCenter);
    hintText->setWordWrap(true);

    this->hintDisplay->setContent(title, hintText, true);
}
