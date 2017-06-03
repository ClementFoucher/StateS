/*
 * Copyright © 2017 Clément Foucher
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
#include "statesxmlanalyzer.h"

// Qt classes
#include <QXmlStreamReader>
#include <QFile>


StateSXmlAnalyzer::StateSXmlAnalyzer()
{
    this->type         = machineType::None;
    this->version      = "Unknown version";
    this->xmlIsCorrect = true;
}

StateSXmlAnalyzer::StateSXmlAnalyzer(shared_ptr<QFile> file) :
    StateSXmlAnalyzer()
{
    if (file->isOpen() == false)
    {
        file->open(QIODevice::ReadOnly);
    }
    else
    {
        file->reset();
    }

    this->xmlReader = shared_ptr<QXmlStreamReader>(new QXmlStreamReader(file.get()));
    this->parse();
}

StateSXmlAnalyzer::StateSXmlAnalyzer(const QString& xmlSource) :
    StateSXmlAnalyzer()
{
    this->xmlReader = shared_ptr<QXmlStreamReader>(new QXmlStreamReader(xmlSource));
    this->parse();
}

StateSXmlAnalyzer::machineType StateSXmlAnalyzer::getMachineType()
{
    return this->type;
}

QString StateSXmlAnalyzer::getStateSVersion()
{
    return this->version;
}

bool StateSXmlAnalyzer::getXmlIsCorrect()
{
    return this->xmlIsCorrect;
}

void StateSXmlAnalyzer::parse()
{
    while (this->xmlReader->atEnd() == false)
    {
        this->xmlReader->readNext();

        if (this->xmlReader->isStartElement())
        {
            if (this->xmlReader->name() == "FSM")
            {
                this->type = machineType::Fsm;
                QString extractedVersion = this->xmlReader->attributes().value("StateS_version").toString();

                if (! extractedVersion.isNull())
                {
                    this->version = extractedVersion;
                }
            }
        }
    }
}
