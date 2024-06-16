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


#include "mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>

#include "aboutdialog.h"
#include "logfiltermodel.h"
#include "utilities.h"
#include "mousewheeleventfilter.h"
#include "excellonparser.h"
#include "hpglparser.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _parser(nullptr)
    , _progress(nullptr)
    , _operationCanceled(false)
{
    setupUi(this);

    setUnifiedTitleAndToolBarOnMac(true);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    _progress = new ProgressStatusWidget();
    _statusBar->addPermanentWidget(_progress);
    _progress->hide();

    MouseWheelEventFilter* mouseWheelEventFilter = new MouseWheelEventFilter(this);
    QList<QAbstractSpinBox*> spinBoxes = _dockMilling->findChildren<QAbstractSpinBox*>() +
        _dockDrilling->findChildren<QAbstractSpinBox*>();

    for (int i = 0; i < spinBoxes.size(); ++i)
        spinBoxes[i]->installEventFilter(mouseWheelEventFilter);

    for (int i = 0; i < _tabs->count(); ++i)
    {
        QWidget* widget = _tabs->widget(i);

        if (widget)
        {
            widget->setWindowTitle(_tabs->tabText(i));
            widget->setWindowIcon(_tabs->tabIcon(i));
        }
    }

    _tabs->removeTab(_tabs->indexOf(_tabSettings));

    QVBoxLayout* logLayout = qobject_cast<QVBoxLayout*>(_tabLog->layout());
    if (logLayout)
    {
        QWidget *spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QToolBar* logToolBar = new QToolBar();
        logToolBar->setMovable(false);
        logToolBar->setAllowedAreas(Qt::TopToolBarArea);
        logToolBar->setIconSize(QSize(16, 24));
        logToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        logToolBar->setFloatable(false);
        logToolBar->addAction(_actionLogErrors);
        logToolBar->addSeparator();
        logToolBar->addAction(_actionLogWarnings);
        logToolBar->addSeparator();
        logToolBar->addAction(_actionLogNotices);
        logToolBar->addWidget(spacer);
        logToolBar->addAction(_actionLogClear);

        logLayout->insertWidget(0, logToolBar);
    }

    LogFilterModel* filterModel = new LogFilterModel(this);

    if (filterModel)
    {
        filterModel->setSourceModel(&_log);
        filterModel->setFilterAction(LogItem::SeverityError, _actionLogErrors);
        filterModel->setFilterAction(LogItem::SeverityWarning, _actionLogWarnings);
        filterModel->setFilterAction(LogItem::SeverityNotice, _actionLogNotices);
        _tableLog->setModel(filterModel);
    }
    else
    {
        _tableLog->setModel(&_log);
    }

    _tableLog->horizontalHeader()->setSectionsClickable(false);
    _tableLog->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    _tableLog->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _tableLog->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    _tableLog->horizontalHeader()->resizeSection(0, 24);
    _tableLog->horizontalHeader()->resizeSection(3, 100);
    _tableLog->horizontalHeader()->setSectionHidden(1, true);
    //_tableLog->verticalHeader()->setResizeContentsPrecision(0);
    _tableLog->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    _iconLog.addFile(":/img/page_log.png");
    _iconLogOk.addFile(":/img/page_log_ok.png");
    _iconLogError.addFile(":/img/page_log_error.png");
    _iconLogWarning.addFile(":/img/page_log_warning.png");

    _iconScript.addFile(":/img/script.png");
    _iconScriptLightning.addFile(":/img/script_lightning.png");
    _iconScriptGreen.addFile(":/img/script_green.png");
    _iconScriptYellow.addFile(":/img/script_yellow.png");
    _iconScriptRed.addFile(":/img/script_red.png");

    // Global
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
        SLOT(handleEditActions()));
    connect(_tabs, SIGNAL(currentChanged(int)), this, SLOT(handleEditActions()));
    connect(_editProgram, SIGNAL(selectionChanged()), this, SLOT(handleEditActions()));
    connect(_editProgram, SIGNAL(textChanged()), this, SLOT(handleEditActions()));

    // Log
    connect(&_log, SIGNAL(updated(int, int, int, int)), this, SLOT(logUpdated(int, int, int, int)));
    connect(_actionLogClear, SIGNAL(triggered()), &_log, SLOT(clear()));
    connect(_actionLogErrors, SIGNAL(toggled(bool)), &_log, SLOT(refresh()));
    connect(_actionLogWarnings, SIGNAL(toggled(bool)), &_log, SLOT(refresh()));
    connect(_actionLogNotices, SIGNAL(toggled(bool)), &_log, SLOT(refresh()));

    // File Operations
    connect(_actionClose, SIGNAL(triggered()), this, SLOT(fileCloseAction()));
    connect(_actionOpen, SIGNAL(triggered()), this, SLOT(fileOpenAction()));
    connect(_actionReload, SIGNAL(triggered()), this, SLOT(fileReloadAction()));
    connect(_actionSave, SIGNAL(triggered()), this, SLOT(fileSaveAction()));
    connect(_actionSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAsAction()));

    // Edit
    connect(_actionCopy, SIGNAL(triggered()), _editProgram, SLOT(copy()));
    connect(_actionSelectAll, SIGNAL(triggered()), _editProgram, SLOT(selectAll()));
    connect(_actionSettings, SIGNAL(triggered()), this, SLOT(settingsOpen()));

    // Program
    connect(_actionGenerate, SIGNAL(triggered()), this, SLOT(generate()));

    // Window
    connect(_actionResetLayout, SIGNAL(triggered()), this, SLOT(layoutReset()));

    // Help
    connect(_actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    // Settings
    connect(_buttonSettingsClose, SIGNAL(clicked()), this, SLOT(settingsClose()));

    _defaultState = saveState();

    loadSettings();

    fileClose();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (fileSave(true))
    {
        saveSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::loadSettings()
{
    QString fileName =
        QDir(QApplication::applicationDirPath()).filePath(QApplication::applicationName() + ".ini");

    QSettings settings(fileName, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    move(settings.value("Position", QPoint()).toPoint());
    resize(settings.value("Size", size()).toSize());
    restoreState(settings.value("State").toByteArray());
    settings.endGroup();

    settings.beginGroup("Files");
    _lastFileDir = settings.value("LastDirectory").toString();
    settings.endGroup();

    settings.beginGroup("Milling");
    _editMillingSpindleSpeed->setValue(settings.value("SpindleSpeed", 10000).toInt());
    _editMillingFeedRate->setValue(settings.value("Feed", 1).toInt());
    _editMillingPlungeRate->setValue(settings.value("Plunge", 1).toInt());
    _editMillingSafeZ->setValue(settings.value("SafeZ", 1.0).toDouble());
    _editMillingDepth->setValue(settings.value("Depth", 0.0).toDouble());

    _editSettingsMillingPrologue->setPlainText(
        settings.value("Prologue", _editSettingsMillingPrologue->toPlainText()).toString());

    _editSettingsMillingEpilogue->setPlainText(
        settings.value("Epilogue", _editSettingsMillingEpilogue->toPlainText()).toString());

    settings.endGroup();

    settings.beginGroup("Drilling");
    _editDrillingSpindleSpeed->setValue(settings.value("SpindleSpeed", 10000).toInt());
    _editDrillingFeedRate->setValue(settings.value("Feed", 1).toInt());
    _editDrillingSafeZ->setValue(settings.value("SafeZ", 1.0).toDouble());
    _editDrillingDepth->setValue(settings.value("Depth", 0.0).toDouble());
    _editDrillingStartHeight->setValue(settings.value("StartHeight", 0.5).toDouble());
    _checkDrillingTcHeight->setChecked(settings.value("TcHeightEnabled", false).toBool());
    _editDrillingTcHeight->setValue(settings.value("TcHeight", 0.0).toDouble());
    _checkDrillingSingleTool->setChecked(settings.value("SingleToolEnabled", false).toBool());

    _editSettingsDrillingPrologue->setPlainText(
        settings.value("Prologue", _editSettingsDrillingPrologue->toPlainText()).toString());

    _editSettingsDrillingEpilogue->setPlainText(
        settings.value("Epilogue", _editSettingsDrillingEpilogue->toPlainText()).toString());

    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QString fileName =
        QDir(QApplication::applicationDirPath()).filePath(QApplication::applicationName() + ".ini");

    QSettings settings(fileName, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("Position", pos());
    settings.setValue("Size", size());
    settings.setValue("State", saveState());
    settings.endGroup();

    settings.beginGroup("Files");
    settings.setValue("LastDirectory", _lastFileDir);
    settings.endGroup();

    settings.beginGroup("Milling");
    settings.setValue("SpindleSpeed", _editMillingSpindleSpeed->value());
    settings.setValue("Feed", _editMillingFeedRate->value());
    settings.setValue("Plunge", _editMillingPlungeRate->value());
    settings.setValue("SafeZ", _editMillingSafeZ->value());
    settings.setValue("Depth", _editMillingDepth->value());
    settings.setValue("Prologue", _editSettingsMillingPrologue->toPlainText());
    settings.setValue("Epilogue", _editSettingsMillingEpilogue->toPlainText());
    settings.endGroup();

    settings.beginGroup("Drilling");
    settings.setValue("SpindleSpeed", _editDrillingSpindleSpeed->value());
    settings.setValue("Feed", _editDrillingFeedRate->value());
    settings.setValue("SafeZ", _editDrillingSafeZ->value());
    settings.setValue("Depth", _editDrillingDepth->value());
    settings.setValue("StartHeight", _editDrillingStartHeight->value());
    settings.setValue("TcHeightEnabled", _checkDrillingTcHeight->isChecked());
    settings.setValue("TcHeight", _editDrillingTcHeight->value());
    settings.setValue("SingleToolEnabled", _checkDrillingSingleTool->isChecked());
    settings.setValue("Prologue", _editSettingsDrillingPrologue->toPlainText());
    settings.setValue("Epilogue", _editSettingsDrillingEpilogue->toPlainText());
    settings.endGroup();
}

void MainWindow::logParser(int severity, const QString& description, const QString& line)
{
    _log.add(severity, description, _inputFileName, line);
}

void MainWindow::logUpdated(int errors, int warnings, int notices, int accepts)
{
    _actionLogErrors->setText(tr("%1 Errors").arg(errors));
    _actionLogWarnings->setText(tr("%1 Warnings").arg(warnings));
    _actionLogNotices->setText(tr("%1 Notices").arg(notices));

    if (errors > 0)
    {
        _tabs->setTabIcon(_tabs->indexOf(_tabLog), _iconLogError);
    }
    else if (warnings > 0)
    {
        _tabs->setTabIcon(_tabs->indexOf(_tabLog), _iconLogWarning);
    }
    else if (accepts > 0)
    {
        _tabs->setTabIcon(_tabs->indexOf(_tabLog), _iconLogOk);
    }
    else
    {
        _tabs->setTabIcon(_tabs->indexOf(_tabLog), _iconLog);
    }
}

void MainWindow::updateProjectState(bool modified)
{
    QString title = QApplication::applicationName() + ' ' + QApplication::applicationVersion();

    if (_currentFileName.isEmpty())
    {
        _actionSave->setDisabled(true);
        _actionSaveAs->setDisabled(true);
    }
    else
    {
        _actionSave->setEnabled(modified);
        _actionSaveAs->setEnabled(modified || !_currentFilePath.isEmpty());

        title.prepend("[*]" + _currentFileName + " - ");

        setWindowModified(modified);
    }

    setWindowTitle(title);
}

void MainWindow::handleEditActions()
{
    if (_editProgram->hasFocus() || _tabs->currentWidget() == _tabProgram)
    {
        _actionCopy->setEnabled(_editProgram->textCursor().hasSelection());
        _actionSelectAll->setEnabled(!_editProgram->document()->isEmpty());
    }
    else
    {
        _actionCopy->setDisabled(true);
        _actionSelectAll->setDisabled(true);
    }
}

void MainWindow::fileCloseAction()
{
    if (fileSave(true))
        fileClose();
}

void MainWindow::fileOpenAction()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), _lastFileDir, tr(
        "Sprint-Layout PCB Export (*.plt *.drl);;"
        "Sprint-Layout HP-GL (*.plt);;"
        "Sprint-Layout Excellon (*.drl);;"
        "All Files (*.*)"));

    if (fileName.isEmpty())
        return;

    if (fileSave(true))
        fileOpen(fileName);
}

void MainWindow::fileReloadAction()
{
    // WARNING: It's important to make a copy of the string with current file name
    QString fileName = _inputFilePath;

    if (fileSave(true))
        fileOpen(fileName);
}

void MainWindow::fileSaveAction()
{
    fileSave(false);
}

void MainWindow::fileSaveAsAction()
{
    fileSave(false, true);
}

void MainWindow::generate()
{
    _log.remove(tr("[Program]"));

    _editProgram->clear();

    if (!_parser)
        return;

    _operationCanceled = false;
    connect(_progress, SIGNAL(canceled()), this, SLOT(cancelOperation()));

    if (_parser->type() == AbstractParser::ParserDrilling)
    {
        operationStarted(tr("Creating Drilling Program"));
        generateDrilling();
        operationFinished();
    }
    else if (_parser->type() == AbstractParser::ParserMillling)
    {
        operationStarted(tr("Creating Millling Program"));
        generateMilling();
        operationFinished();
    }
    else
    {
        return;
    }

    if (_operationCanceled)
    {
        _log.warning(tr("Building the program has been canceled."), tr("[Program]"));
    }
    else
    {
        _log.accept(tr("The program has been successfully built."), tr("[Program]"));
    }

    _tabs->setCurrentWidget(_tabProgram);

    updateProjectState(true);
    setScriptIcon(ScriptGreen);
}

void MainWindow::settingsOpen()
{
    if (_tabs->widget(0) != _tabSettings)
        _tabs->insertTab(0, _tabSettings, _tabSettings->windowIcon(), _tabSettings->windowTitle());

    _tabs->setCurrentIndex(0);
}

void MainWindow::settingsClose()
{
    _tabs->removeTab(_tabs->indexOf(_tabSettings));
}

void MainWindow::layoutReset()
{
    restoreState(_defaultState);
    _actionLogErrors->setChecked(true);
    _actionLogWarnings->setChecked(true);
    _actionLogNotices->setChecked(true);
}

void MainWindow::showAboutDialog()
{
    AboutDialog* dialog = new AboutDialog(this);

    if (dialog)
        dialog->open();
}

void MainWindow::operationStarted(const QString& operation)
{
    _progress->setText(operation);
    _progress->setValue(0);
    _progress->show();
    QApplication::processEvents();
}

void MainWindow::operationProgress(int done, int total)
{
    _progress->setProgress(done, total);
    QApplication::processEvents();
}

void MainWindow::operationFinished()
{
    _progress->hide();
    QApplication::processEvents();
}

void MainWindow::cancelOperation()
{
    _operationCanceled = true;
}

void MainWindow::fileClose()
{
    // File State
    _inputFilePath.clear();
    _inputFileName.clear();
    _currentFileName.clear();
    _currentFilePath.clear();

    updateProjectState(false);
    setScriptIcon(ScriptPlain);

    if (_parser)
    {
        delete _parser;
        _parser = nullptr;
    }

    // Clear Log
    _log.clear();

    // Clear Program
    _editProgram->clear();

    // CNC Options
    _dockMilling->setDisabled(true);
    _dockDrilling->setDisabled(true);

    // Actions
    _actionReload->setDisabled(true);
    _actionClose->setDisabled(true);
    _actionGenerate->setDisabled(true);

    _statusBar->showMessage(tr("Ready"));

    _tabs->setCurrentWidget(_tabLog);
}

void MainWindow::fileOpen(const QString& fileName)
{
    const QString errorHeader = tr("StepCAM cannot open the file");
    const QString errorReason = tr("The file format or file extension is not valid. "
        "Verify that the file has not been corrupted and that the file extension "
        "matches the format of the file.");

    if (fileName.isEmpty())
        return;

    fileClose();

    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    _inputFileName = fileInfo.fileName();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Cannot open the file
        QMessageBox::critical(this, QApplication::applicationName(), QString("%1<br>%2.<br><br>%3")
            .arg(errorHeader, fileName, file.errorString()), QMessageBox::Ok, QMessageBox::Ok);

        _log.error(QString("%1.\n%2").arg(errorHeader, file.errorString()), _inputFileName);

        return;
    }

    QString extension = fileInfo.suffix().toLower();

    if (fileParse(file, extension))
    {
        _inputFilePath = fileName;
        _currentFileName = tr("Untitled");
        _currentFilePath.clear();
        _lastFileDir = fileInfo.path();

        updateProjectState(false);
        setScriptIcon(ScriptLightning);

        _actionReload->setEnabled(true);
        _actionClose->setEnabled(true);
        _actionGenerate->setEnabled(true);
    }
    else
    {
        if (_parser && _parser->isInterrupted())
        {
            _log.warning(tr("The file parsing was interrupted."), fileInfo.fileName());
            delete _parser;
            _parser = nullptr;
            return;
        }

        QMessageBox::critical(this, QApplication::applicationName(), QString("%1<br>%2.<br><br>%3")
            .arg(errorHeader, fileName, errorReason), QMessageBox::Ok, QMessageBox::Ok);

        _log.error(QString("%1.\n%2").arg(errorHeader, errorReason), fileInfo.fileName());
    }
}

