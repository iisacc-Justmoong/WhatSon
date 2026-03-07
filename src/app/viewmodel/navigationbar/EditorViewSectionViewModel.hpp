#pragma once

#include "EditorViewState.hpp"

#include <QObject>
#include <QString>

class EditorViewSectionViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int editorViewValue READ editorViewValue CONSTANT)
    Q_PROPERTY(QString editorViewName READ editorViewName CONSTANT)

public:
    using EditorView = WhatSon::NavigationBar::EditorView;

    explicit EditorViewSectionViewModel(
        EditorView editorView,
        QObject* parent = nullptr);

    bool active() const noexcept;
    int editorViewValue() const noexcept;
    QString editorViewName() const;

    Q_INVOKABLE void setActive(bool active);

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void activeChanged();
    void viewModelHookRequested();

private:
    EditorView m_editorView = EditorView::Plain;
    bool m_active = false;
};
