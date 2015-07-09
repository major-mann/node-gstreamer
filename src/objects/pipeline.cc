#include "pipeline.h"

using namespace nstr;

Pipeline::Pipeline(GstElement* p) : Bin<Pipeline, GstElement>(p) { }

void Pipeline::Init(Handle<Object> instance) {
	LinkedObjectBase<Pipeline, GstElement>::Init(instance, "Pipeline");
}

Pipeline* Pipeline::Create(const char* name) {
	GstElement* pipeline = gst_pipeline_new(name);
	return Create(pipeline);
}

Pipeline* Pipeline::Create(GstElement* p) {
	return LinkedObjectBase<Pipeline, GstElement>::Create(p);
}

Pipeline* Pipeline::Create(const FunctionCallbackInfo<Value>& args) {
	if (args.Length() > 0 && args[0]->IsString()) {
		String::Utf8Value name(args[0]->ToString());
		return Create(*name);
	} else {
		Helper::ThrowV8Error("pipeline MUST have a name");
		return NULL;
	}
}

Pipeline* Pipeline::Unwrap(Handle<Value> value) {
	if (value->IsObject()) {
		return node::ObjectWrap::Unwrap<Pipeline>(value->ToObject());
	} else {
		return NULL;
	}
}

GstPipeline* Pipeline::GetPipeline() {
	return (GstPipeline*)GetObject();
}

unsigned long Pipeline::GetDelay() {
	return gst_pipeline_get_delay(GetPipeline());
}
void Pipeline::SetDelay(unsigned long value) {
	gst_pipeline_set_delay(GetPipeline(), value);
}

Bus* Pipeline::GetBus() {
	GstPipeline* pipeline = (GstPipeline*)GetObject();
	GstBus* gbus = gst_pipeline_get_bus(pipeline);
	Bus* bus = Bus::Create(gbus);
	return bus;
}

/**
* Adds the pipeline members to the v8 instance
*/
void Pipeline::InitializeV8Instance(Handle<Object> instance) {
	//Get the properties from base
	Bin<Pipeline, GstElement>::InitializeV8Instance(instance);

	//Set the properties from pipeline
	Local<External> data = External::New(Isolate::GetCurrent(), this);
	Helper::SetAccessorProp(instance, "delay", DelayGetAccessor, DelaySetAccessor, data);
	Helper::SetFunctionProp(instance, "bus", GetBus, data);
}

Pipeline* Pipeline::Instance(Local<Value> value) {
	Local<External> ext = Local<External>::Cast(value);
	return (Pipeline*)ext->Value();
}

void Pipeline::DelayGetAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	Pipeline* instance = Instance(info.Data());
	info.GetReturnValue().Set(Helper::ToValue(instance->GetDelay()));
}
void Pipeline::DelaySetAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info) {
	Pipeline* instance = Instance(info.Data());
	if (value->IsNumber()) {
		double d = value->NumberValue();
		unsigned long ul = static_cast<unsigned long>(d);
		instance->SetDelay(ul);
	} else {
		Helper::ThrowV8Error("delay MUST be a number");
	}
}

void Pipeline::GetBus(const FunctionCallbackInfo<Value>& args) {
	Pipeline* instance = Instance(args.Data());
	Bus* bus = instance->GetBus();
	args.GetReturnValue().Set(bus->GetV8Instance());
}
