#pragma once

#include <QImage>

class QClipboard;
class QMimeData;

namespace WhatSon::Resources::ClipboardImportSupport
{
    bool extractClipboardImage(const QMimeData* mimeData, QImage* outImage);
    bool extractClipboardImage(const QClipboard* clipboard, QImage* outImage);
    bool clipboardContainsImportableImage();
} // namespace WhatSon::Resources::ClipboardImportSupport
