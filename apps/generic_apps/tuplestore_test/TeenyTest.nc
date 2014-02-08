
configuration TeenyTest {}

implementation {
	components MainC, TeenyLimeC;
	components TeenyTestC as App;

	components new TimerMilliC() as Timer0;
	components new TimerMilliC() as Timer1;
	components ActiveMessageC;
	components new AMSenderC(6);
	components new AMReceiverC(6);

	App.Boot -> MainC; //.Boot;
	App.TeenyLIMESystem -> TeenyLimeC;
	App.Timer0 -> Timer0;
	App.Timer1 -> Timer1;
	App.TS -> TeenyLimeC.TupleSpace[unique("TL")];
	App.Packet -> AMSenderC;
	App.Receive -> AMReceiverC;
	App.AMSend -> AMSenderC;
	App.RadioControl -> ActiveMessageC;
}

