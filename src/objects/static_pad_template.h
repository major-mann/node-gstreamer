#ifndef NSTR_SPTPL_H
#define NSTR_SPTPL_H

#include <node.h>
#include "caps.h"
#include "pad_template.h"
#include "linked_object_base.h"

using namespace v8;

namespace nstr {

	class StaticPadTemplate : public LinkedObjectBase<StaticPadTemplate, GstStaticPadTemplate> {
		friend class LinkedObjectBase<StaticPadTemplate, GstStaticPadTemplate>;
		public:
			static StaticPadTemplate* Create(GstStaticPadTemplate* spt);

			/** Returns the template caps */
			Caps* GetCaps();

			/** Gets the pad direction */
			const char* Direction();

			/** Gets the pad presence */
			const char* Presence();

			/** Returns the pad template specified by the static template */
			PadTemplate* GetPadTemplate();

		private:
			StaticPadTemplate(GstStaticPadTemplate* spt);

			/** Ties up the required functions to the instance */
			void InitializeV8Instance(Handle<Object> instance);

			/** Returns an instance reference from the supplied value */
			static StaticPadTemplate* Instance(Local<Value> value);

			/** v8 wrapper for GetCaps */
			static void GetCaps(const FunctionCallbackInfo<Value>& args);

			/** The v8 Direction wrapper */
			static void Direction(const FunctionCallbackInfo<Value>& args);

			/** The v8 Presence wrapper */
			static void Presence(const FunctionCallbackInfo<Value>& args);

			/** The v8 GetPadTemplate wrapper */
			static void GetPadTemplate(const FunctionCallbackInfo<Value>& args);
	};
}

#endif