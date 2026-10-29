#include "mainwindow.h"
#include "searchworker.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("FileHunter — Copa Development");
    resize(1050, 680);
    setMinimumSize(760, 500);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(22, 20, 22, 18);
    layout->setSpacing(13);

    auto *heading = new QLabel("FileHunter");
    heading->setObjectName("heading");
    auto *subtitle = new QLabel("Fast, local file search for your desktop.");
    subtitle->setObjectName("subtitle");
    layout->addWidget(heading);
    layout->addWidget(subtitle);

    auto *folderRow = new QHBoxLayout;
    m_folderEdit = new QLineEdit(QDir::homePath());
    m_folderEdit->setPlaceholderText("Folder to search");
    m_browseButton = new QPushButton("Browse…");
    folderRow->addWidget(new QLabel("Location"));
    folderRow->addWidget(m_folderEdit, 1);
    folderRow->addWidget(m_browseButton);
    layout->addLayout(folderRow);

    auto *filterRow = new QHBoxLayout;
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("e.g. report, photo, project");
    m_extensionEdit = new QLineEdit;
    m_extensionEdit->setPlaceholderText("e.g. pdf, png, cpp");
    m_caseCheck = new QCheckBox("Case sensitive");
    filterRow->addWidget(new QLabel("Name"));
    filterRow->addWidget(m_nameEdit, 2);
    filterRow->addWidget(new QLabel("Extension"));
    filterRow->addWidget(m_extensionEdit, 1);
    filterRow->addWidget(m_caseCheck);
    layout->addLayout(filterRow);

    auto *actionRow = new QHBoxLayout;
    m_searchButton = new QPushButton("Search files");
    m_searchButton->setObjectName("primary");
    m_stopButton = new QPushButton("Stop");
    m_stopButton->setEnabled(false);
    actionRow->addWidget(m_searchButton);
    actionRow->addWidget(m_stopButton);
    actionRow->addStretch();
    m_status = new QLabel("Choose a folder and start searching.");
    m_status->setObjectName("status");
    actionRow->addWidget(m_status);
    layout->addLayout(actionRow);

    m_results = new QTableWidget(0, 4);
    m_results->setHorizontalHeaderLabels({"Name", "Location", "Size", "Modified"});
    m_results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_results->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_results->setSelectionMode(QAbstractItemView::SingleSelection);
    m_results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_results->setAlternatingRowColors(true);
    m_results->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_results, 1);

    auto *footer = new QLabel("Search runs locally. Restricted folders may be skipped by your operating system.");
    footer->setObjectName("footer");
    layout->addWidget(footer);
    setCentralWidget(central);

    setStyleSheet(R"(
        QMainWindow, QWidget { background: #111318; color: #e8eaf0; font-size: 13px; }
        QLabel#heading { font-size: 29px; font-weight: 700; color: #ffffff; }
        QLabel#subtitle, QLabel#footer { color: #969eae; }
        QLabel#status { color: #8eb7ff; }
        QLineEdit { background: #1b1f28; border: 1px solid #343b49; border-radius: 7px; padding: 9px; selection-background-color: #3978d8; }
        QLineEdit:focus { border: 1px solid #5795f5; }
        QPushButton { background: #252b36; border: 1px solid #3a4352; border-radius: 7px; padding: 9px 15px; }
        QPushButton:hover { background: #303949; }
        QPushButton:disabled { color: #697181; }
        QPushButton#primary { background: #2869c7; border-color: #397bd8; font-weight: 600; }
        QPushButton#primary:hover { background: #3479df; }
        QTableWidget { background: #171a21; alternate-background-color: #1c2029; border: 1px solid #303744; border-radius: 7px; gridline-color: #292f3a; }
        QHeaderView::section { background: #202631; color: #b9c2d1; padding: 9px; border: none; border-bottom: 1px solid #343b49; }
        QTableWidget::item { padding: 6px; }
        QTableWidget::item:selected { background: #244d83; }
        QCheckBox { spacing: 7px; }
        QMenu { background: #202631; border: 1px solid #3a4352; }
        QMenu::item { padding: 7px 24px; }
        QMenu::item:selected { background: #2869c7; }
    )");

    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::chooseFolder);
    connect(m_searchButton, &QPushButton::clicked, this, &MainWindow::startSearch);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopSearch);
    connect(m_results, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { openSelected(); });
    connect(m_results, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}

MainWindow::~MainWindow()
{
    stopSearch();
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }
}

void MainWindow::chooseFolder()
{
    const QString folder = QFileDialog::getExistingDirectory(
        this, "Choose folder", m_folderEdit->text());
    if (!folder.isEmpty())
        m_folderEdit->setText(folder);
}

void MainWindow::startSearch()
{
    if (m_thread) {
        QMessageBox::information(this, "Search running", "Stop the current search before starting another.");
        return;
    }

    const QString folder = QDir::cleanPath(m_folderEdit->text().trimmed());
    if (!QDir(folder).exists()) {
        QMessageBox::warning(this, "Invalid folder", "Choose a folder that exists.");
        return;
    }

    m_results->setRowCount(0);
    m_status->setText("Scanning…");
    setSearching(true);

    m_thread = new QThread(this);
    m_worker = new SearchWorker(folder, m_nameEdit->text(),
                                 m_extensionEdit->text(), m_caseCheck->isChecked());
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &SearchWorker::run);
    connect(m_worker, &SearchWorker::resultFound, this, &MainWindow::addResult);
    connect(m_worker, &SearchWorker::progress, this, &MainWindow::updateProgress);
    connect(m_worker, &SearchWorker::finished, this, &MainWindow::searchFinished);
    connect(m_worker, &SearchWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &SearchWorker::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this] {
        m_thread->deleteLater();
        m_thread = nullptr;
        m_worker = nullptr;
        setSearching(false);
    });
    m_thread->start();
}

void MainWindow::stopSearch()
{
    if (m_worker) {
        m_worker->cancel();
        m_status->setText("Stopping…");
        m_stopButton->setEnabled(false);
    }
}

void MainWindow::addResult(const QString &path, qint64 size, const QDateTime &modified)
{
    const int row = m_results->rowCount();
    m_results->insertRow(row);

    auto *name = new QTableWidgetItem(QFileInfo(path).fileName());
    name->setData(Qt::UserRole, path);
    m_results->setItem(row, 0, name);
    m_results->setItem(row, 1, new QTableWidgetItem(QFileInfo(path).absolutePath()));
    m_results->setItem(row, 2, new QTableWidgetItem(readableSize(size)));
    m_results->setItem(row, 3, new QTableWidgetItem(modified.toString("yyyy-MM-dd HH:mm")));
}

void MainWindow::updateProgress(const QString &path, quint64 scanned)
{
    m_status->setText(QString("Scanned %1 files… %2").arg(scanned).arg(QFileInfo(path).fileName()));
}

void MainWindow::searchFinished(quint64 scanned, quint64 matches)
{
    m_status->setText(QString("Finished — %1 matches from %2 files").arg(matches).arg(scanned));
}

void MainWindow::setSearching(bool searching)
{
    m_searchButton->setEnabled(!searching);
    m_browseButton->setEnabled(!searching);
    m_stopButton->setEnabled(searching);
}

QString MainWindow::selectedPath() const
{
    const int row = m_results->currentRow();
    if (row < 0 || !m_results->item(row, 0))
        return {};
    return m_results->item(row, 0)->data(Qt::UserRole).toString();
}

void MainWindow::openSelected()
{
    const QString path = selectedPath();
    if (!path.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::showContextMenu(const QPoint &position)
{
    if (m_results->itemAt(position))
        m_results->selectRow(m_results->itemAt(position)->row());

    const QString path = selectedPath();
    if (path.isEmpty())
        return;

    QMenu menu(this);
    QAction *openAction = menu.addAction("Open file");
    QAction *folderAction = menu.addAction("Open containing folder");
    QAction *copyAction = menu.addAction("Copy full path");
    QAction *chosen = menu.exec(m_results->viewport()->mapToGlobal(position));

    if (chosen == openAction)
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    else if (chosen == folderAction)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
    else if (chosen == copyAction)
        QApplication::clipboard()->setText(path);
}

QString MainWindow::readableSize(qint64 bytes)
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < units.size() - 1) {
        value /= 1024.0;
        ++unit;
    }
    return QString("%1 %2").arg(value, 0, 'f', unit == 0 ? 0 : 1).arg(units[unit]);
}
