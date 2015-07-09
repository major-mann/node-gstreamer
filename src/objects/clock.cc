#include "clock.h"
#include "../helper.h"

using namespace nstr;

Clock::Clock(GstClock* clock) : LinkedObjectBase<Clock, GstClock>(clock, NULL, NULL) { }

Clock* Clock::Create(GstClock* clock) {
	return LinkedObjectBase<Clock, GstClock>::Create(clock);
}

Clock* Clock::Master() {
	GstClock* master = gst_clock_get_master(GetObject());
	if (master == NULL) {
		return NULL;
	} else {
		return Clock::Create(master);
	}
}
unsigned long Clock::Time() {
	return gst_clock_get_time(GetObject());
}
unsigned long Clock::InternalTime() {
	return gst_clock_get_internal_time(GetObject());
}

unsigned long Clock::GetResolution() {
	return gst_clock_get_resolution(GetObject());
}
unsigned long Clock::SetResolution(unsigned long time) {
	return gst_clock_set_resolution(GetObject(), time);
}
unsigned long Clock::GetTimeout() {
	return gst_clock_get_timeout(GetObject());
}

void Clock::InitializeV8Instance(Handle<Object> instance) {
	//Call the base initialization
	LinkedObjectBase<Clock, GstClock>::InitializeV8Instance(instance);

	//Prepare the data
	Local<External> data = External::New(Isolate::GetCurrent(), this);

	//Add the functions
	Helper::SetFunctionProp(instance, "master", Master, data);
	Helper::SetFunctionProp(instance, "time", Time, data);
	Helper::SetFunctionProp(instance, "internalTime", InternalTime, data);

	//Add the accessors
	Helper::SetAccessorProp(instance, "resolution", ResolutionGetter, ResolutionSetter, data);
	Helper::SetAccessorProp(instance, "timeout", TimeoutGetter, data);
}

Clock* Clock::Instance(Local<Value> value) {
	Local<External> data = Local<External>::Cast(value);
	return (Clock*)data->Value();
}

void Clock::Master(const FunctionCallbackInfo<Value>& args) {
	Clock* self = Instance(args.Data());
	Clock* master = self->Master();
	args.GetReturnValue().Set(master->GetV8Instance());
}
void Clock::Time(const FunctionCallbackInfo<Value>& args) {
	Clock* self = Instance(args.Data());
	unsigned long time = self->Time();
	args.GetReturnValue().Set(Helper::ToValue(time));
}
void Clock::InternalTime(const FunctionCallbackInfo<Value>& args) {
	Clock* self = Instance(args.Data());
	unsigned long time = self->InternalTime();
	args.GetReturnValue().Set(Helper::ToValue(time));
}

void Clock::ResolutionGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	Clock* self = Instance(info.Data());
	unsigned long resolution = self->GetResolution();
	info.GetReturnValue().Set(Helper::ToValue(resolution));
}

void Clock::ResolutionSetter(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info) {
	if (value->IsNumber()) {
		Clock* self = Instance(info.Data());
		double dr = value->NumberValue();
		unsigned long resolution = static_cast<unsigned long>(dr);
		resolution = self->SetResolution(resolution);
		info.GetReturnValue().Set(Helper::ToValue(resolution));
	} else {
		Helper::ThrowV8Error("resolution MUST be a number");
	}
}

void Clock::TimeoutGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	Clock* self = Instance(info.Data());
	unsigned long timeout = self->GetTimeout();
	info.GetReturnValue().Set(Helper::ToValue(timeout));
}