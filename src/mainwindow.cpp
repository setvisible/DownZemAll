/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "about.h"
#include "version.h"
#include "globals.h"

#include <Core/IDownloadItem>
#include <Core/DownloadManager>
#include <Core/FileAccessManager>
#include <Core/Settings>
#include <Dialogs/AddDownloadDialog>
#include <Dialogs/CompilerDialog>
#include <Dialogs/InformationDialog>
#include <Dialogs/PreferenceDialog>
#include <Dialogs/WizardDialog>
#include <Ipc/InterProcessCommunication>
#include <Io/FileReader>
#include <Io/FileWriter>
#include <Widgets/DownloadQueueView>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_downloadManager(new DownloadManager(this))
  , m_fileAccessManager(new FileAccessManager(this))
  , m_settings(new Settings(this))
  , m_statusBarLabel(new QLabel(this))
{
    ui->setupUi(this);

    m_downloadManager->setSettings(m_settings);

    this->setWindowIcon(QIcon(":/icons/logo/maps-pin-place.ico"));
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setAcceptDrops(true);
#ifdef Q_OS_OSX
    this->setUnifiedTitleAndToolBarOnMac(true);
#endif

    /* Connect the GUI to the DownloadManager. */
    ui->downloadQueueView->setEngine(m_downloadManager);

    /* Connect the SceneManager to the MainWindow. */
    /* The SceneManager centralizes the changes. */
    QObject::connect(m_downloadManager, SIGNAL(jobAppended(DownloadRange)),
                     this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    QObject::connect(m_downloadManager, SIGNAL(jobRemoved(DownloadRange)),
                     this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    QObject::connect(m_downloadManager, SIGNAL(jobStateChanged(IDownloadItem*)),
                     this, SLOT(onJobStateChanged(IDownloadItem*)));
    QObject::connect(m_downloadManager, SIGNAL(selectionChanged()),
                     this, SLOT(onSelectionChanged()));


    connect(ui->downloadQueueView, SIGNAL(doubleClicked(IDownloadItem*)),
            this, SLOT(openFile(IDownloadItem*)));


    /* File Access Manager */
    m_fileAccessManager->setSettings(m_settings);

    /* Connect the rest of the GUI widgets together (selection, focus, etc.) */
    createActions();
    createContextMenu();
    createStatusbar();

    readSettings();

    refreshTitleAndStatus();
    refreshMenus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::createActions()
{
    //! [0] File
    connect(ui->actionImportWizard, SIGNAL(triggered()), this, SLOT(openWizard()));
    // --
    connect(ui->actionImportFromFile, SIGNAL(triggered()), this, SLOT(importFromFile()));
    connect(ui->actionExportSelectedToFile, SIGNAL(triggered()), this, SLOT(exportSelectedToFile()));
    // --
    ui->actionExit->setShortcuts(QKeySequence::Quit);
    ui->actionExit->setStatusTip(tr("Quit %0").arg(STR_APPLICATION_NAME));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close())); //&QWidget::close
    //! [0]

    //! [1] Edit
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(ui->actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    connect(ui->actionSelectCompleted, SIGNAL(triggered()), this, SLOT(selectCompleted()));
    // --
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    // --
    connect(ui->actionManageMirrors, SIGNAL(triggered()), this, SLOT(manageMirrors()));
    connect(ui->actionOneMoreSegment, SIGNAL(triggered()), this, SLOT(oneMoreSegment()));
    connect(ui->actionOneFewerSegment, SIGNAL(triggered()), this, SLOT(oneFewerSegment()));
    //! [1]

    //! [2] View
    connect(ui->actionInformation, SIGNAL(triggered()), this, SLOT(showInformation()));
    // --
    connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionRenameFile, SIGNAL(triggered()), this, SLOT(renameFile()));
    connect(ui->actionDeleteFile, SIGNAL(triggered()), this, SLOT(deleteFile()));
    connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
    // --
    connect(ui->actionRemoveCompletedDownloads, SIGNAL(triggered()), this, SLOT(removeCompletedDownloads()));
    connect(ui->actionRemoveAll1, SIGNAL(triggered()), this, SLOT(removeAll()));
    connect(ui->actionCleanGoneFiles, SIGNAL(triggered()), this, SLOT(cleanGoneFiles()));
    // --
    connect(ui->actionRemoveDownloads, SIGNAL(triggered()), this, SLOT(removeDownloads()));
    connect(ui->actionRemoveSelected, SIGNAL(triggered()), this, SLOT(removeSelected()));
    connect(ui->actionRemoveDuplicates, SIGNAL(triggered()), this, SLOT(removeDuplicates()));
    connect(ui->actionRemoveAll2, SIGNAL(triggered()), this, SLOT(removeAll()));
    connect(ui->actionRemoveFailed, SIGNAL(triggered()), this, SLOT(removeFailed()));
    connect(ui->actionRemovePaused, SIGNAL(triggered()), this, SLOT(removePaused()));
    //! [2]

    //! [3] Download
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(add()));
    //--
    connect(ui->actionResume, SIGNAL(triggered()), this, SLOT(resume()));
    connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pause()));
    connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(cancel()));
    //--
    connect(ui->actionUp, SIGNAL(triggered()), this, SLOT(up()));
    connect(ui->actionTop, SIGNAL(triggered()), this, SLOT(top()));
    connect(ui->actionDown, SIGNAL(triggered()), this, SLOT(down()));
    connect(ui->actionBottom, SIGNAL(triggered()), this, SLOT(bottom()));
    //! [3]

    //! [4]  Options
    connect(ui->actionSpeedLimit, SIGNAL(triggered()), this, SLOT(speedLimit()));
    connect(ui->actionAddDomainSpecificLimit, SIGNAL(triggered()), this, SLOT(addDomainSpecificLimit()));
    //--
    connect(ui->actionForceStart, SIGNAL(triggered()), this, SLOT(forceStart()));
    //--
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
    //! [4]

    //! [5] Help
    ui->actionAbout->setShortcuts(QKeySequence::HelpContents);
    ui->actionAbout->setStatusTip(tr("About %0").arg(STR_APPLICATION_NAME));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    ui->actionAboutQt->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1));
    ui->actionAboutQt->setStatusTip(tr("About Qt"));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(ui->actionAboutCompiler, SIGNAL(triggered()), this, SLOT(aboutCompiler()));
    //! [5]
}

