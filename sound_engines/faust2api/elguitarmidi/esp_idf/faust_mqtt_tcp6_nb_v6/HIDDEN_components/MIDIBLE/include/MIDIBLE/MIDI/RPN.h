#pragma once

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		enum RPN
		{
			PitchBendSensitivity = 0x0000,
			ChannelFineTuning = 0x0001,
			ChannelCoarseTuning = 0x0002,
			SelectTuningProgram = 0x0003,
			SelectTuningBank = 0x0004,
			ModulationDepthRange = 0x0005,
			NullFunction = (0x7f << 7) + 0x7f,
		};
	}
}
}