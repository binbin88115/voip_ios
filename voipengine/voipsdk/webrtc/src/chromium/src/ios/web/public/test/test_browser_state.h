// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_PUBLIC_TEST_TEST_BROWSER_STATE_H_
#define IOS_WEB_PUBLIC_TEST_TEST_BROWSER_STATE_H_

#include "ios/web/public/browser_state.h"

namespace web {
class TestBrowserState : public BrowserState {
 public:
  TestBrowserState();
  ~TestBrowserState() override;

  // BrowserState:
  bool IsOffTheRecord() const override;
  base::FilePath GetPath() const override;
  net::URLRequestContextGetter* GetRequestContext() override;
};
}  // namespace web

#endif  // IOS_WEB_PUBLIC_TEST_TEST_BROWSER_STATE_H_
