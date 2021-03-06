// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_INPUT_INPUT_SCROLL_ELASTICITY_CONTROLLER_H_
#define CONTENT_RENDERER_INPUT_INPUT_SCROLL_ELASTICITY_CONTROLLER_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "cc/input/scroll_elasticity_helper.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"

// InputScrollElasticityController is based on
// WebKit/Source/platform/mac/ScrollElasticityController.h
/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

namespace cc {
struct InputHandlerScrollResult;
}  // namespace cc

namespace content {

class InputScrollElasticityController {
 public:
  explicit InputScrollElasticityController(cc::ScrollElasticityHelper* helper);
  virtual ~InputScrollElasticityController();

  base::WeakPtr<InputScrollElasticityController> GetWeakPtr();
  void ObserveWheelEventAndResult(
      const blink::WebMouseWheelEvent& wheel_event,
      const cc::InputHandlerScrollResult& scroll_result);

  void Animate(base::TimeTicks time);

 private:
  // This method is responsible for both scrolling and rubber-banding.
  //
  // Events are passed by IPC from the embedder. Events on Mac are grouped
  // into "gestures". If this method returns 'true', then this object has
  // handled the event. It expects the embedder to continue to forward events
  // from the gesture.
  //
  // This method makes the assumption that there is only 1 input device being
  // used at a time. If the user simultaneously uses multiple input devices,
  // Cocoa does not correctly pass all the gestureBegin/End events. The state
  // of this class is guaranteed to become eventually consistent, once the
  // user stops using multiple input devices.
  bool HandleWheelEvent(const blink::WebMouseWheelEvent& wheel_event);
  void SnapRubberbandTimerFired();

  bool IsRubberbandInProgress() const;

  void StopSnapRubberbandTimer();
  void SnapRubberband();

  // This method determines whether a given event should be handled. The
  // logic for control events of gestures (PhaseBegan, PhaseEnded) is handled
  // elsewhere.
  //
  // This class handles almost all wheel events. All of the following
  // conditions must be met for this class to ignore an event:
  // + No previous events in this gesture have caused any scrolling or rubber
  // banding.
  // + The event contains a horizontal component.
  // + The helper's view is pinned in the horizontal direction of the event.
  // + The wheel event disallows rubber banding in the horizontal direction
  // of the event.
  bool ShouldHandleEvent(const blink::WebMouseWheelEvent& wheel_event);

  cc::ScrollElasticityHelper* helper_;

  // There is an active scroll gesture event. This parameter only gets set to
  // false after the rubber band has been snapped, and before a new gesture
  // has begun. A careful audit of the code may deprecate the need for this
  // parameter.
  bool in_scroll_gesture_;
  // At least one event in the current gesture has been consumed and has
  // caused the view to scroll or rubber band. All future events in this
  // gesture will be consumed and overscrolls will cause rubberbanding.
  bool has_scrolled_;
  bool momentum_scroll_in_progress_;
  bool ignore_momentum_scrolls_;

  // Used with blink::WebInputEvent::timeStampSeconds, in seconds since epoch.
  double last_momentum_scroll_timestamp_;
  gfx::Vector2dF overflow_scroll_delta_;
  gfx::Vector2dF stretch_scroll_force_;
  gfx::Vector2dF momentum_velocity_;

  // Rubber band state.
  base::Time start_time_;
  gfx::Vector2dF start_stretch_;
  gfx::Vector2dF orig_origin_;
  gfx::Vector2dF orig_velocity_;

  bool snap_rubberband_timer_is_active_;

  base::WeakPtrFactory<InputScrollElasticityController> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(InputScrollElasticityController);
};

}  // namespace content

#endif  // CONTENT_RENDERER_INPUT_INPUT_SCROLL_ELASTICITY_CONTROLLER_H_
