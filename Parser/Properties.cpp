#include <QtDebug>
#include <QMap>

#include <Properties.h>
#include <FileAttributeLine.h>
#include <Exception.h>
#include <OrgFile.h>
#include <Drawer.h>
#include <DrawerEntry.h>
#include <FindElements.h>

namespace OrgMode {

class Properties::Private {
public:
    explicit Private(const OrgElement::Pointer &element)
        : element_(element)
    {}
    OrgElement::Pointer element_;
};

template <typename T>
T* findNextHigherUp(OrgElement* element) {
    if (!element) return 0;
    T* p = dynamic_cast<T*>(element);
    if (p) {
        return p;
    } else if (element->parent()) {
        return findNextHigherUp<T>(element->parent());
    } else {
        return 0;
    }
}

static void NilDeleter(OrgElement*) {}

Properties::Properties(const OrgElement::Pointer &element)
    : d(new Private(element))
{
}

Properties::~Properties()
{
    delete d; d = 0;
}

QString Properties::property(const QString& key) const
{
    const Vector attributes(fileAttributes(QString::fromLatin1("PROPERTY")));
    throw NotImplementedException();
}

Properties::Vector Properties::properties() const
{
    throw NotImplementedException();
}

/** @brief Return the value of a file attribute.
 * A RuntimeException is thrown if no value is defined for this key.
 */
QString Properties::fileAttribute(const QString &key) const
{
    auto const props = fileAttributes(key);
    if (props.isEmpty()) {
        throw RuntimeException(tr("No such attribute: %1").arg(key));
    }
    return props.first().value;
}

Properties::Vector Properties::fileAttributes(const QString &key) const
{
    const Vector all(fileAttributes());
    Vector attributes;
    std::copy_if(all.begin(), all.end(), std::back_inserter(attributes),
                 [key](const Property& prop) { return prop.key == key; } );
    return attributes;
}

Properties::Vector Properties::fileAttributes() const
{
    //Find an OrgFile element that is the parent of this one. If there isn't any, no problem, continue.
    //If there is, query its property values and add it to the map as the default for the element
    //local properties:
    auto const file = findNextHigherUp<OrgFile>(d->element_.data());
    QSharedPointer<OrgFile> pf(file, NilDeleter);
    Vector attributes;
    if (file) {
        auto const fileAttributes = findElements<FileAttributeLine>(pf);
        for(auto const attribute : fileAttributes) {
            attributes.append( { attribute->key(), attribute->value() } );
        }
    }
    return attributes;
}

Properties::Vector Properties::drawer(const QString &name) const
{
    auto const decision = [name](const Drawer::Pointer& drawer) -> bool {
        return name == drawer->name();
    };
    auto const drawers = findElements<Drawer>(d->element_, 1, decision);
    if (drawers.isEmpty()) {
        throw RuntimeException(tr("No drawer named %1 found!").arg(name));
    }
    auto const drawer = drawers.first();
    auto const entryElements = findElements<DrawerEntry>(drawer, 1);
    Vector entries;
    std::for_each(entryElements.begin(), entryElements.end(),
                  [&entries](const DrawerEntry::Pointer& entry) {
        entries.append( { entry->key(), entry->value() } );
    } );
    return entries;
}

/** @brief Return the value of an attribute identified by name.
 *
 * The first occurance of the attribute is returned.
 * FIXME: Is that correct? What does OrgMode use if the same attribute is defined twice in a drawer?
 */
QString Properties::attribute(const Properties::Vector &attributes, const QString& key)
{
    //We assume attributes are identified by their name. The exception is #+PROPERTY:, which can occur
    //repeatedly.
    //A "ATTRIBUTE+: value to append" syntax is not accepted (tested in OrgMode).
    for(auto const att : attributes) {
        if (att.key == key) {
            return att.value;
        }
    }
    return QString();
}

}
