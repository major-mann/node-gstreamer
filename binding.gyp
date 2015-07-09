{
    "targets": [
        {
            "target_name": "addon",
            "sources": [
                "src/helper.cc",
                "src/objects/g_object_wrap.cc",
                "src/objects/clock.cc",
                "src/objects/tag_list.cc",
                "src/objects/plugin.cc",
                "src/objects/registry.cc",
                "src/objects/structure.cc",
                "src/objects/caps.cc",
                "src/objects/pad_template.cc",
                "src/objects/static_pad_template.cc",
                "src/objects/pad.cc",
                "src/objects/plugin_feature.cc",
                "src/objects/element.cc",
                "src/objects/element_factory.cc",
                "src/objects/bus.cc",
                "src/objects/pipeline.cc",
                "src/objects/main_loop.cc",
                "src/node.streamer.cc"
            ],
            "include_dirs": [
                "/Library/Frameworks/Gstreamer.framework/Headers"
            ],
            "libraries": [
                "/Library/Frameworks/Gstreamer.framework/Libraries/libgobject-2.0.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libgstreamer-1.0.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libffi.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libglib-2.0.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libgmodule-2.0.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libiconv.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libintl.dylib",
                "/Library/Frameworks/Gstreamer.framework/Libraries/libxml2.dylib"                
            ],
            "conditions": [
                ['OS=="mac"',
                    {
                        'defines': [
                            '__MACOSX_CORE__'
                        ],
                        'link_settings': {
                            'libraries': [
                                '-framework', 'AppKit'
                            ]
                        },
                        'xcode_settings': {
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                        }
                    }
                    ],
            ]
        }
    ]
}