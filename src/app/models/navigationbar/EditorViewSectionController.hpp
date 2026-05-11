#pragma once

#include "app/models/navigationbar/EditorViewState.hpp"

#include <QObject>
#include <QString>

class EditorViewSectionController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int editorViewValue READ editorViewValue CONSTANT)
    Q_PROPERTY(QString editorViewName READ editorViewName CONSTANT)

public:
    using EditorView = WhatSon::NavigationBar::EditorView;

    explicit EditorViewSectionController(
        EditorView editorView,
        QObject* parent = nullptr);

    bool active() const noexcept;
    int editorViewValue() const noexcept;
    QString editorViewName() const;

    Q_INVOKABLE void setActive(bool active);

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

    signals  :



    void activeChanged();
    void controllerHookRequested();

private:
    EditorView m_editorView = EditorView::Plain;
    bool m_active = false;
};