bool MainWindow::fileSave(bool final, bool relocate)
{
    if (!_actionSave->isEnabled() && !relocate)
        return true;

    if (final && !relocate)
    {
        auto button = QMessageBox::question(this, QApplication::applicationName(),
            tr("Do you want to save changes to %1?").arg(_currentFileName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        switch (button)
        {
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
        default:
            break;
        }
    }

    QString fileName = relocate ? QString() : _currentFilePath;

    while (true)
    {
        if (fileName.isEmpty())
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Save As"), QString(),
                tr("G-code (*.ngc);;All Files (*.*)"));
        }

        if (fileName.isEmpty())
            break;

        QFile file(fileName);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            file.write(_editProgram->toPlainText().toUtf8());
            file.close();

            _currentFilePath = fileName;
            _currentFileName = QFileInfo(_currentFilePath).completeBaseName();
            updateProjectState(false);

            return true;
        }
        else
        {
            fileName.clear();
        }
    }

    return false;
}

bool MainWindow::fileParse(QFile& file, const QString& extension)
{
    QDockWidget* dockWidget = nullptr;

    if (extension == "drl")
    {
        _parser = new ExcellonParser(this);
        dockWidget = _dockDrilling;
    }
    else if (extension == "plt")
    {
        _parser = new HpglParser(this);
        dockWidget = _dockMilling;
    }
    else
    {
        return false;
    }

    if (!_parser)
        return false;

    connect(_progress, SIGNAL(canceled()), _parser, SLOT(interrupt()));
    connect(_parser, SIGNAL(started(const QString&)),
        this, SLOT(operationStarted(const QString&)));
    connect(_parser, SIGNAL(progress(int, int)),
        this, SLOT(operationProgress(int, int)));
    connect(_parser, SIGNAL(finished()),
        this, SLOT(operationFinished()));

    connect(_parser, SIGNAL(log(int, const QString&, const QString&)), this,
        SLOT(logParser(int, const QString&, const QString&)));

    bool result = _parser->parse(file);
    if (result && dockWidget)
        dockWidget->setEnabled(true);

    _progress->hide();
    return result;
}

