/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2014  LiteSpeed Technologies, Inc.                        *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#include "ls_rewrite_options.h"
#include "ls_rewrite_driver_factory.h"
#include "net/instaweb/rewriter/public/file_load_policy.h"
#include "net/instaweb/rewriter/public/rewrite_options.h"
#include "net/instaweb/system/public/system_caches.h"
#include "net/instaweb/util/public/timer.h"

namespace net_instaweb
{
    const char kStatisticsPath[] = "StatisticsPath";
    const char kGlobalStatisticsPath[] = "GlobalStatisticsPath";
    const char kConsolePath[] = "ConsolePath";
    const char kMessagesPath[] = "MessagesPath";
    const char kAdminPath[] = "AdminPath";
    const char kGlobalAdminPath[] = "GlobalAdminPath";

    const char* const server_only_options[] =
    {
        "FetcherTimeoutMs",
        "FetchProxy",
        "ForceCaching",
        "GeneratedFilePrefix",
        "ImgMaxRewritesAtOnce",
        "InheritVHostConfig",
        "InstallCrashHandler",
        "MessageBufferSize",
        "NumRewriteThreads",
        "NumExpensiveRewriteThreads",
        "StaticAssetPrefix",
        "TrackOriginalContentLength",
        "UsePerVHostStatistics",  // TODO(anupama): What to do about "No longer used"
        "BlockingRewriteRefererUrls",
        "CreateSharedMemoryMetadataCache",
        "LoadFromFile",
        "LoadFromFileMatch",
        "LoadFromFileRule",
        "LoadFromFileRuleMatch"
    };

// Options that can only be used in the main (http) option scope.
    const char* const main_only_options[] =
    {
    };



    RewriteOptions::Properties* LsiRewriteOptions::lsi_properties_ = NULL;

    LsiRewriteOptions::LsiRewriteOptions( const StringPiece& description,
                                          ThreadSystem* thread_system )
        : SystemRewriteOptions( description, thread_system )
    {
        Init();
    }

    LsiRewriteOptions::LsiRewriteOptions( ThreadSystem* thread_system )
        : SystemRewriteOptions( thread_system )
    {
        Init();
    }

    void LsiRewriteOptions::Init()
    {
        DCHECK( lsi_properties_ != NULL )
                << "Call LsiRewriteOptions::Initialize() before construction";
        InitializeOptions( lsi_properties_ );
    }

    void LsiRewriteOptions::AddProperties()
    {
        // ls-specific options.
        add_lsi_option(
            "", &LsiRewriteOptions::statistics_path_, "nsp", kStatisticsPath,
            kServerScope, "Set the statistics path. Ex: /lsi_pagespeed_statistics" );
        add_lsi_option(
            "", &LsiRewriteOptions::global_statistics_path_, "ngsp",
            kGlobalStatisticsPath, kProcessScope,
            "Set the global statistics path. Ex: /lsi_pagespeed_global_statistics" );
        add_lsi_option(
            "", &LsiRewriteOptions::console_path_, "ncp", kConsolePath, kServerScope,
            "Set the console path. Ex: /pagespeed_console" );
        add_lsi_option(
            "", &LsiRewriteOptions::messages_path_, "nmp", kMessagesPath,
            kServerScope, "Set the messages path.  Ex: /lsi_pagespeed_message" );
        add_lsi_option(
            "", &LsiRewriteOptions::admin_path_, "nap", kAdminPath,
            kServerScope, "Set the admin path.  Ex: /pagespeed_admin" );
        add_lsi_option(
            "", &LsiRewriteOptions::global_admin_path_, "ngap", kGlobalAdminPath,
            kProcessScope, "Set the global admin path.  Ex: /pagespeed_global_admin" );

        MergeSubclassProperties( lsi_properties_ );

        // Default properties are global but to set them the current API requires
        // a RewriteOptions instance and we're in a static method.
        LsiRewriteOptions dummy_config( NULL );
        dummy_config.set_default_x_header_value( kModPagespeedVersion );
    }

    void LsiRewriteOptions::Initialize()
    {
        if( Properties::Initialize( &lsi_properties_ ) )
        {
            SystemRewriteOptions::Initialize();
            AddProperties();
        }
    }

    void LsiRewriteOptions::Terminate()
    {
        if( Properties::Terminate( &lsi_properties_ ) )
        {
            SystemRewriteOptions::Terminate();
        }
    }

    bool LsiRewriteOptions::IsDirective( StringPiece config_directive,
                                         StringPiece compare_directive )
    {
        return StringCaseEqual( config_directive, compare_directive );
    }

