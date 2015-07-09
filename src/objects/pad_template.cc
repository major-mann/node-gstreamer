#include "pad_template.h"

using namespace nstr;

PadTemplate::PadTemplate(GstPadTemplate* padTemplate) : LinkedObjectBase<PadTemplate, GstPadTemplate>(padTemplate, Helper::GstRefWrap, Helper::GstUnrefWrap) {

}

PadTemplate* PadTemplate::Create(GstPadTemplate* pt) {
    return LinkedObjectBase<PadTemplate, GstPadTemplate>::Create(pt);
}

Caps* PadTemplate::GetCaps() {
    GstCaps* caps = gst_pad_template_get_caps(GetObject());
    Caps* c;
    if (caps == NULL) {
        c = NULL;
    } else {
        c = Caps::Create(caps);
    }
    return c;
}

const char* PadTemplate::NameTemplate() {
	return GST_PAD_TEMPLATE_NAME_TEMPLATE(GetObject());
}

const char* PadTemplate::Direction() {
	GstPadDirection direc = GST_PAD_TEMPLATE_DIRECTION(GetObject());

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

const char* PadTemplate::Presence() {
	GstPadPresence pres = GST_PAD_TEMPLATE_PRESENCE(GetObject());
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

void PadTemplate::InitializeV8Instance(Handle<Object> instance) {
	Local<External> data = External::New(Isolate::GetCurrent(), this);
	Helper::SetFunctionProp(instance, "nameTemplate", NameTemplate, data);
    Helper::SetFunctionProp(instance, "caps", GetCaps, data);
    Helper::SetFunctionProp(instance, "direction", Direction, data);
    Helper::SetFunctionProp(instance, "presence", Presence, data);
}

PadTemplate* PadTemplate::Instance(Local<Value> value) {
	Local<External> val = Local<External>::Cast(value);
	return (PadTemplate*)val->Value();
}

void PadTemplate::NameTemplate(const FunctionCallbackInfo<Value>& args) {
	PadTemplate* instance = Instance(args.Data());
	const char* nt = instance->NameTemplate();
	args.GetReturnValue().Set(String::NewFromUtf8(Isolate::GetCurrent(),nt));
}

void PadTemplate::GetCaps(const FunctionCallbackInfo<Value>& args) {
	PadTemplate* instance = Instance(args.Data());
	Caps* caps = instance->GetCaps();
    if (caps == NULL) {
        args.GetReturnValue().Set(v8::Null(Isolate::GetCurrent()));
    } else {
        args.GetReturnValue().Set(caps->GetV8Instance());
    }
}

void PadTemplate::Direction(const FunctionCallbackInfo<Value>& args) {
	PadTemplate* instance = Instance(args.Data());
	const char* direction = instance->Direction();
    args.GetReturnValue().Set(String::NewFromUtf8(Isolate::GetCurrent(), direction));
}

void PadTemplate::Presence(const FunctionCallbackInfo<Value>& args) {
	PadTemplate* instance = Instance(args.Data());
	const char* presence = instance->Presence();
    args.GetReturnValue().Set(String::NewFromUtf8(Isolate::GetCurrent(), presence));
}
