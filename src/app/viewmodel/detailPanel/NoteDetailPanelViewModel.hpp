#pragma once

#include "DetailPanelViewModel.hpp"

class NoteDetailPanelViewModel final : public DetailPanelViewModel
{
    Q_OBJECT

public:
    explicit NoteDetailPanelViewModel(QObject* parent = nullptr)
        : DetailPanelViewModel(parent)
    {
    }

    ~NoteDetailPanelViewModel() override = default;
};
