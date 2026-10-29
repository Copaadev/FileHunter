#pragma once

#include <QObject>
#include <QString>
#include <atomic>

class SearchWorker final : public QObject
{
    Q_OBJECT
public:
    explicit SearchWorker(QString root, QString query, QString extension,
                          bool caseSensitive, QObject *parent = nullptr);

public slots:
    void run();
    void cancel();

signals:
    void resultFound(const QString &path, qint64 size, const QDateTime &modified);
    void progress(const QString &currentPath, quint64 scanned);
    void finished(quint64 scanned, quint64 matches);
    void errorOccurred(const QString &message);

private:
    QString m_root;
    QString m_query;
    QString m_extension;
    bool m_caseSensitive;
    std::atomic_bool m_cancelled{false};
};