void MainWindow::generateDrilling()
{
    _editProgram->append(_editSettingsDrillingPrologue->toPlainText());

    if (!_checkDrillingSingleTool->isChecked())
    {
        foreach (AbstractTool tool, _parser->tools())
        {
            if (tool.id() > 0)
            {
                _editProgram->append(QString("( Drill Bit #%1 / %2 mm )")
                    .arg(tool.id()).arg(Utilities::coordinateToString(tool.diameter())));
            }
        }
    }

    QString feedRate = QString::number(_editDrillingFeedRate->value());
    QString spindleSpeed = QString::number(_editDrillingSpindleSpeed->value());

    QString safeZ = Utilities::doubleToString(_editDrillingSafeZ->value(), 3);
    QString depth = Utilities::doubleToString(_editDrillingDepth->value(), 3);
    QString startHeight = Utilities::doubleToString(_editDrillingStartHeight->value(), 3);
    QString toolHeight = Utilities::doubleToString(_editDrillingTcHeight->value(), 3);

    int toolNumber = 0;

    _editProgram->append(QString("G0 Z").append(safeZ));
    _editProgram->append(QString("G1 F").append(feedRate));

    if (_checkDrillingSingleTool->isChecked())
        _editProgram->append(QString("M3 S").append(spindleSpeed));

    int total = _parser->curves().count();
    for (int i = 0; i < total; ++i)
    {
        const AbstractCurve& point = _parser->curves()[i];
        operationProgress(i, total);

        if (_operationCanceled)
            return;

        if (!_checkDrillingSingleTool->isChecked() && point.tool() != toolNumber)
        {
            QString tool = QString::number(point.tool());
            QString diameter =
                Utilities::coordinateToString(_parser->tools()[point.tool()].diameter());

            // Tool Change
            _editProgram->append("M5");

            _editProgram->append(QString("( Tool Change T%1 / %2 mm )").arg(tool, diameter));
            if (_checkDrillingTcHeight->isChecked())
            {
                _editProgram->append(QString("G0 Z").append(toolHeight));
            }
            _editProgram->append(QString("M6 T").append(tool));
            _editProgram->append(QString("G1 F").append(feedRate));
            _editProgram->append(QString("M3 S").append(spindleSpeed));
            toolNumber = point.tool();
        }

        QString x;
        QString y;

        if (point.count() > 0)
        {
            x = Utilities::coordinateToString(point.x()[0]);
            y = Utilities::coordinateToString(point.y()[0]);
        }

        _editProgram->append(QString("G0 X%1 Y%2").arg(x, y));
        _editProgram->append(QString("G0 Z").append(startHeight));
        _editProgram->append(QString("G1 Z").append(depth));
        _editProgram->append(QString("G0 Z").append(safeZ));
    }

    _editProgram->append(_editSettingsDrillingEpilogue->toPlainText());
}

