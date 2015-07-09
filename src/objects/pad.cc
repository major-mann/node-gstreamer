#include "pad.h"
#include "../helper.h"
#include <gst/gst.h>
#include "element.h"

using namespace nstr;

Pad::Pad(GstPad* pad) : LinkedObjectBase<Pad, GstPad>(pad, Helper::GstRefWrap, Helper::GstUnrefWrap) { }

Pad* Pad::Create(GstPad* pad) {
    return LinkedObjectBase<Pad, GstPad>::Create(pad);
}
Pad* Pad::Create(GstPadTemplate* tpl, const char* name) {
    GstPad* gpad = gst_pad_new_from_template(tpl, name);
    Pad* pad = LinkedObjectBase<Pad, GstPad>::Create(gpad);
    return pad;
}
Pad* Pad::Create(GstStaticPadTemplate* stpl, const char* name) {
    GstPad* gpad = gst_pad_new_from_static_template(stpl, name);
    Pad* pad = LinkedObjectBase<Pad, GstPad>::Create(gpad);
    return pad;
}
const char* Pad::Name() {
    return gst_pad_get_name(GetObject());
}
const char* Pad::Direction() {
    GstPadDirection pd = gst_pad_get_direction(GetObject());
    switch (pd) {
        case GST_PAD_SRC:
            return "src";
        case GST_PAD_SINK:
            return "sink";
        case GST_PAD_UNKNOWN:
        default:
            return "unknown";
    }
}
bool Pad::Linked() {
    return gst_pad_is_linked(GetObject());
}
bool Pad::Active() {
    return gst_pad_is_active(GetObject());
}
Element* Pad::Parent() {
    GstElement* gele = gst_pad_get_parent_element(GetObject());
    Element* ele = Element::Create(gele);
    gst_object_unref(gele);
    return ele;
}
PadTemplate* Pad::Template() {
    GstPadTemplate* gpt = gst_pad_get_pad_template(GetObject());
    PadTemplate* pt = PadTemplate::Create(gpt);
    gst_object_unref(gpt);
    return pt;
}
bool Pad::CanLink(Pad* other) {
    return gst_pad_can_link(GetObject(), other->GetObject());
}
const char* Pad::Link(Pad* other) {
    GstPadLinkReturn plr = gst_pad_link(GetObject(), other->GetObject());
    const char* plrName;
    switch (plr) {
        case GST_PAD_LINK_OK:
            plrName = "ok";
            break;
        case GST_PAD_LINK_WRONG_HIERARCHY:
            plrName = "wrong_hierarchy";
            break;
        case GST_PAD_LINK_WAS_LINKED:
            plrName = "already_linked";
            break;
        case GST_PAD_LINK_WRONG_DIRECTION:
            plrName = "wrong_direction";
            break;
        case GST_PAD_LINK_NOFORMAT:
            plrName = "no_common_format";
            break;
        case GST_PAD_LINK_NOSCHED:
            plrName = "cannot_schedule_cooperate";
            break;
        case GST_PAD_LINK_REFUSED:
            plrName = "refused";
            break;
        default:
            plrName = "unknown";
            break;
    }
    return plrName;
}
bool Pad::Unlink(Pad* other) {
    return gst_pad_unlink(GetObject(), other->GetObject());
}
Caps* Pad::AllowedCaps() {
    GstCaps* gcaps = gst_pad_get_allowed_caps(GetObject());
    Caps* caps = Caps::Create(gcaps);
    gst_caps_unref(gcaps);
    return caps;
}
Caps* Pad::TemplateCaps() {
    const GstCaps* gcaps = gst_pad_get_pad_template_caps(GetObject());
    Caps* caps = Caps::Create(gcaps);
    return caps;
}
Pad* Pad::Peer() {
    GstPad* gpad = gst_pad_get_peer(GetObject());
    Pad* pad = Pad::Create(gpad);
    gst_object_unref(gpad);
    return pad;
}
long Pad::Position(GstFormat format) {
    long position = 0;
    if (!gst_pad_query_position(GetObject(), format, &position)) {
        position = -1;
    }
    return position;
}
long Pad::Duration(GstFormat format) {
    long duration = 0;
    if (!gst_pad_query_duration(GetObject(), format, &duration)) {
        duration = -1;
    }
    return duration;
}