void MainWindow::createContextMenu()
{
    QMenu *contextMenu = new QMenu(this);

    contextMenu->addAction(ui->actionInformation);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionOpenFile);
    contextMenu->addAction(ui->actionRenameFile);
    contextMenu->addAction(ui->actionDeleteFile);
    contextMenu->addAction(ui->actionOpenDirectory);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionCopy);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionResume);
    contextMenu->addAction(ui->actionPause);
    contextMenu->addAction(ui->actionCancel);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionRemoveCompletedDownloads);
    contextMenu->addAction(ui->actionRemoveDownloads);

    QMenu *remove = contextMenu->addMenu(tr("Other"));
    remove->addAction(ui->actionRemoveAll1);
    remove->addSeparator();
    remove->addAction(ui->actionCleanGoneFiles);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveSelected);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveDuplicates);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveFailed);
    remove->addAction(ui->actionRemovePaused);

    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionSelectAll);
    contextMenu->addAction(ui->actionInvertSelection);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionTop);
    contextMenu->addAction(ui->actionUp);
    contextMenu->addAction(ui->actionDown);
    contextMenu->addAction(ui->actionBottom);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionSpeedLimit);

    QMenu *advanced = contextMenu->addMenu(tr("Advanced"));
    advanced->addAction(ui->actionOneMoreSegment);
    advanced->addAction(ui->actionOneFewerSegment);
    advanced->addSeparator();
    advanced->addAction(ui->actionManageMirrors);
    advanced->addSeparator();
    advanced->addAction(ui->actionForceStart);
    advanced->addSeparator();
    advanced->addAction(ui->actionImportFromFile);
    advanced->addAction(ui->actionExportSelectedToFile);
    advanced->addAction(ui->actionAddDomainSpecificLimit);

    ui->downloadQueueView->setContextMenu(contextMenu);
}

