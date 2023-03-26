#pragma once

#include <set>

#include <QAbstractTableModel>

class DataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    typedef std::map<std::string,double> Params;
    typedef std::map<std::string,Params> Variants;
    typedef std::set<std::string> ParamLabels;

public:
    explicit DataModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

public:
    void AddVariant(const Params& params);
//    Variants& GetModel();
//    const Variants& GetModel() const;
    const ParamLabels& GetParamLabels() const;

private:
    std::string GetVariantUniqueName(const std::string& start_string = std::string()) const;

private:
    Variants m_variants;
    ParamLabels m_paramLabels;
};
