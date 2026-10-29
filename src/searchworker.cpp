#include "searchworker.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

SearchWorker::SearchWorker(QString root, QString query, QString extension,
                           bool caseSensitive, QObject *parent)
    : QObject(parent),
      m_root(std::move(root)),
      m_query(std::move(query)),
      m_extension(std::move(extension)),
      m_caseSensitive(caseSensitive)
{
}

void SearchWorker::cancel()
{
    m_cancelled.store(true);
}

void SearchWorker::run()
{
    const Qt::CaseSensitivity sensitivity =
        m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    QDirIterator it(m_root,
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    quint64 scanned = 0;
    quint64 matches = 0;

    while (it.hasNext() && !m_cancelled.load()) {
        const QString path = it.next();
        const QFileInfo info = it.fileInfo();
        ++scanned;

        const QString name = info.fileName();
        const bool nameMatches = m_query.isEmpty() ||
                                 name.contains(m_query, sensitivity);

        QString wantedExtension = m_extension.trimmed();
        if (wantedExtension.startsWith('.'))
            wantedExtension.remove(0, 1);

        const bool extensionMatches =
            wantedExtension.isEmpty() ||
            info.suffix().compare(wantedExtension, Qt::CaseInsensitive) == 0;

        if (nameMatches && extensionMatches) {
            ++matches;
            emit resultFound(info.absoluteFilePath(), info.size(),
                             info.lastModified());
        }

        if (scanned % 250 == 0)
            emit progress(path, scanned);
    }

    emit finished(scanned, matches);
}
