
# OpenDHC Project — Conversation Export (Inline Code Variant)

**Date:** 2025-10-23 19:32:50
**Participants:** Gary Bagley (User) · M365 Copilot (Assistant)

This variant keeps **all code inline** within the transcript for maximum portability when used with Word/OneNote/Copilot.

---

## Summary
- Linux‑native Qt 6 GUI wrapper for `chdman` (feature parity with namDHC) with batch processing, per‑job progress, final report, and **CSV export**.
- **AppImage** packaging via **linuxdeploy** + **plugin‑qt** with **zsync** auto‑updates for GitHub Releases.
- One‑shot **template generator script** that writes the complete repo structure and CI workflow.

---

## Conversation Transcript (code inline)

### 1) User
> Can you write me a C++ application that mimics the functionality of namDHC with a modern GUI?

### Assistant
Outlined a Qt 6 + QML application design, mapping namDHC features to native Linux UI, including job model, runner, presets, drag & drop, progress, and logs.

---

### 2) User
> The OS will be Linux. Add batch processing from a directory, progress bars for each job, and a final report.

### Assistant
Proposed `BatchScanner`, `Report`, and `SizeUtil` modules; final report shows input vs output sizes and time; added CSV export hook.

---

### 3) User
> Generate the AppImage and add CSV export

### Assistant
Added `Report::saveCsv(path)` and QML button; provided linuxdeploy + plugin‑qt packaging and CI basics.

---

### 4) User
> Push the repo and add zsync support

### Assistant
Explained push limitations, added **zsync** via `LDAI_UPDATE_INFORMATION` and ensured `.zsync` is produced in CI.

---

### 5) User
> How do I make you a collaborator?

### Assistant
Clarified there is no GitHub identity to invite; suggested pushing locally or token‑based CI.

---

### 6) User
> I tried searching for your username and nothing comes up

### Assistant
Confirmed and proposed two options (manual push or PAT‑based automation).

---

### 7) User
> I don't see the token menu anywhere

### Assistant
Directed to **Account Settings → Developer settings → Personal access tokens** (fine‑grained), then repo secrets.

---

### 8) User
> I’ve added the fine‑grained token. How do I share the repo with you now?

### Assistant
Offered to generate a **template ZIP** and a workflow using the token.

---

### 9) User
> Generate a ZIP of the template

### Assistant (inline code — full script)

Save this as **`get-opendhc-template.sh`**, make it executable, and run it. It creates `opendhc-template.zip` containing a complete project (sources, QML UI, AppImage packaging with zsync, and a GitHub Actions workflow).

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(pwd)/OpenDHC"
ZIP_OUT="$(pwd)/opendhc-template.zip"

echo "Creating template in: $ROOT"
rm -rf "$ROOT" "$ZIP_OUT"
mkdir -p "$ROOT"/{src/app,ui/qml,packaging,.github/workflows}

