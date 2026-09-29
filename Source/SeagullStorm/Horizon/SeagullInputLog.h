#pragma once

#include "CoreMinimal.h"

/**
 * Compact input log of one run for horizOn Validated Actions.
 *
 * The SDK hashes these bytes (SHA-256) for the validated submit and, when the server asks
 * for evidence, uploads the same bytes. A replay checker can rebuild the run from the seed
 * and the recorded inputs.
 *
 * Layout (little endian):
 *   Header, 8 bytes: magic "SGS1", uint32 seed of the run ticket (0 until the ticket arrives).
 *   Records, 4 bytes each: uint16 tick, uint8 kind, uint8 value.
 *     tick  = run time in 1/20 s (game time, pauses excluded), saturates at 65535.
 *     kind  = EKind below.
 *     value = Move: stick direction on a 3x3 grid (row * 3 + column, 4 = no input),
 *             LevelUpChoice: index of the picked card, Seeded and End: 0.
 *
 * A move record is only written when the direction changes, so a three minute run stays
 * at a few kilobytes. The log never grows beyond MaxBytes (the server's evidence limit):
 * further records are dropped and IsTruncated() turns true.
 */
class FSeagullInputLog
{
public:
	/** Evidence limit of the server (evidence.maxBytes). */
	static constexpr int32 MaxBytes = 32768;

	static constexpr int32 HeaderBytes = 8;
	static constexpr int32 RecordBytes = 4;
	static constexpr int32 TicksPerSecond = 20;

	enum class EKind : uint8
	{
		Move = 1,
		LevelUpChoice = 2,
		Seeded = 3,
		End = 255
	};

	FSeagullInputLog() { Reset(); }

	/** Starts a new log: header with seed 0, no records. */
	void Reset();

	/** Writes the ticket seed into the header and marks the moment it was applied. */
	void SetSeed(int32 Seed, float RunSeconds);

	/** Records the move stick (X right, Y up, -1 to 1) when its 3x3 direction changed. */
	void RecordMove(float RunSeconds, float X, float Y);

	/** Records the index of the chosen level-up card. */
	void RecordLevelUpChoice(float RunSeconds, int32 ChoiceIndex);

	/** Closes the log with an end record. Later calls do nothing. */
	void Finish(float RunSeconds);

	const TArray<uint8>& GetBytes() const { return Bytes; }
	bool IsTruncated() const { return bTruncated; }

	/** 3x3 direction of a stick value: row * 3 + column, 4 = centre (no input). */
	static uint8 QuantizeMove(float X, float Y);

private:
	void AddRecord(float RunSeconds, EKind Kind, uint8 Value);
	static uint16 ToTick(float RunSeconds);

	TArray<uint8> Bytes;
	uint8 LastMove = 4;
	bool bTruncated = false;
	bool bFinished = false;
};
