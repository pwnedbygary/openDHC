#include "BatchScanner.hpp"
#include <QFileInfo>
#include <QUuid>

static QString relUnder(const QString& root, const QString& absDir) {
    if (root.isEmpty()) return QString();
    QDir r(QFileInfo(root).absoluteFilePath());
    QString rel = r.relativeFilePath(absDir);
    if (rel=="." || rel=="..") return QString();
    return rel;
}

bool BatchScanner::isCDInput(const QString& p){
    const auto ext = QFileInfo(p).suffix().toLower();
    return ext=="cue" || ext=="toc" || ext=="gdi";
}
bool BatchScanner::isDVDInput(const QString& p){
    const auto ext = QFileInfo(p).suffix().toLower();
    return ext=="iso";
}

QString BatchScanner::defaultOutputFor(const QString& input, JobType t, MediaType m,
                                       const QString& outRoot, bool preserve, const QString& sourceRoot) {
    const QFileInfo in(input);
    const QString base = in.completeBaseName();
    QDir out(outRoot);
    if (preserve && !sourceRoot.isEmpty()) {
        const QString rel = relUnder(sourceRoot, in.dir().absolutePath());
        if (!rel.isEmpty()) { out.mkpath(rel); out.cd(rel); }
    }
    switch (t) {
        case JobType::Create:  return out.filePath(base + ".chd");
        case JobType::Extract: return out.filePath((m==MediaType::DVD) ? base + ".iso" : base + ".cue");
        case JobType::Verify:
        case JobType::Info:    return QString();
    }
    return out.filePath(base + ".chd");
}

QList<Job> BatchScanner::scan(const QString& sourceDir, const QString& outRoot,
                              JobType jobType, const Options& opt,
                              const QStringList& extraArgs, bool deleteSrc,
                              bool preserveStructure) const {
    QList<Job> jobs;
    QDirIterator it(sourceDir, opt.recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()){
        const auto path = it.next();
        if (!QFileInfo(path).isFile()) continue;
        const bool cd  = isCDInput(path);
        const bool dvd = isDVDInput(path);
        if ((cd && opt.includeCD) || (dvd && opt.includeDVD)) {
            Job j;
            j.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            j.type = jobType;
            j.media = dvd ? MediaType::DVD : MediaType::CD;
            j.inputPath = path;
            j.outputPath = defaultOutputFor(path, jobType, j.media, outRoot, preserveStructure, sourceDir);
            j.extraArgs = extraArgs;
            j.deleteSourceAfter = deleteSrc;
            j.preserveStructure = preserveStructure;
            jobs << j;
        }
    }
    return jobs;
}

QStringList BatchScanner::findInputs(QString sourceDir, bool recursive, bool includeCD, bool includeDVD) const {
    QStringList out;
    QDirIterator it(sourceDir, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()){
        auto p = it.next();
        if (!QFileInfo(p).isFile()) continue;
        const bool cd  = isCDInput(p);
        const bool dvd = isDVDInput(p);
        if ((cd && includeCD) || (dvd && includeDVD))
            out << p;
    }
    return out;
}

QString BatchScanner::defaultOutputForInvokable(QString input, int jobType, int media, QString outRoot, bool preserve) const {
    return defaultOutputFor(input, static_cast<JobType>(jobType),
                            static_cast<MediaType>(media), outRoot, preserve);
}
