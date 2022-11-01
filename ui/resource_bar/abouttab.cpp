/*
 * Copyright © 2014-2020 Clément Foucher
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

// StateS classes
#include "states.h"
#include "svgimagegenerator.h"


AboutTab::AboutTab(QWidget* parent) :
    QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignHCenter);

	layout->addStretch();

	////
	// Header

	this->icon = new QPushButton(this);
	this->icon->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));
	this->icon->setIconSize(QSize(100,100));
	this->icon->setCheckable(true);
	this->icon->setStyleSheet("border:0px");
	connect(this->icon, &QPushButton::clicked, this, &AboutTab::iconClicked);
	layout->addWidget(this->icon);

	QLabel* title = new QLabel("<b>StateS</b>", this);
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	QLabel* versionLabel = new QLabel(tr("Version") + " " + StateS::getVersion(), this);
	versionLabel->setAlignment(Qt::AlignCenter);
	versionLabel->setWordWrap(true);
	layout->addWidget(versionLabel);

	layout->addStretch();

	////
	// Copyright and technical info

	QLabel* techTitle = new QLabel("<b>" + tr("Copyright and technical information") + "</b>", this);
	techTitle->setAlignment(Qt::AlignCenter);
	layout->addWidget(techTitle);

	QLabel* copyrightLabel = new QLabel("© " + StateS::getCopyrightYears() +" Clément Foucher", this);
	copyrightLabel->setAlignment(Qt::AlignCenter);
	copyrightLabel->setWordWrap(true);
	layout->addWidget(copyrightLabel);


	QLabel* licenseLabel = new QLabel(tr("Distributed under the terms of the") + "<br /><a href=\"https://www.gnu.org/licenses/gpl-2.0.html\">" + tr("GNU General Public Licence") + " " + tr("version") + " 2</a>.", this);
	licenseLabel->setAlignment(Qt::AlignCenter);
	licenseLabel->setWordWrap(true);
	licenseLabel->setTextFormat(Qt::RichText);
	licenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	licenseLabel->setOpenExternalLinks(true);
	layout->addWidget(licenseLabel);

	QLabel* artLicenseLabel = new QLabel(tr("See") + " <a href=\"https://raw.githubusercontent.com/ClementFoucher/StateS/main/art/AUTHORS.txt\">" + tr("credits for art used in StateS") + "</a>", this);
	artLicenseLabel->setAlignment(Qt::AlignCenter);
	artLicenseLabel->setWordWrap(true);
	artLicenseLabel->setTextFormat(Qt::RichText);
	artLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	artLicenseLabel->setOpenExternalLinks(true);
	layout->addWidget(artLicenseLabel);

	QLabel* thirdPartyLicenseLabel = new QLabel(tr("This software includes third-party work licensed under the") + " <a href=\"https://www.apache.org/licenses/\">" + tr("Apache License") + " " + tr("version") + " 2.0</a>", this);
	thirdPartyLicenseLabel->setAlignment(Qt::AlignCenter);
	thirdPartyLicenseLabel->setWordWrap(true);
	thirdPartyLicenseLabel->setTextFormat(Qt::RichText);
	thirdPartyLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	thirdPartyLicenseLabel->setOpenExternalLinks(true);
	layout->addWidget(thirdPartyLicenseLabel);

	QLabel* qtVersionLabel = new QLabel(tr("This application was compiled using") + " Qt " + QT_VERSION_STR + ".\n" + tr("Currently running on") + " Qt " + qVersion() + ".", this);
	qtVersionLabel->setAlignment(Qt::AlignCenter);
	qtVersionLabel->setWordWrap(true);
	layout->addWidget(qtVersionLabel);

	layout->addStretch();

	////
	// Contact info

	QLabel* contactTitle = new QLabel("<b>" + tr("Contact and links") + "</b>", this);
	contactTitle->setAlignment(Qt::AlignCenter);
	layout->addWidget(contactTitle);

	QLabel* emailLabel = new QLabel(tr("Contact:") + " <a href=\"mailto:states-dev@outlook.fr\">StateS-dev@outlook.fr</a>" , this);
	emailLabel->setAlignment(Qt::AlignCenter);
	emailLabel->setWordWrap(true);
	emailLabel->setTextFormat(Qt::RichText);
	emailLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	emailLabel->setOpenExternalLinks(true);
	layout->addWidget(emailLabel);

	QLabel* websiteLabel = new QLabel(tr("StateS web site:") + "<br /><a href=\"https://github.com/ClementFoucher/StateS\">https://github.com/ClementFoucher/StateS</a>", this);
	websiteLabel->setAlignment(Qt::AlignCenter);
	websiteLabel->setWordWrap(true);
	websiteLabel->setTextFormat(Qt::RichText);
	websiteLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	websiteLabel->setOpenExternalLinks(true);
	layout->addWidget(websiteLabel);

	QLabel* wikiLabel= new QLabel("<a href=\"https://sourceforge.net/p/states/wiki/Home/\">" + tr("Need help?") + "</a>", this);
	wikiLabel->setAlignment(Qt::AlignCenter);
	wikiLabel->setWordWrap(true);
	wikiLabel->setTextFormat(Qt::RichText);
	wikiLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	wikiLabel->setOpenExternalLinks(true);
	layout->addWidget(wikiLabel);

	QLabel* supportLabel = new QLabel("<a href=\"https://github.com/ClementFoucher/StateS/issues\">" + tr("Found a bug?") + "</a><br />"
	                                  + tr("(Please consult") + " "
	                                  + "<a href=\"https://raw.githubusercontent.com/ClementFoucher/StateS/main/text/known_bugs.txt\">"+ tr("known bugs list")  + "</a><br />"
	                                  + " " + tr("before submitting a ticket)"),
	                                  this);
	supportLabel->setAlignment(Qt::AlignCenter);
	supportLabel->setWordWrap(true);
	supportLabel->setTextFormat(Qt::RichText);
	supportLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	supportLabel->setOpenExternalLinks(true);
	layout->addWidget(supportLabel);

	QLabel* suggestionLabel = new QLabel("<a href=\"https://github.com/ClementFoucher/StateS/issues\">" + tr("Suggestion? Feature request?") + "</a>", this);
	suggestionLabel->setAlignment(Qt::AlignCenter);
	suggestionLabel->setWordWrap(true);
	suggestionLabel->setTextFormat(Qt::RichText);
	suggestionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	suggestionLabel->setOpenExternalLinks(true);
	layout->addWidget(suggestionLabel);

	layout->addStretch();
}

void AboutTab::iconClicked()
{
	QPixmap pixmap = SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"));
	QIcon newIcon;

	if (this->icon->isChecked())
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
