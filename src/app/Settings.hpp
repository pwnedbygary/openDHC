#pragma once
#include <QObject>
#include <QSettings>

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString chdmanPath READ chdmanPath WRITE setChdmanPath NOTIFY changed)
    Q_PROPERTY(QString outputDir READ outputDir WRITE setOutputDir NOTIFY changed)
    Q_PROPERTY(int concurrency READ concurrency WRITE setConcurrency NOTIFY changed)
public:
    explicit Settings(QObject* parent=nullptr) : QObject(parent), s("OpenDHC","OpenDHC") {}

    QString chdmanPath() const { return s.value("chdmanPath").toString(); }
    QString outputDir()  const { return s.value("outputDir").toString(); }
    int concurrency()    const { return s.value("concurrency", 2).toInt(); }

public slots:
    void setChdmanPath(const QString& v) { s.setValue("chdmanPath", v); emit changed(); }
    void setOutputDir(const QString& v)  { s.setValue("outputDir", v); emit changed(); }
    void setConcurrency(int v)           { s.setValue("concurrency", v); emit changed(); }

signals:
    void changed();
private:
    QSettings s;
};
