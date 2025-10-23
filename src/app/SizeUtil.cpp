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