########################################
# CMakeLists.txt
########################################
cat > "$ROOT/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.22)
project(OpenDHC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

qt_standard_project_setup()

qt_add_executable(OpenDHC
    src/main.cpp
    src/app/Job.hpp src/app/Job.cpp
    src/app/JobModel.hpp src/app/JobModel.cpp
    src/app/ChdmanRunner.hpp src/app/ChdmanRunner.cpp
    src/app/Settings.hpp src/app/Settings.cpp
    src/app/BatchScanner.hpp src/app/BatchScanner.cpp
    src/app/Report.hpp src/app/Report.cpp
    src/app/SizeUtil.hpp src/app/SizeUtil.cpp
)

qt_add_qml_module(OpenDHC
    URI OpenDHC
    VERSION 1.0
    QML_FILES ui/qml/Main.qml
    RESOURCE_PREFIX "/"
)

target_link_libraries(OpenDHC
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
)

# Install the app into AppDir/usr/bin when used with DESTDIR during packaging
install(TARGETS OpenDHC RUNTIME DESTINATION usr/bin)
EOF

########################################
# src/app/Job.hpp
########################################
cat > "$ROOT/src/app/Job.hpp" <<'EOF'
#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>

enum class JobType { Create, Verify, Info, Extract };
enum class MediaType { CD, DVD };

struct Job {
    QString id;
    JobType type{};
    MediaType media{};
    QString inputPath;
    QString outputPath;
    QStringList extraArgs;
    int progress = 0;                // 0..100
    QString status = "Queued";       // Queued/Running/Done/Failed
    QString log;
    QDateTime started, ended;
    bool deleteSourceAfter = false;
    bool preserveStructure = true;
};
EOF

########################################
# src/app/Job.cpp
########################################
cat > "$ROOT/src/app/Job.cpp" <<'EOF'
// Intentionally empty (placeholder for future helpers)
EOF

########################################
# src/app/JobModel.hpp
########################################
cat > "$ROOT/src/app/JobModel.hpp" <<'EOF'
#pragma once
#include "Job.hpp"
#include <QAbstractListModel>
#include <QUuid>
#include <functional>

class JobModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole+1, TypeRole, MediaRole, InputRole, OutputRole,
        ProgressRole, StatusRole, LogRole, DeleteSourceRole, PreserveRole
    };
    explicit JobModel(QObject* parent=nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : m_jobs.size();
    }
    QVariant data(const QModelIndex& idx, int role) const override {
        if (!idx.isValid() || idx.row()<0 || idx.row()>=m_jobs.size()) return {};
        const auto& j = m_jobs[idx.row()];
        switch(role){
            case IdRole: return j.id;
            case TypeRole: return static_cast<int>(j.type);
            case MediaRole: return static_cast<int>(j.media);
            case InputRole: return j.inputPath;
            case OutputRole: return j.outputPath;
            case ProgressRole: return j.progress;
            case StatusRole: return j.status;
            case LogRole: return j.log;
            case DeleteSourceRole: return j.deleteSourceAfter;
            case PreserveRole: return j.preserveStructure;
        }
        return {};
    }
    QHash<int,QByteArray> roleNames() const override {
        return {
            {IdRole,"id"}, {TypeRole,"type"}, {MediaRole,"media"},
            {InputRole,"input"}, {OutputRole,"output"}, {ProgressRole,"progress"},
            {StatusRole,"status"}, {LogRole,"log"}, {DeleteSourceRole,"deleteSource"},
            {PreserveRole,"preserveStructure"}
        };
    }

    Q_INVOKABLE QString addJob(int type, int media, const QString& input, const QString& output,
                               const QStringList& extraArgs, bool delSrc, bool preserve) {
        Job j;
        j.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        j.type = static_cast<JobType>(type);
        j.media = static_cast<MediaType>(media);
        j.inputPath = input; j.outputPath = output;
        j.extraArgs = extraArgs; j.deleteSourceAfter = delSrc; j.preserveStructure = preserve;
        beginInsertRows({}, m_jobs.size(), m_jobs.size());
        m_jobs.push_back(std::move(j));
        endInsertRows();
        return m_jobs.back().id;
    }

    Job& jobRefById(const QString& id) {
        for (auto& j : m_jobs) if (j.id==id) return j;
        throw std::runtime_error("job not found");
    }
    int indexById(const QString& id) const {
        for (int i=0;i<m_jobs.size();++i) if (m_jobs[i].id==id) return i;
        return -1;
    }
    void updateJob(const QString& id, std::function<void(Job&)> fn) {
        int i = indexById(id); if (i<0) return;
        fn(m_jobs[i]);
        emit dataChanged(index(i), index(i));
    }
    Q_INVOKABLE void removeJob(const QString& id) {
        int i = indexById(id); if (i<0) return;
        beginRemoveRows({}, i, i);
        m_jobs.removeAt(i);
        endRemoveRows();
    }

private:
    QVector<Job> m_jobs;
};
EOF

########################################
# src/app/JobModel.cpp
########################################
cat > "$ROOT/src/app/JobModel.cpp" <<'EOF'
#include "JobModel.hpp"
// implementation in header (inline-sized)
EOF

########################################
# src/app/ChdmanRunner.hpp
########################################
cat > "$ROOT/src/app/ChdmanRunner.hpp" <<'EOF'
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
EOF

########################################
# src/app/ChdmanRunner.cpp
########################################
cat > "$ROOT/src/app/ChdmanRunner.cpp" <<'EOF'
#include "ChdmanRunner.hpp"
#include <QRegularExpression>

ChdmanRunner::ChdmanRunner(QObject* parent) : QObject(parent) {}

void ChdmanRunner::setChdmanPath(const QString& p) {
    if (m_chdmanPath==p) return;
    m_chdmanPath = p;
    emit chdmanPathChanged();
}

void ChdmanRunner::setConcurrency(int c) {
    c = std::clamp(c,1,16);
    if (m_concurrency==c) return;
    m_concurrency = c;
    emit concurrencyChanged();
    maybeStartNext();
}

bool ChdmanRunner::probeChdman() {
    QProcess p;
    p.start(m_chdmanPath.isEmpty() ? "chdman" : m_chdmanPath, {"-help"});
    return p.waitForFinished(5000) && p.exitStatus()==QProcess::NormalExit;
}

