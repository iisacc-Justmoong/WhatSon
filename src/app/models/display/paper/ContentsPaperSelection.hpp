#pragma once

#include <QObject>
#include <QString>

class ContentsPaperSelection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PaperKind selectedPaperKind READ selectedPaperKind WRITE setSelectedPaperKind NOTIFY selectedPaperKindChanged)
    Q_PROPERTY(QString selectedPaperStandard READ selectedPaperStandard NOTIFY selectedPaperKindChanged)

public:
    enum PaperKind
    {
        Unknown = 0,
        A4,
        Letter,
        Legal,
        A5,
        B5,
    };
    Q_ENUM(PaperKind)

    explicit ContentsPaperSelection(QObject* parent = nullptr);
    ~ContentsPaperSelection() override;

    static QString paperStandardForKind(PaperKind paperKind);
    static PaperKind normalizePaperKind(int value) noexcept;

    PaperKind selectedPaperKind() const noexcept;
    void setSelectedPaperKind(PaperKind selectedPaperKind);
    QString selectedPaperStandard() const;

    Q_INVOKABLE QString paperStandardForValue(int value) const;

public slots:
    void setSelectedPaperKindByValue(int value);
    void requestRefresh();

signals:
    void selectedPaperKindChanged();

private:
    PaperKind m_selectedPaperKind = A4;
};