void MainWindow::generateMilling()
{
    _editProgram->append(_editSettingsMillingPrologue->toPlainText());

    QString feedRate = QString::number(_editMillingFeedRate->value());
    QString plungeRate = QString::number(_editMillingPlungeRate->value());
    QString spindleSpeed = QString::number(_editMillingSpindleSpeed->value());

    QString safeZ = Utilities::doubleToString(_editMillingSafeZ->value(), 3);
    QString depth = Utilities::doubleToString(_editMillingDepth->value(), 3);

    _editProgram->append(QString("G0 Z").append(safeZ));
    _editProgram->append(QString("M3 S").append(spindleSpeed));

    int total = _parser->curves().count();
    for (int i = 0; i < total; ++i)
    {
        const AbstractCurve& curve = _parser->curves()[i];
        operationProgress(i, total);

        if (_operationCanceled)
            return;

        if (curve.type() == AbstractCurve::CurveTypeNone)
            continue;

        for (int i = 0; i < curve.count(); ++i)
        {
            QString x = Utilities::coordinateToString(curve.x()[i]);
            QString y = Utilities::coordinateToString(curve.y()[i]);

            if (i == 0)
            {
                _editProgram->append(QString("G0 X%1 Y%2").arg(x, y));
                _editProgram->append(QString("G1 Z%1 F%2").arg(depth, plungeRate));
                _editProgram->append(QString("G1 F").append(feedRate));
            }
            else
            {
                _editProgram->append(QString("G1 X%1 Y%2").arg(x, y));
            }
        }

        _editProgram->append(QString("G0 Z").append(safeZ));
    }

    _editProgram->append(_editSettingsMillingEpilogue->toPlainText());
}

void MainWindow::setScriptIcon(int icon)
{
    switch (icon)
    {
    case ScriptLightning:
        _tabs->setTabIcon(_tabs->indexOf(_tabProgram), _iconScriptLightning);
        break;
    case ScriptGreen:
        _tabs->setTabIcon(_tabs->indexOf(_tabProgram), _iconScriptGreen);
        break;
    case ScriptYellow:
        _tabs->setTabIcon(_tabs->indexOf(_tabProgram), _iconScriptYellow);
        break;
    case ScriptRed:
        _tabs->setTabIcon(_tabs->indexOf(_tabProgram), _iconScriptRed);
        break;
    default:
        _tabs->setTabIcon(_tabs->indexOf(_tabProgram), _iconScript);
        break;
    }
}
