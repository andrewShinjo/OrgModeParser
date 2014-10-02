#include <QtDebug>
#include <QMap>

#include <Properties.h>
#include <FileAttributeLine.h>
#include <Exception.h>
#include <OrgFile.h>

namespace OrgMode {

class Properties::Private {
public:
    typedef QMap<QString, QString> PropertyMap;

    explicit Private(const OrgElement::Pointer &element)
        : element_(element)
    {}
    PropertyMap properties() const;
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

template <typename T>
QList<QSharedPointer<T>> findChildren(const OrgElement::Pointer& element) {
    QList<QSharedPointer<T>> matches;
    if (!element) return QList<QSharedPointer<T>>();
    auto const p = element.dynamicCast<T>();
    if (p) {
        matches.append(p);
    }
    for(auto const child : element->children()) {
        matches.append(findChildren<T>(child));
    }
    return matches;
}

static void NilDeleter(OrgElement*) {}

Properties::Private::PropertyMap Properties::Private::properties() const
{
    //Find an OrgFile element that is the parent of this one. If there isn't any, no problem, continue.
    //If there is, query its property values and add it to the map as the default for the element
    //local properties:
    auto const file = findNextHigherUp<OrgFile>(element_.data());
    QSharedPointer<OrgFile> pf(file, NilDeleter);
    PropertyMap p;
    if (file) {
        auto const fileAttributes = findChildren<FileAttributeLine>(pf);
        for(auto const attribute : fileAttributes) {
            p.insert(attribute->key(), attribute->value());
        }
    }
    return p;
}

Properties::Properties(const OrgElement::Pointer &element)
    : d(new Private(element))
{
}

QString Properties::property(const QString& key) const
{
    const Private::PropertyMap properties = d->properties();
    Private::PropertyMap::const_iterator it = properties.find(key);
    if (it != properties.end()) {
        return it.value();
    } else {
        return QString();
    }
}

}
