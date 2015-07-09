#ifndef NSTR_PLUGIN_FEATURE_BASE_H
#define NSTR_PLUGIN_FEATURE_BASE_H

#include <node.h>
#include "linked_object_base.h"
#include "../helper.h"
#include "plugin.h"
#include <gst/gst.h>
#include <typeinfo>

using namespace v8;

namespace nstr {

	template <class T, class CT>
	class PluginFeatureBase : public LinkedObjectBase<T, CT> {
		public:
			static T* Create(CT* pf) {
				//Call the underlying init
				return LinkedObjectBase<T, CT>::Create(pf);
			}

			/** Wraps gst_plugin_feature_get_rank method */
			uint Rank() {
				CT* obj = this->GetObject();
				GstPluginFeature* pfobj = (GstPluginFeature*)obj;
				guint rank = gst_plugin_feature_get_rank(pfobj);
				return rank;
			}

			/** Wraps gst_plugin_feature_get_name */
			const char* Name() {
				return gst_plugin_feature_get_name((GstPluginFeature*)this->GetObject());
			}

			/** Wraps gst_plugin_feature_get_plugin */
			Plugin* GetPlugin() {
				GstPluginFeature* pf = (GstPluginFeature*)this->GetObject();
				GstPlugin* gplugin = gst_plugin_feature_get_plugin(pf);
				Plugin* result = Plugin::Create(gplugin);
				return result;
			}

			/** Wraps gst_plugin_feature_load */
			void Load() {
				CT* feat = (CT*)gst_plugin_feature_load((GstPluginFeature*)this->GetObject());
				//Replace with the returned feature
				LinkedObjectBase<T, CT>::SetObject(feat);
			}

			/** gst_plugin_feature_check_version */
			bool CheckVersion(uint minMajor, uint minMinor, uint minMicro) {
				gboolean result = gst_plugin_feature_check_version((GstPluginFeature*)this->GetObject(), minMajor, minMinor, minMicro);
				return result;
			}

		protected:
			PluginFeatureBase(CT* pluginFeature) 
				: LinkedObjectBase<T, CT>(pluginFeature, Helper::GstRefWrap, Helper::GstUnrefWrap) { }

			/** Used to add the required methods to the instance */
	        void InitializeV8Instance(Handle<Object> instance) {
	        	Local<External> data = External::New(Isolate::GetCurrent(), this);

				Helper::SetAccessorProp(instance, "rank", RankGetAccessor, data);
				Helper::SetAccessorProp(instance, "name", NameGetAccessor, data);

				Helper::SetFunctionProp(instance, "plugin", GetPlugin, data);
				Helper::SetFunctionProp(instance, "load", Load, data);
				Helper::SetFunctionProp(instance, "checkVersion", CheckVersion, data);
			}
		private:
			static T* Instance(Local<Value> value) {
				Local<External> data = Local<External>::Cast(value);
				return (T*)data->Value();
			}
			
			static void RankGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info){
				T* instance = Instance(info.Data());
				info.GetReturnValue().Set(Helper::ToValue((double)instance->Rank()));
			}
			static void NameGetAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info) {
				T* instance = Instance(info.Data());
				info.GetReturnValue().Set(Helper::ToValue(instance->Name()));
			}

			static void GetPlugin(const FunctionCallbackInfo<Value>& info){
				T* instance = Instance(info.Data());
				Plugin* plugin = instance->GetPlugin();
				info.GetReturnValue().Set(plugin->GetV8Instance());
			}
			static void Load(const FunctionCallbackInfo<Value>& info) {
				T* instance = Instance(info.Data());
				instance->Load();
			}
			static void CheckVersion(const FunctionCallbackInfo<Value>& args) {
				uint major = 0;
				uint minor = 0;
				uint micro = 0;

				if (args.Length() > 0) {
					major = Helper::AsUInt(args[0]);
				}
				if (args.Length() > 1) {
					minor = Helper::AsUInt(args[1]);
				}
				if (args.Length() > 2) {
					micro = Helper::AsUInt(args[2]);
				}
				T* instance = Instance(args.Data());
				Local<Boolean> result = Boolean::New(Isolate::GetCurrent(), instance->CheckVersion(major, minor, micro));
				args.GetReturnValue().Set(result);
			}
	};
}

#endif