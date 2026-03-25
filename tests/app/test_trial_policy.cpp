#include "WhatSonTrialActivationPolicy.hpp"
#include "WhatSonRegisterManager.hpp"
#include "WhatSonTrialClientIdentityStore.hpp"
#include "WhatSonTrialClockStore.hpp"
#include "WhatSonTrialInstallStore.hpp"
#include "WhatSonTrialRegisterXml.hpp"
#include "WhatSonTrialSecureStore.hpp"
#include "WhatSonTrialWshubAccessBackend.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QtTest/QtTest>

#include <memory>

namespace
{
    class MemoryTrialSecureStoreBackend final : public WhatSonTrialSecureStoreBackend
    {
    public:
        WhatSonTrialSecureStoreReadResult readSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            const QString compositeKey = serviceName + QLatin1Char('\n') + entryKey;
            if (!m_values.contains(compositeKey))
            {
                return {
                    {},
                    {},
                    WhatSonTrialSecureStoreStatus::NotFound
                };
            }

            return {
                m_values.value(compositeKey),
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult writeSecret(
            const QString& serviceName,
            const QString& entryKey,
            const QString& value) const override
        {
            m_values.insert(serviceName + QLatin1Char('\n') + entryKey, value);
            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult removeSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            m_values.remove(serviceName + QLatin1Char('\n') + entryKey);
            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

    private:
        mutable QHash<QString, QString> m_values;
    };

    void prepareIsolatedSettings(const QTemporaryDir& tempDir)
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());

        QSettings settings;
        settings.clear();
        settings.sync();
    }

    QString createHubPath(const QTemporaryDir& tempDir, const QString& hubName)
    {
        const QString hubPath = QDir(tempDir.path()).filePath(hubName);
        QDir().mkpath(QDir(hubPath).filePath(QStringLiteral(".whatson")));
        return hubPath;
    }

    WhatSonTrialSecureStore createSecureStore(const std::shared_ptr<MemoryTrialSecureStoreBackend>& backend)
    {
        return WhatSonTrialSecureStore(QStringLiteral("whatson-trial-policy-tests"), backend);
    }

    WhatSonTrialInstallStore createInstallStore(const std::shared_ptr<MemoryTrialSecureStoreBackend>& backend)
    {
        return WhatSonTrialInstallStore(
            WhatSonTrialInstallStore::defaultInstallDateSettingsKey(),
            createSecureStore(backend));
    }

    WhatSonTrialClientIdentityStore createIdentityStore(const std::shared_ptr<MemoryTrialSecureStoreBackend>& backend)
    {
        return WhatSonTrialClientIdentityStore(
            WhatSonTrialClientIdentityStore::defaultDeviceUuidSettingsKey(),
            WhatSonTrialClientIdentityStore::defaultClientKeySettingsKey(),
            WhatSonTrialClientIdentityStore::defaultRegisterIntegritySecretSettingsKey(),
            createSecureStore(backend));
    }

    WhatSonTrialRegisterXml createRegisterXml(const std::shared_ptr<MemoryTrialSecureStoreBackend>& backend)
    {
        return WhatSonTrialRegisterXml(createIdentityStore(backend));
    }
}

