#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class DetailHierarchySelectionController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(QObject* sourceController READ sourceController WRITE setSourceController NOTIFY sourceControllerChanged)

public:
    explicit DetailHierarchySelectionController(
        const QString& objectName,
        QObject* parent = nullptr);

    QVariantList hierarchyModel() const;
    int selectedIndex() const noexcept;
    int itemCount() const noexcept;
    QObject* sourceController() const noexcept;

    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE void setSelectedIndex(int index);
    void setSourceController(QObject* sourceController);

public
    slots  :




    void synchronizeFromSourceChange()
    {
        synchronizeFromSource(false);
    }

    void synchronizeSelectionFromSourceChange()
    {
        synchronizeFromSource(true);
    }

    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

signals:
    void hierarchyModelChanged();
    void selectedIndexChanged();
    void itemCountChanged();
    void sourceControllerChanged();
    void controllerHookRequested();

private:
    void disconnectSourceSignals();
    void synchronizeFromSource(bool initializeSelectionFromSource);
    static QVariantList resolveHierarchyModelFromSource(const QObject* sourceController);
    static int resolveSelectedIndexFromSource(const QObject* sourceController);
    static QString selectionKeyForIndex(const QVariantList& hierarchyModel, int index);
    static int indexForSelectionKey(const QVariantList& hierarchyModel, const QString& selectionKey);
    static int normalizedIndexForModel(const QVariantList& hierarchyModel, int index) noexcept;

    QPointer<QObject> m_sourceController;
    QVariantList m_hierarchyModel;
    int m_selectedIndex = -1;
    QMetaObject::Connection m_sourceHierarchyModelChangedConnection;
    QMetaObject::Connection m_sourceSelectedIndexChangedConnection;
    QMetaObject::Connection m_sourceDestroyedConnection;
};
