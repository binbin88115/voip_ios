// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_WIZARD_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_WIZARD_H_

#include <string>

namespace gfx {
class Size;
}

namespace chromeos {

// Shows the Chrome OS out-of-box / login UI.
void ShowLoginWizard(const std::string& start_screen);

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_WIZARD_H_