class WhatSonTrialPolicyTest final : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void refreshForDate_persistsInitialInstallDate();
    void refreshForDate_reusesPersistedInstallDate();
    void loadInstallDate_restoresSecureStoreBackedInstallDateAfterSettingsReset();
    void refreshForDate_keepsTrialActiveThroughLastActiveDay();
    void refreshForDate_expiresTrialAfterNinetyDays();
    void refreshForDate_recoversFromInvalidStoredInstallDate();
    void refreshForDate_emitsStateChangedOnlyOnStateMutation();
    void stampExitTimestamp_persistsUtcExitTimestamp();
    void inspect_detectsClockRollbackAgainstLastSeenTimestamp();
    void stampExitTimestamp_preservesMonotonicLastSeenTimestamp();
    void loadLastSeenTimestampUtc_clearsInvalidStoredTimestamp();
    void registerManager_persistsAuthenticatedFlag();
    void ensureIdentity_persistsStableClientIdentity();
    void ensureIdentity_restoresSecureStoreBackedIdentityAfterSettingsReset();
    void writeRegister_writesTrialRegisterXmlForCurrentClient();
    void refreshForDate_bypassesTrialWhenRegisterAuthenticated();
    void evaluateAccess_allowsWshubWithinTrialWindow();
    void evaluateAccess_blocksWshubAfterTrialExpiry();
    void evaluateAccess_blocksWshubContentUriAfterTrialExpiry();
    void evaluateAccess_blocksLocalWshubWithoutTrialRegister();
    void evaluateAccess_bypassesAllTrialGuardsWhenRegisterAuthenticated();
    void evaluateAccess_allowsWshubWhenRegisterKeyMatches();
    void evaluateAccess_ignoresDeviceUuidMismatchWhenKeyMatches();
    void evaluateAccess_blocksWshubWhenRegisterKeyDiffersFromClientKey();
    void evaluateAccess_blocksInvalidTrialRegisterFile();
    void loadRegister_rejectsTamperedRegisterSignature();
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);

    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
    WhatSonTrialActivationPolicy policy(store);
    const WhatSonTrialActivationState state = policy.refreshForDate(installDate);

    QCOMPARE(state.installDate, installDate);
    QCOMPARE(state.lastActiveDate, installDate.addDays(WhatSonTrialActivationPolicy::kDefaultTrialLengthDays - 1));
    QCOMPARE(state.trialLengthDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QCOMPARE(state.elapsedDays, 0);
    QCOMPARE(state.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QVERIFY(state.active);

    QCOMPARE(store.loadInstallDate(), installDate);
}

void WhatSonTrialPolicyTest::refreshForDate_reusesPersistedInstallDate()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);

    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
    WhatSonTrialActivationPolicy policy(store);
    QCOMPARE(policy.refreshForDate(installDate).installDate, installDate);

    const WhatSonTrialActivationState nextState = policy.refreshForDate(installDate.addDays(1));
    QCOMPARE(nextState.installDate, installDate);
    QCOMPARE(nextState.elapsedDays, 1);
    QCOMPARE(nextState.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays - 1);
    QVERIFY(nextState.active);
}

void WhatSonTrialPolicyTest::loadInstallDate_restoresSecureStoreBackedInstallDateAfterSettingsReset()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);
    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
    store.storeInstallDate(installDate);

    QSettings settings;
    settings.remove(WhatSonTrialInstallStore::defaultInstallDateSettingsKey());
    settings.sync();

    WhatSonTrialInstallStore reloadedStore = createInstallStore(secureBackend);
    QCOMPARE(reloadedStore.loadInstallDate(), installDate);
    QCOMPARE(settings.value(WhatSonTrialInstallStore::defaultInstallDateSettingsKey()).toString(),
             installDate.toString(Qt::ISODate));
}

void WhatSonTrialPolicyTest::refreshForDate_keepsTrialActiveThroughLastActiveDay()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);
    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);
    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    QSettings settings;
    settings.setValue(WhatSonTrialInstallStore::defaultInstallDateSettingsKey(), QStringLiteral("not-a-date"));
    settings.sync();

    const QDate fallbackDate(2026, 2, 1);
    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
    WhatSonTrialActivationPolicy policy(store);
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QDate installDate(2026, 1, 10);

    WhatSonTrialInstallStore store = createInstallStore(secureBackend);
    WhatSonTrialActivationPolicy policy(store);
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

void WhatSonTrialPolicyTest::registerManager_persistsAuthenticatedFlag()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);

    WhatSonRegisterManager manager;
    QVERIFY(!manager.authenticated());

    QSignalSpy authenticatedChangedSpy(&manager, &WhatSonRegisterManager::authenticatedChanged);
    manager.setAuthenticated(true);

    QVERIFY(manager.authenticated());
    QCOMPARE(authenticatedChangedSpy.count(), 1);

    WhatSonRegisterManager reloadedManager;
    QVERIFY(reloadedManager.authenticated());

    reloadedManager.clearAuthentication();
    QVERIFY(!reloadedManager.authenticated());

    WhatSonRegisterManager clearedManager;
    QVERIFY(!clearedManager.authenticated());
}

void WhatSonTrialPolicyTest::ensureIdentity_persistsStableClientIdentity()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    WhatSonTrialClientIdentityStore store = createIdentityStore(secureBackend);
    const WhatSonTrialClientIdentity identity = store.ensureIdentity();
    const WhatSonTrialClientIdentity reloadedIdentity = store.ensureIdentity();

    QVERIFY(!identity.deviceUuid.isEmpty());
    QVERIFY(!identity.key.isEmpty());
    QCOMPARE(identity, reloadedIdentity);
    QCOMPARE(identity.deviceUuid, WhatSonTrialClientIdentityStore::normalizeDeviceUuid(identity.deviceUuid));
    QCOMPARE(identity.key.size(), 32);
    QVERIFY(!store.loadRegisterIntegritySecret().isEmpty());
}

