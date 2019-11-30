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
#ifndef HEADLINE_H
#define HEADLINE_H

#include <memory>
#include <set>

#include <QCoreApplication>
#include <QSharedPointer>

#include <OrgElement.h>
#include "orgmodeparser_export.h"

namespace OrgMode {

/** @brief Headline represent a single headline in an OrgMode file. */
class ORGMODEPARSER_EXPORT Headline : public OrgElement
{
    Q_DECLARE_TR_FUNCTIONS(Headline)
public:
    typedef QSharedPointer<Headline> Pointer;
    typedef QList<Pointer> List;
    typedef std::set<QString> Tags;

    explicit Headline(const QString& line, OrgElement* parent = nullptr);
    explicit Headline(OrgElement* parent = nullptr);
    Headline(Headline&&);
    Headline& operator=(Headline&&);
    ~Headline() override;

    QString caption() const;
    void setCaption(const QString& caption);

    Tags tags() const;
    void setTags(const Tags& tags);
    void addTag(const QString& tag);
    void removeTag(const QString& tag);
    bool hasTag(const QString& tag) const;

    bool isMatch(const QRegularExpression& pattern) const override;

protected:
    bool isElementValid() const override;
    QString mnemonic() const override;
    QString description() const override;
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}

#endif // HEADLINE_H