void MainWindow::createStatusbar()
{
    this->statusBar()->addPermanentWidget(m_statusBarLabel);
    this->statusBar()->addAction(ui->actionPreferences);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::openWizard()
{
    // ask for the Url
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Website URL"));
    dialog.setLabelText(tr("Url:"));
    dialog.setTextValue(urlFromClipboard().toString());
    dialog.setOkButtonText(tr("Start!"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);
    dialog.resize(600,400);

    const int ret = dialog.exec();
    const QUrl url = QUrl(dialog.textValue());
    if (ret && !url.isEmpty()) {
        openWizard(url);
    }
}

void MainWindow::openWizard(const QUrl &url)
{
    WizardDialog dialog(m_downloadManager, m_settings, this);
    dialog.loadUrl(url);
    dialog.exec();
}

void MainWindow::openWizard(const QString &message)
{
    WizardDialog dialog(m_downloadManager, m_settings, this);
    dialog.loadResources(message);
    dialog.exec();
}

void MainWindow::handleMessage(const QString &message)
{
    qDebug() << Q_FUNC_INFO << message;
    const QString cleaned = InterProcessCommunication::clean(message);
    if (!cleaned.isEmpty()) {

        if (InterProcessCommunication::isUrl(cleaned)) {
            // Assume it's an unique URL, so a HTML page.
            openWizard(QUrl(cleaned));

        } else if(InterProcessCommunication::isCommandOpenUrl(cleaned)) {
            const QUrl url = InterProcessCommunication::getCurrentUrl(cleaned);
            openWizard(url);

        } else if(InterProcessCommunication::isCommandDownloadLink(cleaned)) {
            const QUrl url = InterProcessCommunication::getDownloadLink(cleaned);
            AddDownloadDialog::quickDownload(url, m_downloadManager);

        } else if(InterProcessCommunication::isCommandOpenManager(cleaned)) {
            // Do nothing

        } else if(InterProcessCommunication::isCommandShowPreferences(cleaned)) {
            showPreferences();

        } else {
            // Otherwise, assume it's a list of resources.
            openWizard(cleaned);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::importFromFile()
{
    const QString filePath = askOpenFileName(FileReader::supportedFormats());
    if (!filePath.isEmpty()) {
        setWorkingDirectory(filePath);
        loadFile(filePath);
    }
}

void MainWindow::exportSelectedToFile()
{
    const QString filePath = askSaveFileName(FileWriter::supportedFormats());
    if (!filePath.isEmpty()) {
        setWorkingDirectory(filePath);
        saveFile(filePath);
    }
}

void MainWindow::selectAll()
{
    m_downloadManager->setSelection(m_downloadManager->downloadItems());
}

void MainWindow::selectNone()
{
    m_downloadManager->clearSelection();
}

void MainWindow::invertSelection()
{
    QList<IDownloadItem *> inverted;
    foreach (auto item, m_downloadManager->downloadItems()) {
        if (!m_downloadManager->isSelected(item)) {
            inverted.append(item);
        }
    }
    m_downloadManager->setSelection(inverted);
}

void MainWindow::selectCompleted()
{
    m_downloadManager->setSelection(m_downloadManager->completedJobs());
}

void MainWindow::copy()
{
    const QString text = m_downloadManager->selectionToClipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void MainWindow::manageMirrors()
{    
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::oneMoreSegment()
{
    m_downloadManager->oneMoreSegment();
}

void MainWindow::oneFewerSegment()
{
    m_downloadManager->oneFewerSegment();
}

void MainWindow::showInformation()
{
    if (!m_downloadManager->selection().isEmpty()) {
        InformationDialog dialog(m_downloadManager->selection(), this);
        dialog.exec();
    }
}

void MainWindow::openFile()
{
    if (!m_downloadManager->selection().isEmpty()) {
        auto item = m_downloadManager->selection().first();
        if (item->state() == IDownloadItem::Completed) {
            openFile(item);
            return;
        }
    }
}

void MainWindow::openFile(IDownloadItem *downloadItem)
{
    auto url = downloadItem->localFileUrl();
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::information(
                    this, tr("Error"),
                    tr("File not found:\n\n%0")
                    .arg(url.toLocalFile()));
    }
}

void MainWindow::renameFile()
{
    ui->downloadQueueView->rename();
}

void MainWindow::deleteFile()
{
    if (!m_downloadManager->selection().isEmpty()) {
        QString text = m_downloadManager->selectionToString();

        QMessageBox msgbox(this);
        msgbox.setWindowTitle(tr("Remove Downloads"));
        msgbox.setText(tr("Are you sure to remove %0 downloads?").arg(text));
        msgbox.setIcon(QMessageBox::Icon::Question);
        QAbstractButton *deleteButton = msgbox.addButton(tr("Delete"), QMessageBox::ActionRole);
        msgbox.addButton(QMessageBox::Cancel);
        msgbox.setDefaultButton(QMessageBox::Cancel);

        msgbox.exec();
        if (msgbox.clickedButton() == deleteButton) {

            qDebug() << Q_FUNC_INFO << "TODO Move to trash";
        }
    }
}

void MainWindow::openDirectory()
{
    if (!m_downloadManager->selection().isEmpty()) {
        auto item = m_downloadManager->selection().first();
        auto url = item->localDirUrl();
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::information(
                        this, tr("Error"),
                        tr("Destination directory not found:\n\n%0")
                        .arg(url.toLocalFile()));
        }
    }
}

bool MainWindow::askConfirmation(const QString &text)
{
    if (m_settings->isConfirmRemovalEnabled()) {
        QMessageBox msgbox(this);
        msgbox.setWindowTitle(tr("Remove Downloads"));
        msgbox.setText(tr("Are you sure to remove %0 downloads?").arg(text));
        msgbox.setIcon(QMessageBox::Icon::Question);
        msgbox.addButton(QMessageBox::Yes);
        msgbox.addButton(QMessageBox::No);
        msgbox.setDefaultButton(QMessageBox::No);

        QCheckBox *cb = new QCheckBox("Don't ask again");
        msgbox.setCheckBox(cb);
        QObject::connect(cb, &QCheckBox::stateChanged, [this](int state){
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                m_settings->setConfirmRemovalEnabled(false);
            }
        });

        int response = msgbox.exec();
        return response == QMessageBox::Yes;
    }
    return true;
}

void MainWindow::cleanGoneFiles()
{
    if (askConfirmation(tr("waiting"))) {
        m_downloadManager->remove(m_downloadManager->waitingJobs());
    }
}

void MainWindow::removeAll()
{    
    if (askConfirmation(tr("ALL"))) {
        m_downloadManager->clear();
    }
}

void MainWindow::removeCompletedDownloads()
{
    if (askConfirmation(tr("completed"))) {
        m_downloadManager->remove(m_downloadManager->completedJobs());
    }
}

void MainWindow::removeDownloads()
{
    removeSelected();
}

void MainWindow::removeSelected()
{
    if (askConfirmation(tr("selected"))) {
        m_downloadManager->remove(m_downloadManager->selection());
    }
}

void MainWindow::removeDuplicates()
{
    qDebug() << Q_FUNC_INFO << "TODO remove Duplicates";
}

void MainWindow::removeFailed()
{
    if (askConfirmation(tr("failed"))) {
        m_downloadManager->remove(m_downloadManager->failedJobs());
    }
}

void MainWindow::removePaused()
{
    if (askConfirmation(tr("paused"))) {
        m_downloadManager->remove(m_downloadManager->pausedJobs());
    }
}

void MainWindow::add()
{
    AddDownloadDialog dialog(urlFromClipboard(), m_downloadManager, m_settings, this);
    dialog.exec();
}

void MainWindow::resume()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->resume(item);
    }
}

void MainWindow::cancel()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->cancel(item);
    }
}

