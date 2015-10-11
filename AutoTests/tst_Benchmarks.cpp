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
#include <QString>
#include <QtTest>

#include <Parser.h>

#include "TestHelpers.h"

using namespace OrgMode;

class Benchmarks : public QObject
{
    Q_OBJECT

public:
    Benchmarks();

private Q_SLOTS:
    void benchmarkParseClocklines();
};

Benchmarks::Benchmarks()
{
}

void Benchmarks::benchmarkParseClocklines()
{
    const QString filename = FL1(":/Benchmarks/TestData/Benchmarks/BenchmarkClocklines.org");
    QFile orgFile(filename);
    QVERIFY(orgFile.open(QIODevice::ReadOnly));
    QTextStream stream(&orgFile);
    QBENCHMARK {
        Parser parser;
        const OrgElement::Pointer element = parser.parse(&stream, filename);
        Q_UNUSED(element);
    }
}

QTEST_APPLESS_MAIN(Benchmarks)

#include "tst_Benchmarks.moc"
