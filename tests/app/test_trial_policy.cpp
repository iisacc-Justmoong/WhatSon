#include "WhatSonTrialActivationPolicy.hpp"
#include "WhatSonTrialClockStore.hpp"
#include "WhatSonTrialInstallStore.hpp"
#include "WhatSonTrialWshubAccessBackend.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QtTest/QtTest>

namespace
{
    void prepareIsolatedSettings(const QTemporaryDir& tempDir)
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());

        QSettings settings;
        settings.clear();
        settings.sync();
    }
}

class WhatSonTrialPolicyTest final : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void refreshForDate_persistsInitialInstallDate();
    void refreshForDate_reusesPersistedInstallDate();
    void refreshForDate_keepsTrialActiveThroughLastActiveDay();
    void refreshForDate_expiresTrialAfterNinetyDays();
    void refreshForDate_recoversFromInvalidStoredInstallDate();
    void refreshForDate_emitsStateChangedOnlyOnStateMutation();
    void stampExitTimestamp_persistsUtcExitTimestamp();
    void inspect_detectsClockRollbackAgainstLastSeenTimestamp();
    void stampExitTimestamp_preservesMonotonicLastSeenTimestamp();
    void loadLastSeenTimestampUtc_clearsInvalidStoredTimestamp();
    void evaluateAccess_allowsWshubWithinTrialWindow();
    void evaluateAccess_blocksWshubAfterTrialExpiry();
    void evaluateAccess_blocksWshubContentUriAfterTrialExpiry();
    void evaluateAccess_ignoresNonWshubTargetsAfterTrialExpiry();
};

void WhatSonTrialPolicyTest::init()
{
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSonTest"));
    QCoreApplication::setApplicationName(QStringLiteral("WhatSonTrialPolicyTest"));
}

void WhatSonTrialPolicyTest::refreshForDate_persistsInitialInstallDate()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDate installDate(2026, 1, 10);

    WhatSonTrialActivationPolicy policy;
    const WhatSonTrialActivationState state = policy.refreshForDate(installDate);

    QCOMPARE(state.installDate, installDate);
    QCOMPARE(state.lastActiveDate, installDate.addDays(WhatSonTrialActivationPolicy::kDefaultTrialLengthDays - 1));
    QCOMPARE(state.trialLengthDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QCOMPARE(state.elapsedDays, 0);
    QCOMPARE(state.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QVERIFY(state.active);

    WhatSonTrialInstallStore store;
    QCOMPARE(store.loadInstallDate(), installDate);
}

void WhatSonTrialPolicyTest::refreshForDate_reusesPersistedInstallDate()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDate installDate(2026, 1, 10);

    WhatSonTrialActivationPolicy policy;
    QCOMPARE(policy.refreshForDate(installDate).installDate, installDate);

    const WhatSonTrialActivationState nextState = policy.refreshForDate(installDate.addDays(1));
    QCOMPARE(nextState.installDate, installDate);
    QCOMPARE(nextState.elapsedDays, 1);
    QCOMPARE(nextState.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays - 1);
    QVERIFY(nextState.active);
}

void WhatSonTrialPolicyTest::refreshForDate_keepsTrialActiveThroughLastActiveDay()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDate installDate(2026, 1, 10);
    WhatSonTrialInstallStore store;
    store.storeInstallDate(installDate);

    WhatSonTrialActivationPolicy policy(store);
    const WhatSonTrialActivationState state = policy.refreshForDate(installDate.addDays(89));

    QCOMPARE(state.installDate, installDate);
    QCOMPARE(state.lastActiveDate, installDate.addDays(89));
    QCOMPARE(state.elapsedDays, 89);
    QCOMPARE(state.remainingDays, 1);
    QVERIFY(state.active);
}