bool Pad::Blocked() {
    return gst_pad_is_blocked(GetObject());
}

bool Pad::Blocking() {
    return gst_pad_is_blocking(GetObject());
}

void Pad::InitializeV8Instance(Handle<Object> instance) {
    Local<External> data = External::New(Isolate::GetCurrent(), this);

    Helper::SetAccessorProp(instance, "name", NameGetter, data);
    Helper::SetAccessorProp(instance, "direction", DirectionGetter, data);
    Helper::SetAccessorProp(instance, "linked", LinkedGetter, data);
    Helper::SetAccessorProp(instance, "active", ActiveGetter, data);
    Helper::SetAccessorProp(instance, "blocked", BlockedGetter, data);
    Helper::SetAccessorProp(instance, "blocking", BlockingGetter, data);

    Helper::SetFunctionProp(instance, "parent", Parent, data);
    Helper::SetFunctionProp(instance, "template", Template, data);
    Helper::SetFunctionProp(instance, "canLink", CanLink, data);
    Helper::SetFunctionProp(instance, "link", Link, data);
    Helper::SetFunctionProp(instance, "unlink", Unlink, data);
    Helper::SetFunctionProp(instance, "allowedCaps", AllowedCaps, data);
    Helper::SetFunctionProp(instance, "templateCaps", TemplateCaps, data);
    Helper::SetFunctionProp(instance, "peer", Peer, data);
    Helper::SetFunctionProp(instance, "position", Position, data);
    Helper::SetFunctionProp(instance, "duration", Duration, data);
}

Pad* Pad::Unwrap(Handle<Value> value) {
    if (value.IsEmpty() || !value->IsObject()) {
        return NULL;
    } else {
        Local<Object> obj = value->ToObject();
        return node::ObjectWrap::Unwrap<Pad>(obj);
    }
}

Pad* Pad::Instance(Local<Value> data) {
    Local<External> ext = Local<External>::Cast(data);
    Pad* result = (Pad*)ext->Value();
    return result;
}

void Pad::NameGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToValue(instance->Name());
    info.GetReturnValue().Set(result);
}
void Pad::DirectionGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToValue(instance->Direction());
    info.GetReturnValue().Set(result);
}
void Pad::LinkedGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToValue(instance->Linked());
    info.GetReturnValue().Set(result);
}
void Pad::ActiveGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToValue(instance->Active());
    info.GetReturnValue().Set(result);
}

void Pad::BlockedGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToBooleanValue(instance->Blocked());
    info.GetReturnValue().Set(result);
}
void Pad::BlockingGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Pad* instance = Instance(info.Data());
    Local<Value> result = Helper::ToBooleanValue(instance->Blocking());
    info.GetReturnValue().Set(result);
}

