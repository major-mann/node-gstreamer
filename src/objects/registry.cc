#include "registry.h"
#include "../helper.h"

using namespace nstr;

Registry::Registry(GstRegistry* obj)
    : LinkedObjectBase<Registry, GstRegistry>(obj, Helper::GstRefWrap, Helper::GstUnrefWrap) {
}

Registry* Registry::Default() {
    GstRegistry* reg = gst_registry_get();
    return Create(reg);
}

Registry* Registry::Create(GstRegistry* reg) {
    return LinkedObjectBase<Registry, GstRegistry>::Create(reg);
}

void Registry::InitializeV8Instance(Handle<Object> instance) {
    Local<External> data = External::New(Isolate::GetCurrent(), this);
    Helper::SetFunctionProp(instance, "plugin", &FindPlugin, data);
    Helper::SetFunctionProp(instance, "plugins", &Plugins, data);
    Helper::SetFunctionProp(instance, "featureCookie", &FeatureCookie, data);
    Helper::SetFunctionProp(instance, "feature", &FindFeature, data);
    Helper::SetFunctionProp(instance, "features", &Features, data);
}

Plugin* Registry::FindPlugin(const char* name) {
    GstPlugin* plugin = gst_registry_find_plugin(GetObject(), name);
    if (plugin == NULL) {
        return NULL;
    } else {
        Plugin* result = Plugin::Create(plugin);
        return result;
    }
}

gboolean Registry::PluginsFilter(GstPlugin* gplugin, gpointer userData) {
    Local<Function> filter = *((Local<Function>*)userData);
    Plugin* plugin = Plugin::Create(gplugin);
    return Helper::RunFilterFunction(filter, plugin->GetV8Instance());
}

Handle<Value> Registry::PluginConvert(void* data) {
    GstPlugin* gplugin = (GstPlugin*)data;
    Plugin* plugin = Plugin::Create(gplugin);
    return plugin->GetV8Instance();
}

Handle<Array> Registry::Plugins(Handle<Function> filter) {
    GList* plugins;
    if (filter.IsEmpty()) {
        //Run list
        plugins = gst_registry_get_plugin_list(GetObject());
    } else {
        //Run filter
        plugins = gst_registry_plugin_filter(GetObject(), PluginsFilter, false, &filter);
    }
    Handle<Array> result = Helper::GListToArray(plugins, &PluginConvert);
    return result;
}

uint Registry::FeatureCookie() {
    return gst_registry_get_feature_list_cookie(GetObject());
}

gboolean Registry::FeatureFilter(GstPluginFeature* gfeature, gpointer userData) {
    Local<Function> filter = *((Local<Function>*)userData);
    if (filter.IsEmpty()) {
        return true;
    } else {
        PluginFeature* plugin = PluginFeature::Create(gfeature);
        return Helper::RunFilterFunction(filter, plugin->GetV8Instance());
    }
}

Handle<Value> Registry::FeatureConvert(void* data) {
    GstPluginFeature* gfeature = (GstPluginFeature*)data;
    PluginFeature* plugin = PluginFeature::Create(gfeature);
    return plugin->GetV8Instance();
}

PluginFeature* Registry::FindFeature(const char* name) {
    GstPluginFeature* feature = gst_registry_lookup_feature(GetObject(), name);
    if (feature == NULL) {
        return NULL;
    } else {
        PluginFeature* result = PluginFeature::Create(feature);
        return result;
    }
}

Handle<Array> Registry::Features(Handle<Function> filter) {
    GList* features;
    features = gst_registry_feature_filter(GetObject(), &FeatureFilter, false, &filter);
    Handle<Array> result = Helper::GListToArray(features, &FeatureConvert);
    return result;
}

Registry* Registry::RegistryInstance(Handle<Value> value) {
    Handle<External> ext = Handle<External>::Cast(value);
    void* ptr = ext->Value();
    return (Registry*)ptr;
}

void Registry::FindPlugin(const v8::FunctionCallbackInfo<v8::Value>& args) {
    HandleScope scope(Isolate::GetCurrent());
    Handle<Value> result;
    if (args.Length() > 0 && args[0]->IsString()) {
        Registry* self = RegistryInstance(args.Data());
        String::Utf8Value pname(args[0]->ToString());
        Plugin* plugin = self->FindPlugin(*pname);
        result = plugin->GetV8Instance();
    } else {
        result = v8::Null(Isolate::GetCurrent());
    }
    args.GetReturnValue().Set(result);
}
void Registry::Plugins(const v8::FunctionCallbackInfo<v8::Value>& args) {
    HandleScope scope(Isolate::GetCurrent());

    Local<Function> filter;
    if (args.Length() > 0 && args[0]->IsFunction()) {
        filter = Local<Function>::Cast(args[0]);
    }
    Registry* self = RegistryInstance(args.Data());
    Handle<Array> plugins = self->Plugins(filter);
    args.GetReturnValue().Set(plugins);
}
void Registry::FeatureCookie(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Registry* self = RegistryInstance(args.Data());
    Local<Number> result = Number::New(isolate, self->FeatureCookie());
    args.GetReturnValue().Set(result);
}

void Registry::FindFeature(const v8::FunctionCallbackInfo<v8::Value>& args) {
    HandleScope scope(Isolate::GetCurrent());
    Handle<Value> result;
    if (args.Length() > 0 && args[0]->IsString()) {
        Registry* self = RegistryInstance(args.Data());
        String::Utf8Value fname(args[0]->ToString());
        PluginFeature* feature = self->FindFeature(*fname);
        result = feature->GetV8Instance();
    } else {
        result = v8::Null(Isolate::GetCurrent());
    }
    args.GetReturnValue().Set(result);
}

void Registry::Features(const v8::FunctionCallbackInfo<v8::Value>& args) {
    HandleScope scope(Isolate::GetCurrent());

    Local<Function> filter;
    if (args.Length() > 0 && args[0]->IsFunction()) {
        filter = Local<Function>::Cast(args[0]);
    }
    Registry* self = RegistryInstance(args.Data());
    Handle<Array> features = self->Features(filter);
    args.GetReturnValue().Set(features);
}