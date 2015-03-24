#include <QtDebug>
#include <QMap>

#include <Properties.h>
#include <FileAttributeLine.h>
#include <Exception.h>
#include <OrgFile.h>
#include <Drawer.h>
#include <DrawerEntry.h>
#include <PropertyDrawer.h>
#include <PropertyDrawerEntry.h>
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
    QList<PropertyDrawerEntry::Pointer> propertyDrawerEntries;
    OrgElement* element(d->element_.data());
    while(element) {
        auto const drawers = findElements<PropertyDrawer>(element, 1);
        for( auto const drawer : drawers) {
            auto isPropertyEntryForKey = [key](const PropertyDrawerEntry::Pointer& element) {
                return element->key() == key;
            };
            auto const elementPropertyDrawers = findElements<PropertyDrawerEntry>(drawer, 1, isPropertyEntryForKey);
            propertyDrawerEntries << elementPropertyDrawers;

        }
        element = element->parent();
    }
    //Create single list of all definitions that affect the property:
    Vector definitions;
    for( const Property prop : attr ) {
        Property property  = parseAttributeAsProperty(prop);
        if (property.key() == key) {
            definitions << property;
        }
    }
    for( auto const entry : propertyDrawerEntries ) {
        definitions << entry->property();
    }
    if (definitions.isEmpty()) {
        throw RuntimeException(tr("Undefined property %1").arg(key));
    }
    //Calculate property value:
    const QString result = propertyValue(key, definitions);
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
        entries.append(Property(entry->key(), entry->value()));
    } );
    return entries;
}

QString Properties::propertyValue(const QString &key, const Properties::Vector &definitions)
{
    Property value;
    for( auto const property : definitions ) {
        if (property.key() == key) {
            value.apply(property);
        }
    }
    return value.value();
}

Property Properties::parseAttributeAsProperty(const Property& attribute)
{
    const QRegularExpression re(QString::fromLatin1("^(\\w+)(\\+{0,1})\\s+(\\w.*)$"));
    auto const match = re.match(attribute.value());
    if (match.hasMatch()) {
        Property result;
        result.setKey(match.captured(1));
        if (match.captured(2) == QLatin1String("+")) {
            result.setOperation(Property::Property_Add);
        }
        result.setValue(match.captured(3));
        return result;
    }
    return Property();
}

}
