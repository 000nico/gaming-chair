#include "global.hpp"
#include "../../gui.h"

int zEntry::zMain( )
{
	if ( !render::initialize( ) )
	{
		std::printf( "failed to initialize renderer\n" );
		return -1;
	}

	render::loop( );
	return 0;
}