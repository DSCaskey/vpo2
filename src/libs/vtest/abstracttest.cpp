/************************************************************************
 **
 **  @file   abstracttest.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "abstracttest.h"

#include <qtestcase.h>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFlags>
#include <QIODevice>
#include <QPointF>
#include <QProcess>
#include <QScopedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

#include "logging.h"
#include "vsysexits.h"
#include "../vgeometry/vgobject.h"

//---------------------------------------------------------------------------------------------------------------------
AbstractTest::AbstractTest(QObject *parent) :
    QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::Comparison(const QVector<QPointF> &ekv, const QVector<QPointF> &ekvOrig) const
{
    // Begin comparison
    QCOMPARE(ekv.size(), ekvOrig.size());// First check if sizes equal

    for (int i=0; i < ekv.size(); i++)
    {
        const QPointF p1 = ekv.at(i);
        const QPointF p2 = ekvOrig.at(i);
        const QString msg = QString("Index: %1. Got '%2;%3', Expected '%4;%5'.")
                .arg(i).arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y());
        // Check each point. Don't use comparison float values
        QVERIFY2((qAbs(p1.x() - p2.x()) <= VGObject::accuracyPointOnLine)
                 && (qAbs(p1.y() - p2.y()) <= VGObject::accuracyPointOnLine), qUtf8Printable(msg));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString AbstractTest::ValentinaPath() const
{
    const QString path = QStringLiteral("/../../../app/valentina/bin/valentina");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + path + QLatin1String(".exe");
#else
    return QCoreApplication::applicationDirPath() + path;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
QString AbstractTest::TapePath() const
{
    const QString path = QStringLiteral("/../../../app/tape/bin/tape");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + path + QLatin1String(".exe");
#else
    return QCoreApplication::applicationDirPath() + path;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
QString AbstractTest::TranslationsPath() const
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/../../../app/valentina/bin/translations");
}

//---------------------------------------------------------------------------------------------------------------------
int AbstractTest::Run(int exit, const QString &program, const QStringList &arguments, QString &error, int msecs)
{
    const QString parameters = QString("Program: %1 \nArguments: %2.").arg(program).arg(arguments.join(", "));

    QFileInfo info(program);
    if (not info.exists())
    {
        error = QString("Can't find binary.\n%1").arg(parameters);
        return TST_EX_BIN;
    }

    QScopedPointer<QProcess> process(new QProcess());
    process->setWorkingDirectory(info.absoluteDir().absolutePath());
    process->start(program, arguments);

    if (not process->waitForStarted(msecs))
    {
        error = QString("The start operation timed out or an error occurred.\n%1").arg(parameters);
        process->kill();
        return TST_EX_START_TIME_OUT;
    }

    if (not process->waitForFinished(msecs))
    {
        error = QString("The finish operation timed out or an error occurred.\n%1").arg(parameters);
        process->kill();
        return TST_EX_FINISH_TIME_OUT;
    }

    if (process->exitStatus() == QProcess::CrashExit)
    {
        error = QString("Program crashed.\n%1\n%2").arg(parameters).arg(QString(process->readAllStandardError()));
        return TST_EX_CRASH;
    }

    if (process->exitCode() != exit)
    {
        error = QString("Unexpected finish.\n%1").arg(QString(process->readAllStandardError()));
        return process->exitCode();
    }

    return process->exitCode();
}

//---------------------------------------------------------------------------------------------------------------------
bool AbstractTest::CopyRecursively(const QString &srcFilePath, const QString &tgtFilePath) const
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir())
    {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        const QString dirName = QFileInfo(tgtFilePath).fileName();
        if (not targetDir.mkdir(dirName))
        {
            const QString msg = QString("Can't create dir '%1'.").arg(dirName);
            QWARN(qUtf8Printable(msg));
            return false;
        }
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden |
                                                    QDir::System);
        foreach (const QString &fileName, fileNames)
        {
            const QString newSrcFilePath = srcFilePath + QDir::separator() + fileName;
            const QString newTgtFilePath = tgtFilePath + QDir::separator() + fileName;
            if (not CopyRecursively(newSrcFilePath, newTgtFilePath))
            {
                return false;
            }
        }
    }
    else
    {
        if (QFileInfo(tgtFilePath).exists())
        {
            const QString msg = QString("File '%1' exists.").arg(srcFilePath);
            QWARN(qUtf8Printable(msg));

            if (QFile::remove(tgtFilePath))
            {
                QWARN("File successfully removed.");
            }
            else
            {
                QWARN("Can't remove file.");
                return false;
            }
        }

        // Check error: Cannot open %file for input
        QFile srcFile(srcFilePath);
        if (not srcFile.open(QFile::ReadOnly))
        {
            const QString msg = QString("Can't copy file '%1'. Error: %2").arg(srcFilePath).arg(srcFile.errorString());
            QWARN(qUtf8Printable(msg));
            return false;
        }
        srcFile.close();

        if (not srcFile.copy(tgtFilePath))
        {
            const QString msg = QString("Can't copy file '%1' to '%2'. Error: %3").arg(srcFilePath).arg(tgtFilePath)
                    .arg(srcFile.errorString());
            QWARN(qUtf8Printable(msg));
            return false;
        }
    }
    return true;
}