    RewriteOptions::OptionScope LsiRewriteOptions::GetOptionScope(
        StringPiece option_name )
    {
        unsigned int i;
        unsigned int size = sizeof( main_only_options ) / sizeof( char* );

        for( i = 0; i < size; i++ )
        {
            if( StringCaseEqual( main_only_options[i], option_name ) )
            {
                return kProcessScopeStrict;
            }
        }

        size = sizeof( server_only_options ) / sizeof( char* );

        for( i = 0; i < size; i++ )
        {
            if( StringCaseEqual( server_only_options[i], option_name ) )
            {
                return kServerScope;
            }
        }

        // This could be made more efficient if RewriteOptions provided a map allowing
        // access of options by their name. It's not too much of a worry at present
        // since this is just during initialization.
        for( OptionBaseVector::const_iterator it = all_options().begin();
                it != all_options().end(); ++it )
        {
            RewriteOptions::OptionBase* option = *it;

            if( option->option_name() == option_name )
            {
                // We treat kProcessScope as kProcessScopeStrict, failing to start if an
                // option is out of place.
                return option->scope() == kProcessScope ? kProcessScopeStrict
                       : option->scope();
            }
        }

        return kDirectoryScope;
    }

    RewriteOptions::OptionSettingResult LsiRewriteOptions::ParseAndSetOptions0(
        StringPiece directive, GoogleString* msg, MessageHandler* handler )
    {
        if( IsDirective( directive, "on" ) )
        {
            set_enabled( RewriteOptions::kEnabledOn );
        }
        else if( IsDirective( directive, "off" ) )
        {
            set_enabled( RewriteOptions::kEnabledOff );
        }
        else if( IsDirective( directive, "unplugged" ) )
        {
            set_enabled( RewriteOptions::kEnabledUnplugged );
        }
        else
        {
            return RewriteOptions::kOptionNameUnknown;
        }

        return RewriteOptions::kOptionOk;
    }

    RewriteOptions::OptionSettingResult
    LsiRewriteOptions::ParseAndSetOptionFromName1(
        StringPiece name, StringPiece arg,
        GoogleString* msg, MessageHandler* handler )
    {
        // FileCachePath needs error checking.
        if( StringCaseEqual( name, kFileCachePath ) )
        {
            if( !StringCaseStartsWith( arg, "/" ) )
            {
                *msg = "must start with a slash";
                return RewriteOptions::kOptionValueInvalid;
            }
        }

        return SystemRewriteOptions::ParseAndSetOptionFromName1(
                   name, arg, msg, handler );
    }

    template <class DriverFactoryT>
    RewriteOptions::OptionSettingResult ParseAndSetOptionHelper(
        StringPiece option_value,
        DriverFactoryT* driver_factory,
        void ( DriverFactoryT::*set_option_method )( bool ) )
    {
        bool parsed_value;

        if( StringCaseEqual( option_value, "on" ) ||
                StringCaseEqual( option_value, "true" ) )
        {
            parsed_value = true;
        }
        else if( StringCaseEqual( option_value, "off" ) ||
                 StringCaseEqual( option_value, "false" ) )
        {
            parsed_value = false;
        }
        else
        {
            return RewriteOptions::kOptionValueInvalid;
        }

        ( driver_factory->*set_option_method )( parsed_value );
        return RewriteOptions::kOptionOk;
    }