void WhatSonTrialPolicyTest::ensureIdentity_restoresSecureStoreBackedIdentityAfterSettingsReset()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    WhatSonTrialClientIdentityStore store = createIdentityStore(secureBackend);
    const WhatSonTrialClientIdentity identity = store.ensureIdentity();
    const QString registerIntegritySecret = store.loadRegisterIntegritySecret();

    QSettings settings;
    settings.remove(WhatSonTrialClientIdentityStore::defaultDeviceUuidSettingsKey());
    settings.remove(WhatSonTrialClientIdentityStore::defaultClientKeySettingsKey());
    settings.remove(WhatSonTrialClientIdentityStore::defaultRegisterIntegritySecretSettingsKey());
    settings.sync();

    WhatSonTrialClientIdentityStore reloadedStore = createIdentityStore(secureBackend);
    QCOMPARE(reloadedStore.ensureIdentity(), identity);
    QCOMPARE(reloadedStore.loadRegisterIntegritySecret(), registerIntegritySecret);
}

void WhatSonTrialPolicyTest::writeRegister_writesTrialRegisterXmlForCurrentClient()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(QDir(hubPath).filePath(QStringLiteral(".whatson"))));

    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    const WhatSonTrialClientIdentity identity = identityStore.ensureIdentity();

    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    QVERIFY2(registerXml.writeRegister(hubPath, identity, &errorMessage), qPrintable(errorMessage));
    QVERIFY(QFileInfo(QDir(hubPath).filePath(QStringLiteral(".whatson/trial_register.xml"))).isFile());

    const WhatSonTrialRegisterLoadResult loadResult = registerXml.loadRegister(hubPath);
    QVERIFY2(loadResult.errorMessage.isEmpty(), qPrintable(loadResult.errorMessage));
    QVERIFY(loadResult.integrityVerified);
    QCOMPARE(loadResult.identity, identity);

    QFile registerFile(QDir(hubPath).filePath(QStringLiteral(".whatson/trial_register.xml")));
    QVERIFY(registerFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(QString::fromUtf8(registerFile.readAll()).contains(QStringLiteral("<signature algorithm=\"HMAC-SHA256\">")));
}

void WhatSonTrialPolicyTest::refreshForDate_bypassesTrialWhenRegisterAuthenticated()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonRegisterManager registerManager;
    registerManager.setAuthenticated(true);

    WhatSonTrialActivationPolicy policy(installStore);
    policy.setRegisterManager(&registerManager);
    const WhatSonTrialActivationState state = policy.refreshForDate(QDate(2026, 4, 10));

    QVERIFY(state.active);
    QVERIFY(state.bypassedByAuthentication);
    QCOMPARE(state.installDate, QDate(2026, 1, 10));
    QCOMPARE(state.remainingDays, WhatSonTrialActivationPolicy::kDefaultTrialLengthDays);
    QCOMPARE(state.elapsedDays, 0);
    QVERIFY(!state.lastActiveDate.isValid());
}

void WhatSonTrialPolicyTest::evaluateAccess_allowsWshubWithinTrialWindow()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("Alpha.wshub"));

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    QVERIFY2(registerXml.writeRegister(hubPath, identityStore.ensureIdentity(), &errorMessage), qPrintable(errorMessage));

    WhatSonTrialWshubAccessBackend backend(installStore, nullptr, identityStore, registerXml);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 10));

    QVERIFY(decision.wshubTarget);
    QVERIFY(decision.allowed);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QVERIFY(decision.registerFilePresent);
    QVERIFY(decision.registerIntegrityVerified);
    QVERIFY(decision.clientKeyMatched);
    QVERIFY(!decision.clientIdentity.key.isEmpty());
    QCOMPARE(decision.clientIdentity, decision.hubIdentity);
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Expired.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonTrialWshubAccessBackend backend(
        installStore,
        nullptr,
        createIdentityStore(secureBackend),
        createRegisterXml(secureBackend));
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
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    installStore.storeInstallDate(QDate(2026, 1, 10));

    const QString hubUri =
        QStringLiteral("content://whatson.provider/document/Expired.wshub");
    WhatSonTrialWshubAccessBackend backend(
        installStore,
        nullptr,
        createIdentityStore(secureBackend),
        createRegisterXml(secureBackend));
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubUri, QDate(2026, 4, 10));

    QVERIFY(decision.wshubTarget);
    QVERIFY(!decision.allowed);
    QVERIFY(decision.restrictedByExpiredTrial);
    QCOMPARE(decision.normalizedTargetPath, hubUri);
}

