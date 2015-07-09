#ifndef NSTR_CAPS_H
#define NSTR_CAPS_H

#include <node.h>
#include <gst/gst.h>
#include "linked_object_base.h"

using namespace v8;

namespace nstr {

	class Caps : public LinkedObjectBase<Caps, GstCaps> {
		friend class LinkedObjectBase<Caps, GstCaps>;
		public:
			static Caps* Create(GstCaps* caps);
			static Caps* Create(const GstCaps* caps);

			/** Returns a string array of feature names. */
			Handle<Array> Structures();

			/* Checks whether the supplied caps instance is equal to the supplied caps object **/
			bool Equals(Caps* caps);

			/** Unwraps a caps object from the supplied v8 object */
			static Caps* Unwrap(Handle<Value> value);

		private:
			Caps(GstCaps* caps);
			Caps(const GstCaps* caps);

			void InitializeV8Instance(Handle<Object> instance);

			/** The v8 wrapper for the structure call */
			static void Structures(const v8::FunctionCallbackInfo<v8::Value>& args);

			/** The v8 wrapper for the equals call */
			static void Equals(const v8::FunctionCallbackInfo<v8::Value>& args);

			/** Unrefs the specified caps object */
			static void RefWrap(GstCaps* caps);
	};

}

#endif