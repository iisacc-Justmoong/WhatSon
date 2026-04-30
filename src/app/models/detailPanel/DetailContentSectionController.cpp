#include "app/models/detailPanel/DetailContentSectionController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

DetailContentSectionController::DetailContentSectionController(
    DetailContentState state,
    QObject* parent)
    : QObject(parent)
      , m_state(state)
{
    setObjectName(QStringLiteral("DetailContent.%1").arg(WhatSon::DetailPanel::stateName(m_state)));
    const QString detail = QStringLiteral("stateName=%1 stateValue=%2")
                           .arg(stateName())
                           .arg(stateValue());
    WhatSon::Debug::traceSelf(this, QStringLiteral("detail.content.controller"), QStringLiteral("ctor"), detail);
}

bool DetailContentSectionController::active() const noexcept
{
    return m_active;
}

int DetailContentSectionController::stateValue() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_state);
}

QString DetailContentSectionController::stateName() const
{
    return WhatSon::DetailPanel::stateName(m_state);
}

void DetailContentSectionController::setActive(bool active)
{
    if (m_active == active)
    {
        const QString detail = QStringLiteral("stateName=%1 active=%2 result=ignored_same")
                               .arg(stateName())
                               .arg(m_active ? QStringLiteral("true") : QStringLiteral("false"));
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.content.controller"),
            QStringLiteral("setActive.ignoredSame"),
            detail);
        return;
    }

    m_active = active;
    const QString detail = QStringLiteral("stateName=%1 active=%2")
                           .arg(stateName())
                           .arg(m_active ? QStringLiteral("true") : QStringLiteral("false"));
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.content.controller"),
        QStringLiteral("setActive.applied"),
        detail);
    emit activeChanged();
}