void Pad::Parent(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Local<Value> result;
    Element* ele = instance->Parent();
    if (ele == NULL) {
        result = v8::Null(Isolate::GetCurrent());
    } else {
        result = ele->GetV8Instance();
    }

    args.GetReturnValue().Set(result);
}
void Pad::Template(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    PadTemplate* pt = instance->Template();
    Local<Value> result = pt->GetV8Instance();

    args.GetReturnValue().Set(result);
}
void Pad::CanLink(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Pad* other = NULL;
    if (args.Length() > 0 && args[0]->IsObject()) {
        other = Pad::Unwrap(args[0]);
    }

    Local<Value> result;
    if (other == NULL) {
        Helper::ThrowV8Error("canLink MUST be called with a pad as the argument");
        result = Undefined(Isolate::GetCurrent());
    } else {
        result = Helper::ToBooleanValue(instance->CanLink(other));
    }

    args.GetReturnValue().Set(result);
}
void Pad::Link(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Pad* other = NULL;
    if (args.Length() > 0 && args[0]->IsObject()) {
        other = Pad::Unwrap(args[0]);
    }

    Local<Value> result;
    if (other == NULL) {
        Helper::ThrowV8Error("link MUST be called with a pad as the argument");
        result = Undefined(Isolate::GetCurrent());
    } else {
        result = Helper::ToValue(instance->Link(other));
    }

    args.GetReturnValue().Set(result);
}
void Pad::Unlink(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Pad* other = NULL;
    if (args.Length() > 0 && args[0]->IsObject()) {
        other = Pad::Unwrap(args[0]);
    }

    Local<Value> result;
    if (other == NULL) {
        Helper::ThrowV8Error("unlink MUST be called with a pad as the argument");
        result = Undefined(Isolate::GetCurrent());
    } else {
        result = Helper::ToBooleanValue(instance->Unlink(other));
    }

    args.GetReturnValue().Set(result);
}
void Pad::AllowedCaps(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Caps* caps = instance->AllowedCaps();
    Local<Value> result = caps->GetV8Instance();

    args.GetReturnValue().Set(result);
}
void Pad::TemplateCaps(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Caps* caps = instance->TemplateCaps();
    Local<Value> result = caps->GetV8Instance();

    args.GetReturnValue().Set(result);
}
void Pad::Peer(const FunctionCallbackInfo<Value>& args) {
    Pad* instance = Instance(args.Data());

    Pad* peer = instance->Peer();
    Local<Value> result;
    if (peer == NULL) {
        result = Null(Isolate::GetCurrent());
    } else {
        result = peer->GetV8Instance();
    }

    args.GetReturnValue().Set(result);
}
void Pad::Position(const FunctionCallbackInfo<Value>& args) {
    GstFormat format;
    if (args.Length() == 0 || !args[0]->IsString()) {
        format = GST_FORMAT_UNDEFINED;
    } else {
        String::Utf8Value uval(args[0]->ToString());
        if (!StringToFormat(*uval, &format)) {
            //No return value on error
            return;
        }
    }

    Pad* instance = Instance(args.Data());

    long pos = instance->Position(format);
    Local<Value> result = Helper::ToValue(pos);

    args.GetReturnValue().Set(result);
}
void Pad::Duration(const FunctionCallbackInfo<Value>& args) {
    GstFormat format;
    if (args.Length() == 0 || !args[0]->IsString()) {
        format = GST_FORMAT_UNDEFINED;
    } else {
        String::Utf8Value uval(args[0]->ToString());
        if (!StringToFormat(*uval, &format)) {
            //No return value on error
            return;
        }
    }

    Pad* instance = Instance(args.Data());

    long dur = instance->Duration(format);
    Local<Value> result = Helper::ToValue(dur);

    args.GetReturnValue().Set(result);
}

bool Pad::StringToFormat(const char* str, GstFormat* format) {
    bool result = true;
    if (strcmp(str, "undefined") == 0) {
        *format = GST_FORMAT_UNDEFINED;
    } else if (strcmp(str, "default") == 0) {
        *format = GST_FORMAT_DEFAULT;
    } else if (strcmp(str, "bytes") == 0) {
        *format = GST_FORMAT_BYTES;
    } else if (strcmp(str, "time") == 0) {
        *format = GST_FORMAT_TIME;
    } else if (strcmp(str, "buffers") == 0) {
        *format = GST_FORMAT_BUFFERS;
    } else if (strcmp(str, "percent") == 0) {
        *format = GST_FORMAT_PERCENT;
    } else {
        //Unrecognized format value
        Helper::ThrowV8Error("Invalid format value");
        result = false;
    }
    return result;
}
