#include "app/models/detailPanel/DetailNoteHeaderSelectionSourceController.hpp"

#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QRegularExpression>
#include <QVariantMap>

namespace
{
    QString normalizeProgressLabelKey(QString value)
    {
        value = value.trimmed().toLower();
        value.remove(QLatin1Char(' '));
        value.remove(QLatin1Char('-'));
        value.remove(QLatin1Char('_'));
        return value;
    }

    int fallbackProgressValueForLabel(const QString& label)
    {
        const QString normalizedLabel = normalizeProgressLabelKey(label);
        if (normalizedLabel == QStringLiteral("ready")
            || normalizedLabel == QStringLiteral("firstdraft"))
        {
            return 0;
        }
        if (normalizedLabel == QStringLiteral("modifieddraft"))
        {
            return 1;
        }
        if (normalizedLabel == QStringLiteral("inprogress"))
        {
            return 2;
        }
        if (normalizedLabel == QStringLiteral("pending"))
        {
            return 3;
        }
        if (normalizedLabel == QStringLiteral("reviewing"))
        {
            return 4;
        }
        if (normalizedLabel == QStringLiteral("waitingforapproval"))
        {
            return 5;
        }
        if (normalizedLabel == QStringLiteral("done"))
        {
            return 6;
        }
        if (normalizedLabel == QStringLiteral("lagacy")
            || normalizedLabel == QStringLiteral("legacy"))
        {
            return 7;
        }
        if (normalizedLabel == QStringLiteral("archived"))
        {
            return 8;
        }
        if (normalizedLabel == QStringLiteral("deletereview"))
        {
            return 9;
        }

        return -1;
    }

    QString clearSelectionLabel(DetailNoteHeaderSelectionSourceController::Field field)
    {
        switch (field)
        {
        case DetailNoteHeaderSelectionSourceController::Field::Project:
            return QStringLiteral("No project");
        case DetailNoteHeaderSelectionSourceController::Field::Bookmark:
            return QStringLiteral("No bookmark");
        case DetailNoteHeaderSelectionSourceController::Field::Progress:
            return QStringLiteral("No progress");
        }

        return {};
    }

    QString clearSelectionKey(DetailNoteHeaderSelectionSourceController::Field field)
    {
        switch (field)
        {
        case DetailNoteHeaderSelectionSourceController::Field::Project:
            return QStringLiteral("detail:none:project");
        case DetailNoteHeaderSelectionSourceController::Field::Bookmark:
            return QStringLiteral("detail:none:bookmark");
        case DetailNoteHeaderSelectionSourceController::Field::Progress:
            return QStringLiteral("detail:none:progress");
        }

        return {};
    }

    QVariantMap clearSelectionEntry(DetailNoteHeaderSelectionSourceController::Field field)
    {
        QVariantMap entry{
            {QStringLiteral("key"), clearSelectionKey(field)},
            {QStringLiteral("label"), clearSelectionLabel(field)},
            {QStringLiteral("clearSelection"), true}
        };
        if (field == DetailNoteHeaderSelectionSourceController::Field::Progress)
        {
            entry.insert(QStringLiteral("progressValue"), -1);
        }
        return entry;
    }

    bool isClearSelectionEntry(const QVariantMap& entry)
    {
        return entry.value(QStringLiteral("clearSelection")).toBool();
    }

    QVariantMap findProgressHierarchyEntryByLabel(const QVariantList& optionsHierarchyModel, const QString& label)
    {
        const QString trimmedLabel = label.trimmed();
        for (const QVariant& option : optionsHierarchyModel)
        {
            const QVariantMap optionMap = option.toMap();
            if (optionMap.value(QStringLiteral("label")).toString().trimmed().compare(
                    trimmedLabel,
                    Qt::CaseInsensitive) == 0)
            {
                return optionMap;
            }
        }

        return {};
    }

    QVariantMap findProgressHierarchyEntryByValue(const QVariantList& optionsHierarchyModel, int progressValue)
    {
        for (const QVariant& option : optionsHierarchyModel)
        {
            const QVariantMap optionMap = option.toMap();
            if (DetailNoteHeaderSelectionSourceController::progressValueForHierarchyEntry(option, -1) == progressValue)
            {
                return optionMap;
            }
        }

        return {};
    }

