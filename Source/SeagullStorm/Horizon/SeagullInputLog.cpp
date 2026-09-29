#include "Horizon/SeagullInputLog.h"

// Named namespace: unity builds merge .cpp files, so helper names must stay unique.
namespace SeagullInputLogPrivate
{
	// Stick values inside this band count as "no input" on that axis.
	constexpr float MoveDeadZone = 0.3f;

	uint8 AxisToCell(float Value)
	{
		if (Value < -MoveDeadZone) return 0;
		if (Value > MoveDeadZone) return 2;
		return 1;
	}
}

void FSeagullInputLog::Reset()
{
	Bytes.Reset();
	Bytes.Reserve(1024);

	// Magic "SGS1" plus a zero seed, filled in by SetSeed.
	const uint8 Header[HeaderBytes] = { 'S', 'G', 'S', '1', 0, 0, 0, 0 };
	for (int32 Index = 0; Index < HeaderBytes; ++Index)
	{
		Bytes.Add(Header[Index]);
	}

	LastMove = 4;
	bTruncated = false;
	bFinished = false;
}

void FSeagullInputLog::SetSeed(int32 Seed, float RunSeconds)
{
	const uint32 Value = static_cast<uint32>(Seed);
	Bytes[4] = static_cast<uint8>(Value & 0xFF);
	Bytes[5] = static_cast<uint8>((Value >> 8) & 0xFF);
	Bytes[6] = static_cast<uint8>((Value >> 16) & 0xFF);
	Bytes[7] = static_cast<uint8>((Value >> 24) & 0xFF);

	AddRecord(RunSeconds, EKind::Seeded, 0);
}

void FSeagullInputLog::RecordMove(float RunSeconds, float X, float Y)
{
	const uint8 Move = QuantizeMove(X, Y);
	if (Move == LastMove)
	{
		return;
	}
	LastMove = Move;
	AddRecord(RunSeconds, EKind::Move, Move);
}

void FSeagullInputLog::RecordLevelUpChoice(float RunSeconds, int32 ChoiceIndex)
{
	const int32 Clamped = ChoiceIndex < 0 ? 0 : (ChoiceIndex > 255 ? 255 : ChoiceIndex);
	AddRecord(RunSeconds, EKind::LevelUpChoice, static_cast<uint8>(Clamped));
}

void FSeagullInputLog::Finish(float RunSeconds)
{
	if (bFinished)
	{
		return;
	}
	AddRecord(RunSeconds, EKind::End, 0);
	bFinished = true;
}

uint8 FSeagullInputLog::QuantizeMove(float X, float Y)
{
	// Row 0 is "up" (Y > 0) so the grid reads like a keypad.
	const uint8 Column = SeagullInputLogPrivate::AxisToCell(X);
	const uint8 Row = static_cast<uint8>(2 - SeagullInputLogPrivate::AxisToCell(Y));
	return static_cast<uint8>(Row * 3 + Column);
}

void FSeagullInputLog::AddRecord(float RunSeconds, EKind Kind, uint8 Value)
{
	if (bFinished)
	{
		return;
	}
	// Every record but the end record keeps room for the end record.
	const int32 ReservedForEnd = (Kind == EKind::End) ? 0 : RecordBytes;
	if (Bytes.Num() + RecordBytes + ReservedForEnd > MaxBytes)
	{
		bTruncated = true;
		return;
	}

	const uint16 Tick = ToTick(RunSeconds);
	Bytes.Add(static_cast<uint8>(Tick & 0xFF));
	Bytes.Add(static_cast<uint8>((Tick >> 8) & 0xFF));
	Bytes.Add(static_cast<uint8>(Kind));
	Bytes.Add(Value);
}

uint16 FSeagullInputLog::ToTick(float RunSeconds)
{
	const float Ticks = RunSeconds * static_cast<float>(TicksPerSecond);
	if (!(Ticks > 0.f)) return 0;
	if (Ticks >= 65535.f) return 65535;
	return static_cast<uint16>(Ticks);
}
