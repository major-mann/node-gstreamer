#include <gst/gst.h>
#include "element.h"
#include "../helper.h"
#include "pad.h"

using namespace nstr;
using namespace v8;

void Element::Init(Handle<Object> exports) {
    LinkedObjectBase<Element, GstElement>::Init(exports, "Element");
}

Element::Element(GstElement* element) : ElementBase<Element, GstElement>(element) { }

Element* Element::Create(GstElement* ele) {
    Element* result = LinkedObjectBase<Element, GstElement>::Create(ele);
    return result;
}

Element* Element::Create(const char* type, const char* name) {
    GstElement* ele = gst_element_factory_make(type, name);
    if (ele == NULL) {
        return NULL;
    } else {
        Element* result = Element::Create(ele);
        return result;
    }
}

//Retain
Element* Element::Create(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() < 1 || !args[0]->IsString()) {
        Helper::ThrowV8Error("Element type MUST be supplied!");
        return NULL;
    }

    //Get the type name and name so we can create the element
    Local<String> v8typeName = args[0]->ToString();
    String::Utf8Value typeName(v8typeName);
    GstElement* ele;

    //Try to create the element
    if (args.Length() > 1 && args[1]->IsString()) {
        String::Utf8Value name(args[1]->ToString());
        ele = gst_element_factory_make(*typeName, *name);
    } else {
        ele = gst_element_factory_make(*typeName, NULL);
    }

    //Ensure we were able to create the element
    if (ele == NULL) {
        Helper::ThrowV8Error("Cannot find the element with the specified type name");
        return NULL;
    }

    Element* result = Element::Create(ele);
    return result;
}

Element* Element::Unwrap(Handle<Value> value) {
    if (value->IsObject()) {
        return node::ObjectWrap::Unwrap<Element>(value->ToObject());
    } else {
        return NULL;
    }
}