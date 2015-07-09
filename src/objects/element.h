#ifndef NSTR_ELEMENT_H
#define NSTR_ELEMENT_H

#include <node.h>
#include <string>
#include "g_object_wrap.h"
#include "linked_object_base.h"
#include "element_base.h"

using namespace v8;

namespace nstr {

	class Element : public ElementBase<Element, GstElement> {
		friend class LinkedObjectBase<Element, GstElement>;
		public:
			static Element* Create(GstElement* ele);
			static Element* Create(const char* type, const char* name);
			static Element* Create(const FunctionCallbackInfo<Value>& args);
			static Element* Unwrap(Handle<Value> value);
			static void Init(Handle<Object> exports);
		protected:
			Element(GstElement* element);
	};
}

#endif