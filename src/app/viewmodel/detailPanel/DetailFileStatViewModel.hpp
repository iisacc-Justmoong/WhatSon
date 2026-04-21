#pragma once

#include "DetailContentSectionViewModel.hpp"
#include "models/file/note/WhatSonNoteHeaderStore.hpp"

#include <QStringList>
#include <QVariantList>

class DetailFileStatViewModel final : public DetailContentSectionViewModel
{
    Q_OBJECT

    Q_PROPERTY(int totalFolders READ totalFolders NOTIFY statsChanged)
    Q_PROPERTY(int totalTags READ totalTags NOTIFY statsChanged)
    Q_PROPERTY(int letterCount READ letterCount NOTIFY statsChanged)
    Q_PROPERTY(int wordCount READ wordCount NOTIFY statsChanged)
    Q_PROPERTY(int sentenceCount READ sentenceCount NOTIFY statsChanged)
    Q_PROPERTY(int paragraphCount READ paragraphCount NOTIFY statsChanged)
    Q_PROPERTY(int spaceCount READ spaceCount NOTIFY statsChanged)
    Q_PROPERTY(int indentCount READ indentCount NOTIFY statsChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY statsChanged)
    Q_PROPERTY(int openCount READ openCount NOTIFY statsChanged)
    Q_PROPERTY(int modifiedCount READ modifiedCount NOTIFY statsChanged)
    Q_PROPERTY(int backlinkToCount READ backlinkToCount NOTIFY statsChanged)
    Q_PROPERTY(int backlinkByCount READ backlinkByCount NOTIFY statsChanged)
    Q_PROPERTY(int includedResourceCount READ includedResourceCount NOTIFY statsChanged)
    Q_PROPERTY(QStringList summaryLines READ summaryLines NOTIFY statsChanged)
    Q_PROPERTY(QStringList textMetricLines READ textMetricLines NOTIFY statsChanged)
    Q_PROPERTY(QStringList activityLines READ activityLines NOTIFY statsChanged)
    Q_PROPERTY(QVariantList overviewItems READ overviewItems NOTIFY statsChanged)
    Q_PROPERTY(QVariantList textItems READ textItems NOTIFY statsChanged)
    Q_PROPERTY(QVariantList relationItems READ relationItems NOTIFY statsChanged)

public:
    explicit DetailFileStatViewModel(QObject* parent = nullptr);

    int totalFolders() const noexcept;
    int totalTags() const noexcept;
    int letterCount() const noexcept;
    int wordCount() const noexcept;
    int sentenceCount() const noexcept;
    int paragraphCount() const noexcept;
    int spaceCount() const noexcept;
    int indentCount() const noexcept;
    int lineCount() const noexcept;
    int openCount() const noexcept;
    int modifiedCount() const noexcept;
    int backlinkToCount() const noexcept;
    int backlinkByCount() const noexcept;
    int includedResourceCount() const noexcept;

    QStringList summaryLines() const;
    QStringList textMetricLines() const;
    QStringList activityLines() const;

    QVariantList overviewItems() const;
    QVariantList textItems() const;
    QVariantList relationItems() const;

    void applyHeader(const WhatSonNoteHeaderStore& header);
    void clearHeader();

signals:
    void statsChanged();

private:
    WhatSonNoteHeaderStore m_header;
};
