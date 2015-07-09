#include "element_factory.h"
#include "static_pad_template.h"
#include "../helper.h"

using namespace nstr;
using namespace v8;

ElementFactory* ElementFactory::Create(const char* factoryName) {
    GstElementFactory* ef = gst_element_factory_find(factoryName);
    if (ef == NULL) {
        Helper::ThrowV8Error("Unable to create factory named \"%s\"", factoryName);
        return NULL;
    } else {
        return PluginFeatureBase<ElementFactory, GstElementFactory>::Create(ef);
    }
}

ElementFactory* ElementFactory::Create(GstElementFactory* factory) {
    return PluginFeatureBase<ElementFactory, GstElementFactory>::Create(factory);
}

ElementFactory* ElementFactory::Create(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() > 0 && args[0]->IsString()) {
        String::Utf8Value uval(args[0]->ToString());
        return ElementFactory::Create(*uval);
    } else {
        Helper::ThrowV8Error("ElementFactory constructor expects name to be supplied");
        return NULL;
    }
}

void ElementFactory::Init(Handle<Object> instance) {
    LinkedObjectBase<ElementFactory, GstElementFactory>::Init(instance, "ElementFactory");
}

ElementFactory::ElementFactory(GstElementFactory* factory)
    : PluginFeatureBase<ElementFactory, GstElementFactory>(factory) { }

void ElementFactory::InitializeV8Instance(Handle<Object> instance) {
    //Allow the base parts to be added
    PluginFeatureBase<ElementFactory, GstElementFactory>::InitializeV8Instance(instance);

    Local<External> data = External::New(Isolate::GetCurrent(), this);
    Helper::SetFunctionProp(instance, "create", CreateElement, data);
    Helper::SetFunctionProp(instance, "type", GetType, data);
    Helper::SetFunctionProp(instance, "padTemplateCount", GetPadTemplateCount, data);
    Helper::SetFunctionProp(instance, "supportedUriType", SupportedUriType, data);
    Helper::SetFunctionProp(instance, "hasInterface", HasInterface, data);
    Helper::SetFunctionProp(instance, "staticPadTemplates", StaticPadTemplates, data);
    Helper::SetFunctionProp(instance, "canSinkAllCaps", CanSinkAllCaps, data);
    Helper::SetFunctionProp(instance, "canSrcAllCaps", CanSrcAllCaps, data);
    Helper::SetFunctionProp(instance, "canSinkAnyCaps", CanSinkAnyCaps, data);
    Helper::SetFunctionProp(instance, "canSrcAnyCaps", CanSrcAnyCaps, data);
}

Element* ElementFactory::CreateElement(const char* elementName) {
    if (elementName == NULL) {
        //TODO: Generate random name
    }
    GstElement* ele = gst_element_factory_create(GetObject(), elementName);
    if (ele == NULL) {
        return NULL;
    } else {
        Element* result = Element::Create(ele);
        return result;
    }
}

Handle<Object> ElementFactory::GetType() {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    GType type = gst_element_factory_get_element_type(GetObject());
    Helper::SetUIntProp(result, "id", type);
    const char* name = g_type_name(type);
    Helper::SetUtf8Prop(result, "name", name);
    return result;
}
uint ElementFactory::GetPadTemplateCount() {
    GstElementFactory* factory = GetObject();
    uint cnt = gst_element_factory_get_num_pad_templates(factory);
    return cnt;
}

const char* ElementFactory::SupportedUriType() {
    gint ut = gst_element_factory_get_uri_type(GetObject());
    const char* result;
    switch (ut) {
        case GST_URI_SRC:
            result = "src";
            break;
        case GST_URI_SINK:
            result = "sink";
            break;
        case GST_URI_UNKNOWN:
        default:
            result = "unknown";
            break;
    }
    return result;
}
bool ElementFactory::HasInterface(const char* name) {
    return gst_element_factory_has_interface(GetObject(), name);
}
bool ElementFactory::CanSinkAllCaps(Caps* caps) {
    return gst_element_factory_can_sink_all_caps(GetObject(), caps->GetObject());
}
bool ElementFactory::CanSrcAllCaps(Caps* caps) {
    return gst_element_factory_can_src_all_caps(GetObject(), caps->GetObject());
}
bool ElementFactory::CanSrcAnyCaps(Caps* caps) {
    return gst_element_factory_can_src_any_caps(GetObject(), caps->GetObject());
}
bool ElementFactory::CanSinkAnyCaps(Caps* caps) {
    return gst_element_factory_can_sink_any_caps(GetObject(), caps->GetObject());
}

