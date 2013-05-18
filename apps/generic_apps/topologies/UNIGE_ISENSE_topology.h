template<typename Position, typename Radio>
Position get_node_info( Radio* _radio )
{
	//basestation 01
	if ( _radio->id() == 0x9710 )
	{
		return Position( 83, 31, 12 );
	}
	else if ( _radio->id() == 0x99ad )
	{
		return Position( 119, 28, 12 );
	}
	else if ( _radio->id() == 0x1b7f)
	{
		return Position( 159, 12, 11 );
	}
	else if ( _radio->id() == 0x96e5 )
	{
		return Position( 92, 65, 10 );
	}
	else if ( _radio->id() == 0x999d )
	{
		return Position( 132, 60, 14 );
	}
	//basestation 02
	else if ( _radio->id() == 0x9731 )
	{
		return Position( 69, 94, 11 );
	}
	else if ( _radio->id() == 0x15d3 )
	{
		return Position( 54, 97, 12 );
	}
	else if ( _radio->id() == 0x1b95 )
	{
		return Position( 81, 161, 12 );
	}
	else if ( _radio->id() == 0x96c2 )
	{
		return Position( 42, 134, 12 );
	}
	else if ( _radio->id() == 0x9700 )
	{
		return Position( 81, 137, 12 );
	}
	else if ( _radio->id() == 0x1725 )
	{
		return Position( 42, 162, 12 );
	}
	//basestation 03
	else if ( _radio->id() == 0x96fc )
	{
		return Position( 42, 33, 11 );
	}
	else if ( _radio->id() == 0x96f4 )
	{
		return Position( 42, 0, 10 );
	}
	else if ( _radio->id() == 0x99bd )
	{
		return Position( 81, 31, 12 );
	}
	else if ( _radio->id() == 0x15e0 )
	{
		return Position( 81, 0, 10 );
	}
	else if ( _radio->id() == 0x1b3b )
	{
		return Position( 53, 61, 12 );
	}
	else if ( _radio->id() == 0x1520 )
	{
		return Position( 69, 63, 12 );
	}
	else if ( _radio->id() == 0x1b84 )
	{
		return Position( 0, 59, 15 );
	}
	//basestation 04
	else if ( _radio->id() == 0x96e0 )
	{
		return Position( 38, 35, 22 );
	}
	else if ( _radio->id() == 0x9708 )
	{
		return Position( 0, 26, 13 );
	}
	else if ( _radio->id() == 0x1cb9 )
	{
		return Position( 38, 0, 15 );
	}
	//basestation 05
	else if ( _radio->id() == 0x9a0d )
	{
		return Position( 39, 129, 12 );
	}
	else if ( _radio->id() == 0x1b99 )
	{
		return Position( 0, 162, 12 );
	}
	else if ( _radio->id() == 0x96eb )
	{
		return Position( 0, 109, 12 );
	}
	else if ( _radio->id() == 0x1be5 )
	{
		return Position( 30, 97, 8 );
	}
	else if ( _radio->id() == 0x9a0c )
	{
		return Position( 39, 162, 12 );
	}
	return Position( 0, 0, 0 );
}
