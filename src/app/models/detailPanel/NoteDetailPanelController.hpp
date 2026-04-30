#pragma once

#include "app/models/detailPanel/DetailPanelController.hpp"

class NoteDetailPanelController final : public DetailPanelController
{
    Q_OBJECT

public:
    explicit NoteDetailPanelController(QObject* parent = nullptr)
        : DetailPanelController(parent)
    {
    }

    ~NoteDetailPanelController() override = default;
};
