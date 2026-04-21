#include "app/platform/Apple/WhatSonIosHubPickerBridge.hpp"

#if defined(Q_OS_IOS)

#import <UIKit/UIKit.h>
#import <UIKit/UIDocumentBrowserViewController.h>
#import <UIKit/UIDocumentBrowserAction.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <MobileCoreServices/MobileCoreServices.h>

#include <QMetaObject>
#include <QPointer>

namespace
{
    NSString* const kWhatSonHubPackageTypeIdentifier = @"com.iisacc.whatson.hub";
    NSString* const kWhatSonOpenHubActionIdentifier = @"com.iisacc.whatson.openHub";

    template<typename T>
    void appendUniqueObject(NSMutableArray<T>* target, T object)
    {
        if (target == nil || object == nil || [target containsObject:object])
        {
            return;
        }

        [target addObject:object];
    }

    UTType* whatSonHubPackageType()
    {
        if (@available(iOS 14.0, *))
        {
            UTType* declaredType = [UTType typeWithIdentifier:kWhatSonHubPackageTypeIdentifier];
            if (declaredType != nil)
            {
                return declaredType;
            }

            return [UTType typeWithFilenameExtension:@"wshub" conformingToType:UTTypePackage];
        }

        return nil;
    }

    NSArray<UTType*>* allowedHubContentTypes()
    {
        if (@available(iOS 14.0, *))
        {
            NSMutableArray<UTType*>* allowedContentTypes = [NSMutableArray array];
            appendUniqueObject<UTType*>(allowedContentTypes, whatSonHubPackageType());
            appendUniqueObject<UTType*>(allowedContentTypes, UTTypePackage);
            appendUniqueObject<UTType*>(allowedContentTypes, UTTypeContent);
            appendUniqueObject<UTType*>(allowedContentTypes, UTTypeItem);
            appendUniqueObject<UTType*>(allowedContentTypes, UTTypeData);
            return allowedContentTypes;
        }

        return @[];
    }

    NSArray<NSString*>* allowedHubContentTypeIdentifiers()
    {
        NSMutableArray<NSString*>* allowedTypeIdentifiers = [NSMutableArray array];
        appendUniqueObject<NSString*>(allowedTypeIdentifiers, kWhatSonHubPackageTypeIdentifier);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        appendUniqueObject<NSString*>(allowedTypeIdentifiers, (__bridge NSString*)kUTTypePackage);
        appendUniqueObject<NSString*>(allowedTypeIdentifiers, (__bridge NSString*)kUTTypeContent);
        appendUniqueObject<NSString*>(allowedTypeIdentifiers, (__bridge NSString*)kUTTypeItem);
        appendUniqueObject<NSString*>(allowedTypeIdentifiers, (__bridge NSString*)kUTTypeData);
#pragma clang diagnostic pop
        return allowedTypeIdentifiers;
    }

    UIWindow* activePresentationWindow()
    {
        UIApplication* application = UIApplication.sharedApplication;
        if (application == nil)
        {
            return nil;
        }

        if (@available(iOS 13.0, *))
        {
            for (UIScene* scene in application.connectedScenes)
            {
                if (![scene isKindOfClass:UIWindowScene.class])
                {
                    continue;
                }

                UIWindowScene* windowScene = (UIWindowScene*)scene;
                if (windowScene.activationState != UISceneActivationStateForegroundActive)
                {
                    continue;
                }

                for (UIWindow* window in windowScene.windows)
                {
                    if (window.isKeyWindow)
                    {
                        return window;
                    }
                }

                for (UIWindow* window in windowScene.windows)
                {
                    if (!window.hidden)
                    {
                        return window;
                    }
                }
            }
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        if (application.keyWindow != nil)
        {
            return application.keyWindow;
        }

        for (UIWindow* window in application.windows)
        {
            if (window.isKeyWindow)
            {
                return window;
            }
        }

        if (application.windows.count > 0)
        {
            return application.windows.firstObject;
        }
#pragma clang diagnostic pop

        return nil;
    }

    UIViewController* topPresenterForWindow(UIWindow* window)
    {
        if (window == nil)
        {
            return nil;
        }

        UIViewController* presenter = window.rootViewController;
        while (presenter.presentedViewController != nil && !presenter.presentedViewController.isBeingDismissed)
        {
            presenter = presenter.presentedViewController;
        }
        return presenter;
    }
}

@class WhatSonIosHubPickerDelegate;

class WhatSonIosHubPickerBridgePrivate
{
public:
    QPointer<WhatSonIosHubPickerBridge> bridge;
    UIDocumentBrowserViewController* controller = nil;
    WhatSonIosHubPickerDelegate* delegate = nil;
    bool finished = false;
};

static void dispatchBridgeAccept(WhatSonIosHubPickerBridgePrivate* owner, NSURL* url)
{
    if (owner == nullptr || owner->bridge == nullptr || url == nil || owner->finished)
    {
        return;
    }

    owner->finished = true;
    const QUrl selectedUrl = QUrl::fromNSURL(url);
    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerAccepted",
        Qt::QueuedConnection,
        Q_ARG(QUrl, selectedUrl));
}