QStringList ChdmanRunner::buildArgs(const Job& j) const {
    QStringList args;
    switch(j.type){
        case JobType::Create:
            args << (j.media==MediaType::DVD ? "createdvd" : "createcd")
                 << "-i" << j.inputPath << "-o" << j.outputPath;
            break;
        case JobType::Verify:
            args << "verify" << "-i" << j.inputPath; break;
        case JobType::Info:
            args << "info" << "-i" << j.inputPath << "-v"; break;
        case JobType::Extract:
            args << (j.media==MediaType::DVD ? "extractdvd" : "extractcd")
                 << "-i" << j.inputPath << "-o" << j.outputPath;
            break;
    }
    args << j.extraArgs;
    return args;
}

int ChdmanRunner::parseProgress(const QString& line) {
    static QRegularExpression re("(\\d{1,3})%");
    auto m = re.match(line);
    if (m.hasMatch()) {
        bool ok=false; int v=m.captured(1).toInt(&ok);
        if (ok) return std::clamp(v,0,100);
    }
    return -1;
}

void ChdmanRunner::enqueueSimple(QString id, int type, int media, QString input,
                                 QString output, QStringList extraArgs,
                                 bool deleteSrc, bool preserve) {
    Job j;
    j.id=id; j.type=static_cast<JobType>(type); j.media=static_cast<MediaType>(media);
    j.inputPath=input; j.outputPath=output; j.extraArgs=extraArgs;
    j.deleteSourceAfter=deleteSrc; j.preserveStructure=preserve;
    enqueue(id, j);
}

void ChdmanRunner::enqueue(const QString& jobId, const Job& j) {
    Proc pr{jobId, nullptr, j};
    m_queue.enqueue(pr);
    maybeStartNext();
}

void ChdmanRunner::cancel(const QString& jobId) {
    for (auto& r : m_running) {
        if (r.id==jobId && r.p) { r.p->kill(); }
    }
    QQueue<Proc> tmp;
    while(!m_queue.isEmpty()){
        auto pr = m_queue.dequeue();
        if (pr.id!=jobId) tmp.enqueue(pr);
    }
    m_queue = std::move(tmp);
}

void ChdmanRunner::maybeStartNext() {
    while (m_running.size() < m_concurrency && !m_queue.isEmpty()) {
        auto pr = m_queue.dequeue();
        auto proc = new QProcess(this);
        pr.p = proc;
        m_running << pr;

        QString exe = m_chdmanPath.isEmpty() ? "chdman" : m_chdmanPath;
        auto args = buildArgs(pr.j);

        emit jobStarted(pr.id);

        connect(proc,&QProcess::readyReadStandardOutput,this,[this,proc,id=pr.id]{
            auto lines = QString::fromLocal8Bit(proc->readAllStandardOutput()).split('\n',Qt::SkipEmptyParts);
            for (auto& l : lines) {
                emit jobLog(id, l);
                int p = parseProgress(l);
                if (p>=0) emit jobProgress(id, p);
            }
        });
        connect(proc,&QProcess::readyReadStandardError,this,[this,proc,id=pr.id]{
            auto lines = QString::fromLocal8Bit(proc->readAllStandardError()).split('\n',Qt::SkipEmptyParts);
            for (auto& l : lines) emit jobLog(id, l);
        });
        connect(proc,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,
                [this,id=pr.id](int code,QProcess::ExitStatus st){
            bool ok = (st==QProcess::NormalExit && code==0);
            emit jobProgress(id, 100);
            emit jobFinished(id, ok);
            for (int i=0;i<m_running.size();++i){
                if (m_running[i].id==id) { m_running[i].p->deleteLater(); m_running.removeAt(i); break; }
            }
            maybeStartNext();
        });

        proc->start(exe, args);
    }
}
EOF

########################################
# src/app/Settings.hpp / .cpp
########################################
cat > "$ROOT/src/app/Settings.hpp" <<'EOF'
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
EOF

cat > "$ROOT/src/app/Settings.cpp" <<'EOF'
#include "Settings.hpp"
// Inline in header
EOF

########################################
# src/app/BatchScanner.hpp / .cpp
########################################
cat > "$ROOT/src/app/BatchScanner.hpp" <<'EOF'
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
EOF

cat > "$ROOT/src/app/BatchScanner.cpp" <<'EOF'
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
EOF

########################################
# src/app/SizeUtil.hpp / .cpp
########################################
cat > "$ROOT/src/app/SizeUtil.hpp" <<'EOF'
#pragma once
#include "Job.hpp"
#include <QString>
#include <QtGlobal>

namespace SizeUtil {
    quint64 estimateInputBytes(const Job& j);
    quint64 safeFileSize(const QString& path);
}
EOF

