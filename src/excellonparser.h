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


#ifndef EXCELLONPARSER_H
#define EXCELLONPARSER_H


#include "abstractparser.h"


class QFile;


class ExcellonParser : public AbstractParser
{
    Q_OBJECT

public:
    explicit ExcellonParser(QObject* parent = nullptr);

    virtual ParserType type() const;

    virtual void clear();
    virtual bool parse(QFile& file);

    virtual const QMap<int, AbstractTool>& tools() const;
    virtual const QList<AbstractCurve>& curves() const;

public slots:
    virtual void interrupt();

private:
    enum Stage
    {
        StageBeginning,
        StageHeader,
        StageBody,
        StageDrill,
        StageTail
    };

    enum Format
    {
        FormatUnknown,
        Format24,
        Format32,
        Format33
    };

    enum Units
    {
        UnitsUnknown,
        UnitsMetric,
        UnitsInch
    };

    bool parseComment(const QString& line, bool& abort);
    bool parseHeader(const QString& line, bool& abort);
    bool parseBody(const QString& line, bool& abort);
    qint64 parseNumber(const QString& number, bool* ok = nullptr);

    QMap<int, AbstractTool> _tools;
    QList<AbstractCurve> _points;

    Stage _stage;
    Format _format;
    Units _units;

    qint64 _minX;
    qint64 _maxX;
    qint64 _minY;
    qint64 _maxY;

    int _toolNumber;

    bool _flagNeedRecalculate;
    bool _flagSetLimits;
};


inline AbstractParser::ParserType ExcellonParser::type() const
{
    return ParserDrilling;
}

inline const QMap<int, AbstractTool>& ExcellonParser::tools() const
{
    return _tools;
}

inline const QList<AbstractCurve>& ExcellonParser::curves() const
{
    return _points;
}


#endif // EXCELLONPARSER_H