void WhatSonTrialPolicyTest::evaluateAccess_blocksLocalWshubWithoutTrialRegister()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("MissingRegister.wshub"));

    WhatSonTrialWshubAccessBackend backend(
        createInstallStore(secureBackend),
        nullptr,
        createIdentityStore(secureBackend),
        createRegisterXml(secureBackend));
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 15));

    QVERIFY(!decision.allowed);
    QVERIFY(decision.wshubTarget);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QVERIFY(!decision.registerFilePresent);
    QVERIFY(!decision.clientKeyMatched);
    QVERIFY(decision.restrictedByMissingRegister);
    QVERIFY(decision.denialReason.contains(QStringLiteral("trial_register.xml")));
}

void WhatSonTrialPolicyTest::evaluateAccess_allowsWshubWhenRegisterKeyMatches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("Matching.wshub"));

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    const WhatSonTrialClientIdentity identity = identityStore.ensureIdentity();
    QVERIFY2(registerXml.writeRegister(hubPath, identity, &errorMessage), qPrintable(errorMessage));

    WhatSonTrialWshubAccessBackend backend(installStore, nullptr, identityStore, registerXml);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 15));

    QVERIFY(decision.allowed);
    QVERIFY(decision.wshubTarget);
    QVERIFY(decision.registerFilePresent);
    QVERIFY(decision.registerIntegrityVerified);
    QVERIFY(decision.clientKeyMatched);
    QVERIFY(!decision.restrictedByClientKeyMismatch);
    QCOMPARE(decision.clientIdentity, identity);
    QCOMPARE(decision.hubIdentity, identity);
    QCOMPARE(decision.denialReason, QString());
}

void WhatSonTrialPolicyTest::evaluateAccess_ignoresDeviceUuidMismatchWhenKeyMatches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("DeviceMismatch.wshub"));

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    const WhatSonTrialClientIdentity identity = identityStore.ensureIdentity();
    WhatSonTrialClientIdentity hubIdentity = identity;
    hubIdentity.deviceUuid = QStringLiteral("01234567-89ab-cdef-0123-456789abcdef");
    QVERIFY2(registerXml.writeRegister(hubPath, hubIdentity, &errorMessage), qPrintable(errorMessage));

    WhatSonTrialWshubAccessBackend backend(installStore, nullptr, identityStore, registerXml);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 15));

    QVERIFY(decision.allowed);
    QVERIFY(decision.registerFilePresent);
    QVERIFY(decision.registerIntegrityVerified);
    QVERIFY(decision.clientKeyMatched);
    QVERIFY(!decision.restrictedByClientKeyMismatch);
    QCOMPARE(decision.clientIdentity, identity);
    QCOMPARE(decision.hubIdentity.key, identity.key);
    QCOMPARE(decision.hubIdentity.deviceUuid, hubIdentity.deviceUuid);
}

void WhatSonTrialPolicyTest::evaluateAccess_blocksWshubWhenRegisterKeyDiffersFromClientKey()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("KeyMismatch.wshub"));

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    const WhatSonTrialClientIdentity identity = identityStore.ensureIdentity();
    WhatSonTrialClientIdentity hubIdentity = identity;
    hubIdentity.key = QStringLiteral("ABCDEFGH12345678IJKLMNOP87654321");
    QVERIFY2(registerXml.writeRegister(hubPath, hubIdentity, &errorMessage), qPrintable(errorMessage));

    WhatSonTrialWshubAccessBackend backend(installStore, nullptr, identityStore, registerXml);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 15));

    QVERIFY(!decision.allowed);
    QVERIFY(decision.registerFilePresent);
    QVERIFY(decision.registerIntegrityVerified);
    QVERIFY(!decision.clientKeyMatched);
    QVERIFY(!decision.restrictedByMissingRegister);
    QVERIFY(decision.restrictedByClientKeyMismatch);
    QCOMPARE(decision.clientIdentity, identity);
    QCOMPARE(decision.hubIdentity.key, hubIdentity.key);
    QVERIFY(decision.denialReason.contains(QStringLiteral("trial_register.xml")));
}

void WhatSonTrialPolicyTest::evaluateAccess_bypassesAllTrialGuardsWhenRegisterAuthenticated()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("Authenticated.wshub"));
    const QString registerPath = QDir(hubPath).filePath(QStringLiteral(".whatson/trial_register.xml"));

    QFile file(registerPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<trialRegister><deviceUUID>01234567-89ab-cdef-0123-456789abcdef</deviceUUID><key>bad</key></trialRegister>\n");
    file.close();

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonRegisterManager registerManager;
    registerManager.setAuthenticated(true);

    WhatSonTrialWshubAccessBackend backend(
        installStore,
        &registerManager,
        createIdentityStore(secureBackend),
        createRegisterXml(secureBackend));
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 4, 10));

    QVERIFY(decision.wshubTarget);
    QVERIFY(decision.allowed);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QVERIFY(decision.trialState.active);
    QVERIFY(decision.trialState.bypassedByAuthentication);
    QVERIFY(!decision.restrictedByMissingRegister);
    QVERIFY(!decision.restrictedByClientKeyMismatch);
    QCOMPARE(decision.denialReason, QString());
}