    QVariantList buildHeaderBackedProgressHierarchyModel(
        const WhatSonNoteHeaderStore& header,
        const QVariantList& optionsHierarchyModel)
    {
        QVariantList hierarchyModel;
        const QStringList progressEnums = header.progressEnums();
        hierarchyModel.reserve(progressEnums.size());

        for (int index = 0; index < progressEnums.size(); ++index)
        {
            const QString label = progressEnums.at(index).trimmed();
            if (label.isEmpty())
            {
                continue;
            }

            QVariantMap optionMap = findProgressHierarchyEntryByLabel(optionsHierarchyModel, label);
            int fallbackProgressValue = -1;
            if (optionMap.isEmpty())
            {
                fallbackProgressValue = fallbackProgressValueForLabel(label);
                if (fallbackProgressValue >= 0)
                {
                    optionMap = findProgressHierarchyEntryByValue(optionsHierarchyModel, fallbackProgressValue);
                }
            }

            const int progressValue = optionMap.isEmpty()
                                          ? (fallbackProgressValue >= 0 ? fallbackProgressValue : index)
                                          : DetailNoteHeaderSelectionSourceController::progressValueForHierarchyEntry(
                                              optionMap,
                                              fallbackProgressValue >= 0 ? fallbackProgressValue : index);
            optionMap.remove(QStringLiteral("clearSelection"));
            optionMap.insert(QStringLiteral("key"), QStringLiteral("progress:%1").arg(progressValue));
            optionMap.insert(QStringLiteral("label"), label);
            optionMap.insert(QStringLiteral("itemId"), progressValue);
            optionMap.insert(QStringLiteral("progressValue"), progressValue);
            hierarchyModel.push_back(optionMap);
        }

        return hierarchyModel;
    }
}

DetailNoteHeaderSelectionSourceController::DetailNoteHeaderSelectionSourceController(Field field, QObject* parent)
    : QObject(parent)
    , m_field(field)
{
}

QVariantList DetailNoteHeaderSelectionSourceController::hierarchyModel() const
{
    return m_hierarchyModel;
}

int DetailNoteHeaderSelectionSourceController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

QString DetailNoteHeaderSelectionSourceController::noteId() const
{
    return m_noteId;
}

QString DetailNoteHeaderSelectionSourceController::noteDirectoryPath() const
{
    return m_noteDirectoryPath;
}

QObject* DetailNoteHeaderSelectionSourceController::optionsSourceController() const noexcept
{
    return m_optionsSourceController.data();
}

