#pragma once


class Uncopyable
{
protected:
	Uncopyable() {}
	~Uncopyable() {}

private:
	Uncopyable( const Uncopyable& );
	Uncopyable& operator=( const Uncopyable& );
};


#define LOG_WARNING( msg )	{ printf( "[Warning] %s, in %s at %d\n", msg, __FUNCTION__, __LINE__ ); }

