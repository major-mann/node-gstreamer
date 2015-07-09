#include <node.h>
#include <gst/gst.h>
#include <string>
#include "helper.h"
#include "objects/registry.h"
#include "objects/element_factory.h"
#include "objects/element.h"
#include "objects/pipeline.h"
#include "objects/main_loop.h"

using namespace v8;

namespace nstr {

    const char* GSREAMER_INIT_ERROR = "Unable to initialize GStreamer.";
    const char* MAJOR_PROPERTY = "major";
    const char* MINOR_PROPERTY = "minor";
    const char* MICRO_PROPERTY = "micro";
    const char* NANO_PROPERTY = "nano";
    const char* FULL_PROPERTY = "full";

    //Creates a version object
    Local<Object> version() {
        guint major, minor, micro, nano;
        Isolate* isolate = Isolate::GetCurrent();

        //Get the version, and the version string
        gst_version(&major, &minor, &micro, &nano);
        gchar *full = gst_version_string();

        //Create an object
        Local<Object> result = Object::New(isolate);

        //Set properties
        Helper::SetUIntProp(result, MAJOR_PROPERTY, major);
        Helper::SetUIntProp(result, MINOR_PROPERTY, minor);
        Helper::SetUIntProp(result, MICRO_PROPERTY, micro);
        Helper::SetUIntProp(result, NANO_PROPERTY, nano);
        Helper::SetUtf8Prop(result, FULL_PROPERTY, full);

        return result;
    }

    //The global module entry point
    void init(Handle<Object> exports) {

        //TODO: See http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/ch04s02.html
        //  to enable silent mode, and other desired global options.

        //Good for debugging
        //g_log_set_always_fatal(G_LOG_LEVEL_WARNING);

        //Initialise gstreamer
        GError* gerr = NULL;
        gst_init_check (NULL, NULL, &gerr);


        if (gerr == NULL) {

            //Export globals
            Helper::SetProp(exports, "version", version());

            //Export singletons
            Registry* dreg = Registry::Default();
            Helper::SetProp(exports, "registry", dreg->GetV8Instance());

            MainLoop* mainLoop = MainLoop::Singleton;
            Helper::SetProp(exports, "mainLoop", mainLoop->GetV8Instance());

            //Export types
            Element::Init(exports);
            ElementFactory::Init(exports);
            Pipeline::Init(exports);

        } else {
            char* msg = new char[std::strlen(GSREAMER_INIT_ERROR) + std::strlen(gerr->message) + 1];
            sprintf(msg, "%s %s", GSREAMER_INIT_ERROR, gerr->message);
            Helper::ThrowV8Error(msg, gerr->code);
        }
    }

    //No semi colon on purpose...
    NODE_MODULE(addon, init)
}