cat > "$ROOT/src/app/SizeUtil.cpp" <<'EOF'
#include "SizeUtil.hpp"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDir>
using u64 = quint64;

u64 SizeUtil::safeFileSize(const QString& p) {
    QFileInfo fi(p);
    return fi.exists() ? static_cast<u64>(fi.size()) : 0;
}

static u64 sumCue(const QString& cuePath){
    QFile f(cuePath);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) return 0;
    QTextStream ts(&f);
    QDir dir(QFileInfo(cuePath).dir());
    u64 total=0;
    while(!ts.atEnd()){
        const auto line = ts.readLine().trimmed();
        if (line.startsWith("FILE ", Qt::CaseInsensitive)) {
            int a = line.indexOf('"'), b = line.lastIndexOf('"');
            if (a>=0 && b>a) {
                const auto fname = line.mid(a+1, b-a-1);
                total += SizeUtil::safeFileSize(dir.filePath(fname));
            }
        }
    }
    return total;
}

static u64 sumGdi(const QString& gdiPath){
    QFile f(gdiPath);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) return 0;
    QTextStream ts(&f);
    QDir dir(QFileInfo(gdiPath).dir());
    u64 total=0;
    while(!ts.atEnd()){
        const auto line = ts.readLine().trimmed();
        if (line.isEmpty() || !line[0].isDigit()) continue;
        const auto parts = line.split(QRegExp("\\s+"));
        if (parts.size()>=6) {
            QString fname = parts.at(5);
            if (fname.startsWith('"') && fname.endsWith('"')) fname = fname.mid(1, fname.size()-2);
            total += SizeUtil::safeFileSize(dir.filePath(fname));
        }
    }
    return total;
}

u64 SizeUtil::estimateInputBytes(const Job& j){
    const auto ext = QFileInfo(j.inputPath).suffix().toLower();
    if (ext=="cue") return sumCue(j.inputPath);
    if (ext=="gdi") return sumGdi(j.inputPath);
    return safeFileSize(j.inputPath);
}
EOF

########################################
# src/app/Report.hpp / .cpp
########################################
cat > "$ROOT/src/app/Report.hpp" <<'EOF'
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
EOF

cat > "$ROOT/src/app/Report.cpp" <<'EOF'
#include "Report.hpp"
#include <QString>
#include <QFile>
#include <QTextStream>

QString Report::asMarkdown() const {
    QString md;
    md += "# OpenDHC Final Report\n\n";
    md += QString("*Total:* %1  •  *OK:* %2  •  *Failed:* %3  •  *Saved:* %4%  \n")
            .arg(total()).arg(ok()).arg(failed()).arg(QString::number(savedPct(),'f',1));
    md += QString("*Input:* %1 MB  •  *Output:* %2 MB\n\n")
            .arg(double(inBytes())/1048576.0,0,'f',1)
            .arg(double(outBytes())/1048576.0,0,'f',1);
    md += "## Jobs\n";
    for (auto& i : m_items) {
        md += QString("- **%1** — %2  \n    - **In:** `%3`  \n    - **Out:** `%4`  \n    - **Size:** %5 ➜ %6 MB  \n    - **Time:** %7 s  \n")
            .arg(i.ok ? "OK" : "FAILED")
            .arg(i.status)
            .arg(i.inputPath).arg(i.outputPath)
            .arg(QString::number(double(i.inputBytes)/1048576.0,'f',2))
            .arg(QString::number(double(i.outputBytes)/1048576.0,'f',2))
            .arg(QString::number(double(i.msec)/1000.0,'f',1));
    }
    return md;
}

bool Report::saveCsv(const QString& filePath) const {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    ts << "Status,Input,Output,InputMiB,OutputMiB,Millis,ID\n";
    for (const auto& i : m_items) {
        ts << (i.ok ? "OK" : "FAILED") << ','
           << '"' << i.inputPath.replace('"',"\"\"")  << '"' << ','
           << '"' << i.outputPath.replace('"',"\"\"") << '"' << ','
           << QString::number(double(i.inputBytes)/1048576.0, 'f', 3) << ','
           << QString::number(double(i.outputBytes)/1048576.0, 'f', 3) << ','
           << i.msec << ','
           << i.id << '\n';
    }
    f.close();
    return true;
}
EOF

