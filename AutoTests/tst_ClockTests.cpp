#include <QString>
#include <QtTest>
#include <QDate>
#include <QDateTime>
#include <QTextStream>

#include <Parser.h>
#include <Headline.h>
#include <Clock.h>
#include <Exception.h>
#include <TimeInterval.h>

#include "TestHelpers.h"

using namespace OrgMode;

class ClockTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTimeIntervals();
    void testAccumulateForDay();
    void testAccumulateForWeek();
};



void ClockTests::testTimeIntervals()
{
    const QDate today(2015, 4, 24);
    const QDateTime six(today, QTime(6,0));
    const QDateTime seven(today, QTime(7,0));
    const QDateTime eight(today, QTime(8,0));
    const QDateTime nine(today, QTime(9,0));
    //simple case: two closed intervals that overlap:
    const TimeInterval sixToEight(six, eight);
    const TimeInterval sevenToNine(seven, nine);
    const TimeInterval sevenToEight(seven, eight);
    QCOMPARE(sixToEight.intersection(sevenToNine), sevenToEight);
    QCOMPARE(sevenToNine.intersection(sixToEight), sevenToEight);
    //more complicated: open intervals
    const TimeInterval toEight(QDateTime(), eight);
    const TimeInterval fromSeven(seven);
    QCOMPARE(toEight.intersection(fromSeven), sevenToEight);
    QCOMPARE(fromSeven.intersection(toEight), sevenToEight);
    //non-intersecting intervals
    const TimeInterval sixToSeven(six, seven);
    const TimeInterval eightToNine(eight, nine);
    QCOMPARE(sixToSeven.intersection(eightToNine), TimeInterval());
    QCOMPARE(eightToNine.intersection(sixToSeven), TimeInterval());
    //touching, but non-intersecting intervals:
    QCOMPARE(sixToSeven.intersection(sevenToEight), TimeInterval());
    QCOMPARE(sevenToEight.intersection(sixToSeven), TimeInterval());
    //invalid intervals on one side
    QCOMPARE(sixToEight.intersection(TimeInterval()), TimeInterval());
    QCOMPARE(TimeInterval().intersection(sixToEight), TimeInterval());
    QCOMPARE(TimeInterval().intersection(TimeInterval()), TimeInterval());
}

void ClockTests::testAccumulateForDay()
{
    const QString filename = FL1("://TestData/Parser/WeirdClockEntries.org");
    QFile input(filename);
    QVERIFY(input.open(QIODevice::ReadOnly));
    QTextStream stream(&input);
    Parser parser;
    try {
        auto const element = parser.parse(&stream, filename);
        auto const headline_1_1 = findElement<Headline>(element, FL1("headline_1_1"));
        QVERIFY(headline_1_1);
        Clock clock(headline_1_1);
        QCOMPARE(clock.duration(), 60 * 150); //150 minutes
        const QDate mar26(2015, 3, 26);
        const QDate mar27(mar26.addDays(1));
        //QCOMPARE(clock.duration(TimeInterval(mar26, mar27)), 60 * 60); //60 minutes on March 26
    } catch(Exception& ex) {
        QFAIL(qPrintable(ex.message()));
    }
}

void ClockTests::testAccumulateForWeek()
{
}

QTEST_APPLESS_MAIN(ClockTests)

#include "tst_ClockTests.moc"
