#include "plugin.h"
#include "../helper.h"
#include "plugin_feature.h"

using namespace v8;
using namespace nstr;

Plugin::Plugin(GstPlugin* plugin) : LinkedObjectBase<Plugin, GstPlugin>(plugin, Helper::GstRefWrap, Helper::GstUnrefWrap) { }

Plugin* Plugin::Create(const char* name) {
    GstPlugin* plugin = gst_plugin_load_by_name(name);
    if (plugin == NULL) {
        Helper::ThrowV8Error("Unable to find plugin with the specified name");
        return NULL;
    } else {
        //Call the underlying init
        return LinkedObjectBase<Plugin, GstPlugin>::Create(plugin);
    }
}
Plugin* Plugin::Create(GstPlugin* plugin) {
    return LinkedObjectBase<Plugin, GstPlugin>::Create(plugin);
}

const char* Plugin::GetName(){
    return gst_plugin_get_name(GetObject());
}
const char* Plugin::Description(){
    return gst_plugin_get_description(GetObject());
}
const char* Plugin::License(){
    return gst_plugin_get_license(GetObject());
}
const char* Plugin::Package(){
    return gst_plugin_get_package(GetObject());
}
const char* Plugin::Origin(){
    return gst_plugin_get_origin(GetObject());
}
const char* Plugin::Source(){
    return gst_plugin_get_source(GetObject());
}
const char* Plugin::Version(){
    return gst_plugin_get_version(GetObject());
}

bool Plugin::IsLoaded(){
    return gst_plugin_is_loaded(GetObject());
}
void Plugin::Load(){
    //TODO
    GstPlugin* plugin = GetObject();
    printf("%s\n", "TODO: This is causing a crash. Investigate... (plugin.cc Line ~52)");
    GstPlugin* newPlugin = gst_plugin_load(plugin);
    SetObject(newPlugin);
}

Handle<Value> Plugin::FeatureToValue(void* feature) {
    GstPluginFeature* gpf = (GstPluginFeature*)feature;
    PluginFeature* pf = PluginFeature::Create(gpf);
    Local<Value> instance = pf->GetV8Instance();
    if (instance.IsEmpty()) {
        printf("%s\n", "EMPTY INSTANCE!!!!");
    }
    return instance;
}

Handle<Array> Plugin::Features(Handle<Function> filter){
    //Set up the user data to pass to the filter function containing the plugin name
    //  and the user supplied filter function (if any)
    std::pair<const char*, Local<Function> > ffd;
    ffd.first = gst_plugin_get_name(GetObject());
    ffd.second = filter;

    //Get the fitered list of features, and add them to an array result.
    GstRegistry* registry = gst_registry_get();
    GList* features = gst_registry_feature_filter(registry, &FeatureFilter, false, &ffd);
    Local<Array> result = Helper::GListToArray(features, &FeatureToValue);

    //Return the result
    return result;
}

gboolean Plugin::FeatureFilter(GstPluginFeature* feature, gpointer userData) {
    std::pair<const char*, Local<Function> >* ffd = (std::pair<const char*, Local<Function> >*)userData;
    GstPlugin* plugin = gst_plugin_feature_get_plugin(feature);
    const char* fpname = gst_plugin_get_name(plugin);
    if (std::strcmp(ffd->first, fpname) == 0) {
        if (ffd->second.IsEmpty()) {
            return true;
        } else {
            v8::Local<v8::Object> fobj = PluginFeature::Create(feature)->GetV8Instance();
            return Helper::RunFilterFunction(ffd->second, fobj);
        }
    } else {
        return false;
    }
}

void Plugin::InitializeV8Instance(Handle<Object> instance) {
    Isolate* isolate = Isolate::GetCurrent();
    Local<External> data = External::New(isolate, this);

    //Accessors
    Helper::SetAccessorProp(instance, "name", NameGetAccessor, data);
    Helper::SetAccessorProp(instance, "description", DescriptionGetAccessor, data);
    Helper::SetAccessorProp(instance, "license", LicenseGetAccessor, data);
    Helper::SetAccessorProp(instance, "package", PackageGetAccessor, data);
    Helper::SetAccessorProp(instance, "origin", OriginGetAccessor, data);
    Helper::SetAccessorProp(instance, "source", SourceGetAccessor, data);
    Helper::SetAccessorProp(instance, "version", VersionGetAccessor, data);
    Helper::SetAccessorProp(instance, "loaded", LoadedGetAccessor, data);
    
    //Functions
    Helper::SetFunctionProp(instance, "load", Load, data);
    Helper::SetFunctionProp(instance, "features", &Features, data);
}

Plugin* Plugin::Instance(Local<Value> arg) {
    Local<External> data = Local<External>::Cast(arg);
    return (Plugin*)data->Value();
}

void Plugin::Features(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Plugin* instance = Instance(args.Data());

    Local<Function> filter;
    if (args.Length() > 0 && args[0]->IsFunction()) {
        filter = Local<Function>::Cast(args[0]);
    }
    Local<Array> arr = instance->Features(filter);
    args.GetReturnValue().Set(arr);
}

void Plugin::NameGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->GetName()));
}

void Plugin::DescriptionGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->Description()));
}

void Plugin::LicenseGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->License()));
}

void Plugin::PackageGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->Package()));
}

void Plugin::OriginGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->Origin()));
}

void Plugin::SourceGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->Source()));
}

void Plugin::VersionGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->Version()));
}

void Plugin::LoadedGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){ 
    Plugin* instance = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(instance->IsLoaded()));
}

void Plugin::Load(const FunctionCallbackInfo<Value>& args){ 
    Plugin* instance = Instance(args.Data());
    instance->Load();
}