    const char* ps_error_string_for_option( StringPiece directive, StringPiece warning )
    {
        GoogleString msg =
            StrCat( "\"", directive, "\" ", warning );
        g_api->log( NULL, LSI_LOG_WARN, "[%s] %s\n", kModuleName, msg.c_str() );
        return msg.c_str();
    }

// Very similar to apache/mod_instaweb::ParseDirective.
    const char* LsiRewriteOptions::ParseAndSetOptions(
        StringPiece* args, int n_args, MessageHandler* handler,
        LsiRewriteDriverFactory* driver_factory,
        RewriteOptions::OptionScope scope )
    {
        CHECK_GE( n_args, 1 );

        StringPiece directive = args[0];

        // Remove initial "ModPagespeed" if there is one.
        StringPiece mod_pagespeed( "ModPagespeed" );

        if( StringCaseStartsWith( directive, mod_pagespeed ) )
        {
            directive.remove_prefix( mod_pagespeed.size() );
        }

        if( GetOptionScope( directive ) > scope )
        {
            return ps_error_string_for_option(
                       directive, "cannot be set at this scope." );
        }

        GoogleString msg;
        OptionSettingResult result;

        if( n_args == 1 )
        {
            result = ParseAndSetOptions0( directive, &msg, handler );
        }
        else if( n_args == 2 )
        {
            StringPiece arg = args[1];

            // TODO(morlovich): Remove these special hacks, and handle these via
            // ParseAndSetOptionFromEnum1.
            if( IsDirective( directive, "UsePerVHostStatistics" ) )
            {
                result = ParseAndSetOptionHelper<LsiRewriteDriverFactory> (
                             arg, driver_factory,
                             &LsiRewriteDriverFactory::set_use_per_vhost_statistics );
            }
            else if( IsDirective( directive, "InstallCrashHandler" ) )
            {
                result = ParseAndSetOptionHelper<LsiRewriteDriverFactory> (
                             arg, driver_factory,
                             &LsiRewriteDriverFactory::set_install_crash_handler );
            }
            else if( IsDirective( directive, "MessageBufferSize" ) )
            {
                int message_buffer_size;
                bool ok = StringToInt( arg.as_string(), &message_buffer_size );

                if( ok && message_buffer_size >= 0 )
                {
                    driver_factory->set_message_buffer_size( message_buffer_size );
                    result = RewriteOptions::kOptionOk;
                }
                else
                {
                    result = RewriteOptions::kOptionValueInvalid;
                }
            }
            else if( IsDirective( directive, "RateLimitBackgroundFetches" ) )
            {
                result = ParseAndSetOptionHelper<LsiRewriteDriverFactory> (
                             arg, driver_factory,
                             &LsiRewriteDriverFactory::set_rate_limit_background_fetches );
            }
            else if( IsDirective( directive, "ForceCaching" ) )
            {
                result = ParseAndSetOptionHelper<SystemRewriteDriverFactory> (
                             arg, driver_factory,
                             &SystemRewriteDriverFactory::set_force_caching );
            }
            else if( IsDirective( directive, "ListOutstandingUrlsOnError" ) )
            {
                result = ParseAndSetOptionHelper<SystemRewriteDriverFactory> (
                             arg, driver_factory,
                             &SystemRewriteDriverFactory::list_outstanding_urls_on_error );
            }
            else if( IsDirective( directive, "TrackOriginalContentLength" ) )
            {
                result = ParseAndSetOptionHelper<SystemRewriteDriverFactory> (
                             arg, driver_factory,
                             &SystemRewriteDriverFactory::set_track_original_content_length );
            }
            else if( IsDirective( directive, "StaticAssetPrefix" ) )
            {
                driver_factory->set_static_asset_prefix( arg );
                result = RewriteOptions::kOptionOk;
            }
            
            //FIXME: I disabled inplace right now for we just do not support it
            else if( IsDirective( directive, "InPlaceResourceOptimization" ) )
            {
                ps_error_string_for_option(directive, "Not support right now.");
                result = RewriteOptions::kOptionOk;
            }
            else
            {
                result = ParseAndSetOptionFromName1( directive, arg, &msg, handler );
            }
        }
        else if( n_args == 3 )
        {
            // Short-term special handling, until this moves to common code.
            // TODO(morlovich): Clean this up.
            if( StringCaseEqual( directive, "CreateSharedMemoryMetadataCache" ) )
            {
                int64 kb = 0;

                if( !StringToInt64( args[2], &kb ) || kb < 0 )
                {
                    result = RewriteOptions::kOptionValueInvalid;
                    msg = "size_kb must be a positive 64-bit integer";
                }
                else
                {
                    bool ok = driver_factory->caches()->CreateShmMetadataCache(
                                  args[1].as_string(), kb, &msg );
                    result = ok ? kOptionOk : kOptionValueInvalid;
                }
            }
            else
            {
                result = ParseAndSetOptionFromName2( directive, args[1], args[2],
                                                     &msg, handler );
            }
        }
        else if( n_args == 4 )
        {
            result = ParseAndSetOptionFromName3(
                         directive, args[1], args[2], args[3], &msg, handler );
        }
        else
        {
            return ps_error_string_for_option(
                       directive, "not recognized or too many arguments" );
        }

        switch( result )
        {
        case RewriteOptions::kOptionOk:
            return 0;

        case RewriteOptions::kOptionNameUnknown:
            return ps_error_string_for_option(
                       directive, "not recognized or too many arguments" );

        case RewriteOptions::kOptionValueInvalid:
        {
            GoogleString full_directive;

            for( int i = 0 ; i < n_args ; i++ )
            {
                StrAppend( &full_directive, i == 0 ? "" : " ", args[i] );
            }

            return ps_error_string_for_option( full_directive, msg );
        }
        }

        CHECK( false );
        return NULL;
    }

    LsiRewriteOptions* LsiRewriteOptions::Clone() const
    {
        LsiRewriteOptions* options = new LsiRewriteOptions(
            StrCat( "cloned from ", description() ), thread_system() );
        options->Merge( *this );
        return options;
    }

    const LsiRewriteOptions* LsiRewriteOptions::DynamicCast(
        const RewriteOptions* instance )
    {
        return dynamic_cast<const LsiRewriteOptions*>( instance );
    }

    LsiRewriteOptions* LsiRewriteOptions::DynamicCast( RewriteOptions* instance )
    {
        return dynamic_cast<LsiRewriteOptions*>( instance );
    }

}  // namespace net_instaweb

