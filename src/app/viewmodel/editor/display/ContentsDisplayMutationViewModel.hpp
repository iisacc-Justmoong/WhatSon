#pragma once

#include "app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"

#include <QString>
#include <QVariant>

class ContentsDisplayMutationViewModel : public ContentsDisplayControllerBridgeViewModel
{
    Q_OBJECT

public:
    using ContentsDisplayControllerBridgeViewModel::ContentsDisplayControllerBridgeViewModel;

public slots:
    bool queueStructuredInlineFormatWrap(const QString& tagName);
    bool queueInlineFormatWrap(const QString& tagName);
    bool queueAgendaShortcutInsertion();
    bool queueCalloutShortcutInsertion();
    bool queueBreakShortcutInsertion();
    void focusStructuredBlockSourceOffset(int sourceOffset);
    bool applyDocumentSourceMutation(const QString& nextSourceText, const QVariant& focusRequest);
    bool setAgendaTaskDone(int taskOpenTagStart, int taskOpenTagEnd, bool checked);
    bool persistEditorTextImmediately(const QString& nextText);
    QVariant selectedEditorRange();
    bool wrapSelectedEditorTextWithTag(const QString& tagName, const QVariant& explicitSelectionRange);
};
