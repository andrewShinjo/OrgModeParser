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
#include "Attributes.h"
#include "Exception.h"
#include "OrgFile.h"
#include "FileAttributeLine.h"
#include "FindElements.h"

namespace OrgMode {

template <typename T>
T* findNextHigherUp(OrgElement* element) {
    if (!element) return nullptr;
    T* p = dynamic_cast<T*>(element);
    if (p) {
        return p;
    } else if (element->parent()) {
        return findNextHigherUp<T>(element->parent());
    } else {
        return nullptr;
    }
}

class Attributes::Private {
public:
    explicit Private(const OrgElement::Pointer &element)
        : element_(element)
    {}
    OrgElement::Pointer element_;
};


Attributes::Attributes(const OrgElement::Pointer &element)
    : d(new Private(element))
{
}

Attributes::Attributes(Attributes && other) = default;
Attributes& Attributes::operator=(Attributes &&other) = default;
Attributes::~Attributes() = default;

/** @brief Return the value of a file attribute.
 * A RuntimeException is thrown if no value is defined for this key.
 */
QString Attributes::fileAttribute(const QString &key) const
{
    auto const props = fileAttributes(key);
    if (props.isEmpty()) {
        throw RuntimeException(tr("No such attribute: %1").arg(key));
    }
    return props.first().value();
}

Attributes::Vector Attributes::fileAttributes(const QString &key) const
{
    const Vector all(fileAttributes());
    Vector attributes;
    std::copy_if(all.begin(), all.end(), std::back_inserter(attributes),
                 [key](const Property& prop) { return prop.key() == key; } );
    return attributes;
}

Attributes::Vector Attributes::fileAttributes() const
{
    //Find an OrgFile element that is the parent of this one. If there isn't any, no problem, continue.
    //If there is, query its property values and add it to the map as the default for the element
    //local properties:
    auto const file = findNextHigherUp<OrgFile>(d->element_.data());
    QSharedPointer<OrgFile> pf(file, NilDeleter);
    Vector attributes;
    if (file) {
        auto const fileAttributes = findElements<FileAttributeLine>(pf);
        for(auto const& attribute : fileAttributes) {
            attributes.append(Property(attribute->key(), attribute->value()));
        }
    }
    return attributes;
}

/** @brief For the element, return the drawer names that are defined. */
const QStringList Attributes::drawerNames() const
{
    static const QStringList defaults = QStringList() << QStringLiteral("PROPERTIES");
    try {
        const QString drawersAttribute = fileAttribute(QString::fromLatin1("DRAWERS"));
        const QStringList names = drawersAttribute.split(QRegularExpression(QLatin1String("\\s+")));
        return defaults + names;
    } catch (const RuntimeException&) {
        // No drawer attribute defined:
        return defaults;
    }
}

/** @brief Return the value of an attribute identified by name.
 *
 * The first occurance of the attribute is returned.
 * FIXME: Is that correct? What does OrgMode use if the same attribute is defined twice in a drawer?
 */
QString Attributes::attribute(const Attributes::Vector &attributes, const QString& key)
{
    //We assume attributes are identified by their name. The exception is #+PROPERTY:, which can occur
    //repeatedly.
    //A "ATTRIBUTE+: value to append" syntax is not accepted (tested in OrgMode).
    for(auto const& att : attributes) {
        if (att.key() == key) {
            return att.value();
        }
    }
    return QString();
}

}