void MainWindow::pause()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->pause(item);
    }
}

void MainWindow::up()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::top()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::down()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::bottom()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::speedLimit()
{    
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::addDomainSpecificLimit()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::forceStart()
{
    qDebug() << Q_FUNC_INFO << "TODO";

    foreach (auto item, m_downloadManager->selection()) {
        /// todo Maybe run the item instantly (in a higher priority queue?)
        m_downloadManager->cancel(item);
        m_downloadManager->resume(item);
    }
}

void MainWindow::showPreferences()
{
    PreferenceDialog dialog(m_settings, this);
    dialog.exec();
}

void MainWindow::about()
{
    QMessageBox msgBox(QMessageBox::NoIcon, tr("About %0").arg(STR_APPLICATION_NAME), aboutHtml());
    msgBox.exec();
}

void MainWindow::aboutCompiler()
{
    CompilerDialog dialog(this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onJobAddedOrRemoved(DownloadRange /*range*/)
{
    refreshTitleAndStatus();
}

void MainWindow::onJobStateChanged(IDownloadItem * /*downloadItem*/)
{
    // if (m_downloadManager->isSelected(downloadItem)) {
    refreshMenus();
    // }
    refreshTitleAndStatus();
}

void MainWindow::onSelectionChanged()
{
    refreshMenus();
}

void MainWindow::refreshTitleAndStatus()
{
    const QString totalSpeed = m_downloadManager->totalSpeed();
    const int completedCount = m_downloadManager->completedJobs().count();
    const int runningCount = m_downloadManager->runningJobs().count();
    const int count = m_downloadManager->count();

    this->setWindowTitle(QString("%0 %1/%2 - %3 v%4")
                         .arg(totalSpeed)
                         .arg(completedCount)
                         .arg(count)
                         .arg(STR_APPLICATION_NAME)
                         .arg(STR_APPLICATION_VERSION).trimmed());

    m_statusBarLabel->setText(
                QString("%0 of %1 (%2), %3 running  %4")
                .arg(completedCount)
                .arg(count)
                .arg(count)
                .arg(runningCount)
                .arg(totalSpeed).trimmed());
}

void MainWindow::refreshMenus()
{
    const bool hasJobs = !m_downloadManager->downloadItems().isEmpty();
    const bool hasSelection = !m_downloadManager->selection().isEmpty();
    const bool hasOnlyOneSelected = m_downloadManager->selection().count() == 1;
    bool hasOnlyCompletedSelected = hasSelection;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasOnlyCompletedSelected = false;
            continue;
        }
    }
    bool hasAtLeastOneUncompletedSelected = false;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasAtLeastOneUncompletedSelected = true;
            continue;
        }
    }
    bool hasResumableSelection = false;
    bool hasPausableSelection = false;
    bool hasCancelableSelection = false;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->isResumable()) {
            hasResumableSelection = true;
        }
        if (item->isPausable()) {
            hasPausableSelection = true;
        }
        if (item->isCancelable()) {
            hasCancelableSelection = true;
        }
    }

    //! [0] File
    //ui->actionImportWizard->setEnabled(hasSelection);
    // --
    //ui->actionImportFromFile->setEnabled(hasSelection);
    ui->actionExportSelectedToFile->setEnabled(hasSelection);
    // --
    //ui->actionExit->setEnabled(hasSelection);
    //! [0]

    //! [1] Edit
    //ui->actionSelectAll->setEnabled(hasSelection);
    ui->actionSelectNone->setEnabled(hasSelection);
    //ui->actionInvertSelection->setEnabled(hasSelection);
    //ui->actionSelectCompleted->setEnabled(hasSelection);
    // --
    ui->actionCopy->setEnabled(hasSelection);
    // --
    ui->actionManageMirrors->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneMoreSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneFewerSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    //! [1]

    //! [2] View
    ui->actionInformation->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionOpenFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionRenameFile->setEnabled(hasOnlyOneSelected);
    ui->actionDeleteFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionOpenDirectory->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionRemoveCompletedDownloads->setEnabled(hasJobs);
    ui->actionRemoveAll1->setEnabled(hasJobs);
    ui->actionCleanGoneFiles->setEnabled(hasJobs);
    // --
    ui->actionRemoveDownloads->setEnabled(hasJobs);
    ui->actionRemoveSelected->setEnabled(hasJobs);
    ui->actionRemoveDuplicates->setEnabled(hasJobs);
    ui->actionRemoveAll2->setEnabled(hasJobs);
    ui->actionRemoveFailed->setEnabled(hasJobs);
    ui->actionRemovePaused->setEnabled(hasJobs);
    //! [2]

    //! [3] Download
    //ui->actionAdd->setEnabled(hasSelection);
    //--
    ui->actionResume->setEnabled(hasResumableSelection);
    ui->actionPause->setEnabled(hasPausableSelection);
    ui->actionCancel->setEnabled(hasCancelableSelection);
    //--
    ui->actionUp->setEnabled(hasSelection);
    ui->actionTop->setEnabled(hasSelection);
    ui->actionDown->setEnabled(hasSelection);
    ui->actionBottom->setEnabled(hasSelection);
    //! [3]

    //! [4]  Options
    ui->actionSpeedLimit->setEnabled(hasSelection);
    ui->actionAddDomainSpecificLimit->setEnabled(hasSelection);
    //--
    ui->actionForceStart->setEnabled(hasSelection);
    //--
    //ui->actionPreferences->setEnabled(hasSelection);
    //! [4]

    //! [5] Help
    //ui->actionAbout->setEnabled(hasSelection);
    //ui->actionAboutQt->setEnabled(hasSelection);
    //ui->actionAboutCompiler->setEnabled(hasSelection);
    //! [5]
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::setWorkingDirectory(const QString &path)
{
    const QFileInfo fi(path); // in case it's a file
    const QString directory =  fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath();
    QDir dir(directory);
    if (!dir.exists()) {
        dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    }
    if (!dir.exists()) {
        dir.setPath(QDir::homePath());
    }
    QDir::setCurrent(dir.absolutePath());
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::readSettings()
{
    QSettings settings;
    if ( !isMaximized() ) {
        const QPoint defaultPosition(100, 100);
        const QSize defaultSize(350, 350);
        QPoint position = settings.value("Position", defaultPosition).toPoint();
        QSize size = settings.value("Size", defaultSize).toSize();

        QRect availableGeometry(0, 0, 0, 0);
        foreach (auto screen, QApplication::screens()) {
            availableGeometry = availableGeometry.united(screen->availableGeometry());
        }

        if (!availableGeometry.intersects(QRect(position, size))) {
            position = defaultPosition;
            size = defaultSize;
        }
        this->move(position);
        this->resize(size);
    }
#if QT_VERSION >= 0x050700
    setWindowState(settings.value("WindowState", 0).value<Qt::WindowStates>());
#else
    setWindowState((Qt::WindowStates)settings.value("WindowState", 0).toInt() );
#endif
    setWorkingDirectory(settings.value("WorkingDirectory").toString());
    ui->downloadQueueView->setColumnWidths(settings.value("ColumnWidths").value<QList<int> >());

    m_settings->readSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    if( !(isMaximized() | isFullScreen()) ) {
        settings.setValue("Position", this->pos());
        settings.setValue("Size", this->size());
    }
    settings.setValue("WindowState", static_cast<int>(this->windowState())); // minimized, maximized, active, fullscreen...
    settings.setValue("WorkingDirectory", QDir::currentPath());
    settings.setValue("ColumnWidths", QVariant::fromValue(ui->downloadQueueView->columnWidths()));

    // --------------------------------------------------------------
    // Write also the current version of application in the settings,
    // because it might be used by 3rd-party update manager softwares like
    // FileHippo or Google Updater (aka gup).
    // --------------------------------------------------------------
    settings.setValue("Version", STR_APPLICATION_VERSION );

    m_settings->writeSettings();
}

/******************************************************************************
 ******************************************************************************/
inline QUrl MainWindow::urlFromClipboard() const
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    return QUrl(mimeData->hasText() ? mimeData->text() : QString());
}

/******************************************************************************
 ******************************************************************************/
QString MainWindow::askOpenFileName(const QString &fileFilter, const QString &title)
{
    return QFileDialog::getOpenFileName(this, title, QDir::currentPath(), fileFilter);
}

QString MainWindow::askSaveFileName(const QString &fileFilter, const QString &title)
{
    return QFileDialog::getSaveFileName(this, title, QDir::currentPath(), fileFilter);
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::saveFile(const QString &path)
{
    FileWriter writer(path);
    if (!writer.write(m_downloadManager)) {
        qWarning("Couldn't open save file.");
        QMessageBox::warning(this, tr("Cannot save file"),
                             tr("Cannot write to file %1:\n%2.")
                             .arg(path)
                             .arg(writer.errorString()));
        return false;
    }
    this->refreshTitleAndStatus();
    this->refreshMenus();
    this->statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::loadFile(const QString &path)
{
    FileReader reader(path);
    if (!reader.read(m_downloadManager)) {
        qWarning("Couldn't open file.");
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(path)
                             .arg(reader.errorString()));
        return false;
    }
    this->refreshTitleAndStatus();
    this->refreshMenus();
    this->statusBar()->showMessage(tr("File loaded"), 5000);
    return true;
}
