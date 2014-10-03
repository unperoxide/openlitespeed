/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013  LiteSpeed Technologies, Inc.                        *
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

/*
 * lsr_thrsafetest
 *
 * usage:
 *   lsr_thrsafetest  mode(`0'=pool,`1'=xpool) numThreads loopCount
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "timer.cpp"
#include <lsr/lsr_pool.h>
#include <lsr/lsr_xpool.h>

static int mode     = 0;    /* 0 = global pool mode, 1 = session pool */
int loopCount       = 10;
int numThreads      = 5;

static lsr_xpool_t pool;

typedef struct {
    pthread_t   m_thread;
    int         m_exit;
    int         m_id;
    int         m_error;
    int         m_waitCount;
} myThread_t;
static myThread_t threadMap[1000000];

static void showEnd( int numThreads )
{
    register int i;
    register myThread_t * p;
    p = threadMap;
    for ( i = 0; i < numThreads; i++, p++ )
    {
        printf( "DONE %d [m_error = %d waitCount=%d] %d\n",
                p->m_id,
                p->m_error, 
                p->m_waitCount,
                p->m_exit);
    }
}

static void * testOneAlloc( int id, int size )
{
    void * ptr;
    unsigned char * p;

    ptr = ( mode? lsr_xpool_alloc( &pool, size ): lsr_palloc( size ) );
    if ( ptr != NULL )
    {
        memset( ptr, id, size );
        p = (unsigned char *)ptr;
        while ( --size >= 0 )
        {
            if ( *p++ != id )
                return NULL;
        }
    }
    return ptr;
}

static inline void freeOneAlloc( void *ptr )
{
    if ( mode )
        lsr_xpool_free( &pool, ptr );
    else
        lsr_pfree( ptr );
}

static void * runTest( void *o )
{
    static int sizes[] = { 32, 8, 256, 48, 4096, 64*1024 };
    void * ptrs[sizeof(sizes)/sizeof(sizes[0])];
    myThread_t * p = (myThread_t *)o;

    printf("runTest %d\n", p->m_id);
    
    int cnt;
    for ( cnt = 0; cnt < loopCount; ++cnt )
    {
        int * psizes = sizes;
        int i;
        i = sizeof(sizes)/sizeof(sizes[0]);
        while ( --i >= 0 )
        {
            if ( (ptrs[i] = testOneAlloc( p->m_id, *psizes++ )) == NULL )
                ++p->m_error;
        }
        i = sizeof(sizes)/sizeof(sizes[0]);
        while ( --i >= 0 )
            freeOneAlloc( ptrs[i] );
    }
    
    pthread_exit( (void *)(long)-p->m_id );
    return NULL;
}


int testthrsafe( int ac, char * av[], void *(*runTest)(void *) )
{
    register myThread_t * p;
    register int i;

    if (ac > 2)
    {
        numThreads = atoi(av[2]);
        if (ac > 3)
            loopCount = atoi(av[3]);
    }
    
    printf( "\nRUNNING numThreads %d loop %d\n", numThreads, loopCount );
    p = threadMap;

    if ( mode )
        lsr_xpool_init( &pool );
 
    Timer x( av[0] );
    for ( i = 0; i < numThreads; i++, p++ )
    {
        p->m_exit = 0;
        p->m_id = i;
        p->m_error = 0;
        p->m_waitCount = 0;
            
        if ( pthread_create( &p->m_thread, NULL, runTest, (void *)p ) )
        {
            numThreads = i;
            break;
        }
    }
    p = threadMap;
    for ( i = 0; i < numThreads; i++, p++ )
        pthread_join( p->m_thread, (void**)&p->m_exit );

    if ( mode )
        lsr_xpool_destroy( &pool );
 
    printf( "RUNNING numThreads %d loop %d\n", numThreads, loopCount );

    showEnd( numThreads );

    return 0;
}

/*
 *  Simple thread safe test
 */
int main( int ac, char *av[] )
{
    register int i;
    
    if (ac > 1)
        mode = atoi(av[1]);
    for ( i = 0; i < 10; ++i )
    {
        std::cout << "\n";
        av[0] = (char *)( mode? "THREAD-SAFE XPOOL": "THREAD-SAFE POOL" );
        fprintf( stderr, "%d: %s\n", i, av[0] );
        testthrsafe( ac, av, runTest );
    }
    std::cout << "Bye.\n";  
}