void WhatSonTrialPolicyTest::refreshForDate_expiresTrialAfterNinetyDays()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDate installDate(2026, 1, 10);
    WhatSonTrialInstallStore store;
    store.storeInstallDate(installDate);

    WhatSonTrialActivationPolicy policy(store);
    const WhatSonTrialActivationState state = policy.refreshForDate(installDate.addDays(90));

    QCOMPARE(state.installDate, installDate);
    QCOMPARE(state.lastActiveDate, installDate.addDays(89));
    QCOMPARE(state.elapsedDays, 90);
    QCOMPARE(state.remainingDays, 0);
    QVERIFY(!state.active);
}

void WhatSonTrialPolicyTest::refreshForDate_recoversFromInvalidStoredInstallDate()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    QSettings settings;
    settings.setValue(WhatSonTrialInstallStore::defaultInstallDateSettingsKey(), QStringLiteral("not-a-date"));
    settings.sync();

    const QDate fallbackDate(2026, 2, 1);
    WhatSonTrialActivationPolicy policy;
    const WhatSonTrialActivationState state = policy.refreshForDate(fallbackDate);

    settings.sync();
    QCOMPARE(state.installDate, fallbackDate);
    QCOMPARE(settings.value(WhatSonTrialInstallStore::defaultInstallDateSettingsKey()).toString(),
             fallbackDate.toString(Qt::ISODate));
}

void WhatSonTrialPolicyTest::refreshForDate_emitsStateChangedOnlyOnStateMutation()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDate installDate(2026, 1, 10);

    WhatSonTrialActivationPolicy policy;
    QSignalSpy stateChangedSpy(&policy, &WhatSonTrialActivationPolicy::stateChanged);

    policy.refreshForDate(installDate);
    QCOMPARE(stateChangedSpy.count(), 1);

    policy.refreshForDate(installDate);
    QCOMPARE(stateChangedSpy.count(), 1);

    policy.refreshForDate(installDate.addDays(1));
    QCOMPARE(stateChangedSpy.count(), 2);
}

void WhatSonTrialPolicyTest::stampExitTimestamp_persistsUtcExitTimestamp()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDateTime exitTimestampUtc(QDate(2026, 1, 10), QTime(12, 30, 15, 120), QTimeZone::UTC);

    WhatSonTrialClockStore store;
    const WhatSonTrialClockCheck check = store.stampExitTimestamp(exitTimestampUtc);

    QCOMPARE(check.lastExitTimestampUtc, exitTimestampUtc);
    QCOMPARE(check.lastSeenTimestampUtc, exitTimestampUtc);
    QCOMPARE(check.rollbackSeconds, 0);
    QVERIFY(!check.clockRollbackDetected);
    QCOMPARE(store.loadLastExitTimestampUtc(), exitTimestampUtc);
    QCOMPARE(store.loadLastSeenTimestampUtc(), exitTimestampUtc);
}

void WhatSonTrialPolicyTest::inspect_detectsClockRollbackAgainstLastSeenTimestamp()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDateTime exitTimestampUtc(QDate(2026, 1, 10), QTime(12, 30, 0), QTimeZone::UTC);
    const QDateTime rollbackTimestampUtc = exitTimestampUtc.addSecs(-90);

    WhatSonTrialClockStore store;
    store.stampExitTimestamp(exitTimestampUtc);

    const WhatSonTrialClockCheck check = store.inspect(rollbackTimestampUtc);
    QCOMPARE(check.lastExitTimestampUtc, exitTimestampUtc);
    QCOMPARE(check.lastSeenTimestampUtc, exitTimestampUtc);
    QCOMPARE(check.rollbackSeconds, 90);
    QVERIFY(check.clockRollbackDetected);
}

