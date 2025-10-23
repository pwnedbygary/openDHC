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
