﻿#include <cassert>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QString>
#include <QWidget>

#include <unistd.h>

#include "statgetter.h"

StatGetterThread::StatGetterThread(const QString& path, QProgressBar* progBar,
    QLabel* label, QTableWidget* statTable, QObject* parent) :
    IProgressWorker(progBar, label, parent),
    path_(path),
    statTable_(statTable)
{
    if (path_.isEmpty())
    {
        throw std::runtime_error("StatGetterThread::StatGetterThread: Path "
                                 "was not setted.");
    }
}

size_t StatGetterThread::GetTotalFilesCount() const
{
    size_t totalCount = 0;

    for (const auto& pair : statTree_)
    {
        totalCount += pair.second.count;
    }

    return totalCount;
}

size_t StatGetterThread::GetTotalFilesSize() const
{
    size_t totalSize = 0;

    for (const auto& pair : statTree_)
    {
        totalSize += pair.second.size;
    }

    return totalSize;
}

size_t StatGetterThread::GetAvgSizeAllFiles() const
{
    return GetTotalFilesSize() / GetTotalFilesCount();

}

size_t StatGetterThread::GetTotalGroupFilesCount(const QString& groupName) const
{
    auto it = statTree_.find(groupName);

    if (it != statTree_.end())
        return it->second.count;

    return 0;
}

size_t StatGetterThread::GetTotalGroupFilesSize(const QString& groupName) const
{
    auto it = statTree_.find(groupName);

    if (it != statTree_.end())
        return it->second.size;

    return 0;
}

size_t StatGetterThread::GetAvgGroupFilesSize(const QString& groupName) const
{
    return GetTotalGroupFilesSize(groupName) / GetTotalGroupFilesCount(groupName);
}

size_t StatGetterThread::GetSubdirsCount()
{
    QDir rootDir(path_);
    rootDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    return rootDir.count();
}

void StatGetterThread::FillPreAnalysisTree()
{
    assert(!path_.isEmpty());

    QDirIterator it(path_, QStringList() << "*", QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QCoreApplication::processEvents();
        QFileInfo fileInfo(it.next());
        preAnalysisTree_[fileInfo.suffix()].push_back(fileInfo);
    }
}

void StatGetterThread::FillStatTreeByPath()
{
    int counter = 0;

    for (auto pair : preAnalysisTree_)
    {
        auto groupName = pair.first;
        auto groupStats = pair.second;

        const size_t elementsCount(GetTotalGroupFilesCount(groupStats));
        const size_t groupSize(GetTotalGroupFilesSize(groupStats));
        GroupStats stats = {elementsCount, groupSize};

        statTree_.insert(std::make_pair(groupName, stats));

        ++counter;
        emit setProgressValue(counter);
    }

}

void StatGetterThread::FillTable()
{
    statTable_->setRowCount(5);
    statTable_->setColumnCount(2);
//    statTable_->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    statTable_->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");
}

/*static*/ size_t StatGetterThread::GetTotalGroupFilesCount(
        const StatGetterThread::infovec_t& infoList)
{
    return infoList.size();
}

/*static*/ size_t StatGetterThread::GetTotalGroupFilesSize(
        const StatGetterThread::infovec_t& infoList)
{
    size_t sum = 0;

    for (const QFileInfo& fileInfo : infoList)
    {
        sum += fileInfo.size();
    }

    return sum;
}

void StatGetterThread::onStart()
{
    setLabel("Retrieving files count..");
    emit showLabel();
    FillPreAnalysisTree();
    const size_t totalValue = preAnalysisTree_.size();

    setLabel("Calculating statistics..");
    setProgressRange(0, totalValue);

    emit setProgressValue(0);
    emit showProgressBar();

    FillStatTreeByPath();
    FillTable();

    emit setProgressValue(totalValue);

    emit hideLabel();
    emit hideProgressBar();

    emit finished();
}

///
/// \brief StatGetter
///
StatGetter::StatGetter(QObject* parent) :
    Controller(parent)
{
}

void StatGetter::GetStatsForPath(const QString& rootPath)
{
    pathInWork_ = rootPath;
    assert(!pathInWork_.isEmpty());
    // TODO: проверить отменяемость потока
    if (IsRunning())
    {
        RiseRunningThreadWarningMsg();
        return;
    }

    StatGetterThread* statGetterThread =
            new StatGetterThread(rootPath, GetProgBar(), GetLabel(), tableWidget_);

    RunThread(statGetterThread);
}

void StatGetter::SetView(QTableWidget* view)
{
    if (!view)
        throw std::runtime_error("StatGetter::SetView: view is null.");

    tableWidget_ = view;
}

void StatGetter::onError(const QString& errorMsg)
{
    Controller::onError(errorMsg);
}

void StatGetter::onWorkDone()
{
    // do work
    Controller::onWorkDone();
}

