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


#include "excellonparser.h"

#include <QFile>
#include <QRegExp>

#include "utilities.h"


ExcellonParser::ExcellonParser(QObject* parent)
    : AbstractParser(parent)
{
    ExcellonParser::clear();
}

void ExcellonParser::clear()
{
    _tools.clear();
    // Insert zero (default) tool
    _tools[0] = AbstractTool();

    _points.clear();

    _stage = StageBeginning;
    _format = FormatUnknown;
    _units = UnitsUnknown;
    _lineNumber = 1;
    _interrupted = false;
    _toolNumber = 0;

    _minX = 0;
    _maxX = 0;
    _minY = 0;
    _maxY = 0;

    _flagNeedRecalculate = false;
    _flagSetLimits = true;
}

bool ExcellonParser::parse(QFile& file)
{
    clear();

    emit started(tr("Loading Excellon"));

    while (!file.atEnd())
    {
        if (_interrupted)
            return false;

        int done = static_cast<int>(file.pos() * 100 / file.size());
        emit progress(done, 100);

        QString line = QString::fromLatin1(file.readLine()).trimmed();

        bool abort = false;

        if (!line.isEmpty())
        {
            if (_stage == StageTail)
            {
                warning(tr("The file contains not empty lines after the end of the program.\n"
                    "They will be ignored."));
                break;
            }

            if (!parseComment(line, abort) && !abort)
            {
                if (!parseHeader(line, abort) && !abort)
                {
                    if (!parseBody(line, abort) && !abort)
                    {
                        warning(tr("Unknown command: '%1'.")
                            .arg((line.size() > 20) ? (line.left(20) + "...") : line));
                    }
                }
            }
        }

        if (abort)
            return false;

        _lineNumber++;
    }

    emit progress(100, 100);

    if (_flagNeedRecalculate)
    {
        emit started(tr("Recalculating Points"));

        bool ok = true;

        if (_format == FormatUnknown)
        {
            ok = false;
        }
        else
        {
            for (int i = 0; i < _points.size(); ++i)
            {
                if (_interrupted)
                    return false;

                emit progress(i, _points.size());

                AbstractCurve& point = _points[i];

                if (point._type == AbstractCurve::CurveTypePoint)
                    continue;

                point._x.resize(1);
                point._x[0] = parseNumber(point._stringX, &ok);

                if (ok)
                {
                    point._y.resize(1);
                    point._y[0] = parseNumber(point._stringY, &ok);
                }

                if (ok)
                {
                    point._type = AbstractCurve::CurveTypePoint;

                    if (_flagSetLimits)
                    {
                        _minX = point._x[0];
                        _maxX = point._x[0];
                        _minY = point._y[0];
                        _maxY = point._y[0];
                        _flagSetLimits = false;
                    }
                    else
                    {
                        _minX = qMin(_minX, point._x[0]);
                        _maxX = qMax(_maxX, point._x[0]);
                        _minY = qMin(_minY, point._y[0]);
                        _maxY = qMax(_maxY, point._y[0]);
                    }
                }
                else
                {
                    break;
                }
            }
            emit progress(_points.size(), _points.size());
        }

        if (!ok)
        {
            error(tr("Unable to determine the number presentation format.\nTry to change the "
                "Excellon export configuration in the Sprint-Layout:\n"
                "- keep leading zeros,\n"
                "- use the output with a decimal point,\n"
                "- do not suppress comments."), " ");
            return false;
        }
    }

    if (_points.empty())
    {
        warning(tr("The file has been successfully loaded, but it does not contain any "
            "coordinates for drilling."), " ");
    }
    else
    {
        QString minX = Utilities::coordinateToString(_minX);
        QString maxX = Utilities::coordinateToString(_maxX);
        QString dltX = Utilities::coordinateToString(_maxX - _minX);

        QString minY = Utilities::coordinateToString(_minY);
        QString maxY = Utilities::coordinateToString(_maxY);
        QString dltY = Utilities::coordinateToString(_maxY - _minY);

        accept(tr("The file has been successfully loaded.\nBoundaries of coordinates:\n"
            "Xmin = %1 mm, Xmax = %2 mm, \xCE\x94X = %3 mm,\n"
            "Ymin = %4 mm, Ymax = %5 mm, \xCE\x94Y = %6 mm.")
            .arg(minX, maxX, dltX, minY, maxY, dltY), " ");
    }

    emit finished();

    return true;
}

void ExcellonParser::interrupt()
{
    _interrupted = true;
}

