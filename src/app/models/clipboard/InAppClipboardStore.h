#pragma once

#include "app/models/clipboard/ClipboardResourceImport.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

class InAppClipboardStore final : public QObject
{
    Q_OBJECT

public:
    explicit InAppClipboardStore(QObject* parent = nullptr);
    ~InAppClipboardStore() override;

    bool hasResource() const noexcept;
    QString resourceFileName() const;
    QString resourceFormat() const;
    QString resourceType() const;
    QString resourceBucket() const;
    QString resourceMimeType() const;
    QVariantMap resourceEntry() const;

    const WhatSon::Clipboard::ClipboardResourceImport& resourceImport() const noexcept;
    WhatSon::Clipboard::ClipboardResourceImport takeResourceImport();

    bool setResourceImport(WhatSon::Clipboard::ClipboardResourceImport resourceImport);
    void clear();

signals:
    void resourceChanged();

private:
    WhatSon::Clipboard::ClipboardResourceImport m_resourceImport;
};
