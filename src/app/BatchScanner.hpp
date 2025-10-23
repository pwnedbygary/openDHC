#pragma once
#include "Job.hpp"
#include <QObject>
#include <QDirIterator>

class BatchScanner : public QObject {
    Q_OBJECT
public:
    struct Options {
        bool recursive = true;
        bool includeCD = true;     // .cue/.toc/.gdi
        bool includeDVD = true;    // .iso
        QString sourceRoot;        // used for mirroring
    };
    explicit BatchScanner(QObject* parent=nullptr):QObject(parent){}

    QList<Job> scan(const QString& sourceDir, const QString& outRoot,
                    JobType jobType, const Options& opt,
                    const QStringList& extraArgs, bool deleteSrc,
                    bool preserveStructure) const;

    // QML helpers
    Q_INVOKABLE QStringList findInputs(QString sourceDir, bool recursive, bool includeCD, bool includeDVD) const;
    Q_INVOKABLE QString defaultOutputForInvokable(QString input, int jobType, int media, QString outRoot, bool preserve) const;

private:
    static bool isCDInput(const QString& path);
    static bool isDVDInput(const QString& path);
    static QString defaultOutputFor(const QString& input, JobType t, MediaType m,
                                    const QString& outRoot, bool preserve, const QString& sourceRoot=QString());
};
