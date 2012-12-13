#define DB 0
template<typename Position, typename Radio>
Position get_node_info( Radio* radio )
{
	typedef typename Radio::TxPower TxPower;
	TxPower power;
//	if ( radio->id() == 0x1b84 )
//	{
//		power.set_dB( DB );
//		radio->set_power( power );
//		return Position ( 15, 20, 10);
//	}
	if ( radio->id() == 0x1bb3 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 45, 10, 10 );
	}
	else if ( radio->id() == 0x99a8 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 35, 10, 10 );
	}
	else if ( radio->id() == 0x997e )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 20, 10, 10 );
	}
	else if ( radio->id() == 0x14e2 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 255, 140, 20 );
	}
	else if ( radio->id() == 0x1cca )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 255, 120, 20 );
	}
	else if ( radio->id() == 0xf85d )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 255, 115, 20 );
	}
	else if ( radio->id() == 0x5a34 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 170, 5, 0 );
	}
	else if ( radio->id() == 0xcf04 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 205, 15, 10 );
	}
	else if ( radio->id() == 0xcc33 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 170, 25, 10 );
	}
	else if ( radio->id() == 0x151f )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 170, 135, 10 );
	}
	else if ( radio->id() == 0x1c6c )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 145, 135, 20 );
	}
	else if ( radio->id() == 0x1c73 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 145, 120, 20 );
	}
	else if ( radio->id() == 0x61e1 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 60, 20 );
	}
	else if ( radio->id() == 0xf7b7 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 60, 20 );
	}
	else if ( radio->id() == 0x1c2c )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 310, 60, 10 );
	}
	else if ( radio->id() == 0x1bd8 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 310, 30, 10 );
	}
	else if ( radio->id() == 0x1bb0 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 285, 10, 10 );
	}
	else if ( radio->id() == 0xcc3a )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 60, 10, 10 );
	}
	else if ( radio->id() == 0x85a4 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 80, 10, 10 );
	}
	else if ( radio->id() == 0x80f5 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 100, 10, 10 );
	}
	else if ( radio->id() == 0x599d )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 25, 140, 10 );
	}
	else if ( radio->id() == 0xcbe4 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 140, 10 );
	}
	else if ( radio->id() == 0xf859 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 110, 10 );
	}
	else if ( radio->id() == 0x99af )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 275, 25, 10 );
	}
	else if ( radio->id() == 0x753d )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 275, 10, 10 );
	}
	else if ( radio->id() == 0x1b57 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 255, 10, 10 );
	}
	else if ( radio->id() == 0x5a35 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 110, 20 );
	}
	else if ( radio->id() == 0x1b74 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 125, 20 );
	}
	else if ( radio->id() == 0xcc3d )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 140, 20 );
	}
	else if ( radio->id() == 0x1b5a )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 345, 105, 20 );
	}
	else if ( radio->id() == 0x970b )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 345, 105, 20 );
	}
	else if ( radio->id() == 0x1b6b )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 345, 105, 20 );
	}
	else if ( radio->id() == 0x1c72 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 315, 120, 20 );
	}
	else if ( radio->id() == 0xf851 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 315, 115, 20 );
	}
	else if ( radio->id() == 0xcff1 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 315, 105, 20 );
	}
	else if ( radio->id() == 0x1cd2 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 140, 10, 10 );
	}
	else if ( radio->id() == 0x7e6c )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 140, 40, 10 );
	}
	else if ( radio->id() == 0xcc43 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 140, 50, 10 );
	}
	else if ( radio->id() == 0x85ba )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 250, 30, 10 );
	}
	else if ( radio->id() == 0x9960 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 250, 20, 10 );
	}
	else if ( radio->id() == 0x9961 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 250, 10, 20 );
	}
	else if ( radio->id() == 0x14f7 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 85, 70, 20 );
	}
	else if ( radio->id() == 0x96f9 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 75, 70, 20 );
	}
	else if ( radio->id() == 0xc179 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 60, 70, 20 );
	}
	else if ( radio->id() == 0x96df )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 135, 10, 10 );
	}
	else if ( radio->id() == 0x9995 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 105, 30, 10 );
	}
	else if ( radio->id() == 0x971e )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 105, 60, 10 );
	}
	else if ( radio->id() == 0xcbe5 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 230, 100, 10 );
	}
	else if ( radio->id() == 0xcbe5 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 230, 100, 10 );
	}
	else if ( radio->id() == 0x1234 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 230, 120, 10 );
	}
	else if ( radio->id() == 0x14e0 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 230, 140, 10 );
	}
	else if ( radio->id() == 0x96f0 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 285, 105, 20 );
	}
	else if ( radio->id() == 0x1721 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 285, 140, 10 );
	}
	else if ( radio->id() == 0x5980 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 315, 140, 10 );
	}
	else if ( radio->id() == 0x61e5 )
	{
		power.set_dB( DB );
		radio->set_power( power );
		return Position( 10, 60, 20 );
	}
	return Position( 0, 0, 0 );
}
