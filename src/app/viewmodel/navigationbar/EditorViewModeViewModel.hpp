#pragma once

#include "EditorViewSectionViewModel.hpp"
#include "EditorViewState.hpp"

#include <QObject>
#include <QString>

class EditorViewModeViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeViewMode READ activeViewMode WRITE setActiveViewMode NOTIFY activeViewModeChanged)
    Q_PROPERTY(QString activeViewModeName READ activeViewModeName NOTIFY activeViewModeChanged)
    Q_PROPERTY(QObject* activeViewModeViewModel READ activeViewModeViewModel NOTIFY activeViewModeChanged)
    Q_PROPERTY(QObject* plainViewModeViewModel READ plainViewModeViewModel CONSTANT)
    Q_PROPERTY(QObject* pageViewModeViewModel READ pageViewModeViewModel CONSTANT)
    Q_PROPERTY(QObject* printViewModeViewModel READ printViewModeViewModel CONSTANT)
    Q_PROPERTY(QObject* webViewModeViewModel READ webViewModeViewModel CONSTANT)
    Q_PROPERTY(QObject* presentationViewModeViewModel READ presentationViewModeViewModel CONSTANT)

public:
    using EditorView = WhatSon::NavigationBar::EditorView;

    explicit EditorViewModeViewModel(QObject* parent = nullptr);
    ~EditorViewModeViewModel() override;

    int activeViewMode() const noexcept;
    QString activeViewModeName() const;
    QObject* activeViewModeViewModel() const noexcept;
    QObject* plainViewModeViewModel() const noexcept;
    QObject* pageViewModeViewModel() const noexcept;
    QObject* printViewModeViewModel() const noexcept;
    QObject* webViewModeViewModel() const noexcept;
    QObject* presentationViewModeViewModel() const noexcept;

    Q_INVOKABLE QObject* viewModeViewModelForState(int viewModeValue) const noexcept;
    Q_INVOKABLE void setActiveViewMode(int viewModeValue);
    Q_INVOKABLE void requestViewModeChange(int viewModeValue);
    Q_INVOKABLE void requestNextViewMode();

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void activeViewModeChanged();
    void viewModelHookRequested();

private:
    void applyActiveViewModeViewModel(EditorView activeViewMode);

    EditorView m_activeViewMode = EditorView::Plain;
    EditorViewSectionViewModel m_plainViewModeViewModel;
    EditorViewSectionViewModel m_pageViewModeViewModel;
    EditorViewSectionViewModel m_printViewModeViewModel;
    EditorViewSectionViewModel m_webViewModeViewModel;
    EditorViewSectionViewModel m_presentationViewModeViewModel;
    QObject* m_activeViewModeViewModel = nullptr;
};
