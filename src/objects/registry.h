#ifndef NSTR_REGISTRY_H
#define NSTR_REGISTRY_H

#include <node.h>
#include <gst/gst.h>
#include <node_object_wrap.h>
#include "plugin.h"
#include "plugin_feature.h"
#include "linked_object_base.h"

using namespace v8;

namespace nstr {

    class Registry : public LinkedObjectBase<Registry, GstRegistry> {
        friend class LinkedObjectBase<Registry, GstRegistry>;
        public:
            static Registry* Create(GstRegistry* obj);

            /** returns the default registry */
            static Registry* Default();

            /** 
            * Applies the filter function to the features and returns an array 
            * containing the filtered items
            */
            Handle<Array> Features(Handle<Function> filter);

            /**
            * Searches the registry for the feature with the given name.
            */
            PluginFeature* FindFeature(const char* name);

            /**
            * Returns the value of the registry feature cookie which changes whenever
            *   the feature set has changed
            */
            uint FeatureCookie();

            /**
            * Searches the registry for the plugin with the given name.
            */
            Plugin* FindPlugin(const char* name);

            /** Applies the filter function to the values found in the registry */
            Handle<Array> Plugins(Handle<Function> filter);

        protected:
            /** Used to add the required methods to the instance */
            void InitializeV8Instance(Handle<Object> instance);

        private:
            /** Creates a new instance of the class */
            Registry(GstRegistry* obj);

            /** Runs the javascript filter function on the supplied plugin */
            static gboolean PluginsFilter(GstPlugin* gplugin, gpointer userData);

            /** Runs the supplied filter function on the feature */
            static gboolean FeatureFilter(GstPluginFeature* gfeature, gpointer userData);

            /** Converts the pointer (assumed to be GstPlugin) to a Plugin and then returns the v8 instance */
            static Handle<Value> PluginConvert(void* data);

            /** Converts a GstPluginFeature to a v8 object */
            static Handle<Value> FeatureConvert(void* data);

            /** Converts the value to external, and returns the casted pointer */
            static Registry* RegistryInstance(Handle<Value> value);

            /** The static v8 wrap for the FindPlugin instance method */
            static void FindPlugin(const v8::FunctionCallbackInfo<v8::Value>& args);
            /** The static v8 wrap for the FindPlugins instance method */
            static void Plugins(const v8::FunctionCallbackInfo<v8::Value>& args);

            /** The static v8 wrap for the FeatureCookie instance method */
            static void FeatureCookie(const v8::FunctionCallbackInfo<v8::Value>& args);
            /** The static v8 wrap for the FindFeature instance method */
            static void FindFeature(const v8::FunctionCallbackInfo<v8::Value>& args);
            /** The static v8 wrap for the Features instance method */
            static void Features(const v8::FunctionCallbackInfo<v8::Value>& args);
    };

}

#endif

/*
gst_registry_get

gst_registry_get_feature_list
gst_registry_get_feature_list_cookie
gst_registry_get_feature_list_by_plugin
gst_registry_feature_filter
gst_registry_lookup_feature
gst_registry_check_feature_version - Handled by feature.cc

gst_registry_get_plugin_list
gst_registry_plugin_filter
gst_registry_find_plugin


//No implement
gst_registry_add_plugin
gst_registry_remove_plugin
gst_registry_remove_feature
gst_registry_add_feature
gst_registry_lookup
gst_registry_scan_path

//Needs GType?
gst_registry_find_feature
*/