Handle<Value> ConvertStaticPadTemplate(void* data) {
    GstStaticPadTemplate* gspt = (GstStaticPadTemplate*)data;
    StaticPadTemplate* spt = StaticPadTemplate::Create(gspt);
    Local<Object> v8i = spt->GetV8Instance();
    return v8i;
}

Handle<Array> ElementFactory::StaticPadTemplates() {
    const GList* templates = gst_element_factory_get_static_pad_templates(GetObject());
    Handle<Array> result = Helper::GListToArray(templates, &ConvertStaticPadTemplate);
    return result;
}

ElementFactory* ElementFactory::Instance(Handle<Value> data) {
    Handle<External> ext = Handle<External>::Cast(data);
    ElementFactory* ef = (ElementFactory*)ext->Value();
    return ef;
}

void ElementFactory::CreateElement(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 0 || !args[0]->IsString()) {
        Helper::ThrowV8Error("name MUST be supplied and MUST a string");
    } else {
        ElementFactory* self = Instance(args.Data());
        String::Utf8Value uval(args[0]->ToString());
        Element* ele = self->CreateElement(*uval);
        args.GetReturnValue().Set(ele->GetV8Instance());
    }
}

void ElementFactory::GetType(const FunctionCallbackInfo<v8::Value>& args) {
    ElementFactory* ins = Instance(args.Data());
    Local<Object> result = ins->GetType();
    args.GetReturnValue().Set(result);
}
void ElementFactory::GetPadTemplateCount(const FunctionCallbackInfo<v8::Value>& args) {
    ElementFactory* ins = Instance(args.Data());
    uint cnt = ins->GetPadTemplateCount();
    Local<Integer> i = Integer::NewFromUnsigned(Isolate::GetCurrent(), cnt);
    args.GetReturnValue().Set(i);
}
void ElementFactory::SupportedUriType(const FunctionCallbackInfo<v8::Value>& args) {
    ElementFactory* ins = Instance(args.Data());
    const char* sut = ins->SupportedUriType();
    Local<String> ssut = String::NewFromUtf8(Isolate::GetCurrent(), sut);
    args.GetReturnValue().Set(ssut);
}
void ElementFactory::HasInterface(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0 && args[0]->IsString()) {
        ElementFactory* ins = Instance(args.Data());
        String::Utf8Value uval(args[0]->ToString());
        args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), ins->HasInterface(*uval)));
    } else {
        Helper::ThrowV8Error("name MUST be a string");
    }
}
void ElementFactory::StaticPadTemplates(const FunctionCallbackInfo<v8::Value>& args) {
    ElementFactory* ins = Instance(args.Data());
    args.GetReturnValue().Set(ins->StaticPadTemplates());
}
void ElementFactory::CanSinkAllCaps(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 0 || !args[0]->IsObject()) {
        Helper::ThrowV8Error("caps MUST be an object");
    } else {
        ElementFactory* ins = Instance(args.Data());
        Caps* caps = ObjectWrap::Unwrap<Caps>(ins->GetV8Instance());
        bool result = ins->CanSinkAllCaps(caps);
        args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), result));
    }
}
void ElementFactory::CanSrcAllCaps(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 0 || !args[0]->IsObject()) {
        Helper::ThrowV8Error("caps MUST be an object");
    } else {
        ElementFactory* ins = Instance(args.Data());
        Caps* caps = ObjectWrap::Unwrap<Caps>(ins->GetV8Instance());
        bool result = ins->CanSrcAllCaps(caps);
        args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), result));
    }
}
void ElementFactory::CanSrcAnyCaps(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 0 || !args[0]->IsObject()) {
        Helper::ThrowV8Error("caps MUST be an object");
    } else {
        ElementFactory* ins = Instance(args.Data());
        Caps* caps = ObjectWrap::Unwrap<Caps>(ins->GetV8Instance());
        bool result = ins->CanSrcAnyCaps(caps);
        args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), result));
    }
}
void ElementFactory::CanSinkAnyCaps(const FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 0 || !args[0]->IsObject()) {
        Helper::ThrowV8Error("caps MUST be an object");
    } else {
        ElementFactory* ins = Instance(args.Data());
        Caps* caps = ObjectWrap::Unwrap<Caps>(ins->GetV8Instance());
        bool result = ins->CanSinkAnyCaps(caps);
        args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), result));
    }
}