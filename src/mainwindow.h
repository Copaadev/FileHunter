#pragma once

#include <QMainWindow>
#include <QThread>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QTableWidget;
class SearchWorker;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void chooseFolder();
    void startSearch();
    void stopSearch();
    void addResult(const QString &path, qint64 size, const QDateTime &modified);
    void updateProgress(const QString &path, quint64 scanned);
    void searchFinished(quint64 scanned, quint64 matches);
    void openSelected();
    void showContextMenu(const QPoint &position);

private:
    void setSearching(bool searching);
    QString selectedPath() const;
    static QString readableSize(qint64 bytes);

    QLineEdit *m_folderEdit{};
    QLineEdit *m_nameEdit{};
    QLineEdit *m_extensionEdit{};
    QCheckBox *m_caseCheck{};
    QPushButton *m_browseButton{};
    QPushButton *m_searchButton{};
    QPushButton *m_stopButton{};
    QLabel *m_status{};
    QTableWidget *m_results{};

    QThread *m_thread{};
    SearchWorker *m_worker{};
};
