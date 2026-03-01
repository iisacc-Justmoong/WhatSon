#include "EventHierarchyViewModel.hpp"

#include "file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

EventHierarchyViewModel::EventHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    setEventNames({});
}

EventHierarchyViewModel::~EventHierarchyViewModel() = default;

FlatHierarchyModel* EventHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int EventHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void EventHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

void EventHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Event"));
    syncModel();
    setSelectedIndex(-1);
}

QVariantList EventHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

void EventHierarchyViewModel::setEventNames(QStringList eventNames)
{
    m_eventNames = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(eventNames));
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Event"),
        m_eventNames,
        QStringLiteral("Event"));
    syncModel();
    setSelectedIndex(-1);
}

QStringList EventHierarchyViewModel::eventNames() const
{
    return m_eventNames;
}

bool EventHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::Support::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        return false;
    }

    QStringList aggregated;

    WhatSonEventHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Event.wsevent"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::Support::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonEventHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : store.eventNames())
        {
            aggregated.push_back(value);
        }
    }

    setEventNames(aggregated);
    return true;
}

void EventHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}
