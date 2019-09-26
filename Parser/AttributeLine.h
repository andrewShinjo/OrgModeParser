/** OrgModeParser - a parser for Emacs Org Mode files, written in C++.
    Copyright (C) 2015 Mirko Boehm

    This file is part of OrgModeParser.
    OrgModeParser is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, version 3 of the
    License.

    OrgModeParser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License for more details. You should
    have received a copy of the GNU General Public License along with
    OrgModeParser. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ATTRIBUTELINE_H
#define ATTRIBUTELINE_H

#include <QCoreApplication>

#include <OrgElement.h>
#include <Property.h>
#include "orgmodeparser_export.h"

namespace OrgMode {

class ORGMODEPARSER_EXPORT AttributeLine : public OrgElement
{
    Q_DECLARE_TR_FUNCTIONS(AttributeLine)
public:
    explicit AttributeLine(OrgElement* parent = nullptr);
    explicit AttributeLine(const QString& line, OrgElement* parent = nullptr);
    ~AttributeLine() override;

    void setProperty(const Property& property);
    Property property() const;
    QString key() const;
    QString value() const;

protected:
    bool isElementValid() const override;
    QString description() const override;

private:
    class Private;
    Private* d;
};

}

#endif // ATTRIBUTELINE_H
