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
#include "sliderwithtitleandvalue.h"

// Qt classes
#include <QBoxLayout>
#include <QLabel>
#include <QSlider>


SliderWithTitleAndValue::SliderWithTitleAndValue(int minValue, int maxValue, int initialValue, const QString& title, const QString& valuePrefix, const QString& valueSuffix, QWidget* parent) :
	QWidget(parent)
{
	this->valuePrefix = valuePrefix;
	this->valueSuffix = valueSuffix;

	//
	// Build widgets

	auto titleLabel = new QLabel(title);

	auto slider = new QSlider(Qt::Horizontal);
	slider->setMinimum(minValue);
	slider->setMaximum(maxValue);
	slider->setValue(initialValue);

	this->valueLabel = new QLabel(this->valuePrefix + QString::number(initialValue) + this->valueSuffix);

	//
	// Build layouts

	// Horizontal layout for slider and value label
	auto sliderAndValueLabelLayout = new QHBoxLayout();
	sliderAndValueLabelLayout->setContentsMargins(0, 0, 0, 0);
	sliderAndValueLabelLayout->addWidget(slider);
	sliderAndValueLabelLayout->addWidget(this->valueLabel);

	// Main layout
	auto titleAndSliderLayout = new QVBoxLayout(this);
	titleAndSliderLayout->setContentsMargins(0, 0, 0, 0);
	titleAndSliderLayout->addWidget(titleLabel);
	titleAndSliderLayout->addLayout(sliderAndValueLabelLayout);

	//
	// Connect signals

	connect(slider, &QSlider::valueChanged, this, &SliderWithTitleAndValue::sliderValueChangedEventHandler);
}

void SliderWithTitleAndValue::sliderValueChangedEventHandler(int newValue)
{
	this->valueLabel->setText(this->valuePrefix + QString::number(newValue) + this->valueSuffix);

	emit this->valueChangedEvent(newValue);
}
