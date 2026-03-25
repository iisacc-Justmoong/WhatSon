#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class DetailHierarchySelectionViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(QObject* sourceViewModel READ sourceViewModel WRITE setSourceViewModel NOTIFY sourceViewModelChanged)

public:
    explicit DetailHierarchySelectionViewModel(
        const QString& objectName,
        QObject* parent = nullptr);

    QVariantList hierarchyModel() const;
    int selectedIndex() const noexcept;
    int itemCount() const noexcept;
    QObject* sourceViewModel() const noexcept;

    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE void setSelectedIndex(int index);
    void setSourceViewModel(QObject* sourceViewModel);

public
    slots  :




    void synchronizeFromSourceChange()
    {
        synchronizeFromSource(false);
    }

    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

signals:
    void hierarchyModelChanged();
    void selectedIndexChanged();
    void itemCountChanged();
    void sourceViewModelChanged();
    void viewModelHookRequested();

private:
    void disconnectSourceSignals();
    void synchronizeFromSource(bool initializeSelectionFromSource);
    static QVariantList resolveHierarchyModelFromSource(const QObject* sourceViewModel);
    static int resolveSelectedIndexFromSource(const QObject* sourceViewModel);
    static QString selectionKeyForIndex(const QVariantList& hierarchyModel, int index);
    static int indexForSelectionKey(const QVariantList& hierarchyModel, const QString& selectionKey);
    static int normalizedIndexForModel(const QVariantList& hierarchyModel, int index) noexcept;

    QPointer<QObject> m_sourceViewModel;
    QVariantList m_hierarchyModel;
    int m_selectedIndex = -1;
    QMetaObject::Connection m_sourceHierarchyModelChangedConnection;
    QMetaObject::Connection m_sourceDestroyedConnection;
};
