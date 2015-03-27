#ifndef CLOCKLINE_H
#define CLOCKLINE_H

#include <QCoreApplication>
#include <QDateTime>

#include <OrgElement.h>
#include "orgmodeparser_export.h"

namespace OrgMode {

class ORGMODEPARSER_EXPORT ClockLine : public OrgElement
{
    Q_DECLARE_TR_FUNCTIONS(ClockLine)
public:
    typedef QSharedPointer<ClockLine> Pointer;

    explicit ClockLine(const QString& line, OrgElement* parent = 0);
    explicit ClockLine(OrgElement* parent = 0);

    void setStartTime(const QDateTime& start);
    QDateTime startTime() const;

    void setEndTime(const QDateTime& end);
    QDateTime endTime() const;

    int duration() const;

protected:
    bool isElementValid() const override;
    QString mnemonic() const override;
    QString description() const override;

private:
    class Private;
    Private* d;
};

}

#endif // CLOCKLINE_H
