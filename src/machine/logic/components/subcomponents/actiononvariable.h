/*
 * Copyright © 2016-2026 Clément Foucher
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

#ifndef ACTIONONVARIABLE_H
#define ACTIONONVARIABLE_H

// Parent
#include <QObject>

// Stdlib
#include <memory>

// StateS
#include "componentid.h"
#include "machinevalue.h"
class Variable;
class Equation;


/**
 * @brief The ActionOnVariable class manages actions on variables.
 *
 * It should normally preserve an action provider at all time,
 * reseting to a constant when the provider is removed.
 * However, the equation for the action provider may at any time
 * be invalid.
 *
 * For actions on Bit Vectors, range parameters allow to act on a
 * subpart of the vector. Range parameters are valid when >= 0.
 * If both parametes are invalid, the action acts on the full variable.
 * If only left range is valid, the action acts on a single bit.
 * If both ranges are valid, the action acts on the [rangeL..rangeR] sub-vector.
 */
class ActionOnVariable : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class Behavior_t : uint
	{
		none       = 0x0,
		continuous = 0x1,
		pulse      = 0x2,
		assign     = 0x4
	};

	/////
	// Static functions
public:
	static QString getBehaviorText(ActionOnVariable::Behavior_t behavior);
	static QIcon   getBehaviorIcon(ActionOnVariable::Behavior_t behavior, bool isDown = false);

	/////
	// Constructors/destructors
public:
	explicit ActionOnVariable(ComponentId targetVariableId, uint actuatorAllowedBehaviors);
	explicit ActionOnVariable(std::shared_ptr<Variable> targetVariable, uint actuatorAllowedBehaviors, ActionOnVariable::Behavior_t behavior, int rangeL, int rangeR); // Build an action on variable when machine is still being parsed

	/////
	// Object functions
public:
	// Functions specific to file loading
	void setValueProviderRaw(std::shared_ptr<Equation> newValueProvider);
	void setConditionRaw    (std::shared_ptr<Equation> newCondition);
	void checkAndFixAction();

	// Mutators
	void setBehavior     (ActionOnVariable::Behavior_t newBehavior);
	void setValueProvider(std::shared_ptr<Equation> newValueProvider);
	void setCondition    (std::shared_ptr<Equation> newCondition);
	void setRange        (int newRangeL, int newRangeR);

	// Accessors
	ComponentId                     getTargetVariableId() const;
	ActionOnVariable::Behavior_t    getBehavior()         const;
	std::shared_ptr<const Equation> getValueProvider()    const;
	std::shared_ptr<const Equation> getCondition()        const;
	int                             getRangeL()           const;
	int                             getRangeR()           const;
	uint                            getAllowedBehaviors() const;

	// Helper functions
	bool isValueProviderValid() const;

	MachineValue::Type_t getExpectedProvidedValueType()          const;
	uint                 getExpectedProvidedValueBitVectorSize() const;

private slots:
	void targetVariableTypeChangedEventHandler();
	void targetVariableMemorizedStateChangedEventHandler();
	void targetVariableInitialValueChangedEventHandler();

	void valueProviderEquationInvalidatedEventHandler();
	void conditionEquationInvalidatedEventHandler();

private:
	void connectTargetVariableSignals(std::shared_ptr<Variable> targetVariable);
	void connectValueProviderSignals();
	void connectConditionSignals();

	bool checkIfRangeFitsTargetVariable(int rangeL, int rangeR) const;

	void checkAndFixRange();
	void checkAndFixBehavior();
	void checkAndFixValueProvider();

	void setDefaultValueProvider();

	bool isValueProviderAConstant() const;

	/////
	// Signals
signals:
	void actionChangedEvent(); // Emitted when action changes whatever the cause
	void actionChangedAsAResultOfAnotherChangeEvent(); // Emitted when action had to be adapted due to a change in a variable it uses (directly or through an equation)

	/////
	// Object variables
private:
	// Immutable characteristics
	const ComponentId targetVariableId;
	const uint        actuatorAllowedBehaviors;

	// Variable characteristics
	ActionOnVariable::Behavior_t behavior;
	std::shared_ptr<Equation>    valueProvider;
	std::shared_ptr<Equation>    condition;
	int                          rangeL;
	int                          rangeR;

};

#endif // ACTIONONVARIABLE_H
