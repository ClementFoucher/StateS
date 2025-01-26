/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "simulatorconfigurator.h"

// Qt classes
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>


SimulatorConfigurator::SimulatorConfigurator(QWidget* parent) :
	QWidget(parent)
{
	auto mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);
	mainLayout->setAlignment(Qt::AlignTop);

	auto actionsLabel = new QLabel(tr("Select desired actions behavior"));
	actionsLabel->setAlignment(Qt::AlignHCenter);
	mainLayout->addWidget(actionsLabel);

	auto actionsNote = new QLabel("<i>" + tr("Note: the default behavior matches a digital electronics implementation where memorized actions are synchronous and other actions are combinatorial") + "</i>");
	actionsNote->setAlignment(Qt::AlignHCenter);
	actionsNote->setWordWrap(true);
	mainLayout->addWidget(actionsNote);


	auto memorizedStateLabel = new QLabel(tr("Memorized actions on states:"));
	mainLayout->addWidget(memorizedStateLabel);

	this->memorizedState = new QComboBox();
	this->memorizedState->addItem(tr("Activate one cycle after state is entered"));
	this->memorizedState->addItem(tr("Activate immediately when state is entered"));
	mainLayout->addWidget(memorizedState);


	auto continuousStateLabel = new QLabel(tr("Continous actions on states:"));
	mainLayout->addWidget(continuousStateLabel);

	this->continuousState = new QComboBox();
	this->continuousState->addItem(tr("Activate immediately after state is entered"));
	this->continuousState->addItem(tr("Activate one cycle after state is entered"));
	mainLayout->addWidget(continuousState);


	auto memorizedTransitionLabel = new QLabel(tr("Memorized actions on transitions:"));
	mainLayout->addWidget(memorizedTransitionLabel);

	this->memorizedTransition = new QComboBox();
	this->memorizedTransition->addItem(tr("Activate immediately after transition is crossed"));
	this->memorizedTransition->addItem(tr("Prepare before transition is crossed"));
	mainLayout->addWidget(memorizedTransition);


	auto pulseTransitionLabel = new QLabel(tr("Pulse actions on transitions:"));
	mainLayout->addWidget(pulseTransitionLabel);

	this->pulseTransition = new QComboBox();
	this->pulseTransition->addItem(tr("Prepare before transition is crossed"));
	this->pulseTransition->addItem(tr("Activate after transition is crossed"));
	mainLayout->addWidget(pulseTransition);
}

SimulationBehavior_t SimulatorConfigurator::getMemorizedStateActionBehavior() const
{
	if (this->memorizedState->currentIndex() == 0)
	{
		return SimulationBehavior_t::after;
	}
	else
	{
		return SimulationBehavior_t::immediately;
	}
}

SimulationBehavior_t SimulatorConfigurator::getContinuousStateActionBehavior() const
{
	if (this->continuousState->currentIndex() == 0)
	{
		return SimulationBehavior_t::immediately;
	}
	else
	{
		return SimulationBehavior_t::after;
	}
}

SimulationBehavior_t SimulatorConfigurator::getMemorizedTransitionActionBehavior() const
{
	if (this->memorizedTransition->currentIndex() == 0)
	{
		return SimulationBehavior_t::immediately;
	}
	else
	{
		return SimulationBehavior_t::prepare;
	}
}

SimulationBehavior_t SimulatorConfigurator::getPulseTransitionActionBehavior() const
{
	if (this->pulseTransition->currentIndex() == 0)
	{
		return SimulationBehavior_t::prepare;
	}
	else
	{
		return SimulationBehavior_t::immediately;
	}
}
