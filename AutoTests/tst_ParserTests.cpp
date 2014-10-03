#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <OrgFileContent.h>
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
#include <Drawer.h>

using namespace OrgMode;

typedef void (*VerificationMethod)(const QByteArray& input, const QByteArray& output, OrgElement::Pointer element);
Q_DECLARE_METATYPE(VerificationMethod)

class ParserTests : public QObject
{
    Q_OBJECT

public:
    ParserTests();

private Q_SLOTS:
    void testOrgFileContent();
    void testParserAndIdentity_data();
    void testParserAndIdentity();
};

QString FL1(const char* text) {
    return QString::fromLatin1(text);
}

ParserTests::ParserTests()
{
}

void ParserTests::testOrgFileContent()
{
    OrgFileContent content;
    QVERIFY(content.atEnd());
    const QString line1(FL1("1"));
    const QString line2(FL1("2"));
    content.ungetLine(line1);
    content.ungetLine(line2);
    QVERIFY(!content.atEnd());
    //Now LIFO-style, line2 is expected first, line1 second:
    QCOMPARE(content.getLine(), line2);
    QCOMPARE(content.getLine(), line1);
    QVERIFY(content.atEnd());
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
        //Test for an existing, non-empty attribute:
        auto const attributeLine = findElement<OrgMode::FileAttributeLine>(element, FL1("DRAWERS"));
        QVERIFY(attributeLine);
        QCOMPARE(attributeLine->value(), FL1("MyDrawers"));
        //An existing but empty file attribute:
        auto const emptyProperty = findElement<OrgMode::FileAttributeLine>(element, FL1("EMPTY_PROPERTY"));
        QVERIFY(emptyProperty);
        QVERIFY(emptyProperty->value().isEmpty());
        //A non-existant file attribute:
        auto const nonExistentAttribute = findElement<OrgMode::FileAttributeLine>(element, FL1("I DO NOT EXIST"));
        QVERIFY(!nonExistentAttribute);
    };
    QTest::newRow("FileAttributes") << FL1("://TestData/Parser/DrawersAndProperties.org") << testFileAttributes;

    //Verify calculation of properties for individual elements:
    VerificationMethod testPropertyCalculation = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        //Headline 1 inherits the attributes from the attributes of the file it is in:
        auto const headline_1 = findElement<OrgMode::Headline>(element, FL1("headline_1"));
        QVERIFY(headline_1);
        Properties properties(headline_1);
        //A file level property:
        QCOMPARE(properties.property(FL1("DRAWERS")), FL1("MyDrawers"));
        //A file level property, but empty:
        QCOMPARE(properties.property(FL1("EMPTY_PROPERTY")), FL1(""));
        //TODO element properties are not parsed yet, depends on drawer parsing
    };
    QTest::newRow("PropertyCalculation") << FL1("://TestData/Parser/DrawersAndProperties.org") << testPropertyCalculation;

    //Test two-pass parsing that provides the file properties first that will influence element parsing later:
    VerificationMethod testTwoPassParsing = [](const QByteArray&, const QByteArray&, OrgElement::Pointer element) {
        qDebug() << endl << qPrintable(element->describe());
        //Drawers are only identified if the first pass yielded a value for the #+DRAWERS: property
        auto const headline_1 = findElement<OrgMode::Headline>(element, FL1("headline_1"));
        QVERIFY(headline_1);
        auto const myDrawer = findElement<OrgMode::Drawer>(element, FL1("MyDrawers"));
        QVERIFY(myDrawer);
//        //TODO verify elements

        //Verify that content in drawer syntax is not considered a drawer if the name is not in #+DRAWERS:
        auto const headline_2 = findElement<OrgMode::Headline>(element, FL1("headline_2"));
        QVERIFY(headline_1);
        auto const notADrawer = findElement<OrgMode::Drawer>(element, FL1("NotADrawer"));
        QVERIFY(!notADrawer);
    };
    QTest::newRow("TwoPassParsing") << FL1("://TestData/Parser/DrawersAndProperties.org") << testTwoPassParsing;
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
