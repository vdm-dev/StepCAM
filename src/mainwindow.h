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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "ui_mainwindow.h"

#include "logtablemodel.h"
#include "abstractparser.h"
#include "progressstatuswidget.h"


class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    void loadExternalFile(const QString& fileName);

protected:
    virtual void closeEvent(QCloseEvent* event);

private slots:
    void loadSettings();
    void saveSettings();
    void logParser(int severity, const QString& description, const QString& line);
    void logUpdated(int errors, int warnings, int notices, int accepts);
    void updateProjectState(bool modified);
    void handleEditActions();
    void fileCloseAction();
    void fileOpenAction();
    void fileReloadAction();
    void fileSaveAction();
    void fileSaveAsAction();
    void generate();
    void settingsOpen();
    void settingsClose();
    void layoutReset();
    void showAboutDialog();
    void operationStarted(const QString& operation);
    void operationProgress(int done, int total);
    void operationFinished();
    void cancelOperation();

private:
    void fileClose();
    void fileOpen(const QString& fileName);
    bool fileSave(bool final, bool relocate = false);
    bool fileParse(QFile& file, const QString& extension);
    void generateDrilling();
    void generateMilling();
    void setScriptIcon(int icon);

private:
    enum Script
    {
        ScriptPlain = 0,
        ScriptLightning,
        ScriptGreen,
        ScriptYellow,
        ScriptRed
    };

    QIcon _iconLog;
    QIcon _iconLogOk;
    QIcon _iconLogError;
    QIcon _iconLogWarning;

    QIcon _iconScript;
    QIcon _iconScriptLightning;
    QIcon _iconScriptGreen;
    QIcon _iconScriptYellow;
    QIcon _iconScriptRed;

    QByteArray _defaultState;

    QString _inputFilePath;
    QString _inputFileName;
    QString _currentFileName;
    QString _currentFilePath;
    QString _lastFileDir;

    LogTableModel _log;

    AbstractParser* _parser;

    ProgressStatusWidget* _progress;
    bool _operationCanceled;
};


inline void MainWindow::loadExternalFile(const QString& fileName)
{
    fileOpen(fileName);
}


#endif // MAINWINDOW_H
