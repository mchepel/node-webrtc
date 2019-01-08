/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcvideosource.h"

#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/media/base/videocapturer.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/converters/dictionaries.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"
#include "src/mediastreamtrack.h"
#include "src/peerconnectionfactory.h"
#include "src/webrtc/fakevideocapturer.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCVideoSource::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

node_webrtc::RTCVideoSource::RTCVideoSource() {
  _source = new rtc::RefCountedObject<RTCVideoTrackSource>();
}

NAN_METHOD(node_webrtc::RTCVideoSource::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSource.");
  }

  auto instance = new RTCVideoSource();
  instance->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::RTCVideoSource::CreateTrack) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());

  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
  auto track = factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), self->_source);
  auto result = node_webrtc::MediaStreamTrack::wrap()->GetOrCreate(factory, track);

  info.GetReturnValue().Set(result->ToObject());
}

NAN_METHOD(node_webrtc::RTCVideoSource::OnFrame) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  self->_source->TriggerOnFrame();
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetNeedsDenoising) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  auto needsDenoising = node_webrtc::From<v8::Local<v8::Value>>(self->_source->needs_denoising());
  info.GetReturnValue().Set(needsDenoising.UnsafeFromValid());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetIsScreencast) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->is_screencast());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetRemote) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->remote());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetState) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  auto state = node_webrtc::From<v8::Local<v8::Value>>(self->_source->state());
  info.GetReturnValue().Set(state.UnsafeFromValid());
}

void node_webrtc::RTCVideoSource::Init(v8::Handle<v8::Object> exports) {
  auto tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCVideoSource").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createTrack", CreateTrack);
  Nan::SetPrototypeMethod(tpl, "onFrame", OnFrame);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("needsDenoising").ToLocalChecked(), GetNeedsDenoising, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("isScreencast").ToLocalChecked(), GetIsScreencast, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remote").ToLocalChecked(), GetRemote, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("state").ToLocalChecked(), GetState, nullptr);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCVideoSource").ToLocalChecked(), tpl->GetFunction());
}
