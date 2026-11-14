/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
#include "cellwitheditandclearbuttonseditor.h"

// Qt
#include <QPushButton>
#include <QHBoxLayout>


CellWithEditAndClearButtonsEditor::CellWithEditAndClearButtonsEditor(QWidget* parent) :
	QWidget{parent}
{
	this->buttonClear = new QPushButton("⌫");
	this->buttonClear->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->buttonClear->setStyleSheet("QPushButton {color: red;}");

	this->buttonEdit = new QPushButton("📝");
	this->buttonEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->mainLayout = new QHBoxLayout(this);

	// Set main spacing to 0, but retain initial value
	auto spacing = this->mainLayout->spacing();
	this->mainLayout->setSpacing(0);

	// Only keep the left margin
	auto contentMargins = this->mainLayout->contentsMargins();
	contentMargins.setBottom(0);
	contentMargins.setTop(0);
	contentMargins.setRight(0);
	this->mainLayout->setContentsMargins(contentMargins);

	this->mainLayout->addSpacing(spacing); // Restore minimal spacing between text and buttons
	this->mainLayout->addStretch();        // Add additional strech in case the cell is wider
	this->mainLayout->addWidget(this->buttonClear, 0, Qt::AlignRight);
	this->mainLayout->addWidget(this->buttonEdit,  0, Qt::AlignRight);

	connect(this->buttonClear, &QPushButton::clicked, this, &CellWithEditAndClearButtonsEditor::clearButtonClickedEventHandler);
	connect(this->buttonEdit,  &QPushButton::clicked, this, &CellWithEditAndClearButtonsEditor::editButtonClickedEventHandler);
}

void CellWithEditAndClearButtonsEditor::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	int height = this->height();

	this->buttonClear->setFixedWidth(height);
	this->buttonEdit ->setFixedWidth(height);
}

void CellWithEditAndClearButtonsEditor::setMainWidget(QWidget* mainWidget)
{
	if (this->mainWidget == nullptr)
	{
		this->mainLayout->insertWidget(0, mainWidget, 0, Qt::AlignLeft);
	}
	else
	{
		auto layoutItem = this->mainLayout->replaceWidget(this->mainWidget, mainWidget);
		delete layoutItem;
	}

	this->mainWidget = mainWidget;
}

void CellWithEditAndClearButtonsEditor::updateClearButtonVisibility()
{
	if (this->shouldClearButtonBeVisibile() == true)
	{
		this->buttonClear->show();
	}
	else // (this->shouldClearButtonBeVisibile() == false)
	{
		this->buttonClear->hide();
	}
}
