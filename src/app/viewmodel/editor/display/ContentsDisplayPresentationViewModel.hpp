#pragma once

#include "app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"

#include <QString>
#include <QVariant>

class ContentsDisplayPresentationViewModel : public ContentsDisplayControllerBridgeViewModel
{
    Q_OBJECT

public:
    using ContentsDisplayControllerBridgeViewModel::ContentsDisplayControllerBridgeViewModel;

public slots:
    void logEditorCreationState(const QString& reason);
    void commitDocumentPresentationRefresh();
    bool documentPresentationRenderDirty();
    void refreshInlineResourcePresentation();
    void requestViewHook(const QString& reason);
    void applyPresentationRefreshPlan(const QVariant& plan);
    void scheduleDeferredDocumentPresentationRefresh();
    void scheduleDocumentPresentationRefresh(bool forceImmediate);
};
