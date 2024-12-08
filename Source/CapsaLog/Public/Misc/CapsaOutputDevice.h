// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "Engine.h"
#include "Misc/BufferedOutputDevice.h"



struct FCapsaOutputDevice : public FBufferedOutputDevice
{
public:

	FCapsaOutputDevice();
	~FCapsaOutputDevice();

	// FBufferedOutputDevice
	virtual void				Serialize( const TCHAR* InData, ELogVerbosity::Type Verbosity, const FName& Category ) override;
	// ~FBufferedOutputDevice

protected:

	/**
	* Perform any specific Initialization.
	*/
	virtual void				Initialize();

	/**
	* Is called every time period, set by TickRate.
	* 
	* @param Seconds The number of seconds since the last tick.
	* @return bool True if Tick was handled correctly, otherwise false.
	*/
	bool						Tick( float Seconds );

	/**
	* How fast, in seconds, to update this Output Device.
	*/
	float						TickRate;

	/**
	* How often this Output Device should attempt to send updates.
	*/
	float						UpdateRate;

	/**
	* How many log lines should be buffered, before we attempt to send updates.
	*/
	int32						MaxLogLines;

private:

	FTSTicker::FDelegateHandle	TickerHandle;
	double						LastUpdateTime;
};
