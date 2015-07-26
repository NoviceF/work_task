﻿#include <cassert>

#include <QMessageBox>

#include "controller.h"


Controller::Controller(QObject* parent):
    QObject(parent),
    running_(false)
{
}

void Controller::RunThread(IProgressWorker* worker)
{
//    qDebug() << "init thread";
    if (running_)
        RemoveThread();

    worker->moveToThread(&workerThread_);

    connect(worker, SIGNAL(error(QString)), this, SLOT(onError(QString)));

    connect(&workerThread_, SIGNAL(started()), worker, SLOT(onStart()));

    connect(worker, SIGNAL(finished()), &workerThread_, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));

    connect(this, SIGNAL(abort()), worker, SLOT(onAbort()));
    connect(worker, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(worker, SIGNAL(finished()), this, SLOT(onWorkDone()));

    workerThread_.start();
    running_ = true;
    //    qDebug() << "thread start";
}

void Controller::RemoveThread()
{
    //    qDebug() << "remove thread";
    workerThread_.quit();
    workerThread_.wait();
    running_ = false;
}

void Controller::RiseRunningThreadWarningMsg()
{
//        qDebug() << "thread already running";
    QMessageBox msgBox;
    connect(this, SIGNAL(closeMsgBox()), &msgBox, SLOT(close()));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Previous operation in progress now..");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.exec();
}

void Controller::SetProgBar(QProgressBar* progBar)
{
    progBar_ = progBar;

    if (!progBar_)
    {
        throw std::runtime_error("DirTreeBuilder::BuildDirTree: Progress bar "
             "must be set before use.");
    }
}

QProgressBar* Controller::GetProgBar()
{
    assert(progBar_);
    return progBar_;
}

void Controller::SetLabel(QLabel* label)
{
    label_ = label;

    if (!label_)
    {
        throw std::runtime_error("DirTreeBuilder::BuildDirTree: Label "
             "must be set before use.");
    }
}

QLabel* Controller::GetLabel()
{
    assert(label_);
    return label_;
}

/*virtual*/ void Controller::SetView(QAbstractItemView*)
{
    throw std::runtime_error("Controller::SetView: Not implemented.");
}

/*virtual*/ QAbstractItemView* Controller::GetView()
{
    throw std::runtime_error("Controller::GetView: Not implemented.");
}


void Controller::RiseErrorMsg(const QString& msg)
{
//        qDebug() << "thread already running";
    QMessageBox msgBox;
    connect(this, SIGNAL(closeMsgBox()), &msgBox, SLOT(close()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("Error occured..");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    switch (ret)
    {
        case QMessageBox::Ok:
            // Save was clicked
            break;
        default:
            assert(false);
            // should never be reached
            break;
    }
}

void Controller::onError(const QString &errorMsg)
{
    RemoveThread();
    RiseErrorMsg(errorMsg);
}

void Controller::onWorkDone()
{
    RemoveThread();
    emit closeMsgBox();
}
