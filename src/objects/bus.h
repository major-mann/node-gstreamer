#ifndef NSTR_BUS_H
#define NSTR_BUS_H

#include <node.h>
#include "linked_object_base.h"

using namespace v8;

namespace nstr {

	class Bus : public LinkedObjectBase<Bus, GstBus> {
		friend class LinkedObjectBase<Bus, GstBus>;
		public:
			static Bus* Create(GstBus* bus);

			uint Watch();
			bool ClearWatch();
			int GetPriority();
			void SetPriority(int priority);

		protected:
			Bus(GstBus* bus);
			void InitializeV8Instance(Handle<Object> instance);
		private:
			struct WatchData {
				Bus* self;
				GstMessage* message;
			};

			int Priority;
			uint WatchId;
			
			static void WatchV8Callback(void* data);
			static gboolean BusCall(GstBus* bus, GstMessage* msg, gpointer data);

			static Bus* Instance(Local<Value> value);

			static void Watch(const FunctionCallbackInfo<Value>& args);
			static void ClearWatch(const FunctionCallbackInfo<Value>& args);
			static void GetPriorityAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void SetPriorityAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);
	};

}

#endif
