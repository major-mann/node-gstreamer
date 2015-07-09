#ifndef NSTR_PLUGIN_H
#define NSTR_PLUGIN_H

#include <node.h>
#include <map>
#include <gst/gst.h>
#include <string>
#include "linked_object_base.h"
#include "structure.h"

using namespace v8;

namespace nstr {
    /**
    * Represents a GstPluginFeature in javascrpt land.
    * The factor method "New" is responsible for maintaining a singleton state.
    */
    class Plugin : public LinkedObjectBase<Plugin, GstPlugin> {
        friend class LinkedObjectBase<Plugin, GstPlugin>;
        public:
            static Plugin* Create(const char* name);
            static Plugin* Create(GstPlugin* plugin);

            const char* GetName();
            const char* Description();
            const char* License();
            const char* Package();
            const char* Origin();
            const char* Source();
            const char* Version();

            bool IsLoaded();
            void Load();
            Handle<Array> Features(Handle<Function> filter);
        protected:
            /** Used to add the required methods to the instance */
            void InitializeV8Instance(Handle<Object> instance);
        private:
            Plugin(GstPlugin* plugin);

            static Plugin* Instance(Local<Value> value);

            /**
            * A function used to filter the feature list according to the plugin name and 
            * optionally a consumer defined filter callback
            */
            static gboolean FeatureFilter(GstPluginFeature* feature, gpointer userData);

            static Handle<Value> FeatureToValue(void* feature);

            static void NameGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void DescriptionGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void LicenseGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void PackageGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void OriginGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void SourceGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void VersionGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);
            static void LoadedGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info);

            static void IsLoaded(const FunctionCallbackInfo<Value>& args);
            static void Load(const FunctionCallbackInfo<Value>& args);

            /**
            * The function that can be called from javascript to return a list of features
            * for the supplied plugin
            */
            static void Features(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}

#endif