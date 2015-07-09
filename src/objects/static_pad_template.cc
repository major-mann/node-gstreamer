#include <gst/gst.h>
#include "static_pad_template.h"
#include "../helper.h"

using namespace v8;
using namespace nstr;

StaticPadTemplate::StaticPadTemplate(GstStaticPadTemplate* spt) 
    : LinkedObjectBase<StaticPadTemplate, GstStaticPadTemplate>(spt, 0, 0) { }

StaticPadTemplate* StaticPadTemplate::Create(GstStaticPadTemplate* spt) {
    return LinkedObjectBase<StaticPadTemplate, GstStaticPadTemplate>::Create(spt);
}

Caps* StaticPadTemplate::GetCaps() {
    //StaticPadTemplate* spt = 
    GstCaps* caps = gst_static_pad_template_get_caps(GetObject());
    Caps* c;
    if (caps == NULL) {
        c = NULL;
    } else {
        c = Caps::Create(caps);
    }
    return c;
}

/** Gets the pad direction */
const char* StaticPadTemplate::Direction() {
    GstPadDirection direc = GetObject()->direction;

    switch (direc) {
        case GST_PAD_SRC:
            return "src";
        case GST_PAD_SINK:
            return "sink";
        case GST_PAD_UNKNOWN:
        default:
            return "unknown";
    }
}

/** Gets the pad presence */
const char* StaticPadTemplate::Presence() {
    GstPadPresence pres = GetObject()->presence;
    switch (pres) {
        case GST_PAD_ALWAYS:
            return "always";
        case GST_PAD_SOMETIMES:
            return "sometimes";
        case GST_PAD_REQUEST:
        default:
            return "request";
    }
}

PadTemplate* StaticPadTemplate::GetPadTemplate() {
    GstPadTemplate* pt = gst_static_pad_template_get(GetObject());
    return PadTemplate::Create(pt);
}

void StaticPadTemplate::InitializeV8Instance(Handle<Object> instance) {
    Local<External> data = External::New(Isolate::GetCurrent(), this);
    Helper::SetFunctionProp(instance, "caps", GetCaps, data);
    Helper::SetFunctionProp(instance, "direction", Direction, data);
    Helper::SetFunctionProp(instance, "presence", Presence, data);
    Helper::SetFunctionProp(instance, "padTemplate", GetPadTemplate, data);
}

StaticPadTemplate* StaticPadTemplate::Instance(Local<Value> value) {
    Local<External> ext = Local<External>::Cast(value);
    void* val = ext->Value();
    return (StaticPadTemplate*)val;
}

void StaticPadTemplate::GetCaps(const FunctionCallbackInfo<Value>& args) {
    StaticPadTemplate* instance = Instance(args.Data());
    Caps* caps = instance->GetCaps();
    if (caps == NULL) {
        args.GetReturnValue().Set(v8::Null(Isolate::GetCurrent()));
    } else {
        args.GetReturnValue().Set(caps->GetV8Instance());
    }
}

void StaticPadTemplate::Direction(const FunctionCallbackInfo<v8::Value>& args) {
    StaticPadTemplate* instance = Instance(args.Data());
    const char* direction = instance->Direction();
    args.GetReturnValue().Set(String::NewFromUtf8(Isolate::GetCurrent(), direction));
}

void StaticPadTemplate::Presence(const FunctionCallbackInfo<v8::Value>& args) {
    StaticPadTemplate* instance = Instance(args.Data());
    const char* presence = instance->Direction();
    args.GetReturnValue().Set(String::NewFromUtf8(Isolate::GetCurrent(), presence));
}

void StaticPadTemplate::GetPadTemplate(const FunctionCallbackInfo<v8::Value>& args) {
    StaticPadTemplate* instance = Instance(args.Data());
    PadTemplate* pt = instance->GetPadTemplate();
    args.GetReturnValue().Set(pt->GetV8Instance());
}