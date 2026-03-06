#pragma once

#include <QString>

class WhatSonTxtFileStore
{
public:
    WhatSonTxtFileStore();
    ~WhatSonTxtFileStore();

    void clear();

    QString filePath() const;
    void setFilePath(QString value);

    QString bodyPlainText() const;
    void setBodyPlainText(QString value);

private:
    QString m_filePath;
    QString m_bodyPlainText;
};