static void dispatchBridgeCancel(WhatSonIosHubPickerBridgePrivate* owner)
{
    if (owner == nullptr || owner->bridge == nullptr || owner->finished)
    {
        return;
    }

    owner->finished = true;
    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerCanceled",
        Qt::QueuedConnection);
}

static void dispatchBridgeFailure(WhatSonIosHubPickerBridgePrivate* owner, NSString* message)
{
    if (owner == nullptr || owner->bridge == nullptr || owner->finished)
    {
        return;
    }

    owner->finished = true;
    const QString errorText = message == nil
                                  ? QStringLiteral("The native iOS hub browser failed.")
                                  : QString::fromUtf8(message.UTF8String).trimmed();
    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerFailed",
        Qt::QueuedConnection,
        Q_ARG(QString, errorText));
}

static UIDocumentBrowserAction* createOpenHubAction(WhatSonIosHubPickerBridgePrivate* owner)
{
    UIDocumentBrowserAction* openHubAction = [[UIDocumentBrowserAction alloc]
        initWithIdentifier:kWhatSonOpenHubActionIdentifier
            localizedTitle:@"Open"
               availability:(UIDocumentBrowserActionAvailabilityMenu
                             | UIDocumentBrowserActionAvailabilityNavigationBar)
                    handler:^(NSArray<NSURL*>* urls) {
                        NSURL* selectedUrl = urls.count > 0 ? urls.firstObject : nil;
                        if (selectedUrl == nil)
                        {
                            dispatchBridgeFailure(owner, @"The native iOS hub browser returned no selected URL.");
                            return;
                        }

                        dispatchBridgeAccept(owner, selectedUrl);
                    }];
    openHubAction.supportsMultipleItems = NO;
    return openHubAction;
}

static UIDocumentBrowserViewController* createHubBrowserController(
    WhatSonIosHubPickerBridgePrivate* owner,
    const QUrl& initialDirectoryUrl)
{
    Q_UNUSED(initialDirectoryUrl);

    UIDocumentBrowserViewController* controller = nil;
    if (@available(iOS 14.0, *))
    {
        controller = [[UIDocumentBrowserViewController alloc] initForOpeningContentTypes:allowedHubContentTypes()];
    }
    else
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        controller = [[UIDocumentBrowserViewController alloc]
            initForOpeningFilesWithContentTypes:allowedHubContentTypeIdentifiers()];
#pragma clang diagnostic pop
    }

    if (controller == nil)
    {
        return nil;
    }

    controller.allowsDocumentCreation = NO;
    controller.allowsPickingMultipleItems = NO;
    controller.modalPresentationStyle = UIModalPresentationFullScreen;
    if ([controller respondsToSelector:@selector(setShouldShowFileExtensions:)])
    {
        controller.shouldShowFileExtensions = YES;
    }

    UIDocumentBrowserAction* openHubAction = createOpenHubAction(owner);
    if (openHubAction != nil)
    {
        controller.customActions = @[openHubAction];
        [openHubAction release];
    }

    return controller;
}

@interface WhatSonIosHubPickerDelegate
    : NSObject<UIDocumentBrowserViewControllerDelegate, UIAdaptivePresentationControllerDelegate>
@property(nonatomic, assign) WhatSonIosHubPickerBridgePrivate* owner;
@property(nonatomic, assign) BOOL finished;
@end

@implementation WhatSonIosHubPickerDelegate

- (void)dispatchAcceptForUrl:(NSURL*)url
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    dispatchBridgeAccept(owner, url);
}