void WhatSonTrialPolicyTest::evaluateAccess_blocksInvalidTrialRegisterFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("InvalidRegister.wshub"));
    const QString registerPath = QDir(hubPath).filePath(QStringLiteral(".whatson/trial_register.xml"));
    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    QVERIFY2(registerXml.writeRegister(hubPath, identityStore.ensureIdentity(), &errorMessage), qPrintable(errorMessage));

    QFile file(registerPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString registerText = QString::fromUtf8(file.readAll());
    file.close();
    registerText.replace(QStringLiteral("<key>") + identityStore.loadClientKey() + QStringLiteral("</key>"),
                         QStringLiteral("<key>ABCDEFGH12345678IJKLMNOP87654321</key>"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    file.write(registerText.toUtf8());
    file.close();

    WhatSonTrialWshubAccessBackend backend(
        createInstallStore(secureBackend),
        nullptr,
        identityStore,
        registerXml);
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(hubPath, QDate(2026, 1, 15));

    QVERIFY(!decision.allowed);
    QVERIFY(decision.registerFilePresent);
    QVERIFY(!decision.registerIntegrityVerified);
    QVERIFY(!decision.clientKeyMatched);
    QVERIFY(!decision.restrictedByMissingRegister);
    QVERIFY(!decision.restrictedByClientKeyMismatch);
    QVERIFY(decision.restrictedByRegisterIntegrityFailure);
    QVERIFY(decision.hubIdentity.key.isEmpty());
    QVERIFY(decision.denialReason.contains(QStringLiteral("integrity")));
}

void WhatSonTrialPolicyTest::loadRegister_rejectsTamperedRegisterSignature()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString hubPath = createHubPath(tempDir, QStringLiteral("TamperedRegister.wshub"));
    const QString registerPath = QDir(hubPath).filePath(QStringLiteral(".whatson/trial_register.xml"));

    WhatSonTrialClientIdentityStore identityStore = createIdentityStore(secureBackend);
    WhatSonTrialRegisterXml registerXml = createRegisterXml(secureBackend);
    QString errorMessage;
    QVERIFY2(registerXml.writeRegister(hubPath, identityStore.ensureIdentity(), &errorMessage), qPrintable(errorMessage));

    QFile file(registerPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString registerText = QString::fromUtf8(file.readAll());
    file.close();
    registerText.replace(QStringLiteral("<deviceUUID>"), QStringLiteral("<deviceUUID>badbadba-"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    file.write(registerText.toUtf8());
    file.close();

    const WhatSonTrialRegisterLoadResult loadResult = registerXml.loadRegister(hubPath);
    QVERIFY(loadResult.identity.key.isEmpty());
    QVERIFY(!loadResult.integrityVerified);
    QVERIFY(loadResult.errorMessage.contains(QStringLiteral("integrity")));
}

void WhatSonTrialPolicyTest::evaluateAccess_ignoresNonWshubTargetsAfterTrialExpiry()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    prepareIsolatedSettings(tempDir);
    const auto secureBackend = std::make_shared<MemoryTrialSecureStoreBackend>();

    const QString nonHubPath = QDir(tempDir.path()).filePath(QStringLiteral("notes.txt"));
    QFile file(nonHubPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("sample");
    file.close();

    WhatSonTrialInstallStore installStore = createInstallStore(secureBackend);
    installStore.storeInstallDate(QDate(2026, 1, 10));

    WhatSonTrialWshubAccessBackend backend(
        installStore,
        nullptr,
        createIdentityStore(secureBackend),
        createRegisterXml(secureBackend));
    const WhatSonTrialWshubAccessDecision decision =
        backend.evaluateAccess(nonHubPath, QDate(2026, 4, 10));

    QVERIFY(!decision.wshubTarget);
    QVERIFY(decision.allowed);
    QVERIFY(!decision.restrictedByExpiredTrial);
    QCOMPARE(decision.denialReason, QString());
}

QTEST_APPLESS_MAIN(WhatSonTrialPolicyTest)

#include "test_trial_policy.moc"
