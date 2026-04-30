#pragma once

#include <QObject>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsBreakBlockController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsBreakBlockController)

    Q_PROPERTY(QObject* breakBlock READ breakBlock WRITE setBreakBlock NOTIFY breakBlockChanged FINAL)
    Q_PROPERTY(QObject* divider READ divider WRITE setDivider NOTIFY dividerChanged FINAL)

public:
    explicit ContentsBreakBlockController(QObject* parent = nullptr);

    QObject* breakBlock() const noexcept;
    void setBreakBlock(QObject* value);

    QObject* divider() const noexcept;
    void setDivider(QObject* value);

    Q_INVOKABLE void selectBreakBlock() const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE QVariantList logicalLineLayoutEntries() const;
    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE bool handleKeyPress(QObject* event) const;
    Q_INVOKABLE void handleTap(int tapCount) const;

signals:
    void breakBlockChanged();
    void dividerChanged();

private:
    QObject* m_breakBlock = nullptr;
    QObject* m_divider = nullptr;
};
