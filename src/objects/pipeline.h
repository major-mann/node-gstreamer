#ifndef NSTR_PIPELINE_H
#define NSTR_PIPELINE_H

#include "bin.h"
#include "bus.h"

namespace nstr {

	class Pipeline : public Bin<Pipeline, GstElement> {
		friend class LinkedObjectBase<Pipeline, GstElement>;
		public:
			static void Init(Handle<Object> instance);
			static Pipeline* Create(GstElement* p);
			static Pipeline* Create(const char* name);
			static Pipeline* Create(const FunctionCallbackInfo<Value>& args);
			static Pipeline* Unwrap(Handle<Value> value);

			unsigned long GetDelay();
			void SetDelay(unsigned long);

			Bus* GetBus();

		protected:
			/**
            * Adds the pipeline members to the v8 instance
            */
            void InitializeV8Instance(Handle<Object> instance);
		private:
			Pipeline(GstElement* p);

			GstPipeline* GetPipeline();

			static Pipeline* Instance(Local<Value> data);
			static void DelayGetAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void DelaySetAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);
			static void GetBus(const FunctionCallbackInfo<Value>& args);
	};

}

#endif