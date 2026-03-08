#include "ApplePermissionBridge.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QTimer>
#include <QUrl>
#include <QtCore/qglobal.h>

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#import <Foundation/Foundation.h>
#import <EventKit/EventKit.h>
#import <Photos/Photos.h>
#if defined(Q_OS_MACOS)
#import <ApplicationServices/ApplicationServices.h>
#endif

@ interface LocalNetworkPermissionRequester : NSObject<NSNetServiceBrowserDelegate>
@ property(nonatomic, strong) NSNetServiceBrowser*browser;
@ property(nonatomic, copy) void (^completion)(BOOL granted);
@ property(nonatomic, assign) BOOL finished;
@ end

@ implementation LocalNetworkPermissionRequester
- (void)finish : (BOOL)granted
{
    if (self.finished)
    {
        return;
    }
    self.finished = YES;
    [self.browser stop];
    if (self.completion)
    {
        self.completion(granted);
    }
}

- (void)start
{
    self.browser =
    [[NSNetServiceBrowser alloc] init];
    self.browser.delegate = self;
    [self.browser searchForServicesOfType:@ "_whatson._tcp." inDomain:@ "local."];

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)),
        dispatch_get_main_queue(),
        ^{
            [self finish:NO];












        });
}

- (void)netServiceBrowserWillSearch : (NSNetServiceBrowser *)browser
{
    Q_UNUSED(browser);
}

- (void)netServiceBrowser : (NSNetServiceBrowser *)browser didFindService: (NSNetService *)service moreComing: (BOOL)
moreComing
{
    Q_UNUSED(browser);
    Q_UNUSED(service);
    Q_UNUSED(moreComing);
    [self finish:YES];
}

- (void)netServiceBrowser : (NSNetServiceBrowser *)browser didNotSearch: (NSDictionary<NSString*, NSNumber*> *)errorDict
{
    Q_UNUSED(browser);
    Q_UNUSED(errorDict);
    [self finish:NO];
}
@end

static NSMutableArray<LocalNetworkPermissionRequester*>* pendingLocalNetworkRequesters()
{
    static NSMutableArray<LocalNetworkPermissionRequester*>* requesters = nil;
    static dispatch_once_t onceToken;
    dispatch_once(
        &onceToken,
        ^{
            requesters = [[NSMutableArray alloc] init];












        });
    return requesters;
}
#endif

namespace
{
    void completeOnQtMain(const WhatSon::Permissions::PermissionCallback& completion, bool granted)
    {
        if (!completion)
        {
            return;
        }

        if (QCoreApplication* app = QCoreApplication::instance())
        {
            QMetaObject::invokeMethod(
                app,
                [completion, granted]()
                {
                    completion(granted);
                },
                Qt::QueuedConnection);
            return;
        }

        completion(granted);
    }

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    bool isPhotoGranted(PHAuthorizationStatus status)
    {
        return status == PHAuthorizationStatusAuthorized || status == PHAuthorizationStatusLimited;
    }

    bool isRemindersGranted(EKAuthorizationStatus status)
    {



#if defined(EKAuthorizationStatusFullAccess)
    if (status== EKAuthorizationStatusFullAccess|| status== EKAuthorizationStatusWriteOnly)
        {
            return true;
        }
#endif
#if defined(EKAuthorizationStatusAuthorized)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        if (status== EKAuthorizationStatusAuthorized)
        {
            return true;
        }
#pragma clang diagnostic pop
#endif
    return false;
    }

    bool hasFullDiskAccessHeuristic()
    {
        const QString homePath = QDir::homePath();
        const QStringList probePaths = {
            homePath + "/Library/Mail",
            homePath + "/Library/Messages/chat.db",
            homePath + "/Library/Safari/History.db",
        };

        bool foundCandidate = false;
        for (const QString& probePath : probePaths)
        {
            QFileInfo info(probePath);
            if (!info.exists())
            {
                continue;
            }

            foundCandidate = true;
            if (info.isDir())
            {
                QDir dir(probePath);
                const QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
                if (!entries.isEmpty() || dir.exists())
                {
                    return true;
                }
                continue;
            }

            QFile file(probePath);
            if (file.open(QIODevice::ReadOnly))
            {
                file.close();
                return true;
            }
        }

        return !foundCandidate;
    }

#endif
} // namespace

