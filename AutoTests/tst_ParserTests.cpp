#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <Headline.h>
#include <Parser.h>
#include <Writer.h>
#include <OrgFile.h>
#include <Clock.h>
#include <Tags.h>
#include <OrgLine.h>
#include <FileAttributeLine.h>
#include <Properties.h>
#include <Exception.h>

using namespace OrgMode;

typedef void (*VerificationMethod)(const QByteArray& input, const QByteArray& output, OrgElement::Pointer element);
Q_DECLARE_METATYPE(VerificationMethod)

class ParserTests : public QObject
{
    Q_OBJECT

public:
    ParserTests();

private Q_SLOTS:
    void testParserAndIdentity_data();
    void testParserAndIdentity();
};

QString FL1(const char* text) {
    return QString::fromLatin1(text);
}

ParserTests::ParserTests()
{
}

void ParserTests::testParserAndIdentity_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<VerificationMethod>("method");

    //Verification of the properties of SimpleTree.org:
    VerificationMethod testSimpleTree = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        QCOMPARE(element->children().count(), 4);
        QVERIFY(findElement<OrgMode::Headline>(element, QLatin1String("Headline 1")));
        QVERIFY(findElement<OrgMode::Headline>(element, QLatin1String("Headline 1.1")));
        QVERIFY(findElement<OrgMode::Headline>(element, QLatin1String("Headline 1.2")));
        QVERIFY(findElement<OrgMode::Headline>(element, QLatin1String("Headline 2")));
        QVERIFY(!findElement<OrgMode::Headline>(element, QLatin1String("Headline 3")));
        //This finds the opening line by regular expression:
        QVERIFY(findElement<OrgMode::OrgLine>(element, QLatin1String("OrgMode file that the parser should parse")));
        //This line does not exist:
        QVERIFY(!findElement<OrgMode::OrgLine>(element, QLatin1String("There is no line like this")));
    };
    QTest::newRow("SimpleTree") << FL1("://TestData/Parser/SimpleTree.org") << testSimpleTree;

    //Verify that CLOCK: lines are detected, parsed, and the numbers calculated and aggregated up the tree:
    VerificationMethod testClockEntries = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        //Headline 1.1 contians one clock entry:
        QCOMPARE(Clock(findElement<OrgMode::Headline>(element, QLatin1String("headline_1_1"))).duration(), 10 * 60);
        //Headline 1.2 contains two clock entries that need to be accumulated:
        QCOMPARE(Clock(findElement<OrgMode::Headline>(element, QLatin1String("headline_1_2"))).duration(), 20 * 60);
        //Headline 1 is the parent of 1.1 and 1.2 and should have their times added up:
        QCOMPARE(Clock(findElement<OrgMode::Headline>(element, QLatin1String("headline_1"))).duration(), 30 * 60);
    };
    QTest::newRow("ClockEntries") << FL1("://TestData/Parser/ClockEntries.org") << testClockEntries;

    //Verify that tags are parsed and can be retrieved:
    VerificationMethod testTagParsing = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        //Headline 1 only has TAG1:
        static QRegularExpression headline1Regex(QLatin1String("headline_1"));
        Headline::Pointer headline_1 = findElement<OrgMode::Headline>(element, headline1Regex);
        QVERIFY(headline_1);
        Tags tags_1(headline_1);
        QVERIFY(tags_1.hasTag(QLatin1String("TAG1")));
        QVERIFY(!tags_1.hasTag(QLatin1String("NONSENSE")));
        //Headline 1.1 is tagged TAG2 and inherits TAG1 from headline 1:
        static QRegularExpression headline1_1Regex(QLatin1String("headline_1_1"));
        Headline::Pointer headline_1_1 = findElement<OrgMode::Headline>(element, headline1_1Regex);
        QVERIFY(headline_1_1);
        Tags tags_1_1(headline_1_1);
        QVERIFY(tags_1_1.hasTag(QLatin1String("TAG1"))); // inherited
        QVERIFY(tags_1_1.hasTag(QLatin1String("TAG2"))); // directly
        QVERIFY(!tags_1_1.hasTag(QLatin1String("NONSENSE")));
        //Headline 1.2 has two tags (this tests parsing of multiple tags):
        static QRegularExpression headline1_2Regex(QLatin1String("headline_1_2"));
        Headline::Pointer headline_1_2 = findElement<OrgMode::Headline>(element, headline1_2Regex);
        QVERIFY(headline_1_2);
        Tags tags_1_2(headline_1_2);
        QVERIFY(tags_1_2.hasTag(QLatin1String("TAG1"))); // inherited
        QVERIFY(tags_1_2.hasTag(QLatin1String("TEST"))); // directly
        QVERIFY(tags_1_2.hasTag(QLatin1String("VERIFY"))); // directly
        QVERIFY(!tags_1_2.hasTag(QLatin1String("NONSENSE")));
    };
    QTest::newRow("Tags") << FL1("://TestData/Parser/Tags.org") << testTagParsing;

    //Verify parsing of file attributes (#+ATTRIBUTE: value):
    VerificationMethod testFileAttributes = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        qDebug() << element->describe();
        auto const attributeLine = findElement<OrgMode::FileAttributeLine>(element, FL1("DRAWERS"));
        QVERIFY(attributeLine);
        QCOMPARE(attributeLine->value(), FL1("MyDrawers"));
        //An existing but empty file attribute:
        auto const emptyProperty = findElement<OrgMode::FileAttributeLine>(element, FL1("EMPTY_PROPERTY"));
        QVERIFY(emptyProperty);
        QVERIFY(emptyProperty->value().isEmpty());
        //auto const toplevel = findElement<OrgMode::OrgFile>(element, FL1("://TestData/Parser/DrawersAndProperties.org"));
        //Properties properties(toplevel);
        //QCOMPARE(properties.property(FL1("DRAWERS")), FL1("MyDrawers"));
    };
    QTest::newRow("FileAttributes") << FL1("://TestData/Parser/DrawersAndProperties.org") << testFileAttributes;
}

void ParserTests::testParserAndIdentity()
{
    QFETCH(QString, filename);
    QFETCH(VerificationMethod, method);

    OrgElement::Pointer element;
    QByteArray input;
    //Read the file into a OrgFile element:
    try {
        QFile orgFile(filename);
        QVERIFY(orgFile.exists());
        if (!orgFile.open(QIODevice::ReadOnly)) {
            throw RuntimeException(tr("Unable to open device for reading: %1.").arg(orgFile.errorString()));
        }
        input = orgFile.readAll();
        QBuffer buffer(&input);
        buffer.open(QBuffer::ReadOnly);
        QTextStream stream(&buffer);
        Parser parser;
        element = parser.parse(&stream, filename);
    } catch(Exception& ex) {
        QFAIL(qPrintable(ex.message()));
    }
    QByteArray output;
    //Write it to a QByteArray and verify input and output are identical:
    try {
        QBuffer outputBuffer(&output);
        outputBuffer.open(QBuffer::WriteOnly);
        QTextStream outputStream(&outputBuffer);
        Writer writer;
        writer.writeTo(&outputStream, element);
    }  catch(Exception& ex) {
        QFAIL(qPrintable(ex.message()));
    }
    //We now have access to the input data, the parsed element and the output data:
    QCOMPARE(input.size(), output.size());
    QCOMPARE(input, output);
    try {
        method(input, output, element);
    }  catch(Exception& ex) {
        QFAIL(qPrintable(ex.message()));
    }
}

QTEST_MAIN(ParserTests)

#include "tst_ParserTests.moc"
