// Copyright Companion Group, Ltd. Made available under the MIT license

#include "Misc/CapsaOutputDevice.h"

#include "Settings/CapsaSettings.h"
#include "CapsaCoreSubsystem.h"


FCapsaOutputDevice::FCapsaOutputDevice()
	: TickRate( 1.f )
	, UpdateRate( 0.f )
	, MaxLogLines( 100 )
	, LastUpdateTime( 0 )
{
	// TODO: Make this a config option
	FilterLevel = ELogVerbosity::All;
	Initialize();
}

FCapsaOutputDevice::~FCapsaOutputDevice()
{
	if( TickerHandle.IsValid() == true )
	{
		GLog->RemoveOutputDevice( this );
		FTSTicker::GetCoreTicker().RemoveTicker( TickerHandle );
	}
}

void FCapsaOutputDevice::Serialize( const TCHAR* InData, ELogVerbosity::Type Verbosity, const FName& Category )
{
	if( Verbosity > FilterLevel )
	{
		return;
	}

	FScopeLock ScopeLock( &SynchronizationObject );
	BufferedLines.Emplace( InData, Category, Verbosity, FDateTime::Now().ToUnixTimestampDecimal() );
}

void FCapsaOutputDevice::Initialize()
{
	UCapsaSettings* CapsaSettings = GetMutableDefault<UCapsaSettings>();
	TickRate = CapsaSettings->GetLogTickRate();
	UpdateRate = CapsaSettings->GetMaxTimeBetweenLogFlushes();
	MaxLogLines = CapsaSettings->GetMaxLogLinesBetweenLogFlushes();

	LastUpdateTime = FPlatformTime::Seconds();

	if( TickRate > 0.0f )
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateRaw( this, &FCapsaOutputDevice::Tick ), TickRate );
		GLog->AddOutputDevice( this );
	}
}

bool FCapsaOutputDevice::Tick( float Seconds )
{
	if( BufferedLines.IsEmpty() == true )
	{
		return true;
	}

	double Now = FPlatformTime::Seconds();
	bool bExceedTime = false;
	bool bExceedLines = false;

	if( LastUpdateTime + UpdateRate < Now )
	{
		bExceedTime = true;
	}

	if( BufferedLines.Num() >= MaxLogLines )
	{
		bExceedLines = true;
	}

	if( bExceedTime == false && bExceedLines == false )
	{
		return true;
	}

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem != nullptr &&
		CapsaCoreSubsystem->IsValidLowLevelFast() == true)
	{
		if( CapsaCoreSubsystem->IsAuthenticated() == true )
		{
			TArray<FBufferedLine> BufferToSend;
			GetContents( BufferToSend );
			CapsaCoreSubsystem->SendLog( BufferToSend );
		} else // Trigger authentication attempt
		{
			CapsaCoreSubsystem->RequestClientAuth();
		}
	}

	LastUpdateTime = Now;
	FScopeLock ScopeLock( &SynchronizationObject );
	BufferedLines.Empty();

	return true;
}
