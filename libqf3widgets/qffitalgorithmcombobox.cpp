#include "qffitalgorithmcombobox.h"
#include "qffitalgorithmmanager.h"

QFFitAlgorithmComboBox::QFFitAlgorithmComboBox(QWidget *parent) :
    QComboBox(parent)
{
    updateFitAlgorithms();
}

QString QFFitAlgorithmComboBox::currentFitAlgorithmID() const
{
    return itemData(currentIndex()).toString();
}

QFFitAlgorithm *QFFitAlgorithmComboBox::createCurrentInstance(QObject *parent) const
{
    QFFitAlgorithmManager* manager=QFFitAlgorithmManager::getInstance();
    return manager->createAlgorithm(currentFitAlgorithmID(), parent);

}

void QFFitAlgorithmComboBox::setCurrentAlgorithm(const QString &id)
{
    int idx=findData(id);
    if (idx>=0) setCurrentIndex(idx);
    else setCurrentIndex(0);
}

void QFFitAlgorithmComboBox::updateFitAlgorithms()
{
    QFFitAlgorithmManager* manager=QFFitAlgorithmManager::getInstance();
    bool upd=updatesEnabled();
    setUpdatesEnabled(false);
    clear();
    QStringList m_ids=manager->getIDList();
    for (int i=0; i<m_ids.size(); i++) {
        QFFitAlgorithm* a=manager->createAlgorithm(m_ids[i], this);
        if (a->isThreadSafe()) addItem(QIcon(":/lib/fitalg_icon_mt.png"), a->name() , m_ids[i]);
        else addItem(QIcon(":/lib/fitalg_icon.png"), a->name() , m_ids[i]);
        delete a;
    }
    model()->sort(0);
    setUpdatesEnabled(upd);

}
