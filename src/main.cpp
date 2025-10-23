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
        url {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    // Track job runtime and sizes for final report
    struct Runtime { qint64 t0=0; quint64 inB=0; };
    QHash<QString, Runtime> rt;

    QObject::connect(&runner,&ChdmanRunner::jobStarted,&jobs,&{
        jobs.updateJob(id,{ j.status="Running"; j.started=QDateTime::currentDateTime(); });
        auto &j = jobs.jobRefById(id);
        rt[id] = { QDateTime::currentMSecsSinceEpoch(), SizeUtil::estimateInputBytes(j) };
    });

    QObject::connect(&runner,&ChdmanRunner::jobProgress,&jobs,&{
        jobs.updateJob(id,[&](Job& jess=p; });
    });

    QObject::connect(&runner,&ChdmanRunner::jobLog,&jobs,&{
        jobs.updateJob(id,&{ j.log += line + '\n'; });
    });

    QObject::connect(&runner,&ChdmanRunner::jobFinished,&jobs,&{
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