void WhatSonTrialPolicyTest::stampExitTimestamp_preservesMonotonicLastSeenTimestamp()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QDateTime laterTimestampUtc(QDate(2026, 1, 10), QTime(14, 0, 0), QTimeZone::UTC);
    const QDateTime earlierTimestampUtc = laterTimestampUtc.addSecs(-3600);

    WhatSonTrialClockStore store;
    store.stampExitTimestamp(laterTimestampUtc);
    const WhatSonTrialClockCheck check = store.stampExitTimestamp(earlierTimestampUtc);

    QCOMPARE(check.lastExitTimestampUtc, earlierTimestampUtc);
    QCOMPARE(check.lastSeenTimestampUtc, laterTimestampUtc);
    QCOMPARE(check.rollbackSeconds, 3600);
    QVERIFY(check.clockRollbackDetected);
    QCOMPARE(store.loadLastExitTimestampUtc(), earlierTimestampUtc);
    QCOMPARE(store.loadLastSeenTimestampUtc(), laterTimestampUtc);
}

void WhatSonTrialPolicyTest::loadLastSeenTimestampUtc_clearsInvalidStoredTimestamp()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    QSettings settings;
    settings.setValue(WhatSonTrialClockStore::defaultLastSeenTimestampSettingsKey(), QStringLiteral("not-a-timestamp"));
    settings.sync();

    WhatSonTrialClockStore store;
    QCOMPARE(store.loadLastSeenTimestampUtc(), QDateTime());

    settings.sync();
    QVERIFY(!settings.contains(WhatSonTrialClockStore::defaultLastSeenTimestampSettingsKey()));
}

void WhatSonTrialPolicyTest::evaluateAccess_allowsWshubWithinTrialWindow()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    WhatSonTrialWshubAccessBackend backend;
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 10));

    QVERIFY(decision.wshubTarget);
    QVERIFY(decision.allowed);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QCOMPARE(decision.normalizedTargetPath, QDir::cleanPath(hubPath));
    QCOMPARE(decision.trialState.installDate, QDate(2026, 1, 10));
    QCOMPARE(decision.trialState.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QCOMPARE(decision.denialReason, QString());
}

void WhatSonTrialPolicyTest::evaluateAccess_blocksWshubAfterTrialExpiry()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Expired.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    WhatSonTrialInstallStore installStore;
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonTrialWshubAccessBackend backend(installStore);
    QString denialReason;
    const bool allowed = backend.canAccess(hubPath, &denialReason, QDate(2026, 4, 10));
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 4, 10));

    QVERIFY(!allowed);
    QVERIFY(decision.wshubTarget);
    QVERIFY(!decision.allowed);
    QVERIFY(decision.restrictedByExpiredTrial);
    QCOMPARE(decision.trialState.installDate, QDate(2026, 1, 10));
    QCOMPARE(decision.trialState.lastActiveDate, QDate(2026, 4, 9));
    QCOMPARE(decision.trialState.remainingDays, 0);
    QVERIFY(!decision.denialReason.isEmpty());
    QCOMPARE(denialReason, decision.denialReason);
}

void WhatSonTrialPolicyTest::evaluateAccess_blocksWshubContentUriAfterTrialExpiry()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    WhatSonTrialInstallStore installStore;
    installStore.storeInstallDate(QDate(2026, 1, 10));

    const QString hubUri =
        QStringLiteral("content://whatson.provider/document/Expired.wshub");
    WhatSonTrialWshubAccessBackend backend(installStore);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubUri, QDate(2026, 4, 10));

    QVERIFY(decision.wshubTarget);
    QVERIFY(!decision.allowed);
    QVERIFY(decision.restrictedByExpiredTrial);
    QCOMPARE(decision.normalizedTargetPath, hubUri);
}

void WhatSonTrialPolicyTest::evaluateAccess_ignoresNonWshubTargetsAfterTrialExpiry()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    const QString nonHubPath = QDir(tempDir.path()).filePath(QStringLiteral("notes.txt"));
    QFile file(nonHubPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("sample");
    file.close();

    WhatSonTrialInstallStore installStore;
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonTrialWshubAccessBackend backend(installStore);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(nonHubPath, QDate(2026, 4, 10));

    QVERIFY(!decision.wshubTarget);
    QVERIFY(decision.allowed);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QCOMPARE(decision.denialReason, QString());
}

QTEST_APPLESS_MAIN(WhatSonTrialPolicyTest)

#include "test_trial_policy.moc"
