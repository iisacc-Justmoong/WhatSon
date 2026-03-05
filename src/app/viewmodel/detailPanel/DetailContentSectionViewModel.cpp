#include "DetailContentSectionViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

DetailContentSectionViewModel::DetailContentSectionViewModel(
    DetailContentState state,
    QObject* parent)
    : QObject(parent)
      , m_state(state)
{
    setObjectName(QStringLiteral("DetailContent.%1").arg(WhatSon::DetailPanel::stateName(m_state)));
    const QString detail = QStringLiteral("stateName=%1 stateValue=%2")
                           .arg(stateName())
                           .arg(stateValue());
    WhatSon::Debug::traceSelf(this, QStringLiteral("detail.content.viewmodel"), QStringLiteral("ctor"), detail);
}

bool DetailContentSectionViewModel::active() const noexcept
{
    return m_active;
}

int DetailContentSectionViewModel::stateValue() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_state);
}

QString DetailContentSectionViewModel::stateName() const
{
    return WhatSon::DetailPanel::stateName(m_state);
}

void DetailContentSectionViewModel::setActive(bool active)
{
    if (m_active == active)
    {
        const QString detail = QStringLiteral("stateName=%1 active=%2 result=ignored_same")
                               .arg(stateName())
                               .arg(m_active ? QStringLiteral("true") : QStringLiteral("false"));
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.content.viewmodel"),
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
        QStringLiteral("detail.content.viewmodel"),
        QStringLiteral("setActive.applied"),
        detail);
    emit activeChanged();
}
