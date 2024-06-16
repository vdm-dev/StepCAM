//
// This file is part of StepCAM 2.
// Project URL: https://github.com/vdm-dev/StepCAM
// Copyright (c) 2020  Dmitry Lavygin (vdm.inbox@gmail.com).
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#include "hpglparser.h"

#include <QFile>
#include <QRegExp>

#include "utilities.h"


HpglParser::HpglParser(QObject* parent)
    : AbstractParser(parent)
{
    HpglParser::clear();
}

void HpglParser::clear()
{
    _tools.clear();
    // Insert zero (default) tool
    _tools[0] = AbstractTool();

    _curves.clear();

    _lineNumber = 1;
    _interrupted = false;

    _toolIsUp = true;
}

bool HpglParser::parse(QFile& file)
{
    clear();

    qint64 minX = 0;
    qint64 maxX = 0;
    qint64 minY = 0;
    qint64 maxY = 0;

    bool flagSetLimits = true;

    emit started(tr("Loading HPGL"));

    while (!file.atEnd())
    {
        if (_interrupted)
            return false;

        int done = static_cast<int>(file.pos() * 100 / file.size());
        emit progress(done, 100);

        QString line = QString::fromLatin1(file.readLine()).trimmed();

        if (line.isEmpty())
        {
            _lineNumber++;
            continue;
        }

        if (line.at(line.size() - 1) != ';')
        {
            warning(tr("Unknown command: '%1'.")
                .arg((line.size() > 20) ? (line.left(20) + "...") : line));

            _lineNumber++;
            continue;
        }

        line.chop(1); // Remove last ';' character

        bool parsed = true;

        if (line.compare("IN", Qt::CaseInsensitive) == 0 ||
            line.startsWith("PT", Qt::CaseInsensitive) ||
            line.startsWith("SP", Qt::CaseInsensitive))
        {
            // Do nothing
        }
        else if (line.compare("PU", Qt::CaseInsensitive) == 0)
        {
            _toolIsUp = true;
        }
        else if (line.compare("PD", Qt::CaseInsensitive) == 0)
        {
            _toolIsUp = false;

            if (!_curves.isEmpty())
            {
                AbstractCurve& curve = _curves.last();

                if (curve._type == AbstractCurve::CurveTypeNone && curve.count() == 1)
                    curve._type = AbstractCurve::CurveTypePoint;
            }
        }
        else if (line.startsWith("PA", Qt::CaseInsensitive))
        {
            int separator = line.indexOf(',');

            if (separator < 0)
            {
                parsed = false;
            }
            else
            {
                bool ok;
                qint64 x = line.midRef(2, separator - 2).toLongLong(&ok);
                parsed = parsed && ok;
                qint64 y = line.midRef(separator + 1).toLongLong(&ok);
                parsed = parsed && ok;

                if (parsed)
                {
                    x *= 25;
                    y *= 25;

                    if (_curves.isEmpty())
                        _curves.append(AbstractCurve());

                    AbstractCurve* curve = &_curves.last();

                    if (_toolIsUp)
                    {
                        if (curve->_type != AbstractCurve::CurveTypeNone)
                        {
                            _curves.append(AbstractCurve());
                            curve = &_curves.last();
                        }

                        curve->_type = AbstractCurve::CurveTypeNone;
                        curve->_x.resize(1);
                        curve->_y.resize(1);
                        curve->_x[0] = x;
                        curve->_y[0] = y;
                    }
                    else
                    {
                        curve->_type = AbstractCurve::CurveTypeCurve;
                        curve->_x.append(x);
                        curve->_y.append(y);
                    }

                    if (flagSetLimits)
                    {
                        minX = x;
                        maxX = x;
                        minY = y;
                        maxY = y;
                        flagSetLimits = false;
                    }
                    else
                    {
                        minX = qMin(minX, x);
                        maxX = qMax(maxX, x);
                        minY = qMin(minY, y);
                        maxY = qMax(maxY, y);
                    }
                }
            }
        }

        if (!parsed)
        {
            warning(tr("Unknown command: '%1'.")
                .arg((line.size() > 20) ? (line.left(20) + "...") : line));
        }

        _lineNumber++;
    }

    emit progress(100, 100);

    QString sMinX = Utilities::coordinateToString(minX);
    QString sMaxX = Utilities::coordinateToString(maxX);
    QString sDltX = Utilities::coordinateToString(maxX - minX);

    QString sMinY = Utilities::coordinateToString(minY);
    QString sMaxY = Utilities::coordinateToString(maxY);
    QString sDltY = Utilities::coordinateToString(maxY - minY);

    accept(tr("The file has been successfully loaded.\nBoundaries of coordinates:\n"
        "Xmin = %1 mm, Xmax = %2 mm, \xCE\x94X = %3 mm,\n"
        "Ymin = %4 mm, Ymax = %5 mm, \xCE\x94Y = %6 mm.")
        .arg(sMinX, sMaxX, sDltX, sMinY, sMaxY, sDltY), " ");

    emit finished();

    return true;
}

void HpglParser::interrupt()
{
    _interrupted = true;
}
