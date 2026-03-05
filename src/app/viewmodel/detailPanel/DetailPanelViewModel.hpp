#pragma once

#include "DetailContentSectionViewModel.hpp"
#include "DetailPanelState.hpp"

#include <QObject>
#include <QVariantList>

class DetailPanelViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeState READ activeState WRITE setActiveState NOTIFY activeStateChanged)
    Q_PROPERTY(QObject* activeContentViewModel READ activeContentViewModel NOTIFY activeStateChanged)
    Q_PROPERTY(QString activeStateName READ activeStateName NOTIFY activeStateChanged)
    Q_PROPERTY(QObject* appearanceViewModel READ appearanceViewModel CONSTANT)
    Q_PROPERTY(QObject* fileFormatViewModel READ fileFormatViewModel CONSTANT)
    Q_PROPERTY(QObject* fileHistoryViewModel READ fileHistoryViewModel CONSTANT)
    Q_PROPERTY(QObject* fileInfoViewModel READ fileInfoViewModel CONSTANT)
    Q_PROPERTY(QObject* fileStatViewModel READ fileStatViewModel CONSTANT)
    Q_PROPERTY(QObject* helpViewModel READ helpViewModel CONSTANT)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarItemsChanged)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailPanelViewModel(QObject* parent = nullptr);
    ~DetailPanelViewModel() override;

    int activeState() const noexcept;
    QObject* activeContentViewModel() const noexcept;
    QString activeStateName() const;
    QObject* appearanceViewModel() const noexcept;
    Q_INVOKABLE QObject* contentViewModelForState(int stateValue) const noexcept;
    QObject* fileFormatViewModel() const noexcept;
    QObject* fileHistoryViewModel() const noexcept;
    QObject* fileInfoViewModel() const noexcept;
    QObject* fileStatViewModel() const noexcept;
    QObject* helpViewModel() const noexcept;
    QVariantList toolbarItems() const;

    Q_INVOKABLE void setActiveState(int stateValue);
    Q_INVOKABLE void requestStateChange(int stateValue);

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :


    void activeStateChanged();
    void toolbarItemsChanged();
    void viewModelHookRequested();

private:
    void applyActiveContentViewModel(DetailContentState activeState);

    WhatSon::DetailPanel::ContentState m_activeState = WhatSon::DetailPanel::ContentState::FileInfo;
    DetailContentSectionViewModel m_fileInfoViewModel;
    DetailContentSectionViewModel m_fileStatViewModel;
    DetailContentSectionViewModel m_fileFormatViewModel;
    DetailContentSectionViewModel m_fileHistoryViewModel;
    DetailContentSectionViewModel m_appearanceViewModel;
    DetailContentSectionViewModel m_helpViewModel;
    QObject* m_activeContentViewModel = nullptr;
    QVariantList m_toolbarItems;
};