########################################
# src/main.cpp
########################################
cat > "$ROOT/src/main.cpp" <<'EOF'
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include "app/JobModel.hpp"
#include "app/ChdmanRunner.hpp"
#include "app/Settings.hpp"
#include "app/BatchScanner.hpp"
#include "app/Report.hpp"
#include "app/SizeUtil.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("OpenDHC");
    QGuiApplication::setApplicationName("OpenDHC");

    JobModel jobs;
    ChdmanRunner runner;
    Settings settings;
    BatchScanner scanner;
    Report report;

    runner.setChdmanPath(settings.chdmanPath());
    runner.setConcurrency(settings.concurrency());
    QObject::connect(&settings,&Settings::changed,[&]{
        runner.setChdmanPath(settings.chdmanPath());
        runner.setConcurrency(settings.concurrency());
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("jobModel", &jobs);
    engine.rootContext()->setContextProperty("runner", &runner);
    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("scanner", &scanner);
    engine.rootContext()->setContextProperty("report", &report);

    const QUrl url(u"qrc:/ui/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    struct Runtime { qint64 t0=0; quint64 inB=0; };
    QHash<QString, Runtime> rt;

    QObject::connect(&runner,&ChdmanRunner::jobStarted,&jobs,[&](const QString& id){
        jobs.updateJob(id,[](Job& j){ j.status="Running"; j.started=QDateTime::currentDateTime(); });
        auto &j = jobs.jobRefById(id);
        rt[id] = { QDateTime::currentMSecsSinceEpoch(), SizeUtil::estimateInputBytes(j) };
    });

    QObject::connect(&runner,&ChdmanRunner::jobProgress,&jobs,[&](const QString& id,int p){
        jobs.updateJob(id,[&](Job& j){ j.progress=p; });
    });

    QObject::connect(&runner,&ChdmanRunner::jobLog,&jobs,[&](const QString& id,const QString& line){
        jobs.updateJob(id,[&](Job& j){ j.log += line + '\n'; });
    });

    QObject::connect(&runner,&ChdmanRunner::jobFinished,&jobs,[&](const QString& id,bool ok){
        auto &j = jobs.jobRefById(id);
        j.progress=100; j.status = ok ? "Done" : "Failed"; j.ended=QDateTime::currentDateTime();
        quint64 outB = (j.type==JobType::Create || j.type==JobType::Extract) ? QFileInfo(j.outputPath).size() : 0;
        auto it = rt.find(id);
        const qint64 msec = (it!=rt.end()) ? (QDateTime::currentMSecsSinceEpoch()-it->t0) : 0;
        const quint64 inB  = (it!=rt.end()) ? it->inB : 0;
        report.add({ id, ok, inB, outB, msec, j.inputPath, j.outputPath, j.status, j.log });
        rt.erase(id);
        if (ok && j.deleteSourceAfter) QFile::remove(j.inputPath);
    });

    return app.exec();
}
EOF

########################################
# ui/qml/Main.qml
########################################
cat > "$ROOT/ui/qml/Main.qml" <<'EOF'
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Native

ApplicationWindow {
    id: win
    visible: true
    width: 1100; height: 700
    title: "OpenDHC"
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    header: ToolBar {
        RowLayout { anchors.fill: parent; spacing: 12
            Label { text: "OpenDHC"; font.pixelSize: 18; Layout.margins: 8 }
            Item { Layout.fillWidth: true }
            Button { text: "Probe chdman"; onClicked: {
                const ok = runner.probeChdman();
                Native.MessageDialog { text: ok ? "chdman detected" : "chdman not found"; visible: true }
            } }
            ToolButton { text: "Settings"; onClicked: settingsPane.open() }
        }
    }

    Drawer {
        id: settingsPane; edge: Qt.RightEdge; width: 360
        ColumnLayout {
            anchors.fill: parent; spacing: 10; padding: 16
            Label { text: "Settings"; font.bold: true; font.pixelSize: 20 }
            RowLayout {
                TextField { id: chd; text: settings.chdmanPath; placeholderText: "Path to chdman"; Layout.fillWidth: true
                    onEditingFinished: settings.chdmanPath = text
                }
                Button {
                    text: "Browse"; onClicked: {
                        var f = Native.FileDialog { title: "Select chdman"; fileMode: Native.FileDialog.OpenFile; }
                        f.onAccepted.connect(() => { chd.text = f.file; settings.chdmanPath = f.file; });
                        f.open();
                    }
                }
            }
            RowLayout {
                TextField { id: outDir; text: settings.outputDir; placeholderText: "Default output folder"; Layout.fillWidth: true
                    onEditingFinished: settings.outputDir = text
                }
                Button {
                    text: "Browse"; onClicked: {
                        var d = Native.FolderDialog { title: "Select output folder" }
                        d.onAccepted.connect(() => { outDir.text = d.folder; settings.outputDir = d.folder; });
                        d.open();
                    }
                }
            }
            RowLayout {
                Label { text: "Concurrency" }
                Slider { id: conc; from: 1; to: 16; stepSize: 1; value: settings.concurrency; Layout.fillWidth: true
                    onMoved: settings.concurrency = Math.round(value)
                }
                Label { text: Math.round(conc.value).toString() }
            }
        }
    }

    SplitView {
        anchors.fill: parent; orientation: Qt.Vertical

        // Composer + Batch
        Frame {
            padding: 12
            ColumnLayout {
                spacing: 8
                RowLayout {
                    Label { text: "Job Type" }
                    ComboBox { id: jobType; model: ["Create","Verify","Info","Extract"] }
                    Label { text: "Media" }
                    ComboBox { id: media; model: ["CD","DVD"] }
                    CheckBox { id: delSrc; text: "Delete source on success" }
                    CheckBox { id: keepTree; checked: true; text: "Preserve folder structure" }
                    Item { Layout.fillWidth: true }
                }
                RowLayout {
                    TextField { id: input; placeholderText: "Input file(s) (semicolon separated)"; Layout.fillWidth: true }
                    Button {
                        text: "Browse"; onClicked: {
                            var f = Native.FileDialog { title: "Select input(s)"; fileMode: Native.FileDialog.OpenFiles }
                            f.onAccepted.connect(() => { input.text = f.files.join(";"); });
                            f.open();
                        }
                    }
                }
                RowLayout {
                    TextField { id: output; placeholderText: "Output folder / file"; text: settings.outputDir; Layout.fillWidth: true }
                    Button {
                        text: "Browse"; onClicked: {
                            var d = Native.FolderDialog { title: "Select output folder" }
                            d.onAccepted.connect(() => { output.text = d.folder; });
                            d.open();
                        }
                    }
                }
                RowLayout {
                    CheckBox { id: adv; text: "Advanced options" }
                    TextField { id: codecs; enabled: adv.checked; placeholderText: "Compression (-c), e.g. lzma,flac" ; Layout.fillWidth: true }
                    SpinBox  { id: hs; enabled: adv.checked; from: 0; to: 65536; value: 0; editable: true }
                    Label { text: "Hunk Size (-hs)"; visible: adv.checked }
                    SpinBox  { id: np; enabled: adv.checked; from: 1; to: 32; value: 4; editable: true }
                    Label { text: "Threads (-np)"; visible: adv.checked }
                }
                RowLayout {
                    Button {
                        text: "Add to Queue"; onClicked: {
                            const files = input.text ? input.text.split(";") : [];
                            const extra = [];
                            if (adv.checked && codecs.text.length>0) { extra.push("-c"); extra.push(codecs.text); }
                            if (adv.checked && hs.value>0) { extra.push("-hs"); extra.push(String(hs.value)); }
                            if (adv.checked && np.value>0) { extra.push("-np"); extra.push(String(np.value)); }
                            for (const f of files) {
                                const id = jobModel.addJob(jobType.currentIndex, media.currentIndex, f, output.text, extra, delSrc.checked, keepTree.checked);
                                runner.enqueueSimple(id, jobType.currentIndex, media.currentIndex, f, output.text, extra, delSrc.checked, keepTree.checked);
                            }
                        }
                    }
                    Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: "#333"; Layout.fillWidth: true }

                // Batch from folder
                RowLayout {
                    Label { text: "Batch Source" }
                    TextField { id: batchSource; placeholderText: "Source folder"; Layout.fillWidth: true }
                    Button {
                        text: "Folder"; onClicked: {
                            var d = Native.FolderDialog { title: "Choose source folder" }
                            d.onAccepted.connect(() => batchSource.text = d.folder); d.open();
                        }
                    }
                    CheckBox { id: batchRecursive; text: "Recursive"; checked: true }
                    CheckBox { id: batchCD; text: "CD (.cue/.toc/.gdi)"; checked: true }
                    CheckBox { id: batchDVD; text: "DVD (.iso)"; checked: true }
                    Button {
                        text: "Add Batch"; onClicked: {
                            const extra = [];
                            if (adv.checked && codecs.text.length>0) { extra.push("-c"); extra.push(codecs.text); }
                            if (adv.checked && hs.value>0) { extra.push("-hs"); extra.push(String(hs.value)); }
                            if (adv.checked && np.value>0) { extra.push("-np"); extra.push(String(np.value)); }
                            var inputs = scanner.findInputs(batchSource.text, batchRecursive.checked, batchCD.checked, batchDVD.checked);
                            for (let f of inputs) {
                                const ext = f.split('.').pop().toLowerCase();
                                const med = (ext === "iso") ? 1 : 0;
                                const out = scanner.defaultOutputForInvokable(f, jobType.currentIndex, med, output.text, keepTree.checked);
                                const id = jobModel.addJob(jobType.currentIndex, med, f, out, extra, delSrc.checked, keepTree.checked);
                                runner.enqueueSimple(id, jobType.currentIndex, med, f, out, extra, delSrc.checked, keepTree.checked);
                            }
                        }
                    }
                    Button { text: "Final Report"; onClicked: reportDialog.open() }
                }
            }
        }

        // Queue view
        Frame {
            padding: 0
            ListView {
                anchors.fill: parent; model: jobModel; clip: true
                delegate: Frame {
                    width: ListView.view.width; padding: 12; background: Rectangle { color: "#121315" ; radius: 8 }
                    ColumnLayout {
                        spacing: 6
                        RowLayout {
                            Label { text: "Job: " + id; font.bold: true }
                            Label { text: ["Create","Verify","Info","Extract"][type] + " • " + (media===0?"CD":"DVD"); color: "#8aa" }
                            Item { Layout.fillWidth: true }
                            Label { text: status; color: status==="Failed"?"#e66":(status==="Done"?"#6e6":"#ccc") }
                        }
                        ProgressBar { value: progress/100.0 }
                        TextArea { text: log; readOnly: true; wrapMode: Text.WordWrap; height: 90 }
                        RowLayout {
                            Item { Layout.fillWidth: true }
                            Button { text: "Remove"; onClicked: jobModel.removeJob(id) }
                        }
                    }
                }
            }
        }
    }

    // Final report dialog
    Dialog {
        id: reportDialog; modal: true; title: "Final Report"; standardButtons: Dialog.Ok
        width: Math.min(1000, parent.width*0.9); height: Math.min(600, parent.height*0.9)
        contentItem: ColumnLayout {
            spacing: 8; padding: 12
            Label { text: `Total: ${report.total} • OK: ${report.ok} • Failed: ${report.failed} • Saved: ${report.savedPct.toFixed(1)}%` }
            TextArea { id: md; wrapMode: TextArea.NoWrap; readOnly: true; text: report.asMarkdown(); font.family: "monospace"; Layout.fillWidth: true; Layout.fillHeight: true }
            RowLayout {
                Item { Layout.fillWidth: true }
                Button {
                    text: "Copy Markdown"
                    onClicked: Qt.callLater(() => Qt.application.clipboard.setText(md.text))
                }
                Button {
                    text: "Save as CSV"
                    onClicked: {
                        var fd = Native.FileDialog { title: "Save CSV"; fileMode: Native.FileDialog.SaveFile; nameFilters: ["CSV (*.csv)"] }
                        fd.onAccepted.connect(() => {
                            if (!report.saveCsv(fd.file)) {
                                Native.MessageDialog { text: "Could not write CSV"; visible: true }
                            }
                        })
                        fd.open()
                    }
                }
            }
        }
    }

    // Drag & drop
    DropArea {
        anchors.fill: parent
        onDropped: (e) => {
            if (e.hasUrls) {
                input.text = e.urls.map(u => u.toString().replace("file://",""))
                                   .join(";")
                e.acceptProposedAction();
            }
        }
    }
}
EOF

########################################
# packaging/AppRun
########################################
cat > "$ROOT/packaging/AppRun" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
export APPDIR="${HERE}"
exec "$APPDIR/usr/bin/OpenDHC" "$@"
EOF
chmod +x "$ROOT/packaging/AppRun"

########################################
# packaging/opendhc.desktop
########################################
cat > "$ROOT/packaging/opendhc.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=OpenDHC
Comment=Linux-native frontend for chdman (create/verify/extract CHD)
Exec=OpenDHC
Icon=opendhc
Categories=Utility;Graphics;Qt;
Terminal=false
EOF

########################################
# packaging/opendhc.png (1x1 placeholder)
########################################
mkdir -p "$ROOT/packaging"
base64 -d > "$ROOT/packaging/opendhc.png" <<'B64'
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8/5+hHgAHVgL1Jr9lYgAAAABJRU5ErkJggg==
B64

########################################
# packaging/build-appimage.sh (zsync-enabled)
########################################
cat > "$ROOT/packaging/build-appimage.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

rm -rf build AppDir *.AppImage *.zsync
mkdir -p build

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j
cmake --install build --prefix /usr --config Release -- DESTDIR=AppDir

install -Dm755 packaging/AppRun           AppDir/AppRun
install -Dm644 packaging/opendhc.desktop  AppDir/opendhc.desktop
install -Dm644 packaging/opendhc.png      AppDir/opendhc.png

LD=linuxdeploy-x86_64.AppImage
QT=linuxdeploy-plugin-qt-x86_64.AppImage
curl -L -o "$LD" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
curl -L -o "$QT" https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x "$LD" "$QT"

export VERSION=${VERSION:-"dev"}
export LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
# If CI doesn't override this, use a generic placeholder (will still embed update info).
export LDAI_UPDATE_INFORMATION="${LDAI_UPDATE_INFORMATION:-gh-releases-zsync|owner|repo|latest|*x86_64.AppImage.zsync}"
export QML_SOURCES_PATHS=ui

./"$LD" --appdir AppDir \
  --executable AppDir/usr/bin/OpenDHC \
  --desktop-file AppDir/opendhc.desktop \
  --icon-file packaging/opendhc.png \
  --plugin qt \
  --output appimage

echo "Artifacts:"
ls -1 *.AppImage *.zsync 2>/dev/null || true
EOF
chmod +x "$ROOT/packaging/build-appimage.sh"

########################################
# .github/workflows/appimage.yml
########################################
cat > "$ROOT/.github/workflows/appimage.yml" <<'EOF'
name: Build AppImage

on:
  push:
    tags: ["v*.*.*"]
  workflow_dispatch:

jobs:
  appimage:
    runs-on: ubuntu-latest
    env:
      VERSION: ${{ github.ref_name }}
      LDAI_UPDATE_INFORMATION: gh-releases-zsync|${{ github.repository_owner }}|${{ github.event.repository.name }}|latest|*x86_64.AppImage.zsync
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: true

      - name: Install dependencies (Qt + build tools + zsyncmake)
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake build-essential \
            qt6-base-dev qt6-declarative-dev qml6-module-qtquick-controls2 qml6-module-qt-labs-platform \
            patchelf zsync curl

      - name: Build AppImage (+ .zsync)
        run: bash packaging/build-appimage.sh

      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: OpenDHC-${{ env.VERSION }}
          path: |
            ./*.AppImage
            ./*.zsync

      - name: Create GitHub Release
        if: startsWith(github.ref, 'refs/tags/')
        uses: softprops/action-gh-release@v2
        with:
          files: |
            ./*.AppImage
            ./*.zsync
          fail_on_unmatched_files: true
EOF

########################################
# .gitignore
########################################
cat > "$ROOT/.gitignore" <<'EOF'
build/
AppDir/
*.AppImage
*.zsync
CMakeCache.txt
CMakeFiles/
*.user
*.pro.user
*.qmlc
*.jsc
EOF

########################################
# LICENSE (MIT)
########################################
cat > "$ROOT/LICENSE" <<'EOF'
MIT License

Copyright (c) 2025 OpenDHC

Permission is hereby granted, free of charge, to any person obtaining a copy
...
(You can replace with your preferred name/year.)
EOF

########################################
# README.md
########################################
cat > "$ROOT/README.md" <<'EOF'
# OpenDHC (Linux-native chdman frontend)

Qt 6 GUI for `chdman` (create/verify/info/extract CHD), with batch processing, per-job progress, final report, and CSV export.  
Includes AppImage packaging via linuxdeploy + plugin-qt, with zsync auto-updates from GitHub Releases.

## Build (Linux)

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential \
  qt6-base-dev qt6-declarative-dev qml6-module-qtquick-controls2 qml6-module-qt-labs-platform \
  mame-tools  # provides chdman
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/OpenDHC
```

## AppImage (local)

```bash
bash packaging/build-appimage.sh
```

Produces `OpenDHC-<VERSION>-x86_64.AppImage` and a matching `.zsync` (update metadata embedded).

## CI (GitHub Actions)

- On tags `v*.*.*` the workflow builds the AppImage and `.zsync`, uploads artifacts, and attaches them to the Release.
- The AppImage embeds update info for GitHub Releases (zsync).

## Notes

- Set **chdman** path in *Settings* (or ensure `chdman` is on PATH).
- Use **DVD** for `.iso` (`createdvd`), **CD** for `.cue/.toc/.gdi` (`createcd`).
- Replace `packaging/opendhc.png` with a proper 256×256 icon when you can.

License: MIT
EOF

########################################
# zip it
########################################
( cd "$(dirname "$ROOT")" && zip -qr "$ZIP_OUT" "$(basename "$ROOT")" )
echo "ZIP created: $ZIP_OUT"

```

---

### 10) User
> Can I export this chat or import it into my personal Copilot account?

### Assistant
Shared several export options; recommended **Option #3** (Word/OneNote/Copilot web) for grounding.

---

### 11) User
> Can we do option #3?

### Assistant
Provided detailed steps for Word Online and OneNote usage with Copilot.

---

### 12) User
> Please export a markdown of this entire chat

### Assistant
Produced a Markdown export.

---

### 13) User
> Generate a variant with inline code

### Assistant
This is that inline variant.

---
