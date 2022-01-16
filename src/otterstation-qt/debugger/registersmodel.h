#pragma once

#include <QAbstractListModel>

class EECore;

class RegistersModel : public QAbstractListModel {
    Q_OBJECT
public:
    RegistersModel(QObject* parent, EECore& ee_core);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;

private:
    EECore& ee_core;

protected:
};