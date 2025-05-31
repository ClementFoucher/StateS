/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "equationeditordialog.h"

// Qt classes
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QKeyEvent>
#include <QStyle>

// StateS classes
#include "equation.h"
#include "operand.h"
#include "equationeditorwidget.h"
#include "templateequationpartswidget.h"


EquationEditorDialog::EquationEditorDialog(shared_ptr<const Equation> initialEquation, QWidget* parent) :
	StatesDialog(parent)
{
	//
	// Window properties
	this->setWindowTitle(tr("Equation editor"));

	//
	// Templates

	// Title text
	auto templatesTitle = new QLabel("<i>" + tr("Drag and drop equation components from here") + "…</i>");

	// Widget
	this->templatesWidget = new TemplateEquationPartsWidget();

	// Scroll area
	this->templatesScrollArea = new QScrollArea();
	this->templatesScrollArea->setWidgetResizable(true);
	this->templatesScrollArea->setWidget(this->templatesWidget);

	//
	// Equation

	// Title text
	auto equationTitle = new QLabel("<i>… " + tr("to here.") + "</i>");

	// Widget
	auto rootEquation = this->buildRootEquation(initialEquation);
	this->equationDisplay = new EquationEditorWidget(rootEquation, 0, false);

	// Bottom text
	auto equationInfoText = tr("Members outlined in green can be edited by clicking on them")
	                         + "<br>"
	                         + tr("Right-click on any equation members to edit");
	auto equationInfo = new QLabel(equationInfoText);
	equationInfo->setAlignment(Qt::AlignCenter);

	//
	// Buttons
	auto buttonsLayout = new QHBoxLayout();

	auto buttonOK     = new QPushButton(tr("OK"));
	auto buttonCancel = new QPushButton(tr("Cancel"));

	buttonsLayout->addWidget(buttonOK);
	buttonsLayout->addWidget(buttonCancel);

	connect(buttonOK,     &QPushButton::clicked, this, &EquationEditorDialog::accept);
	connect(buttonCancel, &QPushButton::clicked, this, &EquationEditorDialog::reject);

	//
	// Build complete rendering
	auto mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(templatesTitle, 0, Qt::AlignHCenter);
	mainLayout->addWidget(this->templatesScrollArea);
	mainLayout->addWidget(equationTitle, 0, Qt::AlignHCenter);
	mainLayout->addWidget(this->equationDisplay, 0, Qt::AlignHCenter);
	mainLayout->addWidget(equationInfo, 0, Qt::AlignHCenter);
	mainLayout->addLayout(buttonsLayout);
}

shared_ptr<Equation> EquationEditorDialog::getResultEquation() const
{
	auto rootEquation = this->equationDisplay->getLogicEquation();

	// Root equation is supposed to be an identity
	if (rootEquation->getOperatorType() != OperatorType_t::identity) return nullptr;

	// Make sure the operand is valid: it may have been deleted
	auto rootEquationOperand = rootEquation->getOperand(0);
	if (rootEquationOperand == nullptr) return nullptr;


	if (rootEquationOperand->getSource() == OperandSource_t::equation)
	{
		// Unwrap the equation
		return rootEquationOperand->getEquation();
	}
	else
	{
		return rootEquation;
	}
}

void EquationEditorDialog::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key_Return)
	{
		if (this->equationDisplay->validEdit() == true)
		{
			transmitEvent = false;
		}
	}
	else if (event->key() == Qt::Key_Escape)
	{
		if (this->equationDisplay->cancelEdit() == true)
		{
			transmitEvent = false;
		}
	}

	if (transmitEvent == true)
	{
		QDialog::keyPressEvent(event);
	}
}

void EquationEditorDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		bool editValidated = this->equationDisplay->validEdit();

		if (editValidated == false)
		{
			event->ignore();
		}
	}
	else
	{
		event->ignore();
	}
}

void EquationEditorDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);

	//
	// Scroll area size

	// Scroll area minimal width
	int templatesScrollAreaMinWidth = this->templatesWidget->width();
	templatesScrollAreaMinWidth += this->templatesScrollArea->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
	templatesScrollAreaMinWidth += 2 * this->templatesScrollArea->frameWidth();
	templatesScrollAreaMinWidth += 12; // TODO: what am I missing that I still have to add 12 here for certain screens???
	this->templatesScrollArea->setMinimumWidth(templatesScrollAreaMinWidth);

	// Scroll area maximal height
	int templatesScrollAreaMaxHeight = this->templatesWidget->height();
	templatesScrollAreaMaxHeight += 2 * this->templatesScrollArea->frameWidth();
	this->templatesScrollArea->setMaximumHeight(templatesScrollAreaMaxHeight);

	// Make sure the initial height is at least 25% of the screen
	auto templatesScrollAreaMinHeight = 0.25 * this->screen()->size().height();
	if (templatesScrollAreaMinHeight > templatesScrollAreaMaxHeight)
	{
		templatesScrollAreaMinHeight = templatesScrollAreaMaxHeight;
	}
	this->templatesScrollArea->setMinimumHeight(templatesScrollAreaMinHeight);

	//
	// Window size

	// Try to set the window size so that we can display everything
	int windowHeightWithoutScrollArea = this->frameGeometry().height() - this->templatesScrollArea->height();
	int windowHeightWithFullScrollArea = windowHeightWithoutScrollArea + templatesScrollAreaMaxHeight;
	int screenAvailableHeight = this->screen()->availableGeometry().height();
	if (windowHeightWithFullScrollArea <= 0.9 * screenAvailableHeight)
	{
		this->setMinimumHeight(windowHeightWithFullScrollArea);
	}
	else
	{
		this->setMinimumHeight(0.9 * screenAvailableHeight);
	}
}

shared_ptr<Equation> EquationEditorDialog::buildRootEquation(shared_ptr<const Equation> initialEquation)
{
	// The root equation will *always* be an identity equation,
	// so that equation parts always have a parent equation.
	shared_ptr<Equation> rootEquation;
	if (initialEquation != nullptr)
	{
		if (initialEquation->getOperatorType() == OperatorType_t::identity)
		{
			// Equation is already an identity (either variable or constant)
			rootEquation = initialEquation->clone();
		}
		else
		{
			// This is a standard equation: add identity wrapper
			rootEquation = make_shared<Equation>(OperatorType_t::identity);
			rootEquation->setOperand(0, initialEquation->clone());
		}
	}
	else
	{
		// The equation is currently undefined: define an empty identity
		rootEquation = make_shared<Equation>(OperatorType_t::identity);
	}

	return rootEquation;
}
