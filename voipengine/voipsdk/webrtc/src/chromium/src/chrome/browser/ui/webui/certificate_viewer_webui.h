// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CERTIFICATE_VIEWER_WEBUI_H_
#define CHROME_BROWSER_UI_WEBUI_CERTIFICATE_VIEWER_WEBUI_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/values.h"
#include "components/web_modal/native_web_contents_modal_dialog.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "net/cert/x509_certificate.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/web_dialogs/web_dialog_delegate.h"

namespace content {
class WebContents;
}

class ConstrainedWebDialogDelegate;

// Modal dialog for displaying detailed certificate information. This is used in
// chromeos builds to display detailed information in a modal dialog when the
// user clicks on "View" from the Certificate Manager dialog.
class CertificateViewerModalDialog : public ui::WebDialogDelegate {
 public:
  // Construct a certificate viewer for the passed in certificate. A reference
  // to the certificate pointer is added for the lifetime of the certificate
  // viewer.
  explicit CertificateViewerModalDialog(
      net::X509Certificate* cert);
  ~CertificateViewerModalDialog() override;

  virtual void Show(content::WebContents* web_contents,
                    gfx::NativeWindow parent);
  virtual web_modal::NativeWebContentsModalDialog
  GetNativeWebContentsModalDialog();
  const content::WebUI* GetWebUI() const { return webui_; }

 protected:
  // Overridden from ui::WebDialogDelegate:
  ui::ModalType GetDialogModalType() const override;
  base::string16 GetDialogTitle() const override;
  GURL GetDialogContentURL() const override;
  void GetWebUIMessageHandlers(
      std::vector<content::WebUIMessageHandler*>* handlers) const override;
  void GetDialogSize(gfx::Size* size) const override;
  std::string GetDialogArgs() const override;
  void OnDialogShown(content::WebUI* webui,
                     content::RenderViewHost* render_view_host) override;
  void OnDialogClosed(const std::string& json_retval) override;
  void OnCloseContents(content::WebContents* source,
                       bool* out_close_dialog) override;
  bool ShouldShowDialogTitle() const override;

  // The certificate being viewed.
  scoped_refptr<net::X509Certificate> cert_;

  // The title of the certificate viewer dialog, Certificate Viewer: CN.
  base::string16 title_;

 private:
  content::WebUI* webui_;
  gfx::NativeWindow window_;
  DISALLOW_COPY_AND_ASSIGN(CertificateViewerModalDialog);
};

// Dialog for displaying detailed certificate information. This is used in linux
// and chromeos builds to display detailed information in a floating dialog when
// the user clicks on "Certificate Information" from the lock icon of a web site
// or "View" from the Certificate Manager.
class CertificateViewerDialog : public CertificateViewerModalDialog {
 public:
  // Construct a certificate viewer for the passed in certificate. A reference
  // to the certificate pointer is added for the lifetime of the certificate
  // viewer.
  explicit CertificateViewerDialog(net::X509Certificate* cert);
  ~CertificateViewerDialog() override;

  // CertificateViewerModalDialog overrides.
  void Show(content::WebContents* web_contents,
            gfx::NativeWindow parent) override;
  web_modal::NativeWebContentsModalDialog GetNativeWebContentsModalDialog()
      override;

 protected:
  // Overridden from ui::WebDialogDelegate:
  GURL GetDialogContentURL() const override;
  ui::ModalType GetDialogModalType() const override;

 private:
  ConstrainedWebDialogDelegate* dialog_;

  DISALLOW_COPY_AND_ASSIGN(CertificateViewerDialog);
};

// Dialog handler which handles calls from the JS WebUI code to view certificate
// details and export the certificate.
class CertificateViewerDialogHandler : public content::WebUIMessageHandler {
 public:
  CertificateViewerDialogHandler(CertificateViewerModalDialog* dialog,
                                 net::X509Certificate* cert);
  ~CertificateViewerDialogHandler() override;

  // Overridden from WebUIMessageHandler
  void RegisterMessages() override;

 private:
  // Brings up the export certificate dialog for the chosen certificate in the
  // chain.
  //
  // The input is an integer index to the certificate in the chain to export.
  void ExportCertificate(const base::ListValue* args);

  // Gets the details for a specific certificate in the certificate chain. Calls
  // the javascript function cert_viewer.getCertificateFields with a tree
  // structure containing the fields and values for certain nodes.
  //
  // The input is an integer index to the certificate in the chain to view.
  void RequestCertificateFields(const base::ListValue* args);

  // Helper function to get the certificate index from |args|. Returns -1 if
  // the index is out of range.
  int GetCertificateIndex(const base::ListValue* args) const;

  // The certificate being viewed.
  scoped_refptr<net::X509Certificate> cert_;

  // The dialog.
  CertificateViewerModalDialog* dialog_;

  // The certificate chain. Does not take references, so only valid as long as
  // |cert_| is.
  net::X509Certificate::OSCertHandles cert_chain_;

  DISALLOW_COPY_AND_ASSIGN(CertificateViewerDialogHandler);
};

#endif  // CHROME_BROWSER_UI_WEBUI_CERTIFICATE_VIEWER_WEBUI_H_
