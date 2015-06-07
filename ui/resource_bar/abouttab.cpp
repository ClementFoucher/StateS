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
#include "abouttab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// StateS classes
#include "states.h"


AboutTab::AboutTab(QWidget* parent) :
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* title = new QLabel("<b>StateS</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel* versionLabel = new QLabel(tr("Version") + " " + StateS::getVersion());
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setWordWrap(true);
    layout->addWidget(versionLabel);

    QLabel* copyrightLabel = new QLabel("© 2014-2015 Clément Foucher");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setWordWrap(true);
    layout->addWidget(copyrightLabel);

    QLabel* licenseLabel = new QLabel(tr("Distributed under the terms of the") + "<br />GNU General Public Licence " + tr("version") + " 2");
    licenseLabel->setAlignment(Qt::AlignCenter);
    licenseLabel->setWordWrap(true);
    layout->addWidget(licenseLabel);

    QLabel* websiteLabel = new QLabel(tr("StateS web site:") + "<br /><a href=\"https://sourceforge.net/projects/states/\">https://sourceforge.net/projects/states/</a>");
    websiteLabel->setAlignment(Qt::AlignCenter);
    websiteLabel->setWordWrap(true);
    websiteLabel->setTextFormat(Qt::RichText);
    websiteLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    websiteLabel->setOpenExternalLinks(true);
    layout->addWidget(websiteLabel);

    QLabel* wikiLabel= new QLabel("<a href=\"https://sourceforge.net/p/states/wiki/Home/\">" + tr("Need help?") + "</a>");
    wikiLabel->setAlignment(Qt::AlignCenter);
    wikiLabel->setWordWrap(true);
    wikiLabel->setTextFormat(Qt::RichText);
    wikiLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    wikiLabel->setOpenExternalLinks(true);
    layout->addWidget(wikiLabel);

    QLabel* supportLabel = new QLabel("<a href=\"https://sourceforge.net/p/states/tickets/\">" + tr("Found a bug?") + "</a><br />"
                                      + tr("(Please consult") + " "
                                      + "<a href=\"https://sourceforge.net/p/states/code/ci/master/tree/known_bugs.txt\">"+ tr("known bugs list")  + "</a><br />"
                                      + " " + tr("before submitting a ticket)"));
    supportLabel->setAlignment(Qt::AlignCenter);
    supportLabel->setWordWrap(true);
    supportLabel->setTextFormat(Qt::RichText);
    supportLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    supportLabel->setOpenExternalLinks(true);
    layout->addWidget(supportLabel);

    QLabel* suggestionLabel= new QLabel("<a href=\"https://sourceforge.net/p/states/discussion/\">" + tr("Suggestion? Feature request?") + "</a>");
    suggestionLabel->setAlignment(Qt::AlignCenter);
    suggestionLabel->setWordWrap(true);
    suggestionLabel->setTextFormat(Qt::RichText);
    suggestionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    suggestionLabel->setOpenExternalLinks(true);
    layout->addWidget(suggestionLabel);
}
