/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "hinttab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// Debug
#include <QDebug>

// StateS classes
#include "machinemanager.h"
#include "machinebuilder.h"
#include "machinecomponentvisualizer.h"
#include "collapsiblewidgetwithtitle.h"


HintTab::HintTab(shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent) :
    QWidget(parent)
{
	this->machineComponentView = machineComponentView;

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);

	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	connect(machineBuilder.get(), &MachineBuilder::changedToolEvent,      this, &HintTab::toolChangedEventHandler);
	connect(machineBuilder.get(), &MachineBuilder::singleUseToolSelected, this, &HintTab::singleUsetoolChangedEventHandler);

	//
	// Hints

	this->hintDisplay = new CollapsibleWidgetWithTitle(this);
	layout->addWidget(this->hintDisplay);

	this->updateHint(MachineBuilderTool_t::none);

	//
	// Machine visualization

	this->machineDisplay = new CollapsibleWidgetWithTitle(tr("Component visualization"), machineComponentView.get(), this);
	layout->addWidget(this->machineDisplay);
}

void HintTab::setHintCollapsed(bool collapse)
{
	this->hintDisplay->setCollapsed(collapse);
}

void HintTab::setVisuCollapsed(bool collapse)
{
	this->machineDisplay->setCollapsed(collapse);
}

bool HintTab::getHintCollapsed()
{
	return this->hintDisplay->getCollapsed();
}

bool HintTab::getVisuCollapsed()
{
	return this->machineDisplay->getCollapsed();
}

void HintTab::showEvent(QShowEvent* e)
{
	// Ensure we get the view back
	shared_ptr<MachineComponentVisualizer> l_machineComponentView = this->machineComponentView.lock();
	if (l_machineComponentView != nullptr)
	{
		this->machineDisplay->setContent(tr("Component visualization"), l_machineComponentView.get());
	}

	QWidget::showEvent(e);
}

void HintTab::toolChangedEventHandler(MachineBuilderTool_t newTool)
{
	this->updateHint(newTool);
}

void HintTab::singleUsetoolChangedEventHandler(MachineBuilderSingleUseTool_t tempTool)
{
	if (tempTool == MachineBuilderSingleUseTool_t::none)
	{
		shared_ptr<MachineBuilder> machineBuiler = machineManager->getMachineBuilder();

		if (machineBuiler != nullptr)
			this->updateHint(machineBuiler->getTool());
		else
			this->updateHint(MachineBuilderTool_t::none);
	}
	else
	{
		QString title;
		QString hint;

		title = tr("Hint:") + " ";

		switch(tempTool)
		{
		case MachineBuilderSingleUseTool_t::drawTransitionFromScene:
			title +=  tr("Drawing a transition");

			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state") + " " + tr("to make it") + " " + tr("the target") + " " + tr("of this transition") + ".";
			hint += "<br />";
			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition insertion") + ".";
			hint += "<br />";

			break;
		case MachineBuilderSingleUseTool_t::editTransitionSource:
			title +=  tr("Editing a transition");

			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("to make it") + " " + tr("the source") + " " + tr("of this transition") + ".";
			hint += "<br />";
			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition edition") + ".";
			hint += "<br />";

			break;
		case MachineBuilderSingleUseTool_t::editTransitionTarget:
			title +=  tr("Editing a transition");

			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a state")  + " " + tr("to make it") + " " + tr("the target") + " " + tr("of this transition") + ".";
			hint += "<br />";
			hint += "<br />";
			hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to cancel") + " " + tr("transition edition") + ".";
			hint += "<br />";

			break;
		default:
			break;
		}

		this->hintDisplay->setContent(title, hint, true);
	}
}

void HintTab::updateHint(MachineBuilderTool_t newTool)
{
	QString title;
	QString hint;

	title = tr("Hint:") + " ";

	switch(newTool)
	{
	case MachineBuilderTool_t::none:
		title +=  tr("Navigation");

		hint += "<br />";
		hint += tr("Use <i>mouse middle button (wheel)</i> to move scene:");
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

		hint += "<br />";
		hint += tr("Tabs:");
		hint += "<br />";
		hint += tr("Machine tab is used to edit machine name and signals");
		hint += "<br />";
		hint += tr("State/transition tab is used to edit the currently selected state or transition (actions, conditions, etc.)");
		hint += "<br />";
		hint += tr("Simulate tab allows for machine simluation");
		hint += "<br />";
		hint += tr("Verify tab provide tools for machine correctness verification");

		break;
	case MachineBuilderTool_t::initial_state:
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
	case MachineBuilderTool_t::state:
		title +=  tr("Adding a state");

		hint += "<br />";
		hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("to add") + " " + tr("a state") + " " + tr("at mouse position") + ".";
		hint += "<br />";
		hint += "<br />";
		hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to unselect current tool") + ".";
		hint += "<br />";

		break;
	case MachineBuilderTool_t::transition:
		title +=  tr("Drawing a transition");

		hint += "<br />";
		hint += tr("Use") + " <i>" + tr("left-click") + "</i> " + tr("on a first state")  + " " + tr("to begin") + " " + tr("drawing a transition") + " "  + tr("from this state") + ".";
		hint += "<br />";
		hint += tr("Then") + " <i>" + tr("left-click") + "</i> " + tr("on a second state")  + " " + tr("to make it") + " " + tr("the target of the transition") + ".";
		hint += "<br />";
		hint += "<br />";
		hint += tr("Use") + " <i>" + tr("right-click") + "</i> " + tr("to unselect current tool") + ".";
		hint += "<br />";

		break;
	}

	this->hintDisplay->setContent(title, hint, true);
}