bool ExcellonParser::parseComment(const QString& line, bool& abort)
{
    Q_UNUSED(abort)

    // Comment
    if (line.startsWith(';'))
    {
        if (_stage == StageBeginning && line.startsWith("; Format: ", Qt::CaseInsensitive))
        {
            QString formatString = line.mid(10, 3);

            if (formatString == "2.4")
            {
                _format = Format24;
            }
            else if (formatString == "3.2")
            {
                _format = Format32;
            }
            else if (formatString == "3.3")
            {
                _format = Format33;
            }
            else
            {
                if (formatString.isEmpty())
                    formatString = tr("<empty>");

                _format = FormatUnknown;
                warning(tr("Unknown number presentation format: '%1'.").arg(formatString));
            }
        }

        return true;
    }

    return false;
}

bool ExcellonParser::parseHeader(const QString& line, bool& abort)
{
    if (line.compare("M48", Qt::CaseInsensitive) == 0)
    {
        if (_stage != StageBeginning)
        {
            error(tr("The redefinition of the header is prohibited.\n"
                "The input file can contain one header only."));

            abort = true;
        }
        else
        {
            _stage = StageHeader;
        }
        return true;
    }

    if (line.startsWith("METRIC", Qt::CaseInsensitive) ||
        line.compare("M71", Qt::CaseInsensitive) == 0)
    {
        if (_stage != StageHeader)
        {
            error(tr("An unexpected occurrence of the measuring system change command.\n"
                "The units of measurement can only be defined inside the header."));

            abort = true;
        }
        else
        {
            if (_units != UnitsUnknown)
            {
                warning(tr("The redefinition of the units of measurement.\n"
                    "Using the metric measuring system."));
            }
            else
            {
                notice(tr("Using the metric measuring system."),
                    QString::number(_lineNumber));
            }

            _units = UnitsMetric;

            if (_format == Format24)
            {
                warning(tr("A mismatch was detected between the number presentation format "
                    "and the current measurement system.\nThe actual number "
                    "presentation format will be determined later."));

                _format = FormatUnknown;
            }
        }
        return true;
    }

    if (line.startsWith("INCH", Qt::CaseInsensitive) ||
        line.compare("M72", Qt::CaseInsensitive) == 0)
    {
        if (_stage != StageHeader)
        {
            error(tr("An unexpected occurrence of the measuring system change command.\n"
                "The units of measurement can only be defined inside the header."));

            abort = true;
        }
        else
        {
            if (_units != UnitsUnknown)
            {
                warning(tr("The redefinition of the units of measurement.\n"
                    "Converting from the inch measuring system."));
            }
            else
            {
                notice(tr("Converting from the inch measuring system."),
                    QString::number(_lineNumber));
            }

            _units = UnitsInch;

            if (_format == Format32 || _format == Format33)
            {
                warning(tr("A mismatch was detected between the number presentation format "
                    "and the current measurement system.\nThe actual number "
                    "presentation format is set to 2.4."));
            }
            else if (_format == FormatUnknown)
            {
                notice(tr("The actual number presentation format is set to 2.4."));
            }
            _format = Format24;
        }
        return true;
    }

    if (line.startsWith('T', Qt::CaseInsensitive))
    {
        QRegExp expression;
        expression.setCaseSensitivity(Qt::CaseInsensitive);

        int toolNumber = -1;
        QString toolDiameterString;

        expression.setPattern("^T(\\d{1,4})C(\\d*\\.\\d+)");

        if (expression.indexIn(line) > -1)
        {
            toolNumber = expression.cap(1).toInt();
            toolDiameterString = expression.cap(2);
        }
        else
        {
            expression.setPattern("^T(\\d{1,4})");

            if (expression.indexIn(line) > -1)
                toolNumber = expression.cap(1).toInt();
        }

        if (toolNumber > -1)
        {
            _toolNumber = toolNumber;
            _tools[toolNumber]._id = toolNumber;

            if (!toolDiameterString.isEmpty())
            {
                if (_units != UnitsUnknown)
                {
                    _tools[toolNumber]._diameter = static_cast<int>(parseNumber(toolDiameterString));

                    int fractional = _tools[toolNumber]._diameter % 10;

                    if (fractional > 4)
                        _tools[toolNumber]._diameter += 10;

                    _tools[toolNumber]._diameter -= fractional;
                }
                else
                {
                    error(tr("Unable to determine the diameter of tool #%1. "
                        "The measuring system has not yet been determined.").arg(toolNumber));

                    abort = true;
                }
            }
        }
        else
        {
            warning(tr("Unknown command: '%1'.")
                .arg((line.size() > 20) ? (line.left(20) + "...") : line));
        }
        return true;
    }

    if (line == "%")
    {
        if (_stage != StageHeader)
        {
            error(tr("An unexpected occurrence of the header end command.\n"
                "The input file can contain one header only."));

            abort = true;
        }
        else if (_units == UnitsUnknown)
        {
            error(tr("The file header does not contain any information about the measurement "
                "system."));

            abort = true;
        }
        else
        {
            _stage = StageBody;
            _toolNumber = 0;
        }
        return true;
    }

    return false;
}

