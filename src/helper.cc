#include <node.h>
#include <gst/gst.h>
#include "helper.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <execinfo.h>

using namespace nstr;

//Define the constants
const char* CODE_PROPERTY = "code";

//Creates a new v8::Exception containing the supplied message
v8::Local<v8::Value> Helper::CreateV8Error(const char* message) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::String> msg = v8::String::NewFromUtf8(isolate, message);
    v8::Local<v8::Value> exception = v8::Exception::Error(msg);
    return exception;
}

v8::Handle<v8::Value> Helper::ToValue(const char* str) {
    return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str);
}

v8::Handle<v8::Value> Helper::ToValue(double num) {
    return v8::Number::New(v8::Isolate::GetCurrent(), num);
}

v8::Handle<v8::Value> Helper::ToBooleanValue(bool b) {
    return v8::Boolean::New(v8::Isolate::GetCurrent(), b);
}

v8::Handle<v8::Value> Helper::DateToValue(double ms) {
    return v8::Date::New(v8::Isolate::GetCurrent(), ms);
}

void Helper::SetProp(v8::Local<v8::Object> obj, const char* propName, v8::Local<v8::Value> value) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::String> prop = v8::String::NewFromUtf8(isolate, propName);
    obj->Set(prop, value);
}

void Helper::SetIntProp(v8::Local<v8::Object> obj, const char* propName, int value) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Integer> val = v8::Integer::New(isolate, value);
    SetProp(obj, propName, val);
}

void Helper::SetUIntProp(v8::Local<v8::Object> obj, const char* propName, uint value) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Integer> val = v8::Integer::NewFromUnsigned(isolate, value);
    SetProp(obj, propName, val);
}

void Helper::SetBoolProp(v8::Local<v8::Object> obj, const char* propName, bool value) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Boolean> val = v8::Boolean::New(isolate, value);
    SetProp(obj, propName, val);
}

void Helper::SetUtf8Prop(v8::Local<v8::Object> obj, const char* propName, const char* value) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::String> val = v8::String::NewFromUtf8(isolate, value);
    SetProp(obj, propName, val);
}

void Helper::SetFunctionProp(v8::Local<v8::Object> obj, const char* propName, v8::FunctionCallback call) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::FunctionTemplate> val = v8::FunctionTemplate::New(isolate, call);
    SetProp(obj, propName, val->GetFunction());
}

void Helper::SetFunctionProp(v8::Local<v8::Object> obj, const char* propName, v8::FunctionCallback call, v8::Handle<v8::Value> data) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::FunctionTemplate> val = v8::FunctionTemplate::New(isolate, call, data);
    SetProp(obj, propName, val->GetFunction());
}

void Helper::SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter) {
    v8::Local<v8::String> pn = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), propName);
    obj->SetAccessor(pn, getter);
}
void Helper::SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::AccessorSetterCallback setter) {
    v8::Local<v8::String> pn = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), propName);
    obj->SetAccessor(pn, getter, setter);
}
void Helper::SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::Handle<v8::Value> data) {
    v8::Local<v8::String> pn = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), propName);
    obj->SetAccessor(pn, getter, 0, data);
}
void Helper::SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::AccessorSetterCallback setter, v8::Handle<v8::Value> data) {
    v8::Local<v8::String> pn = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), propName);
    obj->SetAccessor(pn, getter, setter, data);
}

uint Helper::AsUInt(v8::Handle<v8::Value> value, uint def) {
    uint result = def;
    if (value->IsNumber()) {
        double d = value->NumberValue();
        result = static_cast<uint>(d + 0.5);
    }
    return result;
}

uint Helper::AsUInt(v8::Handle<v8::Value> value) {
    return AsUInt(value, 0);
}

void Helper::ThrowV8Error(const char* message) {
    ThrowV8Error(message, "", "");
}

void Helper::ThrowV8Error(const char* message, const char* arg1) {
    ThrowV8Error(message, arg1, "");
}

void Helper::ThrowV8Error(const char* message, const char* arg1, const char* arg2) {
    int len = strlen(message);
    len += strlen(arg1);
    len += strlen(arg2);
    char* msg = new char[len];
    sprintf(msg, message, arg1, arg2);
    v8::Local<v8::Value> ex = CreateV8Error(msg);
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    isolate->ThrowException(ex);
}

void Helper::ThrowV8Error(const char *message, int code) {
    //Create the error
    v8::Local<v8::Value> ex = CreateV8Error(message);
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    //Set the code
    v8::Local<v8::String> codePropName = v8::String::NewFromUtf8(isolate, CODE_PROPERTY);
    v8::Local<v8::Integer> icode = v8::Integer::New(isolate, code);
    v8::Local<v8::Object> eobj = ex->ToObject();
    eobj->Set(codePropName, icode);

    //throw
    isolate->ThrowException(ex);
}

bool Helper::RunFilterFunction(v8::Handle<v8::Function> func, v8::Handle<v8::Value> item) {
    v8::Handle<v8::Value> argv[1];
    argv[0] = item;
    v8::Local<v8::Value> res = func->Call(item, 1, argv);
    if (res->IsNull() || res->IsFalse() || res->IsUndefined()) {
        return false;
    } else {
        return true;
    }
}

v8::Handle<v8::Array> Helper::GListToArray(GList* list, ToV8Value convert) {
    const GList* l = list;
    v8::Local<v8::Array> result = GListToArray(l, convert);
    gst_plugin_list_free(list);
    return result;
}

v8::Handle<v8::Array> Helper::GListToArray(const GList* list, ToV8Value convert) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Array> result = v8::Array::New(isolate);
    int count = 0;
    while (list) {
        v8::Handle<v8::Value> val = convert(list->data);
        result->Set(count, val);
        list = list->next;
        count++;
    }
    return result;
}

void Helper::PrintStack() {
    PrintStack(100);
}

void Helper::PrintStack(int count)
{
    void *trace_elems[count];
    int trace_elem_count(backtrace( trace_elems, count ));
    char **stack_syms(backtrace_symbols( trace_elems, trace_elem_count ));
    for ( int i = 0 ; i < trace_elem_count ; ++i )
    {
        std::cout << stack_syms[i] << "\n";
    }
    free( stack_syms );

    exit(1);
}  

const char* Helper::StateToString(GstState state) {
    switch (state) {
        case GST_STATE_VOID_PENDING:
            return "void";
        case GST_STATE_NULL:
            return "null";
        case GST_STATE_READY:
            return "ready";
        case GST_STATE_PAUSED:
            return "paused";
        case GST_STATE_PLAYING:
            return "playing";
        default:
            printf("%s\n", "INVALID PROGRAM! UNEXPECTED RETURN!");
            exit(1);
            return NULL;
    }
}