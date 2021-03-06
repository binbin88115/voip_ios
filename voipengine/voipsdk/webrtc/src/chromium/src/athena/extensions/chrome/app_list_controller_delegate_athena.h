// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ATHENA_EXTENSIONS_CHROME_APP_LIST_CONTROLLER_DELEGATE_ATHENA_H_
#define ATHENA_EXTENSIONS_CHROME_APP_LIST_CONTROLLER_DELEGATE_ATHENA_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/browser/ui/app_list/app_list_controller_delegate.h"

namespace athena {

class AppListControllerDelegateAthena : public AppListControllerDelegate {
 public:
  AppListControllerDelegateAthena();
  ~AppListControllerDelegateAthena() override;

 private:
  // AppListControllerDelegate overrides:
  void DismissView() override;
  gfx::NativeWindow GetAppListWindow() override;
  gfx::Rect GetAppListBounds() override;
  gfx::ImageSkia GetWindowIcon() override;
  bool IsAppPinned(const std::string& extension_id) override;
  void PinApp(const std::string& extension_id) override;
  void UnpinApp(const std::string& extension_id) override;
  Pinnable GetPinnable() override;
  void OnShowChildDialog() override;
  void OnCloseChildDialog() override;
  bool CanDoCreateShortcutsFlow() override;
  void DoCreateShortcutsFlow(Profile* profile,
                             const std::string& extension_id) override;
  void CreateNewWindow(Profile* profile, bool incognito) override;
  void OpenURL(Profile* profile,
               const GURL& url,
               ui::PageTransition transition,
               WindowOpenDisposition disposition) override;
  void ActivateApp(Profile* profile,
                   const extensions::Extension* extension,
                   AppListSource source,
                   int event_flags) override;
  void LaunchApp(Profile* profile,
                 const extensions::Extension* extension,
                 AppListSource source,
                 int event_flags) override;
  void ShowForProfileByPath(const base::FilePath& profile_path) override;
  bool ShouldShowUserIcon() override;

  DISALLOW_COPY_AND_ASSIGN(AppListControllerDelegateAthena);
};

}  // namespace athena

#endif  // ATHENA_EXTENSIONS_CHROME_APP_LIST_CONTROLLER_DELEGATE_ATHENA_H_
