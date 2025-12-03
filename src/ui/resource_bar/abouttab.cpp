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
#include "abouttab.h"

// Qt classes
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>

// StateS classes
#include "states.h"
#include "pixmapgenerator.h"


AboutTab::AboutTab(QWidget* parent) :
	QWidget(parent)
{
	//
	// Header

	this->icon = new QPushButton();
	this->icon->setIcon(QIcon(PixmapGenerator::getStatesWindowIcon()));
	this->icon->setIconSize(QSize(100,100));
	this->icon->setCheckable(true);
	this->icon->setStyleSheet("border:0px");
	connect(this->icon, &QPushButton::clicked, this, &AboutTab::iconClicked);

	QLabel* title = new QLabel("<b>StateS</b>");
	title->setAlignment(Qt::AlignCenter);

	QLabel* versionLabel = new QLabel(tr("Version") + " " + StateS::getVersion());
	versionLabel->setAlignment(Qt::AlignCenter);
	versionLabel->setWordWrap(true);

	// Package in a layout
	auto headerLayout = new QVBoxLayout();
	headerLayout->addWidget(this->icon);
	headerLayout->addWidget(title);
	headerLayout->addWidget(versionLabel);

	//
	// Copyright and technical info

	QLabel* copyrightLabel = new QLabel("© " + StateS::getCopyrightYears() + " " + "Clément Foucher");
	copyrightLabel->setAlignment(Qt::AlignCenter);
	copyrightLabel->setWordWrap(true);

	QLabel* licenseLabel = new QLabel(tr("Distributed under the terms of the")
	                                  + " "
	                                  + "<a href=\"https://www.gnu.org/licenses/gpl-2.0.html\">" + tr("GNU General Public Licence") + " " + tr("version") + " " + "2" + "</a>"
	                                 );
	licenseLabel->setAlignment(Qt::AlignCenter);
	licenseLabel->setWordWrap(true);
	licenseLabel->setTextFormat(Qt::RichText);
	licenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	licenseLabel->setOpenExternalLinks(true);

	QLabel* artLicenseLabel = new QLabel(tr("See")
	                                     + " "
	                                     + "<a href=\"https://github.com/ClementFoucher/StateS/blob/main/art/AUTHORS.md\">" + tr("credits for art used in StateS") + "</a>"
	                                    );
	artLicenseLabel->setAlignment(Qt::AlignCenter);
	artLicenseLabel->setWordWrap(true);
	artLicenseLabel->setTextFormat(Qt::RichText);
	artLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	artLicenseLabel->setOpenExternalLinks(true);

	QLabel* thirdPartyLicenseLabel = new QLabel(tr("This software makes use of")
	                                            + " "
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

	QString versionText;
	if (std::strcmp(QT_VERSION_STR, qVersion()) == 0)
	{
		versionText = tr("This application was compiled with and runs on") + " Qt " + QT_VERSION_STR;
	}
	else
	{
		versionText = tr("This application was compiled using") + " Qt " + QT_VERSION_STR + "\n" + tr("Currently running on") + " Qt " + qVersion();
	}
	QLabel* qtVersionLabel = new QLabel(versionText);
	qtVersionLabel->setAlignment(Qt::AlignCenter);
	qtVersionLabel->setWordWrap(true);

	// Package in a group
	auto techInfoGroup = new QGroupBox(tr("Copyright and technical information"));
	auto techInfoLayout = new QVBoxLayout(techInfoGroup);
	techInfoLayout->addWidget(copyrightLabel);
	techInfoLayout->addWidget(licenseLabel);
	techInfoLayout->addWidget(artLicenseLabel);
	techInfoLayout->addWidget(thirdPartyLicenseLabel);
	techInfoLayout->addWidget(qtVersionLabel);

	//
	// Contact info

	QLabel* emailLabel = new QLabel(tr("Contact:")
	                                + " "
	                                + "<a href=\"mailto:states-dev@outlook.fr\">StateS-dev@outlook.fr</a>"
	                               );
	emailLabel->setAlignment(Qt::AlignCenter);
	emailLabel->setWordWrap(true);
	emailLabel->setTextFormat(Qt::RichText);
	emailLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	emailLabel->setOpenExternalLinks(true);

	QLabel* websiteLabel = new QLabel(tr("StateS sources:")
	                                  + " "
	                                  + "<a href=\"https://github.com/ClementFoucher/StateS\">https://github.com/ClementFoucher/StateS</a>"
	                                 );
	websiteLabel->setAlignment(Qt::AlignCenter);
	websiteLabel->setWordWrap(true);
	websiteLabel->setTextFormat(Qt::RichText);
	websiteLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	websiteLabel->setOpenExternalLinks(true);

	/*
	QLabel* wikiLabel= new QLabel("<a href=\"https://sourceforge.net/p/states/wiki/Home/\">" + tr("Need help?") + "</a>");
	wikiLabel->setAlignment(Qt::AlignCenter);
	wikiLabel->setWordWrap(true);
	wikiLabel->setTextFormat(Qt::RichText);
	wikiLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	wikiLabel->setOpenExternalLinks(true);
	*/

	QLabel* supportLabel = new QLabel("<a href=\"https://github.com/ClementFoucher/StateS/issues\">" + tr("Found a bug?") + "</a> "
	                                  + tr("(Please consult the")
	                                  + " "
	                                  + "<a href=\"https://raw.githubusercontent.com/ClementFoucher/StateS/main/text/known_bugs.txt\">"+ tr("known bugs list") + "</a>"
	                                  + " "
	                                  + tr("before filling an issue)")
	                                 );
	supportLabel->setAlignment(Qt::AlignCenter);
	supportLabel->setWordWrap(true);
	supportLabel->setTextFormat(Qt::RichText);
	supportLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	supportLabel->setOpenExternalLinks(true);

	QLabel* suggestionLabel = new QLabel("<a href=\"https://github.com/ClementFoucher/StateS/issues\">" + tr("Suggestion? Feature request?") + "</a>");
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
	//contactLayout->addWidget(wikiLabel);
	contactLayout->addWidget(supportLabel);
	contactLayout->addWidget(suggestionLabel);

	//
	// Build complete rendering

	auto* mainLayout = new QVBoxLayout();

	mainLayout->addStretch(5);
	mainLayout->addLayout(headerLayout);
	mainLayout->addStretch(1);
	mainLayout->addWidget(techInfoGroup);
	mainLayout->addStretch(1);
	mainLayout->addWidget(contactGroup);
	mainLayout->addStretch(5);

	auto* rootLayout = new QHBoxLayout(this);
	rootLayout->addStretch(1);
	rootLayout->addLayout(mainLayout, 10);
	rootLayout->addStretch(1);
}

void AboutTab::iconClicked()
{
	QPixmap pixmap = PixmapGenerator::getStatesWindowIcon();
	QIcon newIcon;

	if (this->icon->isChecked() == true)
	{
		QTransform rotation;
		rotation.rotate(90);
		newIcon = QIcon(pixmap.transformed(rotation));
	}
	else
	{
		newIcon = QIcon(pixmap);
	}

	this->icon->setIcon(newIcon);
}
