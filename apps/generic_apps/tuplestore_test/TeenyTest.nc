
configuration TeenyTest {}

implementation {
	components MainC, TeenyLimeC;
	components TeenyTestC as App;
	components PrintfC;

	components new TimerMilliC() as Timer0;
	components new TimerMilliC() as Timer1;
	components new TimerMilliC() as Timer2;
	components ActiveMessageC;
	components new AMSenderC(6);
	components new AMReceiverC(6);

	components McuSleepC;
	components CC2420ActiveMessageC as LplC;

	App.Boot -> MainC; //.Boot;
	App.TeenyLIMESystem -> TeenyLimeC;
	App.Timer0 -> Timer0;
	App.Timer1 -> Timer1;
	App.Timer2 -> Timer2;
	App.TS -> TeenyLimeC.TupleSpace[unique("TL")];
	App.Packet -> AMSenderC;
	App.Receive -> AMReceiverC;
	App.AMSend -> AMSenderC;
	App.RadioControl -> ActiveMessageC;

	App.PrintfControl -> PrintfC;
	App.PrintfFlush -> PrintfC;

	App.McuSleep -> McuSleepC;
	/*App.PhysSleep -> LplC;*/

	App.LPL -> LplC;

	App.Tuning -> TeenyLimeC.Tuning[unique("TLTuning")];
}

