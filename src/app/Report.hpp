#pragma once
#include "Job.hpp"
#include <QObject>

struct JobResult {
    QString id;
    bool ok = false;
    quint64 inputBytes = 0;
    quint64 outputBytes = 0;
    qint64  msec = 0;
    QString inputPath, outputPath, status, log;
};

class Report : public QObject {
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY updated)
    Q_PROPERTY(int ok READ ok NOTIFY updated)
    Q_PROPERTY(int failed READ failed NOTIFY updated)
    Q_PROPERTY(double savedPct READ savedPct NOTIFY updated)
public:
    explicit Report(QObject* parent=nullptr):QObject(parent){}
    void reset(){ m_items.clear(); emit updated(); }
    void add(JobResult r){ m_items.push_back(std::move(r)); emit updated(); }
    int total() const { return m_items.size(); }
    int ok() const { int c=0; for (auto& i : m_items) if (i.ok) ++c; return c; }
    int failed() const { return total()-ok(); }
    quint64 inBytes() const { quint64 s=0; for (auto& i:m_items) s+=i.inputBytes; return s; }
    quint64 outBytes()const { quint64 s=0; for (auto& i:m_items) s+=i.outputBytes; return s; }
    double savedPct() const {
        auto ib=inBytes(), ob=outBytes();
        if (ib==0) return 0.0; return 100.0 * double(ib - ob) / double(ib);
    }
    Q_INVOKABLE QString asMarkdown() const;
    Q_INVOKABLE bool saveCsv(const QString& filePath) const;

signals: void updated();

private:
    QList<JobResult> m_items;
};
