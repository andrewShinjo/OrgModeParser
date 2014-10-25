#include <QtDebug>
#include <QMap>

#include <Properties.h>
#include <FileAttributeLine.h>
#include <Exception.h>
#include <OrgFile.h>
#include <Drawer.h>
#include <DrawerEntry.h>
#include <PropertyDrawer.h>
#include <FindElements.h>

namespace OrgMode {

class Properties::Private {
public:
    explicit Private(const OrgElement::Pointer &element)
        : element_(element)
    {}
    OrgElement::Pointer element_;
};

Properties::Properties(const OrgElement::Pointer &element)
    : d(new Private(element))
{
}

Properties::~Properties()
{
    delete d; d = 0;
}

/** @brief Query the specified property for this element. */
QString Properties::property(const QString& key) const
{
    //Collect attribute lines that are propeties:
    Attributes attributes(d->element_);
    const Vector attr(attributes.fileAttributes(QString::fromLatin1("PROPERTY")));
    //Traverse the element chain up to the next OrgFile, collecting property definitions for the specified key on the way:
    QList<DrawerEntry::Pointer> definitions;
    //...
    //Create single list of all definitions that affect the property:
    QList<PropertyDrawer::Pointer> propertyDrawers;
    OrgElement* element(d->element_.data());
    while(element) {
        const QList<PropertyDrawer::Pointer> elementPropertyDrawers = findElements<PropertyDrawer>(element, 1);
        propertyDrawers << elementPropertyDrawers;
        element = element->parent();
    }
    //Calculate property value:
    const QString result = propertyValue(key, attr);
    return result;
}

Properties::Vector Properties::properties() const
{
    throw NotImplementedException();
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

QString Properties::propertyValue(const QString &key, const Properties::Vector &definitions)
{
    throw NotImplementedException();
}

}
