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
                [his,id=pr.id{
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
