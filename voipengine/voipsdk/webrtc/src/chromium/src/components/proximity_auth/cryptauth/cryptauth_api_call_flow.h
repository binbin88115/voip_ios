// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_API_CALL_FLOW_H
#define COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_API_CALL_FLOW_H

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "google_apis/gaia/oauth2_api_call_flow.h"

namespace proximity_auth {

// Google API call flow implementation underlying all CryptAuth API calls.
// CryptAuth is a Google service that manages authorization and cryptographic
// credentials for users' devices (eg. public keys).
class CryptAuthApiCallFlow : public OAuth2ApiCallFlow {
 public:
  typedef base::Callback<void(const std::string& serialized_response)>
      ResultCallback;
  typedef base::Callback<void(const std::string& error_message)> ErrorCallback;

  CryptAuthApiCallFlow(const GURL& request_url);
  ~CryptAuthApiCallFlow() override;

  // Starts the API call.
  //   context: The URL context used to make the request.
  //   access_token: The access token for whom to make the to make the request.
  //   serialized_request: A serialized proto containing the request data.
  //   result_callback: Called when the flow completes successfully with a
  //       serialized response proto.
  //   error_callback: Called when the flow completes with an error.
  void Start(net::URLRequestContextGetter* context,
             const std::string& access_token,
             const std::string& serialized_request,
             ResultCallback result_callback,
             ErrorCallback error_callback);

 protected:
  // Reduce the visibility of OAuth2ApiCallFlow::Start() to avoid exposing
  // overloaded methods.
  using OAuth2ApiCallFlow::Start;

  // google_apis::OAuth2ApiCallFlow:
  GURL CreateApiCallUrl() override;
  std::string CreateApiCallBody() override;
  std::string CreateApiCallBodyContentType() override;
  void ProcessApiCallSuccess(const net::URLFetcher* source) override;
  void ProcessApiCallFailure(const net::URLFetcher* source) override;

 private:
  // The URL of the CryptAuth endpoint serving the request.
  const GURL request_url_;

  // Serialized request message proto that will be sent in the API request.
  std::string serialized_request_;

  // Callback invoked with the serialized response message proto when the flow
  // completes successfully.
  ResultCallback result_callback_;

  // Callback invoked with an error message when the flow fails.
  ErrorCallback error_callback_;

  DISALLOW_COPY_AND_ASSIGN(CryptAuthApiCallFlow);
};

}  // namespace proximity_auth

#endif  // COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_API_CALL_FLOW_H
