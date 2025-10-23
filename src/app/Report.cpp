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