- (void)dispatchCancel
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    dispatchBridgeCancel(owner);
}

- (void)dispatchFailure:(NSString*)message
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    dispatchBridgeFailure(owner, message);
}

- (void)documentBrowser:(UIDocumentBrowserViewController*)controller didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls
{
    Q_UNUSED(controller);

    NSURL* selectedUrl = urls.count > 0 ? urls.firstObject : nil;
    if (selectedUrl == nil)
    {
        [self dispatchFailure:@"The native iOS hub picker returned no selected URL."];
        return;
    }

    [self dispatchAcceptForUrl:selectedUrl];
}

- (void)documentBrowser:(UIDocumentBrowserViewController*)controller didPickDocumentURLs:(NSArray<NSURL*>*)documentURLs
{
    Q_UNUSED(controller);

    NSURL* selectedUrl = documentURLs.count > 0 ? documentURLs.firstObject : nil;
    if (selectedUrl == nil)
    {
        [self dispatchFailure:@"The native iOS hub browser returned no selected URL."];
        return;
    }

    [self dispatchAcceptForUrl:selectedUrl];
}

- (void)presentationControllerDidDismiss:(UIPresentationController*)presentationController
{
    Q_UNUSED(presentationController);
    [self dispatchCancel];
}

@end

WhatSonIosHubPickerBridge::WhatSonIosHubPickerBridge(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<WhatSonIosHubPickerBridgePrivate>())
{
    d->bridge = this;
}

WhatSonIosHubPickerBridge::~WhatSonIosHubPickerBridge()
{
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
    }

    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller dismissViewControllerAnimated:NO completion:nil];
        [d->controller release];
        d->controller = nil;
    }

    if (d->delegate != nil)
    {
        [d->delegate release];
        d->delegate = nil;
    }
}

bool WhatSonIosHubPickerBridge::busy() const noexcept
{
    return m_busy;
}

QString WhatSonIosHubPickerBridge::lastError() const
{
    return m_lastError;
}

bool WhatSonIosHubPickerBridge::open(const QUrl& initialDirectoryUrl)
{
    clearLastError();
    if (m_busy)
    {
        setLastError(QStringLiteral("The native iOS hub picker is already open."));
        return false;
    }

    UIWindow* window = activePresentationWindow();
    UIViewController* presenter = topPresenterForWindow(window);
    if (presenter == nil)
    {
        setLastError(QStringLiteral("Failed to find an active iOS presenter for the hub picker."));
        return false;
    }

    UIDocumentBrowserViewController* controller = createHubBrowserController(d.get(), initialDirectoryUrl);
    if (controller == nil)
    {
        setLastError(QStringLiteral("Failed to create the native iOS hub browser."));
        return false;
    }

    WhatSonIosHubPickerDelegate* delegate = [[WhatSonIosHubPickerDelegate alloc] init];
    delegate.owner = d.get();
    delegate.finished = NO;
    d->finished = false;
    controller.delegate = delegate;
    if (controller.presentationController != nil)
    {
        controller.presentationController.delegate = delegate;
    }

    d->controller = controller;
    d->delegate = delegate;
    setBusy(true);
    [presenter presentViewController:controller animated:YES completion:nil];
    return true;
}

void WhatSonIosHubPickerBridge::clearLastError()
{
    setLastError(QString());
}

void WhatSonIosHubPickerBridge::handlePickerAccepted(const QUrl& selectedUrl)
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller dismissViewControllerAnimated:YES completion:nil];
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    if (!selectedUrl.isValid())
    {
        setLastError(QStringLiteral("The native iOS hub picker returned an invalid URL."));
        return;
    }

    clearLastError();
    emit accepted(selectedUrl);
}

void WhatSonIosHubPickerBridge::handlePickerCanceled()
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    clearLastError();
    emit canceled();
}

void WhatSonIosHubPickerBridge::handlePickerFailed(const QString& errorMessage)
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller dismissViewControllerAnimated:YES completion:nil];
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    setLastError(errorMessage);
}

void WhatSonIosHubPickerBridge::setBusy(const bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void WhatSonIosHubPickerBridge::setLastError(const QString& errorMessage)
{
    const QString normalizedError = errorMessage.trimmed();
    if (m_lastError == normalizedError)
    {
        return;
    }

    m_lastError = normalizedError;
    emit lastErrorChanged();
}

#endif
