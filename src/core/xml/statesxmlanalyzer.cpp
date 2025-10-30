/*
 * Copyright © 2017-2025 Clément Foucher
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


StateSXmlAnalyzer::StateSXmlAnalyzer(shared_ptr<QFile> file)
{
	if (file->isOpen() == false)
	{
		file->open(QIODevice::ReadOnly);
	}
	else
	{
		file->reset();
	}

	this->xmlReader = make_shared<QXmlStreamReader>(file.get());
	this->parse();
}

StateSXmlAnalyzer::StateSXmlAnalyzer(const QString& xmlSource)
{
	this->xmlReader = make_shared<QXmlStreamReader>(xmlSource);
	this->parse();
}

MachineType_t StateSXmlAnalyzer::getMachineType() const
{
	return this->type;
}

bool StateSXmlAnalyzer::getHasVersion() const
{
	if ( (this->saveVersionMajor != 0) ||
		 (this->saveVersionMinor != 0) ||
		 (this->saveVersionPatch != 0)
	   )
	{
		return true;
	}

	return false;
}

QString StateSXmlAnalyzer::getStateSVersion() const
{
	return QString::number(this->saveVersionMajor, 16) + "." + QString::number(this->saveVersionMinor, 16) + "." + QString::number(this->saveVersionPatch, 16);
}

StateSXmlAnalyzer::VersionCompatibility_t StateSXmlAnalyzer::getVersionCompatibility() const
{
	bool ok;
	uint current_major = QString(STATES_VERSION_MAJOR).toUInt(&ok, 16);
	uint current_minor = QString(STATES_VERSION_MINOR).toUInt(&ok, 16);
	uint current_patch = QString(STATES_VERSION_PATCH).toUInt(&ok, 16);

	if (this->saveVersionMajor == current_major)
	{
		if (this->saveVersionMinor == current_minor)
		{
			if (this->saveVersionPatch == current_patch)
			{
				return VersionCompatibility_t::same_version;
			}
			else if (this->saveVersionPatch > current_patch)
			{
				return VersionCompatibility_t::patch_newer;
			}
			else
			{
				return VersionCompatibility_t::patch_older;
			}
		}
		else if (this->saveVersionMinor > current_minor)
		{
			return VersionCompatibility_t::minor_newer;
		}
		else
		{
			return VersionCompatibility_t::minor_older;
		}
	}
	else if (this->saveVersionMajor > current_major)
	{
		return VersionCompatibility_t::major_newer;
	}
	else
	{
		return VersionCompatibility_t::major_older;
	}
}

void StateSXmlAnalyzer::parse()
{
	while (this->xmlReader->atEnd() == false)
	{
		this->xmlReader->readNext();

		if (this->xmlReader->isStartElement())
		{
			if (this->xmlReader->name() == QString("FSM"))
			{
				this->type = MachineType_t::fsm;

				auto extractedVersion = this->xmlReader->attributes().value("StateS_version").toString();

				if (extractedVersion.isNull() == false)
				{
					auto versionParts = extractedVersion.split(".");
					if (versionParts.count() >= 3)
					{
						bool ok;
						this->saveVersionMajor = QString(versionParts[0]).toUInt(&ok, 16);
						this->saveVersionMinor = QString(versionParts[1]).toUInt(&ok, 16);
						this->saveVersionPatch = QString(versionParts[2]).toUInt(&ok, 16);
					}
				}
				else
				{
					// Default version number: before 0.4,
					// version was not written in save.
					this->saveVersionMajor = 0;
					this->saveVersionMinor = 3;
					this->saveVersionPatch = 0;
				}

				break;
			}
		}
	}
}
