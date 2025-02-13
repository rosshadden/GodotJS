#include "jsb_settings.h"

#include "jsb_internal_pch.h"
#include "jsb_macros.h"
#include "jsb_logger.h"

#define JSB_SET_RESTART(val) (val)
#define JSB_SET_IGNORE_DOCS(val) (val)
#define JSB_SET_BASIC(val) (val)
#define JSB_SET_INTERNAL(val) (val)

namespace jsb::internal
{
#ifdef TOOLS_ENABLED
    static constexpr char kEdDebuggerPort[] =     JSB_MODULE_NAME_STRING "/debugger/editor_port";
    static constexpr char kEdIgnoredClasses[] =     JSB_MODULE_NAME_STRING "/codegen/ignored_classes";
#endif

    // use unnecessary first category layer (runtime and editor) to make the second layer shown as sections in project settings

    static constexpr char kRtDebuggerPort[] =     JSB_MODULE_NAME_STRING "/runtime/debugger/debugger_port";
    static constexpr char kRtSourceMapEnabled[] = JSB_MODULE_NAME_STRING "/runtime/logger/source_map_enabled";
    static constexpr char kRtAdditionalSearchPaths[] = JSB_MODULE_NAME_STRING "/runtime/core/additional_search_paths";
    static constexpr char kRtEntryScriptPath[] = JSB_MODULE_NAME_STRING "/runtime/core/entry_script_path";

    // editor specific settings, but we need it configured as project-wise instead of global-wise
    static constexpr char kRtPackagingWithSourceMap[] = JSB_MODULE_NAME_STRING "/editor/packaging/source_map_included";
    static constexpr char kRtPackagingIncludeFiles[] = JSB_MODULE_NAME_STRING "/editor/packaging/include_files";

    void init_settings()
    {
        static bool inited = false;
        if (!inited)
        {
            inited = true;
            static constexpr char filter[] = "*." JSB_JAVASCRIPT_EXT
#if JSB_USE_TYPESCRIPT
                                            ",*." JSB_TYPESCRIPT_EXT
#endif
            ;

#ifdef TOOLS_ENABLED
            if (!EditorSettings::get_singleton())
            {
                if (Engine::get_singleton()->is_editor_hint() || Engine::get_singleton()->is_project_manager_hint() || Main::is_cmdline_tool())
                {
                    EditorSettings::create();
                    jsb_check(EditorSettings::get_singleton());
                }
                else
                {
                    JSB_LOG(Verbose, "EditorSettings is not available when initialising %s", jsb_typename(jsb::internal::Settings));
                }
            }

            // check before read to avoid redundant warnings
            if (EditorSettings::get_singleton())
            {
                _EDITOR_DEF(kEdDebuggerPort, 9230, true);
                _EDITOR_DEF(kEdIgnoredClasses, PackedStringArray(), false);
            }
#endif
            _GLOBAL_DEF(kRtDebuggerPort, 9229, JSB_SET_RESTART(true), JSB_SET_IGNORE_DOCS(false), JSB_SET_BASIC(false), JSB_SET_INTERNAL(false));
            _GLOBAL_DEF(kRtSourceMapEnabled, true, JSB_SET_RESTART(false), JSB_SET_IGNORE_DOCS(false), JSB_SET_BASIC(true),  JSB_SET_INTERNAL(false));
            _GLOBAL_DEF(kRtAdditionalSearchPaths, PackedStringArray(), JSB_SET_RESTART(true),  JSB_SET_IGNORE_DOCS(false), JSB_SET_BASIC(true),  JSB_SET_INTERNAL(false));

            {
                PropertyInfo EntryScriptPath;
                EntryScriptPath.type = Variant::STRING;
                EntryScriptPath.name = kRtEntryScriptPath;
                EntryScriptPath.hint = PROPERTY_HINT_FILE;
                EntryScriptPath.hint_string = filter;
                _GLOBAL_DEF(EntryScriptPath, String(), JSB_SET_RESTART(false), JSB_SET_IGNORE_DOCS(false), JSB_SET_BASIC(true),  JSB_SET_INTERNAL(false));
            }

            _GLOBAL_DEF(kRtPackagingWithSourceMap, true, false);
            {
                PropertyInfo PackagingIncludeFiles;
                PackagingIncludeFiles.type = Variant::ARRAY;
                PackagingIncludeFiles.name = kRtPackagingIncludeFiles;
                PackagingIncludeFiles.hint = PROPERTY_HINT_ARRAY_TYPE;
                PackagingIncludeFiles.hint_string = vformat("%s/%s:%s", Variant::STRING, PROPERTY_HINT_FILE, filter);
                _GLOBAL_DEF(PackagingIncludeFiles, Array(), false, JSB_SET_IGNORE_DOCS(false), JSB_SET_BASIC(true),  JSB_SET_INTERNAL(false));
            }
        }
    }

#ifdef TOOLS_ENABLED
    PackedStringArray Settings::get_ignored_classes()
    {
        init_settings();
        return EDITOR_GET(kEdIgnoredClasses);
    }
#endif

    bool Settings::is_packaging_with_source_map()
    {
        init_settings();
        return GLOBAL_GET(kRtPackagingWithSourceMap);
    }

    PackedStringArray Settings::get_packaging_include_files()
    {
        init_settings();
        // rely on auto variant convert from Array
        return (PackedStringArray) GLOBAL_GET(kRtPackagingIncludeFiles);
    }

    uint16_t Settings::get_debugger_port()
    {
        init_settings();
#ifdef TOOLS_ENABLED
        if (Engine::get_singleton()->is_editor_hint())
        {
            return EDITOR_GET(kEdDebuggerPort);
        }
#endif
        return GLOBAL_GET(kRtDebuggerPort);
    }

    bool Settings::get_sourcemap_enabled()
    {
        init_settings();
        return GLOBAL_GET(kRtSourceMapEnabled);
    }

    String Settings::get_project_data_dir_name()
    {
        const String project_data_dir = ProjectSettings::get_singleton()->get_project_data_dir_name();
        jsb_check(!project_data_dir.is_empty());
        return project_data_dir;
    }

    String Settings::get_jsb_out_dir_name()
    {
        return get_project_data_dir_name().path_join(JSB_MODULE_NAME_STRING);
    }

    String Settings::get_tsbuildinfo_path()
    {
        return get_project_data_dir_name().path_join(".tsbuildinfo");
    }

    String Settings::get_jsb_out_res_path()
    {
        return "res://" + get_jsb_out_dir_name();
    }

    PackedStringArray Settings::get_additional_search_paths()
    {
        init_settings();
        return GLOBAL_GET(kRtAdditionalSearchPaths);
    }

    String Settings::get_entry_script_path()
    {
        init_settings();
        return GLOBAL_GET(kRtEntryScriptPath);
    }

    String Settings::get_indentation()
    {
#ifdef TOOLS_ENABLED
        if (Engine::get_singleton()->is_editor_hint())
        {
            init_settings();
            // use_space_indentation
            if (!!EDITOR_GET("text_editor/behavior/indent/type"))
            {
                const int indent_size = EDITOR_GET("text_editor/behavior/indent/size");
                return String(" ").repeat(indent_size);
            }
        }
#endif
        return "\t";
    }

}