void DetailNoteHeaderSelectionSourceController::setSessionStore(IWhatSonNoteHeaderSessionStore* sessionStore)
{
    if (sessionStore != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::Controller,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("DetailNoteHeaderSelectionSourceController::setSessionStore")))
    {
        return;
    }

    if (sessionStore == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailNoteHeaderSelectionSourceController::setSessionStore")))
    {
        return;
    }

    m_sessionStore = sessionStore;
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceController::setNoteId(const QString& noteId)
{
    const QString normalized = noteId.trimmed();
    if (m_noteId == normalized)
    {
        return;
    }
    m_noteId = normalized;
    emit noteIdChanged();
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceController::setNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalized = noteDirectoryPath.trimmed();
    if (m_noteDirectoryPath == normalized)
    {
        return;
    }
    m_noteDirectoryPath = normalized;
    emit noteDirectoryPathChanged();
    synchronize(true);
}

void DetailNoteHeaderSelectionSourceController::setOptionsSourceController(QObject* optionsSourceController)
{
    if (optionsSourceController != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("DetailNoteHeaderSelectionSourceController::setOptionsSourceController")))
    {
        return;
    }

    if (optionsSourceController == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailNoteHeaderSelectionSourceController::setOptionsSourceController")))
    {
        return;
    }

    if (m_optionsSourceController == optionsSourceController)
    {
        return;
    }

    disconnectOptionsSourceSignals();
    m_optionsSourceController = optionsSourceController;
    if (m_optionsSourceController != nullptr)
    {
        m_optionsHierarchyModelChangedConnection = QObject::connect(
            m_optionsSourceController.data(),
            SIGNAL(hierarchyModelChanged()),
            this,
            SLOT(synchronizeOptionsSourceChange()));
        m_optionsDestroyedConnection = QObject::connect(
            m_optionsSourceController.data(),
            &QObject::destroyed,
            this,
            [this]()
            {
                disconnectOptionsSourceSignals();
                m_optionsSourceController = nullptr;
                emit optionsSourceControllerChanged();
                synchronize(false);
            });
    }
    emit optionsSourceControllerChanged();
    synchronize(false);
}

void DetailNoteHeaderSelectionSourceController::setSelectedIndex(int index)
{
    if (m_selectedIndex == index)
    {
        return;
    }
    if (!persistSelection(index))
    {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
}

void DetailNoteHeaderSelectionSourceController::synchronize(bool reloadSession)
{
    if (m_sessionStore != nullptr && reloadSession && !m_noteId.isEmpty() && !m_noteDirectoryPath.isEmpty())
    {
        m_sessionStore->ensureLoaded(m_noteId, m_noteDirectoryPath, nullptr);
    }

    const QVariantList optionsHierarchyModel = m_optionsSourceController != nullptr
                                                   ? m_optionsSourceController->property("hierarchyModel").toList()
                                                   : QVariantList{};
    QVariantList nextOptionsHierarchyModel = optionsHierarchyModel;
    if (m_field == Field::Progress
        && m_sessionStore != nullptr
        && !m_noteId.isEmpty()
        && m_sessionStore->hasEntry(m_noteId))
    {
        const WhatSonNoteHeaderStore header = m_sessionStore->header(m_noteId);
        if (optionsHierarchyModel.isEmpty() && !header.progressEnums().isEmpty())
        {
            // The canonical Progress hierarchy remains the option source when available.
            // Header enums only provide a fallback option list when no source model is injected.
            nextOptionsHierarchyModel = buildHeaderBackedProgressHierarchyModel(header, optionsHierarchyModel);
        }
    }

    const QVariantList nextHierarchyModel = buildHierarchyModelWithClearEntry(nextOptionsHierarchyModel);
    if (m_hierarchyModel != nextHierarchyModel)
    {
        m_hierarchyModel = nextHierarchyModel;
        emit hierarchyModelChanged();
    }

    const int nextSelectedIndex = resolveSelectedIndexForHeader(m_hierarchyModel);
    if (m_selectedIndex != nextSelectedIndex)
    {
        m_selectedIndex = nextSelectedIndex;
        emit selectedIndexChanged();
    }
}

void DetailNoteHeaderSelectionSourceController::disconnectOptionsSourceSignals()
{
    if (m_optionsHierarchyModelChangedConnection)
    {
        QObject::disconnect(m_optionsHierarchyModelChangedConnection);
        m_optionsHierarchyModelChangedConnection = QMetaObject::Connection();
    }
    if (m_optionsDestroyedConnection)
    {
        QObject::disconnect(m_optionsDestroyedConnection);
        m_optionsDestroyedConnection = QMetaObject::Connection();
    }
}

int DetailNoteHeaderSelectionSourceController::progressValueForHierarchyEntry(
    const QVariant& entry,
    int fallbackProgressValue)
{
    const QVariantMap entryMap = entry.toMap();
    if (isClearSelectionEntry(entryMap))
    {
        return -1;
    }

    bool ok = false;
    const int explicitProgressValue = entryMap.value(QStringLiteral("progressValue")).toInt(&ok);
    if (ok && explicitProgressValue >= 0)
    {
        return explicitProgressValue;
    }

    const int itemId = entryMap.value(QStringLiteral("itemId")).toInt(&ok);
    if (ok && itemId >= 0)
    {
        return itemId;
    }

    const int rawValue = entryMap.value(QStringLiteral("value")).toInt(&ok);
    if (ok && rawValue >= 0)
    {
        return rawValue;
    }

    static const QRegularExpression progressKeyRegex(
        QStringLiteral(R"(^progress:(\d+)$)"),
        QRegularExpression::CaseInsensitiveOption);
    const QString key = entryMap.value(QStringLiteral("key")).toString().trimmed();
    const QRegularExpressionMatch progressKeyMatch = progressKeyRegex.match(key);
    if (progressKeyMatch.hasMatch())
    {
        const int keyValue = progressKeyMatch.captured(1).toInt(&ok);
        if (ok && keyValue >= 0)
        {
            return keyValue;
        }
    }

    const int fallbackLabelValue = fallbackProgressValueForLabel(
        entryMap.value(QStringLiteral("label")).toString());
    if (fallbackLabelValue >= 0)
    {
        return fallbackLabelValue;
    }

    return fallbackProgressValue;
}

QVariantList DetailNoteHeaderSelectionSourceController::buildHierarchyModelWithClearEntry(
    const QVariantList& optionsHierarchyModel) const
{
    QVariantList hierarchyModel;
    hierarchyModel.reserve(optionsHierarchyModel.size() + 1);

    const QVariantMap clearEntry = clearSelectionEntry(m_field);
    hierarchyModel.push_back(clearEntry);

    const QString clearKey = clearEntry.value(QStringLiteral("key")).toString();
    for (const QVariant& option : optionsHierarchyModel)
    {
        const QVariantMap optionMap = option.toMap();
        if (optionMap.value(QStringLiteral("key")).toString() == clearKey)
        {
            continue;
        }
        hierarchyModel.push_back(option);
    }

    return hierarchyModel;
}

int DetailNoteHeaderSelectionSourceController::resolveSelectedIndexForHeader(const QVariantList& hierarchyModel) const
{
    if (m_sessionStore == nullptr || m_noteId.isEmpty() || !m_sessionStore->hasEntry(m_noteId))
    {
        return -1;
    }

    const WhatSonNoteHeaderStore header = m_sessionStore->header(m_noteId);
    QString selectedLabel;
    switch (m_field)
    {
    case Field::Project:
        selectedLabel = header.project().trimmed();
        if (selectedLabel.isEmpty())
        {
            return hierarchyModel.isEmpty() ? -1 : 0;
        }
        break;
    case Field::Bookmark:
        if (!header.isBookmarked() || header.bookmarkColors().isEmpty())
        {
            return hierarchyModel.isEmpty() ? -1 : 0;
        }
        if (!header.bookmarkColors().isEmpty())
        {
            selectedLabel = WhatSon::Bookmarks::bookmarkDisplayNameForName(header.bookmarkColors().first());
        }
        break;
    case Field::Progress:
    {
        const int currentProgressValue = header.progress();
        if (currentProgressValue < 0)
        {
            return hierarchyModel.isEmpty() ? -1 : 0;
        }
        for (int index = 0; index < hierarchyModel.size(); ++index)
        {
            if (progressValueForHierarchyEntry(hierarchyModel.at(index)) == currentProgressValue)
            {
                return index;
            }
        }
        break;
    }
    }

    for (int index = 0; index < hierarchyModel.size(); ++index)
    {
        const QString label = hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
        if (label.compare(selectedLabel, Qt::CaseInsensitive) == 0)
        {
            return index;
        }
    }
    return -1;
}

bool DetailNoteHeaderSelectionSourceController::persistSelection(int index)
{
    if (m_sessionStore == nullptr || m_noteId.isEmpty() || !m_sessionStore->hasEntry(m_noteId))
    {
        return false;
    }
    if (index < 0 || index >= m_hierarchyModel.size())
    {
        return false;
    }

    const QVariantMap entry = m_hierarchyModel.at(index).toMap();
    const bool clearSelection = isClearSelectionEntry(entry);
    const QString label = entry.value(QStringLiteral("label")).toString().trimmed();
    switch (m_field)
    {
    case Field::Project:
        return m_sessionStore->updateProject(m_noteId, clearSelection ? QString() : label, nullptr);
    case Field::Bookmark:
    {
        if (clearSelection)
        {
            return m_sessionStore->updateBookmarked(m_noteId, false, {}, nullptr);
        }

        const QString colorName = label.trimmed().toLower();
        return m_sessionStore->updateBookmarked(
            m_noteId,
            !colorName.isEmpty(),
            colorName.isEmpty() ? QStringList{} : QStringList{colorName},
            nullptr);
    }
    case Field::Progress:
    {
        const int progress = progressValueForHierarchyEntry(m_hierarchyModel.at(index), index);
        return m_sessionStore->updateProgress(m_noteId, progress, nullptr);
    }
    }
    return false;
}

QString DetailNoteHeaderSelectionSourceController::entryLabelAt(int index) const
{
    if (index < 0 || index >= m_hierarchyModel.size())
    {
        return {};
    }
    return m_hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
}
