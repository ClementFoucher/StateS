/*
 * Copyright © 2014-2026 Clément Foucher
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
#include "abouttab.h"

// Qt
#include <QGuiApplication>
#include <QStyleFactory>
#include <QStyleHints>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

// StateS
#include "states.h"
#include "pixmapgenerator.h"


AboutTab::AboutTab(QWidget* parent) :
	QWidget(parent)
{
	//
	// Header

	this->logo = new QPushButton();
	this->logo->setIcon(QIcon(PixmapGenerator::getStatesLogo(100)));
	this->logo->setIconSize(QSize(100,100));
	this->logo->setCheckable(true);
	this->logo->setStyleSheet("border:0px");

	auto title = new QLabel("<b>StateS</b>");
	title->setAlignment(Qt::AlignCenter);

	auto versionLabel = new QLabel(tr("Version") + " " + StateS::getVersion());
	versionLabel->setAlignment(Qt::AlignCenter);
	versionLabel->setWordWrap(true);

	// Package in a layout
	auto headerLayout = new QVBoxLayout();
	headerLayout->addWidget(this->logo);
	headerLayout->addWidget(title);
	headerLayout->addWidget(versionLabel);

	//
	// Copyright info

	auto copyrightLabel = new QLabel("© " + StateS::getCopyrightYears() + " " + "Clément Foucher");
	copyrightLabel->setAlignment(Qt::AlignCenter);
	copyrightLabel->setWordWrap(true);

	auto licenseLabel = new QLabel(tr("Distributed under the terms of the")
	                               + " "
	                               + "<a href=\"https://www.gnu.org/licenses/gpl-2.0.html\">" + tr("GNU General Public Licence") + " " + tr("version") + " " + "2" + "</a>"
	                              );
	licenseLabel->setAlignment(Qt::AlignCenter);
	licenseLabel->setWordWrap(true);
	licenseLabel->setTextFormat(Qt::RichText);
	licenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	licenseLabel->setOpenExternalLinks(true);

	auto artLicenseLabel = new QLabel(tr("See")
	                                  + " "
	                                  + "<a href=\"https://github.com/ClementFoucher/StateS/blob/main/art/AUTHORS.md\">" + tr("credits for art used in StateS") + "</a>"
	                                 );
	artLicenseLabel->setAlignment(Qt::AlignCenter);
	artLicenseLabel->setWordWrap(true);
	artLicenseLabel->setTextFormat(Qt::RichText);
	artLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	artLicenseLabel->setOpenExternalLinks(true);

	auto thirdPartyLicenseLabel = new QLabel(tr("This software makes use of:")
	                                         + "<br>"
	                                         + "<a href=\"https://code.qt.io/cgit/qt/qt5.git\">" + "Qt 6" + "</a>" + " "
	                                         + tr("which is licensed under the")
	                                         + " "
	                                         + "<a href=\"https://www.gnu.org/licenses/lgpl-3.0.html\">" + tr("LGPLv3 license") + "</a>"
	                                         + "<br>"
	                                         + "<a href=\"https://github.com/cubicdaiya/dtl\">" + tr("the Diff Template Library") + "</a>"
	                                         + " "
	                                         + tr("which is licensed under the")
	                                         + " "
	                                         + "<a href=\"https://opensource.org/license/BSD-3-Clause\">" + tr("BSD-3-Clause license") + "</a>"
	                                        );
	thirdPartyLicenseLabel->setAlignment(Qt::AlignCenter);
	thirdPartyLicenseLabel->setWordWrap(true);
	thirdPartyLicenseLabel->setTextFormat(Qt::RichText);
	thirdPartyLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	thirdPartyLicenseLabel->setOpenExternalLinks(true);

	// Package in a group
	auto copyrightGroup = new QGroupBox(tr("Copyright information"));
	auto copyrightLayout = new QVBoxLayout(copyrightGroup);
	copyrightLayout->addWidget(copyrightLabel);
	copyrightLayout->addWidget(licenseLabel);
	copyrightLayout->addWidget(artLicenseLabel);
	copyrightLayout->addWidget(thirdPartyLicenseLabel);

	//
	// Contact info

	auto emailLabel = new QLabel(tr("Contact:")
	                             + " "
	                             + "<a href=\"mailto:states-dev@outlook.fr\">StateS-dev@outlook.fr</a>"
	                            );
	emailLabel->setAlignment(Qt::AlignCenter);
	emailLabel->setWordWrap(true);
	emailLabel->setTextFormat(Qt::RichText);
	emailLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	emailLabel->setOpenExternalLinks(true);

	auto websiteLabel = new QLabel(tr("StateS sources can be downloaded")
	                               + " "
	                               + "<a href=\"https://github.com/ClementFoucher/StateS\">" + tr("here") + "</a>"
	                              );
	websiteLabel->setAlignment(Qt::AlignCenter);
	websiteLabel->setWordWrap(true);
	websiteLabel->setTextFormat(Qt::RichText);
	websiteLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	websiteLabel->setOpenExternalLinks(true);

	auto supportLabel = new QLabel(tr("You found a bug?")
	                               + " "
	                               + tr("Please report it on the")
	                               + " "
	                               + "<a href=\"https://github.com/ClementFoucher/StateS/issues\">" +  tr("bug tracker") + "</a>"
	                              );
	supportLabel->setAlignment(Qt::AlignCenter);
	supportLabel->setWordWrap(true);
	supportLabel->setTextFormat(Qt::RichText);
	supportLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	supportLabel->setOpenExternalLinks(true);

	auto suggestionLabel = new QLabel(tr("Suggestion? Feature request?")
	                                  + " "
	                                  + tr("Open a ticket on the")
	                                  + " "
	                                  + "<a href=\"https://github.com/ClementFoucher/StateS/issues\">" +  tr("bug tracker") + "</a>"
	                                  );
	suggestionLabel->setAlignment(Qt::AlignCenter);
	suggestionLabel->setWordWrap(true);
	suggestionLabel->setTextFormat(Qt::RichText);
	suggestionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	suggestionLabel->setOpenExternalLinks(true);

	// Package in a group
	auto contactGroup = new QGroupBox(tr("Contact and links"));
	auto contactLayout = new QVBoxLayout(contactGroup);
	contactLayout->addWidget(emailLabel);
	contactLayout->addWidget(websiteLabel);
	contactLayout->addWidget(supportLabel);
	contactLayout->addWidget(suggestionLabel);

	//
	// Build complete rendering

	// Package text info in a scroll area
	auto textWidget = new QWidget();
	this->textLayout = new QVBoxLayout(textWidget);

	this->textLayout->addStretch();
	this->textLayout->addWidget(copyrightGroup);
	this->textLayout->addStretch();
	this->textLayout->addWidget(contactGroup);
	this->textLayout->addStretch();

	auto scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	scrollArea->setWidget(textWidget);

	// Build main layout
	auto mainLayout = new QVBoxLayout(this);
	mainLayout->addStretch();
	mainLayout->addLayout(headerLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(scrollArea);
	mainLayout->addStretch();

	//
	// Connect signals

	connect(this->logo, &QPushButton::clicked, this, &AboutTab::iconClicked);
}

void AboutTab::iconClicked()
{
	if (this->logo->isChecked() == true)
	{
		if (this->techInfoGroup != nullptr)
		{
			this->techInfoGroup->setVisible(true);
		}
		else
		{
			this->buildTechInfo();
			this->textLayout->addWidget(this->techInfoGroup);
		}
		this->textLayout->addStretch();

		this->logo->setIcon(QIcon(PixmapGenerator::getStatesLogo(100, 90)));
	}
	else
	{
		this->techInfoGroup->setVisible(false);

		auto lastStretch = this->textLayout->takeAt(this->textLayout->count()-1);
		delete lastStretch;

		this->logo->setIcon(QIcon(PixmapGenerator::getStatesLogo(100)));
	}
}

void AboutTab::buildTechInfo()
{
	if (this->techInfoGroup != nullptr) return;


	QString versionText;
	if (std::strcmp(QT_VERSION_STR, qVersion()) == 0)
	{
		versionText = tr("This application was compiled with and runs on") + " Qt " + QT_VERSION_STR;
	}
	else
	{
		versionText = tr("This application was compiled using") + " Qt " + QT_VERSION_STR + "\n" + tr("Currently running on") + " Qt " + qVersion();
	}

	auto qtVersionLabel = new QLabel(versionText);
	qtVersionLabel->setAlignment(Qt::AlignCenter);
	qtVersionLabel->setWordWrap(true);

	QString platformText = tr("Current platform:") + " " + QGuiApplication::platformName();
	auto platformInfo = new QLabel(platformText);
	platformInfo->setAlignment(Qt::AlignCenter);
	platformInfo->setWordWrap(true);

	QString colorMode = tr("OS color scheme:") + " ";

	auto styleHint = QGuiApplication::styleHints();
	auto colorScheme = styleHint->colorScheme();
	if (colorScheme == Qt::ColorScheme::Dark)
	{
		colorMode += tr("dark");
	}
	else
	{
		colorMode += tr("light");
	}
	auto colorInfo = new QLabel(colorMode);
	colorInfo->setAlignment(Qt::AlignCenter);
	colorInfo->setWordWrap(true);

	QString stylesText = tr("Available Qt themes:") + " ";
	const auto styles = QStyleFactory::keys();
	for (auto& key : styles)
	{
		stylesText += key.toStdString();
		if (key != styles.last())
		{
			stylesText += ", ";
		}
	}
	auto stylesInfo = new QLabel(stylesText);
	stylesInfo->setAlignment(Qt::AlignCenter);
	stylesInfo->setWordWrap(true);

	// Package in a group
	this->techInfoGroup = new QGroupBox(tr("Technical information"));
	auto techInfoLayout = new QVBoxLayout(this->techInfoGroup);
	techInfoLayout->addWidget(qtVersionLabel);
	techInfoLayout->addWidget(platformInfo);
	techInfoLayout->addWidget(colorInfo);
	techInfoLayout->addWidget(stylesInfo);
}