namespace WhatSon::Permissions
{
    void requestPhotoLibraryPermission(const PermissionCallback& completion)
    {
        const PermissionCallback completionCopy = completion;
        WhatSon::Debug::trace(QStringLiteral("permissions.photo"), QStringLiteral("request"));
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        if (@ available(macOS 11.0, iOS 14.0, *))
        {
            const PHAuthorizationStatus status =
                [PHPhotoLibrary authorizationStatusForAccessLevel:PHAccessLevelReadWrite];
            WhatSon::Debug::trace(
                QStringLiteral("permissions.photo"),
                QStringLiteral("status"),
                QStringLiteral("mode=readWrite value=%1").arg(static_cast<int>(status)));
            if (status == PHAuthorizationStatusNotDetermined)
            {
                [PHPhotoLibrary requestAuthorizationForAccessLevel:PHAccessLevelReadWrite
                    handler:^(PHAuthorizationStatus newStatus)
                    {
                    WhatSon::Debug::trace(
                        QStringLiteral("permissions.photo"),
                        QStringLiteral("callback"),
                        QStringLiteral("granted=%1 status=%2")
                        .arg(isPhotoGranted(newStatus) ? QStringLiteral("1") : QStringLiteral("0"))
                        .arg(static_cast<int>(newStatus)));
                    completeOnQtMain(completionCopy, isPhotoGranted(newStatus));
                    }];
                return;
            }
            WhatSon::Debug::trace(
                QStringLiteral("permissions.photo"),
                QStringLiteral("resolved"),
                QStringLiteral("granted=%1").arg(isPhotoGranted(status) ? QStringLiteral("1") : QStringLiteral("0")));
            completeOnQtMain(completionCopy, isPhotoGranted(status));
            return;
        }

        const PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
        WhatSon::Debug::trace(
            QStringLiteral("permissions.photo"),
            QStringLiteral("status"),
            QStringLiteral("mode=legacy value=%1").arg(static_cast<int>(status)));
        if (status == PHAuthorizationStatusNotDetermined)
        {
            [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus newStatus)
                {
                WhatSon::Debug::trace(
                    QStringLiteral("permissions.photo"),
                    QStringLiteral("callback"),
                    QStringLiteral("granted=%1 status=%2")
                    .arg(isPhotoGranted(newStatus) ? QStringLiteral("1") : QStringLiteral("0"))
                    .arg(static_cast<int>(newStatus)));
                completeOnQtMain(completionCopy, isPhotoGranted(newStatus));
                }];
            return;
        }
        WhatSon::Debug::trace(
            QStringLiteral("permissions.photo"),
            QStringLiteral("resolved"),
            QStringLiteral("granted=%1").arg(isPhotoGranted(status) ? QStringLiteral("1") : QStringLiteral("0")));
        completeOnQtMain(completionCopy, isPhotoGranted(status));
#else
        WhatSon::Debug::trace(QStringLiteral("permissions.photo"),
                              QStringLiteral("resolved"),
                              QStringLiteral("platformStubGranted=1"));
        completeOnQtMain(completionCopy, true);
#endif
    }

    void requestRemindersPermission(const PermissionCallback& completion)
    {
        const PermissionCallback completionCopy = completion;
        WhatSon::Debug::trace(QStringLiteral("permissions.reminders"), QStringLiteral("request"));
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        const EKAuthorizationStatus status = [EKEventStore authorizationStatusForEntityType:EKEntityTypeReminder];
        WhatSon::Debug::trace(
            QStringLiteral("permissions.reminders"),
            QStringLiteral("status"),
            QStringLiteral("value=%1").arg(static_cast<int>(status)));
        if (status != EKAuthorizationStatusNotDetermined)
        {
            WhatSon::Debug::trace(
                QStringLiteral("permissions.reminders"),
                QStringLiteral("resolved"),
                QStringLiteral("granted=%1")
                .arg(isRemindersGranted(status) ? QStringLiteral("1") : QStringLiteral("0")));
            completeOnQtMain(completionCopy, isRemindersGranted(status));
            return;
        }

        EKEventStore* eventStore =  [[EKEventStore alloc] init];
        if (@ available(macOS 14.0, iOS 17.0, *))
        {
            [eventStore requestFullAccessToRemindersWithCompletion:^(BOOL granted, NSError* error)
                {
                Q_UNUSED(error);
                WhatSon::Debug::trace(
                    QStringLiteral("permissions.reminders"),
                    QStringLiteral("callback"),
                    QStringLiteral("granted=%1").arg(granted ? QStringLiteral("1") : QStringLiteral("0")));
                completeOnQtMain(completionCopy, granted);
                }];
            return;
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [eventStore requestAccessToEntityType:EKEntityTypeReminder
            completion:^(BOOL granted, NSError* error)
            {
            Q_UNUSED(error);
            WhatSon::Debug::trace(
                QStringLiteral("permissions.reminders"),
                QStringLiteral("callback"),
                QStringLiteral("granted=%1").arg(granted ? QStringLiteral("1") : QStringLiteral("0")));
            completeOnQtMain(completionCopy, granted);
            }];
#pragma clang diagnostic pop
#else
        WhatSon::Debug::trace(QStringLiteral("permissions.reminders"),
                              QStringLiteral("resolved"),
                              QStringLiteral("platformStubGranted=1"));
        completeOnQtMain(completionCopy, true);
#endif
    }

    void requestAccessibilityPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.accessibility"), QStringLiteral("request"));
#if defined(Q_OS_MACOS)
        const void* keys[] = {kAXTrustedCheckOptionPrompt};
        const void* values[] = {kCFBooleanTrue};
        CFDictionaryRef options = CFDictionaryCreate(
            kCFAllocatorDefault,
            keys,
            values,
            1,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
        const bool granted = AXIsProcessTrustedWithOptions(options);
        if (options)
        {
            CFRelease(options);
        }
        WhatSon::Debug::trace(
            QStringLiteral("permissions.accessibility"),
            QStringLiteral("resolved"),
            QStringLiteral("granted=%1").arg(granted ? QStringLiteral("1") : QStringLiteral("0")));
        completeOnQtMain(completion, granted);
#else
        WhatSon::Debug::trace(QStringLiteral("permissions.accessibility"),
                              QStringLiteral("resolved"),
                              QStringLiteral("platformStubGranted=1"));
        completeOnQtMain(completion, true);
#endif
    }

    void requestLocalNetworkPermission(const PermissionCallback& completion)
    {
        const PermissionCallback completionCopy = completion;
        WhatSon::Debug::trace(QStringLiteral("permissions.localNetwork"), QStringLiteral("request"));
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        LocalNetworkPermissionRequester* requester =  [[LocalNetworkPermissionRequester alloc] init];
        requester.completion =  ^ (BOOL granted)
        {
            WhatSon::Debug::trace(
                QStringLiteral("permissions.localNetwork"),
                QStringLiteral("callback"),
                QStringLiteral("granted=%1").arg(granted ? QStringLiteral("1") : QStringLiteral("0")));
            completeOnQtMain(completionCopy, granted);
        };

        NSMutableArray<LocalNetworkPermissionRequester*>* requesters = pendingLocalNetworkRequesters();
        [requesters addObject:requester];
        WhatSon::Debug::trace(
            QStringLiteral("permissions.localNetwork"),
            QStringLiteral("requesterQueued"),
            QStringLiteral("pendingCount=%1").arg(static_cast<int>(requesters.count)));
        [requester start];
#else
        WhatSon::Debug::trace(QStringLiteral("permissions.localNetwork"),
                              QStringLiteral("resolved"),
                              QStringLiteral("platformStubGranted=1"));
        completeOnQtMain(completionCopy, true);
#endif
    }

    void requestFullDiskAccessPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.fullDisk"), QStringLiteral("request"));
#if defined(Q_OS_MACOS)
        if (hasFullDiskAccessHeuristic())
        {
            WhatSon::Debug::trace(
                QStringLiteral("permissions.fullDisk"),
                QStringLiteral("resolved"),
                QStringLiteral("granted=1"));
            completeOnQtMain(completion, true);
            return;
        }

        WhatSon::Debug::trace(
            QStringLiteral("permissions.fullDisk"),
            QStringLiteral("openSettings"),
            QStringLiteral("granted=0"));
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("x-apple.systempreferences:com.apple.preference.security?Privacy_AllFiles")));
        completeOnQtMain(completion, false);
#else
        WhatSon::Debug::trace(QStringLiteral("permissions.fullDisk"),
                              QStringLiteral("resolved"),
                              QStringLiteral("platformStubGranted=1"));
        completeOnQtMain(completion, true);
#endif
    }
} // namespace WhatSon::Permissions
