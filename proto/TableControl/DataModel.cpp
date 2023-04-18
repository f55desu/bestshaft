#include "DataModel.h"
//#include "qsize.h"

DataModel::DataModel(QObject *parent)
    : QAbstractTableModel{parent}
{
}

int DataModel::rowCount(const QModelIndex & /* parent */) const
{
    return (int)m_variants.size();
}

int DataModel::columnCount(const QModelIndex & /* parent */) const
{
//    auto pos = std::ranges::max_element(m_variantCol,
//                                        [](const auto& a, const auto& b) {return a.second.size() < b.second.size();});

    return m_paramLabels.empty() ? 0 : (int)m_paramLabels.size() + 1;
}
QVariant DataModel::data(const QModelIndex &index, int role) const
{
//    if (!(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)))
//        return QVariant();

//    if(index.column() == 0)
//        return QString::fromStdString(std::next(m_variants.begin(), index.row())->first);

//    std::string label = *std::next(m_paramLabels.begin(), index.column() - 1);
//    Params params = std::next(m_variants.begin(), index.row())->second;
//    return params.contains(label) ?
//                ((label == "VonMises" && params.at(label) == .0) ?
//                     "--" : QString::number(params.at(label)))
//              : "--";
    return QVariant();
}
QVariant DataModel::headerData(int section,
                                Qt::Orientation orientation,
                                int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole)
        return section + 1;

    /*if (role == Qt::SizeHintRole)
        return QSize(1, 1);*/

    if(section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return "Var\\Par";

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QString::fromStdString(*std::next(m_paramLabels.begin(), section - 1));

    return QVariant();
}

Qt::ItemFlags DataModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

//    if (index.column() == 0)
//        return flags | Qt::ItemIsEditable;

//    if (index.column() != (int)m_paramLabels.size())
//    {
//        std::string label = *std::next(m_paramLabels.begin(), index.column() - 1);
//        Params params = std::next(m_variants.begin(), index.row())->second;

//        return params.contains(label) ? flags | Qt::ItemIsEditable : flags;
//    }

    return flags;
}

bool DataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!(index.isValid() && role == Qt::EditRole))
        return false;

    if (index.column() == 0)
    {
        Params params = std::next(m_variants.begin(), index.row())->second;
        m_variants.erase(std::next(m_variants.begin(), index.row()));
        m_variants.insert(std::pair(GetVariantUniqueName(value.toString().toStdString()),params));
    }
    else
    {
        std::string label = *std::next(m_paramLabels.begin(), index.column() - 1);
        Params& params = std::next(m_variants.begin(), index.row())->second;
        params[label] = value.toDouble();
    }

    emit dataChanged(index, index/*, {Qt::DisplayRole, Qt::EditRole}*/);

    return true;
}

bool DataModel::insertRows(int position, int count, const QModelIndex& parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), position, position + count - 1);

    for (int row = 0; row < count; ++row)
    {
        DataModel::Params params{std::pair("VonMises",.0)};
        m_variants.insert(std::pair(GetVariantUniqueName(),params));
    }

    endInsertRows();
    return true;
}

//DataModel::Variants& DataModel::GetModel()
//{
//    return m_variants;
//}

//const DataModel::Variants& DataModel::GetModel() const
//{
//    return m_variants;
//}

const DataModel::ParamLabels& DataModel::GetParamLabels() const
{
    return m_paramLabels;
}

std::string DataModel::GetVariantUniqueName(const std::string& start_string) const
{
    std::string unique_name = start_string;

//    for(int n = 1; m_variants.find(unique_name = unique_name.empty() ?
//        std::format("Variant #{}",n) : unique_name ) != m_variants.end(); n++, unique_name.clear());

    return unique_name;
}

void DataModel::AddVariant(const Params& params)
{
    //std::vector<std::string> params_in, initial_labels(m_paramLabels);
    //params_in.reserve(params.size());

    /*if(m_paramLabels.size() < params.size())
        insertColumns(m_paramLabels.size(), (int)params.size() - m_paramLabels.size(), QModelIndex());
    insertRows(rowCount(), 1, QModelIndex());*/

    beginInsertColumns(QModelIndex(),0,(int)params.size());
    //std::ranges::for_each(params, [this](const auto& a){ m_paramLabels.insert(a.first); });
    endInsertColumns();
    //insertColumns(0, m_paramLabels.size(), QModelIndex());
    return;
    //std::set_union(initial_labels.begin(),initial_labels.end(),params_in.begin(), params_in.end(),std::back_inserter(m_paramLabels));
//    m_paramLabels = union_set;

    /*std::vector<std::pair<std::string,double>> acc(params.begin(),params.end());

    std::ranges::for_each(m_variants, [&acc](const auto& variant){
        std::vector<std::pair<std::string,double>> result;
        std::set_union(acc.begin(),acc.end(),variant.second.begin(),variant.second.end(), std::back_inserter(result),
                       []( const auto & x, const auto & y ){ return x.first < y.first; });
        acc = result;
    });*/


    std::string name = GetVariantUniqueName();
    m_variants.insert(std::pair(name,params));


    QModelIndex index = this->index(rowCount() - 1, 0, QModelIndex());
    setData(index, QString::fromStdString(name), Qt::EditRole);
    for(const auto& p: params)
    {
        index = this->index(index.row(), index.column() + 1, QModelIndex());
        setData(index, p.second, Qt::EditRole);
    }

    /*m_paramLabels.clear();
    m_paramLabels.reserve(acc.size());
    std::ranges::for_each(acc, [this](const auto& a){ m_paramLabels.push_back(a.first); });*/
}
