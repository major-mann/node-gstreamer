#include "caps.h"
#include "structure.h"
#include "../helper.h"

using namespace nstr;

Caps::Caps(GstCaps* caps) 
	: LinkedObjectBase<Caps, GstCaps>(caps, Caps::RefWrap, gst_caps_unref) { }

Caps::Caps(const GstCaps* caps) 
	: LinkedObjectBase<Caps, GstCaps>(caps) { }

Caps* Caps::Create(GstCaps* caps) {
	return LinkedObjectBase<Caps, GstCaps>::Create(caps);
}

Caps* Unwrap(Handle<Value> value) {
	if (value.IsEmpty() || !value->IsObject()) {
		return NULL;
	} else {
		return node::ObjectWrap::Unwrap<Caps>(value->ToObject());
	}
}

void Caps::RefWrap(GstCaps* caps) {
	gst_caps_ref(caps);
}

void Caps::InitializeV8Instance(Handle<Object> instance) {
	Handle<External> data = External::New(Isolate::GetCurrent(), this);
	Helper::SetFunctionProp(instance, "structures", Structures, data);
	Helper::SetFunctionProp(instance, "equals", Equals, data);
}

Handle<Array> Caps::Structures() {
	Isolate* isolate = Isolate::GetCurrent();
	GstCaps* caps = GetObject();
	uint sz = gst_caps_get_size(caps);
	Local<Array> result = Array::New(isolate, sz);
	uint cnt = 0;
	for (uint i = 0; i < sz; i++) {
		GstStructure* structure = gst_caps_get_structure(caps, i);
		Structure* st = Structure::Create(structure);
		Local<Value> v8i = st->GetV8Instance();
		result->Set(cnt, v8i);
		cnt++;
	}
	return result;
}

bool Caps::Equals(Caps* caps) {
	const GstCaps* c1 = GetObject();
	const GstCaps* c2 = caps->GetObject();
	return gst_caps_is_equal(c1, c2);
}

void Caps::Structures(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Local<External> ext = Local<External>::Cast(args.Data());
	Caps* self = (Caps*)ext->Value();
	Local<Array> result = self->Structures();
	args.GetReturnValue().Set(result);
}

void Caps::Equals(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Local<External> ext = Local<External>::Cast(args.Data());
	Caps* self = (Caps*)ext->Value();

	if (args.Length() == 0 || !args[0]->IsObject()) {
		Helper::ThrowV8Error("MUST supply a caps object");
	} else {
		Caps* arg = ObjectWrap::Unwrap<Caps>(args[0]->ToObject());
		bool result = self->Equals(arg);
		args.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), result));
	}
}