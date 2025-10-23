#pragma once
#include "Job.hpp"
#include <QObject>
#include <QProcess>
#include <QQueue>

class ChdmanRunner : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString chdmanPath READ chdmanPath WRITE setChdmanPath NOTIFY chdmanPathChanged)
    Q_PROPERTY(int concurrency READ concurrency WRITE setConcurrency NOTIFY concurrencyChanged)
public:
    explicit ChdmanRunner(QObject* parent=nullptr);

    QString chdmanPath() const { return m_chdmanPath; }
    void setChdmanPath(const QString& p);

    int concurrency() const { return m_concurrency; }
    void setConcurrency(int c);

    Q_INVOKABLE bool probeChdman();
    Q_INVOKABLE void enqueue(const QString& jobId, const Job& job);
    Q_INVOKABLE void enqueueSimple(QString id, int type, int media, QString input,
                                   QString output, QStringList extraArgs,
                                   bool deleteSrc, bool preserve);
    Q_INVOKABLE void cancel(const QString& jobId);

signals:
    void chdmanPathChanged();
    void concurrencyChanged();
    void jobStarted(const QString& id);
    void jobProgress(const QString& id, int percent);
    void jobLog(const QString& id, const QString& line);
    void jobFinished(const QString& id, bool ok);

private:
    struct Proc { QString id; QProcess* p; Job j; };
    QString m_chdmanPath;
    int m_concurrency = 2;
    QList<Proc> m_running;
    QQueue<Proc> m_queue;

    QStringList buildArgs(const Job& j) const;
    void maybeStartNext();
    static int parseProgress(const QString& line);
};
