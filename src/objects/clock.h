#ifndef NSTR_CLOCK_H
#define NSTR_CLOCK_H

#include <node.h>
#include "linked_object_base.h"

namespace nstr {

	class Clock : public LinkedObjectBase<Clock, GstClock> {
		friend class LinkedObjectBase<Clock, GstClock>;
		public:
			static Clock* Create(GstClock* clock);

			Clock* Master();
			unsigned long Time();
			unsigned long InternalTime();

			unsigned long GetResolution();
			unsigned long SetResolution(unsigned long time);
			unsigned long GetTimeout();

		protected:
			Clock(GstClock* clock);
			void InitializeV8Instance(Handle<Object> instance);

		private:
			static Clock* Instance(Local<Value> value);

			static void Master(const FunctionCallbackInfo<Value>& args);
			static void Time(const FunctionCallbackInfo<Value>& args);
			static void InternalTime(const FunctionCallbackInfo<Value>& args);

			static void ResolutionGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void ResolutionSetter(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);
			static void TimeoutGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
	};

}

#endif
