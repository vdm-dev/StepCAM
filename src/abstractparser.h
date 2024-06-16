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


#ifndef ABSTRACTPARSER_H
#define ABSTRACTPARSER_H


#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>

#include "logitem.h"


class QFile;


class AbstractTool
{
public:
    AbstractTool()
        : _id(0)
        , _diameter(0)
    {
    }

    int id() const { return _id; }
    int diameter() const { return _diameter; }

private:
    int _id;
    int _diameter;

    friend class ExcellonParser;
    friend class HpglParser;
};


class AbstractCurve
{
public:
    enum CurveType
    {
        CurveTypeNone,
        CurveTypePoint,
        CurveTypeCurve
    };

    AbstractCurve()
        : _type(CurveTypeNone)
        , _tool(0)
    {
    }

    virtual ~AbstractCurve()
    {
    }

    int tool() const { return _tool; }
    int count() const { return qMin(_x.size(), _y.size()); }

    const qint64* x() const { return _x.data(); }
    const qint64* y() const { return _y.data(); }

    CurveType type() const { return _type; }

private:
    QVector<qint64> _x;
    QVector<qint64> _y;

    QString _stringX;
    QString _stringY;

    CurveType _type;

    int _tool;

    friend class ExcellonParser;
    friend class HpglParser;
};


class AbstractParser : public QObject
{
    Q_OBJECT

public:
    enum ParserType
    {
        ParserNone,
        ParserDrilling,
        ParserMillling
    };

    explicit AbstractParser(QObject* parent = nullptr)
        : QObject(parent)
        , _lineNumber(0)
        , _interrupted(false)
    {
    }

    virtual ParserType type() const = 0;

    virtual void clear() = 0;
    virtual bool parse(QFile& file) = 0;

    virtual const QMap<int, AbstractTool>& tools() const = 0;
    virtual const QList<AbstractCurve>& curves() const = 0;

    void error(const QString& description, const QString& line = QString());
    void warning(const QString& description, const QString& line = QString());
    void notice(const QString& description, const QString& line = QString());
    void accept(const QString& description, const QString& line = QString());

    bool isInterrupted() const { return _interrupted; }

public slots:
    virtual void interrupt() = 0;

signals:
    void log(int severity, const QString& description, const QString& line);
    void started(const QString& operation);
    void progress(int done, int total);
    void finished();

protected:
    int _lineNumber;
    bool _interrupted;
};


inline void AbstractParser::error(const QString& description, const QString& line)
{
    emit log(LogItem::SeverityError, description,
        line.isEmpty() ? QString::number(_lineNumber) : line);
}

inline void AbstractParser::warning(const QString& description, const QString& line)
{
    emit log(LogItem::SeverityWarning, description,
        line.isEmpty() ? QString::number(_lineNumber) : line);
}

inline void AbstractParser::notice(const QString& description, const QString& line)
{
    emit log(LogItem::SeverityNotice, description,
        line.isEmpty() ? QString::number(_lineNumber) : line);
}

inline void AbstractParser::accept(const QString& description, const QString& line)
{
    emit log(LogItem::SeverityAccept, description,
        line.isEmpty() ? QString::number(_lineNumber) : line);
}


#endif // ABSTRACTPARSER_H
