#pragma once

#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QVariantList>

class ContentsDisplayControllerBridgeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* controller READ controller WRITE setController NOTIFY controllerChanged)

public:
    explicit ContentsDisplayControllerBridgeViewModel(QObject* parent = nullptr);
    ~ContentsDisplayControllerBridgeViewModel() override;

    [[nodiscard]] QObject* controller() const noexcept;

public slots:
    void setController(QObject* controller);

signals:
    void controllerChanged();

protected:
    [[nodiscard]] QVariant invokeController(const char* methodName, const QVariantList& arguments = {}) const;
    [[nodiscard]] bool invokeControllerBool(const char* methodName, const QVariantList& arguments = {}) const;
    void invokeControllerVoid(const char* methodName, const QVariantList& arguments = {}) const;

private:
    QPointer<QObject> m_controller;
};
