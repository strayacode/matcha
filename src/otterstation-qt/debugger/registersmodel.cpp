#include "otterstation-qt/debugger/registersmodel.h"
#include "core/ee/disassembler.h"
#include "core/ee/ee_core.h"

RegistersModel::RegistersModel(QObject* parent, EECore& ee_core) : QAbstractListModel(parent), ee_core(ee_core) {}

int RegistersModel::rowCount(const QModelIndex&) const {
   return 33;
}

int RegistersModel::columnCount(const QModelIndex&) const {
    return 2;
}

QVariant RegistersModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignLeft;
    }
             
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
    case 0:
        switch (index.row()) {
        case 32:
            return "pc";
        default:
            return QString::fromStdString(EEGetRegisterName(index.row()));
        }
        
        break;
    case 1:
        switch (index.row()) {
        case 32:
            return QString::asprintf("%08X", ee_core.pc);
        default:
            // TODO: show upper 64 bits as well
            return QString::asprintf("%016lX", ee_core.GetReg<u64>(index.row()));
        }

        break;
    }

    return QVariant();
}

QVariant RegistersModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return QVariant();
    }
 
    switch (section) {
    case 0:
        return tr("Register");
    case 1:
        return tr("Value");
    }

    return QVariant();
}