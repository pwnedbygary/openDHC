#pragma once
#include "Job.hpp"
#include <QString>
#include <QtGlobal>

namespace SizeUtil {
    quint64 estimateInputBytes(const Job& j);
    quint64 safeFileSize(const QString& path);
}
