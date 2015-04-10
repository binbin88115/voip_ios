// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DELAYED_CALLBACK_RUNNER_H_
#define CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DELAYED_CALLBACK_RUNNER_H_

#include <list>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/task_runner.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace safe_browsing {

// Runs callbacks on a given task runner, waiting a certain amount of time
// between each. The delay also applies to running the first callback (i.e.,
// the first callback will be run some time after Start() is invoked). Callbacks
// are deleted after they are run. Start() is idempotent: calling it while the
// runner is doing its job has no effect.
class DelayedCallbackRunner {
 public:
  // Constructs an instance that runs tasks on |callback_runner|, waiting for
  // |delay| time to pass before and between each callback.
  DelayedCallbackRunner(base::TimeDelta delay,
                        const scoped_refptr<base::TaskRunner>& task_runner);
  ~DelayedCallbackRunner();

  // Registers |callback| with the runner. A copy of |callback| is held until it
  // is run.
  void RegisterCallback(const base::Closure& callback);

  // Starts running the callbacks after the delay.
  void Start();

 private:
  typedef std::list<base::Closure> CallbackList;

  // A callback invoked by the timer to run the next callback. The timer is
  // restarted to process the next callback if there is one.
  void OnTimer();

  base::ThreadChecker thread_checker_;

  // The runner on which callbacks are to be run.
  scoped_refptr<base::TaskRunner> task_runner_;

  // The list of callbacks to run. Callbacks are removed when run.
  CallbackList callbacks_;

  // callbacks_.end() when no work is being done. Any other value otherwise.
  CallbackList::iterator next_callback_;

  // A timer upon the firing of which the next callback will be run.
  base::DelayTimer<DelayedCallbackRunner> timer_;

  DISALLOW_COPY_AND_ASSIGN(DelayedCallbackRunner);
};

}  // namespace safe_browsing

#endif  // CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DELAYED_CALLBACK_RUNNER_H_