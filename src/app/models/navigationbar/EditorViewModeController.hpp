#pragma once

#include "app/models/navigationbar/EditorViewSectionController.hpp"
#include "app/models/navigationbar/EditorViewState.hpp"

#include <QObject>
#include <QString>

class EditorViewModeController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeViewMode READ activeViewMode WRITE setActiveViewMode NOTIFY activeViewModeChanged)
    Q_PROPERTY(QString activeViewModeName READ activeViewModeName NOTIFY activeViewModeChanged)
    Q_PROPERTY(QObject* activeViewModeController READ activeViewModeController NOTIFY activeViewModeChanged)
    Q_PROPERTY(QObject* plainViewModeController READ plainViewModeController CONSTANT)
    Q_PROPERTY(QObject* pageViewModeController READ pageViewModeController CONSTANT)
    Q_PROPERTY(QObject* printViewModeController READ printViewModeController CONSTANT)
    Q_PROPERTY(QObject* webViewModeController READ webViewModeController CONSTANT)
    Q_PROPERTY(QObject* presentationViewModeController READ presentationViewModeController CONSTANT)

public:
    using EditorView = WhatSon::NavigationBar::EditorView;

    explicit EditorViewModeController(QObject* parent = nullptr);
    ~EditorViewModeController() override;

    int activeViewMode() const noexcept;
    QString activeViewModeName() const;
    QObject* activeViewModeController() const noexcept;
    QObject* plainViewModeController() const noexcept;
    QObject* pageViewModeController() const noexcept;
    QObject* printViewModeController() const noexcept;
    QObject* webViewModeController() const noexcept;
    QObject* presentationViewModeController() const noexcept;

    Q_INVOKABLE QObject* viewModeControllerForState(int viewModeValue) const noexcept;
    Q_INVOKABLE void setActiveViewMode(int viewModeValue);
    Q_INVOKABLE void requestViewModeChange(int viewModeValue);
    Q_INVOKABLE void requestNextViewMode();

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

    signals  :



    void activeViewModeChanged();
    void controllerHookRequested();

private:
    void applyActiveViewModeController(EditorView activeViewMode);

    EditorView m_activeViewMode = EditorView::Plain;
    EditorViewSectionController m_plainViewModeController;
    EditorViewSectionController m_pageViewModeController;
    EditorViewSectionController m_printViewModeController;
    EditorViewSectionController m_webViewModeController;
    EditorViewSectionController m_presentationViewModeController;
    QObject* m_activeViewModeController = nullptr;
};