bool ExcellonParser::parseBody(const QString& line, bool& abort)
{
    if (line.compare("G90", Qt::CaseInsensitive) == 0)
        return true;

    if (line.compare("G05", Qt::CaseInsensitive) == 0)
    {
        _stage = StageDrill;
        return true;
    }

    if (line.compare("M30", Qt::CaseInsensitive) == 0)
    {
        _stage = StageTail;
        return true;
    }

    if (_stage != StageDrill)
        return false;

    if (line.startsWith('G', Qt::CaseInsensitive))
    {
        error(tr("Unknown G-command: '%1'. The file analysis will be interrupted to avoid "
            "problems with the interpretation of commands.")
            .arg((line.size() > 20) ? (line.left(20) + "...") : line));

        abort = true;
        return true;
    }

    if (line.startsWith('X', Qt::CaseInsensitive))
    {
        QRegExp expression;
        expression.setCaseSensitivity(Qt::CaseInsensitive);
        expression.setPattern("^X([\\+\\-]?\\d*\\.?\\d+)Y([\\+\\-]?\\d*\\.?\\d+)");

        if (expression.indexIn(line) < 0)
            return false;

        AbstractCurve point;
        point._tool = _toolNumber;
        point._stringX = expression.cap(1);
        point._stringY = expression.cap(2);

        bool ok;

        point._x.resize(1);
        point._x[0] = parseNumber(point._stringX, &ok);

        if (ok)
        {
            point._y.resize(1);
            point._y[0] = parseNumber(point._stringY, &ok);
        }

        if (ok)
        {
            point._type = AbstractCurve::CurveTypePoint;

            if (_flagSetLimits)
            {
                _minX = point._x[0];
                _maxX = point._x[0];
                _minY = point._y[0];
                _maxY = point._y[0];
                _flagSetLimits = false;
            }
            else
            {
                _minX = qMin(_minX, point._x[0]);
                _maxX = qMax(_maxX, point._x[0]);
                _minY = qMin(_minY, point._y[0]);
                _maxY = qMax(_maxY, point._y[0]);
            }
        }
        else
        {
            _flagNeedRecalculate = true;
        }

        _points << point;

        return true;
    }

    return false;
}

qint64 ExcellonParser::parseNumber(const QString& number, bool* ok)
{
    qint64 result = 0;

    if (ok)
        *ok = false;

    if (number.length() < 1)
        return result;

    bool positive = true;
    int offset = 0;

    if (number[offset] == '+')
    {
        offset++;
    }
    else if (number[offset] == '-')
    {
        positive = false;
        offset++;
    }

    if (_units == UnitsInch)
    {
        int index = number.indexOf('.', offset);

        if (index > -1)
        {
            QString fractional = number.mid(index + 1);

            if (fractional.size() > 4)
                fractional = fractional.left(4);

            while (fractional.size() < 4)
                fractional.append('0');

            result = number.mid(offset, index - offset).toLongLong() * 10000;
            result += fractional.toLongLong();
            result = result * 254 / 100;

            if (ok)
                *ok = true;
        }
        else
        {
            QString digits = number.mid(offset);

            if (digits.size() > 6)
                digits = digits.right(6);

            result = digits.toLongLong();
            result = result * 254 / 100;

            if (ok)
                *ok = true;
        }
    }
    else if (_units == UnitsMetric)
    {
        int index = number.indexOf('.', offset);

        if (index > -1)
        {
            QString fractional = number.mid(index + 1);

            if (fractional.size() > 3)
                fractional = fractional.left(3);

            while (fractional.size() < 3)
                fractional.append('0');

            result = number.mid(offset, index - offset).toLongLong() * 1000;
            result += fractional.toLongLong();

            if (ok)
                *ok = true;
        }
        else if (_format == Format32)
        {
            QString digits = number.mid(offset);

            if (digits.size() > 5)
                digits = digits.right(5);

            result = digits.toLongLong() * 10;

            if (ok)
                *ok = true;
        }
        else if (_format == Format33)
        {
            QString digits = number.mid(offset);

            if (digits.size() > 6)
                digits = digits.right(6);

            result = digits.toLongLong();

            if (ok)
                *ok = true;
        }
        else
        {
            QString digits = number.mid(offset);

            if (digits.size() == 6)
            {
                _format = Format33;
                warning(tr("The actual number presentation format is set to 3.3. "
                    "Check the output program carefully."));

                result = digits.toLongLong();

                if (ok)
                    *ok = true;
            }
            else if (digits.size() == 5 && digits[0] == '0')
            {
                _format = Format32;
                warning(tr("The actual number presentation format is set to 3.2. "
                    "Check the output program carefully."));

                result = digits.toLongLong() * 10;

                if (ok)
                    *ok = true;
            }
        }
    }

    // Round
    //result = ((result % 10) > 4) ? (result + 10 - (result % 10)) : (result - (result % 10));

    return positive ? result : -result;
}
