#ifndef NSTR_PT
#define NSTR_PT

#include <node.h>
#include "linked_object_base.h"
#include "caps.h"

using namespace v8;

namespace nstr {
	class  PadTemplate : public LinkedObjectBase<PadTemplate, GstPadTemplate> {
		friend class LinkedObjectBase<PadTemplate, GstPadTemplate>;
		public:
			static PadTemplate* Create(GstPadTemplate* pt);
			
			static PadTemplate* Create(const FunctionCallbackInfo<Value>& args);

			/** Returns the template caps */
			Caps* GetCaps();

			/** Returns the name template */
			const char* NameTemplate();

			/** Gets the pad direction */
			const char* Direction();

			/** Gets the pad presence */
			const char* Presence();
		private:
			PadTemplate(GstPadTemplate* padTemplate);

			/** Ties up the required functions to the instance */
			void InitializeV8Instance(Handle<Object> instance);

			/** Returns an instance reference from the supplied value */
			static PadTemplate* Instance(Local<Value> value);

			/** v8 wrapper for NameTemplate */
			static void NameTemplate(const FunctionCallbackInfo<Value>& args);

			/** v8 wrapper for GetCaps */
			static void GetCaps(const FunctionCallbackInfo<Value>& args);

			/** The v8 Direction wrapper */
			static void Direction(const FunctionCallbackInfo<Value>& args);

			/** The v8 Presence wrapper */
			static void Presence(const FunctionCallbackInfo<Value>& args);	
	};
}

#endif
