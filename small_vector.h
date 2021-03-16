#ifndef SMALLVECTOR_H
#define SMALLVECTOR_H

#include <cstring>
#include <algorithm>

namespace sv
{
    template <typename T, int CapacityRequest = 1>
    struct small_vector
    {
        using SizeType = unsigned int;
        constexpr static auto StructSize = sizeof( small_vector );
        constexpr static auto ElementSize = sizeof( T );
        constexpr static auto InternalCapacity = ( StructSize - sizeof( SizeType ) ) / ElementSize;

        union {
            struct {
                SizeType UsingExternal : 1, Size : 8 * sizeof( SizeType ) - 1;
                T InternalBuffer[ CapacityRequest ];
            };
            struct {
                SizeType : 8 * sizeof( SizeType );
                SizeType Capacity;
                T* ExternalBuffer;
            };
        };

        small_vector()  { memset( this, 0, StructSize ); }
        ~small_vector() { if ( UsingExternal ) delete[] ExternalBuffer; }

        SizeType  size()      const { return Size; }
        bool      empty()     const { return size() == 0; }
        SizeType  capacity()  const { return UsingExternal ? Capacity : InternalCapacity; }
        T*        begin()     const { return UsingExternal ? ExternalBuffer : (T*)InternalBuffer; }
        T*        end()       const { return begin() + size(); }

        T& operator[]( int n ) { return begin()[ n ]; }

        T* allocate( SizeType n ) { return new T[ n ]; }

        void reallocate( SizeType n )
        {
            if ( UsingExternal )
                delete[] ExternalBuffer;
            else
                UsingExternal = true;
            ExternalBuffer = allocate( n );
            Capacity = n;
        }
        
        void write_to( void* d, int n ) const { memcpy( d, begin(), n * ElementSize ); }
        
        void reserve( SizeType n )
        {
            if ( n <= capacity() ) return;
            auto NewBuffer = allocate( n );
            write_to( NewBuffer, size() );
            if ( UsingExternal )
                delete[] ExternalBuffer;
            else
                UsingExternal = true;
            ExternalBuffer = NewBuffer;
            Capacity = n;
        }

        template<int N>
        auto& assign_imp( const small_vector<T, N>& Source )
        {
            Size = Source.size();
            if ( capacity() < Size ) reallocate( Size );
            Source.write_to( begin(), Size );
            return *this;
        }

        template <int N>
        auto& operator=( const small_vector<T, N>& Source ) { return assign_imp( Source ); }
        auto& operator=( const small_vector      & Source ) { return assign_imp( Source ); }

        void push_back( const T& NewElement )
        {
            if ( size() >= capacity() ) reserve( capacity() * 2 );
            *end() = NewElement;
            ++Size;
        }
        
        void operator+=( const T& NewElement ) { push_back( NewElement ); }
        
        void pop_back() { if ( size() ) --Size; }

        void erase_every(const T TargetValue)
        {
            Size -= end() - std::partition( begin(), end(), [ TargetValue ]( T Element ) { return Element != TargetValue; } );
        }
    };

    namespace debug
    {
    
        template <typename T>
        void DumpBinary( T& src, int bytes = 1 )
        {
            bytes *= sizeof( T );

            constexpr int ColumnCount = 8;

            if ( bytes > ColumnCount && bytes % ColumnCount > 0 )
                for ( int padding = ColumnCount - bytes % ColumnCount; padding-- > 0; )
                    printf( "         " );

            auto Byte = (unsigned char*)( &src );
            for ( int i = bytes; i-- > 0; )
            {
                for ( int bit = 8; bit-- > 0; )
                    putchar( Byte[ i ] >> bit & 1 ? '1' : 'o' );
                putchar(' ');
                if ( i % ColumnCount == 0 ) printf( "[%3u ]\n", i );
            }
            putchar( '\n' );
        }
    
        struct dump {
        
            auto& operator<<( const char* str )
            {
                printf( "%s", str );
                return *this;
            }

            template <typename T,int N>
            auto& operator<<( const small_vector<T, N>& Source )
            {
                printf("[ Small Vector Content ]   Size: %i   Capacity: %i \n", Source.size(), Source.capacity() );
                DumpBinary(Source);
                if ( Source.UsingExternal )
                {
                    puts("[ External Content ]");
                    DumpBinary( Source.ExternalBuffer[ 0 ], Source.capacity() );
                }
                return *this;
            }
        };
        
    }  // namespace debug

}  // namespace sv



#endif

