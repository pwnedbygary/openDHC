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
