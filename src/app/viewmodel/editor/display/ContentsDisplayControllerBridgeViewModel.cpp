#include "app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"

#include <QMetaObject>

ContentsDisplayControllerBridgeViewModel::ContentsDisplayControllerBridgeViewModel(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayControllerBridgeViewModel::~ContentsDisplayControllerBridgeViewModel() = default;

QObject* ContentsDisplayControllerBridgeViewModel::controller() const noexcept
{
    return m_controller.data();
}

void ContentsDisplayControllerBridgeViewModel::setController(QObject* controller)
{
    if (m_controller == controller)
    {
        return;
    }

    m_controller = controller;
    emit controllerChanged();
}

QVariant ContentsDisplayControllerBridgeViewModel::invokeController(
    const char* methodName,
    const QVariantList& arguments) const
{
    if (m_controller == nullptr || methodName == nullptr)
    {
        return {};
    }

    QVariant result;
    const auto argumentCount = arguments.size();
    switch (argumentCount)
    {
    case 0:
        QMetaObject::invokeMethod(m_controller, methodName, Q_RETURN_ARG(QVariant, result));
        break;
    case 1:
        QMetaObject::invokeMethod(
            m_controller,
            methodName,
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(QVariant, arguments.at(0)));
        break;
    case 2:
        QMetaObject::invokeMethod(
            m_controller,
            methodName,
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(QVariant, arguments.at(0)),
            Q_ARG(QVariant, arguments.at(1)));
        break;
    case 3:
        QMetaObject::invokeMethod(
            m_controller,
            methodName,
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(QVariant, arguments.at(0)),
            Q_ARG(QVariant, arguments.at(1)),
            Q_ARG(QVariant, arguments.at(2)));
        break;
    case 4:
        QMetaObject::invokeMethod(
            m_controller,
            methodName,
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(QVariant, arguments.at(0)),
            Q_ARG(QVariant, arguments.at(1)),
            Q_ARG(QVariant, arguments.at(2)),
            Q_ARG(QVariant, arguments.at(3)));
        break;
    default:
        break;
    }
    return result;
}

bool ContentsDisplayControllerBridgeViewModel::invokeControllerBool(
    const char* methodName,
    const QVariantList& arguments) const
{
    return invokeController(methodName, arguments).toBool();
}

void ContentsDisplayControllerBridgeViewModel::invokeControllerVoid(
    const char* methodName,
    const QVariantList& arguments) const
{
    Q_UNUSED(invokeController(methodName, arguments));
}
