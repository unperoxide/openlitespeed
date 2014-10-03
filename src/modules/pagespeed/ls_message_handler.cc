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
#include "pagespeed.h"
#include "ls_message_handler.h"

#include <signal.h>

#include "net/instaweb/util/public/abstract_mutex.h"
#include "net/instaweb/util/public/debug.h"
#include "net/instaweb/util/public/shared_circular_buffer.h"
#include "net/instaweb/util/public/string_util.h"
#include "pagespeed/kernel/base/posix_timer.h"
#include "pagespeed/kernel/base/time_util.h"

extern "C" {
    static void signal_handler( int sig )
    {
        alarm( 2 );
        g_api->log( NULL, LSI_LOG_ERROR, "Trapped signal [%d]\n%s\n",
                    sig, net_instaweb::StackTraceString().c_str() );
        kill( getpid(), SIGKILL );
    }
}  // extern "C"

namespace net_instaweb
{

    LsiMessageHandler::LsiMessageHandler( AbstractMutex* mutex )
        : mutex_( mutex ),
          buffer_( NULL )
    {
        SetPidString( static_cast<int64>( getpid() ) );
    }

// Installs a signal handler for common crash signals, that tries to print
// out a backtrace.
    void LsiMessageHandler::InstallCrashHandler()
    {
        signal( SIGTRAP, signal_handler );  // On check failures
        signal( SIGABRT, signal_handler );
        signal( SIGFPE, signal_handler );
        signal( SIGSEGV, signal_handler );
    }

    bool LsiMessageHandler::Dump( Writer* writer )
    {
        // Can't dump before SharedCircularBuffer is set up.
        if( buffer_ == NULL )
        {
            return false;
        }

        return buffer_->Dump( writer, &handler_ );
    }

    lsi_log_level LsiMessageHandler::GetLsiLogLevel( MessageType type )
    {
        switch( type )
        {
        case kInfo:
            return LSI_LOG_INFO;

        case kWarning:
            return LSI_LOG_WARN;

        case kError:
        case kFatal:
            return LSI_LOG_ERROR;

        default
                :
            return LSI_LOG_DEBUG;
        }
    }

    void LsiMessageHandler::set_buffer( SharedCircularBuffer* buff )
    {
        ScopedMutex lock( mutex_.get() );
        buffer_ = buff;
    }

    void LsiMessageHandler::MessageVImpl( MessageType type, const char* msg,
                                          va_list args )
    {
        lsi_log_level log_level = GetLsiLogLevel( type );
        GoogleString formatted_message = Format( msg, args );
        g_api->log( NULL, log_level, "[%s @%ld] %s\n",
                    kModuleName, static_cast<long>( getpid() ),
                    formatted_message.c_str() );

        // Prepare a log message for the SharedCircularBuffer only.
        // Prepend time and severity to message.
        // Format is [time] [severity] [pid] message.
        GoogleString message;
        GoogleString time;
        PosixTimer timer;

        if( !ConvertTimeToString( timer.NowMs(), &time ) )
        {
            time = "?";
        }

        StrAppend( &message, "[", time, "] ",
                   "[", MessageTypeToString( type ), "] " );
        StrAppend( &message, pid_string_, " ", formatted_message, "\n" );
        {
            ScopedMutex lock( mutex_.get() );

            if( buffer_ != NULL )
            {
                buffer_->Write( message );
            }
        }
    }

    void LsiMessageHandler::FileMessageVImpl( MessageType type, const char* file,
            int line, const char* msg,
            va_list args )
    {
        lsi_log_level log_level = GetLsiLogLevel( type );
        GoogleString formatted_message = Format( msg, args );
        g_api->log( NULL, log_level, "[%s @%ld] %s\n",
                    kModuleName, static_cast<long>( getpid() ),
                    formatted_message.c_str() );
    }

}  // namespace net_instaweb

